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

// msutil: generic utility functions not dependent on any engine components.

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>    // open O_RDONLY
#include <stdarg.h>
#include <unistd.h>    // read
#include <sys/mman.h>
#include <sys/stat.h>
#include "msutil.h"
#include "xmutil.h"

#ifndef MAP_NOCORE  // BSD
#   define  MAP_NOCORE  0
#endif

typedef unsigned uns;
MAKE_MINMAX(int); // int_min, int_max
MAKE_MINMAX(uns); // uns_min, uns_max

void
vec_allow(VEC* v, int limit)
{
    if (limit > v->limit)
        v->ptr = realloc(v->ptr, v->width * (v->limit = limit));
}

void
vec_free(VEC* v)
{
    if (!v) return;
    vec_resize(v, 0);
    free(v->ptr);
    free(v);
}

VEC*
vec_new(int width, DTOR *dtor, void *context)
{
    VEC     *vp = malloc(sizeof(VEC));
    *vp = (VEC){ 1, 0, malloc(width), width, dtor, context };
    return  vp;
}

void*
vec_push(VEC* v, void *elem)
{
    if (v->count == v->limit)
        v->ptr = realloc(v->ptr, v->width * (v->limit <<= 1));
    return memcpy(_vel(v, v->count++), elem, v->width);
}

void
vec_resize(VEC* v, int limit)
{
    if (v->dtor)
        while (v->count > limit)
            v->dtor(vec_pop(v), v->context);

    v->ptr = realloc(v->ptr, v->width * (v->limit = limit));
}

STR
STRcat(STR str, char const *s)
{
    int     size = vec_count(str);
    vec_allow(str, size + strlen(s) + 1);
    strcpy(STRptr(str) + size, s);
    return  str;
}

STR
STRcpy(STR str, char const *s)
{
    vec_resize(str, strlen(s) + 1);
    strcpy(STRptr(str), s);
    return  str;
}

/* raw mail address strings are of the form:
 *    "something <someone@somewhere> (comment)"
 * OR   " someone@somewhere (comment)"
 * ... "comment" being optional.
 */

MEMREF
addr_part(MEMREF r)
{
    const char  *dp, *cp = memchr(r.ptr, '(', r.len);
    size_t        len = cp ? (size_t)(cp - r.ptr) : r.len;

    if ((cp = memchr(r.ptr, '<', len)) && (dp = memchr(cp, '>', r.ptr+len-cp))) {
        len = dp - cp - 1;
        ++cp;
    } else {
        cp = r.ptr;
    }
        // Trim leading space
    while (len > 0 && isspace(cp[len-1])) --len;
    while (len > 0 && isspace(*cp)) ++cp, --len;
        // (cp) should match /.[@%]./
    return    memref(cp, len);
}

int
belch(char const *filename, MEMREF data)
{
    FILE    *fp = fopen(filename, "w");
    if (!fp) return 0;
    int ret = fwrite(data.ptr, data.len, 1, fp);
    fclose(fp);
    return  ret;
}

// bitwid: 1+floor(log2(u))
int
bitwid(unsigned u)
{
#ifdef __BSD_VISIBLE
    int	ret = fls(u);
    if (u > 1U << ret)
	++ret;
#else
    if (!u) return 0;
    int        ret;
    if (u & 0xFFFF0000) u >>= 16, ret = 17; else ret = 1;
    if (u & 0x0000FF00) u >>=  8, ret += 8;
    if (u & 0x000000F0) u >>=  4, ret += 4;
    if (u & 0x0000000C) u >>=  2, ret += 2;
    if (u & 0x00000002) ret++;
#endif
    return ret;
}

static inline int cmp(int diff, uint8_t const*src, uint8_t const*dst)
{ return diff = ffs(diff) - 1, (int)src[diff] - dst[diff]; }

// typeof(cmpxm) == typeof(memcmp)
int
cmpxm(void const*_src, void const*_dst, int len)
{
    uint8_t const *src = (uint8_t const*)_src;
    uint8_t const *dst = (uint8_t const*)_dst;
    int ret, srcoff = 15 & (intptr_t)src;

    if (srcoff && len > 31) {
        if ((ret = xm_diff(xm_loud(src), xm_loud(dst))))
            return cmp(ret, src, dst);

        src += 16 - srcoff;
        dst += 16 - srcoff;
        len -= 16 - srcoff;
    }

    // A special case for srcoff==dstoff => xm_load(dst)
    // seems to make no difference.
    // Prefetch windows from 128..512 all do about the same.
#   define PRE (512-64)
    for (; len >= 16; src += 16, dst += 16, len -=16) {
        _mm_prefetch(src+PRE, _MM_HINT_NTA);
        _mm_prefetch(dst+PRE, _MM_HINT_NTA);
        if ((ret = xm_diff(xm_load(src), xm_loud(dst))))
            return cmp(ret, src, dst);
    }

    // xm_loud(dst) can actually go beyond valid mem :-(
    ret = xm_diff(xm_loud(src), xm_loud(dst)) & ~(-1 << len);
    return ret ? cmp(ret,  src, dst) : 0;
}

