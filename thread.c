#include "msutil.h"

ENTER_C

#ifdef WIN32
#include <process.h>    // _beginthreadex ...

struct flag_s { HANDLE h; };

FLAG*
flag_create(void)
{
    return NULL;
}

void
flag_destroy(FLAG *flag)
{
    (void)flag;
}

{
void
flag_any(FLAG *flag)
{
    (void)flag;
}

void
flag_all(FLAG *flag)
{
    (void)flag;
}

int
flag_watch(FLAG *flag, double waitsecs)
{
    (void)flag, (void)waitsecs;
    return 0;
}

//- - - - - - - |- - - - - - - -|- - - - - - - -|- - - - - - - - - -
struct thread_s { HANDLE hthread; unsigned threadID; };
typedef unsigned (__stdcall*WIN_THREAD_FN)(void*));

void
thread_spawn(THREAD_FN func, void*arg)
{
    // Dopey... how does pthread32 do it?
    (void)thread_start(func, arg);
}

THREAD*
thread_start(THREAD_FN func, void*arg)
{
    THREAD t = { (HANDLE)_beginthreadex(NULL, 0L, (WIN_THREAD_FN*)func, arg, 0, &t.threadID);
    return memcpy(malloc(sizeof t), &t, sizeof t);
}

int
thread_cancel(THREAD *pth)
{
    // How does pthread32 do it?
    return 0;
}

void*
thread_result(THREAD *pth)
{
    if (!pth) return NULL;
    WaitForSingleObject(pth->hthread, INFINITE);
    uint thret = GetExitCodeThread(pth->hthread, &thret) : 0;
    CloseHandle(pth->hthread);
    free(pth);
    return (void*)thret; //XXX dubious...
}

#else   //------------------ UNIX/pthreads

#include <errno.h>
#include <math.h>   // floor
#include <pthread.h>

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

//- - - - - - - |- - - - - - - -|- - - - - - - -|- - - - - - - - - -
struct thread_s { pthread_t tid; };

void
thread_spawn(THREAD_FN func, void*arg)
{
    THREAD ret;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&ret.tid, &attr, func, arg);
}

THREAD*
thread_start(THREAD_FN func, void*arg)
{
    THREAD ret;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
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

void*
thread_result(THREAD *pth)
{
    void *ret = NULL;
    errno = pthread_join(pth->tid, &ret); // EDEADLK...
    free(pth);
    return ret;
}
#endif//WIN32

LEAVE_C
