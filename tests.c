/*
 * Very simple but very effective user-space memory tester.
 * Originally by Simon Kirby <sim@stormix.com> <sim@neato.org>
 * Version 2 by Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Version 3 not publicly released.
 * Version 4 rewrite:
 * Copyright (C) 2004-2012 Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 * This file contains the functions for the actual tests, called from the
 * main routine in memtester.c.  See other comments in that file.
 *
 */

#include <sys/types.h>
#include <stdlib.h>
#include <limits.h>

#include "io.h"
#include "types.h"
#include "sizes.h"
#include "consts.h"

#define ONE 0x00000001L


void compare_regions_print_error(ul va, ul vb, ul index) {
    ul const byte_index = (ul)(index * sizeof(ul));
#ifdef USE_MMAP
    off_t const physaddr = PHYS_ADDR_BASE_VAL + byte_index;
    DEBUG_FPRINTF(
        stderr,
        "FAILURE: 0x%08lx != 0x%08lx at physical address 0x%08lx.\n",
        va,
        vb,
        physaddr);
#else
    DEBUG_FPRINTF(
        stderr,
        "FAILURE: 0x%08lx != 0x%08lx at byte index 0x%08lx.\n",
        va,
        vb,
        byte_index);
#endif
    DEBUG_FFLUSH(stderr);
    DEBUG_FSYNC(fileno(stderr));
}

size_t compare_regions(ulv *bufa, ulv *bufb, size_t count) {
    size_t num_errors = 0;
    for (size_t i = 0; i != count; ++i) {
        ul const va = bufa[i], vb = bufb[i];
        if (va != vb) {
            ++num_errors;
            compare_regions_print_error(va, vb, i);
        }
    }
    return num_errors;
}


void test_stuck_address_print_error(ul index) {
    ul const byte_index = (ul)(index * sizeof(ul));
#ifdef USE_MMAP
    off_t const physaddr = PHYS_ADDR_BASE_VAL + byte_index;
    DEBUG_FPRINTF(
        stderr,
        "FAILURE: possible bad address line at physical address 0x%08lx.\n",
        physaddr);
#else
    DEBUG_FPRINTF(
        stderr,
        "FAILURE: possible bad address line at byte index 0x%08lx.\n",
        byte_index);
#endif
}

size_t test_stuck_address(ulv *bufa, size_t count) {
    size_t num_errors = 0;
    ulv *p1 = bufa;

    for (uint j = 0; j != 16; ++j) {
        p1 = (ulv *) bufa;
        for (size_t i = 0; i != count; ++i) {
            *p1 = ((j + i) % 2) == 0 ? (ul) p1 : ~((ul) p1);
            *p1++;
        }
        p1 = (ulv *) bufa;
        for (size_t i = 0; i != count; ++i, ++p1) {
            ul const expected = ((j + i) % 2) == 0 ? (ul) p1 : ~((ul) p1);
            if (*p1 != expected) {
                ++num_errors;
                test_stuck_address_print_error(i);
            }
        }
    }
    return num_errors;
}


size_t test_binop(
    ulv* bufa,
    ulv* bufb,
    size_t count,
    binop_t binop,
    ul val)
{
    ulv *p1 = bufa, *p2 = bufb;
    for (size_t i = 0; i != count; ++i) {
        *p1 = binop(*p1, val);
        *p2 = binop(*p2, val);
        ++p1;
        ++p2;
    }
    return compare_regions(bufa, bufb, count);
}


size_t test_solidbits_cmp(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa, *p2 = bufb;
    uint j;
    ul q;

    size_t num_errors = 0;
    for (j = 0; j < 64; j++) {
        q = (j % 2) == 0 ? UL_ONEBITS : 0;
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        for (size_t i = 0; i != count; ++i) {
            *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
        }
        num_errors += compare_regions(bufa, bufb, count);
    }
    return num_errors;
}


size_t test_checkerboard_cmp(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa, *p2 = bufb;
    ul q;
    size_t num_errors = 0;

    for (size_t j = 0; j != 64; ++j) {
        q = (j % 2) == 0 ? CHECKERBOARD1 : CHECKERBOARD2;
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        for (size_t i = 0; i != count; ++i) {
            *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
        }
        num_errors += compare_regions(bufa, bufb, count);
    }
    return num_errors;
}


size_t test_blockseq_cmp(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa, *p2 = bufb;
    unsigned int j;
    size_t i;

    size_t num_errors = 0;
    for (j = 0; j < 256; j++) {
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        for (i = 0; i < count; i++) {
            *p1++ = *p2++ = (ul) UL_BYTE(j);
        }
        num_errors += compare_regions(bufa, bufb, count);
    }
    return num_errors;
}

size_t test_walkbits0_cmp(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa, *p2 = bufb;
    unsigned int j;
    size_t i;

    size_t num_errors = 0;
    for (j = 0; j < UL_LEN * 2; j++) {
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = ONE << j;
            } else { /* Walk it back down. */
                *p1++ = *p2++ = ONE << (UL_LEN * 2 - j - 1);
            }
        }
        num_errors += compare_regions(bufa, bufb, count);
    }
    return num_errors;
}

size_t test_walkbits1_cmp(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa, *p2 = bufb;
    unsigned int j;
    size_t i;

    size_t num_errors = 0;
    for (j = 0; j < UL_LEN * 2; j++) {
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = UL_ONEBITS ^ (ONE << j);
            } else { /* Walk it back down. */
                *p1++ = *p2++ = UL_ONEBITS ^ (ONE << (UL_LEN * 2 - j - 1));
            }
        }
        num_errors += compare_regions(bufa, bufb, count);
    }
    return num_errors;
}


size_t test_bitspread_cmp(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa, *p2 = bufb;
    unsigned int j;
    size_t i;

    size_t num_errors = 0;
    for (j = 0; j < UL_LEN * 2; j++) {
        p1 = (ulv *) bufa;
        p2 = (ulv *) bufb;
        for (i = 0; i < count; i++) {
            if (j < UL_LEN) { /* Walk it up. */
                *p1++ = *p2++ = (i % 2 == 0)
                    ? (ONE << j) | (ONE << (j + 2))
                    : UL_ONEBITS ^ ((ONE << j)
                                    | (ONE << (j + 2)));
            } else { /* Walk it back down. */
                *p1++ = *p2++ = (i % 2 == 0)
                    ? (ONE << (UL_LEN * 2 - 1 - j)) | (ONE << (UL_LEN * 2 + 1 - j))
                    : UL_ONEBITS ^ (ONE << (UL_LEN * 2 - 1 - j)
                                    | (ONE << (UL_LEN * 2 + 1 - j)));
            }
        }
        num_errors += compare_regions(bufa, bufb, count);
    }
    return num_errors;
}


size_t test_bitflip_cmp(ulv *bufa, ulv *bufb, size_t count) {
    ulv *p1 = bufa, *p2 = bufb;
    unsigned int j, k;
    ul q;
    size_t i;

    size_t num_errors = 0;
    for (k = 0; k < UL_LEN; k++) {
        q = ONE << k;
        for (j = 0; j < 8; j++) {
            q = ~q;
            p1 = (ulv *) bufa;
            p2 = (ulv *) bufb;
            for (i = 0; i < count; i++) {
                *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
            }
            num_errors += compare_regions(bufa, bufb, count);
        }
    }
    return num_errors;
}
