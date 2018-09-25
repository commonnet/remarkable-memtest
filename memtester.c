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
#include <stdio.h>
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


int buf_init(buf_t* buf, size_t init_len) {
    buf->len = init_len;
#ifdef USE_MMAP
    buf->fd = open(kDeviceName, O_RDWR | O_SYNC);
    if (buf->fd < 0) {
        DEBUG_FPRINTF(
            stderr,
            "failed to open %s for physical memory: %s\n",
            kDeviceName,
            strerror(errno));
        return buf;
    }
    buf->ptr = mmap(
        NULL,
        buf->len,
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_LOCKED,
        memfd,
        PHYS_ADDR_BASE_VAL);
    if (buf->ptr == MAP_FAILED) {
        DEBUG_FPRINTF(
            stderr,
            "failed to mmap %s for physical memory: %s\n",
            kDeviceName,
            strerror(errno));
        close(buf->fd);
        return -1;
    }
#else // USE_MMAP
    buf->ptr = NULL;
    size_t back_off_bytes = PAGE_SIZE_VAL;
    while (buf->ptr == NULL && buf->len > 0) {
        buf->ptr = malloc(buf->len);
        if (buf->ptr == NULL) {
            buf->len -= back_off_bytes;
            back_off_bytes *= 2;
            back_off_bytes =
                back_off_bytes >= kMaxBackoffBytes ?
                kMaxBackoffBytes : back_off_bytes;
        }
    }
    if (buf->ptr == NULL) {
        DEBUG_FPRINTF(
            stderr,
            "failed to malloc: %s\n",
            strerror(errno));
        return -1;
    }
    DEBUG_FPRINTF(stdout, "malloced %lu bytes\n", buf->len);
    int const ret = mlock((void const *)buf->ptr, buf->len);
    if (ret < 0) {
        DEBUG_FPRINTF(stderr, "Failed to mlock\n");
        free(buf->ptr);
        return ret;
    }
#endif // USE_MMAP
    return 0;
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


typedef struct test_info_t {
    size_t bytes_tested;
    size_t num_failures;
    clock_t run_time;
} test_info_t;


void test_info_print_to_stream(test_info_t* info, FILE* stream) {
    fprintf(stream, "Bytes Tested: %zu\n", info->bytes_tested);
    fprintf(stream, "Test failures: %zu\n", info->num_failures);
    fprintf(stream, "Run Time(s): %zu\n", info->run_time/CLOCKS_PER_SEC);
}


int main(int argc, char const * const * argv) {
    int exit_code = 0;
    buf_t buf;
    if (buf_init(&buf, PHYS_ADDR_SIZE_VAL) != 0) {
        exit(EXIT_FAIL_NONSTARTER);
    }
    test_info_t info = {.bytes_tested = buf.len, .num_failures = 0};

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
