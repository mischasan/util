// From: http://www-igm.univ-mlv.fr/~lecroq/string/node26.html

static inline int
MAX (int a, int b)
{
    return a > b ? a : b;
}

// Computing of the maximal suffix for <=.
static int
maxSuf (char *x, int m, int *p)
{
    int ms, j, k;
    char a, b;

    ms = -1;
    j = 0;
    k = *p = 1;
    while (j + k < m) {
        a = x[j + k];
        b = x[ms + k];
        if (a < b) {
            j += k;
            k = 1;
            *p = j - ms;
        }
        else if (a == b)
            if (k != *p)
                ++k;
            else {
                j += *p;
                k = 1;
            }
        else {                  /* a > b */
            ms = j;
            j = ms + 1;
            k = *p = 1;
        }
    }
    return (ms);
}

// Computing of the maximal suffix for >=.
static int
maxSufTilde (char *x, int m, int *p)
{
    int ms, j, k;
    char a, b;

    ms = -1;
    j = 0;
    k = *p = 1;
    while (j + k < m) {
        a = x[j + k];
        b = x[ms + k];
        if (a > b) {
            j += k;
            k = 1;
            *p = j - ms;
        }
        else if (a == b)
            if (k != *p)
                ++k;
            else {
                j += *p;
                k = 1;
            }
        else {                  /* a < b */
            ms = j;
            j = ms + 1;
            k = *p = 1;
        }
    }
    return (ms);
}

/* Two Way string matching algorithm. */
char *
twoway(char *tgt, int tgtlen, char *pat, int patlen)
{
    int i, j, ell, memory, p, per, q;

    /* Preprocessing */
    i = maxSuf (pat, patlen, &p);
    j = maxSufTilde(pat, patlen, &q);
    if (i > j) {
        ell = i;
        per = p;
    }
    else {
        ell = j;
        per = q;
    }

    /* Searching */
    if (!memcmp (pat, pat + per, ell + 1)) {
        j = 0;
        memory = -1;
        while (j <= tgtlen - patlen) {
            i = MAX (ell, memory) + 1;
            while (i < patlen && pat[i] == tgt[i + j])
                ++i;
            if (i >= patlen) {
                i = ell;
                while (i > memory && pat[i] == tgt[i + j])
                    --i;
                if (i <= memory)
                    return tgt + j;
                j += per;
                memory = patlen - per - 1;
            }
            else {
                j += (i - ell);
                memory = -1;
            }
        }
    } else {
        per = MAX (ell + 1, patlen - ell - 1) + 1;
        j = 0;
        while (j <= tgtlen - patlen) {
            i = ell + 1;
            while (i < patlen && pat[i] == tgt[i + j])
                ++i;
            if (i >= patlen) {
                i = ell;
                while (i >= 0 && pat[i] == tgt[i + j])
                    --i;
                if (i < 0)
                    return tgt + j;
                j += per;
            }
            else
                j += (i - ell);
        }
    }
    return NULL;
}
