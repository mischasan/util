// Copyright (C) 2009-2013 Mischa Sandberg <mischasan@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License Version 2 as
// published by the Free Software Foundation.  You may not use, modify or
// distribute this program under any other version of the GNU General
// Public License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// IF YOU ARE UNABLE TO WORK WITH GPL2, CONTACT ME.
//-------------------------------------------------------------------

// map, kvs: variable-sized hash tables.
//  Compaction by map_del makes double-hashing impossible.
//  Consider disabling map_del, or going to closed addressing,
//  because step=1 causes a mediocre hash distribution.
//
// Note that map almost always requires SOME kind of wrapper,
//  because map explicitly takes no ownership of the memory
//  for keys and values.

#include "msutil.h"

#define    MINMASK 7

typedef struct keyval { MKEY key; void *val; } KEYVAL;

struct map {
    map_diff    *fndiff;
    map_hash    *fnhash;
    void	*context;
    int		count, mask;
    int         last;   // cursor for "map_next"
    KEYVAL      *vdata;
};

// Load factor must be less than 80%.
#define IS_FULL(mp)    ((mp)->count * 5 / 4 >= (mp)->mask)
#define FNHASH(mp,key) ((mp)->fnhash(key, (mp)->context) & (mp)->mask)

static int _find(MAP const mp, MKEY key);

MAP
map_create(map_diff *diff, map_hash *hash, void *context)
{
    MAP    mp = calloc(1, sizeof(*mp));

    *mp = (struct map) {
		diff, hash, context, 0, MINMASK, -1,
		calloc(MINMASK + 1, sizeof(KEYVAL))
	    };

    return mp;
}

int  map_count(MAP const mp) { return mp->count; }

// Deletion from an open-addr hash table:
//  - find alpha: the last occupied position, starting at
//      the new hole, working backward.
//  - search for an entry in pos+1... that belongs in the
//      range (alpha..hole). Move it to hole, then advance hole to
//      the (now empty) position.
//      When you hit an empty entry, you are done.

void *
map_del(MAP mp, MKEY key)
{
    int         pos = _find(mp, key), alpha = pos, hole = pos;
    void        *oldval = mp->vdata[pos].val;

    if (!mp->vdata[pos].key)
        return NULL;

    --mp->count;
    if (mp->last > pos)
	mp->last = pos;    //TODO

    do alpha = (alpha - 1) & mp->mask;
    while (mp->vdata[alpha].key);

    while (mp->vdata[pos = (pos + 1) & mp->mask].key) {
        int     want = FNHASH(mp, mp->vdata[pos].key);
        if ((unsigned)(want - alpha) <= (unsigned)(hole - alpha)) {
            mp->vdata[hole] = mp->vdata[pos];
            hole = pos;
        }
    }
    mp->vdata[hole] = (KEYVAL){NULL, NULL};

    return oldval;
}

void map_destroy(MAP mp) { if (mp) free(mp->vdata), free(mp);}

void *
map_get(MAP const mp, MKEY key)
{
    return mp->vdata[_find(mp, key)].val;
}

MKEY
map_key(MAP const mp, MKEY key)
{
    return mp->vdata[_find(mp, key)].key;
}

int
map_next(MAP mp, void const**pkey, void **pval)
{
    do if (++mp->last > mp->mask) return 0;
    while (!mp->vdata[mp->last].key);

    *pkey = mp->vdata[mp->last].key;
    *pval = mp->vdata[mp->last].val;
    return 1;
}

void
map_resize(MAP mp, int limit)
{
    int		i, j, dups = 0, oldmask = mp->mask;
    KEYVAL      *vdata;
    const void	*kp;

    if (--limit < MINMASK) limit = MINMASK;
    if (limit < mp->count * 5/4) limit = mp->count * 5/4;
    while (mp->mask > limit) mp->mask >>= 1;
    while (mp->mask < limit) mp->mask = (mp->mask << 1) | 1;
    vdata = calloc(mp->mask + 1, sizeof(KEYVAL));

        // PASS 1: insert where there are no collisions,
    for (i = 0; i <= oldmask; ++i) {

        if ((kp = mp->vdata[i].key)) {
            if (vdata[j = FNHASH(mp, kp)].key) {
                mp->vdata[dups++] = mp->vdata[i];
            } else {
                vdata[j] = mp->vdata[i];
            }
        }
    }

        // PASS 2: insert the rest
    for (i = 0; i < dups; ++i) {
        kp = mp->vdata[i].key;
        j = FNHASH(mp, kp);
        do j = (j + 1) & mp->mask; while (vdata[j].key);
        vdata[j] = mp->vdata[i];
    }

    free(mp->vdata);
    mp->vdata = vdata;
}

void *
map_set(MAP mp, MKEY key, void *val)
{
    int         pos     = _find(mp, key);
    void        *oldval = mp->vdata[pos].val;

    if (!mp->vdata[pos].key) {    // new entry
        if (IS_FULL(mp)) {
            map_resize(mp, (mp->mask + 1) << 1);
            pos = FNHASH(mp, key);
            while (mp->vdata[pos].key)
                pos = (pos + 1) & mp->mask;
        }

        mp->vdata[pos].key = key;
        ++mp->count;
    }

    mp->vdata[pos].val = val;
    return oldval;
}

void map_start(MAP mp)       { mp->last = -1; }

//--------------|---------------------------------------------
static int
_find(MAP const mp, MKEY key)
{
    int     pos = FNHASH(mp, key);

    while (mp->vdata[pos].key &&
           mp->fndiff(mp->vdata[pos].key, key, mp->context))
        pos = (pos + 1) & mp->mask;

    return pos;
}
//--------------|---------------------------------------------
void
kvs_del(KVS sh, const char *key)
{
    char    *ownkey = (char*)(intptr_t)map_key(sh, key);
    if (ownkey) {
        free(map_del(/*(MAP)*/sh, ownkey));
        free(ownkey);
    }
}

void
kvs_destroy(KVS sh)
{
    char        *key, *val;

    kvs_start(sh);
    while (kvs_next(sh, &key, &val))
	free(key), free(val);

    map_destroy(sh);
}

KVS
kvs_load(char const *file, char sep)
{
    MEMBUF  data = chomp(slurp(file));
    if (nilbuf(data))
        return NULL;

    int     nstrs, i;
    MEMREF  *strv = refsplit(data.ptr, '\n', &nstrs);
    KVS     kvs = kvs_create();
    kvs_resize(kvs, nstrs);

    for (i = 0; i < nstrs; ++i) {
        char *cp = (char*)(intptr_t)strchr(strv[i].ptr, sep);
        if (cp)  {
            *cp++ = 0;
            kvs_set(kvs, strv[i].ptr, cp);
        }
    }

    free(strv);
    buffree(data);
    return  kvs;
}

void
kvs_set(KVS sh, const char *key, const char *val)
{
    int         pos = _find(sh, key);

    if (sh->vdata[pos].key) {
        free(sh->vdata[pos].val);
        sh->vdata[pos].val = strdup(val);
    } else if (!IS_FULL(sh)) {
        sh->vdata[pos] = (KEYVAL){strdup(key), strdup(val)};
        ++sh->count;
    } else {    // redundantly calls FNHASH:
        map_set(sh, strdup(key), strdup(val));
    }
}
//EOF
