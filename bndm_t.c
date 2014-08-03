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

#include <tap.h>
#include "msutil.h"

int main(int argc, char **argv)
{
    plan_tests(1);
    char dflt[] = "one", *patt = argc > 1 ? argv[1] : dflt;
    char const *util = getenv("util") ? getenv("util") : ".";
    char const *file = argc > 1 ? argv[1] : "words";
    FILE *fp = fopenf("r", "%s/%s", util, file);
    if (!fp) return fprintf(stderr, "bndm_t: cannot open %s\n", file);
    int patlen = strlen(patt), lines = 0, goods = 0, hits = 0;
    char buf[999];

    while (fgets(buf, sizeof buf, fp)) {
        lines++;
        char *cp = strstr(buf, patt);
        hits += !!cp;
        if (cp == bndmem(buf, strlen(buf) - 1, patt, patlen)) ++goods;
        else fprintf(stderr, "# failed: %s", buf);
    }
    ok(goods == lines, "strstr == bndmem %d/%d times (%d hits)", goods, lines, hits);

    return exit_status();
}
