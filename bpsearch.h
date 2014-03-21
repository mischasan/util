// Copyright (C) 2014 Mischa Sandberg <mischasan@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License Version 2 as
// published by the Free Software Foundation.  You may not use, modify or
// distribute this program under any other version of the GNU General
// Public License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// IF YOU ARE UNABLE TO WORK WITH GPL2, CONTACT ME.
//-------------------------------------------------------------------
#ifndef BPSEARCH_H
#define BPSEARCH_H

#include <stdint.h>

typedef uint8_t byte;
typedef uint32_t uint;

typedef struct BPSEARCH BPSEARCH;
typedef struct { byte const *ptr; uint len; } MEM;

// BPSEARCH performs parallel string search on strings of "ACGT" (or "acgt").
//  If any pattern string matches the target text, bpsearch_scan sets 
//  *matchnum to the pattv[] index, sets *matchpos to that starting offset in text,
//  and returns 1. Otherwise, bpsearch_scan returns 0.

BPSEARCH *bpsearch_create(MEM const*pattv, uint npatts);
int       bpsearch_scan(BPSEARCH *bp, MEM text, uint *matchnum, uint *matchpos);
void      bpsearch_destroy(BPSEARCH *bp);

// acgtpack translates a vector of "ACGT" chars into a vector of bitpairs 00 01 10 11,
//  packed 4 per byte, least significant pair first. The last byte is padded with 00's.
// acgtpack may read up to inp[len+3].
//  which conceivably could read beyond valid mem.
// acgtpack benefits if (inp) is __attribute__((aligned(16))).

void acgtpack(byte const *inp, uint len, byte *out);

#endif//BPSEARCH_H
