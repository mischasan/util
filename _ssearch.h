// Copyright (C) 2009-2013 Sophos LLC. All rights reserved.
//-------------------------------------------------------------------
#ifndef _SSEARCH_H_
#define _SSEARCH_H_

#include "msutil.h"
#include "ssearch.h"

// MASK defines the longest backward search for two-byte match: 8 bits -> 9 chars.
typedef uint16_t        MASK;

struct ssearch {
    const MEMREF* strv;         // Original array of input strings
    int         nstrs;          // only used by ssearch_dump
    int         suflen;         // length of suffix to match with  symv/mapv
    MASK*       mapv;           // [nsyms][1 << symwid] bitmasks
    MAP         suffixh;        // Index into lists of prefixes grouped by suffix
    int*        prefixv;        // Catenated lists of prefixes (strv[] indexes)
#   define      NONSTR (-1)     // Terminator for each list
    int         symwid;         // # of bits in symv[] values
    short       symv[256];      // Map input bytes to (-1, 0..nsyms-1)
#   define NONSYM ((short)-1)   // symv[] value for bytes that do not occur in suffixes
#   define SYMVALID(x) ((x) >= 0)
};

#endif//_SSEARCH_H_
