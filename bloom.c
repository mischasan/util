#include <stdio.h>
#include <stdint.h>
#include "msutil.h" // bitwid
#include "bloom.h"

typedef struct { int point, count; } OVFL;

struct bloom_s {
    int     hash_bits;  // Bit width of hash.
    int     npoints;    // Must be 2**N
    int     point_bits; // Bit width of each point.
    int     hash_chunk; // Bits per slice of hash used as pointv[] index.
    int     hash_mask;  // Wide enough to mask [0..npoints-1]
    int     point_max;  // Largest value that fits in a point (a power of 2 - 1)
    uint8_t*pointv;
    int     ovfl_limit; // Must be 2**N - 1
    int     ovfl_count;
    OVFL*   ovfl;
#   define MIN_OVFL_LIMIT 7     // because (7-1)*5/4>=7 but (3-1)*5/4<3
};

static int  ovfl_find(BLOOM const*, int point);
static void ovfl_inc(BLOOM*, int point);
static int  ovfl_dec(BLOOM*, int point); // return 1 if decrements to 0

// CHUNK: returns a bit substring of a multibyte hash.
static inline int CHUNK(BLOOM const *bp, char const *hash, int bitpos)
{ return ((*(uint32_t const*)(hash + (bitpos>>3)) >> (bitpos & 7)) & bp->hash_mask) & (bp->npoints - 1); }

#define FOR_CHUNKS(i, bp) \
    for (i = 0; i <= (bp)->hash_bits - (bp)->hash_chunk; i += (bp)->hash_chunk)

//XXX change these to bitfield functions:
static inline int  point_val(BLOOM const*bp, int point) { return bp->pointv[point]; }
static inline void point_inc(BLOOM const*bp, int point) { ++bp->pointv[point]; }
static inline void point_dec(BLOOM const*bp, int point) { --bp->pointv[point]; }
//--------------|---------------------------------------------
BLOOM *
bloom_create(int hash_bits, int npoints, int point_bits)
{
    if (hash_bits <= 0 || npoints <= 0 || npoints & (npoints - 1)
        || point_bits > 8 || point_bits & (point_bits - 1))
        return NULL;
        //XXX for bitfields change calloc(npoints,1) to calloc(npoints*point_bits/8 + 1,1)
    int wid = bitwid(npoints);
    BLOOM *bp = malloc(sizeof *bp);
    *bp = (BLOOM){ hash_bits, npoints, point_bits, wid, (1 << wid) - 1,
                   (1 << point_bits) - 1, calloc(npoints,1), MIN_OVFL_LIMIT, 0,
                   calloc(MIN_OVFL_LIMIT, sizeof(OVFL)) };
    return bp;
}

void
bloom_destroy(BLOOM*bp)
{ if (bp) free(bp->pointv), free(bp->ovfl), free(bp); }

void
bloom_add(BLOOM *bp, char const *hash)
{
    int bitoff;
    FOR_CHUNKS(bitoff, bp) {
        int pt = CHUNK(bp, hash, bitoff);
        if (point_val(bp, pt) < bp->point_max)
            point_inc(bp, pt);
        else ovfl_inc(bp, pt);
    }
}

void
bloom_del(BLOOM *bp, char const *hash)
{
    int bitoff;
    FOR_CHUNKS(bitoff, bp) {
        int pt = CHUNK(bp, hash, bitoff);
        if (point_val(bp, pt) < bp->point_max || ovfl_dec(bp, pt))
            point_dec(bp, pt);
    }
}

int
bloom_chk(BLOOM const*bp, char const *hash)
{
    int bitoff;
    FOR_CHUNKS(bitoff, bp)
        if (!point_val(bp, CHUNK(bp, hash, bitoff)))
            return 0;
    return 1;
}

int
bloom_stat(BLOOM const*bp)
{
    int i = 0, tot = 0;
    for (; i < bp->npoints; ++i) tot += !!point_val(bp, i);
    return tot;
}

int
bloom_over(BLOOM const*bp)
{
    int i = 0, tot = 0 ;
    while (i <= bp->ovfl_limit) tot += !!bp->ovfl[i++].count;
    return tot;
}
//--------------|---------------------------------------------
void
bloom_dump(BLOOM const *bp, FILE *fp)
{
    fprintf(fp, "{ hash_bits:%d npoints:%d point_bits:%d"
                    " hash_chunk:%d hash_mask:%d point_max:%d"
                    " ovfl_limit:%d ovfl_count:%d"
                "\n  pointv:( ",
                bp->hash_bits, bp->npoints, bp->point_bits,
                bp->hash_chunk, bp->hash_mask, bp->point_max,
                bp->ovfl_limit, bp->ovfl_count);
    int i, val;
    for (i = 0; i < bp->npoints; ++i)
        if ((val = point_val(bp, i)))
            fprintf(fp, "%s[%d]:%d ", !i || i%10 ? "" : "\n\t\t", i,
                    val + bp->ovfl[ovfl_find(bp,i)].count);

    fputs(")\n  ovfl:{", fp);
    for (i = 0; i <= bp->ovfl_limit; ++i)
        if (bp->ovfl[i].count)
            fprintf(fp, " %d:%d", bp->ovfl[i].point, bp->ovfl[i].count);

    fputs(" }\n}\n", fp);
}
//--------------|---------------------------------------------
// ovfl: open-addressing hash table, max 4/5 full.

static int
ovfl_find(BLOOM const*bp, int point)
{
    int i;
    for (i = point;; ++i) {
        i &= bp->ovfl_limit;
        if (!bp->ovfl[i].count || bp->ovfl[i].point == point)
            return i;
    }
}

static void
ovfl_inc(BLOOM *bp, int point)
{
    int i = bp->ovfl_limit;
    if (i <= bp->ovfl_count * 5 / 4) {
        // Double the hash table size:
        OVFL *oldp = bp->ovfl;
        bp->ovfl_limit += bp->ovfl_limit + 1;
        bp->ovfl = calloc(bp->ovfl_limit, sizeof(OVFL));
        for (; i >= 0; --i)
            bp->ovfl[ovfl_find(bp, oldp[i].point)] = oldp[i];
        free(oldp);
    }

    i = ovfl_find(bp, point);
    if (!bp->ovfl[i].count++)
        bp->ovfl[i].point = point, bp->ovfl_count++;
}

static int
ovfl_dec(BLOOM *bp, int point)
{
    int i = ovfl_find(bp, point);
    if (!  bp->ovfl[i].count) return 1;
    if (!--bp->ovfl[i].count) {
        // .count has gone from 1 to 0: the ovfl[] entry is dead,
        //  so higher entries may need to shift down.
        --bp->ovfl_count;
        while (bp->ovfl[i = (i + 1) & bp->ovfl_limit].count) {
            int j = ovfl_find(bp, bp->ovfl[i].point);
            if (j != i)
                bp->ovfl[j] = bp->ovfl[i], bp->ovfl[i].count = 0;
        }
    }

    return 0;
}
