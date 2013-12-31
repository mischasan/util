// railgun7w, copyleft 2013-Nov-11, Kaze.
// This code presumes unaligned loads of (uint{16,32}) is okay.

#include <assert.h>
#ifndef STR_X
#include <stdint.h>
#include <stdlib.h> // abort NULL
char *Railgun7w(char *pbTarget, int cbTarget, char *pbPattern, int cbPattern);
#endif

//==================================================================
typedef uint32_t uint;
typedef uint8_t  byte;

#define HaystackThresholdSekireiTchittoGritto 961   // Quadruplet works up to this value, if bigger then BMH2 takes over.
#define NeedleThreshold2vs4TchittoGritto 22 // Should be bigger than 8. BMH2 works up to this value (inclusive), if bigger then BMH4 takes over.
#define NeedleThresholdBIGSekireiTchittoGritto (12+700)   // Should be bigger than 'HasherezadeOrder'. BMH2 works up to this value (inclusive).
#define HashTableSizeSekireiTchittoGritto 17-1  // In fact the real size is -3, because it is BITwise, when 17-3=14 it means 16KB, (17-1)-3=13 it means 8KB.
#define HASHBITS    16 
#define HASHMASK    ((1 << (HASHBITS)) - 1)

static byte *check3(byte *pbTarget, byte *pbPattern, uint cbTarget);
static byte *check4(byte *pbTarget, byte *pbPattern, uint cbTarget, uint cbPattern);
static byte *bloom2(byte *pbTarget, byte *pbPattern, uint cbTarget, uint cbPattern);
static byte *bloom4(byte *pbTarget, byte *pbPattern, uint cbTarget, uint cbPattern);
static byte *bloom12(byte *pbTarget, byte *pbPattern, uint cbTarget, uint cbPattern);

static uint fold4(byte *tp);
static uint hash12(byte *tp);

char *
Railgun7w(char *pbTarget, int cbTarget, char *pbPattern, int cbPattern)
{
    assert(cbPattern > 2);
    return (char*)(
           cbPattern > cbTarget ? NULL
         : cbPattern == 3       ? check3(  (byte*)pbTarget, (byte*)pbPattern, (uint)cbTarget)
         : cbTarget < 961       ? check4(  (byte*)pbTarget, (byte*)pbPattern, (uint)cbTarget, (uint)cbPattern)
         : cbPattern <= 22      ? bloom2(  (byte*)pbTarget, (byte*)pbPattern, (uint)cbTarget, (uint)cbPattern)
         : cbPattern <= 712     ? bloom4(  (byte*)pbTarget, (byte*)pbPattern, (uint)cbTarget, (uint)cbPattern)
                                : bloom12( (byte*)pbTarget, (byte*)pbPattern, (uint)cbTarget, (uint)cbPattern)
        );
}

// In the following, targets are expressed as "abc..xyz", patterns are expressed as "ABC...XYZ"

// For pattern "ABC", test "abc" vs "AC" then "B". Skip forward by 1..3
static byte *
check3(byte *pbTarget, byte *pbPattern, uint cbTarget)
{
    byte *pbTargetMax = pbTarget + cbTarget;
    pbTarget += 3;

    uint ulHashPattern = (pbPattern[0] << 8) + pbPattern[2];
    do {
        if (ulHashPattern == ((uint)pbTarget[-3] << 8) + pbTarget[-1]
                && pbPattern[1] == pbTarget[-2])
            return pbTarget - 3;

        if ((byte)(ulHashPattern >> 8) != pbTarget[-2]) {
            pbTarget++;
            if ((byte)(ulHashPattern >> 8) != pbTarget[-2])
                pbTarget++;
        }
    } while (++pbTarget <= pbTargetMax);

    return NULL;
}

// For pattern "ABC...", test "abcd" vs "ABCD", then vs "AAAA"; skip by 1..4
static byte *
check4(byte *pbTarget, byte *pbPattern, uint cbTarget, uint cbPattern)
{
    byte *pbTargetMax = pbTarget + cbTarget;
    uint ulHashPattern = *(uint*)pbPattern;
    uint skip, quad = pbPattern[0];
    quad |= quad << 8;
    quad |= quad << 16;
    pbTarget = pbTarget + cbPattern;

    do {
        uint ulHashTarget = *(uint*) (pbTarget - cbPattern);
        skip = 1;
        if (ulHashPattern == ulHashTarget) {    // Three unnecessary comparisons here, but 'skip' must be calculated - it has a higher priority.
            int count = cbPattern - 1;
            while (count && pbPattern[cbPattern - count] == pbTarget[-count]) {
                if (cbPattern - 1 == skip + count && (byte)quad != pbTarget[-count])
                    skip++;
                count--;
            } 

            if (count == 0)
                return pbTarget - cbPattern;
        } else { // The goal here: to avoid memory accesses by stressing the registers.
            ulHashTarget ^= quad; //XXX Slightly cheaper way to test each byte vs pbPattern[0]
            if (ulHashTarget & 0x0000FF00) {
                skip++;
                if (ulHashTarget & 0x00FF0000) {
                    skip++;
                    if (ulHashTarget & 0xFF000000)
                        skip++;
                }
            }
        }
    } while ((pbTarget += skip) <= pbTargetMax);

    return NULL;
}

// 64KB Bloom filter for 2-byte substrs of pattern.
//  Skip forward by 1 or by (length(pattern) - 1|3), based on mismatch or last 4 chars in window.

