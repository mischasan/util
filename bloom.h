#ifndef BLOOM_H
#define BLOOM_H

typedef struct bloom_s BLOOM;

// The caller is expected to calculate the number of hash bits to provide
//  for a given npoints. QV:
//  http://pages.cs.wisc.edu/~cao/papers/summary-cache/node8.html

// There's no obvious incremental way to "grow" a bloom filter.
//TODO Add bloom_save/bloom_open/bloom_close for mmap'd filter.
//  Also: possibly logging updates to replay on rebuilding filter.

// (npoints) and (point_bits) must be powers of 2.
BLOOM *bloom_create(int hash_bits, int npoints, int point_bits);
void bloom_destroy(BLOOM*);

void bloom_add(BLOOM*, char const *hash);
void bloom_del(BLOOM*, char const *hash);
int  bloom_chk(BLOOM const*, char const *hash);

// bloom_stat: number of points in use. If >npoints/2, npoints is too small.
int bloom_stat(BLOOM const*);

// bloom_over: number of overflows. If >npoints/64, point_bits is too small.
int bloom_over(BLOOM const*);

#ifdef _STDIO_H
void bloom_dump(BLOOM const*, FILE*);
#endif

#endif/*BLOOM_H*/
