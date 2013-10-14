#include <stdint.h>     // intptr_t
#include <xmmintrin.h>
#include "msutil.h"

/* At what point does ssesum perform better than intsum, ever??
 */

inline int intsum(int const *vec, int len)
{
    int	sum = 0;

    switch (len) {
    default:
	for (; len > 16; len -= 16, vec += 16) {
	    _mm_prefetch(vec+16, 0);    // 64 byte stride
	    sum += vec[ 0]; sum += vec[ 1]; sum += vec[ 2]; sum += vec[ 3];
	    sum += vec[ 4]; sum += vec[ 5]; sum += vec[ 6]; sum += vec[ 7];
	    sum += vec[ 8]; sum += vec[ 9]; sum += vec[10]; sum += vec[11];
	    sum += vec[12]; sum += vec[13]; sum += vec[14]; sum += vec[15];
	}

    case 16 : sum += vec[15];
    case 15 : sum += vec[14];
    case 14 : sum += vec[13];
    case 13 : sum += vec[12];
    case 12 : sum += vec[11];
    case 11 : sum += vec[10];
    case 10 : sum += vec[ 9];
    case  9 : sum += vec[ 8];
    case  8 : sum += vec[ 7];
    case  7 : sum += vec[ 6];
    case  6 : sum += vec[ 5];
    case  5 : sum += vec[ 4];
    case  4 : sum += vec[ 3];
    case  3 : sum += vec[ 2];
    case  2 : sum += vec[ 1];
    case  1 : sum += vec[ 0];
    case  0 : ;
    }

    return  sum;
}

#define THRESHOLD 12
int ssesum(int const *vec, int len)
{
    if (!len)
	return	0;

    // Too small to take advantage of m128; or hopeless unaligned
    if (len < THRESHOLD || (intptr_t)vec & 3)    
        return intsum(vec, len);

    int	const	*last = vec + len - 4;
    int         sum = 0;

    switch ((intptr_t)vec & 12) {
    case 12:    sum += *vec++;
    case  8:    sum += *vec++;
    case  4:    sum += *vec++;
    }

    XMM const *mp = (XMM const*) vec;
    _mm_prefetch(mp+4, 0); 
    XMM      msum = *mp++;

    do {
        _mm_prefetch(mp+4, 0); 
        msum = _mm_add_epi32(msum, *mp);
    } while ((int const*)mp <= last);

    msum = _mm_add_epi32(msum, _mm_shuffle_epi32(msum, 0x4E));
    sum += __builtin_ia32_vec_ext_v4si((__v4si)msum, 0)
         + __builtin_ia32_vec_ext_v4si((__v4si)msum, 1);

    vec = (int const *)mp;
    switch (last + 4 - vec) {
    case 3:     sum += *vec++;
    case 2:     sum += *vec++;
    case 1:     sum += *vec;
    }

    return  sum;
}
