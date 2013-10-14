#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
// LACK OF A CLEAR DEFINITION OF A BIT INDEX MAKES THIS A MESS
#include "ubloom.h"

/* Bloom parameters M/N and K chosen to guarantee FP rate < 0.5%
 * M/N = filter-bits / expected-records (16)
 *  K  = number of hash functions (5)
 */

#define NHASHES    5
#define BITS      16
#define PGSIZE  4096 // efficient mappable pagesize.
    // F2B: ratio of file size to bitv size.
#define F2B     ((BITS + 7)/ 8 + BITS * sizeof(uint16_t))
    // PGCOUNTS: number of countv elements per page
#define PGCOUNTS    (PGSIZE/sizeof(uint16_t))

// .blm file is:
//      bit     bitv[nrecs * BITS] -- ie byte bitv[nrecs*BITS/8]
//      ushort  countv[nrec * BITS]

struct ubloom {
    int         fd;         // File of F2B*nrecs bytes.
    uint32_t    nbytes;     // Size of bitv[] in bytes
    uint8_t    *bitv;       // bitv[] mmap'd start of file
    off_t       point;      // seek-offset of current countp
    uint16_t   *countp;     // mmap'd 4K page @ point
};

typedef struct { uint32_t pos; uint8_t mask; } BIT;

static int cmp(BIT const *a, BIT const *b)
{ return a->pos - b->pos; }

static inline BIT
mkbit(UBLOOM const *rp, uint32_t hash, uint32_t bits)
{ return (BIT){ hash & (rp->nbytes - 1), 1 << (bits & 7) }; }

static void flip(UBLOOM *rp, BIT b) 
{ rp->bitv[b.pos] ^= b.mask; }

static int test(UBLOOM *rp, BIT b)
{ return rp->bitv[b.pos] & b.mask; }

static void add(UBLOOM*, int addend, BIT b);
static void addv(UBLOOM*, int addend, UHASH const[], int);

//--------------|-------|-------------------------------------
void
ubloom_create(char const *filename, int nrecs)
{
    // Round nrecs up to the next power of two.
    if (nrecs & (nrecs - 1)) {
        do nrecs &= nrecs - 1; while (nrecs & (nrecs - 1));
        nrecs <<= 1;
    }

    // Ensure that bitv occupies at least one page,
    // so that countv starts on a page boundary in the file.

    uint32_t    nbytes = nrecs * BITS / 8;
    if (nbytes < PGSIZE)
        nbytes = PGSIZE;

    int fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    assert(fd >= 0);
    int rc = ftruncate(fd, (off_t)F2B * nbytes);
    assert(rc >= 0);
    close(fd);
}

void
ubloom_destroy(char const *filename) { unlink(filename); }

UBLOOM*
ubloom_open(char const *filename, int mode)
{
    UBLOOM      r;

    r.fd         = open(filename, 0666, mode);
    assert(r.fd >= 0);

    off_t       bytes = lseek(r.fd, 0L, SEEK_END);
    assert(bytes > 0);
    assert(bytes % F2B == 0);

    r.nbytes    = bytes / F2B;
    r.bitv      = mmap(NULL, r.nbytes, PROT_READ|PROT_WRITE, MAP_SHARED, r.fd, 0);
    assert(r.bitv != MAP_FAILED);

    r.point     = 0;
    r.countp    = mmap(NULL, PGSIZE, PROT_READ|PROT_WRITE, MAP_SHARED, r.fd, r.nbytes);
    assert(r.bitv != MAP_FAILED);

    return  memcpy(malloc(sizeof(UBLOOM)), &r, sizeof(r));
}

void
ubloom_close(UBLOOM *rp)
{
    if (!rp) {
        munmap(rp->bitv, rp->nbytes);
        munmap(rp->countp, PGSIZE);
        close(rp->fd);
        free(rp);
    }
}

int
ubloom_test(UBLOOM  const*rp, UHASH const hash)
{
    uint32_t    i, b = hash[5];

    for (i = 0; i < 5; ++i, b >>= 3) {
        BIT     x = mkbit(rp, hash[i], b);
        if (!(rp->bitv[x.pos] & x.mask))
            return  0;
    }

    return  1;
}

void
ubloom_add(UBLOOM *rp, UHASH const hash)
{
    uint32_t    i, b = hash[5];
    for (i = 0, b = hash[5]; i < 5; ++i, b >>= 3)
        add(rp, 1, mkbit(rp, hash[i], b));
}

void
ubloom_del(UBLOOM *rp, UHASH const hash)
{
    unsigned    i, b;
    for (i = 0, b = hash[5]; i < 5; ++i, b >>= 3)
        add(rp, -1, mkbit(rp, hash[i], b));
}

void
ubloom_addv(UBLOOM *rp, UHASH const hashv[], int nhashes)
{
    addv(rp, 1, hashv, nhashes);
}

void
ubloom_delv(UBLOOM *rp, UHASH const hashv[], int nhashes)
{
    addv(rp, -1, hashv, nhashes);
}

//--------------|-------|-------------------------------------
static void
add(UBLOOM *rp, int addend, BIT b)
{
    off_t       point = b.pos * 8 * sizeof(uint16_t) & -PGSIZE;

    if (rp->point != point) {
        munmap(rp->countp, PGSIZE);
        mmap(rp->countp, PGSIZE, PROT_WRITE, MAP_FIXED,
                rp->fd, rp->nbytes + point);
        rp->point = point;
    }

    uint16_t    *np  = rp->countp + (b.pos * 8 & -PGSIZE;
    if (!*np || !(*np + addend))
        flip(rp, b);

    *np += addend;
}

static void
addv(UBLOOM *rp, int addend, UHASH const hash[], int nhashes)
{
    if (!nhashes)
        return;

    BIT     *bits = malloc(nhashes*5 * sizeof(BIT));
    BIT     *bitp = bitv;
    int     i, j, b;

    for (i = 0; i < nhashes; ++i)
        for (j = 0, b = hash[i][5]; j < 5; ++j, b >>= 3)
            *bitp++ = mkbit(hash[i][j], b);

    qsort(numv, nhashes*5, sizeof(BIT), (comparison_fn_t) cmp);

    for (i = 0; i < nhashes * 5; ++i)
        add(rp, addend, bitv[i]);

    free(bits);
}

//EOF
