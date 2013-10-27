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

int main(int argc, char **argv)
{
    if (argc < 4) die("Usage: brk_x <file> <count> <brk>...");
    MEMBUF      x = chomp(slurp(argv[1]));
    if (nilbuf(x)) die(": cannot read %s:", argv[1]);
    int         count = atoi(argv[2]);
    int         nr;
    MEMREF      *rv = refsplit(x.ptr, '\n', &nr);
    double      tstr = 0, tscn = 0;
    int         xstr = 0, xscn = 0;
    int         a, i, j;

    for (a = 3; a < argc; ++a)
        for (i = 0; i < nr; ++i)
            if (strpbrk(rv[i].ptr, argv[a]) != scanbrk(rv[i].ptr, argv[a]))
                die("failed on brk('%s',[%s])", rv[i].ptr, argv[a]);

    for (a = 3; a < argc; ++a) {
        char    *cp = argv[a];

        double  t0 = tick();
        for (j = 0; j < count; ++j)
            for (i = 0; i < nr; ++i)
                xstr += strpbrk(rv[i].ptr, cp) - cp;
        tstr += tick() - t0;

        t0 = tick();
        for (j = 0; j < count; ++j)
            for (i = 0; i < nr; ++i)
                xscn += scanbrk(rv[i].ptr, cp) - cp;
        tscn += tick() - t0;
    }

    printf("nbrks=%d nchrs=%"FSIZE"u scanbrk:%s%.2f %.0f%%\n",
            nr, x.len, xscn==xstr?"":"FAIL:", tscn, 100 * tscn / tstr);
    free(rv);
    buffree(x);
    return xscn != xstr;
}
