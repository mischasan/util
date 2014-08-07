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

#include <msutil.h>
#include "tap.h"
#include "bloom.h"

extern int _bloomax;

int
main(void)
{
    plan_tests(6);
    setvbuf(stdout, 0, _IOLBF, 0);

    BLOOM *bp = bloom_create(12, 16, 2);
    ok(bp, "created");

    int rc = bloom_chk(bp, "AB");
    ok(rc == 0, "Check 'AB' not in empty table: %d", rc);

    printf("# Add AB\n");
    bloom_add(bp, "AB");

    rc = bloom_chk(bp, "AB");
    ok(rc != 0, "Check 'AB' after add: %d", rc);

    printf("# Add AC\n");
    bloom_add(bp, "AC");

    rc = bloom_chk(bp, "!@");
    ok(rc != 0, "Check '!@' without add: %d (false positive)", rc);

    printf("# Before adding CA...DZ\n");
    bloom_dump(bp, stdout);

    int antestat = bloom_stat(bp), anteover = bloom_over(bp);

    char hash[] = "__";
    for (hash[1] = 'A'; hash[1] <= 'D'; ++hash[1])
        for (hash[0] = 'C'; hash[0] <= 'Z'; ++hash[0])
            bloom_add(bp, hash);

    printf("# After adding CA...DZ:\n");
    bloom_dump(bp, stdout);

    for (hash[1] = 'A'; hash[1] <= 'D'; ++hash[1])
        for (hash[0] = 'C'; hash[0] <= 'Z'; ++hash[0])
            bloom_del(bp, hash);

    printf("# After deleting CA...DZ:\n");
    bloom_dump(bp, stdout);

    int poststat = bloom_stat(bp), postover = bloom_over(bp);
    ok(poststat == antestat, "stat %d -> %d", antestat, poststat);
    ok(postover == anteover, "over %d -> %d", anteover, postover);

    bloom_destroy(bp);

    return exit_status();
}
