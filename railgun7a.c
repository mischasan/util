#ifndef STR_X
#include <stdint.h>
#include <string.h>
char *Railgun7a(char *tgt, int tgtlen, char *pat, int patlen);
#endif
char *
Railgun7a(char *target, int tgtlen, char *pattern, int patlen)
{
    uint8_t     *tgt = (uint8_t*)target, *pat = (uint8_t*)pattern;
    uint8_t     *tgtMax = tgt + tgtlen;
    uint32_t    patwin, tgtwin;
    int         skip, count;
    int         bm_bc[256];

    if (patlen > tgtlen)
        return NULL;
    if (patlen == 1)
        return memchr(tgt, *pat, tgtlen);
    if (patlen == 2) {
        tgt += 2;
        patwin =(pat[0] << 8) + pat[1];
        while (1) {
            if (patwin == ((uint32_t)tgt[-2] << 8) + tgt[-1])
                return (char*)tgt - 2;
            tgt += (uint8_t)(patwin >> 8) == tgt[-1] ? 1 : 2;
            if (tgt > tgtMax)
                return NULL;
        }
    } else if (patlen == 3) {
        tgt += 3;
        // Why is this better than *(uint32_t*)pat & 0x00FFFFFF ??
        patwin =(pat[0] << 8) + pat[2];
        while (1) {
            if (patwin == ((uint32_t)tgt[-3] << 8) + tgt[-1]) {
                if (pat[1] == tgt[-2])
                    return (char*)tgt - 3;
            }

            tgt += (uint8_t)(patwin >> 8) == tgt[-2] ? 1 : 2;
            if (tgt > tgtMax)
                return NULL;
        }
    } else if (tgtlen < 961) {// || patlen < 11) {    // 1024 - 64 + 1 ?!
        tgt = tgt + patlen;
        patwin = *(uint32_t*)pat;
        uint8_t     pat0 = (uint8_t)patwin;
        uint32_t    ppp0 = (pat0 << 8) | (pat0 << 16) | (pat0 << 24);
        while (1) {
            tgtwin = *(uint32_t*) (tgt - patlen);
            if (patwin == tgtwin) {
                skip = 1;
                for (count = patlen - 1; count && pat[patlen - count] == tgt[-count]; count--) {
                    if (patlen - 1 == skip + count && pat0 != tgt[-count])
                        skip++;
                }

                if (count == 0)
                    return (char*)tgt - patlen;
            }
            else {          // The goal here: to avoid memory accesses by stressing the registers.
                uint32_t    diff = tgtwin ^ ppp0;
                skip  = !(diff & 0x0000FF00) ? 1 
                      : !(diff & 0x00FF0000) ? 2
                      : !(diff & 0xFF000000) ? 3 : 4;
            }

            tgt = tgt + skip;
            if (tgt > tgtMax)
                return NULL;
        }
    } else {
        // Horspool with an improved 4byte test instead of a single-char test
        // XXX ?worth trying to do the 4byte match overlapping the last
        // (lookup) byte?
        int      i;
        for (i = 0; i < 256; i++)
            bm_bc[i] = patlen;

        for (i = 0; i < patlen - 1; i++)
            bm_bc[pat[i]] = patlen - i - 1;

        patwin = *(uint32_t*)pat;
        for (i = 0; i <= tgtlen - patlen; i = i + bm_bc[tgt[i + patlen - 1]]) {
            if (*(uint32_t*) &tgt[i] == patwin) {
                count = patlen - 4;
                while (count > 0 && pat[patlen - count] == tgt[i + patlen - count])
                    count--;
                if (count == 0)
                    return (char*)tgt + i;
            }
        }

        return NULL;
    }
}
