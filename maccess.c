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

//TODO: build two maps in parallel: read-only and read-write
//  Both will be smaller than the combined map, since 
//  read-only and read-write segments frequently abut.

#include <assert.h>
#include <unistd.h>     // getpid...
#include "msutil.h"

#if defined(__FreeBSD__)
#   define  MAPFILE "map"
#   define  MAPSCAN "%p %p %*s %*s %*s %[^\n]"
#elif defined(__linux)
#   define  MAPFILE "maps"
#   define  MAPSCAN "%p-%p%[^\n]"
#else
#   error
#endif
extern int etext;
#define MAXRNGS 100
static struct rng { uintptr_t alpha, omega; int writable; } rngv[MAXRNGS], *rend;

int maccess_top(void);  // Only for testing

void
maccess_init(void)
{
    uintptr_t       brkptr = (uintptr_t)sbrk(0);
    char            buf[999];
    sprintf(buf, "/proc/%d/"MAPFILE, getpid());
    FILE            *fp = fopen(buf, "re");
    assert(fp);

    rend = rngv;
    while (3 == fscanf(fp, MAPSCAN, (void**)&rend->alpha, (void**)&rend->omega, buf)) {
        rend->writable = buf[1] == 'w';
        if ((buf[1] == '-' && buf[2] == 'x') || rend->alpha < brkptr)
            continue;
        if (rend > rngv && rend->alpha == rend[-1].omega && rend->writable == rend[-1].writable)
            rend[-1].omega  = rend->omega;
        else
            rend++;
    }
    assert(rend < rngv+MAXRNGS);
    fclose(fp);
}

int
maccess(void const *mem, int len, int mode)
{
    if ((intptr_t)mem < 0 && (intptr_t)mem + len >= 0)
        return 0;
    // On my AMD64, Debian: sbrk() takes .01 usec.
    if ((char const*)mem + len <= (char const*)sbrk(0))
        return mem > (void*)&etext;
    if (!rend)
        maccess_init();

    struct rng *p;
    for (p = rngv; p != rend; ++p)
        if ((uintptr_t)mem + len <= p->omega)
            return (uintptr_t)mem >= p->alpha && p->writable >= mode;
    return 0;
}

int
maccess_top(void) { if (!rend) maccess_init(); return rend[-1].omega; }
#if 0
FREEBSD:
0x80d5000 0x80d9000 4 0 0xce25a770 rw- 1 0 0x3000 COW NNC vnode /usr/bin/bash CH 1013
0xbfbe0000 0xbfc00000 9 0 0xcccbd550 rwx 1 0 0x3000 COW NNC default - CH 1013

LINUX:
006b2000-006bc000 rw-p 000b2000 fd:00 80707616                           /bin/bash
7fff096fa000-7fff096fd000 r-xp 7fff096fa000 00:00 0                      [vdso]
ffffffffff600000-ffffffffffe00000 ---p 00000000 00:00 0                  [vsyscall]

#endif
