// Copyright (C) 2009-2013 Mischa Sandberg <mischasan@gmail.com>
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

/* fnv: Fowler-Noll-Vo hashes of various widths.
 *  Note the one hole in FNV: very short keys (e.g. 3 uint8_ts)
 *  leave constant holes in the upper part of the hash.
 *  For fnv16, the bottom 6 uint8_ts are well-hashed,
 *  the next 6 uint8_ts are constant.
 */
#include <emmintrin.h>	// XMM
#include "msutil.h"
#include "xmutil.h"

uint32_t
fnv04(char const *buf, int len)
{
    // B=2166136261 M=16777619
    uint32_t	hash = 0x811C9DC5;

    for (; --len >= 0; ++buf)
	hash = (hash ^ *(uint8_t const*)buf) * 0x01000193;

    hash += hash << 13;
    hash ^= hash >> 7;
    hash += hash << 3;
    hash ^= hash >> 17;
    hash += hash << 5;

    return  hash;
}

uint64_t
fnv08(char const *buf, int len)
{
    uint64_t hash = 0xCBF29CE484222325ULL;

    for (; --len >= 0; ++buf)
	hash = (hash ^ *(uint8_t const*)buf) * 0x00000100000001B3ULL;

    hash ^= hash >> 33;
    hash *= 0xFF51AFD7ED558CCDULL;
    hash ^= hash >> 33;
    hash *= 0xC4CEB9FE1A85EC53ULL;
    hash ^= hash >> 33;

    return hash;
}

void
fnv16(char const *buf, int len, char out[16])
{
    XMM	hash = (__v2di) { 0x07BB01426C62272EULL, 0x6295C58D62B82175ULL };

    while (--len >= 0) {
        unsigned x = *buf++;
	//hash = _mm_xor_si128(hash, _mm_set_epi32(0, 0, 0, 255 & *buf++));
	hash = _mm_xor_si128(hash, _mm_set_epi32(x * 2, x * 3, x * 7, x * 11));
	/* This emulates 128bit mul of the FNV128 coefficient using shift/add/sub:
	 *	h := (h << 88) + (h << 8) + (h << 6) - (h << 2) - h
	 * "{add,sub}_epi64" each lose a carry bit (from [63] to [64]).
	 */
	hash =	_mm_sub_epi64(
		    _mm_sub_epi64(
			_mm_add_epi64(
			    _mm_add_epi64(xm_shl_013(hash),
                                          xm_shl_010(hash)),
			    xm_shl_006(hash)),
			xm_shl_002(hash)),
		    hash);
    }

    //XXX: add final mix-in!

    _mm_storeu_si128((XMM*)out, hash);
}
