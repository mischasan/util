/*
** Copyright (C) 2009-2013 Mischa Sandberg <mischasan@gmail.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef PSEARCH_H
#define PSEARCH_H
#define STAT

// "psearch" uses MEMREF {ptr,len} bytevec structs for "string" args,
// rather than NUL-terminated "C" strings.

#ifndef MSUTIL_H
#include <stdio.h>
typedef struct { char const *ptr; size_t len; } MEMREF;
#endif

typedef struct psearch PSEARCH;

// For each match, psearch_scan calls its PSEARCH_ACTION fn,
// giving it the strv[] index of the matched string, and
// the text offset of the byte AFTER the end of the string.
// If PSEARCH_ACTION returns 0, search continues; otherwise,
// psearch_scan returns that nonzero value immediately.

typedef int (PSEARCH_ACTION)(int strnum, int textpos, void *context);

PSEARCH* psearch_create(MEMREF const *strv, int nstrs);
void psearch_destroy(PSEARCH*);

// If sequential blocks of (text) are passed to repeated psearch_scan calls,
//  then psearch_more() continues where the previous psearch_scan/psearch_more
//  left off -- string matches can cross block boundaries.
// *state should initially be (0).
int psearch_more(PSEARCH const*, MEMREF const text,
                 PSEARCH_ACTION *fn, void *fndata, int *state);

static inline int psearch_scan(PSEARCH const*psp, MEMREF const text,
                               PSEARCH_ACTION *fn, void *fndata)
{
    int state = 0;
    return psearch_more(psp, text, fn, fndata, &state);
}

void psearch_save(FILE*, PSEARCH const*);
PSEARCH* psearch_load(FILE*);
PSEARCH* psearch_mmap(FILE*);

// diagnostics
typedef enum {
    PS_STATS=1, PS_TRAN=2, PS_HASH=4, PS_TREE=8, PS_ALL=-1
} PS_DUMP_TYPE;

// If (pattv) is not NULL, dump output includes strings.
void psearch_dump(PSEARCH const*, PS_DUMP_TYPE, FILE*, MEMREF const*pattv);

#endif//PSEARCH_H
