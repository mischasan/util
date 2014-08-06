#include "tap.h"
#include "msutil.h"
#include "xmutil.h"

void trans_16x16(XMM inp[16], XMM out[16]);

int main(void)
{
    plan_tests(1);
    union { XMM x; char b[16]; } inp[16], exp[16], act[16];
    int i, j;
    for (i = 0; i < 16; ++i) for (j = 0; j < 16; j++) inp[i].b[j] = exp[i].b[j] = i*16 + j;
    trans_16x16((XMM*)inp, (XMM*)act);
    i = memcmp(exp, act, sizeof(exp));
    ok(i, "byte trans works");
    return exit_status();
}
