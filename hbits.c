#include <stdio.h>
#include <stdlib.h>	// abs
#include <string.h>

typedef int (*QSORT_CMP)(const void *, const void *);
static unsigned sortmask;
static int maskcmp(unsigned *a, unsigned *b)
{
    return (int)(*a & sortmask) - (int)(*b & sortmask);
}

int
main(int argc, char **argv)
{
    unsigned	size = 1024;
    unsigned	count = 0;
    unsigned	*v = malloc(size * sizeof(*v));

    if (argc < 2)
	return fputs("usage: hbits <numberfile>\n", stderr), 1;

    FILE	*fp = strcmp(argv[1], "-") ? fopen(argv[1], "r") : stdin;
    if (!fp)
	return fprintf(stderr, "hbits: unable to open %s\n", argv[1]), 2;
    while (fscanf(fp, "%u", &v[count]) == 1)
	if (++count == size)
	    v = realloc(v, (size <<= 1) * sizeof(*v));
    fclose(fp);
    fprintf(stderr, "count: %d\n", count);

    unsigned	i, j, k = 0;
    
//    for (i = 0; i < count; ++i) printf("%8X\n", v[i]); 
    unsigned	min = 0xFFFFFFFF, max = 0;
    for (i = 0; i < count; ++i)
	min &= v[i], max |= v[i];
    unsigned	left = min ^ max, w, bits;
    for (bits = 0, w = left; w; ++bits, w >>= 1);
    printf("min %8X max %8X chg %8X bits %d\n", min, max, left, bits);

    //WTF: how does this result in a final iteration where maxseg > 1?
    unsigned	dbits = 0, done = 0, next = 0, prev, segs, maxseg;
    for (; left; left &= ~next, ++dbits) {
	unsigned    sum[bits], psum[bits];
	prev = 0;
	segs = 0;
	maxseg = 0;
	for (j = 0; j < bits; ++j)
	    sum[j] = psum[j] = 0;

	for (i = 0; i < count; ++i) {
	    next = v[i] & done;
	    if (i == 0 || next != prev) {
		if (i > 0) {

		    for (j = 0; j < bits; ++j)
			if (left & (1 << j))
			    sum[j] += (i - k) - abs((i - k)/2 - psum[j]);

		    for (j = 0; j < bits; ++j)
			psum[j] = 0;

		    if (maxseg < i - k)
			maxseg = i - k;
		}

		++segs;
		k = i;
		prev = next;
	    }

	    if (maxseg < i - k)
		maxseg = i - k;

	    for (j = 0; j < bits; ++j) 
		if (v[i] & left & (1 << j))
		    ++psum[j];
	}

	for (j = 0; j < bits; ++j)
	    if (left & (1 << j))
		sum[j] += (i - k) - abs((i - k)/2 - psum[j]);

	for (j = 1, k = 0, max = sum[0]; j < bits; ++j)
	    if (max < sum[j])
		max = sum[k = j]; 

	sortmask = done |= next = 1 << k;
	printf("mask: %8X max: %d segs: %d maxseg: %d want: %d\n", 
		sortmask, max, segs, maxseg, (count*3) >> (dbits+1));
	qsort(v, count, sizeof(*v), (QSORT_CMP)maskcmp);
	if ((count* 3) >> (dbits+1) < maxseg)
	    break;
    }

    for (i = 0, prev = -1; i < count; ++i) {
	next = v[i] & sortmask;
	if (prev != next)
	    printf("\n%08X ", prev = next);
	printf(" %X", v[i] & ~sortmask);
	//printf("%6d %8X %8X\n", i, v[i], i ? v[i] ^ v[i-1] : 0);
    }

    putchar('\n');
    // Calculate hash(x) : (x & M >> S) ^ (x & ~M) to maximize the number
    // of bits in (done) that get packed into the low bits.
    return  0;
}
