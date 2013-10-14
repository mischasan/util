// Bounded non-deterministic DAWG search; a suffix shift-and algorithm.

#include <stdint.h>
#include <immintrin.h> // Permit AVX (__m256i)
#include "msutil.h"

#define INTBITS (sizeof(int)*8)
static inline void setbit(void *v, int p)   // p in 0..255
{ ((int*)v)[p / INTBITS] |= 1 << (p & (INTBITS - 1)); }


// SSE2 has no 128-bit bit shift. "Manually" carry the top bit
//  of the low (64bit) half to the bottom of the high half.
static inline __m128i xm_shl_001(__m128i x) 
{ return _mm_or_si128(_mm_slli_epi64(x, 1), _mm_srli_epi64(_mm_slli_si128(x, 8), 63)); }

char *
bndmem(char *target, int tgtlen, char *pattern, int patlen)
{
    uint8_t *tgt = (uint8_t*)target, *pat = (uint8_t*)pattern;
    uint8_t *tgtend = tgt + tgtlen - patlen;
    int i, skip;
#if __SIZEOF_INT__ == 4
    if (patlen <= 32) {
        int32_t mask, maskv[256] = {};
        
        for (i = 0; i < patlen; ++i)
            setbit(&maskv[pat[i]], sizeof(mask)*8 - 1 - i);

        for (; tgt <= tgtend; tgt += skip)
            for (i = skip = patlen, mask = maskv[tgt[--i]];
                    mask; mask = (mask << 1) & maskv[tgt[--i]])
                if (mask < 0 && !(skip = i)) return (char*)tgt;
    } else 
#endif
    if (patlen <= 64) {
        int64_t mask, maskv[256] = {};

        for (i = 0; i < patlen; ++i)
            setbit(&maskv[pat[i]], sizeof(mask)*8 - 1 - i);

        for (; tgt <= tgtend; tgt += skip)
            for (i = skip = patlen, mask = maskv[tgt[--i]];
                    mask; mask = (mask << 1) & maskv[tgt[--i]])
                if (mask < 0 && !(skip = i)) return (char*)tgt;

    } else if (patlen <= 128) {
        __m128i mask, maskv[256], zero = {};
        int8_t used[256] = {};

        for (i = 0; i < patlen; ++i) {
            if (!used[pat[i]]) used[pat[i]] = 1, maskv[pat[i]] = zero;
            setbit(&maskv[pat[i]], sizeof(mask)*8 - 1 - i);
        }
        
        for (; tgt <= tgtend; tgt += skip) {
            i = skip = patlen;
            if (!used[tgt[--i]]) continue;
            mask = maskv[tgt[i]]; // gteed not zero.
            do {
                if (0 > (int16_t)_mm_movemask_epi8(mask) && !(skip = i))
                    return (char*)tgt;
                if (!used[tgt[--i]]) break;
                mask = _mm_and_si128(xm_shl_001(mask), maskv[tgt[i]]);
            } while (0xFFFF != _mm_movemask_epi8(_mm_cmpeq_epi8(mask, zero)));
        }
    }
    //TODO: AVX 256-bit ops.
    return NULL;
}
