// railgun7w, copyleft 2013-Nov-11, Kaze.
// This code presumes unaligned loads of (uint{16,32}) is okay.
// This is sanmayce's SeikiReigan/Wolfram code, reorganized so that
//  the decision logic is in one place and common code (inlined)
//  is easy to see and improve.

#include <assert.h>

#ifndef STR_X   // stress.c test harness inlines this file.
#include <stdint.h>
#include <stdlib.h> // abort NULL
char *Railgun7w(char *pbTarget, int cbTarget, char *pbPattern, int cbPattern);
#endif

//==================================================================
typedef uint32_t uint;  //XXX time to add u64 logic
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

static uint foldp(byte *tp);
static uint hash12(byte *tp);
static inline uint intdiff(byte *pbOne, byte *pbTwo, uint cbLen);
static inline uint foldu(uint x) { return (x >> 16) ^ (x & 0xFFFF); }
#ifndef ROLLHASH_MOD
static uint32_t rollhash_arg(uint32_t leng);
static uint32_t rollhash_init(uint8_t const*data, uint32_t leng);
static uint32_t rollhash_step(uint32_t arg, uint32_t hash, uint8_t old, uint8_t new);
#endif

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

// For pattern "ABC", test "a-c" vs "AC" then "B". Skip forward by 1..3
static byte *
check3(byte *pbTarget, byte *pbPattern, uint cbTarget)
{
    byte *pbTargetMax = pbTarget + cbTarget - 3, A = pbPattern[0];
    uint ulHashPattern = ((uint)A << 8) + pbPattern[2];
    do { 
        if (ulHashPattern == ((uint)pbTarget[0] << 8) + pbTarget[2]
                && pbPattern[1] == pbTarget[1])
            return pbTarget;

        if (pbTarget[1] != A) {
            pbTarget++;
            if (pbTarget[1] != A)
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
//XXX make this do a real BHM skipahead by computing filter[...] = cbPattern - i - 2;
//  then:   if (!(skip = filter[ foldp(pbPattern + i) ])) ... rather than skip = 1.
static byte *
bloom2(byte *pbTarget, byte *pbPattern, uint cbTarget, uint cbPattern)
{
    uint i, skip, ulHashPattern = *(uint*) pbPattern;  // First four bytes
    byte filter[256 * 256] = {}; //XXX Let compiler figure out fastest way to zero this out.
    for (i = 0; i < cbPattern - 1; i++)
        filter[*(uint16_t*)(pbPattern + i)] = 1;

    for (i = 0; i <= cbTarget - cbPattern; i += skip) {
        //MSS overlapping loads of uint16 ... see G's comment on registers in check4
        if (!filter[*(uint16_t*) &pbTarget[i + cbPattern - 2]])    // Test YZ
            skip = cbPattern - 1;
        else if (!filter[*(uint16_t*) &pbTarget[i + cbPattern - 4]] || //XXX Testing WX || XY may save one lookup.
                 !filter[*(uint16_t*) &pbTarget[i + cbPattern - 3]]) // Test XY
            skip = cbPattern - 3;
        else if (*(uint*)&pbTarget[i] == ulHashPattern && !intdiff(pbTarget + i, pbPattern, cbPattern))
            return pbTarget + i;
        else skip = 1;
    }

    return NULL;
}

// 64KB Bloom filter indexed by WX+YZ ("hash")
//XXX make this do a real BHM skipahead by computing filter[...] = cbPattern - i - 4;
static byte *
bloom4(byte *pbTarget, byte *pbPattern, uint cbTarget, uint cbPattern)
{
    uint i, skip, ulHashPattern = *(uint*)pbPattern;  // First four bytes
    byte filter[256 * 256] = {};

    for (i = 0; i < cbPattern - 3; i++)
        filter[ foldp(pbPattern + i) ] = 1;

    for (i = 0; i <= cbTarget - cbPattern; i += skip) {
        if (!filter[ foldp(pbTarget + cbPattern - 4) ])
            skip = cbPattern - 3;
        else if (!filter[ foldp(pbTarget + i + cbPattern - 8) ])
            skip = cbPattern - 7;
        else if (*(uint*)&pbTarget[i] == ulHashPattern && !intdiff(pbTarget + i, pbPattern, cbPattern))
            return pbTarget + i;
        else skip = 1;
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

    for (i = 0; i <= cbPattern - 12; i++) {
        uint h = hash12(pbPattern + i);
        filter[h >> 3] |= 1 << (h & 7);
    }

    uint rharg = rollhash_arg(12);
    uint rhash = rollhash_init(pbTarget + cbPattern - 12, 12);

    for (i = 0; i <= cbTarget - cbPattern; i += skip) {
        //uint h = hash12(pbTarget + i + cbPattern - 12);
        uint h = foldu(rhash);
        if (!(filter[h >> 3] & (1 << (h & 7))))
            skip = cbPattern - 11,
            rhash = rollhash_init(pbTarget + cbPattern + i + 1 - 12, 12);
        else if (*(uint*)&pbTarget[i] == ulHashPattern && !intdiff(pbTarget + i, pbPattern, cbPattern))
            return pbTarget + i;
        else skip = 1,
            rhash = rollhash_step(rharg, rhash, pbTarget[cbPattern + i + cbPattern - 12], pbTarget[cbPattern + i + cbPattern]);
    }

    return NULL;
}

// Fold 4 bytes into 16 bits.
static inline uint
foldp(byte *tp)
{
    uint chunk = *(uint*)tp;
    return (chunk >> 16) ^ (chunk & 0xFFFF);
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

static inline uint
intdiff(byte *pbOne, byte *pbTwo, uint cbLen)
{
    int i = cbLen;
    while ((i -= sizeof(uint)) > 0)
         if (*(uint*)&pbOne[i] != *(uint*)&pbTwo[i]) return 1;
    return 0;
}

#ifndef ROLLHASH_MOD
#define ROLLHASH_MOD 8355967

// rollhash_arg returns (256 ^ (leng - 1) mod ROLLHASH_MOD).
//  Most efficient to compute this once then pass it to rollhash_step.
//  Calling   rollhash_step(1, hash, data[i]*arg, data[i+leng])
//  amounts to the same thing. 

static uint32_t
rollhash_arg(uint32_t leng)
{
    uint32_t arg = 1;
    while (--leng) arg = arg * 256 % ROLLHASH_MOD;
    return arg;
}

static uint32_t
rollhash_init(uint8_t const*data, uint32_t leng)
{
    uint32_t hash = 0;
    while (leng--) hash = (hash * 256 + *data++) % ROLLHASH_MOD;
    return hash;
}

static uint32_t
rollhash_step(uint32_t arg, uint32_t hash, uint8_t old, uint8_t new)
{
    return ((hash + 256*ROLLHASH_MOD - old * arg) % ROLLHASH_MOD * 256 + new) % ROLLHASH_MOD;
}
#endif
