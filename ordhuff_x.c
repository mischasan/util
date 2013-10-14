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
