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

#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>

#include "sha256.h"

#define N (2048)

uint32_t buf[N] __attribute__((aligned(16)));
uint32_t H[SHA256_HASH_SIZE];

#define N_SAMPLE (10)

static void one(unsigned n)
{
	unsigned i, j;
	struct timeval tv_start, tv_end;
	uint64_t delta, best;
	unsigned n_iter;

	n_iter =  1000*(8192/n);
	best = ~0ULL;
	for (j = 0; j < N_SAMPLE; ++j) {
		gettimeofday(&tv_start, 0);
		for (i = 0; i < n_iter; ++i) {
			sha256_step(H, buf, n/SHA256_STEP_SIZE);
		}
		gettimeofday(&tv_end, 0);

		__asm volatile("emms");

		delta = (tv_end.tv_sec - tv_start.tv_sec)*1000000ULL
			+ (tv_end.tv_usec - tv_start.tv_usec);
		if (delta < best) {
			best = delta;
		}
	}
	// print a number similar to what openssl reports
	printf("%.2f KB/s (for %u byte buffer)\n",
		(double)(n * sizeof(uint32_t) * n_iter) * 1000.0 / best,
		n*sizeof(uint32_t));
}


int main(void)
{
	memset(buf, 0, sizeof(buf));
	one(16);
	one(64);
	one(256);
	one(1024);
	one(2048);

	return 0;
}
