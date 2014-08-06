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
#include "tap.h"
#include "psearch.h"

static int nmatches = 0;
static int on_match(int,int,void*);

int
main(int argc, char **argv)
{
    if (argc != 3) die(": requires args: psearch_file text_file");

    plan_tests(2);

    FILE *pfp = fopen(argv[1], "r");
    if (!pfp) die(": unable to open %s:", argv[1]);

    MEMREF text = bufref(slurp(argv[2]));
    if (nilref(text)) die(": unable to load %s:", argv[2]);

    PSEARCH *psp = psearch_mmap(pfp);
    ok(psp, "psearch_mmap returned");
    fclose(pfp);
    double t0 = tick();
    ok(!psearch_scan(psp, text, on_match, NULL),
        "mmap-ed psearch object works");

    fprintf(stderr, "# nmatches: %d %.4f secs\n", nmatches, tick() - t0);
    psearch_dump(psp, PS_STATS, stderr, NULL);
    psearch_destroy(psp);

    return exit_status();
}

static int
on_match(int s, int t, void *c)
{
    (void)s, (void)t, (void)c;
    ++nmatches;
    return 0;
}
