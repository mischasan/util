#include "msutil.h"
#include "xmutil.h"

#define VEC_ALIGNED

#ifdef VEC_ALIGNED
#   define  LOAD(mp)    *(mp)
#else
#   define  LOAD(mp)    _mm_loadu_si128(mp)
#endif

int ssesum(int const *vec, int len)
{
    int		sum = 0;

    if (!len)
	return	0;

    int	const	*end = vec + len;
    if (len < 8) {
	do sum += *vec++; while (vec < end);
        return sum;
    }

    XMM const *mp = (void const*)vec;
    XMM	xsum = _mm_add_epi32(LOAD(mp), LOAD(mp+1));

    for (mp += 2, len -= 8; len >= 4; ++mp, len -= 4)
        xsum = _mm_add_epi32(LOAD(mp), xsum);

    // 0x4E: (1,0,3,2);
    // After shuffle and add, xsum = (w0+w1,w1+w0,w2+w3,w3+w2)
    // Horizontal-add is only implemented for floats, not ints.
    xsum = _mm_add_epi32(_mm_shuffle_epi32(xsum, 0x4E), xsum);

    // "_mm_cvtsi128_si32(xsum)" returns xsum[0],
    // but there is no _mm function to xsum[1..].
    sum = __builtin_ia32_vec_ext_v4si((__v4si)xsum, 0)
        + __builtin_ia32_vec_ext_v4si((__v4si)xsum, 1);

    vec = (void const*)mp;

    switch (end - vec) {
    case 3: sum += vec[2];
    case 2: sum += vec[1];
    case 1: sum += vec[0];
    }

    return  sum;
}
