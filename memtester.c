/*
 * memtester version 5
 *
 * Very simple but very effective user-space memory tester.
 * Originally by Simon Kirby <sim@stormix.com> <sim@neato.org>
 * Version 2 by Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Version 3 not publicly released.
 * Version 4 rewrite:
 * Copyright (C) 2004-2012 Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 */

#define __version__ "5.0.0"

#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "io.h"
#include "types.h"
#include "sizes.h"
#include "consts.h"
#include "tests.h"

#define EXIT_FAIL_NONSTARTER    0x01
#define EXIT_FAIL_ADDRESSLINES  0x02
#define EXIT_FAIL_OTHERTEST     0x04

test_t const kCmpTests[] = {
    test_bitflip_cmp,
    test_blockseq_cmp,
    test_checkerboard_cmp,
    test_bitspread_cmp,
    test_solidbits_cmp,
    test_walkbits1_cmp,
    test_walkbits0_cmp,
};
#define kCmpTestsLen (size_t)(sizeof(kCmpTests) / sizeof(kCmpTests[0]))

size_t const kMaxBackoffBytes = 1ull << 20ull;
size_t const kNumLoops = 1;
#ifdef USE_MMAP
char const * const kDeviceName = "/dev/mem";
#endif // USE_MMAP


typedef struct buf_t {
    void* ptr;
    size_t len;
#ifdef USE_MMAP
    int fd;
#endif
} buf_t;


buf_t buf_new(size_t init_len) {
    buf_t buf = {
        .ptr = NULL,
        .len = init_len,
#ifdef USE_MMAP
        .fd = -1
#endif
    };
#ifdef USE_MMAP
    buf.fd = open(kDeviceName, O_RDWR | O_SYNC);
    if (buf.fd < 0) {
        DEBUG_FPRINTF(
            stderr,
            "failed to open %s for physical memory: %s\n",
            kDeviceName,
            strerror(errno));
        return buf;
    }
    buf.ptr = mmap(
        NULL,
        buf.len,
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_LOCKED,
        memfd,
        PHYS_ADDR_BASE_VAL);
    if (buf.ptr == MAP_FAILED) {
        DEBUG_FPRINTF(
            stderr,
            "failed to mmap %s for physical memory: %s\n",
            kDeviceName,
            strerror(errno));
        buf.ptr = NULL;
        close(buf.fd);
        return buf;
    }
#else // USE_MMAP
    size_t back_off_bytes = PAGE_SIZE_VAL;
    while (buf.ptr == NULL && buf.len > 0) {
        buf.ptr = malloc(buf.len);
        if (buf.ptr == NULL) {
            buf.len -= back_off_bytes;
            back_off_bytes *= 2;
            back_off_bytes =
                back_off_bytes >= kMaxBackoffBytes ?
                kMaxBackoffBytes : back_off_bytes;
        }
    }
    if (buf.ptr == NULL) {
        DEBUG_FPRINTF(
            stderr,
            "failed to malloc: %s\n",
            strerror(errno));
        return buf;
    }
    DEBUG_FPRINTF(stdout, "malloced %lu bytes\n", buf.len);
    int const ret = mlock((void const *)buf.ptr, buf.len);
    if (ret < 0) {
        DEBUG_FPRINTF(stderr, "Failed to mlock\n");
        free(buf.ptr);
        buf.ptr = NULL;
    }
#endif // USE_MMAP
    return buf;
}


void buf_drop(buf_t* buf) {
#ifdef USE_MMAP
    munmap(buf->ptr, buf->len);
    close(buf->fd);
#else
    munlock((void const *)buf->ptr, buf->len);
    free(buf->ptr);
#endif
}


int buf_is_valid(buf_t* buf) {
    return buf->ptr != NULL;
}


typedef struct test_info_t {
    size_t bytes_tested;
    size_t num_failures;
    clock_t run_time;
} test_info_t;


test_info_t test_info_new() {
    test_info_t info = { .bytes_tested = 0, .num_failures = 0, .run_time = 0};
}


void test_info_print_to_stream(test_info_t* info, FILE* stream) {
    fprintf(stream, "Bytes Tested: %zu\n", info->bytes_tested);
    fprintf(stream, "Test failures: %zu\n", info->num_failures);
    fprintf(stream, "Run Time(s): %zu\n", info->run_time/CLOCKS_PER_SEC);
}


int main(int argc, char const * const * argv) {
    int exit_code = 0;
    buf_t buf = buf_new(PHYS_ADDR_SIZE_VAL);
    if (!buf_is_valid(&buf)) {
        exit(EXIT_FAIL_NONSTARTER);
    }
    test_info_t info = test_info_new();
    info.bytes_tested = buf.len;

    size_t const half_size = buf.len / 2;
    size_t const ul_size = buf.len / sizeof(ul);
    size_t const half_ul_size = ul_size / 2;
    ulv* const bufa = (ulv * const)buf.ptr;
    ulv* const bufb = (ulv * const)((size_t)buf.ptr + half_size);

    clock_t const start_time = clock();
    for (size_t loop = 0; loop != kNumLoops; ++loop) {
        DEBUG_FPRINTF(stdout, "Loop %lu/%lu\n", loop+1, kNumLoops);
        DEBUG_FPRINTF(stdout, "Running stuck address test: ");
        DEBUG_FFLUSH(stdout);
        size_t const num_errors = test_stuck_address(bufa, ul_size);
        if (num_errors == 0) {
            DEBUG_FPRINTF(stdout, "ok\n");
        } else {
            DEBUG_FPRINTF(stdout, "failed\n");
            exit_code |= EXIT_FAIL_ADDRESSLINES;
        }
        DEBUG_FFLUSH(stdout);
        info.num_failures += num_errors;
        for (size_t i = 0; i != kCmpTestsLen; ++i) {
            DEBUG_FPRINTF(stdout, "Running test %lu: ", i);
            DEBUG_FFLUSH(stdout);
            size_t const num_errors = kCmpTests[i](bufa, bufb, half_ul_size);
            if (num_errors == 0) {
                DEBUG_FPRINTF(stdout, "ok\n");
            } else {
                DEBUG_FPRINTF(stdout, "failed\n");
                exit_code |= EXIT_FAIL_OTHERTEST;
            }
            DEBUG_FFLUSH(stdout);
            info.num_failures += num_errors;
        }
        DEBUG_FPRINTF(stdout, "\n");
        DEBUG_FFLUSH(stdout);
    }
    info.run_time = clock() - start_time;
    DEBUG_FPRINTF(stdout, "Done.\n");
    DEBUG_FFLUSH(stdout);
    test_info_print_to_stream(&info, stdout);
    buf_drop(&buf);
    return exit_code;
}
