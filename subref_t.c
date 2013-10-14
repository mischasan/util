#include <tap.h>
#include "msutil.h"

int main(void)
{
    plan_tests(9);

    MEMREF  act, x = strref("hello, world");
#   define S(s) ((s) ? (s) : "(null)")
#   define TRY(p, l, exp) \
        if (!ok(!refcmp(act = subref(x,p,l), strref(exp)), "(%d, %d): %s", p, l, S(exp))) \
            fprintf(stderr, "act[%"FSIZE"d]: %.*s\n", act.len, (int)act.len, S(act.ptr))

    TRY(  0,  5, "hello");
    TRY(  2,  3, "llo");
    TRY( -5,  2, "wo");
    TRY( -5, 99, "world");
    TRY(-12,  2, "he");
    TRY(  0, -1, x.ptr);    // (len) is unsigned, so (-1) is just a very big positive number.
    TRY(  0,  0, NULL);
    TRY(  1,  0, NULL);
    TRY(-13,  2, NULL);

    return exit_status();
}
