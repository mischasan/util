#include "msutil.h"

static char *bergstr(char *phaystack, char *pneedle);
extern char *strchrs(char *tgt, char *pat);
static char *winstr(char *tgt, char *pat);
static char *Railgun7a(char *tgt, int tgtlen, char *pat, int patlen);
static char *Railgun7g(char *pbTarget, int cbTarget, char *pbPattern, int cbPattern);
static char *Railgun7h(char *pbTarget, int cbTarget, char *pbPattern, int cbPattern);

typedef char *(*MEMF)(char *tgt, int tgtlen, char *pat, int patlen);
typedef char *(*STRF)(char *tgt, char *pat);
static double do_mem(char const *name, MEMF fn, MEMREF const*rv, int nr, int count, char *pat, char **exp);
static double do_str(char const *name, STRF fn, MEMREF const*rv, int nr, int count, char *pat, char **exp);

static int verbose = 0;

#define sse2str scanstr

int
main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    int         count = 10001;  // count MUST be odd, or hash is always zero!
    int         patlen, nr, a;
    if (argc < 4)
        usage("[-v] <file> <count> <str>...\n"
              "Compares STRSTR (SSE4.2) with:\n"
              "- SSE2: scan for leading pair of chars using SSE2\n"
              "- BERG: Steven van den Berg's strstr\n"
              "- CHRS: use (SSE2) strchr then memcmp\n"
              "- WINS: use 2,3,4-byte sliding window\n"
              "- BNDM: Backward Non-deterministic DAWG matching (suffix shift-and)\n"
              "- RG7A: Georgi's Railgun version 7-Elsiane\n"
              "- RG7G: Georgi's Railgun version 7-Gulliver\n"
              "- RG7G: Georgi's Railgun version 7-Hasherezade\n");
    argv += verbose = !strcmp(argv[1], "-v");
    MEMBUF x = chomp(slurp(argv[1]));
    if (nilbuf(x)) die(": cannot read %s:", argv[1]);
    count = atoi(argv[2]) | 1;  // count MUST be odd
    MEMREF  *rv = refsplit(x.ptr, '\n', &nr);
    char    **exp = malloc(nr * sizeof(*exp));

    if (verbose) printf("SSE2 BERG CHRS WINS BNDM RG7a RG7g RG7h nchrs=%"FSIZE"d\n", x.len);
    for (a = 3; argv[a]; ++a) {
        patlen = strlen(argv[a]);
        memset(exp, -1, nr * sizeof(*exp));

#           define DO_MEM(T,F) double T = do_mem(#F, (MEMF)F, rv, nr, count, argv[a], exp)
#           define DO_STR(T,F) double T = do_str(#F, (STRF)F, rv, nr, count, argv[a], exp)
        DO_STR(tbase, strstr);
        DO_STR(tsse2, sse2str);
        DO_STR(tberg, bergstr);
        DO_STR(tchrs, strchrs);
        DO_STR(twins, winstr);
        DO_MEM(tbndm, bndmem);
        DO_MEM(trg7a, Railgun7a);
        DO_MEM(trg7g, Railgun7g);
        DO_MEM(trg7h, Railgun7h);
#           undef  DO_MEM
#           undef  DO_STR
        if (verbose) printf("%4.0f %4.0f %4.0f %4.0f %4.0f %4.0f %4.0f %4.0f P=%d '%s'\n",
               tsse2 * 100 / tbase,
               tberg * 100 / tbase,
               tchrs * 100 / tbase,
               twins * 100 / tbase,
               tbndm * 100 / tbase,
               trg7a * 100 / tbase,
               trg7g * 100 / tbase,
               trg7h * 100 / tbase,
               patlen, argv[a]);
    }

    free(rv);
    buffree(x);

    return 0;
}

static double
do_mem(char const *name, MEMF fn, MEMREF const*rv, int nr, int count, char *pat, char **exp)
{
    int     i, r, patlen = strlen(pat);
    double  start = tick();
    char    *cp, *sp;

    //do_mem never called before do_str
    for (r = 0; r < nr; ++r) {
        cp = fn((char*)(intptr_t)rv[r].ptr, rv[r].len, pat, patlen);
        if (exp[r] != cp) {
            fprintf(stderr, "%s(pat='%s' [%d] exp=%"FSIZE"d act=%"FSIZE"d) failed\n\t[%"FSIZE"d] %s\n", name, pat, r,
                    exp[r] ? exp[r] - rv[r].ptr : -1, cp ? cp - rv[r].ptr : -1, 
                    rv[r].len, sp = acstr(rv[r].ptr, rv[r].len));
            free(sp);
            return 0;
        }
    }

    for (i = 0; i < count; ++i)
        for (r = 0; r < nr; ++r)
            if (exp[r] != fn((char*)(intptr_t)rv[r].ptr, rv[r].len, pat, patlen))
                return 0;

    return tick() - start;
}

static double
do_str(char const *name, STRF fn, MEMREF const*rv, int nr, int count, char *pat, char **exp)
{
    int     i, r;
    double  start = tick();
    char    *cp, *sp;

    for (r = 0; r < nr; ++r) {
        cp = fn((char*)(intptr_t)rv[r].ptr, pat);
        if (exp[r] == (char*)-1) 
            exp[r] = cp;
        else if (exp[r] != cp) {
            fprintf(stderr, "%s(pat='%s' [%d] exp=%"FSIZE"d act=%"FSIZE"d) failed\n\t[%"FSIZE"d] %s\n",
                    name, pat, r, exp[r] ? exp[r] - rv[r].ptr : -1, cp ? cp - rv[r].ptr : -1, 
                    rv[r].len, sp = acstr(rv[r].ptr, rv[r].len));
            free(sp);
            return 0;
        }
    }

    for (i = 0; i < count; ++i)
        for (r = 0; r < nr; ++r)
            if (exp[r] != fn((char*)(intptr_t)rv[r].ptr, pat))
                return 0;

    return tick() - start;
}

// STR_X blocks some duplicate declarations.
#define  STR_X 1
#include "bergstr.c"
#include "bndmem.c"
#include "railgun7a.c"
#include "railgun7g.c"
#include "railgun7h.c"
#include "winstr.c"