int memsame(void const*_src, void const*_dst, int len)
{
    uint8_t const *src = (uint8_t const*)_src;
    uint8_t const *dst = (uint8_t const*)_dst;
    int ret, srcoff = 15 & (intptr_t)src;

    if (srcoff && len > 31) {
        if ((ret = xm_diff(xm_loud(src), xm_loud(dst))))
            return src - (uint8_t const*)_src + ffs(ret) - 1;

        src += 16 - srcoff;
        dst += 16 - srcoff;
        len -= 16 - srcoff;
    }

    // A special case for srcoff==dstoff => xm_load(dst)
    //  seems to make no difference.
    // Prefetch windows from 128..512 all do about the same.
#   define PRE (512-64)
    for (; len >= 16; src += 16, dst += 16, len -=16) {
        _mm_prefetch(src+PRE, _MM_HINT_NTA);
        _mm_prefetch(dst+PRE, _MM_HINT_NTA);
        if ((ret = xm_diff(xm_load(src), xm_loud(dst))))
            return src - (uint8_t const*)_src + ffs(ret) - 1;
    }

    if ((ret = xm_diff(xm_loud(src), xm_loud(dst)) & ~(-1 << len)))
        return src - (uint8_t const*)_src + ffs(ret) - 1;
    return 0;
}

int
bit_count(char const *vec, int len)
{
    XMM     sum, x;
    static __v2di
        m0 = { 0x5555555555555555ULL, 0x5555555555555555ULL },
        m1 = { 0x3333333333333333ULL, 0x3333333333333333ULL },
        m2 = { 0x0F0F0F0F0F0F0F0FULL, 0x0F0F0F0F0F0F0F0FULL },
        m3 = { 0x00FF00FF00FF00FFULL, 0x00FF00FF00FF00FFULL },
        m4 = { 0x0000FFFF0000FFFFULL, 0x0000FFFF0000FFFFULL };
#   define  ADDSTEP(n) x = _mm_add_epi32(_mm_and_si128(m##n, x), _mm_and_si128(m##n, _mm_srli_epi64(x,1<<n)))
#   define  ADDBITS ADDSTEP(0); ADDSTEP(1); ADDSTEP(2); ADDSTEP(3); ADDSTEP(4);

    unsigned    f = 15 & (intptr_t)vec;
    if (f) {
        x = _mm_setzero_si128();
        memcpy(&x, vec, f = 16 - f);
        len -= f, vec += f;
        ADDBITS;
        sum = x;
    } else {
        sum = _mm_setzero_si128();
    }

    XMM const *mp = (XMM const*)vec;
    for (; len >= 16; len -= 16) {
        x = *mp++;
        ADDBITS;
        sum = _mm_add_epi32(sum, x);
    }

    if (len > 0) {
        x = _mm_setzero_si128();
        memcpy(&x, mp, len);
        ADDBITS;
        sum = _mm_add_epi32(sum, x);
    }

    return __builtin_ia32_vec_ext_v4si((__v4si)sum, 0)
         + __builtin_ia32_vec_ext_v4si((__v4si)sum, 1)
         + __builtin_ia32_vec_ext_v4si((__v4si)sum, 2)
         + __builtin_ia32_vec_ext_v4si((__v4si)sum, 3);
}

MEMREF
fileref(char const *filename)
{
    MEMREF  mr = NILREF;
    int     fd = open(filename, O_RDONLY, 0);
    if (fd >= 0) {
        mr.len = lseek(fd, 0L, SEEK_END);
        mr.ptr = mmap(NULL, mr.len, PROT_READ, MAP_SHARED|MAP_NOCORE, fd, 0);
        close(fd);
    }

    return fd < 0 || mr.len <= 0 || mr.ptr == MAP_FAILED ? NILREF : mr;
}


#define Bytes(x) (((x) + 7) >> 3)    /* nbits -> nbytes */
#define BYTEDIFF(p,x) (0xFFFF ^ (_mm_movemask_epi8( _mm_cmpeq_epi8(*(__m128i const*)(p),(x)))))

