#include "msutil.h"
#include "xmutil.h"

// This code assumes that memory is allocated (valid)
//  in segments that start and end on 16byte boundaries;
//  i.e. *cp is valid => *(cp & -16) and *(cp | 15) are valid.

char *strchrs(char *tgt, char const *pat);

// intcmp is (!) faster than memcmp or any complex REP CMPSx.
//  Memory alignment is less and less an issue

static int intcmp(char const *dst, char const *src, int len);
//--------------|-------|-------------------------------------
char const*
scanbrk(char const* tgt, char const* chrs)
{
    int         nchrs = strlen(chrs);
    if (nchrs != 2) return strpbrk(tgt, chrs);
    XMM         zero = {}, x;
    XMM         xp0 = xm_fill(chrs[0]), xp1 = xm_fill(chrs[1]);
    unsigned    z, d, f = 15 & (uintptr_t)tgt;

    // Initial unaligned chunk of tgt:
    if (f) {
        z = xm_same(x = xm_load(tgt - f), zero) >> f;
        d = ((xm_same(x, xp0) | xm_same(x, xp1)) >> f) & under(z);
        if (d) return tgt + ffs(d) - 1;
        if (z) return NULL;
        tgt += 16 - f;
    }

    // 16-byte aligned chunks of tgt:
    for (; !(z = xm_same(x = xm_load(tgt), zero)); tgt += 16)
        if ((d = xm_same(x, xp0) | xm_same(x, xp1)))
            return tgt + ffs(d) - 1;

    // Final 0..15 bytes of tgt:
    if ((d = (xm_same(x, xp0) | xm_same(x, xp1)) & under(z)))
        return tgt + ffs(d) - 1;

    return NULL;
}

int
scancmp(char const* s, char const* t)
{
    XMM         xs, zero = {};
    unsigned    m, z, f = 15 & (intptr_t) s;

    if (f) {
        z = xm_same(xs = xm_load(s - f), zero) >> f;
        m = under(z) & (xm_diff(xs, xm_loud(t - f)) >> f) & under(z);
        if (m) return m = ffs(m) - 1, s[m] - t[m];
        if (z) return 0;
        s += 16 - f, t += 16 - f;
    }

    for (;!(z = xm_same(xs = xm_load(s), zero)); s += 16, t +=16)
        if ((m = xm_diff(xs, xm_loud(t))))
            return m = ffs(m) - 1, s[m] - t[m];

    m = xm_diff(xm_load(s), xm_loud(t)) & under(z);
    return m ? m = ffs(m) - 1, s[m] - t[m] : 0;
}

// scanstr puts the memcmp(pat+2,*) INSIDE the loop.
/// You can also use a loop that calls scanstr2 then memcmp(pat+2,*) outside the call.

//XXX for patterns >32 bytes, you may be able to step forward by a
//  multiple of 16 bytes, instead of just "tgt += 16".

char const*
scanstr(char const* tgt, char const* pat)
{
    if (!pat[0]) return tgt;
    if (!pat[1]) return strchr(tgt, *pat);

    XMM         zero = {}, xt;
    XMM         xp0  = xm_fill(pat[0]);
    XMM         xp1  = xm_fill(pat[1]);
    unsigned    m    = 15 & (intptr_t)tgt, patlen = strlen(pat);
    unsigned    mz   = (-1 << m) & xm_same(zero, xt = xm_load(tgt -= m));
    unsigned    m0   = (-1 << m) & xm_same(xp0, xt);
    char const  *p;

    while (!mz) {
        if (m0) {
            unsigned m1 = xm_same(xp1, xt);
            m0 &= (m1 >> 1) | (tgt[16] == pat[1] ? 0x8000 : 0);
            for (m = m0; m; m &= m - 1) {
                int pos = ffs(m) - 1;
                if (!intcmp(pat+2, tgt+pos+2, patlen-2)) return tgt+pos;
            }
        }
        mz = xm_same(zero, xt = xm_load(tgt += 16));
        m0 = xm_same(xp0, xt);
    }

    if ((m0 &= under(mz))) {
        m0 &= (xm_same(xp1, xt) >> 1);
        for (m = m0; m; m &= m - 1) {
             p = tgt + ffs(m) - 1;
             if (!intcmp(pat+2, p+2, patlen-2))
                return p;
        }
    }

    return NULL;
}

char const *
scanstrx(char const *tgt, char const *pat)
{
    int patlen = strlen(pat);
    if (patlen == 0) return tgt;
    if (patlen == 1) return strchr(tgt, *pat);
    while ((tgt = scanstr2(tgt, pat)) && memcmp(tgt+2, pat+2, patlen-2)) ++tgt;
    return tgt;
}

#define load16(p)   (*(uint16_t const*)(p))
#define load32(p)   (*(uint32_t const*)(p))
char const*
scanstr2(char const *tgt, char const pat[2])
{
    XMM const zero = _mm_setzero_si128();
    XMM const p0   = xm_fill(pat[0]);
    XMM const p1   = xm_fill(pat[1]);
    uint16_t  pair = load16(pat);
    unsigned  f    = 15 & (uintptr_t)tgt;
    tgt -= f;
    while (1) {
        XMM x = xm_load(tgt);
        unsigned u = xm_same(x, zero) >> f;
        unsigned v = ~u & (u - 1) & ((xm_same(x, p0) & (xm_same(x, p1) >> 1)) >> f);
        if (v) return tgt + f + ffs(v) - 1;
        if (u) return NULL;
        tgt += 16;
        if (load16(tgt - 1) == pair) return tgt - 1;
        f = 0;
    }
}

// strchrs test strchr being a fast (SSE2) implementation.
char *strchrs(char *tgt, char const *pat)
{
    int     p0 = *pat++, rest = strlen(pat), hop = 1;
#if 0
    If the first char of pat occurs next at offset hop,
    then instead of memcmp, count the chars of pat+1 that match tgt,
    and (on failure) advance to min(hop, count+1).
    
    This just SCREAMS for REP CMPSB, which is faster than SCASB.
    Instead of memcmp, you need memprefix, an optimization of str
#endif

    hop = strcspn(pat, (char*)&p0);
    if (hop > 3) {
        for (; (tgt = strchr(tgt, p0)) && rest < memcmp(tgt+1, pat, rest); tgt += hop < rest ? hop : rest+1);
    } else {
        for (; (tgt = strchr(tgt, p0)) && memcmp(tgt+1, pat, rest); tgt++);
    }
//    for (; (tgt = strchr(tgt, p0)) && memcmp(tgt+1, pat, rest); tgt += hop);
    return tgt;
}
//--------------|---------------------------------------------

static int intcmp(char const *dst, char const *src, int len)
{
    while (len >= (int)sizeof(int)) {
        if (*(int const*)src != *(int const*)dst) return 1;
        len -= sizeof(int), src += sizeof(int), dst +=sizeof(int);
    }

#   if __SIZEOF_INT__ == 8
    if (len >= 4) {
        if (*(int32_t const*)src != *(int32_t const*)dst) return 1;
        len -= 4, src += 4, dst += 4;
    }
#   endif

    if (len >= (int)sizeof(short)) {
        if (*(short const*)src != *(short const*)dst) return 1;
        len -= sizeof(short), src += sizeof(short), dst += sizeof(short);
    }

    return len && *src != *dst;
}
//EOF
