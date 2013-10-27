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
#include <stdarg.h>
#include <tap.h>

// Mock syslog; there's no other reliable way to read the log.
void syslog(int pri, const char *fmt, ...);
char buf[9999];
FILE *save;

int
main(void)
{
    plan_tests(2);
    save = stderr;
    tolog(&stderr);
    ok(!!stderr, "non-null");
    fputs("NOTICE: hello, ", stderr);
    fputs("world\n", stderr);

    // Necessary else TAP output goes to syslog.
    stderr = save;

    char    exp[] = "[5] hello, world\n";
    char    *cp = acstr(buf, strlen(buf));
    ok(!strcmp(exp, buf), "fputs(%s, stderr) written as one line to syslog", cp);
    free(cp);
    fclose(stderr); // coverage
    return exit_status();
}

void syslog (int pri, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf + sprintf(buf, "[%d] ", pri), fmt, ap);
    fprintf(save, "buf=[%s]\n", buf);
}