int
findbit_0(uint8_t const*vec, int len)
{
    int off = 15 & (intptr_t)vec;
    uint8_t const *bp = vec - off;  // Back up to a 16b boundary.
    __m128i ones = {-1, -1};
    uint16_t m = BYTEDIFF(bp, ones);
    len += off;
        // Mask off trailing bits (tiny vec)
    if (len < 16) m &= ~(-1 << len);  
        // First time through, mask off leading (off) bits
    for (m &= -1 << off; !m && len >= 16; m = BYTEDIFF(bp, ones))
        bp += 16, len -= 16, _mm_prefetch(bp+512, _MM_HINT_NTA);
        // Mask off trailing bits (tail of vec)
    if (len < 16) m &= ~(-1 << len);  
    return m ? (off = bsrl(m), 8*(off+bp-vec) + bsrl(bp[off] ^ 0xFF)) : -1;
}

int
findbit_1(uint8_t const*vec, int len)
{
    int off = 15 & (intptr_t)vec;
    uint8_t const *bp = vec - off;  // Back up to a 16b boundary.
    __m128i zero = _mm_setzero_si128();
    uint16_t m = BYTEDIFF(bp, zero);
    len += off;
        // Mask off trailing bits (tiny vec)
    if (len < 16) m &= ~(-1 << len);  
        // First time through, mask off leading (off) bits
    for (m &= -1 << off; !m && len >= 16; m = BYTEDIFF(bp, zero))
        bp += 16, len -= 16, _mm_prefetch(bp+512, _MM_HINT_NTA);
        // Mask off trailing bits (tail of vec)
    if (len < 16) m &= ~(-1 << len);  
    if (!m) return -1;
    off = bsrl(m);
    return 8*(off + bp - vec) + bsrl(bp[off]);
}

void
bufcat(MEMBUF* bp, MEMREF r)
{
    if (nilref(r)) return;
    bp->ptr = realloc(bp->ptr, bp->len + r.len + 1);
    bp->ptr[bp->len + r.len] = 0;
    memcpy(bp->ptr + bp->len, r.ptr, r.len);
    bp->len += r.len;
}

MEMBUF
chomp(MEMBUF buf)
{
    if (buf.ptr)
	while (buf.len > 0 && isspace(buf.ptr[buf.len - 1]))
	    buf.ptr[--buf.len] = 0;
    return  buf;
}

void
die(char const *fmt, ...)
{
    va_list	vargs;
    va_start(vargs, fmt);
    if (*fmt == ':') fputs(getprogname(), stderr);
    vfprintf(stderr, fmt, vargs);
    va_end(vargs);
    if (fmt[strlen(fmt)-1] == ':')
        fprintf(stderr, " %s %s", errname[errno], strerror(errno));
    putc('\n', stderr);
    _exit(1);
}

FILE *
fopenf(char const *mode, char const *fmt, ...)
{
    char	*path;
    va_list	vargs;
    va_start(vargs, fmt);
    vasprintf(&path, fmt, vargs);
    va_end(vargs);
    FILE    *fp = fopen(path, mode);
    free(path);
    return  fp;
}


#if defined(linux)
char const *
getprogname(void)
{
    static char *progname;

    if (!progname) {
        char    buf[999];
        int     len;
        sprintf(buf, "/proc/%d/exe", getpid());
        len = readlink(buf, buf, sizeof(buf));
        if (len < 0 || len == sizeof(buf))
            return NULL;
        buf[len] = 0;
        char    *cp = strrchr(buf, '/');
        progname = strdup(cp ? cp + 1 : buf);
    }

    return  progname;
}
#elif defined(AIX) || defined(hpux)
char const*
getprogname(void)
{
    return "<program>";
}
#endif

void
hexdump(FILE *fp, void const*_buf, int len)
{
    uint8_t const *buf = (uint8_t const*)_buf;
    if (!fp) return;
    int line, i, wid = 32;

    for (line = 0; line < len; line += wid) {
        putc('\t', fp);
        for (i = 0; i < wid; ++i) {
            if (i == wid/2) putc(' ', fp);
            putc(line+i >= len ? ' ' : isprint(buf[line+i]) ? buf[line+i] : '.', fp);
        }
        putc('|', fp);
        for (i = 0; i < wid && line+i < len; ++i) fprintf(fp, " %s%02X", i == wid/2 ? " " : "", buf[line+i]);
        putc('\n', fp);
    }
}

