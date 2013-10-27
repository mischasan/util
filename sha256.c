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

/*
 * this code was inspired by this paper:
 *
 *     SHA: A Design for Parallel Architectures?
 *     Antoon Bosselaers, Ren´e Govaerts and Joos Vandewalle
 *     <http://www.esat.kuleuven.ac.be/~cosicart/pdf/AB-9700.pdf>
 *
 * more information available on this implementation here:
 *
 * 	http://arctic.org/~dean/crypto/sha1.html
 *
 * version: 1
 */

#include <stdint.h>
#include "sse2-helper.h"
#include "sha256.h"

#undef DEBUG
#ifdef DEBUG
#include <stdio.h>
#define debug(x) printf x
#else
#define debug(x)
#endif

static const v4si K[16] = {
	{ .u32 = { 0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U, } },
	{ .u32 = { 0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U, } },
	{ .u32 = { 0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U, } },
	{ .u32 = { 0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U, } },
	{ .u32 = { 0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU, } },
	{ .u32 = { 0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU, } },
	{ .u32 = { 0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U, } },
	{ .u32 = { 0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U, } },
	{ .u32 = { 0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U, } },
	{ .u32 = { 0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U, } },
	{ .u32 = { 0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U, } },
	{ .u32 = { 0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U, } },
	{ .u32 = { 0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U, } },
	{ .u32 = { 0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U, } },
	{ .u32 = { 0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U, } },
	{ .u32 = { 0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U, } },
};

#define UNALIGNED 1
#if UNALIGNED
#define load(p)	_mm_loadu_si128(p)
#else
#define load(p) (*p)
#endif

#define BYTESWAP 1

// prepare W[0] through W[15] -- which require a byte swap.
// we also add in K[0] through K[15].
#define prep00_15(prep, W, t)	do { 					\
		if (BYTESWAP) {						\
			__m128i r1 = W;					\
			r1 = pshufhw(r1, _MM_SHUFFLE(2, 3, 0, 1));	\
			r1 = pshuflw(r1, _MM_SHUFFLE(2, 3, 0, 1));	\
			W = prorw(r1, 8);				\
		}							\
		(prep).u128 = paddd(W, K[t>>2].u128);			\
	} while(0)


// prepare W[16]..W[63], and the K[16]..K[63] additions.
//
// consider here the W[t] calculations for t=16..19:
//
// W[16] = (ROR(W[14],17)^ROR(W[14],19)^SHR(W[14],10)) + W[ 9] + (ROR(W[ 1],7)^ROR(W[ 1],18)^SHR(W[ 1],3)) + W[ 0]
// W[17] = (ROR(W[15],17)^ROR(W[15],19)^SHR(W[15],10)) + W[10] + (ROR(W[ 2],7)^ROR(W[ 2],18)^SHR(W[ 2],3)) + W[ 1]
// W[18] = (ROR(W[16],17)^ROR(W[16],19)^SHR(W[16],10)) + W[11] + (ROR(W[ 3],7)^ROR(W[ 3],18)^SHR(W[ 3],3)) + W[ 2]
// W[19] = (ROR(W[17],17)^ROR(W[17],19)^SHR(W[17],10)) + W[12] + (ROR(W[ 4],7)^ROR(W[ 4],18)^SHR(W[ 4],3)) + W[ 3]
//
// we can calculate all of that in parallel in 128-bit xmm registers
// except for the W[16]/W[17] dependencies in W[18]/W[19].  so we
// substitute 0 for the W[16]/W[17] values:
//
//       / (ROR(W[14],17)^ROR(W[14],19)^SHR(W[14],10)) + W[ 9] + (ROR(W[ 1],7)^ROR(W[ 1],18)^SHR(W[ 1],3)) + W[ 0]
//  r0 = + (ROR(W[15],17)^ROR(W[15],19)^SHR(W[15],10)) + W[10] + (ROR(W[ 2],7)^ROR(W[ 2],18)^SHR(W[ 2],3)) + W[ 1]
//       + (ROR(    0,17)^ROR(    0,19)^SHR(    0,10)) + W[11] + (ROR(W[ 3],7)^ROR(W[ 3],18)^SHR(W[ 3],3)) + W[ 2]
//       \ (ROR(    0,17)^ROR(    0,19)^SHR(    0,10)) + W[12] + (ROR(W[ 4],7)^ROR(W[ 4],18)^SHR(W[ 4],3)) + W[ 3]
//
// note that r0 has the finished W[16]/W[17] computation, and we use
// that to calculate the final W[18] and W[19] values.
//
// finally we throw in the K[16]..K[19] additions.
//
// the parameters to this are:
//
// W0 = W[t]..W[t+3]
// W1 = W[t+4]..W[t+7]
// W2 = W[t+8]..W[t+11]
// W3 = W[t+12]..W[t+15]
// p = mem location to store the W[t]+K[t]..W[t+3]+K[t+3] computation
//
// W0 is modified and contains the new W[t+16]..W[t+19] results.
//
#define prep(p, W0, W1, W2, W3, t) do { 				\
	__m128i r0, r1, r2;						\
									\
	r0 = pxor(psrldq(W0, 4), pslldq(W1, 12));			\
	r0 = pxor(pxor(prord(r0, 7), prord(r0, 18)), psrld(r0, 3));	\
									\
	r1 = punpckhqdq(W3, pzero());					\
	r1 = pxor(pxor(prord(r1, 17), prord(r1, 19)), psrld(r1, 10));	\
									\
	r2 = pxor(psrldq(W2, 4), pslldq(W3, 12));			\
									\
	r0 = paddd(paddd(r0, W0), paddd(r2, r1));			\
									\
	r1 = punpcklqdq(pzero(), r0);					\
	r1 = pxor(pxor(prord(r1, 17), prord(r1, 19)), psrld(r1, 10));	\
									\
	W0 = paddd(r0, r1);						\
	p.u128 = paddd(W0, K[t>>2].u128);				\
} while(0)


