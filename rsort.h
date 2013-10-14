#ifndef _RSORT_H
#define _RSORT_H

#include <string.h>     // memcmp

typedef unsigned char byte;
typedef struct { unsigned short leng; byte data[]; } RSREC;

void rsort(RSREC **recv, int nrecs);

static inline int
KEYLENG(RSREC *rp)
{ return rp->leng; }

static inline byte const*
KEYDATA(RSREC*rp, int pos, int len, byte *buf)
{ (void)len, (void)buf; return rp->data + pos; }

static inline int
KEYDIFF(RSREC *ap, RSREC *bp, int pos, int len)
{ return memcmp(ap->data + pos, bp->data + pos, len); }

#endif//_RSORT_H
