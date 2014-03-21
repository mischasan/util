// Copyright (C) 2014 Mischa Sandberg <mischasan@gmail.com>
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

// SSEARCH is a better-behaved Wu-Manber-like algorithm. The alphabet of
//  packed ACGT strings is closer to 256 than to 16, so hash-hop algorithms
//  should outperform SBOM.
// ssearch_create will build its machine based on the shortest factor length (6 bytes),
//  so it will usually skip through the target in steps of 4-5 bytes.

#include "bpsearch.h"
#include <assert.h>
#include <emmintrin.h>
#include "ssearch.h"

typedef struct {
    byte        ante_bits, ante_mask; // 0..6 bits preceding the factor match.
    byte        post_bits, post_mask; // 0..6 bits following the factor match.
    uint        pattnum;              // index into the original pattv[]
    uint        factlen;              // size of fact; pattlen / 4 - (0 or 1)
#   define LAST ((uint)-1)
    uint        next;      // Link to next BPATT having the same factor; or LAST.
} BPATT;

struct BPSEARCH {
    BPATT*      bpattv;
    SSEARCH*    sp;
};

typedef struct {
    BPATT const*bpattv;
    byte const* target;     // Result of acgtpack.
    uint        targpairs;    // #bitpairs in target[]
    byte        offtarget;  // Bits in last byte of target beyond actgpack output.
    uint*       matchnum;
    uint*       matchpos;
} BPMATCH;

static uint on_match(uint factnum, uint endpos, BPMATCH *ctx);

// Lookup; probably faster than ~(-1 << 2*i)
static byte const post_mask[] = { 0x00, 0x03, 0x0F, 0x3F };
//------------------------------------------------------------------
BPSEARCH *
bpsearch_create(MEM const*pattv, uint npatts)
{
    if (!pattv || !npatts) return NULL;

    // tot_factbytes is an upper bound: for the 4 different shifts, a pattern's factors
    //  will have take at most (4*floor(pattlen / 4) - pattlen%4 + 3) bytes.
    uint    tot_factbytes = 0; // Upper bound on space required for all facts.
    uint    p;

    for (p = 0; p < npatts; ++p) {

        // ssearch_create will not accept a factor length < 2.
        if (pattv[p].len < 11) return NULL; 

        //PRO TEM: using only one __m128i in "pat"
        //  limits (factor+post_bits) to 13 bytes.
        if (pattv[p].len > 52) return NULL; 

        tot_factbytes += pattv[p].len / 4;
    }

    // For each pattern, there are four shifted versions, and both
    //  normal and complemented versions; hence "* 8" everywhere.
    // bnext is the exact #bpattv, and an upper bound on #factv.

    uint    nfacts = 0;             // # of factors allocated in factv[] so far.
    uint    bnext = npatts * 8;     // Allocate overflow bpatts from the END of bpattv[].
    uint    hashsize = npatts * 10; // Open-addr hash table with at least 25% empty entries.

    MEM*    factv = malloc(bnext * sizeof*factv);
    uint*   hashv = calloc(hashsize, sizeof*hashv);
    BPATT*  bpattv = malloc(bnext * sizeof*bpattv);
    byte*   factdata = malloc(tot_factbytes * 8);
    byte*   nextfact = factdata;                // Ptr to next allocation in factdata.

    // post_bits is the byte after the factor, i.e. pat.s.factor[factlen]
    //  post_mask is calculated from (bpcount % 4), which cycles through (3,2,1,0)
    //  from different starting points.

    union { __m128i bits; struct { byte ante_mask, mask_fill, ante_bits, factor[14]; } s; } pat;
    __m128i xorbits = { -256LL, -1LL }; // Flips all bits except ante_mask.
    pat.s.factor[13] = 0;               // In case pattv[p].len is 52.

    for (p = 0; p < npatts; ++p) {
        uint shift, flip;
        uint bpcount = pattv[p].len;    // #bitpairs is initially the #chars in the input string.

        pat.s.ante_mask = 0;
        pat.s.mask_fill = 0xFF;         // These bits shift into the top of ante_mask.
        pat.s.ante_bits = 0;            // The first byte of factor shifts into ante_bits.
        acgtpack(pattv[p].ptr, bpcount, pat.s.factor); // Populate (factor, post_bits)

        for (shift = 0;; ++shift, bpcount -= 2) {
            uint factlen = bpcount / 4;   // #bytes in factor

            for (flip = 0; flip < 2; ++flip) { // Do patt then ~patt:

                // Fowler/Noll/Vo-1a[32] hash function. Good enough for factlen > 4.
                uint h, hpos = 2166136261;
                for (h = factlen; h; hpos = (hpos ^ pat.s.factor[--h]) * 16777619);

                // Hash table values are either 0 (empty) or (factv-index + 1).
                while ((h = hashv[hpos %= hashsize]) && factv[h-1].len != factlen
                            && memcmp(factv[h-1].ptr, pat.s.factor, factlen)) { ++hpos; }

                // If h is 0, hpos is where to insert the new entry.

                BPATT *bp;
                if (h) {                    // Existing factor (multiple patterns have the same factor).
                    bp = &bpattv[--bnext];          // Alloc ovfl from beyond any possible factv-index.
                    bp->next = bpattv[h-1].next;    // Insert into linked list directly after list head.
                    bpattv[h-1].next = bnext;
                } else {                    // New factor.
                    bp = &bpattv[nfacts];           // Alloc bp at same bpattv-index as new factor in factv.
                    bp->next = LAST;
                    factv[nfacts] = (MEM){ nextfact, factlen };
                    memcpy(nextfact, pat.s.factor, factlen); // factv[x] points at bytes in factdata
                    nextfact += factlen;
                    hashv[hpos] = ++nfacts;
                }

                bp->pattnum   = p;
                bp->factlen   = factlen;
                bp->ante_mask = pat.s.ante_mask;
                bp->ante_bits = pat.s.ante_bits;
                bp->post_mask = post_mask[bpcount % 4];
                bp->post_bits = pat.s.factor[factlen] & bp->post_mask;

                pat.bits = _mm_xor_si128(pat.bits, xorbits);
            }

            // In parallel, shift a bitpair down:
            //  - from mask_fill into ante_mask
            //  - from factor[0] into ante_bits
            //  - from post_bits into factor[factlen-1].
            // This expression amounts to _mm_srli_si128(pat.bits, 2), if such a function existed.
            pat.bits = _mm_or_si128( _mm_srli_epi64(pat.bits, 2),
                                     _mm_slli_epi64( _mm_srli_si128(pat.bits, 8), 62 ) );
        }
    }

    BPSEARCH ret = { bpattv, ssearch_create((MEMREF const*)factv, nfacts) };

    free(factdata);
    free(factv);
    free(hashv);

    return memcpy(malloc(sizeof ret), &ret, sizeof ret);
}

