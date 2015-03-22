//NOTE: msutil.h defines rollhash functions as inlines; do not 

#ifndef ROLLHASH_MOD
#define ROLLHASH_MOD 8355967

#include <stdint.h>

// rollhash_arg: compute the multiplier for a window of (num) bytes,
//                  i.e. (pow(256,leng - 1) mod ROLLHASH_MOD)
uint32_t rollhash_arg(uint32_t num);

// rollhash_init: compute hash of initial window
uint32_t rollhash_init(uint8_t const*data, uint32_t leng);

// rollhash_step: update (hash) where (old) is what leaves the window and (new) is what enters.
uint32_t rollhash_step(uint32_t arg, uint32_t hash, uint8_t lose, uint8_t gain);

// ROLLHASH_MOD: a large prime small enough that ROLLHASH_MOD*(256+1) does not overflow uint32
uint32_t
rollhash_arg(uint32_t num)
{
    uint32_t arg = 1;
    while (--num) arg = arg * 256 % ROLLHASH_MOD;
    return arg;
}

uint32_t
rollhash_init(uint8_t const*data, uint32_t leng)
{
    uint32_t hash = 0;
    while (leng--) hash = (hash * 256 + *data++) % ROLLHASH_MOD;
    return hash;
}

// rollhash_step does 5 MUL's (% implemented with MUL and SHIFT).
uint32_t
rollhash_step(uint32_t arg, uint32_t hash, uint8_t lose, uint8_t gain)
{
    return ((hash + 256*ROLLHASH_MOD - lose * arg) % ROLLHASH_MOD * 256 + gain) % ROLLHASH_MOD;
}
#endif
