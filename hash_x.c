#include "msutil.h"
uint8_t hval[32]; 

static void _fnv04(char const *buf, int len, uint8_t *hash)
{ *(uint32_t*)hash = fnv04(buf, len); }

static void _fnv08(char const *buf, int len, uint8_t *hash)
{ *(uint64_t*)hash = fnv08(buf, len); }

int
main(int argc, char **argv)
{
    if (argc != 2) 
        die("Usage: hash_x datafile");

    MEMBUF  data = chomp(slurp(argv[1]));
    if (!data.ptr)
        die(": cannot read %s", argv[1]);

    int     i, nkeys, seed = 0;
    MEMREF  *keys = refsplit(data.ptr, '\n', &nkeys);

    int n = 0;
    static struct { char const *name; double time; } tv[] = {
        { "murmur16",   0 },
        { "cessu4",     0 },
        { "fnv16",      0 },
        { "fnv08",      0 },
        { "fnv04",      0 },
        { "jlu4",       0 },
    };

    double  t0 = tick(), t1;

    for (i = 0; i < nkeys; ++i)
        mmhash3_x86_16((uint8_t const*)keys[i].ptr, keys[i].len, hval, seed);
    tv[n++].time = (t1 = tick()) - t0; t0 = t1;

    for (i = 0; i < nkeys; ++i)
        cessu32((uint8_t const*)keys[i].ptr, keys[i].len, hval);
    tv[n++].time = (t1 = tick()) - t0; t0 = t1;

    for (i = 0; i < nkeys; ++i)
        fnv16(keys[i].ptr, (int)keys[i].len, (char*)hval);
    tv[n++].time = (t1 = tick()) - t0; t0 = t1;

    for (i = 0; i < nkeys; ++i)
        _fnv08(keys[i].ptr, (int)keys[i].len, hval);
    tv[n++].time = (t1 = tick()) - t0; t0 = t1;

    for (i = 0; i < nkeys; ++i)
        _fnv04(keys[i].ptr, (int)keys[i].len, hval);
    tv[n++].time = (t1 = tick()) - t0; t0 = t1;

    for (i = 0; i < n; ++i)
        printf("%9.4f %s\n", tv[i].time, tv[i].name);

    return  0;
}
