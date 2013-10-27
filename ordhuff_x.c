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
#define    MAXTEST 1000

static void prbits (char *buf, int code, int bits);

int
main (int argc, char **argv)
{
    unsigned        freq[MAXTEST], code[MAXTEST], bits[MAXTEST];
    char            zcode[33];
    int             i, ncodes;

    if (argc != 2)
        return fputs ("Usage: ordhuff_x (freqfile | -)\n", stderr);

    FILE           *finp =
        strcmp (argv[1], "-") ? fopen (argv[1], "r") : stdin;

    if (!finp)
        return 1;

    for (ncodes = 0; 
         ncodes < MAXTEST && fscanf (finp, "%ud", &freq[ncodes]) == 1; 
        ++ncodes);

    ordhuff(ncodes, freq, code, bits);

    unsigned        raw = 0, pkd = 0;

    for (i = 0; i < ncodes; ++i) {
        prbits(zcode, (int) code[i], (int) bits[i]);
        printf ("%4d %8d %s\n", i, freq[i], zcode);
        raw += freq[i];
        pkd += freq[i] * strlen (zcode);
    }

    fprintf (stderr, "Exprevbits: %4.2f\n", (float) pkd / raw);
    return 0;
}

static void
prbits (char *buf, int code, int bits)
{
    for (buf[bits] = 0; bits-- > 0; code >>= 1)
        buf[bits] = code & 1 ? '1' : '0';
}
