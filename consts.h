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

#ifndef PHYS_ADDR_BASE
#define PHYS_ADDR_BASE 0
#endif
#define PHYS_ADDR_BASE_VAL (off_t)(PHYS_ADDR_BASE)

#ifndef PHYS_ADDR_SIZE
#define PHYS_ADDR_SIZE (size_t)2 << 30
#endif
#define PHYS_ADDR_SIZE_VAL (size_t)(PHYS_ADDR_SIZE)

#endif // MEMTESTER_CONSTS_H
