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

#include "msutil.h"

static inline uint32_t load32(uint8_t const *p, int i);
static inline uint32_t rol32(uint32_t h, int n);
static inline uint32_t fix32(uint32_t h);
static inline void     save32(uint8_t *p, int i, uint32_t h);

static inline uint64_t load64(uint8_t const *p, int i);
static inline uint64_t rol64(uint64_t h, int n);
static inline uint64_t fix64(uint64_t k);
static inline void     save64(uint8_t *p, int i, uint64_t h);

//-----------------------------------------------------------------------------
void
mmhash3_x86_04(uint8_t const *inp, int len, uint8_t *out, uint32_t seed)
{
    uint32_t    c0 = 0x95543787, c1 = 0x2ad7eb25;
    uint32_t    h0 = 0x971e137b ^ seed;
#   define MIX32 do {                \
        k0 = rol32(k0 * c0, 11) * c1;       \
        h0 = (h0 ^ k0) * 3 + 0x52dce729;    \
        c0 = c0 * 5 + 0x7b7d159c;           \
        c1 = c1 * 5 + 0x6bce6396; } while(0)

    for (; len >= 4; len -= 4, inp += 4) {
        uint32_t k0 = load32(inp, 0);
        MIX32;
    }

    uint32_t    k0 = 0;
    switch (len) {
    case 3: k0 ^= inp[2] << 16;
    case 2: k0 ^= inp[1] << 8;
    case 1: k0 ^= inp[0];
    };
    if (len) MIX32;

    h0 = fix32(h0 ^ len);

    save32(out, 0, h0);
#   undef  MIX32
}

#define MAD(ha,hb) do {           \
            ha = ha * 3 + 0x52dce729;   \
            hb = hb * 3 + 0x38495ab5;   \
            c0 = c0 * 5 + 0x7b7d159c;   \
            c1 = c1 * 5 + 0x6bce6396; } while (0)
void
mmhash3_x86_08(uint8_t const *inp, int len, uint8_t *out, uint32_t seed)
{
    uint32_t    c0 = 0x95543787, c1 = 0x2ad7eb25;
    uint32_t    h0 = 0x8de1c3ac ^ seed;
    uint32_t    h1 = 0xbab98226 ^ seed;
#   define MIX32 do {          \
            k0 = rol32(k0 * c0, 11) * c1;   \
            h0 = (h0 ^ k0) + h1;            \
            k1 = rol32(k1 * c1, 11) * c0;   \
            h1 = (rol32(h1, 17) ^ k1) + h0; \
            MAD(h0, h1); } while (0)

    for (; len >= 8; len -= 8, inp += 8) {
        uint32_t k0 = load32(inp, 0);
        uint32_t k1 = load32(inp, 1);
        MIX32;
    }

    uint32_t    k0 = 0, k1 = 0;

    switch (len) {
    case 7: k1 ^= inp[6] << 16;
    case 6: k1 ^= inp[5] << 8;
    case 5: k1 ^= inp[4] << 0;
    case 4: k0 ^= load32(inp, 0);
            break;
    case 3: k0 ^= inp[2] << 16;
    case 2: k0 ^= inp[1] << 8;
    case 1: k0 ^= inp[0] << 0;
    };
    if (len) MIX32;

    h0 += h1 ^= len;
    h1 += h0;
    h0 = fix32(h0);
    h1 = fix32(h1);
    h1 += h0 += h1;

    save32(out, 0, h0);
    save32(out, 1, h1);
#   undef  MIX32
}

