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

// http://cessu.blogspot.com/2008/11/hashing-with-sse2-revisited-or-my-hash.html
//      superseding:
// http://cessu.blogspot.com/2007/09/hashing-with-sse2.html
// Padding inputs whose length is not 16N is not perfect, but better than
//  just padding with zeroes. A 16N+15-byte value has the same hash as 
//  a 16N+16-byte value whose last byte is (15).

#include "msutil.h"
#include "xmutil.h"

// Note XORSHR uses a BIT shift within 64bit words;
//      ADDSHL uses a BYTE shift across 16 bytes.
#define XORSHR(S,N) S = _mm_xor_si128(S, _mm_srli_epi64(S, N))
#define ADDSHL(S,N) S = _mm_add_epi64(S, _mm_slli_si128(S, N/8))
#define SUBSHF(S,T,A,B,C,D) \
            S = _mm_sub_epi64(_mm_shuffle_epi32(S, _MM_SHUFFLE(A,B,C,D)), T)

#define MIX do {\
    /* Step 1: four 32x32->64 bit multiplications of (C1,C2)*inp.
        This propagates a bit change in input to 32 positions,
        mixed into (state) by subtraction. */\
    s1 = _mm_sub_epi64(s1, _mm_mul_epu32(C1, _mm_unpackhi_epi32(input, input))); \
    s2 = _mm_sub_epi64(s2, _mm_mul_epu32(C2, _mm_unpacklo_epi32(input, input))); \
    /* Step 2: propagate the 32-bit changes to 64-bit changes */\
    XORSHR(s1, 29); XORSHR(s2, 29); ADDSHL(s1, 16); ADDSHL(s2, 16); \
    XORSHR(s1, 21); XORSHR(s2, 21); ADDSHL(s1, 32); ADDSHL(s2, 32); \
    /* Step 3: propagate changes among the four 64-bit words
        by 64-bit subtractions and 32-bit shuffling. */\
    s1 = _mm_sub_epi64(s1, s2); \
    SUBSHF(s2, s1, 0,3,2,1); SUBSHF(s1, s2, 0,1,3,2); \
    SUBSHF(s2, s1, 2,1,0,3); SUBSHF(s1, s2, 2,1,0,3); } while (0)

    // At this point, any one-bit flip in the input has now
    // changed all bits in state with a probability between
    // 45% to 55%.

void cessu32(uint8_t const *inp, int len, uint8_t out[32])
{
    XMM const   C1 = (__v2di) { 0x0000000098B365A1LL, 0x0000000052C69CABLL },
                    C2 = (__v2di) { 0x00000000B76A9A41LL, 0x00000000CC4D2C7BLL },
                    AA = (__v2di) { 0xC7265595564A4447LL, 0x128FA608E20C241DLL };
    XMM         input, s1 = AA, s2 = AA;
    uint8_t         buf[16] = {};

    for (; len > 0; len -= 16, inp += 16) {
        input = xm_loud(len < 16 ? memcpy(buf, inp, buf[len] = len)
                                         : (void const*)inp);
        MIX;
    }

    input = s1; s1 = AA; MIX;
    _mm_storeu_si128((XMM*)out, s1);
    _mm_storeu_si128((XMM*)&out[16], s2);
}