//Note: this is not actually Boyer-Moore-Horspool or even Horspool.
//  To be BMH, filter[] would become skip[]:
//    for (i = cbPattern; i--;) skip[*(uint16_t*)(pbPattern + i)] = i - 1;
// ... and search would advance by:
//      i += skip[ *(uint16_t*) &pbTarget[i + cbPattern - 2..4 ] - 2..4

static byte *
bloom2(byte *pbTarget, byte *pbPattern, uint cbTarget, uint cbPattern)
{
    uint i, skip, ulHashPattern = *(uint*) pbPattern;  // First four bytes
    byte filter[256 * 256] = {}; //XXX Let compiler figure out fastest way to zero this out.
    for (i = 0; i < cbPattern - 1; i++)
        filter[*(uint16_t*)(pbPattern + i)] = 1;

    for (i = 0; i <= cbTarget - cbPattern; i += skip) {
        //MSS overlapping loads of uint16 ... see G's comment on registers in check4
        if (!filter[*(uint16_t*) &pbTarget[i + cbPattern - 2]]) {   // Test YZ
            skip = cbPattern - 1;
        } else if (!filter[*(uint16_t*) &pbTarget[i + cbPattern - 4]] || //XXX Testing WX || XY may save one lookup.
                   !filter[*(uint16_t*) &pbTarget[i + cbPattern - 3]]) { // Test XY
            skip = cbPattern - 3;
        } else {
            if (*(uint*) &pbTarget[i] == ulHashPattern) {
                int count = cbPattern - 3;
                // Handrolled memcmp using uints:
                while (count > 0 && *(uint*) (pbPattern + count - 1) == *(uint*) (&pbTarget[i] + count - 1))
                    count = count - 4;
                if (count <= 0)
                    return pbTarget + i;
            }
            skip = 1;
        }
    }

    return NULL;
}

// 64KB Bloom filter indexed by WX+YZ ("hash")
static byte *
bloom4(byte *pbTarget, byte *pbPattern, uint cbTarget, uint cbPattern)
{
    uint i, skip, ulHashPattern = *(uint*)pbPattern;  // First four bytes
    byte filter[256 * 256] = {};

    for (i = 0; i < cbPattern - 3; i++)
        filter[ fold4(pbPattern + i) ] = 1;

    for (i = 0; i <= cbTarget - cbPattern; i += skip) {
        if (!filter[ fold4(pbTarget + cbPattern - 4) ]) {
            skip = cbPattern - 3;
        } else if (!filter[ fold4(pbTarget + i + cbPattern - 8) ]) {
            skip = cbPattern - 7;
        } else {
            int count = cbPattern - 3;
            while (count > 0 && *(uint*) (pbPattern + count - 1) == *(uint*) (&pbTarget[i] + (count - 1)))
                count = count - 4;  // - order, of course order 4 is much more SWEET&CHEAP - less loops
            if (count <= 0 && *(uint*) &pbTarget[i] == ulHashPattern)
                return pbTarget + i;
            skip = 1;
        }
    }

    return NULL;
}

// Hash every 12-byte substr of pattern to a bit in a 64Kbit Bloom filter.
//  When a 12-byte substr of target hits a bit in the filter, skip ahead by one.
//  Otherwise, skip ahead by pattern length - 11.
static byte *
bloom12(byte *pbTarget, byte *pbPattern, uint cbTarget, uint cbPattern)
{
    uint ulHashPattern = *(uint*)pbPattern;  // First four bytes
    byte filter[1 << (HASHBITS - 3)] = {};
    uint i, skip;

    for (i = 0; i < cbPattern - 12 + 1; i++) {
        uint h = hash12(pbPattern + i);
        filter[h >> 3] |= 1 << (h & 7);
    }

    for (i = 0; i <= cbTarget - cbPattern; i += skip) {
        //XXX This would get a performance benefit by changing hash12 into a rolling hash!
        // especially when skip == 1
        uint h = hash12(pbTarget + i + cbPattern - 12 + 0);
        if (filter[h >> 3] & (1 << (h & 7))) {
            skip = 1;
            int count = cbPattern - 4 + 1;
            while (count > 0 && *(uint*) (pbPattern + count - 1) == *(uint*) (&pbTarget[i] + (count - 1)))
                count = count - 4;  // - order, of course order 4 is much more SWEET&CHEAP - less loops
            if (count <= 0 && *(uint*) &pbTarget[i] == ulHashPattern)
                return pbTarget + i;
        } else {
            skip = cbPattern - 11;
        }
    }

    return NULL;
}

// Fold 4 bytes into 16 bits.
static inline uint
fold4(byte *tp)
{
    uint chunk = *(uint*)tp;
    return (chunk >> 16) + (chunk & 0xFFFF);
}


// Hash 12 bytes into 16 bits.
static uint
hash12(byte *tp)
{
    uint hash32 = (2166136261 ^ *(uint*) (tp + 0)) * 709607;
    uint hash32B = (2166136261 ^ *(uint*) (tp + 4)) * 709607;
    uint hash32C = (2166136261 ^ *(uint*) (tp + 8)) * 709607;

#   define _rotl_KAZE(x, n) (((x) << (n)) | ((x) >> (32-(n))))
    hash32 = (hash32 ^ _rotl_KAZE(hash32C, 5)) * 709607;
    hash32 = (hash32 ^ _rotl_KAZE(hash32B, 5)) * 709607;
    hash32 = (hash32 ^ (hash32 >> 16)) & HASHMASK;

    return hash32;
}
