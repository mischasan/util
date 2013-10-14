#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sheap.h"

struct SHEAP_s { int size, count, hole; SCORE **vec; };

SHEAP*
sheap_create(int size)
{
    SHEAP sh = { size, 0, 0, calloc(size, sizeof(SCORE*)) };
    return memcpy(malloc(sizeof sh), &sh, sizeof sh);
}

void
sheap_destroy(SHEAP *shp)
{ if (shp) free(shp->vec), free(shp); }

int
sheap_size(SHEAP const*shp)
{ return shp->size; }

int
sheap_count(SHEAP const*shp)
{ return shp->count - (shp->hole < shp->count); }

SCORE
sheap_head(SHEAP const*shp)
{ assert(shp->count > 0); return *shp->vec[0]; }

SCORE*
sheap_pull(SHEAP *shp)
{
    if (shp->hole < shp->count)
        shp->vec[shp->hole] = shp->vec[--shp->count];
    assert(shp->count > 0);
    SCORE *head = shp->vec[shp->hole = 0];
    int i;
    for (; (i = 2*shp->hole + 1) < shp->count; shp->hole = i) {
        i += i + 1 < shp->count && *shp->vec[i] >= *shp->vec[i+1];
        shp->vec[shp->hole] = shp->vec[i];
    }

    shp->count -= shp->count == shp->hole + 1;
    return head;
}

void
sheap_push(SHEAP *shp, SCORE *scp)
{
    assert(sheap_count(shp) < shp->size);
    shp->count += shp->count == shp->hole;
    int i;
    while ((i = shp->hole - 1) >= 0 && *shp->vec[i /= 2] > *scp) {
        shp->vec[shp->hole] = shp->vec[i];
        shp->hole = i;
    }

    shp->vec[shp->hole] = scp;
    shp->hole = shp->count;
}

void
sheap_dump(SHEAP const*shp, FILE *fp)
{
    SCORE *base = shp->count ? (SCORE*)(void*)-1 : NULL;
    int i;
    for (i = 0; i < shp->count; ++i)
        if (base > shp->vec[i]) base = shp->vec[i];
    fprintf(fp, "size=%d count=%d hole=%d\n",
            shp->size, shp->count, shp->hole);
    for (i = 0; i < shp->count; ++i)
        if (i != shp->hole)
            fprintf(fp, "%d: @%04lx %d\n", i, shp->vec[i] - base, *shp->vec[i]);
}
