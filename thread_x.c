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

    thread_wait(thrd);

    thrd = thread_start(sleeping, flag);
    TRY(thread_cancel(thrd));
    TRY(thread_wait(thrd));

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
