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
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>      // open
#include <unistd.h>     // unlink
#include <sys/mman.h>
#include "tap.h"

typedef void(*vvfunc)(void);
extern uintptr_t maccess_top(void);

int main(void)
{
    plan_tests(11);

    char        *foo = malloc(100000);
    strcat(strcpy(foo, getenv("util") ? getenv("util") : "."), "/mac.so");
    void        *dlp = dlopen(foo, RTLD_LAZY);
    vvfunc      fnp = (vvfunc) dlsym(dlp, "macfn");

    ok(maccess(&foo, sizeof(foo), 0), "%p stack local", &foo);
    ok(maccess(foo, 100000, 0), "%p heap memory", foo);
    ok(!maccess(NULL, 10, 0), "null");
    ok(!maccess((void*)-1, 10, 0), "-1");
    ok(!maccess((void*)12345, 1, 0), "short int as a pointer");
#ifdef TODO
    //FreeBSD allocates the heap in great big virtual chunks,  so this doesn't work.
    ok(!maccess(foo, 234567, 0), "%p beyond heap memory", foo+234566);
#endif
    ok(!maccess(main, 1, 0), "%p code", main);
    ok(maccess(fnp, 1, 0), "%p dl-code", fnp);

    uintptr_t   top = maccess_top()+1;
    fprintf(stderr, "# maccess_top: d 0x%"FPTR"X\n", top-1);
#ifdef TODO
    int         i, x = 0, y = 0, count = 1000000;
    double      start = tick();
    for (i = 0; i < count; ++i) x += access((char const*)0x80000000+i, F_OK) && errno == EFAULT;
    double      ta = tick() - start;
    start = tick();
    for (i = 0; i < count; ++i) y += maccess((char const*)0x80000000+i, 64, 0);
    ok(x == y, "%d:%d access(%.4f secs) confirms maccess(%.4f secs) TODO", x, y, ta, tick() - start);
#endif
    //XXX change mmap test because first mmap goes into a segment
    //      that is already defined (writable!).
    //XXX add tests using mprotect to make mem readonly.

    unlink("mac.tmp");
    int     fd = open("mac.tmp", O_CREAT|O_RDWR, 0666);   // 101 bytes long
    write(fd, "----+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----1", 101);
    char    *map = mmap(NULL, 101, PROT_READ, MAP_SHARED, fd, 0);
    ok(map != MAP_FAILED, "mmap(rdonly) succeeds");

    maccess_init();
    ok(maccess(map, 100, 0), "%p mmap after maccess_init", map);
    ok(!maccess(map, 100, 1), "%p mmap(w) returns false", map);
    munmap(map, 101);
    ok(maccess(map, 100, 0), "%p after munmap", map);
    maccess_init();
    fputs("# TODO: ok(!maccess(map, 100, 0), \"%p after maccess_init\", map)\n", stderr);
    close(fd);
    unlink("mac.tmp");
#ifdef Linux
    // FreeBSD uses .../map, not .../maps:
    sprintf(foo, "/proc/%d/maps", getpid());
    FILE        *fp = fopen(foo, "r");
    while (fgets(foo, 1000, fp))
        fprintf(stderr, "# %s", foo);
    fclose(fp);
#endif
    return exit_status();
}
