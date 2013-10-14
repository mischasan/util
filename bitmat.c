#include <assert.h>
#include "msutil.h"
#include "xmutil.h"

struct bitmat_s { uint8_t *data; unsigned nrows, ncols; };

BITMAT*
bitmat(int nrows, int ncols)
{
    assert (nrows % 8 == 0 && ncols % 8 == 0);
    BITMAT *bmp = malloc(sizeof*bmp);
    *bmp = (BITMAT) { calloc(nrows, ncols/8), nrows, ncols };
    return bmp;
}

void
bitmat_destroy(BITMAT*bmp)
{
    if (bmp) free(bmp->data), free(bmp);
}

int
bitmat_get(BITMAT const*bmp, int row, int col)
{
    assert((unsigned)row < bmp->nrows && (unsigned)col < bmp->ncols);
    col += row * bmp->ncols;
    return !!(bmp->data[col >> 3] & (1 << (col & 7)));
}

void
bitmat_set(BITMAT *bmp, int row, int col, int val)
{
    assert((unsigned)row < bmp->nrows && (unsigned)col < bmp->ncols);
    col += row * bmp->ncols;
    uint8_t bit = 1 << (col & 7), *at = &bmp->data[col >> 3];
    *at = val ? *at | bit : *at & ~bit;
}

BITMAT*
bitmat_copy(BITMAT const*bmp)
{
    BITMAT *ret = bitmat(bmp->nrows, bmp->ncols);
    memcpy(ret->data, bmp->data, bmp->nrows * bmp->ncols / 8);
    return ret;
}

int bitmat_rows(BITMAT const *bmp) { return bmp->nrows; }
int bitmat_cols(BITMAT const *bmp) { return bmp->ncols; }
#if 0
// bitmat_prod: bit matrix product using SSE2.
// Assumes that the right-hand input is the TRANSPOSE of the usual right-hand
// argument to matrix-multiplication.
// z[row,cols] := a[rows,mids] x trans(b)[cols,mids]

void
bitmat_prod(uint8_t *z, uint8_t const *a, uint8_t const *bt, int nrows, int nmids, int ncols)
{
    int         r, c, m, cb = Bytes(ncols), mb = Bytes(nmids);
    memset(z, 0, nrows * cb);
    for (r = 0; r < nrows; r++) {
        for (c = 0; c < ncols; c++) {
            uint8_t const *ap = a + mb*c, *bp = bt + mb*c;
            for (m = 0; m < nmids && !(ap[m] & bp[m]); m++);
            if (m < nmids)
                z[r*cb + (c >> 3)] |= 1 << (c & 7);
        }
    }
}
#endif

MEMREF
bitmat_ref(BITMAT const*bmp)
{ return (MEMREF){ (char const*)bmp->data, bmp->nrows * bmp->ncols / 8 }; }

// BIT inp[nrows, ncols] => out[ncols, nrows]
static void sse_trans(uint8_t const *inp, uint8_t *out, int nrows, int ncols);

BITMAT*
bitmat_trans(BITMAT const*bmp)
{
    BITMAT *ret = bitmat(bmp->ncols, bmp->nrows);
    sse_trans(bmp->data, ret->data, bmp->nrows, bmp->ncols);
    return ret;
}

#include <assert.h>
#include <stdint.h>
#include <emmintrin.h>

void
sse_trans(uint8_t const *inp, uint8_t *out, int nrows, int ncols)
{
#   define INP(x,y) inp[(x)*ncols/8 + (y)/8]
#   define OUT(x,y) out[(y)*nrows/8 + (x)/8]
    int rr, cc, i, h;
    union { XMM x; uint8_t b[16]; } tmp;

    // Do the main body in 16x8 blocks:
    for (rr = 0; rr <= nrows - 16; rr += 16)
        for (cc = 0; cc < ncols; cc += 8) {
            for (i = 0; i < 16; ++i)
                tmp.b[i] = INP(rr + i, cc);
            for (i = 8; --i >= 0; tmp.x = _mm_slli_epi64(tmp.x, 1))
                *(uint16_t*)&OUT(rr, cc + i) = _mm_movemask_epi8(tmp.x);
        }

    if (rr == nrows) return;

    // The remainder is a block of 8x16n+8 bits (n >= 0).
    //  Do a PAIR of 8x8 blocks in each step:
    for (cc = 0; cc <= ncols - 16; cc += 16) {
        for (i = 0; i < 8; ++i)
            tmp.b[i] = h = *(uint16_t const*)&INP(rr + i, cc),
            tmp.b[i + 8] = h >> 8;
        for (i = 8; --i >= 0; tmp.x = _mm_slli_epi64(tmp.x, 1))
            OUT(rr, cc + i) = h = _mm_movemask_epi8(tmp.x),
            OUT(rr, cc + i + 8) = h >> 8;
    }

    if (cc == ncols) return;

    //  Do the remaining 8x8 block:
    for (i = 0; i < 8; ++i)
        tmp.b[i] = INP(rr + i, cc);
    for (i = 8; --i >= 0; tmp.x = _mm_slli_epi64(tmp.x, 1))
        OUT(rr, cc + i) = _mm_movemask_epi8(tmp.x);
}

// Transpose a 16x16 BYTE array using SSE2.
//  Is this faster than a simple nested loop?
//  gcc -O9 unrolls all the loops.
//  Naive is ~630 instructions, sse2 is ~100.
#if 0
void naive_16x16(char inp[16][16], char out[16][16]);
void
naive_16x16(char inp[16][16], char out[16][16])
{
    int i, j;
    for (i = 0; i < 16; ++i)
        for (j = 0; j < 16; ++j)    
            out[j][i] = inp[i][j];
}

void trans_16x16(XMM inp[16], XMM out[16]);
void trans_16x16(XMM inp[16], XMM out[16])
{
    XMM tmp[16];
    int i;

    for (i = 0; i < 7; ++i)
        tmp[i + 0] = _mm_unpacklo_epi8(inp[2*i + 0], inp[2*i + 1]),
        tmp[i + 8] = _mm_unpackhi_epi8(inp[2*i + 0], inp[2*i + 1]);

    for (i = 0; i < 15; i += 2)
        out[i + 0] = _mm_unpacklo_epi8(tmp[i + 0], tmp[i + 1]),
        out[i + 1] = _mm_unpackhi_epi8(tmp[i + 0], tmp[i + 1]);
}
#endif
