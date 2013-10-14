// Scheherezade -> Hasherezade
#define ROL(x, n) (((x) << (n)) | ((x) >> (32-(n))))
#define HashTableSize 18
//
// Revision: 1, 2012-Feb-01, the main disadvantage: the preprocessing overhead PLUS a hasher.
// Caution: For better speed the case 'if (cbPattern==1)' was removed, so Pattern must be longer than 1 char.
#ifndef STR_X
#include <stdint.h>
char *Railgun7h(char *tgt, int tgtlen, char *pat, int patlen);
#endif
char *Railgun7h(char *target, int tgtlen, char *pattern, int patlen)
{
    uint8_t *tgt = (uint8_t*)target;
    uint8_t *pat = (uint8_t*)pattern;
    uint8_t *tgtMax = tgt + tgtlen;
    uint32_t ulHashPattern;
    uint32_t ulHashTarget;
    int32_t count;
    int32_t countSTATIC;

    uint8_t SINGLET;
    uint32_t Quadruplet2nd;
    uint32_t Quadruplet3rd;
    uint32_t Quadruplet4th;

    //uint32_t AdvanceHopperGrass;
    int AdvanceHopperGrass;

    int i;                     //BMH needed
    int a, j;
    uint32_t bm_bc[256];        //BMH needed
    uint32_t bm_bc2nd[256];     //BMS needed
    uint8_t bm_Horspool_Order2[256 * 256];  //BMHSS(Elsiane) needed, 'char' limits patterns to 255, if 'long' then table becomes 256KB, grrr.
    uint8_t bm_Hasherezade_HASH[1 << (HashTableSize)];  // Jesteressing 8bytes (Horspool Order 8) for fast lookup, should be bitwise (i.e. 8times smaller) since it says yes/no for presence.
    uint32_t hash32;
    uint32_t Gulliver;          // or uint8_t or uint16_t

    if (patlen > tgtlen)
        return 0;
    if (patlen == 1)
        return memchr(tgt, *pat, tgtlen);
    if (patlen == 2) {
        tgt += 2;
        ulHashPattern =((*(char *)(pat)) << 8) + *(pat +(2 - 1));
        for (;;) {
            if (ulHashPattern == ((uint32_t)(*(uint8_t *)(tgt - 2)) << 8) +(uint32_t)*(uint8_t*)(tgt - 1))
                return (char*)(tgt - 2);
            if ((char)(ulHashPattern >> 8) != *(tgt - 1))
                tgt++;
            tgt++;
            if (tgt > tgtMax)
                return NULL;
        }
    } else if (patlen == 3) {
        tgt += 3;
        ulHashPattern =((*(char *)(pat)) << 8) + *(pat +(3 - 1));
        for (;;) {
            if (ulHashPattern == ((uint32_t)(*(uint8_t *)(tgt - 3)) << 8) +(uint32_t)*(uint8_t*)(tgt - 1)) {
                if (*(uint8_t *)(pat + 1) == *(uint8_t *)(tgt - 2))
                    return(char*)(tgt - 3);
            }
            if ((char)(ulHashPattern >> 8) != *(tgt - 2))
                tgt++;
            tgt++;
            if (tgt > tgtMax)
                return NULL;
        }
    } else if (tgtlen < 961) {// || patlen < 11) {    // 1024 - 64 + 1 ?!
        tgt = tgt + patlen;
        ulHashPattern = *(uint32_t *) (pat);

        SINGLET = ulHashPattern & 0xFF;
        Quadruplet2nd = SINGLET << 8;
        Quadruplet3rd = SINGLET << 16;
        Quadruplet4th = SINGLET << 24;

        for (;;) {
            AdvanceHopperGrass = 0;
            ulHashTarget = *(uint32_t *) (tgt - patlen);

            if (ulHashPattern == ulHashTarget) {    // Three unnecessary comparisons here, but 'AdvanceHopperGrass' must be calculated - it has a higher priority.
                count = patlen - 1;
                while (count
                       && *(uint8_t*) (pat + (patlen - count)) ==
                       *(uint8_t*) (tgt - count)) {
                    if (patlen - 1 == AdvanceHopperGrass + count
                        && SINGLET != *(uint8_t*) (tgt - count))
                        AdvanceHopperGrass++;
                    count--;
                }
                if (count == 0)
                    return (char*)tgt-patlen;
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

            tgt = tgt + AdvanceHopperGrass;
            if (tgt > tgtMax)
                return 0;
        }
    } else {                  // tgtlen >= 961
        countSTATIC = patlen - 2 - 2;

        for (a = 0; a < 256; a++) {
            bm_bc[a] = patlen;
            bm_bc2nd[a] = patlen + 1;
        }
        for (j = 0; j < patlen - 1; j++)
            bm_bc[(uint8_t)pat[j]] = patlen - j - 1;
        for (j = 0; j < patlen; j++)
            bm_bc2nd[(uint8_t)pat[j]] = patlen - j;

        ulHashPattern = *(uint32_t *) (pat);    // First four bytes
        //ulHashTarget = *(uint16_t *)(pat+patlen-1-1); // Last two bytes

        AdvanceHopperGrass = 0;
        i = 0;

        // Elsiane r.2  [
        for (a = 0; a < 256 * 256; a++) {
            bm_Horspool_Order2[a] = patlen - 1;
        }                   // patlen-(Order-1) for Horspool; 'memset' if not optimized

        // alfalfa 7 long 6 BBs (al lf fa al lf fa) 3 distinct BBs (al lf fa)
        // fast 4 0-1-2 fa as st
        for (j = 0; j < patlen - 1; j++)
            bm_Horspool_Order2[*(uint16_t *) (pat + j)] = j;  // Rightmost appearance/position is needed

        // Elsiane r.2  ]

        if (patlen > 10) {
            // Hasherezade r.1 [
            // OSHO.TXT has 00,046,486 03bytes distinct BBs
            // OSHO.TXT has 00,248,019 04bytes distinct BBs
            // OSHO.TXT has 00,855,682 05bytes distinct BBs
            // OSHO.TXT has 02,236,138 06bytes distinct BBs
            // OSHO.TXT has 04,803,152 07bytes distinct BBs
            // OSHO.TXT has 08,956,496 08bytes distinct BBs to be hashed in 18bit i.e. 256KB i.e. 262,144 slots i.e. 34 vs 1.
            // OSHO.TXT has 15,006,172 09bytes distinct BBs
            // OSHO.TXT has 22,992,127 10bytes distinct BBs
            // Note: BB stands for Building-Block (also suffix)

            for (a = 0; a < 1 << (HashTableSize); a++) {
                bm_Hasherezade_HASH[a] = 0;
            }               // to-do: bit to replace byte; 'memset' if not optimized
            // patlen - Order + 1 i.e. number of BBs for 11 'fastest fox' 11-8+1=4: 'fastest ', 'astest f', 'stest fo', 'test fox'
            for (j = 0; j < patlen - 8 + 1; j++) {
                hash32 =
                    (2166136261U ^
                     (ROL (*(uint32_t *) (pat + j), 5) ^
                      *(uint32_t *) (pat + j + 4))) * 709607;
                bm_Hasherezade_HASH[(hash32 ^ (hash32 >> 16)) &
                                    ((1 << (HashTableSize)) - 1)] = 1;
/*
for (a=0; a<8; a++)
printf("%c",*(uint8_t*)(pat+j+a) );
printf(" %lu\n",( hash32 ^ (hash32 >> 16) ) & ( (1<<(HashTableSize))-1 ));
//Input Pattern(up to 19+2000 chars): and every day a continuous cleaning goes on
//Doing Search for Pattern(43bytes) into String(206908949bytes) as-one-line ...
BBs      Slot(HashCode for 18bit HashTable)
and ever 117013
nd every 108604
d every  155516
every d 170959
every da 115291
very day 73191
ery day  97042
ry day a 83793
y day a  11244
day a c 115855
day a co 101797
ay a con 222568
y a cont 29130
a conti 20978
a contin 258405
continu 252691
continuo 123607
ontinuou 56546
ntinuous 135857
tinuous  15332
inuous c 250584
nuous cl 48224
uous cle 106616
ous clea 137020
us clean 35751
s cleani 178989
cleanin 213855
cleaning 63337
leaning  97138
eaning g 62366
aning go 247590
ning goe 36571
ing goes 41142
ng goes  228365
g goes o 229696
goes on 176852
*/
            }
            // Hasherezade r.1 ]

            while (i <= tgtlen - patlen - 1) {  // -1 because Sunday is used
                Gulliver =
                    bm_Horspool_Order2[*(uint16_t *)
                                       &tgt[i + patlen - 1 - 1]];

                if ((int)Gulliver == patlen - 2) {   // CASE #1: means the pair (char order 2) is found
                    if (*(uint32_t *) & tgt[i] == ulHashPattern) {
                        count = countSTATIC;    // Last two chars already matched, to be fixed with -2
                        while (count != 0
                               && *(uint8_t*) (pat + (countSTATIC - count) +
                                             4) ==
                               *(uint8_t*) (&tgt[i] + (countSTATIC - count) +
                                          4))
                            count--;
                        if (count == 0)
                            return (char*)tgt+i;
                    }
                    //i = i + 1; // r.1, obviuosly this is the worst skip so turning to 'SunHorse': lines below
                    if (bm_bc[(uint8_t) tgt[i + patlen - 1]] <
                        bm_bc2nd[(uint8_t) tgt[i + (patlen)]])
                        Gulliver = bm_bc2nd[(uint8_t) tgt[i + (patlen)]];
                    else
                        Gulliver = bm_bc[(uint8_t) tgt[i + patlen - 1]];
                }
                else if ((int)Gulliver != patlen - 1)    // CASE #2: if equal means the pair (char order 2) is not found i.e. Gulliver remains intact, skip the whole pattern and fall back (Order-1) chars i.e. one char for Order 2
                    Gulliver = patlen - Gulliver - 2;   // CASE #3: the pair is found and not as suffix i.e. rightmost position

                // The goal: to jump when the rightmost 8bytes (Order 8 Horspool) of window do not look like any of Needle prefixes i.e. are not to be found. This maximum jump equals patlen-(Order-1) or 11-(8-1)=4 for 'fastest fox' - a small one but for Needle 31 bytes the jump equals 31-(8-1)=24
                if ((int)Gulliver < patlen - (8 - 1)) {
                    hash32 =
                        (2166136261U^
                         (ROL (*(uint32_t *) (tgt + i + patlen - 8), 5) ^
                          *(uint32_t *) (tgt + i + patlen - 8 +
                                         4))) * 709607;
                    if (bm_Hasherezade_HASH
                        [(hash32 ^ (hash32 >> 16)) &
                         ((1 << (HashTableSize)) - 1)] == 0)
                        Gulliver = patlen - (8 - 1);
                }
                i = i + Gulliver;
                AdvanceHopperGrass++;
/*
; 4155 :                                // The goal: to jump when the rightmost 8bytes (Order 8 Horspool) of window do not look like any of Needle prefixes i.e. are not to be found. This maximum jump equals patlen-(Order-1) or 11-(8-1)=4 for 'fastest fox' - a small one but for Needle 31 bytes the jump equals 31-(8-1)=24
; 4156 :                                if ((int)Gulliver < patlen-(8-1)) {

01f16 8d 43 f9         lea     eax, DWORD PTR [ebx-7]
01f19 3b c8            cmp     ecx, eax
01f1b 73 30            jae     SHORT $LN18@Railgun_Qu@8

; 4157 :                                        hash32 = (2166136261U^ (ROL(*(uint32_t *)(tgt+i+patlen-8),5)^*(uint32_t *)(tgt+i+patlen-8+4))) * 709607;

01f1d 8b 44 32 f8      mov     eax, DWORD PTR [edx+esi-8]
01f21 c1 c0 05         rol     eax, 5
01f24 33 44 32 fc      xor     eax, DWORD PTR [edx+esi-4]
01f28 35 c5 9d 1c 81   xor     eax, -2128831035       ; 811c9dc5H
01f2d 69 c0 e7 d3 0a
    00               imul    eax, 709607            ; 000ad3e7H

; 4158 :                                        if ( bm_Hasherezade_HASH[( hash32 ^ (hash32 >> 16) ) & ( (1<<(HashTableSize))-1 )]==0 )

01f33 8b f8            mov     edi, eax
01f35 c1 ef 10         shr     edi, 16                        ; 00000010H
01f38 33 f8            xor     edi, eax
01f3a 81 e7 ff ff 03
    00               and     edi, 262143            ; 0003ffffH
01f40 80 bc 3c 28 08
    01 00 00         cmp     BYTE PTR _bm_Hasherezade_HASH$[esp+edi+329776], 0
01f48 75 03            jne     SHORT $LN18@Railgun_Qu@8

; 4159 :                                                Gulliver = patlen-(8-1);

01f4a 8d 4b f9         lea     ecx, DWORD PTR [ebx-7]
$LN18@Railgun_Qu@8:

; 4160 :                                }
; 4161 :                                        i = i + Gulliver;
; 4162 :                                AdvanceHopperGrass++;
*/
            }

        } else {
            while (i <= tgtlen - patlen - 1) {  // -1 because Sunday is used
                Gulliver =
                    bm_Horspool_Order2[*(uint16_t *)
                                       &tgt[i + patlen - 1 - 1]];

                if ((int)Gulliver == patlen - 2) {   // CASE #1: means the pair (char order 2) is found
                    if (*(uint32_t *) & tgt[i] == ulHashPattern) {
                        count = countSTATIC;    // Last two chars already matched, to be fixed with -2
                        while (count != 0
                               && *(uint8_t*) (pat + (countSTATIC - count) +
                                             4) ==
                               *(uint8_t*) (&tgt[i] + (countSTATIC - count) +
                                          4))
                            count--;
                        if (count == 0)
                            return (char*)tgt+i;
                    }
                    //i = i + 1; // r.1, obviuosly this is the worst skip so turning to 'SunHorse': lines below
                    if (bm_bc[(uint8_t) tgt[i + patlen - 1]] <
                        bm_bc2nd[(uint8_t) tgt[i + (patlen)]])
                        Gulliver = bm_bc2nd[(uint8_t) tgt[i + (patlen)]];
                    else
                        Gulliver = bm_bc[(uint8_t) tgt[i + patlen - 1]];
                }
                else if ((int)Gulliver != patlen - 1)    // CASE #2: if equal means the pair (char order 2) is not found i.e. Gulliver remains intact, skip the whole pattern and fall back (Order-1) chars i.e. one char for Order 2
                    Gulliver = patlen - Gulliver - 2;   // CASE #3: the pair is found and not as suffix i.e. rightmost position

                i = i + Gulliver;

// 32323218 Order 1 Horspool Skip-table A
// 01234568 Order 1 Horspool Skip-table B
// fa af fa af fa as st Order 2 Horspool Skip-table B
//  0  1  2  3  4  5  6
// HIKARIfast
// fafafast
//   fafafast +2 Order 1 'a' vs 't'
//   fafafast +2 = (patlen-SkipB-Order = 8-5-1 = 2) Order 1 'a' vs 't'
//   fafafast +2 = (patlen-SkipB-Order = 8-4-2 = 2) Order 2 'fa' vs 'st' i.e. CASE #3

// 76543218 Order 1 Horspool
// lo on ng gp pa ac ce Order 2 Horspool
//  0  1  2  3  4  5  6
// HIKARIfast
// longpace
//   longpace +2 Order 1 'a' vs 'e'
//        longpace +7 = (patlen-(Order-1) = 8-(2-1) = 7) Order 2 'fa' vs 'ce' i.e. CASE #2

                AdvanceHopperGrass++;
            }
        }                   

        if (i == tgtlen - patlen) {
            if (*(uint32_t *) & tgt[i] == ulHashPattern) {
                count = countSTATIC;
                while (count != 0 && pat[countSTATIC - count + 4] == tgt[i + countSTATIC - count + 4])
                    count--;
                if (count == 0)
                    return (char*)tgt+i;
            }
            AdvanceHopperGrass++;
        }
#if 0
        GlobalSP += (int) ((double) tgtlen / AdvanceHopperGrass * 100);
        GlobalI += AdvanceHopperGrass;
        printf
            ("Skip-Performance(bigger-the-better): %d%%, %d skips/iterations\n",
             (int) ((double) tgtlen / AdvanceHopperGrass * 100),
             AdvanceHopperGrass);
#endif
        return 0;
    }                           //if ( patlen<4)
}
