#define XXX 1
#define TEST

#include <xmmintrin.h>
#include <stdint.h>

static inline char* xmemcpy(char *dst, char const *src, int len);

#ifdef TEST
#include <stdio.h>
#include <stdlib.h>     // atoi
#include <string.h>     // memcpy
#include <sys/time.h>   // gettimeofday

static inline double tick(void)
{
    struct timeval t;
    gettimeofday(&t, 0);
    return t.tv_sec + 1E-6 * t.tv_usec;
}

int
main(int argc, char **argv)
{
    if (argc < 3) return fputs("Usage: xmemcpy <reps> <bytes>\n", stderr);

    int nreps = atoi(argv[1]);
    int nbytes = atoi(argv[2]);
    if (nbytes > 9000) return fputs("nbytes too big\n", stderr);

    int i;
    static char src[9999], dst[9999];

    double t0 = tick();
    for (i = 0; i < nreps; ++i) memcpy(dst+(i & 63), src+(i & 127), nbytes);
    double t1 = tick(); 
    for (i = 0; i < nreps; ++i) xmemcpy(dst+(i & 63), src+(i & 127), nbytes);
    double t2 = tick();

    fprintf(stderr, "%.4g %.4g\n", t1 - t0, t2 - t1);

    return 0;
}
#endif//TEST

static char* xmemcpy(char *dst, char const *src, int len)
{
    __m128i val;    // Without an intermediate var, MSVC generates no code (!?)

#   define COPY16(dst, src) \
        val = _mm_loadu_si128((__m128i const*)(src)); \
        _mm_storeu_si128((__m128i*)(dst), val);

    while (len >= 256) {
        _mm_prefetch(src + 256, _MM_HINT_NTA);
        COPY16(dst + 00, src + 00);
        COPY16(dst + 16, src + 16);
        COPY16(dst + 32, src + 32);
        COPY16(dst + 48, src + 48);
        dst += 64, src += 64, len -= 64;
    }

    while (len >= 16) {
        COPY16(dst, src);
        dst += 16, src += 16, len -= 16;
    }

    if (len >= 8) {
        *(uint64_t*)dst = *(uint64_t const*)src;
        dst += 8, src +=8, len -= 8;
    }

    if (len >= 4) {
        *(uint32_t*)dst = *(uint32_t const*)src;
        dst += 4, src +=4, len -= 4;
    }

    if (len >= 2) {
        *(uint16_t*)dst = *(uint16_t const*)src;
        dst += 2, src +=2, len -= 2;
    }

    if (len >= 1)
        *dst++ = *src;

    return dst;
#   undef COPY16
}
