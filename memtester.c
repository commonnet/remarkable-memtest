/*
 * memtester version 4
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

#define __version__ "4.3.0"

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
#ifdef TEST_NARROW_WRITES
    test_8bit_wide_random,
    test_16bit_wide_random,
#endif
};
size_t const kTestsLen = sizeof(kTests) / sizeof(kTests[0]);

size_t const kPageSize = 4096;
char const * const kDeviceName = "/dev/mem";

#ifdef PHYS_ADDR_BASE
off_t const kPhysAddrBase = (off_t)PHYS_ADDR_BASE;
#else
off_t const kPhysAddrBase = (off_t)0;
#endif

#ifdef PHYS_ADDR_SIZE
size_t const kTestAddrSpaceBytes = (size_t)PHYS_ADDR_SIZE;
#else
size_t const kTestAddrSpaceBytes = (size_t)2 << 30;
#endif

size_t const kTestAddrSpaceUl = kTestAddrSpaceBytes / sizeof(ul);
size_t const kHalfSpaceBytes = kTestAddrSpaceBytes / 2;
size_t const kHalfSpaceUl = kHalfSpaceBytes / sizeof(ul);
size_t const kNumLoops = 1;


int main(int argc, char const * const * argv) {
    int exit_code = 0;

    int const memfd = open(kDeviceName, O_RDWR | O_SYNC);
    if (memfd < 0) {
        fprintf(
            stderr,
            "failed to open %s for physical memory: %s\n",
            kDeviceName,
            strerror(errno));
        exit(EXIT_FAIL_NONSTARTER);
    }
    void volatile * const buf = (void volatile * const) mmap(
        NULL,
        kTestAddrSpaceBytes,
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_LOCKED,
        memfd,
        kPhysAddrBase);
    if (buf == MAP_FAILED) {
        fprintf(
            stderr,
            "failed to mmap %s for physical memory: %s\n",
            kDeviceName,
            strerror(errno));
        exit(EXIT_FAIL_NONSTARTER);
    }

    ulv* const bufa = (ulv * const)buf;
    ulv* const bufb = (ulv * const)((size_t)buf + kHalfSpaceBytes);

    for (size_t loop = 0; loop != kNumLoops; ++loop) {
        fprintf(stdout, "Loop %lu/%lu:\n", loop+1, kNumLoops);
        fprintf(stdout, "  %-20s: ", "Stuck Address");
        fflush(stdout);
        if (!test_stuck_address(buf, kTestAddrSpaceUl)) {
            printf("ok\n");
        } else {
            exit_code |= EXIT_FAIL_ADDRESSLINES;
        }
        for (size_t i = 0; i != kTestsLen; ++i) {
            if (!kTests[i](bufa, bufb, kHalfSpaceUl)) {
                fprintf(stdout, "ok\n");
                fflush(stdout);
            } else {
                exit_code |= EXIT_FAIL_OTHERTEST;
            }
        }
        fprintf(stdout, "\n");
        fflush(stdout);
    }
    fprintf(stdout, "Done.\n");
    fflush(stdout);
    return exit_code;
}
