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

#include "tap.h"
#include "msutil.h"
#include "xmutil.h"

int
main(void)
{
    char        buf[99];
    plan_tests(31);

    MEMREF  mr = fileref(__FILE__);
    ok(!nilref(mr), "fileref("__FILE__")");
    ok(!memcmp(mr.ptr, "// Copyright", 12), "readable");
    ok(!memcmp(mr.ptr + mr.len - 7, "\n//EOF\n", 7), "to the end");
    defileref(mr);  // coverage

    FILE *fp = popenf("r", "echo hello");
    ok(fp, "popenf works");
    ok(fgets(buf, 99, fp) && !strcmp(buf, "hello\n"), "popenf cmd works");
    ok(!systemf("PS4='# '; set -x; echo '# %d'", 666), "systemf works");
    int     act;
#undef TRY
#define TRY(x,exp) act=fls(x); if (!ok(act==exp, "fls(0x%X) = %d", x,exp)) fprintf(stderr, "# returned: %d\n", act)
    TRY( 0,  0);
    TRY( 1,  1);
    TRY(64,  7);
    TRY(66,  7);
    TRY(-1, 32);

    ok(!strcmp(getprogname(), "msutil_t"), "getprogname");

#undef TRY
#define TRY(x,exp) ok(!strcmp(strim(strcpy(buf,x)), exp), "strim('%s')", x)
    TRY("", "");
    TRY(" \t  \n  \r", "");
    TRY("abc", "abc");
    TRY(" abc", "abc");
    TRY(" abc  ", "abc");
    TRY("a b c", "a b c");

//TODO:
// addr_part(MEMREF r)
// belch(char const *filename, MEMREF data)
// bit_count(char const*vec, int len)
    char    data[49] = {
        15,  0,  2,  0,  6,  0,  2,  0, 15,  0,  2,  0,  6,  0,  2,  0,
        15,  0,  2,  0,  6,  0,  2,  0, 15,  0,  2,  0,  6,  0,  2,  0,
        15,  0,  2,  0,  6,  0,  2,  0, 15,  0,  2,  0,  6,  0,  2,  0, 15
    };
    int     iact = bit_count(data, 49);
    ok(iact == 52, "bit_count is %d", iact);
    
    MEMBUF      mb = NILBUF;
    bufcat(&mb, strref("hello,"));
    bufcat(&mb, strref(" world"));
    ok(!refcmp(bufref(mb), strref("hello, world")), "bufcat works");
    ok(0 < refcmp(bufref(mb), strref("hello")), "refcmp works");
    char        *cp = refdup(bufref(mb));
    ok(!strcmp(cp, "hello, world"), "refdup works");
    buffree(mb);

// die(char const *fmt, ...)
// findbit_1
    int i, off, succ = 0;
    for (off = 0; off < 16; ++off) {
        for (i = 0; i < 256; i++) {
            uint8_t ff[50] = {};
            ff[off + (i >> 3)] = 1 << (i & 7);
            iact = findbit_1(ff + off, off + (i >> 3) + 2);
            if (iact == i) ++succ;
            else fprintf(stderr, "%3d %3d %3d\n", off, i, iact);
        }
    }
    ok(succ == 4096, "%d/4096 findbit_1 tests passed", succ);
// fopenf(char const *mode, char const *fmt, ...)
// memfind(char const *tgt, int tgtlen, char const *pat, int patlen)

    MEMREF  x[] = { strref("middle"), strref("head"), strref("tail") };
    qsort(x, 3, sizeof(MEMREF), (qsort_cmp)refpcmp);
    ok(refcmp(x[0], x[1]) < 0 && refcmp(x[1], x[2]) < 0, "refpcmp works");

// vec_new(int width, DTOR *dtor, void *context)
// vec_push(VEC* v, void *elem)
// vec_allow(VEC* v, int limit)
// vec_resize(VEC* v, int limit)
// vec_free(VEC* v)

    char    *cs = acstr("one\2\t\377\0'two\0\0\0", 14);
    ok(!strcmp(cs,  "\"one\\x02\\t\\xFF\\x00'two\""), "acstr formats buffer as %s", cs);

    uint32_t    a32 = revbit32(0x2357BD0F);
    ok(a32 == 0xF0BDEAC4, "rev(2357BD0F) = %8X", a32);

    uint64_t    a64 = revbit64(0xC23456789AB10DEFULL);
    ok(a64 == 0xF7B08D591E6A2C43ULL, "rev(C23456789AB10DEF) = %16llX", a64);

    YMD     ymd, exp;
    exp = (YMD) { 2004,02,29 };
    unsigned day = ymd2day(exp);
    ymd = day2ymd(day);
    ok(!memcmp(&ymd, &exp, sizeof(YMD)), "%d/%02d/%02d", exp.yyyy, exp.mm, exp.dd);

    exp = (YMD) { 2004,03,01 };
    ymd = day2ymd(day+1);
    ok(!memcmp(&ymd, &exp, sizeof(YMD)), "+ 1 = %d/%02d/%02d", exp.yyyy, exp.mm, exp.dd);

    exp = (YMD) { 2000,12,31 };
    ymd = day2ymd(ymd2day((YMD){2001,1,1}) - 1);
    ok(!memcmp(&ymd, &exp, sizeof(YMD)), "%d/%02d/%02d", exp.yyyy, exp.mm, exp.dd);

    exp = (YMD) { 2005,03,01 };
    ymd = day2ymd(ymd2day((YMD){2005,02,29}));
    ok(!memcmp(&ymd, &exp, sizeof(YMD)), "2005/02/29 => %d/%02d/%02d", exp.yyyy, exp.mm, exp.dd);

    return exit_status();
}
//EOF
