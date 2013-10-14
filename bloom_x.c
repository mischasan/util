#include <ctype.h>      // isxdigit
#include <stdio.h>
#include <stdlib.h>     // atoi
#include <string.h>

#include "bloom.h"

static int xdigit[] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0,  // '0'..'9', ':'..'?'
    0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // '@', 'A'..'F', 'G'..'O'
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 'P'..'_'
    0, 10, 11, 12, 13, 14, 15                                       // '`', 'a'..'f'
};

static int gethex(char const *inpstr, char *outhash, int outsize);
 
int main(int argc, char **argv)
{
    if (argc < 5 || argc > 6 || (argc == 6 && !strcmp(argv[5], "-") && !strcmp(argv[6], "-")))
        return fputs("USAGE: bloom_x hashwid npoints pointwid hashes [tests]\n"
                     "PARAMETERS:\n"
                     "\thashwid:  # of leading bits of each (hashfile) line to use\n"
                     "\tnpoints:  size of bloom filter\n"
                     "\tpointwid: bits per point in bloom filter (e.g. 2..8)\n"
                     "\thashes:   a file of hex hashes\n"
                     "\ttests:    a file of input hashes to test against the filter\n"
                     "hashes or tests (but not both) may be '-' meaning stdin\n"
                    ,stderr);
     
    int nlines, hashbits = atoi(argv[1]), hashbytes = (hashbits + 7) / 8;
    BLOOM *bp = bloom_create(hashbits, atoi(argv[2]), atoi(argv[3]));

    FILE *fp = strcmp(argv[4], "-") ? fopen(argv[4], "r") : stdin;
    char buf[99];

    for (nlines = 1; fgets(buf, sizeof buf, fp); nlines++) {
        char hash[hashbytes];
        if (gethex(buf, hash, hashbytes) < hashbytes)
            return fprintf(stderr, "bloom_x: bad hashes line[%d]: %s", nlines, buf);
        bloom_add(bp, hash);
    }
    fclose(fp);

    fprintf(stderr, "stat:%d over:%d\n", bloom_stat(bp), bloom_over(bp));
    //bloom_dump(bp, stdout);

    if (argc > 5) {
        fp =  strcmp(argv[5], "-") ? fopen(argv[5], "r") : stdin;
        for (nlines = 1; fgets(buf, sizeof buf, fp); nlines++) {
            char hash[hashbytes];
            if (gethex(buf, hash, hashbytes) < hashbytes)
                return fprintf(stderr, "bloom_x: bad test line[%d]: %s", nlines, buf);
            printf("%d\t%s", bloom_chk(bp, hash), buf);
        }
    }
    
    bloom_destroy(bp);
    return 0;
}

static int gethex(char const *inp, char *hash, int outsize)
{
    int  i = 0;
    for (; i < outsize && isxdigit(inp[0]) && isxdigit(inp[1]); inp += 2)
        hash[i++] = xdigit[inp[0] - '0'] * 16 + xdigit[inp[1] - '0'];
    return i;
}
