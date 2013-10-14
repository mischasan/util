// From www.sanmayce.com (Georgi Kaze?)
// Test harness for "railgun" algorithms 6+++ (sic) and 7.
// 2012-01-11 mischasan Modifications to run on x64.
// Revision 2:
/*
DANNII MINOGUE:
Where do we go now?
I don't know
Innocence over
Fading fast
...
You're still promising perfection, perfection
With empty words
With empty words
With empty words
With empty words
And it's hard to break a habit
You're lost inside it
...
A moment of coldness
Cuts through me(cuts through me)
I've tried to remember
Why I don't leave(I don't leave)
And you're the cause of my confusion
Closing down the way I feel
How come I don't see so clearly, so clearly
...
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

long  HORSPOOL(char*y, char *x, long n, int m);
long  HORSPOOL_hits(char*y, char *x, long n, int m);
long  Boyer_Moore_Horspool_Kaze(char*y, char *x, long n, int m);
long  Brute_Force_Dummy(char*y, char *x, long n, int m);
long  Karp_Rabin(char*y, char *x, long n, int m);

char* KarpRabinKaze(char *pbTarget, char *pbPattern, unsigned long cbTarget, unsigned long cbPattern);
char* Railgun_Quadruplet(char *pbTarget, char *pbPattern, unsigned long cbTarget, unsigned long cbPattern);
char* Railgun_Quadruplet_6pp(char *pbTarget, char *pbPattern, unsigned long cbTarget, unsigned long cbPattern);
char* Railgun_Quadruplet_6pp_count_hits(char *pbTarget, char *pbPattern, unsigned long cbTarget, unsigned long cbPattern);
char* Railgun(char *pbTarget, char *pbPattern, unsigned long cbTarget, unsigned long cbPattern);
char* Railgun_totalhits(char *pbTarget, char *pbPattern, unsigned long cbTarget, unsigned long cbPattern);
char* KarpRabinKaze_BOOSTED(char *pbTarget, char *pbPattern, unsigned long cbTarget, unsigned long cbPattern);
char* strstr_Microsoft(const char *str1, const char *str2);
char* strstr_GNU_C_Library(char const *phaystack, char const *pneedle);

clock_t clocks1, clocks2;
clock_t clocks3, clocks4;
double TotalRoughSearchTime = 0;

long Railgunhits = 0;

#define ASIZE 256

// ### Boyer-Moore-Horspool algorithm [
long
HORSPOOL(char *y, char *x, long n, int m)
{
    long i;
    int a, j, bm_bc[ASIZE];
    unsigned char ch, lastch;

    /* Preprocessing */
    for (a = 0; a < ASIZE; a++)
        bm_bc[a] = m;
    for (j = 0; j < m - 1; j++)
        bm_bc[(unsigned char)x[j]] = m - j - 1;

    /* Searching */
    lastch = x[m - 1];
    i = 0;
    while (i <= n - m) {
        ch = y[i + m - 1];
        if (ch == lastch)
            //if (memcmp(&y[i],x,m-1) == 0) OUTPUT(i);
            if (memcmp (&y[i], x, m - 1) == 0)
                return (i);
        i += bm_bc[ch];
    }
    return (-1);
}

long  
HORSPOOL_hits(char*y, char *x, long n, int m)
{
    long i;
    int a, j, bm_bc[ASIZE];
    unsigned char ch, lastch;

    /* Preprocessing */
    for (a = 0; a < ASIZE; a++)
        bm_bc[a] = m;
    for (j = 0; j < m - 1; j++)
        bm_bc[(unsigned char)x[j]] = m - j - 1;

    /* Searching */
    lastch = x[m - 1];
    i = 0;
    while (i <= n - m) {
        ch = y[i + m - 1];
        if (ch == lastch)
            //if (memcmp(&y[i],x,m-1) == 0) OUTPUT(i);
            //if (memcmp(&y[i],x,m-1) == 0) return(i);
            if (memcmp (&y[i], x, m - 1) == 0)
                Railgunhits++;
        i += bm_bc[ch];
    }
    return (-1);
}

long
Boyer_Moore_Horspool_Kaze(char *y, char *x, long n, int m)
{
    long i;
    int a, j, bm_bc[ASIZE];
    unsigned char ch;
    unsigned char lastch;
    unsigned char firstch;

    unsigned long count;
    unsigned long countSTATIC;

    /* Preprocessing */
    for (a = 0; a < ASIZE; a++)
        bm_bc[a] = m;
    for (j = 0; j < m - 1; j++)
        bm_bc[(unsigned char)x[j]] = m - j - 1;

    /* Searching */
    lastch = x[m - 1];
    firstch = x[0];
    i = 0;
    countSTATIC = m - 2;
    while (i <= n - m) {
        ch = y[i + m - 1];
        //if (ch ==lastch)
        //if (memcmp(&y[i],x,m-1) == 0) OUTPUT(i);
// Below line gives: 315KB/clock
        //if (ch ==lastch && y[i] == firstch && memcmp(&y[i],x,m-1) == 0) return(i);  // Kaze: The idea(to prevent execution of slower 'memcmp') is borrowed from Karp-Rabin i.e. to perform a slower check only when the target "looks like".
// Below line gives: 328KB/clock
//          if (ch == lastch && y[i] == firstch && memcmp(&y[i+1],&x[0+1],m-1-1) == 0) return(i);  // Kaze: The idea(to prevent execution of slower 'memcmp') is borrowed from Karp-Rabin i.e. to perform a slower check only when the target "looks like".

        if (ch == lastch && y[i] == firstch) {
            count = countSTATIC;
            while (count
                   && *(char *) (x + 1 + (countSTATIC - count)) ==
                   *(char *) (&y[i] + 1 + (countSTATIC - count))) {
                count--;
            }
            if (count == 0)
                return (i);
        }


        i += bm_bc[ch];
    }
    return (-1);
}

// ### Boyer-Moore-Horspool algorithm ]


// ### Brute force 'Dummy' algorithm [
long
Brute_Force_Dummy(char *y, char *x, long n, int m)
{
    long i, j;

    /* Searching */
    for (i = 0; i <= n - m; i++) {
        j = 0;
        while (j < m && y[i + j] == x[j])
            j++;
        if (j >= m)
            return (i);
    }
    return (-1);
}

// ### Brute force 'Dummy' algorithm ]


// ### Karp-Rabin algorithm [
#define REHASH(a, b, h) ((((h) - (a)*d) << 1) + (b))
long
Karp_Rabin(char *y, char *x, long n, int m)
{
    int d, hx, hy, i, j;

    /* Preprocessing */
    /* computes d = 2^(m-1) with
       the left-shift operator */
    for (d = i = 1; i < m; ++i)
        d = (d << 1);

    for (hy = hx = i = 0; i < m; ++i) {
        hx = ((hx << 1) + x[i]);
        hy = ((hy << 1) + y[i]);
    }

    /* Searching */
    j = 0;
    while (j <= n - m) {
        if (hx == hy && memcmp (x, y + j, m) == 0)
            return (j);
        hy = REHASH (y[j], y[j + m], hy);
        ++j;
    }
    return (-1);
}

// ### Karp-Rabin algorithm ]


// ### Karp-Rabin-Kaze algorithm [
char *
KarpRabinKaze(char *pbTarget,
               char *pbPattern, unsigned long cbTarget, unsigned long cbPattern)
{
    //MSS unsigned int i;
    char *pbTargetMax = pbTarget + cbTarget;
    char *pbPatternMax = pbPattern + cbPattern;
    //MSS unsigned long ulBaseToPowerMod = 1;
    register unsigned long ulHashPattern = 0;
    unsigned long ulHashTarget = 0;
    //MSS long hits = 0;
//unsigned long count;
    //char *  buf1;
    //char *  buf2;

    if (cbPattern > cbTarget)
        return (NULL);

    // Compute the power of the left most character in base ulBase
    //for (i = 1; i < cbPattern; i++) ulBaseToPowerMod = (ulBase * ulBaseToPowerMod);

    // Calculate the hash function for the src (and the first dst)
    while (pbPattern < pbPatternMax) {
        // Below lines give 366KB/clock for 'underdog':
        //ulHashPattern = (ulHashPattern*ulBase + *pbPattern);
        //ulHashTarget = (ulHashTarget*ulBase + *pbTarget);
        pbPattern++;
        pbTarget++;
    }
    // Below lines give 436KB/clock for 'underdog' + requirement pattern to be 4 chars min.:
    //ulHashPattern = ( (*(long *)(pbPattern-cbPattern)) & 0xffffff00 ) + *(pbPattern-1);
    //ulHashTarget = ( (*(long *)(pbTarget-cbPattern)) & 0xffffff00 ) + *(pbTarget-1);
    // Below lines give 482KB/clock for 'underdog' + requirement pattern to be 2 chars min.:
    //ulHashPattern = ( (*(unsigned short *)(pbPattern-cbPattern)) | *(pbPattern-1) );
    //ulHashTarget = ( (*(unsigned short *)(pbTarget-cbPattern)) | *(pbTarget-1) );
    // Below lines give 482KB/clock for 'underdog' + requirement pattern to be 2 chars min.:
    //ulHashPattern = ( (*(unsigned short *)(pbPattern-cbPattern)) & 0xff00 ) + *(pbPattern-1);
    //ulHashTarget = ( (*(unsigned short *)(pbTarget-cbPattern)) & 0xff00 ) + *(pbTarget-1);
    // Below lines give 605KB/clock for 'underdog' + requirement pattern to be 2 chars min.:
    //ulHashPattern = ( (*(unsigned short *)(pbPattern-cbPattern))<<8 ) + *(pbPattern-1);
    //ulHashTarget = ( (*(unsigned short *)(pbTarget-cbPattern))<<8 ) + *(pbTarget-1);
    // Below lines give 668KB/clock for 'underdog':
    ulHashPattern =
        ((*(char *) (pbPattern - cbPattern)) << 8) + *(pbPattern - 1);
    ulHashTarget = ((*(char *) (pbTarget - cbPattern)) << 8) + *(pbTarget - 1);

    // Dynamically produce hash values for the string as we go
    for (;;) {
        if ((ulHashPattern == ulHashTarget)
            && !memcmp (pbPattern - cbPattern, pbTarget - cbPattern,
                        (unsigned int) cbPattern))
            // if ( ulHashPattern == ulHashTarget ) {
            // 
            //  count = cbPattern;
            //  buf1 = pbPattern-cbPattern;
            //  buf2 = pbTarget-cbPattern;
            //  while ( --count && *(char *)buf1 == *(char *)buf2 ) {
            //          buf1 = (char *)buf1 + 1;
            //          buf2 = (char *)buf2 + 1;
            //  }
            //                 
            //  if ( *((unsigned char *)buf1) - *((unsigned char *)buf2) == 0) hits++;
            //  }
            return ((pbTarget - cbPattern));
        //hits++;

        if (pbTarget == pbTargetMax)
            return (NULL);

        // Below line gives 482KB/clock for 'underdog' + requirement pattern to be 2 chars min.:
        //ulHashTarget = ( (*(unsigned short *)(pbTarget+1-cbPattern)) | *pbTarget );
        // Below line gives 436KB/clock for 'underdog' + requirement pattern to be 4 chars min.:
        //ulHashTarget = ( (*(long *)(pbTarget+1-cbPattern)) & 0xffffff00 ) + *pbTarget;
//; Line 696
//        movsx   esi, BYTE PTR [ebx]
//        mov     ecx, DWORD PTR [edx+1]
//        and     ecx, -256                               ; ffffff00H
//        add     ecx, esi
        // Below line gives 482KB/clock for 'underdog' + requirement pattern to be 2 chars min.:
        //ulHashTarget = ( (*(unsigned short *)(pbTarget+1-cbPattern)) & 0xff00 ) + *pbTarget;
//; Line 691
//        movsx   esi, BYTE PTR [ebx]
//        xor     ecx, ecx
//        mov     cx, WORD PTR [edx+1]
//        and     ecx, 65280                              ; 0000ff00H
//        add     ecx, esi
        // Below line gives 605KB/clock for 'underdog' + requirement pattern to be 2 chars min.:
        //ulHashTarget = ( (*(unsigned short *)(pbTarget+1-cbPattern))<<8 ) + *pbTarget;
        // Below line gives 668KB/clock for 'underdog':
        ulHashTarget =
            ((*(char *) (pbTarget + 1 - cbPattern)) << 8) + *pbTarget;
//; Line 718
//        movsx   ecx, BYTE PTR [eax+1]
//        movsx   edx, BYTE PTR [ebp]
//        shl     ecx, 8
//        add     ecx, edx
        // Below line gives 366KB/clock for 'underdog':
        //ulHashTarget = (ulHashTarget - *(pbTarget-cbPattern)*ulBaseToPowerMod)*ulBase + *pbTarget;
        pbTarget++;
    }
}

