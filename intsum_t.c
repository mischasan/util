#include <xmmintrin.h>
#include "msutil.h"

static int unrolled(int const *vec, int len);
static int naive(int const *vec, int len);
static void try(int count, int reps, int *data, char const*name, int (*)(int const *,int));

int
main(int argc, char **argv)
{
    if (argc < 2 || argc > 3)
        die("Usage: t.unrolled <veclen> [reps]");

    int     j, count = atoi(argv[1]);
    int     reps = argc == 3 ? atoi(argv[2]) : 1000;

    // Force data to be aligned on a 16byte boundary:
    int     *data = malloc((count + 7) * sizeof(int));
    data += (-(int)data & 15) >> 2;
    for (j = 0; j < count; ++j) data[j] = j + 1;

    try(count, reps, data, "naive", naive);
    try(count, reps, data, "unrolled", unrolled);
    try(count, reps, data, "ssesum", ssesum);
    try(count, reps, data, "naive", naive);
    try(count, reps, data, "unrolled", unrolled);
    try(count, reps, data, "ssesum", ssesum);

    return  0;
}

static void try(int count, int reps, int *data, char const*name, int (*func)(int const *,int))
{
    int     i, sum = 0;
    double  t0 = tick();
    for (i = 1; i <= reps; ++i)
	sum = func(data, count);
    printf("%d * %8s[%d]: %.4f %9d\n", reps, name, count, tick() - t0, sum);
}

static int naive(int const *vec, int len)
{
    int     j, sum;
    for (j = sum = 0; j < len; ++j)
        sum += vec[j];
    return  sum;
}

#define UNROLL 16
int unrolled(int const *vec, int len)
{
    int	sum = 0;

    for (; len >= UNROLL; len -= UNROLL, vec += UNROLL) {
        sum += vec[ 0]; sum += vec[ 1]; sum += vec[ 2]; sum += vec[ 3];
#if UNROLL > 4
        sum += vec[ 4]; sum += vec[ 5]; sum += vec[ 6]; sum += vec[ 7];
#if UNROLL > 8
        sum += vec[ 8]; sum += vec[ 9]; sum += vec[10]; sum += vec[11];
        sum += vec[12]; sum += vec[13]; sum += vec[14]; sum += vec[15];
#endif
#endif
    }

    switch (len) {
#if UNROLL > 8
    case 15 : sum += vec[14];
    case 14 : sum += vec[13];
    case 13 : sum += vec[12];
    case 12 : sum += vec[11];
    case 11 : sum += vec[10];
    case 10 : sum += vec[ 9];
    case  9 : sum += vec[ 8];
    case  8 : sum += vec[ 7];
#endif
#if UNROLL > 4
    case  7 : sum += vec[ 6];
    case  6 : sum += vec[ 5];
    case  5 : sum += vec[ 4];
    case  4 : sum += vec[ 3];
#endif
    case  3 : sum += vec[ 2];
    case  2 : sum += vec[ 1];
    case  1 : sum += vec[ 0];
    case  0 : ;
    }

    return  sum;
}
