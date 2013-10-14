#define DIAG 1
#include <stdint.h>             //
#include <stdlib.h>             // malloc etc
#include <string.h>             // memcmp etc
#ifdef DIAG
#include <stdio.h>
#include <sys/time.h>
static int _nrecs;
static inline double
tick(void)
{
    struct timeval t;
    gettimeofday(&t, 0);
    return t.tv_sec + 1E-6 * t.tv_usec;
}

#define  DIAG_TICK(x) double x = tick()
#define  DIAG_NRECS(x) _nrecs = (x)
#else
#define  DIAG_TICK(x)
#define  DIAG_NRECS(x)
#endif

#include "rsort.h"

//---------- "Tunable" parameters ----------
// MINRANGE: threshold for doing an insertion sort.
#define MINRANGE        16
// SCANWID: bytes analyzed per keyscan()
#define SCANWID         16

//---------- Space-limiting parameters ----------
#define PARTBITS        16
//#define MAXPARTS        (257 * 257)
#define MAXPARTS        (257 * 257 + 2000)
#define MAXMAPS         (SCANWID + PARTBITS + 1)
// Marker for mdata rows not allocated to mapv 
#define UNUSED          MAXPARTS

// RANGE: a range of recv[] entries (alpha..omega-1)
//  which have the same prefix of (len) bytes.
//  Segs have at least MINRANGE entries; shorter runs
//  are sorted directly (linear insertion sort).

typedef struct { RSREC **alpha, **omega; } RANGE;

// PART: a linked list of records grouped by source range.
//  This uses pdata[] indexes, not pointers, as links, so that
//  the end-of-list test is against (0). The end-of-list
//  is a dummy element, not NULL, to simplify the innermost
//  loop of gather().

typedef unsigned PARTNO;
typedef struct
{
    PARTNO next;
    RANGE *rng;
    RSREC *rec;
} PART;

static inline int imin(int a, int b) { return a < b ? a : b; }

// keyscan: collect byte stats for a segment across all unsorted keys.
// analyze: compute a mapping from a key segment to (0..MAXPARTS-1)
// scatter: partition keys into linked lists.
// gather:  reinsert keys from lists into original vector.
// insort:  insertion sort for tiny buckets.

static void keyscan(int pos, RANGE const *, int nsrcs,
                    PARTNO mdata[][256], PARTNO ** mapv);

static int analyze(int maxleng, int const *lengv,
                   int nsrcs, RANGE const *srcv,
                   PARTNO ** mapv, PARTNO mdata[][256],
                   int *posp, int *skipp, int *lenp);

static void scatter(int nsrcs, RANGE * srcv, int pos,
                    int len, PARTNO ** mapv, PARTNO * partv, PART * pdata);

static int gather(int nextpos, int nparts,
                  PARTNO const *partv, PART const *pdata, RANGE * dstv);

