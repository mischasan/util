#ifndef STRESS_C
#include <stdint.h>
#include <string.h>
char *Railgun7g(char *tgt, int tgtlen, char *pat, int patlen);
#endif
char *
Railgun7g(char *pbTarget, int cbTarget, char *pbPattern, int cbPattern)
{
    char *pbTargetMax = pbTarget + cbTarget;
    register unsigned long ulHashPattern;
    register unsigned long ulHashTarget;
    signed long count;
    signed long countSTATIC;

    unsigned char SINGLET;
    unsigned long Quadruplet2nd;
    unsigned long Quadruplet3rd;
    unsigned long Quadruplet4th;

    int         AdvanceHopperGrass;

    long i;                     //BMH needed
    int a, j;
    //UNUSED unsigned int bm_bc[256];    //BMH needed
    //UNUSED unsigned int bm_bc2nd[256]; //BMS needed
    unsigned char bm_Sunday_Order2[256 * 256];  //BMHSS(Elsiane) needed, 'char' limits patterns to 255, if 'long' then table becomes 256KB, grrr.
    int     Gulliver;     // or unsigned char or unsigned short

    if (cbPattern > cbTarget)
        return (NULL);

    if (cbPattern < 4) {
        pbTarget = pbTarget + cbPattern;
        ulHashPattern =
            ((*(char *) (pbPattern)) << 8) + *(pbPattern + (cbPattern - 1));

        if (cbPattern == 3) {
            for (;;) {
                if (ulHashPattern == (uint32_t)
                    ((*(uint8_t const *) (pbTarget - 3)) << 8) + *(uint8_t const*)(pbTarget - 1)) {
                    if (*(uint8_t const*) (pbPattern + 1) == *(uint8_t const *) (pbTarget - 2))
                        return ((pbTarget - 3));
                }
                if ((char) (ulHashPattern >> 8) != *(pbTarget - 2))
                    pbTarget++;
                pbTarget++;
                if (pbTarget > pbTargetMax)
                    return (NULL);
            }
        }
        else {
        }
        for (;;) {
            if (ulHashPattern == (uint32_t)
                ((*(uint8_t const*) (pbTarget - 2)) << 8) + *(uint8_t const*)(pbTarget - 1))
                return((pbTarget-2));
            if ((char) (ulHashPattern >> 8) != *(pbTarget - 1))
                pbTarget++;
            pbTarget++;
            if (pbTarget > pbTargetMax)
                return (NULL);
        }
    }
    else {                      //if ( cbPattern<4)
        if (cbTarget < 961) {   // This value is arbitrary(don't know how exactly), it ensures(at least must) better performance than 'Boyer_Moore_Horspool'.
            pbTarget = pbTarget + cbPattern;
            ulHashPattern = *(unsigned long *) (pbPattern);

            SINGLET = ulHashPattern & 0xFF;
            Quadruplet2nd = SINGLET << 8;
            Quadruplet3rd = SINGLET << 16;
            Quadruplet4th = SINGLET << 24;

            for (;;) {
                AdvanceHopperGrass = 0;
                ulHashTarget = *(unsigned long *) (pbTarget - cbPattern);

                if (ulHashPattern == ulHashTarget) {    // Three unnecessary comparisons here, but 'AdvanceHopperGrass' must be calculated - it has a higher priority.
                    count = cbPattern - 1;
                    while (count
                           && *(char *) (pbPattern + (cbPattern - count)) ==
                           *(char *) (pbTarget - count)) {
                        if (cbPattern - 1 == AdvanceHopperGrass + count
                            && SINGLET != *(char *) (pbTarget - count))
                            AdvanceHopperGrass++;
                        count--;
                    }
                    if (count == 0)
                        return((pbTarget-cbPattern));
                }
                else {          // The goal here: to avoid memory accesses by stressing the registers.
                    if (Quadruplet2nd != (ulHashTarget & 0x0000FF00)) {
                        AdvanceHopperGrass++;
                        if (Quadruplet3rd != (ulHashTarget & 0x00FF0000)) {
                            AdvanceHopperGrass++;
                            if (Quadruplet4th != (ulHashTarget & 0xFF000000))
                                AdvanceHopperGrass++;
                        }
                    }
                }

                AdvanceHopperGrass++;

                pbTarget = pbTarget + AdvanceHopperGrass;
                if (pbTarget > pbTargetMax)
                    return (NULL);
            }
        }
        else {                  //if (cbTarget<961)
            countSTATIC = cbPattern - 2 - 2;

            //UNUSED for (a = 0; a < 256; a++) bm_bc[a] = cbPattern, bm_bc2nd[a] = cbPattern + 1;
            //for (j = 0; j < cbPattern - 1; j++) bm_bc[(uint8_t)pbPattern[j]] = cbPattern - j - 1;
            //for (j = 0; j < cbPattern; j++) bm_bc2nd[(uint8_t)pbPattern[j]] = cbPattern - j;

            // Elsiane r.2  [
            for (a = 0; a < 256 * 256; a++) {
                bm_Sunday_Order2[a] = cbPattern - 1;
            }                   // 'memset' if not optimized

            // alfalfa 7 long 6 BBs (al lf fa al lf fa) 3 distinct BBs (al lf fa) 
            // fast 4 0-1-2 fa as st
            for (j = 0; j < cbPattern - 1; j++)
                bm_Sunday_Order2[*(unsigned short *) (pbPattern + j)] = j;  // Rightmost appearance/position is needed

            // Elsiane r.2 ]

            ulHashPattern = *(unsigned short *) (pbPattern + cbPattern - 1 - 1);    // Last two bytes
            ulHashTarget = *(unsigned long *) (pbPattern);  // First four bytes

            AdvanceHopperGrass = 0;
            i = 0;
            while (i <= cbTarget - cbPattern - 1 - 1) {
                Gulliver =
                    bm_Sunday_Order2[*(unsigned short *)
                                     &pbTarget[i + cbPattern - 1 - 1]];

                if (Gulliver == cbPattern - 2) {    // CASE #1: means the pair (char order 2) is found
                    if (*(unsigned long *) &pbTarget[i] == ulHashTarget) {
                        count = countSTATIC;    // Last two chars already matched, to be fixed with -2
                        while (count != 0
                               && *(char *) (pbPattern + (countSTATIC - count) +
                                             4) ==
                               *(char *) (&pbTarget[i] + (countSTATIC - count) +
                                          4))
                            count--;
                        if (count == 0)
                            return(pbTarget+i);
                    }
                    i = i + 1;
                }
                else if (Gulliver == cbPattern - 1) // CASE #2: means the pair (char order 2) is found
                    i = i + Gulliver;   // the pair is not found, skip the whole pattern and fall back one char
                else
                    i = i + cbPattern - Gulliver - 2;   // CASE #3: the pair is found and not as suffix i.e. rightmost position

// 32323218 Order 1 Horspool
// fa af fa af fa as st Order 2 Horspool
//  0  1  2  3  4  5  6
// HIKARIfast
// fafafast
//   fafafast +2 Order 1 'a' vs 't'
//   fafafast +2 = (cbPattern-Gulliver-2 = 8-4-2 = 2) Order 2 'fa' vs 'st' i.e. CASE #3

// 76543218 Order 1 Horspool
// lo on ng gp pa ac ce Order 2 Horspool
//  0  1  2  3  4  5  6
// HIKARIfast
// longpace
//   longpace +2 Order 1 'a' vs 'e'
//        longpace +7 = (cbPattern-1 = 8-1 = 7) Order 2 'fa' vs 'ce' i.e. CASE #2

                AdvanceHopperGrass++;
            }

            if (i == cbTarget - cbPattern - 1) {
                if (*(unsigned long *) &pbTarget[i] == ulHashPattern) {
                    count = countSTATIC;
                    while (count != 0
                           && *(char *) (pbPattern + (countSTATIC - count) +
                                         4) ==
                           *(char *) (&pbTarget[i] + (countSTATIC - count) + 4))
                        count--;
                    if (count == 0)
                        return(pbTarget+i);
                }
                i++;
                AdvanceHopperGrass++;
            }

            if (i == cbTarget - cbPattern) {
                if (*(unsigned long *) &pbTarget[i] == ulHashPattern) {
                    count = countSTATIC;
                    while (count != 0
                           && *(char *) (pbPattern + (countSTATIC - count) +
                                         4) ==
                           *(char *) (&pbTarget[i] + (countSTATIC - count) + 4))
                        count--;
                    if (count == 0)
                        return(pbTarget+i);
                }
                AdvanceHopperGrass++;
            }
#if 0
            GlobalSP += (int) ((double) cbTarget / AdvanceHopperGrass * 100);
            GlobalI += AdvanceHopperGrass;
            printf
                ("Skip-Performance(bigger-the-better): %d%%, %d skips/iterations\n",
                 (int) ((double) cbTarget / AdvanceHopperGrass * 100),
                 AdvanceHopperGrass);
#endif
            return (NULL);
        }                       //if (cbTarget<961)
    }                           //if ( cbPattern<4)
}

// ### Mix(2in1) of Karp-Rabin & Boyer-Moore-Sunday-Horspool algorithm ]
