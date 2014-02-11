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

#include "msutil.h" // ffs,fls
#include "xmutil.h"

#ifdef __POPCNT__
#   include <popcntintrin.h>
#endif

int xm_popcount(XMM x)
{
#ifdef __POPCNT__
    return _mm_popcnt_u64(((uint64_t const*)&x)[0])
         + _mm_popcnt_u64(((uint64_t const*)&x)[0]);
#else
    static XMM  k1 = { 0x5555555555555555ULL, 0x5555555555555555ULL },
                k2 = { 0x3333333333333333ULL, 0x3333333333333333ULL },
                k4 = { 0x0F0F0F0F0F0F0F0FULL, 0x0F0F0F0F0F0F0F0FULL },
                kf = { 0x0101010101010101ULL, 0x0101010101010101ULL };
    x = x - ((x >> 1) & k1)
    x = (x & k2) + ((x >> 2) & k2)
    x = (x       + ((x >> 4) & k4)
    x = (x * kf) >> 56
    // Now add bytes 0 and 8 from
    //XXX http://chessprogramming.wikispaces.com/Population+Count ...
#endif
    return 0;
}

#undef  DO
#define DO_8x8(op,a) DO_8(op,a,0) DO_8(op,a,1) DO_8(op,a,2) DO_8(op,a,3) DO_8(op,a,4) DO_8(op,a,5) DO_8(op,a,6) DO_8(op,a,7)
#define DO_8(op,a,b) DO(op,a,b,0) DO(op,a,b,1) DO(op,a,b,2) DO(op,a,b,3) DO(op,a,b,4) DO(op,a,b,5) DO(op,a,b,6) DO(op,a,b,7)
#define DO(op,a,b,c) case 0##a##b##c: x = xm_##op##_##a##b##c(x); break;

#define xm_shl_000(x) (x)
XMM xm_shl(XMM x, unsigned nbits)
{
    switch (nbits) { DO_8x8(shl,0) DO_8x8(shl,1) default: x = xm_zero; }
    return x;
}

#define xm_shr_000(x) (x)
XMM
xm_shr(XMM x, unsigned nbits)
{
    switch (nbits) { DO_8x8(shr,0) DO_8x8(shr,1) default: x = xm_zero; }
    return x;
}

int
xm_ffs(XMM x)
{
    int pos = ffs(xm_diff(x, xm_zero)) - 1;
    return  pos < 0 ? pos : (pos << 3) + ffs(((unsigned char const*)&x)[pos]) - 1;
}

int
xm_fls(XMM x)
{
    int pos = fls(xm_diff(x, xm_zero)) - 1;
    return  pos < 0 ? pos : (pos << 3) + fls(((unsigned char const*)&x)[pos]) - 1;
}

char*
xm_dbl(__m128d x, char buf[48])
{
    union { __m128d xm; double b[2]; } a = { x };
    sprintf(buf, "%g,%g", a.b[0], a.b[1]);
    return buf;
}

char*
xm_llx(XMM x, char buf[48])
{
    union { XMM xm; unsigned long long b[2]; } a = { x };
    sprintf(buf, "0x%016llX,0x%016llX", a.b[0], a.b[1]);
    return buf;
}

char*
xm_str(XMM x, char buf[48])
{
    int i;
    union { XMM xm; unsigned char b[16]; } a = { x };
    char *cp = buf + sprintf(buf, "%02X", a.b[0]);
    for (i = 1; i < 16; ++i) cp += sprintf(cp, "%c%02X", i == 8 ? '-' : ',', a.b[i]);
    return buf;
}
