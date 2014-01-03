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

static char *bergstr(char *phaystack, char *pneedle);
extern char *strchrs(char *tgt, char *pat);
static char *winstr(char *tgt, char *pat);
//static char *Railgun7a(char *tgt, int tgtlen, char *pat, int patlen);
static char *Railgun7g(char *pbTarget, int cbTarget, char *pbPattern, int cbPattern);
static char *Railgun7h(char *pbTarget, int cbTarget, char *pbPattern, int cbPattern);
static char *Railgun7s(char *pbTarget, int cbTarget, char *pbPattern, int cbPattern);
static char *Railgun7w(char *pbTarget, int cbTarget, char *pbPattern, int cbPattern);

#define sse2str scanstr

int
main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    int         count = 10001;  // count MUST be odd, or hash is always zero!
    int         tgtlen, patlen, m, t, p, i, r;
    intptr_t    hash = 0, exp = 0;
    int         tv[] = { 65536, 32768, 16384, 8192, 4096, 2048, 1024,
                         962, 961, 960, 512, 256, 128, 64, 32, 16, 8, 0 };
    int         pv[] = { 1025, 1024, 1023, 
                         714, 713, 712, 711, 258, 
                         257, 256, 255, 254, 
                         130, 129, 128, 127, 126,
                         98, 97, 96, 95, 94, 
                         66,  65,  64,  63,  62, 
                         50, 49, 48, 47, 46, 34,  33,  32,  31,  30, 23, 22, 21,
                         18,  17,  16,  15,  14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 0 };
    char        pat[9999], tgt[99999];
    int         lo, hi;
    switch (argc) {
    case 1: lo = -(hi = 4); break;
    case 2: lo = -(hi = atoi(argv[1])); break;
    case 3: lo = atoi(argv[1]), hi = atoi(argv[2]); break;
    default: die("Usage: stress <limit>  or  stress <min> <max>");
    }

    for (m = lo; m <= hi; m++) {
        printf("M TARG. PATT | SSE2 BERG WINS BNDM | RG7g RG7h RG7s RG7w best winner(s)\n");
        for (t = 0; (tgtlen = tv[t]); ++t) {
            for (p = 0; (patlen = pv[p]); ++p) {
                if (patlen >= tgtlen || patlen <= m || patlen <= -m) continue;

                char c258[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,abcdefghijklmnopqrstuvwxyz.AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0918273645.5463728190zZyYxXwWvVuUtTsSrRqQpPoOnNmMlLkKjJiIhHgGfFeEdDcCbBaA.zyxwvutsrqponmlkjihgfedcba,9876543210ZYXWVUTSRQPONMLKJIHGFEDCBA,@ACEGIKMOQSUW";
                if (patlen < (int)sizeof c258) {
                    memcpy(pat, c258, patlen);
                } else {
                    strcpy(pat, c258);
                    char flip = 3, *cp;
                    while ((int)strlen(pat) < patlen) {
                        for (cp = c258; *cp; cp++) *cp ^= flip;
                        strcat(pat, c258);
                        flip <<= 1;
                    }
                }
                pat[patlen] = 0;

                if  (m < 0) for (i = 0; i < tgtlen; i -= m) memcpy(tgt+i, &pat[patlen+m], -m);
                else if (m) for (i = 0; i < tgtlen; i += m) memcpy(tgt+i, pat, m);
                else memset(tgt, '-', tgtlen);
                strcpy(tgt+tgtlen, pat);
                // The "hash ^=" is necessary, else gcc OPTIMIZES OUT the strstr calls.
#               define DO_MEM(T,F) \
                            double T = tick(); \
                            for (r = 1; r < 6553600/count; ++r, tgt[1] ^= 1) \
                                hash ^= (intptr_t)F(tgt, tgtlen+patlen, pat, patlen); \
                            T = tick() - T; if (F == bndmem && patlen > 128) T = 0; \
                            if (exp && hash != exp) fprintf(stderr, #F "(%d,%d) failed\n", tgtlen,patlen)
#               define DO_STR(T,F) \
                            double T = tick(); \
                            for (r = 1; r < 6553600/count; ++r, tgt[1] ^= 1) \
                                hash ^= (intptr_t)F(tgt, pat); \
                            T = tick() - T
                DO_STR(tbase, strstr);  // sets exp[]
                exp = hash;
                DO_STR(tsse2, sse2str);
                DO_STR(tberg, bergstr);
//                DO_STR(tchrs, strchrs);
                DO_STR(twins, winstr);
                DO_MEM(tbndm, bndmem);
                DO_MEM(trg7g, Railgun7g);
                DO_MEM(trg7h, Railgun7h);
                DO_MEM(trg7s, Railgun7s);
                DO_MEM(trg7w, Railgun7w);
#                   undef  DO_MEM
#                   undef  DO_STR

                double *timev[] =    { &tbase, &tsse2, &tberg, &twins, &tbndm, &trg7g, &trg7h, &trg7s, &trg7w, NULL };
                char const *namev[] = { "base", "sse2", "berg", "wins", "bndm", "gull", "hash", "seki", "wolf", NULL };
                int b = 0;
                for (i = 1; timev[i]; ++i) if (*timev[i] && *timev[b] > *timev[i]) b = i;
                
                printf("%1d %5d %4d | %4.0f %4.0f %4.0f %4.0f | %4.0f %4.0f %4.0f %4.0f %4.0f %s",
                       m, tgtlen, patlen,
                       tsse2 * 100 / tbase,
                       tberg * 100 / tbase,
                       twins * 100 / tbase,
                       tbndm * 100 / tbase,
                       trg7g * 100 / tbase,
                       trg7h * 100 / tbase,
                       trg7s * 100 / tbase,
                       trg7w * 100 / tbase,
                       *timev[b] * 100 / tbase,
                       namev[b]);

                for (i = 0; timev[i]; ++i)
                    if (i != b && *timev[i] && *timev[i] < 1.1 * *timev[b])
                        printf(",%s", namev[i]);

                putchar('\n');
            }
        }
    }

    return 0;
}

// STRESS_C blocks some duplicate declarations.
#define  STRESS_C 1
#include "bergstr.c"
#include "bndmem.c"
#include "railgun7g.c"
#include "railgun7h.c"
#include "railgun7s.c"
#include "railgun7w.c"
#include "winstr.c"
