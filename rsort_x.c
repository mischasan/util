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

//#define TAP

#include "msutil.h"	// slurp ...
#include <sys/resource.h>
#ifdef TAP
#include "tap.h"
#endif
#include "rsort.h"

static int rsreccmp(RSREC **a, RSREC **b);

int
main(int argc, char **argv)
{
    if (argc < 2 || argc > 3) {
	fputs("usage: rsort_x <inputfile> [outfile | -]\n", stderr);
	return 0;
    }
    MEMBUF  inp = chomp(slurp(argv[1]));
    if (nilbuf(inp)) die(": cannot read %s\n", argv[1]);
#ifdef TAP
    plan_tests(1);
#endif
    struct rusage ru;
    getrusage(0, &ru);
    long    rss = ru.ru_maxrss;     

    int	    nrecs;
    MEMREF  *inpv = refsplit(inp.ptr, '\n', &nrecs);
    RSREC   **actv = malloc(nrecs * sizeof(*actv));
    RSREC   **expv = malloc(nrecs * sizeof(*expv));
    uint8_t    *data = malloc(inp.len + nrecs * sizeof(RSREC));
    uint8_t    *dp = data;
    int	    i;

    for (i = 0; i < nrecs; ++i) {
	actv[i] = (RSREC*) dp;
	actv[i]->leng = inpv[i].len;
	memcpy(actv[i]->data, inpv[i].ptr, inpv[i].len);
	dp = &actv[i]->data[inpv[i].len];
    }

    memcpy(expv, actv, nrecs * sizeof(*expv));
    double  t0 = tick();
    rsort(actv, nrecs);
    double  t_rsort = tick() - t0;

    getrusage(0, &ru);
    rss = ru.ru_maxrss - rss;

    t0 = tick();
    qsort(expv, nrecs, sizeof(*expv), (qsort_cmp)rsreccmp);
    double  t_qsort = tick() - t0;

    if (argc > 2) {
	FILE	*fp = strcmp(argv[2], "-") ? fopen(argv[2], "w") : stdout;
        char    buf[16384];
        setvbuf(fp, buf, _IOFBF, 16384);
	for (i = 0; i < nrecs; ++i)
	    fprintf(fp, "%.*s\n", actv[i]->leng, actv[i]->data);
	if (fp != stdout)
	    fclose(fp);
    }

    int	    unstable = 0;
    for (i = 1; i < nrecs; ++i) {
	int	rc = rsreccmp(&actv[i-1], &actv[i]);
	if (rc > 0)
	    break;
	if (rc == 0 && actv[i-1] > actv[i]) {
	    ++unstable;
	    fprintf(stderr, "unstable at %d: %.*s\n", i, actv[i]->leng, actv[i]->data);
	}
    }
#ifdef TAP
    ok(i == nrecs, "%d recs (%"FSIZE"d bytes) sorted correctly (%sstably) in %.2f/%.2f secs; rsort.rss=%ldKB",
	nrecs, inp.len, unstable ? "un" : "", t_rsort, t_qsort, rss);
#else
    fprintf(stderr, "%d recs (%"FSIZE"d bytes) sorted %scorrectly (%sstably) in %.2f/%.2f secs; rsort.rss=%ldKB\n",
	nrecs, inp.len, i == nrecs ? "" : "in", unstable ? "un" : "", t_rsort, t_qsort, rss);
    if (i < nrecs) {
	fprintf(stderr, "[%d] %.*s\n", i-1, actv[i-1]->leng, actv[i-1]->data);
	fprintf(stderr, "[%d] %.*s\n", i, actv[i]->leng, actv[i]->data);
    }
#endif

    buffree(inp);
    free(actv);
    free(data);
    free(expv);
    free(inpv);
#ifdef TAP
    return exit_status();
#else
    return  0;
#endif
}

static inline int imin(int a, int b) { return a < b ? a : b; }
static int rsreccmp(RSREC **a, RSREC **b)
{
    int	    len = imin((*a)->leng, (*b)->leng);
    int	    rc  = memcmp((*a)->data, (*b)->data, len);
    return rc ? rc : (*a)->leng - (*b)->leng;
}
