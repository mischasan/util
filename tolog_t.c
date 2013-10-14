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
