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
// Make all print functions no-ops in the final firmare build to minimize
// binary size.
int printf(char const *, ...) {}
int fprintf(FILE*, char const *, ...) {}
int fflush(FILE*) {}
int fsync(int) {}
#else // FIRMWARE_BUILD
#include <stdio.h>
#include <unistd.h>
#endif // FIRMWARE_BUILD

#endif // MEMTESTER_IO_H
