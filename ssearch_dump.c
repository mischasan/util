#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "_ssearch.h"

void
ssearch_dump(SSEARCH*sse, FILE *fp)
{
    int         i, j, maxsym;
    char        symchar[256];
    if (!sse)
	return;

    fprintf(fp, "sse {nstrs=%d suflen=%d symwid=%d}\n",
	sse->nstrs, sse->suflen, sse->symwid);
#if 0
    for (i = 0; i < sse->nstrs; ++i)
        fprintf(fp, "[%d] %s\n", i+1, sse->strv[i].ptr);
#endif
    for (i = maxsym = 0; i < 256; ++i) {
        if (sse->symv[i] >= 0)
            symchar[sse->symv[i]] = i;
        if (maxsym < sse->symv[i])
            maxsym = sse->symv[i];
    }

#   define FMASK "%02X"
    fputs(" __", fp);
    for (j = 0; j <= maxsym; ++j)
        fprintf(fp, isgraph(symchar[j]) ? " _%c" : " "FMASK, symchar[j]);
    putc('\n', fp);

    if (sse->mapv) {
        for (i = 0; i <= maxsym; ++i) {
            fprintf(fp, isgraph(symchar[i]) ? " %c_" : FMASK":", symchar[i]);
            for (j = 0; j <= maxsym; ++j)
                fprintf(fp, " "FMASK, sse->mapv[(j << sse->symwid) + i]);
            putc('\n', fp);
        }
    }
#if 0
    if (sse->suffixh) {
        char        *cp;
        int         *pos;
        for (map_start(sse->suffixh);
		map_next(sse->suffixh,
			     (void const**)(uintptr_t)&cp,
			     (void**)(uintptr_t)&pos);) {

            fprintf(fp, "%.*s %d:\t", sse->suflen, cp,
					(int)(pos - sse->prefixv));
            for (; *pos != NONSTR; ++pos)
                fprintf(stderr, " [%d]%.*s", *pos,
					(int)sse->strv[*pos].len,
					sse->strv[*pos].ptr);
            putc('\n', stderr);
        }
    }
#endif
}
