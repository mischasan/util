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
#include <math.h>       // floor
#include <pthread.h>
#include "msutil.h"

struct flag_s { pthread_cond_t  cond; pthread_mutex_t lock; };

FLAG*
flag_create(void)
{
    FLAG flag = { PTHREAD_COND_INITIALIZER, PTHREAD_MUTEX_INITIALIZER };
    return memcpy(malloc(sizeof flag), &flag, sizeof flag);
}

void
flag_destroy(FLAG *flag)
{
    if (!flag) return;
    pthread_cond_destroy(&flag->cond);
    pthread_mutex_destroy(&flag->lock);
    free(flag);
}

void
flag_any(FLAG *flag)
{
    pthread_cond_signal(&flag->cond);
}

void
flag_all(FLAG *flag)
{
    pthread_cond_broadcast(&flag->cond);
}

int
flag_watch(FLAG *flag, double waitsecs)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    waitsecs += tv.tv_sec + 1E-6*tv.tv_usec;

    struct timespec ts;
    ts.tv_sec = floor(waitsecs);
    ts.tv_nsec = (waitsecs - ts.tv_sec) * 1E9;

    pthread_mutex_lock(&flag->lock);
    errno = pthread_cond_timedwait(&flag->cond, &flag->lock, &ts);
    pthread_mutex_unlock(&flag->lock);

    return -!!errno;
}
//--------------|---------------------------------------------
struct thread_s { pthread_t tid; };

THREAD*
thread_start(THREAD_FN func, void*arg)
{
    THREAD ret;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    errno = pthread_create(&ret.tid, &attr, func, arg);
    return errno ? NULL : memcpy(malloc(sizeof ret), &ret, sizeof ret);
}

// A pthread may set its own cancelstate/canceltype
//      to DISABLED/ASYNCHRONOUS. But that's not smart.

int
thread_cancel(THREAD *pth)
{
    return -!!(errno = pthread_cancel(pth->tid));
}

//XXX thread_wait cannot catch detached threads!
void*
thread_wait(THREAD *pth)
{
    void *ret = NULL;
    errno = pthread_join(pth->tid, &ret); // EDEADLK...
    free(pth);
    return ret;
}
