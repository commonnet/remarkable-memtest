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

#include "io.h"
#include "types.h"
#include "sizes.h"
#include "consts.h"
#include "tests.h"

#define EXIT_FAIL_NONSTARTER    0x01
#define EXIT_FAIL_ADDRESSLINES  0x02
#define EXIT_FAIL_OTHERTEST     0x04

test_t const kTests[] = {
    test_random_value,
    test_xor_comparison,
    test_sub_comparison,
    test_mul_comparison,
    test_div_comparison,
    test_or_comparison,
    test_and_comparison,
    test_seqinc_comparison,
    test_bitflip_comparison,
    test_blockseq_comparison,
    test_checkerboard_comparison,
    test_bitspread_comparison,
    test_solidbits_comparison,
    test_walkbits1_comparison,
    test_walkbits0_comparison,
};
#define kTestsLen (size_t)(sizeof(kTests) / sizeof(kTests[0]))

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
        fprintf(
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
        fprintf(
            stderr,
            "failed to mmap %s for physical memory: %s\n",
            kDeviceName,
            strerror(errno));
        buf.ptr = NULL;
        close(buf.fd);
        return buf;
    }
#else // USE_MMAP
    while (buf.ptr == NULL && buf.len > 0) {
        buf.ptr = malloc(buf.len);
        if (buf.ptr == NULL) {
            buf.len -= PAGE_SIZE_VAL;
        }
    }
    if (buf.ptr == NULL) {
        fprintf(
            stderr,
            "failed to malloc: %s\n",
            strerror(errno));
        return buf;
    }
    fprintf(stdout, "malloced %lu bytes\n", buf.len);
    int const ret = mlock((void const *)buf.ptr, buf.len);
    if (ret < 0) {
        fprintf(stderr, "Failed to mlock\n");
        free(buf.ptr);
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


int main(int argc, char const * const * argv) {
    int exit_code = 0;
    buf_t buf = buf_new(PHYS_ADDR_SIZE_VAL);
    if (!buf_is_valid(&buf)) {
        exit(EXIT_FAIL_NONSTARTER);
    }

    size_t const half_size = buf.len / 2;
    ulv* const bufa = (ulv * const)buf.ptr;
    ulv* const bufb = (ulv * const)((size_t)buf.ptr + half_size);

    for (size_t loop = 0; loop != kNumLoops; ++loop) {
        fprintf(stdout, "Loop %lu/%lu\n", loop+1, kNumLoops);
        fprintf(stdout, "Running stuck address test: ");
        fflush(stdout);
        if (!test_stuck_address(bufa, buf.len / sizeof(ul))) {
            fprintf(stdout, "ok\n");
        } else {
            fprintf(stdout, "failed\n");
            exit_code |= EXIT_FAIL_ADDRESSLINES;
        }
        fflush(stdout);
        for (size_t i = 0; i != kTestsLen; ++i) {
            fprintf(stdout, "Running test %lu: ", i);
            fflush(stdout);
            if (!kTests[i](bufa, bufb, half_size / sizeof(ul))) {
                fprintf(stdout, "ok\n");
            } else {
                fprintf(stdout, "failed\n");
                exit_code |= EXIT_FAIL_OTHERTEST;
            }
            fflush(stdout);
        }
        fprintf(stdout, "\n");
        fflush(stdout);
    }
    fprintf(stdout, "Done.\n");
    fflush(stdout);
    buf_drop(&buf);
    return exit_code;
}
