/* lohuff: calculate lowbit Huffman codes for a given frequency vector.
 *
 * INPUT:
 *    sym[]    Symbol frequencies. No output code created where frequency is 0.
 * OUTPUT
 *    sym[]    Lowbit huffman codes, in the form (code << 8) + leng.
 *        Codes are bit-reversed: stream parsing starts at bit 0.
 */

#include <stdlib.h>    // qsort_r
#include <string.h>    // memcpy
#include "msutil.h"

static int pintcmp(int *work, int *a, int *b) { return -(work[*a] - work[*b]); }

void
lohuff(int len, unsigned freq[len], unsigned code[len])
{
    int        i, nsyms, nworks = len;
    unsigned    work[2*len], rank[len];

    memcpy(work, freq, len * sizeof(unsigned));
    for (i = nsyms = 0; i < len; ++i) if (work[i]) rank[nsyms++] = i;

    qsort_r(rank, nsyms, sizeof(int), (qsort_r_cmp)pintcmp, (void*)work);

    // Build tree by pairing lowest-frequency symbols
    while (nsyms > 1) {
        --nsyms;
        work[nworks] = work[rank[nsyms]] + work[rank[nsyms-1]];
        work[rank[nsyms-1]] = (nworks << 1);
        work[rank[nsyms]] = (nworks << 1) + 1;
        for (i = nsyms; --i > 0 && work[rank[i-1]] < work[nworks];)
            rank[i] = rank[i - 1];
        rank[i] = nworks++;
    }
    // assert(rank[0] == nworks - 1)
    // assert(rank[nworks-1] == sum(work[0..len-1]))
    work[nworks - 1] = 0;
    // Convert tree nodes ((parent<<1) | side) into lohuff ((code<<8) | leng) values
    for (i = nworks - 2; i >= 0; --i) {
        if (work[i]) {
            unsigned parent  = work[work[i] >> 1];
            work[i] = parent + 1 + ((work[i] & 1) << (8+(unsigned char)parent));
        }
    }

    // Force work[len - 1] to be zero (ssearch requirement)
    unsigned last = work[len - 1] & ~255;
    if (last)
        for (i = 0; i < len; ++i)
            work[i] ^= last & ((1 << (8+(unsigned char)work[i])) - 1);

    memcpy(code, work, len * sizeof(unsigned));
}
