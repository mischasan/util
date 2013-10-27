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
#include <errno.h> 
#include <fcntl.h> // open(2)
#include <tap.h>
#include "psearch.h"

#ifdef STAT
typedef struct { long long val; const char *name; } PSSTAT;
extern PSSTAT psstat[];
extern int pscand[];
#endif//STAT

static FILE *matchfp;
static int actual = 0;
static int
on_match(int strnum, int textpos, MEMREF const *pattv)
{
    (void)strnum, (void)textpos, (void)pattv;
    ++actual;
//    fprintf(matchfp, "%9d %7d '%.*s'\n", textpos, strnum, (int)pattv[strnum].len, pattv[strnum].ptr);
    return 0;
}

int
main(int argc, char **argv)
{
    if (argc < 2 || argc > 5)
        usage("pattern_file [target_file]\n" "Creates psearch.tmp\n");

    MEMBUF patt = chomp(slurp(argv[1]));
    if (!patt.ptr)
        die("cannot read %s", argv[1]);

    int npatts;
    MEMREF *pattv = refsplit(patt.ptr, '\n', &npatts);

    double t = tick();
    PSEARCH *psp = psearch_create(pattv, npatts);
    t = tick() - t;

    plan_tests(argc < 3 ? 1 : 3);

    ok(psp, "psearch_create(pattv[%d]) compiled, in %.3f secs", npatts, t);

#if 1
    FILE *fp = fopen("dump.tmp", "w");
    psearch_dump(psp, PS_ALL, fp, pattv); // coverage!
    fclose(fp);
    fp = fopen("psearch.tmp", "w");
    psearch_save(fp, psp);
    fclose(fp);

    psearch_destroy(psp);

    fp = fopen("psearch.tmp", "r");
    psp = psearch_load(fp);
    fclose(fp);
#endif
    if (argc > 2) {
        // It's more important to get test coverage of psearch_more,
        // than to test that psearch_scan calls psearch_more correctly.
        int fd = open(argv[2], O_RDONLY);
        if (fd < 0) return fprintf(stderr, "psearch_x: %s: cannot open: %s\n", argv[2], strerror(errno));

        static char buf[1024*1024];
        MEMREF text = {buf, 0};
        int state = 0;
        double elapsed = 0, start = tick();
        matchfp = fopen("match.tmp", "w");
        while (0 < (text.len = read(fd, buf, sizeof buf))) {
            t = tick();
            (void)psearch_more(psp, text, (PSEARCH_ACTION*)on_match, pattv, &state);
            elapsed += tick() - t;
            putc('.', stderr);
        }
        putc('\n', stderr);
        close(fd);
        fclose(matchfp);
        ok(text.len == 0, "text_file loaded in 1M blocks; read took %.3f secs", tick() - start - elapsed);

        int expected = argc == 4 ? atoi(argv[3]) : actual;
        if (!ok(actual == expected, "%d matches found, in %.3f secs", expected, elapsed))
            diag("actual: %d\n", actual);
    }
    psearch_dump(psp, PS_STATS, stderr, pattv);
#ifdef STAT
    int i;
    for (i = 1; i < (int)psstat[0].val; ++i)
        if (psstat[i].val)
            fprintf(stderr, "stat %4d: %11llu %s\n", i, psstat[i].val, psstat[i].name);
#endif//STAT

    buffree(patt);
    free(pattv);
    psearch_destroy(psp);

    return exit_status();
}
