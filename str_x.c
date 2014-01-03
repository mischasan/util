// Copyright (C) 2009-2013 Mischa Sandberg <mischasan@gmail.com>
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

#include "msutil.h"

static char *bergstr(char *phaystack, char *pneedle);
//scanstr in msutil.h
static char *winstr(char *tgt, char *pat);
static char *Railgun7g(char *pbTarget, int cbTarget, char *pbPattern, int cbPattern);
static char *Railgun7w(char *pbTarget, int cbTarget, char *pbPattern, int cbPattern);

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
              "- WINS: use 2,3,4-byte sliding window\n"
              "- BNDM: Backward Non-deterministic DAWG matching (suffix shift-and)\n"
              "- RG7G: Georgi's Railgun version 7-Gulliver\n"
              "- RG7W: Georgi's Railgun version 7-Wolfram\n");
    argv += verbose = !strcmp(argv[1], "-v");
    MEMBUF x = chomp(slurp(argv[1]));
    if (nilbuf(x)) die(": cannot read %s:", argv[1]);
    count = atoi(argv[2]) | 1;  // count MUST be odd
    MEMREF  *rv = refsplit(x.ptr, '\n', &nr);
    char    **exp = malloc(nr * sizeof(*exp));

    if (verbose) printf("SSE2 BERG WINS BNDM RG7g RG7w nchrs=%"FSIZE"d\n", x.len);
    for (a = 3; argv[a]; ++a) {
        patlen = strlen(argv[a]);
        memset(exp, -1, nr * sizeof(*exp));

#           define DO_MEM(T,F) double T = do_mem(#F, (MEMF)F, rv, nr, count, argv[a], exp)
#           define DO_STR(T,F) double T = do_str(#F, (STRF)F, rv, nr, count, argv[a], exp)
        DO_STR(tbase, strstr);
        DO_STR(tsse2, sse2str);
        DO_STR(tberg, bergstr);
        DO_STR(twins, winstr);
        DO_MEM(tbndm, bndmem);
        DO_MEM(trg7g, Railgun7g);
        DO_MEM(trg7w, Railgun7w);
#           undef  DO_MEM
#           undef  DO_STR
        if (verbose) printf("%4.0f %4.0f %4.0f %4.0f %4.0f %4.0f P=%d '%s'\n",
               tsse2 * 100 / tbase,
               tberg * 100 / tbase,
               twins * 100 / tbase,
               tbndm * 100 / tbase,
               trg7g * 100 / tbase,
               trg7w * 100 / tbase,
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

// STRESS_C blocks some duplicate declarations.
#define  STRESS_C 1
#include "bergstr.c"
#include "bndmem.c"
#include "railgun7g.c"
#include "railgun7w.c"
#include "winstr.c"
