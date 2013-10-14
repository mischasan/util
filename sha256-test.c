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
#include <string.h>
#include "sha256.h"

#define SWAP(x)	(((x) >> 24) | (((x)&0x00FF0000) >> 8) | (((x)&0x0000FF00) << 8) | (x << 24))

const uint32_t message0[SHA256_STEP_SIZE] = {
	SWAP(0x61626380), SWAP(0x00000000), SWAP(0x00000000), SWAP(0x00000000),
	SWAP(0x00000000), SWAP(0x00000000), SWAP(0x00000000), SWAP(0x00000000),
	SWAP(0x00000000), SWAP(0x00000000), SWAP(0x00000000), SWAP(0x00000000),
	SWAP(0x00000000), SWAP(0x00000000), SWAP(0x00000000), SWAP(0x00000018)
};

const uint32_t H_expected0[SHA256_HASH_SIZE] = {
	0xba7816bfU, 0x8f01cfeaU, 0x414140deU, 0x5dae2223U,
	0xb00361a3U, 0x96177a9cU, 0xb410ff61U, 0xf20015adU,
};


const uint32_t message1[2*SHA256_STEP_SIZE] = {
	SWAP(0x61626364), SWAP(0x62636465), SWAP(0x63646566), SWAP(0x64656667),
	SWAP(0x65666768), SWAP(0x66676869), SWAP(0x6768696a), SWAP(0x68696a6b),
	SWAP(0x696a6b6c), SWAP(0x6a6b6c6d), SWAP(0x6b6c6d6e), SWAP(0x6c6d6e6f),
	SWAP(0x6d6e6f70), SWAP(0x6e6f7071), SWAP(0x80000000), SWAP(0x00000000),

	SWAP(0x00000000), SWAP(0x00000000), SWAP(0x00000000), SWAP(0x00000000),
	SWAP(0x00000000), SWAP(0x00000000), SWAP(0x00000000), SWAP(0x00000000),
	SWAP(0x00000000), SWAP(0x00000000), SWAP(0x00000000), SWAP(0x00000000),
	SWAP(0x00000000), SWAP(0x00000000), SWAP(0x00000000), SWAP(0x000001c0)
};

const uint32_t H_expected1[SHA256_HASH_SIZE] = {
	0x248d6a61U, 0xd20638b8U, 0xe5c02693U, 0x0c3e6039U,
	0xa33ce459U, 0x64ff2167U, 0xf6ecedd4U, 0x19db06c1U,
};



uint32_t H[SHA256_HASH_SIZE];


static void dump_H(const uint32_t *h);
static int test(const uint32_t *message, uint32_t num_steps, const uint32_t *H_expected);

static void dump_H(const uint32_t *h)
{
	int i;

	for (i = 0; i < SHA256_HASH_SIZE; ++i) {
		printf(" %08x", h[i]);
	}
}


static int test(const uint32_t *message, uint32_t num_steps, const uint32_t *H_expected)
{
	unsigned i;

	sha256_init(H);
	sha256_step(H, message, num_steps);

	printf("\nmessage:");
	for (i = 0; i < num_steps * SHA256_STEP_SIZE; ++i) {
		if (i % 8 == 0) {
			printf("\n\t");
		}
		printf(" %08x", SWAP(message[i]));
	}
	printf("\nexpected:");
	dump_H(H_expected);
	printf("\nresult  :");
	dump_H(H);
	printf("\n");

	if (memcmp(H, H_expected, SHA256_HASH_SIZE*sizeof(uint32_t))) {
		printf("==> FAILURE\n");
		return 1;
	}
	printf("==> PASS\n");
	return 0;
}


int main(void)
{
	int fails;

	fails = 0;

	fails += test(message0, sizeof(message0)/sizeof(uint32_t)/SHA256_STEP_SIZE, H_expected0);
	fails += test(message1, sizeof(message1)/sizeof(uint32_t)/SHA256_STEP_SIZE, H_expected1);

	return fails != 0;
}