static inline uint32_t ROR(uint32_t x, unsigned n)
{
	return ((x >> n) | (x << (32 - n)));
}


static inline uint32_t Ch(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) | (~x & z);
}


static inline uint32_t Maj(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) ^ (x & z) ^ (y & z);
}


static inline uint32_t sigma0(uint32_t x)
{
	// the standard says: ROR(x, 2) ^ ROR(x, 13) ^ ROR(x, 22)
	// but if we rearrange it slightly we can improve performance
	// on p4-3, k8, p-m, efficeon... we lose a bit on p4-2.
	return ROR(x ^ ROR(x, 11) ^ ROR(x, 20), 2);
}


static inline uint32_t sigma1(uint32_t x)
{
	// the standard says: ROR(x, 6) ^ ROR(x, 11) ^ ROR(x, 25)
	// but similar to sigma0 we prefer this form:
	return ROR(x ^ ROR(x, 5) ^ ROR(x, 19), 6);
}


#if 1
#define step(xa, xb, xc, xd, xe, xf, xg, xh, xz, input) do {	\
	xz = sigma1(xe) + Ch(xe, xf, xg) + (xh + input); 	\
	xd += xz; 						\
	xz += sigma0(xa) + Maj(xa, xb, xc); 			\
	debug(("%08x %08x %08x %08x %08x %08x %08x %08x\n",	\
		xz, xa, xb, xc, xd, xe, xf, xg));		\
} while(0)
#else
// this was a failed attempt to produce reasonable performance from gcc
// by including a hand-scheduled step function.
#define step(xa, xb, xc, xd, xe, xf, xg, xh, xz, xinput) do {	\
	uint32_t t1, t2, t3, t4, t5; \
	__asm volatile( \
		"\n	mov    %[t1],%[t4]"	/* e */ \
		"\n	rol    $0x1b,%[t4]" \
		"\n	xor    %[t1],%[t4]" \
		"\n	mov    %[t1],%[z]" \
		"\n	rol    $0xd,%[z]" \
		"\n	xor    %[z],%[t4]" \
		"\n	mov    %[t1],%[z]"	/* e */ \
		"\n	and    %[t2],%[z]"	/* e & f */ \
		"\n	not    %[t1]"		/* ~e */ \
		"\n	and    %[t3],%[t1]"	/* ~e & g */ \
		"\n	or     %[t1],%[z]"	/* Ch(e, f, g) */ \
		"\n	add    %[input],%[z]"	/* Ch(e, f, g) + input */ \
		"\n	add    %[in_h],%[z]"	/* Ch + input + h */ \
		"\n	mov    %[in_a],%[t5]" \
		"\n	mov    %[in_b],%[t1]" \
		"\n	rol    $0x1a,%[t4]"	/* sigma1(e) */ \
		"\n	add    %[t4],%[z]"	/* z = sigma1 + Ch + input + h */ \
		"\n	add    %[z],%[out_c]"	/* d */ \
		"\n	mov    %[t5],%[t2]" \
		"\n	mov    %[out_c],%[out_d]" \
		"\n	mov    %[in_c],%[out_c]" \
		"\n	rol    $0x15,%[t2]" \
		"\n	xor    %[t5],%[t2]" \
		"\n	mov    %[t5],%[t3]" \
		"\n	rol    $0xc,%[t3]" \
		"\n	xor    %[t3],%[t2]" \
		"\n	mov    %[t1],%[t4]" \
		"\n	xor    %[out_c],%[t4]"	/* c ^ b */ \
		"\n	and    %[t5],%[t4]"	/* a & (c ^ b) */ \
		"\n	mov    %[t1],%[t3]" \
		"\n	and    %[out_c],%[t3]"	/* b & c */ \
		"\n	xor    %[t3],%[t4]"	/* Maj = (b & c) ^ (a & (c ^ b)) */ \
		"\n	add    %[t4],%[z]"	/* z += Maj */ \
		"\n	rol    $0x1e,%[t2]"	/* sigma0(a) */ \
		"\n	add    %[t2],%[z]"	/* z += sigma0 */ \
 \
		: [z] "=&r" (xz), [out_d] "=m" (xd), [out_c] "=&r" (xc), [t1] "=&r" (t1), \
		  [t2] "=&r" (t2), [t3] "=&r" (t3), [t4] "=&r" (t4), [t5] "=&r" (t5) \
		: [in_a] "m" (xa), [in_b] "m" (xb), [in_c] "m" (xc), "[out_c]" (xd), \
		  "[t1]" (xe), "[t2]" (xf), "[t3]" (xg), [in_h] "m" (xh), \
		  [input] "m" (xinput) \
	); \
} while(0)
#endif
	  