// ### Karp-Rabin-Kaze algorithm ]


// ### Mix(2in1) of Karp-Rabin & Boyer-Moore-Horspool algorithm [
// Caution: For better speed the case 'if (cbPattern==1)' was removed, so Pattern must be longer than 1 char.
char *
Railgun_Quadruplet(char *pbTarget,
                    char *pbPattern,
                    unsigned long cbTarget, unsigned long cbPattern)
{
    char *pbTargetMax = pbTarget + cbTarget;
    register unsigned long ulHashPattern;
    unsigned long ulHashTarget;
    unsigned long count;
    unsigned long countSTATIC;
//  unsigned long countRemainder;

/*
    const unsigned char SINGLET = *(char *)(pbPattern);
    const unsigned long Quadruplet2nd = SINGLET<<8;
    const unsigned long Quadruplet3rd = SINGLET<<16;
    const unsigned long Quadruplet4th = SINGLET<<24;
*/
    unsigned char SINGLET;
    unsigned long Quadruplet2nd;
    unsigned long Quadruplet3rd;
    unsigned long Quadruplet4th;

    unsigned long AdvanceHopperGrass;

    long i;                     //BMH needed
    int a, j, bm_bc[ASIZE];     //BMH needed
    unsigned char ch;           //BMH needed
//    unsigned char lastch, firstch; //BMH needed

    if (cbPattern > cbTarget)
        return (NULL);

// Doesn't work when cbPattern = 1
// The next IF-fragment works very well with cbPattern>1, OBVIOUSLY IT MUST BE UNROLLED(but crippled with less functionality) SINCE either cbPattern=2 or cbPattern=3!
    if (cbPattern < 4) {        // This IF makes me unhappy: it slows down from 390KB/clock to 367KB/clock for 'fast' pattern. This fragment(for 2..3 pattern lengths) is needed because I need a function different than strchr but sticking to strstr i.e. lengths above 1 are to be handled.
        pbTarget = pbTarget + cbPattern;
        ulHashPattern =
            ((*(char *) (pbPattern)) << 8) + *(pbPattern + (cbPattern - 1));
//        countSTATIC = cbPattern-2;

        if (cbPattern == 3) {
            for (;;) {
                if (ulHashPattern ==
                    ((*(char *) (pbTarget - 3)) << 8) + *(pbTarget - 1)) {
                    if (*(char *) (pbPattern + 1) == *(char *) (pbTarget - 2))
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
            // The line below gives for 'cbPattern'>=1:
            // Karp_Rabin_Kaze_4_OCTETS_hits/Karp_Rabin_Kaze_4_OCTETS_clocks: 4/543
            // Karp_Rabin_Kaze_4_OCTETS performance: 372KB/clock
/*
        if ( (ulHashPattern == ( (*(char *)(pbTarget-cbPattern))<<8 ) + *(pbTarget-1)) && !memcmp(pbPattern, pbTarget-cbPattern, (unsigned int)cbPattern) )
            return((long)(pbTarget-cbPattern));
*/

            // The fragment below gives for 'cbPattern'>=2:
            // Karp_Rabin_Kaze_4_OCTETS_hits/Karp_Rabin_Kaze_4_OCTETS_clocks: 4/546
            // Karp_Rabin_Kaze_4_OCTETS performance: 370KB/clock

/*
//For 2 and 3 [
        if ( ulHashPattern == ( (*(char *)(pbTarget-cbPattern))<<8 ) + *(pbTarget-1) ) {
//         count = countSTATIC;
         count = cbPattern-2;
//         while ( count && *(char *)(pbPattern+1+(countSTATIC-count)) == *(char *)(pbTarget-cbPattern+1+(countSTATIC-count)) ) {
         while ( count && *(char *)(pbPattern+1) == *(char *)(pbTarget-2) ) { // Crippling i.e. only 2 and 3 chars are allowed!
               count--;
         }
         if ( count == 0) return((pbTarget-cbPattern));
        }
        if ( (char)(ulHashPattern>>8) != *(pbTarget-cbPattern+1) ) pbTarget++;
//For 2 and 3 ]
*/


            if (ulHashPattern ==
                ((*(char *) (pbTarget - 2)) << 8) + *(pbTarget - 1))
                return ((pbTarget - 2));
            if ((char) (ulHashPattern >> 8) != *(pbTarget - 1))
                pbTarget++;


            // The fragment below gives for 'cbPattern'>=2:
            // Karp_Rabin_Kaze_4_OCTETS_hits/Karp_Rabin_Kaze_4_OCTETS_clocks: 4/554
            // Karp_Rabin_Kaze_4_OCTETS performance: 364KB/clock
/*
        if ( ulHashPattern == ( (*(char *)(pbTarget-cbPattern))<<8 ) + *(pbTarget-1) ) {
         count = countSTATIC>>2;
         countRemainder = countSTATIC % 4;

         while ( count && *(unsigned long *)(pbPattern+1+((count-1)<<2)) == *(unsigned long *)(pbTarget-cbPattern+1+((count-1)<<2)) ) {
               count--;
         }
	 //if (count == 0) {  // Disastrous degradation only from this line(317KB/clock when 1+2x4+2+1 bytes pattern: 'skillessness'; 312KB/clock when 1+1x4+2+1 bytes pattern: 'underdog'), otherwise 368KB/clock.
         while ( countRemainder && *(char *)(pbPattern+1+(countSTATIC-countRemainder)) == *(char *)(pbTarget-cbPattern+1+(countSTATIC-countRemainder)) ) {
               countRemainder--;
         }
         //if ( countRemainder == 0) return((long)(pbTarget-cbPattern));
         if ( count+countRemainder == 0) return((long)(pbTarget-cbPattern));
         //}
        }
*/

            pbTarget++;
            if (pbTarget > pbTargetMax)
                return (NULL);
        }
    }
    else {                      //if ( cbPattern<4)
        if (cbTarget < 961)     // This value is arbitrary(don't know how exactly), it ensures(at least must) better performance than 'Boyer_Moore_Horspool'.
        {
            pbTarget = pbTarget + cbPattern;
            ulHashPattern = *(unsigned long *) (pbPattern);
//        countSTATIC = cbPattern-1;

            //SINGLET = *(char *)(pbPattern);
            SINGLET = ulHashPattern & 0xFF;
            Quadruplet2nd = SINGLET << 8;
            Quadruplet3rd = SINGLET << 16;
            Quadruplet4th = SINGLET << 24;

            for (;;) {
                AdvanceHopperGrass = 0;
                ulHashTarget = *(unsigned long *) (pbTarget - cbPattern);

                if (ulHashPattern == ulHashTarget) {    // Three unnecessary comparisons here, but 'AdvanceHopperGrass' must be calculated - it has a higher priority.
//         count = countSTATIC;
//         while ( count && *(char *)(pbPattern+1+(countSTATIC-count)) == *(char *)(pbTarget-cbPattern+1+(countSTATIC-count)) ) {
//         if ( countSTATIC==AdvanceHopperGrass+count && SINGLET != *(char *)(pbTarget-cbPattern+1+(countSTATIC-count)) ) AdvanceHopperGrass++;
//               count--;
//         }
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
                        return ((pbTarget - cbPattern));
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
            countSTATIC = cbPattern - 2;
            /* Preprocessing */
            for (a = 0; a < ASIZE; a++)
                bm_bc[a] = cbPattern;
            for (j = 0; j < cbPattern - 1; j++)
                bm_bc[(unsigned char)pbPattern[j]] = cbPattern - j - 1;

            /* Searching */
            //lastch=pbPattern[cbPattern-1];
            //firstch=pbPattern[0];
            i = 0;
            while (i <= cbTarget - cbPattern) {
                ch = pbTarget[i + cbPattern - 1];
                //if (ch ==lastch)
                //if (memcmp(&pbTarget[i],pbPattern,cbPattern-1) == 0) OUTPUT(i);
                //if (ch == lastch && pbTarget[i] == firstch && memcmp(&pbTarget[i],pbPattern,cbPattern-1) == 0) return(i);  // Kaze: The idea(to prevent execution of slower 'memcmp') is borrowed from Karp-Rabin i.e. to perform a slower check only when the target "looks like".
                if (ch == pbPattern[cbPattern - 1]
                    && pbTarget[i] == pbPattern[0]) {
                    count = countSTATIC;
                    while (count
                           && *(char *) (pbPattern + 1 +
                                         (countSTATIC - count)) ==
                           *(char *) (&pbTarget[i] + 1 +
                                      (countSTATIC - count))) {
                        count--;
                    }
                    if (count == 0)
                        return (pbTarget + i);
                }
                i += bm_bc[ch];
            }
            return (NULL);
        }                       //if (cbTarget<961)
    }                           //if ( cbPattern<4)
}

// ### Mix(2in1) of Karp-Rabin & Boyer-Moore-Horspool algorithm ]


// ### Mix(2in1) of Karp-Rabin & Boyer-Moore-Horspool algorithm [
// Caution: For better speed the case 'if (cbPattern==1)' was removed, so Pattern must be longer than 1 char.
char *
Railgun_Quadruplet_6pp(char *pbTarget,
                        char *pbPattern,
                        unsigned long cbTarget, unsigned long cbPattern)
{
    char *pbTargetMax = pbTarget + cbTarget;
    register unsigned long ulHashPattern;
    unsigned long ulHashTarget;
    //unsigned long count; //r.6+
    signed long count;
    //unsigned long countSTATIC; //r.6+
    signed long countSTATIC;
//  unsigned long countRemainder;

/*
    const unsigned char SINGLET = *(char *)(pbPattern);
    const unsigned long Quadruplet2nd = SINGLET<<8;
    const unsigned long Quadruplet3rd = SINGLET<<16;
    const unsigned long Quadruplet4th = SINGLET<<24;
*/
    unsigned char SINGLET;
    unsigned long Quadruplet2nd;
    unsigned long Quadruplet3rd;
    unsigned long Quadruplet4th;

    unsigned long AdvanceHopperGrass;

    long i;                     //BMH needed
    int a, j, bm_bc[ASIZE];     //BMH needed
    unsigned char ch;           //BMH needed
//    unsigned char lastch, firstch; //BMH needed

    if (cbPattern > cbTarget)
        return (NULL);

// Doesn't work when cbPattern = 1
// The next IF-fragment works very well with cbPattern>1, OBVIOUSLY IT MUST BE UNROLLED(but crippled with less functionality) SINCE either cbPattern=2 or cbPattern=3!
    if (cbPattern < 4) {        // This IF makes me unhappy: it slows down from 390KB/clock to 367KB/clock for 'fast' pattern. This fragment(for 2..3 pattern lengths) is needed because I need a function different than strchr but sticking to strstr i.e. lengths above 1 are to be handled.
        pbTarget = pbTarget + cbPattern;
        ulHashPattern =
            ((*(char *) (pbPattern)) << 8) + *(pbPattern + (cbPattern - 1));
//        countSTATIC = cbPattern-2;

        if (cbPattern == 3) {
            for (;;) {
                if (ulHashPattern ==
                    ((*(char *) (pbTarget - 3)) << 8) + *(pbTarget - 1)) {
                    if (*(char *) (pbPattern + 1) == *(char *) (pbTarget - 2))
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
            // The line below gives for 'cbPattern'>=1:
            // Karp_Rabin_Kaze_4_OCTETS_hits/Karp_Rabin_Kaze_4_OCTETS_clocks: 4/543
            // Karp_Rabin_Kaze_4_OCTETS performance: 372KB/clock
/*
        if ( (ulHashPattern == ( (*(char *)(pbTarget-cbPattern))<<8 ) + *(pbTarget-1)) && !memcmp(pbPattern, pbTarget-cbPattern, (unsigned int)cbPattern) )
            return((long)(pbTarget-cbPattern));
*/

            // The fragment below gives for 'cbPattern'>=2:
            // Karp_Rabin_Kaze_4_OCTETS_hits/Karp_Rabin_Kaze_4_OCTETS_clocks: 4/546
            // Karp_Rabin_Kaze_4_OCTETS performance: 370KB/clock

/*
//For 2 and 3 [
        if ( ulHashPattern == ( (*(char *)(pbTarget-cbPattern))<<8 ) + *(pbTarget-1) ) {
//         count = countSTATIC;
         count = cbPattern-2;
//         while ( count && *(char *)(pbPattern+1+(countSTATIC-count)) == *(char *)(pbTarget-cbPattern+1+(countSTATIC-count)) ) {
         while ( count && *(char *)(pbPattern+1) == *(char *)(pbTarget-2) ) { // Crippling i.e. only 2 and 3 chars are allowed!
               count--;
         }
         if ( count == 0) return((pbTarget-cbPattern));
        }
        if ( (char)(ulHashPattern>>8) != *(pbTarget-cbPattern+1) ) pbTarget++;
//For 2 and 3 ]
*/


            if (ulHashPattern ==
                ((*(char *) (pbTarget - 2)) << 8) + *(pbTarget - 1))
                return ((pbTarget - 2));
            if ((char) (ulHashPattern >> 8) != *(pbTarget - 1))
                pbTarget++;


            // The fragment below gives for 'cbPattern'>=2:
            // Karp_Rabin_Kaze_4_OCTETS_hits/Karp_Rabin_Kaze_4_OCTETS_clocks: 4/554
            // Karp_Rabin_Kaze_4_OCTETS performance: 364KB/clock
/*
        if ( ulHashPattern == ( (*(char *)(pbTarget-cbPattern))<<8 ) + *(pbTarget-1) ) {
         count = countSTATIC>>2;
         countRemainder = countSTATIC % 4;

         while ( count && *(unsigned long *)(pbPattern+1+((count-1)<<2)) == *(unsigned long *)(pbTarget-cbPattern+1+((count-1)<<2)) ) {
               count--;
         }
	 //if (count == 0) {  // Disastrous degradation only from this line(317KB/clock when 1+2x4+2+1 bytes pattern: 'skillessness'; 312KB/clock when 1+1x4+2+1 bytes pattern: 'underdog'), otherwise 368KB/clock.
         while ( countRemainder && *(char *)(pbPattern+1+(countSTATIC-countRemainder)) == *(char *)(pbTarget-cbPattern+1+(countSTATIC-countRemainder)) ) {
               countRemainder--;
         }
         //if ( countRemainder == 0) return((long)(pbTarget-cbPattern));
         if ( count+countRemainder == 0) return((long)(pbTarget-cbPattern));
         //}
        }
*/

            pbTarget++;
            if (pbTarget > pbTargetMax)
                return (NULL);
        }
    }
    else {                      //if ( cbPattern<4)
        if (cbTarget < 961)     // This value is arbitrary(don't know how exactly), it ensures(at least must) better performance than 'Boyer_Moore_Horspool'.
        {
            pbTarget = pbTarget + cbPattern;
            ulHashPattern = *(unsigned long *) (pbPattern);
//        countSTATIC = cbPattern-1;

            //SINGLET = *(char *)(pbPattern);
            SINGLET = ulHashPattern & 0xFF;
            Quadruplet2nd = SINGLET << 8;
            Quadruplet3rd = SINGLET << 16;
            Quadruplet4th = SINGLET << 24;

            for (;;) {
                AdvanceHopperGrass = 0;
                ulHashTarget = *(unsigned long *) (pbTarget - cbPattern);

                if (ulHashPattern == ulHashTarget) {    // Three unnecessary comparisons here, but 'AdvanceHopperGrass' must be calculated - it has a higher priority.
//         count = countSTATIC;
//         while ( count && *(char *)(pbPattern+1+(countSTATIC-count)) == *(char *)(pbTarget-cbPattern+1+(countSTATIC-count)) ) {
//         if ( countSTATIC==AdvanceHopperGrass+count && SINGLET != *(char *)(pbTarget-cbPattern+1+(countSTATIC-count)) ) AdvanceHopperGrass++;
//               count--;
//         }
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
                        return ((pbTarget - cbPattern));
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
            //countSTATIC = cbPattern-2; //r.6+
            //countSTATIC = cbPattern-2-3;
            countSTATIC = cbPattern - 2 - 2;    // r.6+++ I suppose that the awful degradation comes from 2bytes more (from either 'if (countSTATIC<0) countSTATIC=0;' or 'count >0' fixes) which make the function unfittable in code cache lines?!
            ulHashPattern = *(unsigned long *) (pbPattern);
// Next line fixes the BUG from r.6++: but with awful speed degradation! So the bug is fixed in the definitions by setting 'countSTATIC = cbPattern-2-2;', bug appears only for patterns with lengths of 4, The setback is one unnecessary comparison for 5bytes patterns, stupidly such setback exists (from before) for 4bytes as well.
//if (countSTATIC<0) countSTATIC=0;
            /* Preprocessing */
            for (a = 0; a < ASIZE; a++)
                bm_bc[a] = cbPattern;
            for (j = 0; j < cbPattern - 1; j++)
                bm_bc[(unsigned char)pbPattern[j]] = cbPattern - j - 1;

            /* Searching */
            //lastch=pbPattern[cbPattern-1];
            //firstch=pbPattern[0];
            i = 0;
            while (i <= cbTarget - cbPattern) {
                ch = pbTarget[i + cbPattern - 1];
                //if (ch ==lastch)
                //if (memcmp(&pbTarget[i],pbPattern,cbPattern-1) == 0) OUTPUT(i);
                //if (ch == lastch && pbTarget[i] == firstch && memcmp(&pbTarget[i],pbPattern,cbPattern-1) == 0) return(i);  // Kaze: The idea(to prevent execution of slower 'memcmp') is borrowed from Karp-Rabin i.e. to perform a slower check only when the target "looks like".
                //if (ch == pbPattern[cbPattern-1] && pbTarget[i] == pbPattern[0]) // r.6+
                //if (ch == pbPattern[cbPattern-1] && *(long *)&pbTarget[i] == *(long *)&pbPattern[0]) // No problema here since we have 4[+] long pattern here. Overlapping (1 byte recompared) when length=4, grmbl.
                if (ch == pbPattern[cbPattern - 1] && *(long *) &pbTarget[i] == ulHashPattern)  // No problema here since we have 4[+] long pattern here. Overlapping (1 byte recompared) when length=4, grmbl.
                {
                    count = countSTATIC;
                    //while ( count && *(char *)(pbPattern+1+(countSTATIC-count)) == *(char *)(&pbTarget[i]+1+(countSTATIC-count)) ) { // r.6+
// A BUG (in next line) crushed from r.6++: 'count !=0' becomes 'count >0' in r.6+++ but with awful speed degradation! So the bug is fixed outside the cycles by setting 'countSTATIC' from -1 to 0, bug appears only for patterns with lengths of 4.
                    while (count != 0 && *(char *) (pbPattern + 1 + 3 + (countSTATIC - count)) == *(char *) (&pbTarget[i] + 1 + 3 + (countSTATIC - count))) {   // if pattern length is 4 or 5 we have count=-1 and count=0 respectively i.e. no need of comparing in-between chars.
                        count--;
                    }
                    if (count <= 0)
                        return (pbTarget + i);
                }
                i += bm_bc[ch];
            }
            return (NULL);
        }                       //if (cbTarget<961)
    }                           //if ( cbPattern<4)
}

// ### Mix(2in1) of Karp-Rabin & Boyer-Moore-Horspool algorithm ]


// ### Mix(2in1) of Karp-Rabin & Boyer-Moore-Horspool algorithm [
// Caution: For better speed the case 'if (cbPattern==1)' was removed, so Pattern must be longer than 1 char.
char *
Railgun_Quadruplet_6pp_count_hits(char *pbTarget,
                                   char *pbPattern,
                                   unsigned long cbTarget,
                                   unsigned long cbPattern)
{
    char *pbTargetMax = pbTarget + cbTarget;
    register unsigned long ulHashPattern;
    unsigned long ulHashTarget;
    //unsigned long count; //r.6+
    signed long count;
    //unsigned long countSTATIC; //r.6+
    signed long countSTATIC;
//  unsigned long countRemainder;

/*
    const unsigned char SINGLET = *(char *)(pbPattern);
    const unsigned long Quadruplet2nd = SINGLET<<8;
    const unsigned long Quadruplet3rd = SINGLET<<16;
    const unsigned long Quadruplet4th = SINGLET<<24;
*/
    unsigned char SINGLET;
    unsigned long Quadruplet2nd;
    unsigned long Quadruplet3rd;
    unsigned long Quadruplet4th;

    unsigned long AdvanceHopperGrass;

    long i;                     //BMH needed
    int a, j, bm_bc[ASIZE];     //BMH needed
    unsigned char ch;           //BMH needed
//    unsigned char lastch, firstch; //BMH needed

    if (cbPattern > cbTarget)
        return (NULL);

// Doesn't work when cbPattern = 1
// The next IF-fragment works very well with cbPattern>1, OBVIOUSLY IT MUST BE UNROLLED(but crippled with less functionality) SINCE either cbPattern=2 or cbPattern=3!
    if (cbPattern < 4) {        // This IF makes me unhappy: it slows down from 390KB/clock to 367KB/clock for 'fast' pattern. This fragment(for 2..3 pattern lengths) is needed because I need a function different than strchr but sticking to strstr i.e. lengths above 1 are to be handled.
        pbTarget = pbTarget + cbPattern;
        ulHashPattern =
            ((*(char *) (pbPattern)) << 8) + *(pbPattern + (cbPattern - 1));
//        countSTATIC = cbPattern-2;

        if (cbPattern == 3) {
            for (;;) {
                if (ulHashPattern ==
                    ((*(char *) (pbTarget - 3)) << 8) + *(pbTarget - 1)) {
                    if (*(char *) (pbPattern + 1) == *(char *) (pbTarget - 2))
                        Railgunhits++;  //return((pbTarget-3));
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
            // The line below gives for 'cbPattern'>=1:
            // Karp_Rabin_Kaze_4_OCTETS_hits/Karp_Rabin_Kaze_4_OCTETS_clocks: 4/543
            // Karp_Rabin_Kaze_4_OCTETS performance: 372KB/clock
/*
        if ( (ulHashPattern == ( (*(char *)(pbTarget-cbPattern))<<8 ) + *(pbTarget-1)) && !memcmp(pbPattern, pbTarget-cbPattern, (unsigned int)cbPattern) )
            return((long)(pbTarget-cbPattern));
*/

            // The fragment below gives for 'cbPattern'>=2:
            // Karp_Rabin_Kaze_4_OCTETS_hits/Karp_Rabin_Kaze_4_OCTETS_clocks: 4/546
            // Karp_Rabin_Kaze_4_OCTETS performance: 370KB/clock

/*
//For 2 and 3 [
        if ( ulHashPattern == ( (*(char *)(pbTarget-cbPattern))<<8 ) + *(pbTarget-1) ) {
//         count = countSTATIC;
         count = cbPattern-2;
//         while ( count && *(char *)(pbPattern+1+(countSTATIC-count)) == *(char *)(pbTarget-cbPattern+1+(countSTATIC-count)) ) {
         while ( count && *(char *)(pbPattern+1) == *(char *)(pbTarget-2) ) { // Crippling i.e. only 2 and 3 chars are allowed!
               count--;
         }
         if ( count == 0) return((pbTarget-cbPattern));
        }
        if ( (char)(ulHashPattern>>8) != *(pbTarget-cbPattern+1) ) pbTarget++;
//For 2 and 3 ]
*/


            if (ulHashPattern ==
                ((*(char *) (pbTarget - 2)) << 8) + *(pbTarget - 1))
                Railgunhits++;  //return((pbTarget-2));
            if ((char) (ulHashPattern >> 8) != *(pbTarget - 1))
                pbTarget++;


            // The fragment below gives for 'cbPattern'>=2:
            // Karp_Rabin_Kaze_4_OCTETS_hits/Karp_Rabin_Kaze_4_OCTETS_clocks: 4/554
            // Karp_Rabin_Kaze_4_OCTETS performance: 364KB/clock
/*
        if ( ulHashPattern == ( (*(char *)(pbTarget-cbPattern))<<8 ) + *(pbTarget-1) ) {
         count = countSTATIC>>2;
         countRemainder = countSTATIC % 4;

         while ( count && *(unsigned long *)(pbPattern+1+((count-1)<<2)) == *(unsigned long *)(pbTarget-cbPattern+1+((count-1)<<2)) ) {
               count--;
         }
	 //if (count == 0) {  // Disastrous degradation only from this line(317KB/clock when 1+2x4+2+1 bytes pattern: 'skillessness'; 312KB/clock when 1+1x4+2+1 bytes pattern: 'underdog'), otherwise 368KB/clock.
         while ( countRemainder && *(char *)(pbPattern+1+(countSTATIC-countRemainder)) == *(char *)(pbTarget-cbPattern+1+(countSTATIC-countRemainder)) ) {
               countRemainder--;
         }
         //if ( countRemainder == 0) return((long)(pbTarget-cbPattern));
         if ( count+countRemainder == 0) return((long)(pbTarget-cbPattern));
         //}
        }
*/

            pbTarget++;
            if (pbTarget > pbTargetMax)
                return (NULL);
        }
    }
    else {                      //if ( cbPattern<4)
        if (cbTarget < 961)     // This value is arbitrary(don't know how exactly), it ensures(at least must) better performance than 'Boyer_Moore_Horspool'.
        {
            pbTarget = pbTarget + cbPattern;
            ulHashPattern = *(unsigned long *) (pbPattern);
//        countSTATIC = cbPattern-1;

            //SINGLET = *(char *)(pbPattern);
            SINGLET = ulHashPattern & 0xFF;
            Quadruplet2nd = SINGLET << 8;
            Quadruplet3rd = SINGLET << 16;
            Quadruplet4th = SINGLET << 24;

            for (;;) {
                AdvanceHopperGrass = 0;
                ulHashTarget = *(unsigned long *) (pbTarget - cbPattern);

                if (ulHashPattern == ulHashTarget) {    // Three unnecessary comparisons here, but 'AdvanceHopperGrass' must be calculated - it has a higher priority.
//         count = countSTATIC;
//         while ( count && *(char *)(pbPattern+1+(countSTATIC-count)) == *(char *)(pbTarget-cbPattern+1+(countSTATIC-count)) ) {
//         if ( countSTATIC==AdvanceHopperGrass+count && SINGLET != *(char *)(pbTarget-cbPattern+1+(countSTATIC-count)) ) AdvanceHopperGrass++;
//               count--;
//         }
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
                        Railgunhits++;  //return((pbTarget-cbPattern));
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
            //countSTATIC = cbPattern-2; //r.6+
            //countSTATIC = cbPattern-2-3;
            countSTATIC = cbPattern - 2 - 2;    // r.6+++ I suppose that the awful degradation comes from 2bytes more (from either 'if (countSTATIC<0) countSTATIC=0;' or 'count >0' fixes) which make the function unfittable in code cache lines?!
            ulHashPattern = *(unsigned long *) (pbPattern);
// Next line fixes the BUG from r.6++: but with awful speed degradation! So the bug is fixed in the definitions by setting 'countSTATIC = cbPattern-2-2;', bug appears only for patterns with lengths of 4, The setback is one unnecessary comparison for 5bytes patterns, stupidly such setback exists (from before) for 4bytes as well.
//if (countSTATIC<0) countSTATIC=0;
            /* Preprocessing */
            for (a = 0; a < ASIZE; a++)
                bm_bc[a] = cbPattern;
            for (j = 0; j < cbPattern - 1; j++)
                bm_bc[(unsigned char)pbPattern[j]] = cbPattern - j - 1;

            /* Searching */
            //lastch=pbPattern[cbPattern-1];
            //firstch=pbPattern[0];
            i = 0;
            while (i <= cbTarget - cbPattern) {
                ch = pbTarget[i + cbPattern - 1];
                //if (ch ==lastch)
                //if (memcmp(&pbTarget[i],pbPattern,cbPattern-1) == 0) OUTPUT(i);
                //if (ch == lastch && pbTarget[i] == firstch && memcmp(&pbTarget[i],pbPattern,cbPattern-1) == 0) return(i);  // Kaze: The idea(to prevent execution of slower 'memcmp') is borrowed from Karp-Rabin i.e. to perform a slower check only when the target "looks like".
                //if (ch == pbPattern[cbPattern-1] && pbTarget[i] == pbPattern[0]) // r.6+
                //if (ch == pbPattern[cbPattern-1] && *(long *)&pbTarget[i] == *(long *)&pbPattern[0]) // No problema here since we have 4[+] long pattern here. Overlapping (1 byte recompared) when length=4, grmbl.
                if (ch == pbPattern[cbPattern - 1] && *(long *) &pbTarget[i] == ulHashPattern)  // No problema here since we have 4[+] long pattern here. Overlapping (1 byte recompared) when length=4, grmbl.
                {
                    count = countSTATIC;
                    //while ( count && *(char *)(pbPattern+1+(countSTATIC-count)) == *(char *)(&pbTarget[i]+1+(countSTATIC-count)) ) { // r.6+
// A BUG (in next line) crushed from r.6++: 'count !=0' becomes 'count >0' in r.6+++ but with awful speed degradation! So the bug is fixed outside the cycles by setting 'countSTATIC' from -1 to 0, bug appears only for patterns with lengths of 4.
                    while (count != 0 && *(char *) (pbPattern + 1 + 3 + (countSTATIC - count)) == *(char *) (&pbTarget[i] + 1 + 3 + (countSTATIC - count))) {   // if pattern length is 4 or 5 we have count=-1 and count=0 respectively i.e. no need of comparing in-between chars.
                        count--;
                    }
                    if (count <= 0)
                        Railgunhits++;  //return(pbTarget+i);
                }
                i += bm_bc[ch];
            }
            return (NULL);
        }                       //if (cbTarget<961)
    }                           //if ( cbPattern<4)
}

// ### Mix(2in1) of Karp-Rabin & Boyer-Moore-Horspool algorithm ]


// ### Mix(2in1) of Karp-Rabin & Boyer-Moore-Horspool algorithm [
// Caution: For better speed the case 'if (cbPattern==1)' was removed, so Pattern must be longer than 1 char.
char *
Railgun(char *pbTarget,
         char *pbPattern, unsigned long cbTarget, unsigned long cbPattern)
{
    char *pbTargetMax = pbTarget + cbTarget;
    register unsigned long ulHashPattern;
    unsigned long ulHashTarget;
    unsigned long count;
    unsigned long countSTATIC, countRemainder;

    long i;                     //BMH needed
    int a, j, bm_bc[ASIZE];     //BMH needed
    unsigned char ch;           //BMH needed
//    unsigned char lastch, firstch; //BMH needed

    if (cbPattern > cbTarget)
        return (NULL);

    countSTATIC = cbPattern - 2;

// Doesn't work when cbPattern = 1
    if (cbTarget < 961)         // This value is arbitrary(don't know how exactly), it ensures(at least must) better performance than 'Boyer_Moore_Horspool'.
    {
        pbTarget = pbTarget + cbPattern;
        ulHashPattern =
            ((*(char *) (pbPattern)) << 8) + *(pbPattern + (cbPattern - 1));

        for (;;) {
            // The line below gives for 'cbPattern'>=1:
            // Karp_Rabin_Kaze_4_OCTETS_hits/Karp_Rabin_Kaze_4_OCTETS_clocks: 4/543
            // Karp_Rabin_Kaze_4_OCTETS performance: 372KB/clock
/*
        if ( (ulHashPattern == ( (*(char *)(pbTarget-cbPattern))<<8 ) + *(pbTarget-1)) && !memcmp(pbPattern, pbTarget-cbPattern, (unsigned int)cbPattern) )
            return((long)(pbTarget-cbPattern));
*/

            // The fragment below gives for 'cbPattern'>=2:
            // Karp_Rabin_Kaze_4_OCTETS_hits/Karp_Rabin_Kaze_4_OCTETS_clocks: 4/546
            // Karp_Rabin_Kaze_4_OCTETS performance: 370KB/clock

            if (ulHashPattern ==
                ((*(char *) (pbTarget - cbPattern)) << 8) + *(pbTarget - 1)) {
                count = countSTATIC;
                while (count
                       && *(char *) (pbPattern + 1 + (countSTATIC - count)) ==
                       *(char *) (pbTarget - cbPattern + 1 +
                                  (countSTATIC - count))) {
                    count--;
                }
                if (count == 0)
                    return ((pbTarget - cbPattern));
            }

            // The fragment below gives for 'cbPattern'>=2:
            // Karp_Rabin_Kaze_4_OCTETS_hits/Karp_Rabin_Kaze_4_OCTETS_clocks: 4/554
            // Karp_Rabin_Kaze_4_OCTETS performance: 364KB/clock
/*
        if ( ulHashPattern == ( (*(char *)(pbTarget-cbPattern))<<8 ) + *(pbTarget-1) ) {
         count = countSTATIC>>2;
         countRemainder = countSTATIC % 4;

         while ( count && *(unsigned long *)(pbPattern+1+((count-1)<<2)) == *(unsigned long *)(pbTarget-cbPattern+1+((count-1)<<2)) ) {
               count--;
         }
	 //if (count == 0) {  // Disastrous degradation only from this line(317KB/clock when 1+2x4+2+1 bytes pattern: 'skillessness'; 312KB/clock when 1+1x4+2+1 bytes pattern: 'underdog'), otherwise 368KB/clock.
         while ( countRemainder && *(char *)(pbPattern+1+(countSTATIC-countRemainder)) == *(char *)(pbTarget-cbPattern+1+(countSTATIC-countRemainder)) ) {
               countRemainder--;
         }
         //if ( countRemainder == 0) return((long)(pbTarget-cbPattern));
         if ( count+countRemainder == 0) return((long)(pbTarget-cbPattern));
         //}
        }
*/

            pbTarget++;
            if (pbTarget > pbTargetMax)
                return (NULL);
        }
    }
    else {
        /* Preprocessing */
        for (a = 0; a < ASIZE; a++)
            bm_bc[a] = cbPattern;
        for (j = 0; j < cbPattern - 1; j++)
            bm_bc[(unsigned char)pbPattern[j]] = cbPattern - j - 1;

        /* Searching */
        //lastch=pbPattern[cbPattern-1];
        //firstch=pbPattern[0];
        i = 0;
        while (i <= cbTarget - cbPattern) {
            ch = pbTarget[i + cbPattern - 1];
            //if (ch ==lastch)
            //if (memcmp(&pbTarget[i],pbPattern,cbPattern-1) == 0) OUTPUT(i);
            //if (ch == lastch && pbTarget[i] == firstch && memcmp(&pbTarget[i],pbPattern,cbPattern-1) == 0) return(i);  // Kaze: The idea(to prevent execution of slower 'memcmp') is borrowed from Karp-Rabin i.e. to perform a slower check only when the target "looks like".
            if (ch == pbPattern[cbPattern - 1] && pbTarget[i] == pbPattern[0]) {
                count = countSTATIC;
                while (count
                       && *(char *) (pbPattern + 1 + (countSTATIC - count)) ==
                       *(char *) (&pbTarget[i] + 1 + (countSTATIC - count))) {
                    count--;
                }
                if (count == 0)
                    return (pbTarget + i);
            }
            i += bm_bc[ch];
        }
        return (NULL);
    }
}

// ### Mix(2in1) of Karp-Rabin & Boyer-Moore-Horspool algorithm ]


// ### Railgun_totalhits [
char *
Railgun_totalhits(char *pbTarget,
                   char *pbPattern,
                   unsigned long cbTarget, unsigned long cbPattern)
{
    char *pbTargetMax = pbTarget + cbTarget;
    register unsigned long ulHashPattern;
    unsigned long ulHashTarget;
    unsigned long count;
    unsigned long countSTATIC, countRemainder;

    long i;                     //BMH needed
    int a, j, bm_bc[ASIZE];     //BMH needed
    unsigned char ch;           //BMH needed
//    unsigned char lastch, firstch; //BMH needed

    if (cbPattern > cbTarget)
        return (NULL);

    countSTATIC = cbPattern - 2;

// Doesn't work when cbPattern = 1
    if (cbTarget < 961)         // This value is arbitrary(don't know how exactly), it ensures(at least must) better performance than 'Boyer_Moore_Horspool'.
    {
        pbTarget = pbTarget + cbPattern;
        ulHashPattern =
            ((*(char *) (pbPattern)) << 8) + *(pbPattern + (cbPattern - 1));

        for (;;) {
            // The line below gives for 'cbPattern'>=1:
            // Karp_Rabin_Kaze_4_OCTETS_hits/Karp_Rabin_Kaze_4_OCTETS_clocks: 4/543
            // Karp_Rabin_Kaze_4_OCTETS performance: 372KB/clock
/*
        if ( (ulHashPattern == ( (*(char *)(pbTarget-cbPattern))<<8 ) + *(pbTarget-1)) && !memcmp(pbPattern, pbTarget-cbPattern, (unsigned int)cbPattern) )
            return((long)(pbTarget-cbPattern));
*/

            // The fragment below gives for 'cbPattern'>=2:
            // Karp_Rabin_Kaze_4_OCTETS_hits/Karp_Rabin_Kaze_4_OCTETS_clocks: 4/546
            // Karp_Rabin_Kaze_4_OCTETS performance: 370KB/clock

            if (ulHashPattern ==
                ((*(char *) (pbTarget - cbPattern)) << 8) + *(pbTarget - 1)) {
                count = countSTATIC;
                while (count
                       && *(char *) (pbPattern + 1 + (countSTATIC - count)) ==
                       *(char *) (pbTarget - cbPattern + 1 +
                                  (countSTATIC - count))) {
                    count--;
                }
                if (count == 0)
                    Railgunhits++;  //return((pbTarget-cbPattern));
            }

            // The fragment below gives for 'cbPattern'>=2:
            // Karp_Rabin_Kaze_4_OCTETS_hits/Karp_Rabin_Kaze_4_OCTETS_clocks: 4/554
            // Karp_Rabin_Kaze_4_OCTETS performance: 364KB/clock
/*
        if ( ulHashPattern == ( (*(char *)(pbTarget-cbPattern))<<8 ) + *(pbTarget-1) ) {
         count = countSTATIC>>2;
         countRemainder = countSTATIC % 4;

         while ( count && *(unsigned long *)(pbPattern+1+((count-1)<<2)) == *(unsigned long *)(pbTarget-cbPattern+1+((count-1)<<2)) ) {
               count--;
         }
	 //if (count == 0) {  // Disastrous degradation only from this line(317KB/clock when 1+2x4+2+1 bytes pattern: 'skillessness'; 312KB/clock when 1+1x4+2+1 bytes pattern: 'underdog'), otherwise 368KB/clock.
         while ( countRemainder && *(char *)(pbPattern+1+(countSTATIC-countRemainder)) == *(char *)(pbTarget-cbPattern+1+(countSTATIC-countRemainder)) ) {
               countRemainder--;
         }
         //if ( countRemainder == 0) return((long)(pbTarget-cbPattern));
         if ( count+countRemainder == 0) return((long)(pbTarget-cbPattern));
         //}
        }
*/

            pbTarget++;
            if (pbTarget > pbTargetMax)
                return (NULL);
        }
    }
    else {
        /* Preprocessing */
        for (a = 0; a < ASIZE; a++)
            bm_bc[a] = cbPattern;
        for (j = 0; j < cbPattern - 1; j++)
            bm_bc[(unsigned char)pbPattern[j]] = cbPattern - j - 1;

        /* Searching */
        //lastch=pbPattern[cbPattern-1];
        //firstch=pbPattern[0];
        i = 0;
        while (i <= cbTarget - cbPattern) {
            ch = pbTarget[i + cbPattern - 1];
            //if (ch ==lastch)
            //if (memcmp(&pbTarget[i],pbPattern,cbPattern-1) == 0) OUTPUT(i);
            //if (ch == lastch && pbTarget[i] == firstch && memcmp(&pbTarget[i],pbPattern,cbPattern-1) == 0) return(i);  // Kaze: The idea(to prevent execution of slower 'memcmp') is borrowed from Karp-Rabin i.e. to perform a slower check only when the target "looks like".
            if (ch == pbPattern[cbPattern - 1] && pbTarget[i] == pbPattern[0]) {
                count = countSTATIC;
                while (count
                       && *(char *) (pbPattern + 1 + (countSTATIC - count)) ==
                       *(char *) (&pbTarget[i] + 1 + (countSTATIC - count))) {
                    count--;
                }
                if (count == 0)
                    Railgunhits++;  //return(pbTarget+i);
            }
            i += bm_bc[ch];
        }
        return (NULL);
    }
}

// ### Railgun_totalhits ]


// ### Karp-Rabin-Kaze_BOOSTED algorithm [
char *
KarpRabinKaze_BOOSTED(char *pbTarget,
                       char *pbPattern,
                       unsigned long cbTarget, unsigned long cbPattern)
{
    char *pbTargetMax = pbTarget + cbTarget;
    register unsigned long ulHashPattern;
    unsigned long ulHashTarget;

    if (cbPattern > cbTarget)
        return (NULL);

    pbTarget = pbTarget + cbPattern;
    ulHashPattern =
        ((*(char *) (pbPattern)) << 8) + *(pbPattern + (cbPattern - 1));

    for (;;) {
        // Kaze: The idea(FAILED) here is to add an additional(second) layer in order to prevent execution of slower hash calculation(i.e. first layer) which(hash) prevents execution of even slower 'memcmp'.
        // The line below gives: 314KB/clock
        //if ( *pbPattern == *(char *)(pbTarget-cbPattern) && (ulHashPattern == ( (*(char *)(pbTarget-cbPattern))<<8 ) + *(pbTarget-1)) && !memcmp(pbPattern, pbTarget-cbPattern, (unsigned int)cbPattern) )
        // The line below gives: 370KB/clock
        if ((ulHashPattern ==
             ((*(char *) (pbTarget - cbPattern)) << 8) + *(pbTarget - 1))
            && !memcmp (pbPattern, pbTarget - cbPattern,
                        (unsigned int) cbPattern))
            return ((pbTarget - cbPattern));

        pbTarget++;
        if (pbTarget > pbTargetMax)
            return (NULL);
    }
}

// ### Karp-Rabin-Kaze_BOOSTED algorithm ]

char *
strstr_Microsoft(const char *str1, const char *str2)
{
    char *cp = (char *) str1;
    char *s1, *s2;

    if (!*str2)
        return ((char *) str1);

    while (*cp) {
        s1 = cp;
        s2 = (char *) str2;

        while (*s1 && *s2 && !(*s1 - *s2))
            s1++, s2++;

        if (!*s2)
            return (cp);

        cp++;
    }
    return (NULL);
}

char *
strstr_GNU_C_Library(phaystack, pneedle)
     const char *phaystack;
     const char *pneedle;
{
    const unsigned char *haystack, *needle;
    char b;
    const unsigned char *rneedle;

    haystack = (const unsigned char *) phaystack;

    if ((b = *(needle = (const unsigned char *) pneedle))) {
        char c;
        haystack--;             /* possible ANSI violation */

        {
            char a;
            do
                if (!(a = *++haystack))
                    goto ret0;
            while (a != b);
        }

        if (!(c = *++needle))
            goto foundneedle;
        ++needle;
        goto jin;

        for (;;) {
            {
                char a;
                if (0)
              jin:{
                    if ((a = *++haystack) == c)
                        goto crest;
                }
                else
                    a = *++haystack;
                do {
                    for (; a != b; a = *++haystack) {
                        if (!a)
                            goto ret0;
                        if ((a = *++haystack) == b)
                            break;
                        if (!a)
                            goto ret0;
                    }
                }
                while ((a = *++haystack) != c);
            }
          crest:
            {
                char a;
                {
                    const unsigned char *rhaystack;
                    if (*(rhaystack = haystack-- + 1) ==
                        (a = *(rneedle = needle)))
                        do {
                            if (!a)
                                goto foundneedle;
                            if (*++rhaystack != (a = *++needle))
                                break;
                            if (!a)
                                goto foundneedle;
                        }
                        while (*++rhaystack == (a = *++needle));
                    needle = rneedle;   /* took the register-poor aproach */
                }
                if (!a)
                    break;
            }
        }
    }
  foundneedle:
    return (char *) haystack;
  ret0:
    return 0;
}

int
main(int argc, char *argv[])
{
    (void)argv; //MSS "testgun xxx" skips searches for a string taken from stdin.
    FILE *fp_inLINE;
    int Bozan;
    long ThunderwithL, ThunderwithR;
    char *Strng;
    long Strnglen;
    long StrnglenTRAVERSED;
    char Pattern[20 + 2000];    // skillessness=12 human consciousness=19 I should have known=19
// In the East, enlightenment is described as a state of ultimate=62
    int Patternlen;
    long LinesEncountered = 0;
    long BruteForceDummyhits = 0;
    long KarpRabinKazehits = 0;
    long KarpRabinKaze_BOOSTEDhits = 0;
    long Karp_Rabin_Kaze_4_OCTETShits = 0;
    long Karp_Rabin_Kaze_4_OCTETShits_DOUBLET = 0;
    long KarpRabinhits = 0;
    long HORSPOOLhits = 0;
    long HORSPOOL_Kazehits = 0;
    long strstrMicrosofthits = 0;
    long strstrGNUCLibraryhits = 0;
    long Railgun_Quadruplet_6pp_GO = 0;
    long dummy;

    int i, j;
    char const *DumboBox[8][2] = { 
        {"an\0", "to\0"},
        { "TDK\0", "the\0" },
        { "fast\0", "easy\0" },
        { "grmbl\0", "email\0" },
        { "pasting\0", "amazing\0" },
        { "underdog\0", "superdog\0" },
        { "participants\0", "skillessness\0" },
        { "I should have known\0", "human consciousness\0" }
    };

    long FoundIn;
    char *FoundInPTR;

    printf ("strstr_SHORT-SHOWDOWN, revision 6+++, written by Kaze.\n");
    Pattern[0] = 0x00;
    Strng = (char *) malloc (200 * 1024 * 1024);
    if (Strng == NULL) {
        puts ("strstr_SHORT-SHOWDOWN: Needed memory allocation denied!\n");
        return (1);
    }
    if ((fp_inLINE = fopen ("OSHO.TXT", "rb")) == NULL) {
        printf ("strstr_SHORT-SHOWDOWN: Can't open 'OSHO.TXT' file.\n");
        return (1);
    }

    fseek (fp_inLINE, 0, SEEK_END);
    Strnglen = ftell (fp_inLINE);
    fseek (fp_inLINE, 0, SEEK_SET);
    fread (Strng, 1, Strnglen, fp_inLINE);

    if (argc == 1) {
        printf ("Input Pattern(up to 19+2000 chars): ");
        fgets(Pattern, sizeof(Pattern), stdin);
        //Mischa gets (Pattern);         // char * __cdecl gets(char *);
        Patternlen = strlen (&Pattern[0]);
    }

// Replacing CR with NULL i.e. 13->0
    for (ThunderwithL = 0; ThunderwithL < Strnglen; ThunderwithL++)
        if (Strng[ThunderwithL] == 13)
            Strng[ThunderwithL] = 0;
    ThunderwithL = 0;
    ThunderwithR = 0;


    if (argc == 1) {

// As one line: [
        Railgunhits = 0;
        printf
            ("\nDoing Search for Pattern(%dbytes) into String(%ldbytes) as-one-line ...\n",
             Patternlen, Strnglen);
        clocks1 = clock ();
        for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
        {
//Search area is between Strng[0] .. Strng[n-1]

            dummy =
                HORSPOOL_hits (&Strng[0], &Pattern[0], Strnglen, Patternlen);

        }
        clocks2 = clock ();
        TotalRoughSearchTime = clocks2 - clocks1;
        TotalRoughSearchTime++;
        printf ("BM_HORSPOOL_hits/BM_HORSPOOL_clocks: %lu/%lu\n",
                Railgunhits >> 4, (long) (TotalRoughSearchTime) >> 4);
        printf ("BM_HORSPOOL performance: %ldKB/clock\n",
                (Strnglen / ((long) (TotalRoughSearchTime) >> 4)) >> 10);
// As one line: ]



// As one line: [
        Railgunhits = 0;
        printf
            ("\nDoing Search for Pattern(%dbytes) into String(%ldbytes) as-one-line ...\n",
             Patternlen, Strnglen);
        clocks1 = clock ();
        for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
        {
//Search area is between Strng[0] .. Strng[n-1]

            FoundInPTR =
                Railgun_totalhits (&Strng[0], &Pattern[0], Strnglen,
                                   Patternlen);

        }
        clocks2 = clock ();
        TotalRoughSearchTime = clocks2 - clocks1;
        TotalRoughSearchTime++;
        printf ("Railgun_hits/Railgun_clocks: %lu/%lu\n", Railgunhits >> 4,
                (long) (TotalRoughSearchTime) >> 4);
        printf ("Railgun performance: %ldKB/clock\n",
                (Strnglen / ((long) (TotalRoughSearchTime) >> 4)) >> 10);
// As one line: ]



// As one line: [
        Railgunhits = 0;
        printf
            ("\nDoing Search for Pattern(%dbytes) into String(%ldbytes) as-one-line ...\n",
             Patternlen, Strnglen);
        printf
            ("WARNING! The next line works with BMH only for pattern 4[+] long, otherwise (for 2 and 3) other searcher takes over!\n");
        clocks1 = clock ();
        for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
        {
//Search area is between Strng[0] .. Strng[n-1]

            FoundInPTR =
                Railgun_Quadruplet_6pp_count_hits (&Strng[0], &Pattern[0],
                                                   Strnglen, Patternlen);

        }
        clocks2 = clock ();
        TotalRoughSearchTime = clocks2 - clocks1;
        TotalRoughSearchTime++;
        printf ("Railgun_6pp_hits/Railgun_6pp_clocks: %lu/%lu\n",
                Railgunhits >> 4, (long) (TotalRoughSearchTime) >> 4);
        printf ("Railgun_6pp performance: %ldKB/clock\n",
                (Strnglen / ((long) (TotalRoughSearchTime) >> 4)) >> 10);
// As one line: ]

    }                           //if (argc==1) {




// As one line: [
    printf
        ("\nDoing Search for 8x2 Patterns into String(%ldbytes) as-one-line ...\n",
         Strnglen);
    clocks1 = clock ();
    for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
    {
//Search area is between Strng[0] .. Strng[n-1]

        for (i = 0; i < 8; i++) {
            Railgunhits = 0;
            clocks3 = clock ();
            dummy =
                HORSPOOL_hits (&Strng[0], DumboBox[i][0], Strnglen,
                               strlen (DumboBox[i][0]));
            clocks4 = clock ();
            if (Bozan == (1 << 4) - 1) {
                TotalRoughSearchTime = clocks4 - clocks3;
                TotalRoughSearchTime++;
                printf
                    ("Found ('%s') %ld time(s), BM_HORSPOOL performance: %ldKB/clock\n",
                     DumboBox[i][0], Railgunhits,
                     (Strnglen / ((long) (TotalRoughSearchTime))) >> 10);
            }
            Railgunhits = 0;
            clocks3 = clock ();
            dummy =
                HORSPOOL_hits (&Strng[0], DumboBox[i][1], Strnglen,
                               strlen (DumboBox[i][1]));
            clocks4 = clock ();
            if (Bozan == (1 << 4) - 1) {
                TotalRoughSearchTime = clocks4 - clocks3;
                TotalRoughSearchTime++;
                printf
                    ("Found ('%s') %ld time(s), BM_HORSPOOL performance: %ldKB/clock\n",
                     DumboBox[i][1], Railgunhits,
                     (Strnglen / ((long) (TotalRoughSearchTime))) >> 10);
            }
        }
    }
    clocks2 = clock ();
    TotalRoughSearchTime = clocks2 - clocks1;
    TotalRoughSearchTime++;
//printf( "BM_HORSPOOL_hits/BM_HORSPOOL_clocks: %lu/%lu\n", Railgunhits>>4, (long)(TotalRoughSearchTime)>>4);
    printf ("BM_HORSPOOL 8x2 i.e. average performance: %lluKB/clock\n",
            ((unsigned long long) 8 * 2 * Strnglen /
             ((long) (TotalRoughSearchTime) >> 4)) >> 10);
// As one line: ]



// As one line: [
    printf
        ("\nDoing Search for 8x2 Patterns into String(%ldbytes) as-one-line ...\n",
         Strnglen);
    clocks1 = clock ();
    for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
    {
//Search area is between Strng[0] .. Strng[n-1]

        for (i = 0; i < 8; i++) {
            Railgunhits = 0;
            clocks3 = clock ();
            FoundInPTR =
                Railgun_Quadruplet_6pp_count_hits (&Strng[0], DumboBox[i][0],
                                                   Strnglen,
                                                   strlen (DumboBox[i][0]));
            clocks4 = clock ();
            if (Bozan == (1 << 4) - 1) {
                TotalRoughSearchTime = clocks4 - clocks3;
                TotalRoughSearchTime++;
                printf
                    ("Found ('%s') %ld time(s), Railgun_6pp performance: %ldKB/clock\n",
                     DumboBox[i][0], Railgunhits,
                     (Strnglen / ((long) (TotalRoughSearchTime))) >> 10);
            }
            Railgunhits = 0;
            clocks3 = clock ();
            FoundInPTR =
                Railgun_Quadruplet_6pp_count_hits (&Strng[0], DumboBox[i][1],
                                                   Strnglen,
                                                   strlen (DumboBox[i][1]));
            clocks4 = clock ();
            if (Bozan == (1 << 4) - 1) {
                TotalRoughSearchTime = clocks4 - clocks3;
                TotalRoughSearchTime++;
                printf
                    ("Found ('%s') %ld time(s), Railgun_6pp performance: %ldKB/clock\n",
                     DumboBox[i][1], Railgunhits,
                     (Strnglen / ((long) (TotalRoughSearchTime))) >> 10);
            }
        }
    }
    clocks2 = clock ();
    TotalRoughSearchTime = clocks2 - clocks1;
    TotalRoughSearchTime++;
//printf( "Railgun_6pp_hits/Railgun_6pp_clocks: %lu/%lu\n", Railgunhits>>4, (long)(TotalRoughSearchTime)>>4);
    printf ("Railgun_6pp 8x2 i.e. average performance: %lluKB/clock\n",
            ((unsigned long long) 8 * 2 * Strnglen /
             ((long) (TotalRoughSearchTime) >> 4)) >> 10);
// As one line: ]




    if (argc == 1) {

        printf
            ("\nDoing Search for Pattern(%dbytes) into String(%ldbytes) line-by-line ...\n",
             Patternlen, Strnglen);

// 7[
        clocks1 = clock ();
        for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
        {
//Search area is between Strng[0] .. Strng[n-1]
            StrnglenTRAVERSED = 0;  // Only traversed chars i.e. real
            ThunderwithL = 0;
            ThunderwithR = 0;
            for (;;) {
                while (Strng[ThunderwithR] != 10 && ThunderwithR < Strnglen - 1) {
                    ThunderwithR++;
                }
                FoundInPTR =
                    strstr_Microsoft (&Strng[ThunderwithL], &Pattern[0]);
                if (FoundInPTR != NULL) {
                    strstrMicrosofthits++;
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (FoundInPTR - &Strng[ThunderwithL]);
                }
                else
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (ThunderwithR - ThunderwithL);
                LinesEncountered++;
                ThunderwithR++;
                ThunderwithL = ThunderwithR;
                if (ThunderwithR >= Strnglen - 1)
                    break;
            }
            if (Bozan != (1 << 4) - 1)
                LinesEncountered = 0;
        }
        clocks2 = clock ();
        TotalRoughSearchTime = clocks2 - clocks1;
        TotalRoughSearchTime++;
        printf ("LinesEncountered: %lu\n", LinesEncountered);
        printf ("strstr_Microsoft_hits/strstr_Microsoft_clocks: %lu/%lu\n",
                strstrMicrosofthits >> 4, (long) (TotalRoughSearchTime) >> 4);
        printf ("strstr_Microsoft performance: %ldKB/clock\n",
                (StrnglenTRAVERSED /
                 ((long) (TotalRoughSearchTime) >> 4)) >> 10);
        printf ("StrnglenTRAVERSED: %lu bytes\n", StrnglenTRAVERSED);
// 7]

// 8[
        clocks1 = clock ();
        for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
        {
//Search area is between Strng[0] .. Strng[n-1]
            StrnglenTRAVERSED = 0;  // Only traversed chars i.e. real
            ThunderwithL = 0;
            ThunderwithR = 0;
            for (;;) {
                while (Strng[ThunderwithR] != 10 && ThunderwithR < Strnglen - 1) {
                    ThunderwithR++;
                }
                FoundInPTR =
                    strstr_GNU_C_Library (&Strng[ThunderwithL], &Pattern[0]);
                if (FoundInPTR != 0) {
                    strstrGNUCLibraryhits++;
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (FoundInPTR - &Strng[ThunderwithL]);
                }
                else
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (ThunderwithR - ThunderwithL);
                LinesEncountered++;
                ThunderwithR++;
                ThunderwithL = ThunderwithR;
                if (ThunderwithR >= Strnglen - 1)
                    break;
            }
            if (Bozan != (1 << 4) - 1)
                LinesEncountered = 0;
        }
        clocks2 = clock ();
        TotalRoughSearchTime = clocks2 - clocks1;
        TotalRoughSearchTime++;
        printf ("LinesEncountered: %lu\n", LinesEncountered);
        printf
            ("strstr_GNU_C_Library_hits/strstr_GNU_C_Library_clocks: %lu/%lu\n",
             strstrGNUCLibraryhits >> 4, (long) (TotalRoughSearchTime) >> 4);
        printf ("strstr_GNU_C_Library performance: %ldKB/clock\n",
                (StrnglenTRAVERSED /
                 ((long) (TotalRoughSearchTime) >> 4)) >> 10);
        printf ("StrnglenTRAVERSED: %lu bytes\n", StrnglenTRAVERSED);
// 8]

// 9[
        clocks1 = clock ();
        for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
        {
//Search area is between Strng[0] .. Strng[n-1]
            StrnglenTRAVERSED = 0;  // Only traversed chars i.e. real
            ThunderwithL = 0;
            ThunderwithR = 0;
            for (;;) {
                while (Strng[ThunderwithR] != 10 && ThunderwithR < Strnglen - 1) {
                    ThunderwithR++;
                }
                FoundInPTR =
                    Railgun (&Strng[ThunderwithL], &Pattern[0],
                             ThunderwithR - ThunderwithL, Patternlen);
                if (FoundInPTR != NULL) {
                    Karp_Rabin_Kaze_4_OCTETShits++;
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (FoundInPTR - &Strng[ThunderwithL]);
                }
                else
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (ThunderwithR - ThunderwithL);
                LinesEncountered++;
                ThunderwithR++;
                ThunderwithL = ThunderwithR;
                if (ThunderwithR >= Strnglen - 1)
                    break;
            }
            if (Bozan != (1 << 4) - 1)
                LinesEncountered = 0;
        }
        clocks2 = clock ();
        TotalRoughSearchTime = clocks2 - clocks1;
        TotalRoughSearchTime++;
        printf ("LinesEncountered: %lu\n", LinesEncountered);
        printf ("Railgun_hits/Railgun_clocks: %lu/%lu\n",
                Karp_Rabin_Kaze_4_OCTETShits >> 4,
                (long) (TotalRoughSearchTime) >> 4);
        printf ("Railgun performance: %ldKB/clock\n",
                (StrnglenTRAVERSED /
                 ((long) (TotalRoughSearchTime) >> 4)) >> 10);
        printf ("StrnglenTRAVERSED: %lu bytes\n", StrnglenTRAVERSED);
// 9]

// +[
        clocks1 = clock ();
        for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
        {
//Search area is between Strng[0] .. Strng[n-1]
            StrnglenTRAVERSED = 0;  // Only traversed chars i.e. real
            ThunderwithL = 0;
            ThunderwithR = 0;
            for (;;) {
                while (Strng[ThunderwithR] != 10 && ThunderwithR < Strnglen - 1) {
                    ThunderwithR++;
                }
                FoundInPTR =
                    Railgun_Quadruplet (&Strng[ThunderwithL], &Pattern[0],
                                        ThunderwithR - ThunderwithL,
                                        Patternlen);
                if (FoundInPTR != NULL) {
                    Karp_Rabin_Kaze_4_OCTETShits_DOUBLET++;
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (FoundInPTR - &Strng[ThunderwithL]);
                }
                else
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (ThunderwithR - ThunderwithL);
                LinesEncountered++;
                ThunderwithR++;
                ThunderwithL = ThunderwithR;
                if (ThunderwithR >= Strnglen - 1)
                    break;
            }
            if (Bozan != (1 << 4) - 1)
                LinesEncountered = 0;
        }
        clocks2 = clock ();
        TotalRoughSearchTime = clocks2 - clocks1;
        TotalRoughSearchTime++;
        printf ("LinesEncountered: %lu\n", LinesEncountered);
        printf ("Railgun_Quadruplet_hits/Railgun_Quadruplet_clocks: %lu/%lu\n",
                Karp_Rabin_Kaze_4_OCTETShits_DOUBLET >> 4,
                (long) (TotalRoughSearchTime) >> 4);
        printf ("Railgun_Quadruplet performance: %ldKB/clock\n",
                (StrnglenTRAVERSED /
                 ((long) (TotalRoughSearchTime) >> 4)) >> 10);
        printf ("StrnglenTRAVERSED: %lu bytes\n", StrnglenTRAVERSED);
// +]

// Z[
        clocks1 = clock ();
        for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
        {
//Search area is between Strng[0] .. Strng[n-1]
            StrnglenTRAVERSED = 0;  // Only traversed chars i.e. real
            ThunderwithL = 0;
            ThunderwithR = 0;
            for (;;) {
                while (Strng[ThunderwithR] != 10 && ThunderwithR < Strnglen - 1) {
                    ThunderwithR++;
                }
                FoundInPTR =
                    Railgun_Quadruplet_6pp (&Strng[ThunderwithL], &Pattern[0],
                                            ThunderwithR - ThunderwithL,
                                            Patternlen);
                if (FoundInPTR != NULL) {
                    Railgun_Quadruplet_6pp_GO++;
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (FoundInPTR - &Strng[ThunderwithL]);
                }
                else
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (ThunderwithR - ThunderwithL);
                LinesEncountered++;
                ThunderwithR++;
                ThunderwithL = ThunderwithR;
                if (ThunderwithR >= Strnglen - 1)
                    break;
            }
            if (Bozan != (1 << 4) - 1)
                LinesEncountered = 0;
        }
        clocks2 = clock ();
        TotalRoughSearchTime = clocks2 - clocks1;
        TotalRoughSearchTime++;
        printf ("LinesEncountered: %lu\n", LinesEncountered);
        printf
            ("Railgun_Quadruplet_6pp_hits/Railgun_Quadruplet_6pp_clocks: %lu/%lu\n",
             Railgun_Quadruplet_6pp_GO >> 4,
             (long) (TotalRoughSearchTime) >> 4);
        printf ("Railgun_Quadruplet_6pp performance: %ldKB/clock\n",
                (StrnglenTRAVERSED /
                 ((long) (TotalRoughSearchTime) >> 4)) >> 10);
        printf ("StrnglenTRAVERSED: %lu bytes\n", StrnglenTRAVERSED);
// Z]

// 6[
        clocks1 = clock ();
        for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
        {
//Search area is between Strng[0] .. Strng[n-1]
            StrnglenTRAVERSED = 0;  // Only traversed chars i.e. real
            ThunderwithL = 0;
            ThunderwithR = 0;
            for (;;) {
                while (Strng[ThunderwithR] != 10 && ThunderwithR < Strnglen - 1) {
                    ThunderwithR++;
                }
                FoundInPTR =
                    KarpRabinKaze_BOOSTED (&Strng[ThunderwithL], &Pattern[0],
                                           ThunderwithR - ThunderwithL,
                                           Patternlen);
                if (FoundInPTR != NULL) {
                    KarpRabinKaze_BOOSTEDhits++;
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (FoundInPTR - &Strng[ThunderwithL]);
                }
                else
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (ThunderwithR - ThunderwithL);
                LinesEncountered++;
                ThunderwithR++;
                ThunderwithL = ThunderwithR;
                if (ThunderwithR >= Strnglen - 1)
                    break;
            }
            if (Bozan != (1 << 4) - 1)
                LinesEncountered = 0;
        }
        clocks2 = clock ();
        TotalRoughSearchTime = clocks2 - clocks1;
        TotalRoughSearchTime++;
        printf ("LinesEncountered: %lu\n", LinesEncountered);
        printf
            ("KarpRabinKaze_BOOSTED_hits/KarpRabinKaze_BOOSTED_clocks: %lu/%lu\n",
             KarpRabinKaze_BOOSTEDhits >> 4,
             (long) (TotalRoughSearchTime) >> 4);
        printf ("KarpRabinKaze_BOOSTED performance: %ldKB/clock\n",
                (StrnglenTRAVERSED /
                 ((long) (TotalRoughSearchTime) >> 4)) >> 10);
        printf ("StrnglenTRAVERSED: %lu bytes\n", StrnglenTRAVERSED);
// 6]

// 2[
        clocks1 = clock ();
        for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
        {
//Search area is between Strng[0] .. Strng[n-1]
            StrnglenTRAVERSED = 0;  // Only traversed chars i.e. real
            ThunderwithL = 0;
            ThunderwithR = 0;
            for (;;) {
                while (Strng[ThunderwithR] != 10 && ThunderwithR < Strnglen - 1) {
                    ThunderwithR++;
                }
                FoundInPTR =
                    KarpRabinKaze (&Strng[ThunderwithL], &Pattern[0],
                                   ThunderwithR - ThunderwithL, Patternlen);
                if (FoundInPTR != NULL) {
                    KarpRabinKazehits++;
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (FoundInPTR - &Strng[ThunderwithL]);
                }
                else
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (ThunderwithR - ThunderwithL);
                LinesEncountered++;
                ThunderwithR++;
                ThunderwithL = ThunderwithR;
                if (ThunderwithR >= Strnglen - 1)
                    break;
            }
            if (Bozan != (1 << 4) - 1)
                LinesEncountered = 0;
        }
        clocks2 = clock ();
        TotalRoughSearchTime = clocks2 - clocks1;
        TotalRoughSearchTime++;
        printf ("LinesEncountered: %lu\n", LinesEncountered);
        printf ("KarpRabinKaze_hits/KarpRabinKaze_clocks: %lu/%lu\n",
                KarpRabinKazehits >> 4, (long) (TotalRoughSearchTime) >> 4);
        printf ("KarpRabinKaze performance: %ldKB/clock\n",
                (StrnglenTRAVERSED /
                 ((long) (TotalRoughSearchTime) >> 4)) >> 10);
        printf ("StrnglenTRAVERSED: %lu bytes\n", StrnglenTRAVERSED);
// 2]

// 3[
        clocks1 = clock ();
        for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
        {
//Search area is between Strng[0] .. Strng[n-1]
            StrnglenTRAVERSED = 0;  // Only traversed chars i.e. real
            ThunderwithL = 0;
            ThunderwithR = 0;
            for (;;) {
                while (Strng[ThunderwithR] != 10 && ThunderwithR < Strnglen - 1) {
                    ThunderwithR++;
                }
                FoundIn =
                    Karp_Rabin (&Strng[ThunderwithL], &Pattern[0],
                                ThunderwithR - ThunderwithL, Patternlen);
                if (FoundIn != -1) {
                    KarpRabinhits++;
                    StrnglenTRAVERSED = StrnglenTRAVERSED + FoundIn;
                }
                else
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (ThunderwithR - ThunderwithL);
                LinesEncountered++;
                ThunderwithR++;
                ThunderwithL = ThunderwithR;
                if (ThunderwithR >= Strnglen - 1)
                    break;
            }
            if (Bozan != (1 << 4) - 1)
                LinesEncountered = 0;
        }
        clocks2 = clock ();
        TotalRoughSearchTime = clocks2 - clocks1;
        TotalRoughSearchTime++;
        printf ("LinesEncountered: %lu\n", LinesEncountered);
        printf ("Karp_Rabin_hits/Karp_Rabin_clocks: %lu/%lu\n",
                KarpRabinhits >> 4, (long) (TotalRoughSearchTime) >> 4);
        printf ("Karp_Rabin performance: %ldKB/clock\n",
                (StrnglenTRAVERSED /
                 ((long) (TotalRoughSearchTime) >> 4)) >> 10);
        printf ("StrnglenTRAVERSED: %lu bytes\n", StrnglenTRAVERSED);
// 3]

// 1[
        clocks1 = clock ();
        for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
        {
//Search area is between Strng[0] .. Strng[n-1]
            StrnglenTRAVERSED = 0;  // Only traversed chars i.e. real
            ThunderwithL = 0;
            ThunderwithR = 0;
            for (;;) {
                while (Strng[ThunderwithR] != 10 && ThunderwithR < Strnglen - 1) {
                    ThunderwithR++;
                }
                FoundIn =
                    Brute_Force_Dummy (&Strng[ThunderwithL], &Pattern[0],
                                       ThunderwithR - ThunderwithL, Patternlen);
                if (FoundIn != -1) {
                    BruteForceDummyhits++;
                    StrnglenTRAVERSED = StrnglenTRAVERSED + FoundIn;
                }
                else
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (ThunderwithR - ThunderwithL);
                LinesEncountered++;
                ThunderwithR++;
                ThunderwithL = ThunderwithR;
                if (ThunderwithR >= Strnglen - 1)
                    break;
            }
            if (Bozan != (1 << 4) - 1)
                LinesEncountered = 0;
        }
        clocks2 = clock ();
        TotalRoughSearchTime = clocks2 - clocks1;
        TotalRoughSearchTime++;
        printf ("LinesEncountered: %lu\n", LinesEncountered);
        printf ("Brute_Force_Dummy_hits/Brute_Force_Dummy_clocks: %lu/%lu\n",
                BruteForceDummyhits >> 4, (long) (TotalRoughSearchTime) >> 4);
        printf ("Brute_Force_Dummy performance: %ldKB/clock\n",
                (StrnglenTRAVERSED /
                 ((long) (TotalRoughSearchTime) >> 4)) >> 10);
        printf ("StrnglenTRAVERSED: %lu bytes\n", StrnglenTRAVERSED);
// 1]

// 4[
        clocks1 = clock ();
        for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
        {
//Search area is between Strng[0] .. Strng[n-1]
            StrnglenTRAVERSED = 0;  // Only traversed chars i.e. real
            ThunderwithL = 0;
            ThunderwithR = 0;
            for (;;) {
                while (Strng[ThunderwithR] != 10 && ThunderwithR < Strnglen - 1) {
                    ThunderwithR++;
                }
                FoundIn =
                    HORSPOOL (&Strng[ThunderwithL], &Pattern[0],
                              ThunderwithR - ThunderwithL, Patternlen);
                if (FoundIn != -1) {
                    HORSPOOLhits++;
                    StrnglenTRAVERSED = StrnglenTRAVERSED + FoundIn;
                }
                else
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (ThunderwithR - ThunderwithL);
                LinesEncountered++;
                ThunderwithR++;
                ThunderwithL = ThunderwithR;
                if (ThunderwithR >= Strnglen - 1)
                    break;
            }
            if (Bozan != (1 << 4) - 1)
                LinesEncountered = 0;
        }
        clocks2 = clock ();
        TotalRoughSearchTime = clocks2 - clocks1;
        TotalRoughSearchTime++;
        printf ("LinesEncountered: %lu\n", LinesEncountered);
        printf
            ("Boyer-Moore-Horspool_hits/Boyer-Moore-Horspool_clocks: %lu/%lu\n",
             HORSPOOLhits >> 4, (long) (TotalRoughSearchTime) >> 4);
        printf ("Boyer-Moore-Horspool performance: %ldKB/clock\n",
                (StrnglenTRAVERSED /
                 ((long) (TotalRoughSearchTime) >> 4)) >> 10);
        printf ("StrnglenTRAVERSED: %lu bytes\n", StrnglenTRAVERSED);
// 4]

// 5[
        clocks1 = clock ();
        for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
        {
//Search area is between Strng[0] .. Strng[n-1]
            StrnglenTRAVERSED = 0;  // Only traversed chars i.e. real
            ThunderwithL = 0;
            ThunderwithR = 0;
            for (;;) {
                while (Strng[ThunderwithR] != 10 && ThunderwithR < Strnglen - 1) {
                    ThunderwithR++;
                }
                FoundIn =
                    Boyer_Moore_Horspool_Kaze (&Strng[ThunderwithL],
                                               &Pattern[0],
                                               ThunderwithR - ThunderwithL,
                                               Patternlen);
                if (FoundIn != -1) {
                    HORSPOOL_Kazehits++;
                    StrnglenTRAVERSED = StrnglenTRAVERSED + FoundIn;
                }
                else
                    StrnglenTRAVERSED =
                        StrnglenTRAVERSED + (ThunderwithR - ThunderwithL);
                LinesEncountered++;
                ThunderwithR++;
                ThunderwithL = ThunderwithR;
                if (ThunderwithR >= Strnglen - 1)
                    break;
            }
            if (Bozan != (1 << 4) - 1)
                LinesEncountered = 0;
        }
        clocks2 = clock ();
        TotalRoughSearchTime = clocks2 - clocks1;
        TotalRoughSearchTime++;
        printf ("LinesEncountered: %lu\n", LinesEncountered);
        printf
            ("Boyer_Moore_Horspool_Kaze_hits/Boyer_Moore_Horspool_Kaze_clocks: %lu/%lu\n",
             HORSPOOL_Kazehits >> 4, (long) (TotalRoughSearchTime) >> 4);
        printf ("Boyer_Moore_Horspool_Kaze performance: %ldKB/clock\n",
                (StrnglenTRAVERSED /
                 ((long) (TotalRoughSearchTime) >> 4)) >> 10);
        printf ("StrnglenTRAVERSED: %lu bytes\n", StrnglenTRAVERSED);
// 5]



// DUMBO ... [
        printf ("\nDUMBO 8x2 ...\n");
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 2; j++) {

                strcpy (Pattern, DumboBox[i][j]);
                Patternlen = strlen (&Pattern[0]);

                printf
                    ("\nSearching for Pattern('%s',%dbytes) into String(%ldbytes) line-by-line ...\n\n",
                     Pattern, Patternlen, Strnglen);

// 7[
                clocks1 = clock ();
                for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
                {
                    strstrMicrosofthits = 0;
//Search area is between Strng[0] .. Strng[n-1]
                    StrnglenTRAVERSED = 0;  // Only traversed chars i.e. real
                    ThunderwithL = 0;
                    ThunderwithR = 0;
                    for (;;) {
                        while (Strng[ThunderwithR] != 10
                               && ThunderwithR < Strnglen - 1) {
                            ThunderwithR++;
                        }
                        FoundInPTR =
                            strstr_Microsoft (&Strng[ThunderwithL],
                                              &Pattern[0]);
                        if (FoundInPTR != NULL) {
                            strstrMicrosofthits++;
                            StrnglenTRAVERSED =
                                StrnglenTRAVERSED + (FoundInPTR -
                                                     &Strng[ThunderwithL]);
                        }
                        else
                            StrnglenTRAVERSED =
                                StrnglenTRAVERSED + (ThunderwithR -
                                                     ThunderwithL);
                        LinesEncountered++;
                        ThunderwithR++;
                        ThunderwithL = ThunderwithR;
                        if (ThunderwithR >= Strnglen - 1)
                            break;
                    }
                    if (Bozan != (1 << 4) - 1)
                        LinesEncountered = 0;
                }
                clocks2 = clock ();
                TotalRoughSearchTime = clocks2 - clocks1;
                TotalRoughSearchTime++;
                printf ("LinesEncountered: %lu\n", LinesEncountered);
                printf
                    ("strstr_Microsoft_hits/strstr_Microsoft_clocks: %lu/%lu\n",
                     strstrMicrosofthits, (long) (TotalRoughSearchTime) >> 4);
                printf ("strstr_Microsoft performance: %ldKB/clock\n",
                        (StrnglenTRAVERSED /
                         ((long) (TotalRoughSearchTime) >> 4)) >> 10);
                printf ("StrnglenTRAVERSED: %lu bytes\n", StrnglenTRAVERSED);
// 7]

// 8[
                clocks1 = clock ();
                for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
                {
                    strstrGNUCLibraryhits = 0;
//Search area is between Strng[0] .. Strng[n-1]
                    StrnglenTRAVERSED = 0;  // Only traversed chars i.e. real
                    ThunderwithL = 0;
                    ThunderwithR = 0;
                    for (;;) {
                        while (Strng[ThunderwithR] != 10
                               && ThunderwithR < Strnglen - 1) {
                            ThunderwithR++;
                        }
                        FoundInPTR =
                            strstr_GNU_C_Library (&Strng[ThunderwithL],
                                                  &Pattern[0]);
                        if (FoundInPTR != 0) {
                            strstrGNUCLibraryhits++;
                            StrnglenTRAVERSED =
                                StrnglenTRAVERSED + (FoundInPTR -
                                                     &Strng[ThunderwithL]);
                        }
                        else
                            StrnglenTRAVERSED =
                                StrnglenTRAVERSED + (ThunderwithR -
                                                     ThunderwithL);
                        LinesEncountered++;
                        ThunderwithR++;
                        ThunderwithL = ThunderwithR;
                        if (ThunderwithR >= Strnglen - 1)
                            break;
                    }
                    if (Bozan != (1 << 4) - 1)
                        LinesEncountered = 0;
                }
                clocks2 = clock ();
                TotalRoughSearchTime = clocks2 - clocks1;
                TotalRoughSearchTime++;
                printf ("LinesEncountered: %lu\n", LinesEncountered);
                printf
                    ("strstr_GNU_C_Library_hits/strstr_GNU_C_Library_clocks: %lu/%lu\n",
                     strstrGNUCLibraryhits, (long) (TotalRoughSearchTime) >> 4);
                printf ("strstr_GNU_C_Library performance: %ldKB/clock\n",
                        (StrnglenTRAVERSED /
                         ((long) (TotalRoughSearchTime) >> 4)) >> 10);
                printf ("StrnglenTRAVERSED: %lu bytes\n", StrnglenTRAVERSED);
// 8]

// 9[
                clocks1 = clock ();
                for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
                {
                    Karp_Rabin_Kaze_4_OCTETShits = 0;
//Search area is between Strng[0] .. Strng[n-1]
                    StrnglenTRAVERSED = 0;  // Only traversed chars i.e. real
                    ThunderwithL = 0;
                    ThunderwithR = 0;
                    for (;;) {
                        while (Strng[ThunderwithR] != 10
                               && ThunderwithR < Strnglen - 1) {
                            ThunderwithR++;
                        }
                        FoundInPTR =
                            Railgun (&Strng[ThunderwithL], &Pattern[0],
                                     ThunderwithR - ThunderwithL, Patternlen);
                        if (FoundInPTR != NULL) {
                            Karp_Rabin_Kaze_4_OCTETShits++;
                            StrnglenTRAVERSED =
                                StrnglenTRAVERSED + (FoundInPTR -
                                                     &Strng[ThunderwithL]);
                        }
                        else
                            StrnglenTRAVERSED =
                                StrnglenTRAVERSED + (ThunderwithR -
                                                     ThunderwithL);
                        LinesEncountered++;
                        ThunderwithR++;
                        ThunderwithL = ThunderwithR;
                        if (ThunderwithR >= Strnglen - 1)
                            break;
                    }
                    if (Bozan != (1 << 4) - 1)
                        LinesEncountered = 0;
                }
                clocks2 = clock ();
                TotalRoughSearchTime = clocks2 - clocks1;
                TotalRoughSearchTime++;
                printf ("LinesEncountered: %lu\n", LinesEncountered);
                printf ("Railgun_hits/Railgun_clocks: %lu/%lu\n",
                        Karp_Rabin_Kaze_4_OCTETShits,
                        (long) (TotalRoughSearchTime) >> 4);
                printf ("Railgun performance: %ldKB/clock\n",
                        (StrnglenTRAVERSED /
                         ((long) (TotalRoughSearchTime) >> 4)) >> 10);
                printf ("StrnglenTRAVERSED: %lu bytes\n", StrnglenTRAVERSED);
// 9]

// +[
                clocks1 = clock ();
                for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
                {
                    Karp_Rabin_Kaze_4_OCTETShits_DOUBLET = 0;
//Search area is between Strng[0] .. Strng[n-1]
                    StrnglenTRAVERSED = 0;  // Only traversed chars i.e. real
                    ThunderwithL = 0;
                    ThunderwithR = 0;
                    for (;;) {
                        while (Strng[ThunderwithR] != 10
                               && ThunderwithR < Strnglen - 1) {
                            ThunderwithR++;
                        }
                        FoundInPTR =
                            Railgun_Quadruplet (&Strng[ThunderwithL],
                                                &Pattern[0],
                                                ThunderwithR - ThunderwithL,
                                                Patternlen);
                        if (FoundInPTR != NULL) {
                            Karp_Rabin_Kaze_4_OCTETShits_DOUBLET++;
                            StrnglenTRAVERSED =
                                StrnglenTRAVERSED + (FoundInPTR -
                                                     &Strng[ThunderwithL]);
                        }
                        else
                            StrnglenTRAVERSED =
                                StrnglenTRAVERSED + (ThunderwithR -
                                                     ThunderwithL);
                        LinesEncountered++;
                        ThunderwithR++;
                        ThunderwithL = ThunderwithR;
                        if (ThunderwithR >= Strnglen - 1)
                            break;
                    }
                    if (Bozan != (1 << 4) - 1)
                        LinesEncountered = 0;
                }
                clocks2 = clock ();
                TotalRoughSearchTime = clocks2 - clocks1;
                TotalRoughSearchTime++;
                printf ("LinesEncountered: %lu\n", LinesEncountered);
                printf
                    ("Railgun_Quadruplet_hits/Railgun_Quadruplet_clocks: %lu/%lu\n",
                     Karp_Rabin_Kaze_4_OCTETShits_DOUBLET,
                     (long) (TotalRoughSearchTime) >> 4);
                printf ("Railgun_Quadruplet performance: %ldKB/clock\n",
                        (StrnglenTRAVERSED /
                         ((long) (TotalRoughSearchTime) >> 4)) >> 10);
                printf ("StrnglenTRAVERSED: %lu bytes\n", StrnglenTRAVERSED);
// +]

// Z[
                clocks1 = clock ();
                for (Bozan = 0; Bozan < (1 << 4); Bozan++)  // 16 times, at end >>4
                {
                    Karp_Rabin_Kaze_4_OCTETShits_DOUBLET = 0;
//Search area is between Strng[0] .. Strng[n-1]
                    StrnglenTRAVERSED = 0;  // Only traversed chars i.e. real
                    ThunderwithL = 0;
                    ThunderwithR = 0;
                    for (;;) {
                        while (Strng[ThunderwithR] != 10
                               && ThunderwithR < Strnglen - 1) {
                            ThunderwithR++;
                        }
                        FoundInPTR =
                            Railgun_Quadruplet_6pp (&Strng[ThunderwithL],
                                                    &Pattern[0],
                                                    ThunderwithR - ThunderwithL,
                                                    Patternlen);
                        if (FoundInPTR != NULL) {
                            Karp_Rabin_Kaze_4_OCTETShits_DOUBLET++;
                            StrnglenTRAVERSED =
                                StrnglenTRAVERSED + (FoundInPTR -
                                                     &Strng[ThunderwithL]);
                        }
                        else
                            StrnglenTRAVERSED =
                                StrnglenTRAVERSED + (ThunderwithR -
                                                     ThunderwithL);
                        LinesEncountered++;
                        ThunderwithR++;
                        ThunderwithL = ThunderwithR;
                        if (ThunderwithR >= Strnglen - 1)
                            break;
                    }
                    if (Bozan != (1 << 4) - 1)
                        LinesEncountered = 0;
                }
                clocks2 = clock ();
                TotalRoughSearchTime = clocks2 - clocks1;
                TotalRoughSearchTime++;
                printf ("LinesEncountered: %lu\n", LinesEncountered);
                printf
                    ("Railgun_Quadruplet_6pp_hits/Railgun_Quadruplet_6pp_clocks: %lu/%lu\n",
                     Karp_Rabin_Kaze_4_OCTETShits_DOUBLET,
                     (long) (TotalRoughSearchTime) >> 4);
                printf ("Railgun_Quadruplet_6pp performance: %ldKB/clock\n",
                        (StrnglenTRAVERSED /
                         ((long) (TotalRoughSearchTime) >> 4)) >> 10);
                printf ("StrnglenTRAVERSED: %lu bytes\n", StrnglenTRAVERSED);
// Z]

            }
        }
// DUMBO ... ]

    }                           //if (argc==1) {

//exit(1); // Comment it to get on!

    return (0);
}
