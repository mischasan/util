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

#include "msutil.h"
#include <tap.h>

static inline int isign(int a)
{ return a < 0 ? -1 : !!a; }

int
main(void)
{
    plan_tests(83);

#undef  TRY
#define TRY(tgt,pat) ok(scanstr(tgt,pat) == strstr(tgt,pat), "'%s'~'%s' -> %p:%p", tgt, pat, scanstr(tgt,pat), strstr(tgt,pat))
    TRY("", "a");
    TRY("hello", "");
    TRY("art", "xe");

    TRY("hello", "h");
    TRY("hello", "e");
    TRY("hello", "l");
    TRY("hello", "p");

    TRY("ab", "cd");
    TRY("ab", "ab");
    TRY("cab", "ab");
    TRY("dcab", "ab");
    TRY("edcab", "ab");
    TRY("edcabe", "ab");
    TRY("fedcabf", "ab");

    char ab[] = "ab";
    TRY(ab, "abc");
    TRY("abc", "xbc");
    TRY("abc", "axc");
    TRY("abc", "abx");
    TRY("abc", "abc");
    TRY("abcd", "abc");
    TRY("zabcd", "abc");
    TRY("yzabcd", "abc");
    TRY("xyzabcd", "abc");
    TRY("wxyzabcd", "abc");
    TRY("xabc", "abc");

    TRY("", "abcd");
    TRY("a", "abcd");
    TRY("ab", "abcd");
    TRY("abcd", "xbcd");
    TRY("abcd", "axcd");
    TRY("abcd", "abxd");
    TRY("abcd", "abcx");
    TRY("abcd", "abcd");
    TRY("abcde", "abcd");
    TRY("zabcd", "abcd");
    TRY("yzabcd", "abcd");
    TRY("xyzabcd", "abcd");
    TRY("vwxyzabcd", "abcd");
    TRY("uvwxyzabcd", "abcd");
    TRY("tuvwxyzabcd", "abcd");
    TRY("stuvwxyzabcd", "abcd");
    TRY("rstuvwxyzabcd", "abcd");
    TRY("qrstuvwxyzabcd", "abcd");
    TRY("qrstuvwxyzabcdabcd", "abcd");
    TRY("xabcde", "abcd");

    TRY("", "abcde");
    TRY("a", "abcde");
    TRY("ab", "abcde");
    TRY("abcde", "xbcd");
    TRY("abcde", "axcd");
    TRY("abcde", "abxd");
    TRY("abcde", "abcx");
    TRY("abcde", "abcde");
    TRY("abcdee", "abcde");
    TRY("zabcde", "abcde");
    TRY("yzabcde", "abcde");
    TRY("xyzabcde", "abcde");
    TRY("vwxyzabcde", "abcde");
    TRY("uvwxyzabcde", "abcde");
    TRY("tuvwxyzabcde", "abcde");
    TRY("stuvwxyzabcde", "abcde");
    TRY("rstuvwxyzabcde", "abcde");
    TRY("qrstuvwxyzabcde", "abcde");
    TRY("qrstuvwxyzabcdeabcde", "abcde");
    TRY("xabcdee", "abcde");
    char y[] = "451AC87F_1271_3613_1\t2006-09-27T18:52:48|f=<pgsql-bugs-owner+M15844@postgresql.org>|t=<mischa@ca.sophos.com>|h=__FRAUD_419_REFNUM|h=__HAS_MSGID|h=__MIME_TEXT_ONLY|h=__SANE_MSGID|Size=7477|inbound|action=deliver|p=0.076|S=?q?[BUGS]_BUG_#2653:_failed_to_make|b=ok|r=200.46.204.254|tm=1.00|a=a/eom";
    TRY(y,"009");

#undef  TRY
#define TRY(tgt,pat) ok(scanbrk(tgt,pat) == strpbrk(tgt,pat), "'%s'~[%s] -> %p:%p", tgt, pat, scanbrk(tgt,pat), strpbrk(tgt,pat))
    TRY("hello, world", "wa");
    TRY("hello, world", "wo");
    TRY("hello, world", "xy");
    TRY("hello, world", "war");
    TRY("hello, world", "dar");
    TRY("hello, world", "xyz");
    TRY("", "");
    TRY("", "abcde");
    TRY("abcde", "");
    TRY("hello, world", "w");
    TRY("abcdefghijklmnopqrstuvwxyz","fghijklmnopqrstuvwxyz");

    char x[] = "451A8BF4_1271_3531_1\t2006-09-27T14:34:30|f=<borea";
    TRY(x, "ab");
#undef TRY
#define TRY(x,y)    ok(isign(scancmp(x,y)) == isign(strcmp(x,y)), "'%s'=='%s' -> %d:%d", x,y,scancmp(x,y), strcmp(x,y));

    TRY("hello, world", "hello, world");
    TRY("hellow, world", "hello, world");

    union {double a; char s[99]; } d0, d1;

    strcpy(d0.s, "hello, world");
    strcpy(d1.s, "hello, world");
    TRY(d0.s, d1.s);

    int         i, j;
    for (i = 1; i <= 64; ++i) {
        for (j = 32; j < 64; ++j) {
            char * pat = refdup(subref(strref(y), i, j));
            if (scanstr(y, pat) != strstr(y, pat)) break;
            free(pat);
        }
    }
    ok(i == 65, "scanstr[%d,%d]", i, j);

    __attribute__((aligned(16))) const char edge[] = "0123456789abdef0123456789abcdef";
    TRY(edge+1, "01");

    return exit_status();
}
