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

#include <errno.h>
#include <stdarg.h>
#include "msutil.h"

static double start;
static void diag(char const*fmt, ...);
static void *flagging(void *_flag);
static void *sleeping(void *_flag);

int main(void)
{
    start = tick();
    setvbuf(stderr, NULL, _IOLBF, 0);

    FLAG   *flag = flag_create();
    THREAD *thrd = thread_start(flagging, flag);

    int rc;
#undef  TRY
#define TRY(x) diag(#x " ...\n"); rc = (intptr_t)(x); diag(#x ": %d %s", rc, errname[errno])
    TRY(flag_watch(flag, 0));
    TRY(flag_watch(flag, 0.5));
    TRY(flag_watch(flag, 0.5));
    TRY(sleep(1));

    thread_result(thrd);

    thrd = thread_start(sleeping, flag);
    TRY(thread_cancel(thrd));
    TRY(thread_result(thrd));

    flag_destroy(flag);

    return 0;
}

static void
diag(char const*fmt, ...) 
{
    va_list     ap;
    va_start(ap, fmt);
    fprintf(stderr, "%.6f ", tick() - start);
    vfprintf(stderr, fmt, ap);
    putc('\n', stderr);
}

#undef  TRY
#define TRY(x) diag("\t" #x); x

static void*
flagging(void *_flag)
{
    FLAG    *flag = _flag;

    diag("\tflagging thread starts", flag);
    TRY(sleep(1));
    TRY(flag_any(flag));
    TRY(flag_all(flag));
    
    diag("\tthread exit");
    return _flag;
}

static void*
sleeping(void *arg)
{
    (void)arg;
    diag("\tsleeping thread starts");
    TRY(sleep(1));
    diag("\tNOTREACHED");
    return arg;
}
