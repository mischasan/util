#ifndef ROLLHASH_MOD
#define ROLLHASH_MOD 8355967
#include <stdint.h>

//NOTE: msutil.h defines these as inlines:
uint32_t rollhash_arg(uint32_t num);
uint32_t rollhash_init(uint8_t const*data, uint32_t leng);
uint32_t rollhash_step(uint32_t arg, uint32_t hash, uint8_t old, uint8_t new);

// ROLLHASH_MOD: a large prime small enough that ROLLHASH_MOD*(256+1) does not overflow uint32

uint32_t rollhash_arg(uint32_t num)
{
    uint32_t arg = 1;
    while (--num) arg = arg * 256 % ROLLHASH_MOD;
    return arg;
}

uint32_t rollhash_init(uint8_t const*data, uint32_t leng)
{
    uint32_t hash = 0;
    while (leng--) hash = (hash * 256 + *data++) % ROLLHASH_MOD;
    return hash;
}

// rollhash_step does 5 MUL's (% implemented with MUL and SHIFT).
uint32_t rollhash_step(uint32_t arg, uint32_t hash, uint8_t old, uint8_t new)
{
    return ((hash + 256*ROLLHASH_MOD - old * arg) % ROLLHASH_MOD * 256 + new) % ROLLHASH_MOD;
}
#endif
