// Copyright (C) 2009-2013 Sophos LLC. All rights reserved.
//-------------------------------------------------------------------

#include "msutil.h"
#include <assert.h>
#include "_ssearch.h"

// VOIDP: cast away const-ness without gcc whinging
#define    VOIDP(x)     ((void*)(uintptr_t)x)
int ssearch_skip[16];

SSEARCH*
ssearch_create(const MEMREF strv[], int nstrs)
{
    int         i, j, cum, nsyms, nsuffixes, suflen;
    const char  *suffix;

    for (i = 0, suflen = 8 * sizeof(MASK); i < nstrs; ++i)
        if (suflen > (int)strv[i].len)
            suflen = strv[i].len;

    if (suflen < 2)
        return NULL;

    SSEARCH        *ssp = calloc(1, sizeof(*ssp));
    ssp->strv   = strv;
    ssp->nstrs  = nstrs;
    ssp->suflen = suflen;

    // Find all distinct bytevals that occur in suffixes
    for (i = nstrs; --i >= 0;)
        for (j = suflen + 1; --j > 0;)
            ssp->symv[(uint8_t)strv[i].ptr[strv[i].len - j]]=1;

    // Create a map from [0..255] to [NONSYM,0..nsyms-1]
    for (i = nsyms = 0; i < 256; ++i)
        ssp->symv[i] = ssp->symv[i] ? nsyms++ : NONSYM;

    // Calculate column count of mapv[][]: a power of 2
    for (ssp->symwid = 1; (1 << ssp->symwid) < nsyms;)
         ++ssp->symwid;

    // Set mapv[][] for each adj pair of bytes in suffixes
    const MASK bits        = (1 << (ssp->suflen - 1)) - 1;
    const int  mapmask  = (1 << 2*ssp->symwid) - 1;

    ssp->mapv   = calloc(nsyms << ssp->symwid, sizeof(MASK));
    for (i = 0 ; i < nstrs; ++i) {
        const char *cp = strv[i].ptr + strv[i].len;
        int            symsym = ssp->symv[(uint8_t)*--cp];
        MASK        mask;

        for (mask = 1; mask & bits; mask <<= 1) {
            int            sym = ssp->symv[(uint8_t)*--cp];
            symsym = ((symsym << ssp->symwid) | sym) & mapmask;
            ssp->mapv[symsym] |= mask;
        }
    }

        // Map each unique suffix to a set of strings

    // Count #(unique suffixes) and #(strings per suffix)
    ssp->suffixh = map_create( (map_diff*)memcmp, 
                                (map_hash*)fnv04, 
                                    VOIDP(ssp->suflen));
    int                *locp, *locv = calloc(nstrs, sizeof(int));
    for (i = nsuffixes = 0; i < nstrs; ++i) {
        suffix        = strv[i].ptr + strv[i].len - ssp->suflen;
        locp = map_get(ssp->suffixh, suffix);
        if (!locp) {
            locp = &locv[nsuffixes++];
            map_set(ssp->suffixh, suffix, locp);
        }
        ++*locp;
    }

    // Convert sublist lengths to sublist offsets in prefixv[].
    // Pre-mark list-ends.
    ssp->prefixv = calloc(nstrs + nsuffixes, sizeof(int));
    for (i = cum = 0; i < nsuffixes; ++i) {
        locv[i] = cum += locv[i] + 1;
        ssp->prefixv[--locv[i]] = NONSTR;
    }

    // Populate lists in prefixv[]
    for (i = nstrs; --i >= 0;) {
        suffix = strv[i].ptr + strv[i].len - ssp->suflen;
        locp = map_get(ssp->suffixh, suffix);
        ssp->prefixv[--*locp] = i;
    }

    // Convert map values from (list**) to (list*)
    map_start(ssp->suffixh); 
    while (map_next(ssp->suffixh, VOIDP(&suffix),
                        VOIDP(&locp))) {
        map_set(ssp->suffixh, suffix, &ssp->prefixv[*locp]);
    }

    free(locv);
    return    ssp;
}

void
ssearch_destroy(SSEARCH*ssp)
{
    if (!ssp)
        return;

    free(ssp->mapv);
    map_destroy(ssp->suffixh);
    free(ssp->prefixv);
    free(ssp);
}

void
ssearch_scan(const SSEARCH*ssp, const MEMREF text,
                SSEARCH_CB cb, void *context)
{
    const SSEARCH ss     = *ssp;
    const char        *tp      = text.ptr + ss.suflen;
    const int        skipinit = 1 << (ss.suflen - 1);
    const int        mapmask  = (1 << 2*ss.symwid) - 1;
    int                skip;

    // "window" means the portion of the target (text) being
    // being matched. When match fails, the window is moved
    // forward by (skip) bytes.
    
    for (; tp <= text.ptr + text.len; ++ssearch_skip[skip], tp += skip) {
        const uint8_t *cp = (const uint8_t*)tp;

        // symsym: the mapv[] matrix index created from
        // the symbol values for two adjacent target chars.
        int        symsym = ss.symv[cp[-1]];
        if (!SYMVALID(symsym)) {
            skip = ss.suflen;
            continue;
        }

        int        sym = ss.symv[cp[-2]];
        if (!SYMVALID(sym)) {
            skip = ss.suflen - 1;
            continue;
        }

        // i: (backward) offset from end of window (0,1,2, ...)
        // skipmask: bits indicate where search should resume
        // (how much to shift forward). skipmask is initialized
        // to a bit that skips (suflen - 1) ahead,
        // since the initial value of symsym is the only place
        // that can skip (suflen) ahead.

        int        i = 0, matchmask, skipmask = skipinit;
        cp -= 2;

        // As long as matchmask & 1 == 1, the suffix matches
        // so far. matchmask acts like the and... of
        // mapv[] bitmasks for overlapping byte pairs.

        do {
            symsym = ((symsym << ss.symwid) | sym) & mapmask;
            skipmask |= matchmask = ss.mapv[symsym] >> i;

        } while (matchmask & 1 
                 && ++i != ss.suflen - 1
                 && SYMVALID(sym = ss.symv[*--cp]));

        skip = ffs(skipmask >> 1);
        if (!(matchmask & 1 && i == ss.suflen - 1))
            continue;

        int        *set = map_get(ss.suffixh, cp);
        if (!set)
            continue;

        //TODO: replace this with a faster (binary?) search
        for (; *set != NONSTR; ++set) {
            MEMREF s = ss.strv[*set];

            if (tp - s.len >= text.ptr
                && !memcmp(s.ptr, tp - s.len,
                           s.len - ss.suflen) 
                && !cb(*set, tp - s.len, context))
                return;
        }
    }
}
