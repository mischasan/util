#define _rotl_KAZE(x, n) (((x) << (n)) | ((x) >> (32-(n))))
#define HaystackThresholdSekirei 961    // Quadruplet works up to this value, if bigger then BMH2 takes over.
#define NeedleThresholdBIGSekirei 12+40 // Should be bigger than 'HasherezadeOrder'. BMH2 works up to this value (inclusive).
#define HashTableSizeSekirei 17-1   // In fact the real size is -3, because it is BITwise, when 17-3=14 it means 16KB, (17-1)-3=13 it means 8KB.
//Railgun_Sekireigan(char *pbTarget, char *pbPattern, uint32_t cbTarget,
char *
Railgun7s(char *pbTarget, int cbTarget, char *pbPattern, int cbPattern)

{
    char   *pbTargetMax = pbTarget + cbTarget;
    register uint32_t ulHashPattern;
    register uint32_t ulHashTarget;
    signed long count;
    signed long countSTATIC;

    unsigned char SINGLET;
    uint32_t Quadruplet2nd;
    uint32_t Quadruplet3rd;
    uint32_t Quadruplet4th;

    uint32_t AdvanceHopperGrass;

    long    i;                  //BMH needed
    int     a, j;
    unsigned char bm_Horspool_Order2[256 * 256];    // BMHSS(Elsiane) needed, 'char' limits patterns to 255, if 'long' then table becomes 256KB, grrr.
    uint32_t Gulliver;          // or unsigned char or unsigned short

    unsigned char bm_Hasherezade_HASH[1 << (HashTableSizeSekirei - 3)];
    uint32_t hash32;
    uint32_t hash32B;
    uint32_t hash32C;

    if (cbPattern > cbTarget)
        return (NULL);

    if (cbPattern < 4) {

        pbTarget = pbTarget + cbPattern;
        ulHashPattern =
            ((*(char *)(pbPattern)) << 8) + *(pbPattern + (cbPattern - 1));
        if (cbPattern == 3) {
            for (;;) {
                if ((int)ulHashPattern ==
                    ((*(char *)(pbTarget - 3)) << 8) + *(pbTarget - 1)) {
                    if (*(char *)(pbPattern + 1) == *(char *)(pbTarget - 2))
                        return ((pbTarget - 3));
                }
                if ((char)(ulHashPattern >> 8) != *(pbTarget - 2)) {
                    pbTarget++;
                    if ((char)(ulHashPattern >> 8) != *(pbTarget - 2))
                        pbTarget++;
                }
                pbTarget++;
                if (pbTarget > pbTargetMax)
                    return (NULL);
            }
        } else {
        }
        for (;;) {
            if ((int)ulHashPattern == ((*(char *)(pbTarget - 2)) << 8) + *(pbTarget - 1))
                return ((pbTarget - 2));
            if ((char)(ulHashPattern >> 8) != *(pbTarget - 1))
                pbTarget++;
            pbTarget++;
            if (pbTarget > pbTargetMax)
                return (NULL);
        }

    } else {
        if (cbTarget < HaystackThresholdSekirei) {  // This value is arbitrary (don't know how exactly), it ensures (at least must) better performance than 'Boyer_Moore_Horspool'.

            pbTarget = pbTarget + cbPattern;
            ulHashPattern = *(uint32_t *) (pbPattern);
            SINGLET = ulHashPattern & 0xFF;
            Quadruplet2nd = SINGLET << 8;
            Quadruplet3rd = SINGLET << 16;
            Quadruplet4th = SINGLET << 24;
            for (;;) {
                AdvanceHopperGrass = 0;
                ulHashTarget = *(uint32_t *) (pbTarget - cbPattern);
                if (ulHashPattern == ulHashTarget) {    // Three unnecessary comparisons here, but 'AdvanceHopperGrass' must be calculated - it has a higher priority.
                    count = cbPattern - 1;
                    while (count &&
                           *(char *)(pbPattern + (cbPattern - count)) ==
                           *(char *)(pbTarget - count)) {
                        if (cbPattern - 1 == AdvanceHopperGrass + count &&
                            SINGLET != *(char *)(pbTarget - count))
                            AdvanceHopperGrass++;
                        count--;
                    }
                    if (count == 0)
                        return ((pbTarget - cbPattern));
                } else {        // The goal here: to avoid memory accesses by stressing the registers.
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

        } else {                //if (cbTarget<HaystackThresholdSekirei)

            if (cbPattern <= NeedleThresholdBIGSekirei) {

                countSTATIC = cbPattern - 2 - 2;
                ulHashPattern = *(uint32_t *) (pbPattern);  // First four bytes
                //ulHashTarget = *(unsigned short *)(pbPattern+cbPattern-1-1); // Last two bytes
                i = 0;
                //for (a=0; a < 256*256; a++) {bm_Horspool_Order2[a]= cbPattern-1;} // cbPattern-(Order-1) for Horspool; 'memset' if not optimized
                //for (j=0; j < cbPattern-1; j++) bm_Horspool_Order2[*(unsigned short *)(pbPattern+j)]=j; // Rightmost appearance/position is needed
                for (a = 0; a < 256 * 256; a++) {
                    bm_Horspool_Order2[a] = 0;
                }
                for (j = 0; j < cbPattern - 1; j++)
                    bm_Horspool_Order2[*(unsigned short *)(pbPattern + j)] =
                        1;
                while (i <= cbTarget - cbPattern) {
                    Gulliver = 1;
                    if (bm_Horspool_Order2
                        [*(unsigned short *)&pbTarget[i + cbPattern - 1 - 1]]
                        != 0) {
                        //if ( Gulliver == 1 ) { // Means the Building-Block order 2 is found somewhere i.e. NO MAXIMUM SKIP
                        if (*(uint32_t *) & pbTarget[i] == ulHashPattern) {
                            count = countSTATIC;    // Last two chars already matched, to be fixed with -2
                            while (count != 0 &&
                                   *(char *)(pbPattern +
                                             (countSTATIC - count) + 4) ==
                                   *(char *)(&pbTarget[i] +
                                             (countSTATIC - count) + 4))
                                count--;
                            if (count == 0)
                                return (pbTarget + i);
                        }
                        //}
                    } else
                        Gulliver = cbPattern - (2 - 1);
                    i = i + Gulliver;
                    //GlobalI++; // Comment it, it is only for stats.
                }
                return (NULL);

            } else {            // if ( cbPattern<=NeedleThresholdBIGSekirei )
// MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() [
                countSTATIC = cbPattern - 2 - 2;

                ulHashPattern = *(uint32_t *) (pbPattern);  // First four bytes
                //ulHashTarget = *(unsigned short *)(pbPattern+cbPattern-1-1); // Last two bytes

                i = 0;

                for (a = 0; a < 1 << (HashTableSizeSekirei - 3); a++) {
                    bm_Hasherezade_HASH[a] = 0;
                }               // to-do: 'memset' if not optimized
                // cbPattern - Order + 1 i.e. number of BBs for 11 'fastest fox' 11-8+1=4: 'fastest ', 'astest f', 'stest fo', 'test fox'
                for (j = 0; j < cbPattern - 12 + 1; j++) {
                    hash32 =
                        (2166136261 ^ *(uint32_t *) (pbPattern + j + 0)) *
                        709607;
                    hash32B =
                        (2166136261 ^ *(uint32_t *) (pbPattern + j + 4)) *
                        709607;
                    hash32C =
                        (2166136261 ^ *(uint32_t *) (pbPattern + j + 8)) *
                        709607;
                    hash32 = (hash32 ^ _rotl_KAZE(hash32C, 5)) * 709607;
                    hash32 = (hash32 ^ _rotl_KAZE(hash32B, 5)) * 709607;
                    hash32 =
                        (hash32 ^ (hash32 >> 16)) &
                        ((1 << (HashTableSizeSekirei)) - 1);
                    bm_Hasherezade_HASH[hash32 >> 3] =
                        bm_Hasherezade_HASH[hash32 >> 3] | (1 <<
                                                            (hash32 & 0x7));
                }

                while (i <= cbTarget - cbPattern) {
                    Gulliver = 1;   // Assume minimal jump as initial value.
                    // The goal: to jump when the rightmost 8bytes (Order 8 Horspool) of window do not look like any of Needle prefixes i.e. are not to be found. This maximum jump equals cbPattern-(Order-1) or 11-(8-1)=4 for 'fastest fox' - a small one but for Needle 31 bytes the jump equals 31-(8-1)=24
                    //GlobalHashSectionExecution++; // Comment it, it is only for stats.
                    hash32 =
                        (2166136261 ^
                         *(uint32_t *) (pbTarget + i + cbPattern - 12 +
                                        0)) * 709607;
                    hash32B =
                        (2166136261 ^
                         *(uint32_t *) (pbTarget + i + cbPattern - 12 +
                                        4)) * 709607;
                    hash32C =
                        (2166136261 ^
                         *(uint32_t *) (pbTarget + i + cbPattern - 12 +
                                        8)) * 709607;
                    hash32 = (hash32 ^ _rotl_KAZE(hash32C, 5)) * 709607;
                    hash32 = (hash32 ^ _rotl_KAZE(hash32B, 5)) * 709607;
                    hash32 =
                        (hash32 ^ (hash32 >> 16)) &
                        ((1 << (HashTableSizeSekirei)) - 1);
                    if ((bm_Hasherezade_HASH[hash32 >> 3] &
                         (1 << (hash32 & 0x7))) == 0)
                        Gulliver = cbPattern - (12 - 1);

                    if (Gulliver == 1) {    // Means the Building-Block order 8/12 is found somewhere i.e. NO MAXIMUM SKIP
                        if (*(uint32_t *) & pbTarget[i] == ulHashPattern) {
                            count = countSTATIC;
                            while (count != 0 &&
                                   *(char *)(pbPattern +
                                             (countSTATIC - count) + 4) ==
                                   *(char *)(&pbTarget[i] +
                                             (countSTATIC - count) + 4))
                                count--;
                            if (count == 0)
                                return (pbTarget + i);
                        }
                    }
                    i = i + Gulliver;
                    //GlobalI++; // Comment it, it is only for stats.
                }               // while (i <= cbTarget-cbPattern)
                return (NULL);
// MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() MEMMEM() ]
            }                   // if ( cbPattern<=NeedleThresholdBIGSekirei )
        }                       //if (cbTarget<HaystackThresholdSekirei)
    }                           //if ( cbPattern<4 )
}

// The function's section with simplified BMH order 2 is 3781-3720+2 = 99 bytes long.

/*
; Listing generated by Microsoft (R) Optimizing Compiler Version 16.00.30319.01 
$LL23@Railgun_Se:
  03720 8b 44 24 24      mov     eax, DWORD PTR tv836[esp+73788]
  03724 0f b7 04 38      movzx   eax, WORD PTR [eax+edi]
  03728 80 bc 04 38 20
        00 00 00         cmp     BYTE PTR _bm_Horspool_Order2$[esp+eax+73788], 0
  03730 b9 01 00 00 00   mov     ecx, 1
  03735 74 3e            je      SHORT $LN21@Railgun_Se
  03737 8b 45 00         mov     eax, DWORD PTR [ebp]
  0373a 39 04 1f         cmp     DWORD PTR [edi+ebx], eax
  0373d 75 38            jne     SHORT $LN110@Railgun_Se
  0373f 8b 44 24 20      mov     eax, DWORD PTR _countSTATIC$[esp+73788]
  03743 85 c0            test    eax, eax
  03745 74 18            je      SHORT $LN83@Railgun_Se
  03747 8d 75 04         lea     esi, DWORD PTR [ebp+4]
  0374a 8d 7c 1f 04      lea     edi, DWORD PTR [edi+ebx+4]
  0374e 8b ff            npad    2
$LL19@Railgun_Se:
  03750 8a 16            mov     dl, BYTE PTR [esi]
  03752 3a 17            cmp     dl, BYTE PTR [edi]
  03754 75 11            jne     SHORT $LN18@Railgun_Se
  03756 47               inc     edi
  03757 46               inc     esi
  03758 48               dec     eax
  03759 75 f5            jne     SHORT $LL19@Railgun_Se
$LN108@Railgun_Se:
  0375b 8b 7c 24 10      mov     edi, DWORD PTR _i$[esp+73788]
$LN83@Railgun_Se:
  0375f 8d 04 1f         lea     eax, DWORD PTR [edi+ebx]
  03762 e9 fa fd ff ff   jmp     $LN111@Railgun_Se
$LN18@Railgun_Se:
  03767 85 c0            test    eax, eax
  03769 74 f0            je      SHORT $LN108@Railgun_Se
  0376b 8b 7c 24 10      mov     edi, DWORD PTR _i$[esp+73788]
  0376f 8b 54 24 1c      mov     edx, DWORD PTR tv609[esp+73788]
  03773 eb 02            jmp     SHORT $LN110@Railgun_Se
$LN21@Railgun_Se:
  03775 8b ca            mov     ecx, edx
$LN110@Railgun_Se:
  03777 03 f9            add     edi, ecx
  03779 89 7c 24 10      mov     DWORD PTR _i$[esp+73788], edi
  0377d 3b 7c 24 18      cmp     edi, DWORD PTR tv639[esp+73788]
  03781 76 9d            jbe     SHORT $LL23@Railgun_Se
  03783 e9 d7 fd ff ff   jmp     $LN7@Railgun_Se
$LN30@Railgun_Se:
*/

/*
; Listing generated by Microsoft (R) Optimizing Compiler Version 16.00.30319.01 
; 7829 :                        while (i <= cbTarget-cbPattern) {
  03701 8b 84 24 48 20
        01 00            mov     eax, DWORD PTR _cbTarget$[esp+73784]
  03708 2b c6            sub     eax, esi
  0370a 89 44 24 18      mov     DWORD PTR tv639[esp+73788], eax
  0370e 8d 44 33 fe      lea     eax, DWORD PTR [ebx+esi-2]
  03712 89 44 24 24      mov     DWORD PTR tv836[esp+73788], eax
  03716 eb 08 8d a4 24
        00 00 00 00 90   npad    10
$LL23@Railgun_Se:
; 7830 :                                Gulliver = 1;
; 7831 :                                if ( bm_Horspool_Order2[*(unsigned short *)&pbTarget[i+cbPattern-1-1]] != 0 ) {
  03720 8b 44 24 24      mov     eax, DWORD PTR tv836[esp+73788]
  03724 0f b7 04 38      movzx   eax, WORD PTR [eax+edi]
  03728 80 bc 04 38 20
        00 00 00         cmp     BYTE PTR _bm_Horspool_Order2$[esp+eax+73788], 0
  03730 b9 01 00 00 00   mov     ecx, 1
  03735 74 3e            je      SHORT $LN21@Railgun_Se
; 7832 :                                        //if ( Gulliver == 1 ) { // Means the Building-Block order 2 is found somewhere i.e. NO MAXIMUM SKIP
; 7833 :                                                if ( *(uint32_t *)&pbTarget[i] == ulHashPattern) {
  03737 8b 45 00         mov     eax, DWORD PTR [ebp]
  0373a 39 04 1f         cmp     DWORD PTR [edi+ebx], eax
  0373d 75 38            jne     SHORT $LN110@Railgun_Se
; 7834 :                                                        count = countSTATIC; // Last two chars already matched, to be fixed with -2
  0373f 8b 44 24 20      mov     eax, DWORD PTR _countSTATIC$[esp+73788]
; 7835 :                                                        while ( count !=0 && *(char *)(pbPattern+(countSTATIC-count)+4) == *(char *)(&pbTarget[i]+(countSTATIC-count)+4) )
  03743 85 c0            test    eax, eax
  03745 74 18            je      SHORT $LN83@Railgun_Se
; 7834 :                                                        count = countSTATIC; // Last two chars already matched, to be fixed with -2
  03747 8d 75 04         lea     esi, DWORD PTR [ebp+4]
  0374a 8d 7c 1f 04      lea     edi, DWORD PTR [edi+ebx+4]
  0374e 8b ff            npad    2
$LL19@Railgun_Se:
; 7835 :                                                        while ( count !=0 && *(char *)(pbPattern+(countSTATIC-count)+4) == *(char *)(&pbTarget[i]+(countSTATIC-count)+4) )
  03750 8a 16            mov     dl, BYTE PTR [esi]
  03752 3a 17            cmp     dl, BYTE PTR [edi]
  03754 75 11            jne     SHORT $LN18@Railgun_Se
; 7836 :                                                                count--;
  03756 47               inc     edi
  03757 46               inc     esi
  03758 48               dec     eax
  03759 75 f5            jne     SHORT $LL19@Railgun_Se
$LN108@Railgun_Se:
; 7829 :                        while (i <= cbTarget-cbPattern) {
  0375b 8b 7c 24 10      mov     edi, DWORD PTR _i$[esp+73788]
$LN83@Railgun_Se:
; 7885 :                                                if ( count == 0) return(pbTarget+i);
  0375f 8d 04 1f         lea     eax, DWORD PTR [edi+ebx]
  03762 e9 fa fd ff ff   jmp     $LN111@Railgun_Se
$LN18@Railgun_Se:
; 7837 :                                                        if ( count == 0) return(pbTarget+i);
  03767 85 c0            test    eax, eax
  03769 74 f0            je      SHORT $LN108@Railgun_Se
; 7838 :                                                }
  0376b 8b 7c 24 10      mov     edi, DWORD PTR _i$[esp+73788]
  0376f 8b 54 24 1c      mov     edx, DWORD PTR tv609[esp+73788]
  03773 eb 02            jmp     SHORT $LN110@Railgun_Se
$LN21@Railgun_Se:
; 7839 :                                        //}
; 7840 :                                } else Gulliver = cbPattern-(2-1);
  03775 8b ca            mov     ecx, edx
$LN110@Railgun_Se:
; 7841 :                                i = i + Gulliver;
  03777 03 f9            add     edi, ecx
  03779 89 7c 24 10      mov     DWORD PTR _i$[esp+73788], edi
  0377d 3b 7c 24 18      cmp     edi, DWORD PTR tv639[esp+73788]
  03781 76 9d            jbe     SHORT $LL23@Railgun_Se
; 7829 :                        while (i <= cbTarget-cbPattern) {
  03783 e9 d7 fd ff ff   jmp     $LN7@Railgun_Se
$LN30@Railgun_Se:
; 7842 :                                //GlobalI++; // Comment it, it is only for stats.
; 7843 :                        }
; 7844 :                        return(NULL);
*/
