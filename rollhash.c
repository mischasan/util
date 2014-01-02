#include "msutil.h"

// MOD: a prime large enough that MOD*(256+1) does not overflow uint32
#define MOD 8355967

uint32_t rollhash_arg(uint32_t num)
{
    uint32_t arg = 1;
    while (--num) arg = arg * 256 % MOD;
    return arg;
}

uint32_t rollhash_init(uint8_t *data, uint32_t leng)
{
    uint32_t hash = 0;
    while (leng--) hash = (hash * 256 + *data++) % MOD;
    return hash;
}

uint32_t rollhash_step(uint32_t arg, uint32_t hash, uint8_t old, uint8_t new)
{
    return ((hash + 256*MOD - old * arg) % MOD * 256 + new) % MOD;
}
