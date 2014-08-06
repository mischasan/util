#include "tap.h"
#include "msutil.h"

typedef double D16[16];// __attribute( (__aligned__(16)) );
typedef int (*cmpfn_t)(const void *, const void *);

void dumpd(double v[16], const char *label);
static int cmpd(double const*ap, double const *bp);

#define LOOPS   1000000

static void insort(double data[16]);

int
main(void)
{
    int i;

    plan_tests(2);
    {
	double	init[16] = { 29, 16, 25, 14, 21, 12, 17, 10,
                             13,  8,  9,  6,  5,  4,  1, 2 }, expect[16], actual[16];

	dumpd(init, "unsorted");
        double  t0 = tick();
        for (i = 0; i < LOOPS; ++i) {
            memcpy(expect, init, sizeof(init));
            qsort(expect, 16, sizeof(double), (cmpfn_t)cmpd);
        }
        double  t1 = tick();
	dumpd(expect, "sorted");

        double t2 = tick();
        for (i = 0; i < LOOPS; ++i) {
            memcpy(actual, init, sizeof(init));
            insort(actual);
        }

        double t3 = tick();
        for (i = 0; i < LOOPS; ++i) {
            memcpy(actual, init, sizeof(init));
            ssesort16d(actual);
        }

        int j, rank[16];
        double work[16];

        double t4 = tick();
        for (i = 0; i < LOOPS; ++i) {
            memcpy(work, init, sizeof(init));
            sserank16d(work, rank);
            for (j = 0; j < 16; ++j) actual[j] = init[rank[j]];
        }

        double t5 = tick();
	if (!ok(!memcmp(actual, expect, sizeof(actual)), "sserank output is sorted"))
	    dumpd(actual, "actual");
	if (!ok(!memcmp(actual, work, sizeof(actual)), "ssesort output is sorted"))
	    dumpd(actual, "actual");

        fprintf(stderr, "x %d: qsort: %.4f insertion: %.4f ssesort: %.4f sserank: %.4f\n",
                LOOPS, t1 - t0, t3 - t2, t4 - t3, t5 - t4);
    }

    return exit_status();
}

static void
insort(double data[16])
{
    int     i;
    for (i = 1; i < 16; ++i) {
        if (data[i] < data[i - 1]) {
            double  tmp = data[i];
            do    data[i] = data[i-1];
            while (--i > 0 && tmp < data[i-1]);
            data[i] = tmp;
        }
    }
}

static int cmpd(double const*ap, double const *bp)
{
    double	diff = *ap - *bp;

    return  diff < 0 ? -1 : diff > 0 ? 1 : 0;
}
#if 0
static int cmpc8(char const**ap, char const **bp)
{
    return memcmp(ap, bp, 8);
}

static void 
dumpc8(char v[16][8], const char *label)
{
    int		r;

    fprintf(stderr, "# %s:\n", label);
    for (r = 0; r < 16; ++r)
	fprintf(stderr, " %.8s", v[r]);
    putc('\n', stderr);
}
#endif

void 
dumpd(double v[16], const char *label)
{
    int		r;

    fprintf(stderr, "# %s:\n", label);
    for (r = 0; r < 16; r += 2) {
	int	n0 = 15 & *(char*)&v[r];
	int	n1 = 15 & *(char*)&v[r+1];

	fprintf(stderr, "# %2d:%5g    %2d:%5g\n",
		n0, v[r], n1, v[r+1]);
    }
}
