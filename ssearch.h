// Copyright (C) 2009-2013 Sophos LLC. All rights reserved.
//-------------------------------------------------------------------
// ssearch: multi-string search using SSE2 ops.
//      This uses a variation on WuManber to search a block of
//      text for many strings.
//
// ssearch_create expects (strv) argument to remain valid
//      for ssearch_scan calls.
//      
// ssearch_scan CALLBACK argument:
//	ssearch_scan calls this function for every match.
//	The callback is given:
//	- the strv[] index of the matching string
//	- the position in text.ptr[] of the match
//	- the (void*) context passed to ssearch_scan.
//	This function must return 
//	- 1: to continue the scan
//	- 0: to end the scan
//
// ssearch_dump prints a diagnostic dump of the SSEARCH object.
//	This shows:
//	- suflen: length of the shortest pattern string
//	- symwid: the power of 2 no less than number of 
//		unique characters found in the last
//		(suflen) bytes of all pattern strings, 
//	- mask array: with input char values as row/col headings.

#ifndef SSEARCH_H_
#define SSEARCH_H_

#include "msutil.h"			// MEMREF

typedef struct ssearch SSEARCH;
typedef int (*SSEARCH_CB)(int strnum, const char *textp,
			    void *context);
SSEARCH*ssearch_create(const MEMREF *strv, int nstrs);
void    ssearch_destroy(SSEARCH*);
int     ssearch_scan(const SSEARCH*, const MEMREF text,
			SSEARCH_CB, void *context);
void    ssearch_dump(SSEARCH*, FILE*);

#endif//SSEARCH_H_
