#include <assert.h>
#include <string.h>
#include "msutil.h"

#define min(a,b) ((a) < (b) ? (a) : (b))
#define SWAP(x,y) do { typeof(x) __t=x; x=y; y=__t; } while (0)
static void simple(MEMREF *v, int n);
static inline void simpl3(MEMREF *v);
static void quick(MEMREF *v, int n);
static void owsort(MEMREF *v, int n);
//static void resort(MEMREF *v, int n);
#define MAXWID  64

int
main(int argc, char **argv)
{
    if (argc != 2)
        return fputs("usage: t.sort <inpfile>\n", stderr);

    MEMBUF  inp = chomp(slurp(argv[1]));
    int	    nrecs;
    MEMREF  *inpv = refsplit(inp.ptr, '\n', &nrecs);
    MEMREF  *v1 = malloc(nrecs * sizeof(MEMREF));
    MEMREF  *v2 = malloc(nrecs * sizeof(MEMREF));
    MEMREF  *v3 = malloc(nrecs * sizeof(MEMREF));
    int     wid, i;

    double  t1[MAXWID] = {}, t2[MAXWID] = {}, t3[MAXWID] = {};
    
    for (wid = 2; wid < MAXWID; wid++) {
        memcpy(v1, inpv, nrecs * sizeof(MEMREF));
        double  start = tick();
        for (i = 0; i + wid < nrecs; i+= wid) {
            switch (wid) {
            case 2:
                if (refcmp(v1[0], v1[1]) > 0)
                    SWAP(v1[0], v1[1]);
                break;
            case 3:
                simpl3(v1);
                break;
            default:
                simple(v1, nrecs);
            }
        }
        t1[wid] += tick() - start;

        memcpy(v2, inpv, nrecs * sizeof(MEMREF));
        start = tick();
        for (i = 0; i + wid < nrecs; i+= wid) quick(v2, nrecs);
        t2[wid] += tick() - start;
        assert(!memcmp(v1, v2, nrecs * sizeof(MEMREF)));

        memcpy(v3, inpv, nrecs * sizeof(MEMREF));
        start = tick();
        for (i = 0; i + wid < nrecs; i+= wid) owsort(v3, nrecs);
        t3[wid] += tick() - start;
        assert(!memcmp(v1, v3, nrecs * sizeof(MEMREF)));
    }

    for (wid = 2; wid < MAXWID; wid++)
        printf("%4d %.6f %.6f %.6f ms\n", wid, 
                1000 * t1[wid] * wid / nrecs,
                1000 * t2[wid] * wid / nrecs,
                1000 * t3[wid] * wid / nrecs);

    return  0;
}

static void
quick(MEMREF *v, int n)
{
    qsort(v, n, sizeof(MEMREF), (int (*)(const void *, const void *))refpcmp);
}

static void
simple(MEMREF *v, int n)
{
    MEMREF      *rp = v + 1, *omega = v + n;
    do {
        MEMREF   *qp = rp, r = *rp;
        while (refcmp(*--qp, r) > 0 && qp > v) qp[1] = qp[0];
        qp[1] = r;
    } while (++rp != omega);
}

static void
simpl3(MEMREF *v)
{
    MEMREF  a = v[0], b = v[1], c = v[2];
    if (refcmp(a, b) <= 0)
        if (refcmp(b, c) <= 0)
            ;
        else 
        if (refcmp(a, c) <= 0)
            v[1] = c, v[2] = b;
        else
            v[0] = c, v[1] = a, v[2] = b;
    else
    if (refcmp(a, c) <= 0)
        v[1] = c, v[2] = b;
    else
    if (refcmp(b, c) <= 0)
        v[0] = b, v[1] = c, v[2] = a;
    else
        v[0] = c, v[1] = b, v[2] = a;
}

// Oosterwal sort 
// http://sites.google.com/site/computersciencesourcecode/sortroutines
//      #TOC-The-O-Sort-Oosterwal-Sort---version
static void
owsort(MEMREF *v, int n)
{
    //float   fstep = n;
    int     step = n, i;
    while ((step = step * 25 >> 5))
        for (i = n - step; --i >= 0;)
            if (refpcmp(&v[i], &v[i + step]) > 0)
                SWAP(v[i], v[i + step]);
}

#if 0
int resort_min = 16;

static void
resort(MEMREF *v, int n)
{
    MEMREF       *rp = v + 1, *omega = v + n;
    MEMREF       *eta = min(omega, v + resort_min);

    do {
        MEMREF   *ins = rp, r = *rp;

        while (refpcmp(ins-1, &r) > 0) {
            *ins = ins[-1];
            if (--ins == v) break;
        }

        *ins = r;
    } while (++rp < eta);

    for (; rp < omega; ++rp) {
        MEMREF   *ins = rp, r = *rp;
        int     back  = v - rp;
        int     rc    = 1;

        do {
            back = (back - 1) >> 1;
            if (0 < (rc = refpcmp(ins+back, &r)))
                ins += back;
        } while (back < -4);    //assert(resort_min > 4)

        // At this point, is it possible that ins == v
        //  or ins + 1 == rp? If not, change while-loops.
        if (rc > 0)
            while (ins > v &&  refpcmp(ins-1, &r))
                --ins;
        else
            while (++ins < rp && !refpcmp(ins, &r));

        if (ins < rp) {
            //do rp[0] = rp[-1]; while (--rp > ins);
            asm("std\n\trep movsd" :: "c" (rp - ins),
                                    "S" (rp - 1),
                                    "D" (rp));
            *ins = r;
        }
    }
}
#endif
