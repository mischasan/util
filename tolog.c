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
