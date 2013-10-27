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

// This program demonstrates that fopencookie (in tolog)
// is magic. There are no hook pointers in the FILE{} itself.
// And fputs(..., stdout) bypasses the hook!

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> // dup

void tolog(FILE **pfp);

// Mock syslog:
void syslog(int pri, const char *fmt, ...);
char logbuf[9999];

int
main(void)
{
    FILE *fp = fdopen(dup(1), "w");
    strcpy(logbuf, "<nothing yet>");

    tolog(&stdout);
    puts("PUTS");
    fprintf(fp, "PUTS >>> '%s'\n", logbuf);

    printf("NO NEWLINE");
    fprintf(fp, "NO NEWLINE >>> '%s'\n", logbuf);

    //setvbuf(stdout, NULL, _IOLBF, 0); 
    printf(" THEN NEWLINE\n");
    fprintf(fp, "THEN NEWLINE >>> '%s'\n", logbuf);

    fputs("FPUTS", stdout);
    fprintf(fp, "FPUTS >>> '%s'\n", logbuf);

    return 0;
}

void syslog(int pri, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsprintf(logbuf + sprintf(logbuf, "[%d] ", pri), fmt, ap);
}
