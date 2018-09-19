/*
 * Copyright (C) 2018 Common Networks
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 * This file acts as a partial stdio shim to elide print statements to minimize
 * binary output size where desired.
 */

#ifndef MEMTESTER_CONSTS_H
#define MEMTESTER_CONSTS_H

#include <stddef.h>
#include <sys/types.h>

#ifndef PHYS_ADDR_BASE
#define PHYS_ADDR_BASE 0
#endif
#define PHYS_ADDR_BASE_VAL (off_t)(PHYS_ADDR_BASE)

#ifndef PHYS_ADDR_SIZE
#define PHYS_ADDR_SIZE 2 << 30
#else
#endif
#define PHYS_ADDR_SIZE_VAL (size_t)(PHYS_ADDR_SIZE)

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif
#define PAGE_SIZE_VAL (size_t)4096

#endif // MEMTESTER_CONSTS_H
