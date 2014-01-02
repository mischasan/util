#include <tap.h>
#include "msutil.h"

int main(void)
{
    plan_tests(14);

    uint8_t test[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    uint32_t i, arg = rollhash_arg(12), roll = rollhash_init(test, 12);

    for (i = 0; i < 26 - 12; i++) {
        uint hash = rollhash_init(test + i + 1, 12);
        roll = rollhash_step(arg, roll, test[i], test[i+12]);
        ok(roll == hash, "-%c +%c", test[i], test[i+12]);
    }   

    return exit_status();
}   