int
bpsearch_scan(BPSEARCH *bp, MEM text, uint* matchnum, uint*matchpos)
{
    byte    btarget[(text.len + 3) / 4];
    MEMREF  sstext = { btarget, text.len / 4 };
    BPMATCH ctx = { bp->bpattv, btarget, text.len, ~post_mask[text.len % 4], matchnum, matchpos };

    acgtpack(text.ptr, text.len, btarget);
    return ssearch_scan(bp->sp, sstext, (SSEARCH_CB)on_match, &ctx);
}

void
bpsearch_destroy(BPSEARCH*bp)
{
    if (bp) ssearch_destroy(bp->sp), free(bp->bpattv), free(bp);
}

//------------------------------------------------------------------
// acgtpack makes use of the ASCII representation of the chars:
//      A:01000001 C:01000011 G:01000111 T:01010100
//             ^^         ^^         ^^         ^^
//  It compresses 16 bytes to 4 bytes in 20 register ops; no jumps, no lookups.
//XXX: extend this to AVX where available (32 bytes per iteration).
//  Performance issue is with packing 100-byte targets, not (27..43)-byte patterns.

void
acgtpack(byte const *inp, uint len, byte *out)
{
    uint32_t*outp = (uint32_t*)out;
    __m128i  m02   = _mm_set_epi64x(0x0202020202020202ULL, 0x0202020202020202ULL);
    __m128i  m06   = _mm_set_epi64x(0x0606060606060606ULL, 0x0606060606060606ULL);
    __m128i  m00ff = _mm_set_epi64x(0x000000FF000000FFULL, 0x000000FF000000FFULL);

    for (; len > 0; len -= sizeof m02, inp += sizeof m02) {

        __m128i x = (__m128i)_mm_loadu_pd( (double const*)inp );

        // Translate 16 bytes in parallel: "ACGT" -> (0,1,3,2)
        x = _mm_srli_epi64( _mm_and_si128(x, m06), 1 );

        // Translate (G=3 T=2) -> (G=2 T=3).
        x = _mm_xor_si128( x, _mm_srli_epi64( _mm_and_si128(x, m02), 1 ) );

        // Pack the low 2 bits of each byte into the low byte of each (32-bit) word.
        x = _mm_and_si128( m00ff, _mm_or_si128( _mm_or_si128( _mm_or_si128( x, _mm_srli_epi64(x, 6) ),
                                                                               _mm_srli_epi64(x,12) ),
                                                                               _mm_srli_epi64(x,18) ) );
        // Pack the low byte of each word into the low word of x.
        uint32_t val = _mm_cvtsi128_si32( _mm_packus_epi16( _mm_packs_epi32(x, x), x) );
        if (len < sizeof m02) {
            out = (byte*)outp;
            for (val &= ~(-1 << 2*len); len > 0; len -=4, val >>= 8)
                *out++ = (byte)val;
            break;
        }
        *outp++ = val;
    }
}

// on_match: called by ssearch_scan each time a factor matches the target.
//  The callback checks the list of bp-patterns containing that factor,
//  for a complete match. If found, the bp-pattern number and the offset
//  in the target text are saved as outputs from bpsearch_scan.
//  on_match returns 1 to exit via ssearch_scan from bpsearch_scan.

static uint
on_match(uint factnum, uint endpos, BPMATCH *ctx)
{
    BPATT const*bp;

    for (; factnum != LAST; factnum = bp->next) {
        bp = ctx->bpattv + factnum;
        uint len = ctx->targpairs / 4;

        // This is not the most ridiculous if-condition I've ever written. Really.
        if ((!bp->ante_mask || (endpos - len > 0
                                && !(bp->ante_mask & (bp->ante_bits ^ ctx->target[endpos - len - 1]))))

         && (!bp->post_mask || (endpos + 1 < len
                                && !(bp->post_mask & (bp->post_bits ^ ctx->target[endpos + 1] ))
                                && (endpos + 1 < len - 1 || !(bp->post_mask & ctx->offtarget))))
           ) {

            *ctx->matchnum = bp->pattnum;
            byte m = bp->ante_mask;
            *ctx->matchpos = endpos * 4 - ctx->targpairs
                                    - (!m ? 0 : m == 3 ? 1 : m == 15 ? 2 : 3);
            return 1;   // End the search.
        }
    }

    return 0;
}
