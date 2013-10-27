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

/*
** Copyright (C) 2009-2013 Mischa Sandberg <mischasan@gmail.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include "_psearch.h"
#ifndef MAP_NOCORE
# define MAP_NOCORE 0
#endif

void
psearch_save(FILE *fp, PSEARCH const*psp)
{
    PSEARCH ps = *psp;
    // Overwrite pointers with magic signature.
    assert(8 <= sizeof ps.tranv + sizeof ps.hashv);
    memcpy(&ps, "ACMischa", 8);
    ps.flags &= ~IS_MMAP;
    int len = fwrite(&ps, sizeof(PSEARCH), 1, fp);
    len = fwrite(psp->tranv, p_size(psp), 1, fp);
    (void)len;
}

PSEARCH*
psearch_load(FILE *fp)
{
    PSEARCH *psp = calloc(sizeof(PSEARCH), 1);

    if (fread(psp, sizeof(PSEARCH), 1, fp) == 1
            && !memcmp(psp, "ACMischa", 8)
            && (set_tranv(psp, malloc(p_size(psp))), 1)
            && fread(psp->tranv, p_size(psp), 1, fp)) {
        return psp;
    }

    psearch_destroy(psp);
    return NULL;
}

PSEARCH*
psearch_mmap(FILE *fp)
{
    char *mp = mmap(0, lseek(fileno(fp), 0L, 2), PROT_READ,
                    MAP_SHARED|MAP_NOCORE, fileno(fp), 0);
    if (mp == MAP_FAILED) return NULL;

    PSEARCH *psp = malloc(sizeof*psp);
    *psp = *(PSEARCH*)mp;
    psp->flags |= IS_MMAP;
    if (memcmp(psp, "ACMischa", 8)) {
        psearch_destroy(psp);
        return NULL;
    }

    set_tranv(psp, mp + sizeof(PSEARCH));
    return psp;
}

void
psearch_destroy(PSEARCH *psp)
{
    if (!psp) return;
    if (psp->flags & IS_MMAP)
        munmap((char*)psp->tranv - sizeof(PSEARCH),
               sizeof(PSEARCH) + p_size(psp));
    else free(psp->tranv);
    free(psp);
}
//EOF
