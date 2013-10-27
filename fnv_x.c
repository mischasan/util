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
#include <ctype.h>
#include <xmmintrin.h>

static void dump(char const xm[16], char const *label);

int
main(int argc, char const **argv)
{
    if (argc != 2)
        return fputs("Usage: fnv_x (keyfile | -)\n", stderr);

    FILE    *fp = strcmp(argv[1], "-") ? fopen(argv[1], "r") : stdin;
    if (!fp)
        die(": %s: cannot open", argv[1]);

    char    buf[999];
    while (fgets(buf, sizeof(buf), fp)) {
        char    *cp = strim(buf);
        int     len = strlen(cp);
        printf("%08X %16"F64"u", fnv04(cp, len), fnv08(cp, len));

        char    hash[16];
	fnv16(cp, len, hash);
	dump(hash, buf);
    }

    fclose(fp);

    return  0;
}

static void
dump(char const *xm, char const *label)
{
    char const *cp = &xm[16];

    do printf("%02X ", 255 & *--cp); while (cp != xm);

    printf(" %s\n", label);
}
