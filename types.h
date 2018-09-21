/*
 * Very simple but very effective user-space memory tester.
 * Originally by Simon Kirby <sim@stormix.com> <sim@neato.org>
 * Version 2 by Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Version 3 not publicly released.
 * Version 4 rewrite:
 * Copyright (C) 2004-2010 Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 * This file contains typedefs, structure, and union definitions.
 *
 */

#ifndef MEMTESTER_TYPES_H
#define MEMTESTER_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include "sizes.h"

typedef unsigned int uint;
typedef unsigned long ul;
typedef unsigned long volatile ulv;
typedef uint8_t volatile u8v;
typedef uint16_t volatile u16v;

typedef size_t (*test_t)(ulv*, ulv*, size_t);
typedef ul (*binop_t)(ul, ul);

#endif // MEMTESTER_TYPES_H