void
mmhash3_x86_16(uint8_t const *inp, int len, uint8_t *out, uint32_t seed)
{
    uint32_t    c0 = 0x95543787, c1 = 0x2ad7eb25;
    uint32_t    h0 = 0x8de1c3ac ^ seed;
    uint32_t    h1 = 0xbab98226 ^ seed;
    uint32_t    h2 = 0xfcba5b2d ^ seed;
    uint32_t    h3 = 0x32452e3e ^ seed;
#   define MIX32 do {   \
            k0 = rol32(k0 * c0, 11) * c1;               \
            h0 = rol32((h0 ^ k0) + h1 + h2 + h3, 17);   \
            k1 = rol32(k1 * c1, 11) * c0;                \
            h1 = (h1 ^ k1) + h0;                        \
            MAD(h0, h1);                                \
            k2 = rol32(k2 * c0, 11) * c1;               \
            h2 = (h2 ^ k2) + h0;                        \
            k3  = rol32(k3 * c1, 11) * c0;               \
            h3 = (h3 ^ k3) + h0;                        \
            MAD(h2, h3); } while (0)
    for (; len >= 16; len -=16, inp += 16) {
        uint32_t k0 = load32(inp, 0);
        uint32_t k1 = load32(inp, 1);
        uint32_t k2 = load32(inp, 2);
        uint32_t k3 = load32(inp, 3);
        MIX32;
    }

    uint32_t    k0 = 0, k1 = 0, k2 = 0, k3 = 0;

    switch (len) {
    case 15: k3 ^= inp[14] << 16;
    case 14: k3 ^= inp[13] << 8;
    case 13: k3 ^= inp[12] << 0;
    case 12: k2 ^= load32(inp, 2);
             k1 ^= load32(inp, 1);
             k0 ^= load32(inp, 0);
            break;
    case 11: k2 ^= inp[10] << 16;
    case 10: k2 ^= inp[ 9] << 8;
    case  9: k2 ^= inp[ 8] << 0;
    case  8: k1 ^= load32(inp, 1);
             k0 ^= load32(inp, 0);
            break;
    case  7: k1 ^= inp[ 6] << 16;
    case  6: k1 ^= inp[ 5] << 8;
    case  5: k1 ^= inp[ 4] << 0;
    case  4: k0 ^= load32(inp, 0);
            break;
    case  3: k0 ^= inp[ 2] << 16;
    case  2: k0 ^= inp[ 1] << 8;
    case  1: k0 ^= inp[ 0] << 0;
    };
    if (len) MIX32;

    h3 ^= len;
    h0 += h1 + h2 + h3;
    h1 += h0; h2 += h0; h3 += h0;

    h0 = fix32(h0);
    h1 = fix32(h1);
    h2 = fix32(h2);
    h3 = fix32(h3);

    h0 += h1 + h2 + h3;
    h1 += h0; h2 += h0; h3 += h0;

    save32(out, 0, h0);
    save32(out, 1, h1);
    save32(out, 2, h2);
    save32(out, 3, h3);
#   undef  MIX32
}

void
mmhash3_x64_16(uint8_t const *inp, int len, uint8_t *out, uint32_t seed)
{
    uint64_t    c0 = 0x87c37b91114253d5ULL, c1 = 0x4cf5ad432745937fULL;
    uint64_t    h0 = 0x9368e53c2f6af274ULL ^ seed;
    uint64_t    h1 = 0x586dcd208f7cd3fdULL ^ seed;
#   define MIX64 do {   \
            k0 = rol64(k0 * c0, 23) * c1;   \
            h0 = (h0 ^ k0) + h1;            \
            k1 = rol64(k1 * c1, 23) * c0;   \
            h1 = (rol64(h1, 41) ^ k1) + h0; \
            MAD(h0, h1); } while (0)

    for (; len >= 16; len -= 16, inp += 16) {
        uint64_t k0 = load64(inp, 0);
        uint64_t k1 = load64(inp, 1);
        MIX64;
    }

    uint64_t    k0 = 0, k1 = 0;
    switch (len) {
    case 15: k1 ^= (uint64_t)inp[14] << 48;
    case 14: k1 ^= (uint64_t)inp[13] << 40;
    case 13: k1 ^= (uint64_t)inp[12] << 32;
    case 12: k1 ^= load32(inp, 2);
             k0 ^= load64(inp, 0);
            break;
    case 11: k1 ^= (uint64_t)inp[10] << 16;
    case 10: k1 ^= (uint64_t)inp[ 9] << 8;
    case  9: k1 ^= (uint64_t)inp[ 8] << 0;
    case  8: k0 ^= load64(inp, 0);
            break;
    case  7: k0 ^= (uint64_t)inp[ 6] << 48;
    case  6: k0 ^= (uint64_t)inp[ 5] << 40;
    case  5: k0 ^= (uint64_t)inp[ 4] << 32;
    case  4: k0 ^= load32(inp, 0);
            break;
    case  3: k0 ^= (uint64_t)inp[ 2] << 16;
    case  2: k0 ^= (uint64_t)inp[ 1] << 8;
    case  1: k0 ^= (uint64_t)inp[ 0] << 0;
    };
    if (len) MIX64;

    h1 ^= len;
    h0 += h1;
    h1 += h0;
    h0 = fix64(h0);
    h1 = fix64(h1);
    h0 += h1;
    h1 += h0;
    save64(out, 0, h0);
    save64(out, 1, h1);
