// Copyright (C) 2009-2013 Sophos LLC. All rights reserved.
//-------------------------------------------------------------------

#include "msutil.h"
#include <tap.h>
#include "ssearch.h"

extern int ssearch_skip[16];

static int on_match(int strnum, const char *textp, MEMREF const *);

static MEMBUF   text;
static int      verbose;
static int      hits;

int
main(int argc, char **argv)
{
    char    *pgm = strrchr(argv[0], '/');
    pgm	= pgm ? pgm + 1 : argv[0];

    while (argc > 1 && !strcmp(argv[1], "-v"))
	++verbose, ++argv;

    if (argc < 2) {
	fprintf(stderr, "Usage: %s [-v [-v]...] <patt_file> [text_file [matches]]\n", pgm);
	return	1;
    }

    MEMBUF  patt = chomp(slurp(argv[1]));
    fprintf(stderr, "# %s: %"FSIZE"d bytes\n", argv[1], patt.len);

    int	    npatts, i;
    MEMREF  *pattv = refsplit(patt.ptr, '\n', &npatts);
    text = argc > 2 && strcmp(argv[1], argv[2]) ? slurp(argv[2]) : patt;
    // text now contains \0's instead of \n's,
    //  but ssearch treats \0 like any other byte.
    int	    matches = argc > 3 ? atoi(argv[3]) : npatts;

    SSEARCH *ssp = ssearch_create(pattv, npatts);
    ssearch_dump(ssp, verbose ? stderr : fopen("/dev/null", "w")); // coverage

    plan_tests(4);
    ok(patt.ptr, "patt_file loaded");
    ok(npatts, "text_file loaded");
    ok(ssp, "ssearch_create returns a value");

    hits = 0;
    double	t = tick();
    ssearch_scan(ssp, bufref(text), (SSEARCH_CB)on_match, pattv);
    t = tick() - t;
    if (!ok(hits == matches, "match count"))
	fprintf(stderr, "# actual: %d expected: %d\n", hits, matches);
    fprintf(stderr, "# elapsed: %G secs\n", t);
    if (verbose)
        for (i = 0; i < 16; ++i)
            if (ssearch_skip[i]) 
                fprintf(stderr, "# skip %2d: %8d\n", i, ssearch_skip[i]);
    ssearch_destroy(ssp);   // coverage
    buffree(patt);
    free(pattv);
    return exit_status();
}

static int
on_match(int strnum, const char *textp, MEMREF const *pattv)
{
    ++hits;
    if (verbose > 1)
	fprintf(stderr, "match patt[%d] '%.*s' at %"FSIZE"d\n",
		strnum, (int)pattv[strnum].len, pattv[strnum].ptr,
		textp - text.ptr);
    return  1;
}
