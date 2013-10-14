#include <tap.h>
#include <msutil.h>

void prbm(BITMAT const*);
void dotest(int nrows, int ncols, int *xys);

int
main(void)
{
    plan_tests(5);
    setvbuf(stdout, 0, _IOLBF, 0);
    int pts[] = { 1,3, 2,5, 4,0, 8,9, 5,20, 6,23, 10,10, 10,11, 10,12, 15,22, 13,23, -1 };
    
    dotest( 8,  8, pts);
    dotest( 8, 24, pts);
    dotest(16, 16, pts);
    dotest(16, 24, pts);
    dotest(24, 24, pts);
 
    return exit_status();
}

void
dotest(int nrows, int ncols, int *xys)
{
    BITMAT *inp = bitmat(nrows, ncols);
    BITMAT *exp = bitmat(ncols, nrows);

    for (; *xys >= 0; xys += 2) {
        bitmat_set(inp, xys[0] % nrows, xys[1] % ncols, 1);
        bitmat_set(exp, xys[1] % ncols, xys[0] % nrows, 1);
    }

    BITMAT *act = bitmat_trans(inp);
    int rc = refcmp(bitmat_ref(act), bitmat_ref(exp));
    ok(!rc, "trans %d x %d", nrows, ncols);
    if (rc) {
        puts("expected:"); prbm(exp);
        puts("actual:"); prbm(act);
    }

    bitmat_destroy(inp);
    bitmat_destroy(exp);
    bitmat_destroy(act);
}

void
prbm(BITMAT const*bmp)
{
    int r, c, nrows = bitmat_rows(bmp), ncols = bitmat_cols(bmp);

    for (r = 0; r < nrows; ++r) {
        printf("%3d ", r);
        for (c = 0; c < ncols; ++c) {
            if (!(c & 7)) putchar(' ');
            putchar("-#"[bitmat_get(bmp, r, c)]);
        }
        putchar('\n');
    }
}

#if 0
BITMAT*
naive_trans(BITMAT const*bmp)
{
    unsigned i, j;
    BITMAT *ret = bitmat(bmp->ncols, bmp->nrows);
    for (i = 0; i < bmp->nrows; ++i)
        for (j = 0; j < bmp->ncols; ++j)
            bitmat_set(ret, j, i, bitmat_get(bmp, i, j));
    return ret;
}
#endif