static void insort(RSREC ** alpha, RSREC ** omega, int pos);
//--------------|-------|-------------------------------------
void
rsort(RSREC ** recv, int nrecs)
{
    int i, k = 256, maxleng = 0;
    int *lengv = calloc(k, sizeof(*lengv));

    for (i = 0; i < nrecs; ++i) {
        int leng = KEYLENG(recv[i]);
        if (leng >= k) {
            int kk = k;
            while (kk <= leng)
                kk <<= 1;
            lengv = realloc(lengv, kk * sizeof(int));
            memset(lengv + k, 0, (kk - k) * sizeof(int));
            k = kk;
        }

        ++lengv[leng];
        if (maxleng < leng)
            maxleng = leng;
    }

    DIAG_NRECS(nrecs);

    if (maxleng == 0) {         // edge case: empty input 
        free(lengv);
        return;
    }

    int pos = 0, nsrcs = 1, skip;
    PART *pdata = malloc((nrecs + 1) * sizeof(PART));
    PARTNO *partv = malloc(MAXPARTS * sizeof(*partv));
    int rsize = ((nrecs - 1) / MINRANGE + 1) * sizeof(RANGE);
    RANGE *srcv = malloc(rsize);
    RANGE *dstv = malloc(rsize);
    PARTNO *mapv[maxleng + 1];
    PARTNO mdata[MAXMAPS][256];

    for (i = 0; i <= maxleng; ++i)
        mapv[i] = 0;
    for (i = 1; i < MAXMAPS; ++i)
        *mdata[i] = UNUSED;

    // mdata[0] is permanently all 0. It is used to "map"
    //        bytes in the middle of the range (pos;len) that
    //        have exactly one value.
    memset(mdata[0], 0, sizeof(mdata[0]));

    // pdata[0]: a dummy list terminator; simplifies a loop 
    pdata[0] = (PART) {
    0, NULL, NULL};

    // Initial srcv: a single range of the entire input 
    srcv[0] = (RANGE) {recv, recv + nrecs};
    for (pos = 0; nsrcs; pos += skip) {
        int len, nparts;

        DIAG_TICK(t1);
        nparts = analyze(maxleng, lengv, nsrcs, srcv,
                         mapv, mdata, &pos, &skip, &len);
        DIAG_TICK(t2);

        // Initialize partv[] to the index of dummy pdata[0]. 
        for (i = 0; i < nparts; ++i)
            partv[i] = 0;

        scatter(nsrcs, srcv, pos, len, mapv, partv, pdata);

        for (i = 0; i < len; ++i)
            *mapv[pos + i] = UNUSED;
        mdata[0][0] = 0;        // Undo "freeing" of mdata[0] 
#if DIAG
        int maxpart = -1;
        for (i = k = 0; i < nparts; ++i) {
            k += !!partv[i];
            if (partv[i]) maxpart = i;
        }
        fprintf(stderr, "# %9d recs %5d rngs on"
                " %d:%d%s %d/%d parts max=%d",
                _nrecs, nsrcs, pos, pos + len - 1,
                len == skip ? "" : "+", k, nparts, maxpart);
        DIAG_TICK(t3);
        DIAG_NRECS(0);          // updated by gather() 
#endif
        nsrcs = gather(pos + skip, nparts, partv, pdata, dstv);
#if DIAG
        fprintf(stderr, "\t%.3f %.3f %.3f secs\n",
                t2 - t1, t3 - t2, tick() - t3);
#endif
        RANGE *tmp = srcv;
        srcv = dstv;
        dstv = tmp;
    }

    free(dstv);
    free(srcv);
    free(partv);
    free(pdata);
    free(lengv);
}

// Compute (pos,len,mapv[]) that maps substr(key,pos,len)
//  in sort-order to a PARTNO 0..nparts-1, allowing for
//  variable-length keys. (len) depends on how many unique
//  values each byte in that range can have; the product
//  of those counts must be no greater than MAXPARTS.

static int
analyze(int maxleng, int const *lengv,
        int nsrcs, RANGE const *srcv,
        PARTNO ** mapv, PARTNO mdata[][256], int *posp, int *skipp, int *lenp)
{
    int i, j, k, step, width[maxleng + 1];
    int pos = *posp, skip = 0, len = 0, nparts = 1;
//fprintf(stderr,"calculate nparts: "); char const *sep = "";
    while (1) {
        int p = pos + skip, more;

        if (p >= maxleng)
            break;

        if (!mapv[p])           // get (SCANWID) more maps 
            keyscan(p, srcv, nsrcs, mdata, mapv);

        for (i = more = 0; i < 256; ++i)
            more += mapv[p][i];
        if (!more)
            break;
        width[p] = more += lengv[p] && p > *posp;
        if (more == 1) {
            *mapv[p] = UNUSED;
            mapv[p] = mdata[0];
        }

        // TODO: nparts at any stage is not this value! It needs to be
        // the sum of child indices, not just the product of dimensions.
        if (nparts * more == 1)
            ++pos;              // skip leading const bytes at start of range in first iteration only 
        else if (nparts * more > MAXPARTS)  //WRONG
            break;
        else
            ++skip, nparts *= more; //WRONG
//fprintf(stderr, "%s(%d%s)", sep, more, lengv[p] && p > *posp ? "" : "-1"); sep = " * ";
    }
    for (len = skip; width[pos + len - 1] == 1; --len);

//fprintf(stderr, " = %d; recalc = 1", nparts);
    for (i = len, nparts = step = 1; --i >= 0;) {

        if (width[pos + i] > 1) {

            j = lengv[pos + i] ? step : 0;
            for (k = 0; k < 256; ++k) {
                if (mapv[pos + i][k]) {
                    mapv[pos + i][k] = j;
                    j += step;
                }
            }
//fprintf(stderr, " + %d", j - step);
            nparts += j - step;
            step = j;
        }
    }
//fprintf(stderr, "= %d\n", nparts);
    *posp = pos;
    *skipp = skip;
    *lenp = len;
    return nparts;
}

