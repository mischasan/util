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

int main(void)
{
    plan_tests(9);

    MEMREF  act, x = strref("hello, world");
#   define S(s) ((s) ? (s) : "(null)")
#   define TRY(p, l, exp) \
        if (!ok(!refcmp(act = subref(x,p,l), strref(exp)), "(%d, %d): %s", p, l, S(exp))) \
            fprintf(stderr, "act[%"FSIZE"d]: %.*s\n", act.len, (int)act.len, S(act.ptr))

    TRY(  0,  5, "hello");
    TRY(  2,  3, "llo");
    TRY( -5,  2, "wo");
    TRY( -5, 99, "world");
    TRY(-12,  2, "he");
    TRY(  0, -1, x.ptr);    // (len) is unsigned, so (-1) is just a very big positive number.
    TRY(  0,  0, NULL);
    TRY(  1,  0, NULL);
    TRY(-13,  2, NULL);

    return exit_status();
}
