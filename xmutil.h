#ifndef XMUTIL_H
#define XMUTIL_H
#include <xmmintrin.h>

// xm_same(x,y)           -  16bit mask with bits set where bytes of two xmm values match.
// xm_diff(x,y)           -  16bit mask with bits set where bytes of two xmm values differ.
// xm_fill(b)             -  repeat (byte) x 16 as an xmm value.
// xm_load(XMM const*)    -  LOad/AligneD xmm value
// xm_loud(void const*)   -  LOad/UnaligneD xmm value -- only used by scancmp.
// xm_ones()              - efficient (__m128i){-1,-1}. Uses two instructions
//                          (pxor x,x; pcmpeqb x,x) the pxo is unnecessary!
// xm_f[fl]s(x)          - find first/last bit set (0..127) in an XMM value.
//                          xm_f[fl]s(0) = -1
// xm_shl(x,n)           - 128-bit variable shift
// xm_shl001(x) ... xm_shl177(x) - optimal inline constant shifts.
// Format x as a string ...
// xm_dbl(x,buf) - pair of doubles
// xm_llx(x,buf) - pair of ints (hex)
// xm_str(x,buf) - LSB-first "xx,xx,xx,..,xx-xx,xx,xx,..,xx"

typedef __m128i XMM;

#define xm_fill(c)    _mm_set1_epi8(c)
#define xm_bshl(x,n)  _mm_slli_si128(x,n) // xm <<= 8*n  -- BYTE shift
#define xm_bshr(x,n)  _mm_srli_si128(x,n) // xm <<= 8*n  -- BYTE shift
#define xm_shl64(x,n) _mm_slli_epi64(x,n) // xm.hi <<= n, xm.lo <<= n
#define xm_shr64(x,n) _mm_srli_epi64(x,n) // xm.hi >>= n, xm.lo >>= n

#define xm_and(a,b)    _mm_and_si128(a,b)
#define xm_andnot(a,b) _mm_andnot_si128(a,b)    // AKA "bic"
#define xm_not(a)      xm_xor(a, xm_ones)
#define xm_or(a,b)     _mm_or_si128(a,b)
#define xm_xor(a,b)    _mm_xor_si128(a,b)

#define xm_ones        xmones()
#define xm_zero        _mm_setzero_si128()

static inline unsigned xm_same(XMM a, XMM b)
{ return _mm_movemask_epi8(_mm_cmpeq_epi8(a, b)); }

static inline unsigned xm_diff(XMM a, XMM b)
{ return xm_same(a, b) ^ 0xFFFF; }

static inline XMM xm_load(void const*p)
{ return _mm_load_si128((XMM const*)p); }

static inline XMM xm_loud(void const*p)
{ return (XMM)_mm_loadu_pd((double const*)p); }

static inline XMM xmones(void)
{ XMM x = {}; return _mm_cmpeq_epi8(x,x); }

int xm_ffs(XMM x);
int xm_fls(XMM x);

XMM xm_shl(XMM x, unsigned nbits);
XMM xm_shr(XMM x, unsigned nbits);

char*xm_dbl(__m128d x, char buf[48]);
char*xm_llx(XMM x, char buf[48]);
char*xm_str(XMM x, char buf[48]);

// Generate static inline functions xm_shl_001 .. xm_shl_177
//  which can be called directly, and are used by xm_shl.
// (Ditto for xm_shr).
// ops per shift: 15(1) 56(2) 7(5) 49(6)

#define DO_7x7(A)    DO_7(A,1)    DO_7(A,2)    DO_7(A,3)    DO_7(A,4)    DO_7(A,5)    DO_7(A,6)    DO_7(A,7)
#define DO_7(A,B)    DO_LR(A,B,1) DO_LR(A,B,2) DO_LR(A,B,3) DO_LR(A,B,4) DO_LR(A,B,5) DO_LR(A,B,6) DO_LR(A,B,7)
#define DO_LR(A,B,C) DO(A,B,C,shl,shr) DO(A,B,C,shr,shl)

#undef  DO
#define DO(A,B,C,FWD,BAK) \
    static inline XMM xm_##FWD##_00##C(XMM x) \
    { return xm_or(xm_##FWD##64(x, 0##C), \
                   xm_##BAK##64(xm_b##FWD(x, 8), 64-0##C)); }
DO_7(0,0)       // 1..7

#undef  DO
#define DO(A,B,C,FWD,BAK) \
    static inline XMM xm_##FWD##_##A##C##0(XMM x) \
    { return xm_b##FWD(x, 0##A##C); } 
DO_7(0,0)       // 8,16 .. 56
DO_LR(1,0,0)    // 64
DO_7(1,0)       // 72,80 .. 120

#undef  DO
#define DO(A,B,C,FWD,BAK) \
    static inline XMM xm_##FWD##_##A##B##C(XMM x) \
    { return xm_or(xm_##FWD##64(xm_b##FWD(x, 0##A##B), 0##C), \
                   xm_##BAK##64(xm_b##FWD(x, 0##A##B+8), 64-0##A##B)); }
DO_7x7(0)       // 9..63 except 16,24, ...

#undef  DO
#define DO(A,B,C,FWD,BAK) \
    static inline XMM xm_##FWD##_##A##B##C(XMM x) \
    { return xm_##FWD##64(xm_b##FWD(x, 0##A##B), 0##C); }
DO_7(1,0)       // 65..71
DO_7x7(1)       // 73..127 except 80,88,...

#endif//XMUTIL_H
