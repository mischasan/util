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
static char *Railgun7a(char *tgt, int tgtlen, char *pat, int patlen);
static char *Railgun7g(char *pbTarget, int cbTarget, char *pbPattern, int cbPattern);
static char *Railgun7h(char *pbTarget, int cbTarget, char *pbPattern, int cbPattern);
static char *Railgun7s(char *pbTarget, int cbTarget, char *pbPattern, int cbPattern);

#define sse2str scanstr

int
main(void)
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    int         count = 10001;  // count MUST be odd, or hash is always zero!
    int         tgtlen, patlen, m, t, p, i, r;
    intptr_t    hash = 0;
    int         tv[] = { 65536, 32768, 16384, 8192, 4096, 2048, 1024,
                         961, 960, 959, 512, 256, 128, 64, 32, 16, 8, 0 };
    int         pv[] = { 258, 257, 256, 255, 254, 130, 129, 128, 127, 126,
                         98, 97, 96, 95, 94, 66,  65,  64,  63,  62, 
                         50, 49, 48, 47, 46, 34,  33,  32,  31,  30,
                         18,  17,  16,  15,  14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 0 };
    char        pat[256 + 2 + 1], tgt[99999];

    for (m = 5; m >= -1; m--) {
        //printf("M TARG. PATT | SSE2 BERG CHRS WINS BNDM | RG7a RG7g RG7h best\n");
        printf("M TARG. PATT | SSE2 BERG WINS BNDM | RG7a RG7g RG7h RG7s best\n");
        for (t = 0; (tgtlen = tv[t]); ++t) {
            for (p = 0; (patlen = pv[p]); ++p) {
                if (patlen >= tgtlen || patlen <= m) continue;

                memcpy(pat, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,abcdefghijklmnopqrstuvwxyz.AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0918273645.5463728190zZyYxXwWvVuUtTsSrRqQpPoOnNmMlLkKjJiIhHgGfFeEdDcCbBaA.zyxwvutsrqponmlkjihgfedcba,9876543210ZYXWVUTSRQPONMLKJIHGFEDCBA,@ACEGIKMOQSUW", patlen);
                pat[patlen] = 0;
                if  (m < 0) memset(tgt, pat[patlen - 1], tgtlen);
                else if (m) for (i = 0; i < tgtlen; i += m) memcpy(tgt+i, "ABCDEFGHIJK", m);
                else memset(tgt, '-', tgtlen);
                strcpy(tgt+tgtlen, pat);
                // The "hash ^=" is necessary, else gcc OPTIMIZES OUT the strstr calls.
#               define DO_MEM(T,F) \
                            double T = tick(); \
                            for (r = 1; r < 6553600/count; ++r, tgt[1] ^= 1) \
                                hash ^= (intptr_t)F(tgt, tgtlen+patlen, pat, patlen); \
                            T = tick() - T; if (F == bndmem && patlen > 128) T = 0
#               define DO_STR(T,F) \
                            double T = tick(); \
                            for (r = 1; r < 6553600/count; ++r, tgt[1] ^= 1) \
                                hash ^= (intptr_t)F(tgt, pat); \
                            T = tick() - T; 
                DO_STR(tbase, strstr);  // sets exp[]
                DO_STR(tsse2, sse2str);
                DO_STR(tberg, bergstr);
//                DO_STR(tchrs, strchrs);
                DO_STR(twins, winstr);
                DO_MEM(tbndm, bndmem);
                DO_MEM(trg7a, Railgun7a);
                DO_MEM(trg7g, Railgun7g);
                DO_MEM(trg7h, Railgun7h);
                DO_MEM(trg7s, Railgun7s);
#                   undef  DO_MEM
#                   undef  DO_STR

                //double *timev[] =    { &tbase, &tsse2, &tberg, &tchrs, &twins, &tbndm, &trg7a, &trg7g, &trg7h, NULL };
                //char const *namev[] = { "base", "sse2", "berg", "chrs", "wins", "bndm", "elsi", "gull", "hash", NULL };
                double *timev[] =    { &tbase, &tsse2, &tberg, &twins, &tbndm, &trg7a, &trg7g, &trg7h, &trg7s, NULL };
                char const *namev[] = { "base", "sse2", "berg", "wins", "bndm", "elsi", "gull", "hash", "seki", NULL };
                int b = 0;
                for (i = 1; timev[i]; ++i) if (*timev[i] && *timev[b] > *timev[i]) b = i;
                
                //printf("%1d %5d %4d | %4.0f %4.0f %4.0f %4.0f %4.0f | %4.0f %4.0f %4.0f %s%s",
                printf("%1d %5d %4d | %4.0f %4.0f %4.0f %4.0f | %4.0f %4.0f %4.0f %4.0f %s%s",
                       m, tgtlen, patlen,
                       tsse2 * 100 / tbase,
                       tberg * 100 / tbase,
                       //tchrs * 100 / tbase,
                       twins * 100 / tbase,
                       tbndm * 100 / tbase,
                       trg7a * 100 / tbase,
                       trg7g * 100 / tbase,
                       trg7h * 100 / tbase,
                       trg7s * 100 / tbase,
                        (int)hash & 1 ? "FAIL " : "", namev[b]);

                for (i = 0; timev[i]; ++i)
                    if (i != b && *timev[i] && *timev[i] < 1.1 * *timev[b])
                        printf(",%s", namev[i]);

                putchar('\n');
            }
        }
    }

    return 0;
}

// STR_X blocks some duplicate declarations.
#define  STR_X 1
#include "bergstr.c"
#include "bndmem.c"
#include "railgun7a.c"
#include "railgun7g.c"
#include "railgun7h.c"
#include "railgun7s.c"
#include "winstr.c"
