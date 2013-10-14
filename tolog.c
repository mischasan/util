#undef  _GNU_SOURCE
#define _GNU_SOURCE
#include "msutil.h"
#include <ctype.h>
#include <syslog.h>

//XXX: map aliases FATAL=CRIT ERROR=ERR WARN=WARNING NOTE=NOTICE DIAG=DEBUG
static char const *priov[] = {
    "EMERG:",   "ALERT:",  "CRIT:", "ERR:",
    "WARNING:", "NOTICE:", "INFO:", "DEBUG:"
};

static int writer(void *_log, char const *data, int leng)
{
    (void)_log;
    int     p = LOG_DEBUG, len;

    do len = strlen(priov[p]);
    while (memcmp(data, priov[p], len) && --p >= 0);

    if (p < 0)  p = LOG_INFO;
    else        data += len, leng -= len;

    while (leng && isblank(*data)) ++data, --leng;

    syslog(p, "%.*s", leng, data);
    return  leng;
}

#if defined(__linux__)
static int noop(void) {return 0;}
#endif

void tolog(FILE **pfp)
{
#if defined(__FreeBSD__)
    *pfp = fwopen(NULL, writer);
#elif defined(__linux__)
    *pfp = fopencookie(NULL, "w", (cookie_io_functions_t) {(void*)noop, (void*)writer, (void*)noop, (void*)noop});
#else
#   error "This version of Unix has fwopen?fopencookie?"
#endif
    setvbuf(*pfp, NULL, _IOLBF, 0);
}
