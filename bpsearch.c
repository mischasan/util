#include "bpsearch.h"
#include <emmintrin.h>
#include "ssearch.h"

typedef struct {
    byte        ante_bits, ante_mask; // 0-6 bits preceding the factor match.
    byte        post_bits, post_mask; // 0-6 bits following the factor match.
    uint        pattnum;              // index into the original pattv[]
    uint        factlen;              // size of fact; pattlen/4 - (0 or 1)
#   define LAST ((uint)-1)
    uint        next;      // link to next BPATT having the same factor; or LAST.
} BPATT;

struct BPSEARCH {
    BPATT*      bpattv;
    SSEARCH*    sp;
};

typedef struct {
    BPATT*      bpattv;
    byte const* target;
    uint        targlen;    // bitpair count
    uint*       matchnum;
    uint*       matchpos;
} BPMATCH;

static void bppack(byte const *inp, uint len, byte *out);
static uint on_match(uint factnum, uint endpos, BPMATCH *ctx);
//------------------------------------------------------------------
BPSEARCH *
bpsearch_create(const MEM *pattv, uint npatts)
{
    if (!pattv || !npatts) return NULL;

    uint        maxbplen = 0;
    uint        factbytes = 0;              // Upb on space required for all facts.
    uint        p;

    for (p = 0; p < npatts; ++p) {
        if (pattv[p].len <  7) return NULL; // ssearch_create would barf on a zero-length factor.
        if (pattv[p].len > 52) return NULL; // SSE2 bitshift limit; code around this some day.
        factbytes += pattv[p].len / 4;
        if (maxbplen < pattv[p].len) maxbplen = pattv[p].len;
    }

    // For each pattern, there are four shifted versions,
    //  and both normal and complemented versions; hence "* 8" everywhere:

    uint        bnext = npatts * 8;     // Allocate overflow bpatts from the END of bpattv[].
    uint        hashsize = npatts * 10; // Open-addr hash table with at least 25% empty entries.

    byte*       factdata = malloc(factbytes * 8);
    MEM*        factv = malloc(bnext * sizeof*factv);
    uint*       hashv = calloc(hashsize, sizeof*hashv);
    BPATT*      bpattv = malloc(bnext * sizeof*bpattv);

    // XOR flips: { ante_bits factor, post_bits } -- all bits except the ante_mask:
    __m128i     xorbits = { -65536ULL, -1ULL };
    __m128i     bits;
    byte*       bpatt = (byte*)&bits;
    byte*       datap = factdata;           // Ptr to next allocation in factdata.
    uint        nfacts = 0;                 // factors allocated in factv[]

    for (p = 0; p < npatts; ++p) {
        uint    shift, flip;
        uint    bplen = pattv[p].len;       // #BIT-PAIRS (#chars in the original input string)

        bpatt[0] = 0;       // Initial ante_mask
        bpatt[1] = 0xFF;    // Future ante_mask bits
        bpatt[2] = 0;       // Initial ante_bits
        bppack(pattv[p].ptr, bplen, bpatt + 3); // Factor and post_bits

        for (shift = 0; shift < 4; ++shift, bplen -= 2) {
            uint factlen = bplen / 4;

            for (flip = 0; flip < 2; ++flip) { // do bpatt then ~bpatt:

                // Fowler/Noll/Voh-1a. Good enough for factlen > 4.
                uint h, hash = 2166136261; 
                for (h = factlen; h; hash = (hash ^ bpatt[--h + 3]) * 16777619);

                while ((h = hashv[hash %= hashsize]) && factv[h-1].len != factlen
                            && memcmp(factv[h-1].ptr, bpatt, factlen)) { ++hash; }
                BPATT *bp;
                if (h) { // Insert link into chain.
                    bp = &bpattv[--bnext];
                    bp->next = bpattv[h-1].next;
                    bpattv[h-1].next = bnext;
                } else { // Start a new chain at same index as the new factor in factv[]
                    bp = &bpattv[nfacts];
                    bp->next = LAST;
                    factv[nfacts] = (MEM){ datap, factlen };
                    memcpy(datap, bpatt, factlen);
                    datap += factlen;
                    hashv[p] = nfacts++;
                }

                static byte const post_mask[] = { 0x00, 0x03, 0x0F, 0x3F };
                bp->ante_mask = bpatt[0];
                bp->ante_bits = bpatt[2];
                bp->post_mask = post_mask[bplen % 4]; // AKA ~(-1 << (bplen%4*2))
                bp->post_bits = bpatt[3 + bplen / 4] & bp->post_mask;
                bp->pattnum = p;
                bp->factlen = bplen / 4;

                bits = _mm_xor_si128(bits, xorbits);
            }
            
            // The SSE2 version of >>2:
            bits = _mm_or_si128( _mm_srli_epi64(bits, 2), _mm_slli_epi64( _mm_srli_si128(bits, 8), 62 ) );
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
    uint    targlen = (text.len + 3)/ 4;
    byte    btarget[targlen];
    MEMREF  mtarg = { btarget, targlen };
    BPMATCH ctx = { bp->bpattv, btarget, text.len, matchnum, matchpos };
    bppack(text.ptr, text.len, btarget);
    return ssearch_scan(bp->sp, mtarg, (SSEARCH_CB)on_match, &ctx);
}

void
bpsearch_destroy(BPSEARCH*bp)
{
    if (bp) free(bp->bpattv), ssearch_destroy(bp->sp), free(bp);
}

//------------------------------------------------------------------

// bppack translates a vector of [ACGT] chars into a vector of bitpairs 00 01 10 11.
//  Input is is case-insensitive. Bitpairs are packed 4 per byte, least significant bit first.
// bppack makes use of the ASCII representation of the chars:
//      A:01000001 C:01000011 G:01000111 T:01010100
//             ^^         ^^         ^^         ^^
// bppack relies on:
//  - the input buffer is actually a multiple of 16 bytes long, even if fewer bytes are used.
//  - the output buffer is actually a multiple of 4 bytes long; trailing bitpairs are zeroed.
// bppack benefits from the input buffer being __attribute__((aligned(16))).
//  It compresses 16 bytes to 4 bytes in 20 instructions; no jumps, no lookups.
//XXX: extend this to AVX where available (32 bytes per iteration).

static void
bppack(byte const *inp, uint len, byte *out)
{
    uint32_t*   outp = (uint32_t*)out;
    __m128i     m02   = _mm_set_epi64x(0x0202020202020202ULL, 0x0202020202020202ULL);
    __m128i     m06   = _mm_set_epi64x(0x0606060606060606ULL, 0x0606060606060606ULL);
    __m128i     m00ff = _mm_set_epi64x(0x000000FF000000FFULL, 0x000000FF000000FFULL);

    for (; len > 0; len -= sizeof m02, inp += sizeof m02) {

        __m128i x = (__m128i)_mm_loadu_pd( (double const*)inp );

        // Translate 16 bytes in parallel: "ACGT" -> (0,1,3,2)
        x = _mm_srli_epi64( _mm_and_si128(x, m06), 1 );

        // Transform (G=3 T=2) -> (G=2 T=3). "AND" is needed for lack of a _mm_srli_epi8().
        x = _mm_xor_si128( x, _mm_srli_epi64( _mm_and_si128(x, m02), 1 ) );

        // Pack the low 2 bits of each byte into the low byte of each 32bit word.
        x = _mm_and_si128( m00ff, _mm_or_si128( _mm_or_si128( _mm_or_si128( x, _mm_srli_epi64(x, 6) ),
                                                                               _mm_srli_epi64(x,12) ),
                                                                               _mm_srli_epi64(x,18) ) );
        // Pack the bottom byte of each 32-bit word into one 32-bit word.
        *outp++ = _mm_cvtsi128_si32( _mm_packus_epi16( _mm_packs_epi32(x, x), x) );
    }

    if (len)    // Mask out trailing garbage bits.
        outp[-1] &= (uint32_t)-1 >> -2*len;
}

// on_match: called by ssearch_scan each time a factor matches the target.
//  The callback checks the list of bp-patterns containing that factor,
//  for a complete match. If found, the bp-pattern number and the offset
//  in the target text are saved to be returned by bpsearch_scean;
//  and on_match returns 1 to exit ssearch_scan.

static uint
on_match(uint factnum, uint endpos, BPMATCH *ctx)
{
    BPATT*  bp;

    for (; factnum != LAST; factnum = bp->next) {
        bp = ctx->bpattv + factnum;
        uint len = ctx->targlen / 4;

        //XXX this is not quite right for trailing zeroes!
        if ((!bp->ante_mask || (endpos - len > 0 &&
                                !(bp->ante_mask & (bp->ante_bits ^ ctx->target[endpos - len - 1]))))
         && (!bp->post_mask || (endpos + 1 < len &&
                                !(bp->post_mask & (bp->post_bits ^ ctx->target[endpos + 1] ))))) {

            *ctx->matchnum = bp->pattnum;
            byte m = bp->ante_mask;
            *ctx->matchpos = endpos * 4 - ctx->targlen
                                - (!m ? 0 : m == 3 ? 1 : m == 15 ? 2 : 3);
            return 1;   // End the search.
        }
    }

    return 0;
}
