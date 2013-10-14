// Retrieve the key strings from a file created using Storable::store
//      on a perl hash whose values are all (1).
// That's how we store reporting groups, so that's all it will do.
// The list can be destroyed with free(ret).

#include "msutil.h"

char **
phkeys(char const *psfile, int *nvals)
{
    // File format --- "(number)" is a 4byte LSBF int; else all are bytes.
    //      "pst0" 4 7 4 "1234"  4 4 4 8 3 (nvals)
    // followed by (nvals) instances of:
    //      8 129 (len) "<len bytes>"
    // (8 129) is a varlen representation of (int)1.

    MEMBUF      data = slurp(psfile);
    if (nilbuf(data) || data.len < 20 || !(*nvals = *(int*)(data.ptr + 16)))
        return  buffree(data), NULL;

    int         nv = 0, nbytes = data.len - 20 - 6 * nv;
    char        **ret = malloc(*nvals * sizeof(char*) + nbytes + *nvals);
    char        *dst = (char*) &ret[*nvals];
    char        *src = data.ptr + 20, *fin = data.ptr + data.len;

    while (nv < *nvals && src+6 < fin && *(ushort*)src == 0x8180) {
        int     len = *(int*)(src + 2);
        if (len > fin - src - 6) break;
        memcpy(ret[nv++] = dst, src + 6, len);
        dst[len] = 0;
        dst += len + 1;
        src += len + 6;
    }

    buffree(data);
    return nv == *nvals && src == fin ? ret : free(ret), NULL;
}
