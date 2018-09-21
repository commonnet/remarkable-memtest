/*
 * Copyright (C) 2018 Common Networks
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 * This file acts as a partial stdio shim to elide print statements to minimize
 * binary output size where desired.
 */

#ifndef MEMTESTER_IO_H
#define MEMTESTER_IO_H

#ifdef FIRMWARE_BUILD
// Make all debug print functions no-ops in the final firmare build to minimize
// binary size.
#define DEBUG_FPRINTF(...) (void)
#define DEBUG_FFLUSH(f) (void)
#define DEBUG_FSYNC(fd) (void)
#else // FIRMWARE_BUILD
#include <stdio.h>
#include <unistd.h>
#define DEBUG_FPRINTF(fd, ...) fprintf(fd, __VA_ARGS__)
#define DEBUG_FFLUSH(f) fflush(f)
#define DEBUG_FSYNC(fd) fsync(fd)
#endif // FIRMWARE_BUILD

#endif // MEMTESTER_IO_H