#   undef  MIX64
}

void
mmhash3_x64_04(uint8_t const *inp, int len, uint8_t *out, uint32_t seed)
{
    uint8_t     temp[16];
    mmhash3_x64_16(inp, len, temp, seed);
    memcpy(out, temp, 4);
}

void
mmhash3_x64_08(uint8_t const *inp, int len, uint8_t *out, uint32_t seed)
{
    uint8_t     temp[16];
    mmhash3_x64_16(inp, len, temp, seed);
    memcpy(out, temp, 8);
}

//-----------------------------------------------------------------------------
static inline uint32_t
load32(uint8_t const *p, int i)
{ return ((uint32_t const *)p)[i]; }

static inline uint64_t
load64(uint8_t const *p, int i)
{ return ((uint64_t const *)p)[i]; }

static inline void
save32(uint8_t *p, int i, uint32_t h)
{ ((uint32_t *)p)[i] = h; }

static inline void
save64(uint8_t *p, int i, uint64_t h)
{ ((uint64_t *)p)[i] = h; }

static inline uint32_t
rol32(uint32_t h, int n)
{
    asm("roll %%cl, %0" : "=g" (h) : "g" (h), "c" (n));
    return  h;
}

static inline uint64_t
rol64(uint64_t h, int n)
{
#if defined(__x86_64) || __WORDSIZE == 64
    asm("rolq %%cl, %0" : "=g" (h) : "g" (h), "c" (n));
    return h;
#else
    uint32_t    hi, lo;
    asm("   cmpb  $31,   %%cl;"
        "   jbe   1f;"
        "   xchgl %%eax, %%edx;"
        "   andl  $31,   %%ecx;"
        "1: movl  $-1,   %%ebx;"
        "   roll  %%cl,  %%eax;"
        "   sall  %%cl,  %%ebx;"
        "   roll  %%cl,  %%edx;"
        "   notl  %%ebx;"
        "   movl  %%eax, %%ecx;"
        "   xorl  %%edx, %%ecx;"
        "   andl  %%ecx, %%ebx;"
        "   xorl  %%ebx, %%eax;"
        "   xorl  %%ebx, %%edx;"
        : "=a" (lo), "=d" (hi)
        : "a" ((uint32_t) h),
          "d" ((uint32_t)(h >> 32)),
          "c" ((uint8_t) (n  & 63))
    );
    return  (uint64_t)(h << 32) | lo;
#endif
}

static inline uint32_t
fix32(uint32_t h)
{
    h = (h ^ (h >> 16)) * 0x85ebca6b;
    h = (h ^ (h >> 13)) * 0xc2b2ae35;
    return h ^ (h >> 16);
}

static inline uint64_t
fix64(uint64_t h)
{
    h = (h ^ (h >> 33)) * 0xff51afd7ed558ccdULL;
    h = (h ^ (h >> 33)) * 0xc4ceb9fe1a85ec53ULL;
    return h ^ (h >> 33);
}