// memfind: the "mem" equivalent of strstr
//XXX: work out which range of (tgtlen,patlen) for which
//      this is the optimal algorithm
// Hang onto this code as an example of using inline asm.
//  In all other senses, it's lame compared to scanstr,
//  which would be even faster if it didn't have to scan for \0.
char const *
memfind(char const *tgt, int tgtlen, char const *pat, int patlen)
{
#   define  _   "\n"
    asm("cld");
    ++pat;
    if (patlen--) asm(
        _"0:repne scasb"
        _"  jne     8f"
        _"  movl    %%ecx, %%edx"
        _"  movl    %%edi, %%ebx"
        _"  movl    %[pat], %%esi"
        _"  movl    %[len], %%ecx"
        _"  repe cmpsb"
        _"  je	    9f"
        _"  movl    %%edx, %%ecx"
        _"  movl    %%ebx, %%edi"
        _"  jmp	    0b"
        _"8:movl    $1,%%ebx"
        _"9:leal    -1(%%ebx), %%eax"
        :     "=a" (tgt)
        :      "a" (pat[-1]),
               "c" (tgtlen - patlen),
               "D" (tgt),
          [pat]"m" (pat),
          [len]"m" (patlen));

    return  tgt;
}

FILE*
popenf(char const *type, char const *fmt, ...)
{
    char	*cmd;
    va_list	vargs;
    va_start(vargs, fmt);
    int junk = vasprintf(&cmd, fmt, vargs); (void)junk;
    va_end(vargs);
    FILE        *ret = popen(cmd, type);
    free(cmd);
    return  ret;
}

int
refcmp(MEMREF const a, MEMREF const b)
{
    int     rc, na = nilref(a), nb = nilref(b);
    return na || nb ? na - nb
         : (rc = memcmp(a.ptr, b.ptr, int_min(a.len, b.len))) ? rc
         : (int)a.len - (int)b.len;
}

char*
refdup(MEMREF r)
{
    if (nilref(r)) return NULL;
    char *cp = malloc(r.len + 1);
    cp[r.len] = 0;
    return memcpy(cp, r.ptr, r.len);
}

int
refpcmp(MEMREF const *ap, MEMREF const *bp)
{
    int rc = memcmp(ap->ptr, bp->ptr, int_min(ap->len, bp->len));
    return  rc ? rc : (int)ap->len - (int)bp->len;
}

MEMREF *
refsplit(char *text, char sep, int *pcount)
{
    char	*cp;
    int		i, nstrs = 0;
    MEMREF      *strv = NULL;

    if (*text) {
        for (cp = text, nstrs = 1; (cp = strchr(cp, sep)); ++cp) 
            ++nstrs;

        strv = malloc(nstrs * sizeof(MEMREF));

        for (i = 0, cp = text; (cp = strchr(strv[i].ptr = cp, sep)); ++i, ++cp) {
            strv[i].len = cp - strv[i].ptr;
            *cp = 0;
        }

        strv[i].len = strlen(strv[i].ptr);
    }

    if (pcount) 
	*pcount = nstrs;
    return    strv;
}

MEMREF
subref(MEMREF r, int pos, unsigned len)
{ 
    return !r.ptr || !len || pos >= (int)r.len || (pos < 0 && (pos += r.len) < 0)
            ? NILREF : (MEMREF){r.ptr+pos, uns_min(len, r.len-pos)};
}

void
defileref(MEMREF mr)
{ munmap((void*)(intptr_t)mr.ptr, mr.len); }

MEMBUF
slurp(const char *filename)
{
    MEMBUF      ret = NILBUF;
    int         fd = filename && *filename && strcmp(filename, "-") ? open(filename, O_RDONLY) : 0;
    struct stat s;
    if (fd < 0 || fstat(fd, &s))
        goto ERROR;

    if (S_ISREG(s.st_mode)) {
        ret = membuf(s.st_size);
	if (ret.len != (unsigned)read(fd, ret.ptr, ret.len))
            goto ERROR;

    } else {
        int     len, size = 4096;
        ret.ptr = malloc(size + 1);
        for (;0 < (len = read(fd, ret.ptr+ret.len, size-ret.len)); ret.len += len)
            if (len == size - (int)ret.len)
                ret.ptr = realloc(ret.ptr, (size <<= 1) + 1);
        if (len < 0)
            goto ERROR;
        ret.ptr = realloc(ret.ptr, ret.len + 1);
    }

    close(fd);
    ret.ptr[ret.len] = 0;
    return  ret;

ERROR:
    if (fd >= 0) close(fd);
    buffree(ret);
    return NILBUF;
}

char *
strim(char *s)
{
    char    *e = s + strlen(s);
    while (e > s && isspace(e[-1])) --e;
    *e = 0;
    while (isspace(*s)) ++s;
    return  s;
}

int
systemf(char const *fmt, ...)
{
    char	*cmd;
    va_list	vargs;
    va_start(vargs, fmt);
    int ret = vasprintf(&cmd, fmt, vargs);
    va_end(vargs);
    ret = system(cmd);
    free(cmd);
    return  ret;
}

double
tick(void)
{
    struct timeval t;
    gettimeofday(&t, 0);
    return t.tv_sec + 1E-6 * t.tv_usec;
}
