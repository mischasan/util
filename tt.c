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
