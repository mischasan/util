#include <stdio.h>
#include <stdlib.h>        // getenv
#include "tap.h"
#include "concur.h"

int
main(void)
{
    plan_tests(1);
    fprintf(stderr, "# t.concur: $CONCUR=%s\n", getenv("CONCUR"));
    concur("ignored event");
    char    set[] = "CONCUR=t.concur";
    putenv(set);
    concur("logged event");
    concur_exit();
    ok(1, "done");

    return    0;
}