void sha256_step(uint32_t * restrict H, const uint32_t * restrict inputu, size_t num_steps)
{
	const __m128i * restrict input = (const __m128i *)inputu;
	__m128i W0, W1, W2, W3;
	v4si prep0, prep1, prep2;
	uint32_t a, b, c, d, e, f, g, h, z;

	for (;;) {
		a = H[0];
		b = H[1];
		c = H[2];
		d = H[3];
		e = H[4];
		f = H[5];
		g = H[6];
		h = H[7];

		W0 = load(&input[0]);
		prep00_15(prep0, W0, 0);			// prepare for  0 through 03
		W1 = load(&input[1]);
		prep00_15(prep1, W1, 4);			// prepare for  4 through 07
		W2 = load(&input[2]);
		prep00_15(prep2, W2, 8);			// prepare for  8 through 11

		W3 = load(&input[3]);
		step(a, b, c, d, e, f, g, h, z, prep0.u32[0]);	//  0
		step(z, a, b, c, d, e, f, g, h, prep0.u32[1]);	//  1
		step(h, z, a, b, c, d, e, f, g, prep0.u32[2]);	//  2
		step(g, h, z, a, b, c, d, e, f, prep0.u32[3]);	//  3
		prep00_15(prep0, W3, 12);		// prepare for 12 through 15
		step(f, g, h, z, a, b, c, d, e, prep1.u32[0]);	//  4
		step(e, f, g, h, z, a, b, c, d, prep1.u32[1]);	//  5
		step(d, e, f, g, h, z, a, b, c, prep1.u32[2]);	//  6
		step(c, d, e, f, g, h, z, a, b, prep1.u32[3]);	//  7
		prep(prep1, W0, W1, W2, W3, 16);	// prepare for 16 through 19
		step(b, c, d, e, f, g, h, z, a, prep2.u32[0]);	//  8
		step(a, b, c, d, e, f, g, h, z, prep2.u32[1]);	//  9
		step(z, a, b, c, d, e, f, g, h, prep2.u32[2]);	// 10
		step(h, z, a, b, c, d, e, f, g, prep2.u32[3]);	// 11
		prep(prep2, W1, W2, W3, W0, 20);
		step(g, h, z, a, b, c, d, e, f, prep0.u32[0]);	// 12
		step(f, g, h, z, a, b, c, d, e, prep0.u32[1]);	// 13
		step(e, f, g, h, z, a, b, c, d, prep0.u32[2]);	// 14
		step(d, e, f, g, h, z, a, b, c, prep0.u32[3]);	// 15
		prep(prep0, W2, W3, W0, W1, 24);
		step(c, d, e, f, g, h, z, a, b, prep1.u32[0]);	// 16
		step(b, c, d, e, f, g, h, z, a, prep1.u32[1]);	// 17
		step(a, b, c, d, e, f, g, h, z, prep1.u32[2]);	// 18
		step(z, a, b, c, d, e, f, g, h, prep1.u32[3]);	// 19
		prep(prep1, W3, W0, W1, W2, 28);
		step(h, z, a, b, c, d, e, f, g, prep2.u32[0]);	// 20
		step(g, h, z, a, b, c, d, e, f, prep2.u32[1]);	// 21
		step(f, g, h, z, a, b, c, d, e, prep2.u32[2]);	// 22
		step(e, f, g, h, z, a, b, c, d, prep2.u32[3]);	// 23
		prep(prep2, W0, W1, W2, W3, 32);
		step(d, e, f, g, h, z, a, b, c, prep0.u32[0]);	// 24
		step(c, d, e, f, g, h, z, a, b, prep0.u32[1]);	// 25
		step(b, c, d, e, f, g, h, z, a, prep0.u32[2]);	// 26
		step(a, b, c, d, e, f, g, h, z, prep0.u32[3]);	// 27
		prep(prep0, W1, W2, W3, W0, 36);
		step(z, a, b, c, d, e, f, g, h, prep1.u32[0]);	// 28
		step(h, z, a, b, c, d, e, f, g, prep1.u32[1]);	// 29
		step(g, h, z, a, b, c, d, e, f, prep1.u32[2]);	// 30
		step(f, g, h, z, a, b, c, d, e, prep1.u32[3]);	// 31
		prep(prep1, W2, W3, W0, W1, 40);
		step(e, f, g, h, z, a, b, c, d, prep2.u32[0]);	// 32
		step(d, e, f, g, h, z, a, b, c, prep2.u32[1]);	// 33
		step(c, d, e, f, g, h, z, a, b, prep2.u32[2]);	// 34
		step(b, c, d, e, f, g, h, z, a, prep2.u32[3]);	// 35
		prep(prep2, W3, W0, W1, W2, 44);
		step(a, b, c, d, e, f, g, h, z, prep0.u32[0]);	// 36
		step(z, a, b, c, d, e, f, g, h, prep0.u32[1]);	// 37
		step(h, z, a, b, c, d, e, f, g, prep0.u32[2]);	// 38
		step(g, h, z, a, b, c, d, e, f, prep0.u32[3]);	// 39
		prep(prep0, W0, W1, W2, W3, 48);
		step(f, g, h, z, a, b, c, d, e, prep1.u32[0]);	// 40
		step(e, f, g, h, z, a, b, c, d, prep1.u32[1]);	// 41
		step(d, e, f, g, h, z, a, b, c, prep1.u32[2]);	// 42
		step(c, d, e, f, g, h, z, a, b, prep1.u32[3]);	// 43
		prep(prep1, W1, W2, W3, W0, 52);
		step(b, c, d, e, f, g, h, z, a, prep2.u32[0]);	// 44
		step(a, b, c, d, e, f, g, h, z, prep2.u32[1]);	// 45
		step(z, a, b, c, d, e, f, g, h, prep2.u32[2]);	// 46
		step(h, z, a, b, c, d, e, f, g, prep2.u32[3]);	// 47
		prep(prep2, W2, W3, W0, W1, 56);
		step(g, h, z, a, b, c, d, e, f, prep0.u32[0]);	// 48
		step(f, g, h, z, a, b, c, d, e, prep0.u32[1]);	// 49
		step(e, f, g, h, z, a, b, c, d, prep0.u32[2]);	// 50
		step(d, e, f, g, h, z, a, b, c, prep0.u32[3]);	// 51
		prep(prep0, W3, W0, W1, W2, 60);
		step(c, d, e, f, g, h, z, a, b, prep1.u32[0]);	// 52
		step(b, c, d, e, f, g, h, z, a, prep1.u32[1]);	// 53
		step(a, b, c, d, e, f, g, h, z, prep1.u32[2]);	// 54
		step(z, a, b, c, d, e, f, g, h, prep1.u32[3]);	// 55

		step(h, z, a, b, c, d, e, f, g, prep2.u32[0]);	// 56
		step(g, h, z, a, b, c, d, e, f, prep2.u32[1]);	// 57
		step(f, g, h, z, a, b, c, d, e, prep2.u32[2]);	// 58
		step(e, f, g, h, z, a, b, c, d, prep2.u32[3]);	// 59
		
		step(d, e, f, g, h, z, a, b, c, prep0.u32[0]);	// 60
		step(c, d, e, f, g, h, z, a, b, prep0.u32[1]);	// 61
		step(b, c, d, e, f, g, h, z, a, prep0.u32[2]);	// 62
		step(a, b, c, d, e, f, g, h, z, prep0.u32[3]);	// 63

		  // z, a, b, c, d, e, f, g, h
		H[0] += z;
		H[1] += a;
		H[2] += b;
		H[3] += c;
		H[4] += d;
		H[5] += e;
		H[6] += f;
		H[7] += g;

		--num_steps;
		if (num_steps == 0) break;
		input += 4;
	}
}


void sha256_init(uint32_t * restrict H)
{
	H[0] = 0x6a09e667U;
	H[1] = 0xbb67ae85U;
	H[2] = 0x3c6ef372U;
	H[3] = 0xa54ff53aU;
	H[4] = 0x510e527fU;
	H[5] = 0x9b05688cU;
	H[6] = 0x1f83d9abU;
	H[7] = 0x5be0cd19U;
}