// Partition recs across partv[] by their (pos,seglen) bytes.
//  Store keys with leng <= (pos), in their final location.

static void
scatter(int nsrcs, RANGE * srcv, int pos, int seglen,
        PARTNO ** mapv, PARTNO * partv, PART * pdata)
{
    int i, dn = 1;
    uint8_t buf[seglen];

    for (i = 0; i < nsrcs; ++i) {
        RSREC **rpp = srcv[i].alpha;

        while (rpp != srcv[i].omega) {
            RSREC *rp = *rpp++;
            int keybytes = KEYLENG(rp) - pos;

            if (keybytes <= 0) {

                *srcv[i].alpha++ = rp;

            }
            else {
                int n = imin(seglen, keybytes);
                uint8_t const *dp = KEYDATA(rp, pos, n, buf);
                int rix = mapv[pos][*dp];

                while (--n > 0)
                    rix += mapv[pos + n][dp[n]];

                pdata[dn] = (PART) {
                partv[rix], &srcv[i], rp};
                partv[rix] = dn++;
            }
        }
    }
}

// Copy each partition of each range back into recv[],
//  starting from the back to make the sort stable.
//  A partition of <MINRANGE keys is immediately sorted
//  (insertion sort). Otherwise, it becomes a range
//  to be processed by the next pass (dstv).

static int
gather(int nextpos, int nparts, PARTNO const *partv,
       PART const *pdata, RANGE * dstv)
{
    int ndsts = 0, i, curr;

    for (i = nparts; --i >= 0;) {

        for (curr = partv[i]; curr;) {
            RANGE *rngp = pdata[curr].rng;
            RSREC **rpp = rngp->omega, **omega;

            do {
                *--rpp = pdata[curr].rec;
                curr = pdata[curr].next;
            } while (pdata[curr].rng == rngp);

            omega = rpp;
            while (KEYLENG(*rpp) <= nextpos && ++rpp < rngp->omega);

            int count = rngp->omega - rpp;

            if (count >= MINRANGE) {

                dstv[ndsts++] = (RANGE) {
                rpp, rngp->omega};
                DIAG_NRECS(_nrecs + count);

            }
            else if (count > 1 && KEYLENG(*rpp) > nextpos) {

                insort(rpp, rngp->omega, nextpos);
            }

            rngp->omega = omega;
        }
    }

    return ndsts;
}

static void
insort(RSREC ** alpha, RSREC ** omega, int pos)
{
    RSREC **rpp = alpha;

    while (++rpp != omega) {
        RSREC **qpp, *rp = *rpp;
        int rlen = KEYLENG(rp) - pos;

        for (qpp = rpp; --qpp >= alpha; qpp[1] = qpp[0]) {
            int qlen = KEYLENG(*qpp) - pos;
            int rc = KEYDIFF(*qpp, rp, pos, imin(rlen, qlen));
            if (rc < 0 || (rc == 0 && qlen <= rlen))
                break;
        }

        qpp[1] = rp;
    }
}

// Compile histograms of unique byte-value occurrences
//  in the next SCANWID bytes of all (unsorted) keys.

static void
keyscan(int pos, RANGE const *srcv, int nsrcs,
        PARTNO mdata[][256], PARTNO ** mapv)
{
    int i, j;
    uint8_t buf[SCANWID];

    // Allocate [SCANWID] mdata vectors 
    for (i = j = 0; i < SCANWID; ++j) {
        if (*mdata[j] == UNUSED) {
            memset(mdata[j], 0, sizeof(mdata[0]));
            mapv[pos + i++] = mdata[j];
        }
    }

    for (i = 0; i < nsrcs; ++i) {
        RSREC **rpp = srcv[i].alpha;

        for (; rpp != srcv[i].omega; ++rpp) {
            j = imin(SCANWID, KEYLENG(*rpp) - pos);
            uint8_t const *dp = KEYDATA(*rpp, pos, j, buf);
            while (--j >= 0)
                mapv[pos + j][dp[j]] = 1;
        }
    }
}
