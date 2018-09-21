/*
 * Very simple yet very effective memory tester.
 * Originally by Simon Kirby <sim@stormix.com> <sim@neato.org>
 * Version 2 by Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Version 3 not publicly released.
 * Version 4 rewrite:
 * Copyright (C) 2004-2012 Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 * This file contains the declarations for the functions for the actual tests,
 * called from the main routine in memtester.c.  See other comments in that
 * file.
 *
 */

#ifndef MEMTESTER_TESTS_H
#define MEMTESTER_TESTS_H

#include <stddef.h>

#include "types.h"

size_t test_stuck_address(ulv *bufa, size_t count);
size_t test_solidbits_cmp(ulv *bufa, ulv *bufb, size_t count);
size_t test_checkerboard_cmp(ulv *bufa, ulv *bufb, size_t count);
size_t test_blockseq_cmp(ulv *bufa, ulv *bufb, size_t count);
size_t test_walkbits0_cmp(ulv *bufa, ulv *bufb, size_t count);
size_t test_walkbits1_cmp(ulv *bufa, ulv *bufb, size_t count);
size_t test_bitspread_cmp(ulv *bufa, ulv *bufb, size_t count);
size_t test_bitflip_cmp(ulv *bufa, ulv *bufb, size_t count);

#endif // MEMTESTER_TESTS_H
