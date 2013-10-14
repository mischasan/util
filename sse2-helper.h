/*
 * Copyright (c) 2004 Dean Gaudet <dean@arctic.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SSE2_HELPER_H_INCLUDED
#define SSE2_HELPER_H_INCLUDED

#ifdef __INTEL_COMPILER
#include <emmintrin.h>
#else
#include <xmmintrin.h>
#endif

// i can't for the life of me figure out the right way to initialize
// vectors... aside from using this union hack. -dean
typedef union {
	uint32_t u32[4];
	__m128i u128;
} v4si __attribute__((aligned(16)));

#define paddd(a, b) _mm_add_epi32(a, b)
#define pshufhw(a, n) _mm_shufflehi_epi16(a, n)
#define pshuflw(a, n) _mm_shufflelo_epi16(a, n)
#define pslld(a, n) _mm_slli_epi32(a, n)
#define pslldq(a, n) _mm_slli_si128(a, n)
#define psllw(a, n) _mm_slli_epi16(a, n)
#define psrld(a, n) _mm_srli_epi32(a, n)
#define psrldq(a, n) _mm_srli_si128(a, n)
#define psrlw(a, n) _mm_srli_epi16(a, n)
#define punpckhqdq(dst, src) _mm_unpackhi_epi64(dst, src)
#define punpcklqdq(dst, src) _mm_unpacklo_epi64(dst, src)
#define pxor(a, b) _mm_xor_si128(a, b)

#define pzero() _mm_setzero_si128()

#define prord(a, n) pxor(psrld((a), (n)), pslld((a), 32 - (n)))
#define prorw(a, n) pxor(psrlw((a), (n)), psllw((a), 16 - (n)))
#define restrict
#endif
