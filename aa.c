// This implements the ANALYZE step for rsort, taking a bit more time
//  but computing the densest linear function possible, based on
//  adjacent-byte dependencies for each column position --- i.e.
//  given the sets of unique byte values C[p] at position (p),
//  mapped to a range of contiguous integers (codes), use the
//  (lo,hi) bounds of such codes found to follow each byte (x) at position (p-1),
//  to compute a map M[p,b] that rsort uses to compute a 
//  radix index for a given range of bytes [p..q] in each key.
//  See the google docs explanation of how this can be computed
//  cheaply.
// For most data, this M is not measurably better than rectangular
//  co-ordinates (single-byte values per column position).
// For some edge cases, 

#include <ctype.h>
#include <stdint.h>
#include "msutil.h"     // tick

#define	MAXREC	      999
#define MAXWID	      100
#define	MAXPARTS    65536

typedef struct { uint16_t lo; int16_t hi; } LOHI;

typedef struct {
    int		alpha, omega;	// bounds of keyscan
    int		final;		// min(omega, max(keyleng)-1))
    uint8_t	*bend;		// bend[i]==1: some key has length (i)
    LOHI	(*succ)[256];	// [omega-alpha+1][256]
    uint8_t	(*has)[256];	// has[p][c]==1: (c) occurs at some key[p]
    int16_t	*csets;		// [(omega-alpha+1) * 257]
    int16_t	*chead;		// start of last cset in csets
    int16_t	*cstop;		// end of csets[]
} SCAN;

static void scan_init(SCAN *sp, int alpha, int omega);
static void scan_line(SCAN *sp, uint8_t const *buf);   // >>(has,final)
static int  scan_fini(SCAN *sp); // returns (q)
static void scan_cset(SCAN *sp, int q); // has[q] >> csets [chead]..(-1)
static int  scan_comp(SCAN *sp, int q, int map[][256]);    // returns max(Z)

static void scan_dump(SCAN const *, int pos);
#define	SWAP(x,y) do { typeof(x) __z = x; x = y; y = __z; } while (0)
#define	IMIN(x,y) ((x) < (y) ? (x) : (y))
#define	IMAX(x,y) ((x) > (y) ? (x) : (y))

static int debug;
#define DIAG(...) if (debug) fprintf(stderr, __VA_ARGS__)
#define DIAGX(x)  if (debug) x

int
main(int argc, char **argv)
{
    if (argc < 2 || argc > 4)
	return fputs("Usage: aa <file> [alpha [omega]]\n", stderr);

    debug = !!getenv("DEBUG");
    FILE    *fp = strcmp(argv[1], "-") ? fopen(argv[1], "r") : stdin;
    if (!fp) die(": unable to open %s:", argv[1]);
    int		alpha = argc > 2 ? atoi(argv[2]) : 0;
    int		omega = argc > 3 ? atoi(argv[3]) : MAXWID;

    SCAN	scan;
    scan_init(&scan, alpha, omega);
    uint8_t	buf[MAXREC];

    while (fgets((char*)buf, sizeof(buf), fp))
	scan_line(&scan, buf);
    fclose(fp);

    int		wider, width;
    int		_m[2][MAXWID][256], (*m0)[256] = _m[0], (*m1)[256] = _m[1];

    // scan_fini returns min(q) where product of cset widths[0..q] > MAXPARTS.
    double	t0 = tick();
    int		q = scan_fini(&scan);

    // Compute m[alpha..q]
    if ((wider = scan_comp(&scan, q, m0)) >= MAXPARTS) {

	while (*scan.chead++ >= 0); // delete last cset
	width = scan_comp(&scan, --q, m1);

    } else {
	// Try for higher values of q until one fails
	do {
	    width = wider;
	    SWAP(m0, m1);
	    if (++q > scan.final)
		break;
	    scan_cset(&scan, q);
	    wider = scan_comp(&scan, q, m0);
	} while (wider < MAXPARTS);

	--q;
    }

    printf("elapsed: %.4f %d...%d q:%d wid:%d\ncsets:\n",
            tick() - t0, scan.alpha, scan.omega, q, width);
    scan_dump(&scan, q);

    return  0;
}

static void
scan_init(SCAN *sp, int alpha, int omega)
{
    int	    wid = omega - alpha + 1;
    int	    size = wid * 256 * sizeof(LOHI);

    sp->alpha = alpha;
    sp->omega = omega;
    sp->final = -1;
    sp->bend  = calloc(wid, sizeof(*sp->bend));
    sp->has   = calloc(wid, sizeof(*sp->has));
    sp->succ  = malloc(size);
    sp->csets = malloc(wid * 257 * sizeof(int16_t));
    sp->chead = sp->cstop = &sp->csets[wid * 257];
    memset(sp->succ, -1, size); // set lo=65535, hi=-1
}

static void
scan_line(SCAN *sp, uint8_t const *buf)
{
    uint8_t const	*bp = buf;
    int16_t	curr, prev;
    int		alpha = sp->alpha;
	// i: offset of last key char in buf
    int		i = strlen((char const*)buf) - 2;

    if (i < alpha)    // too int16_t ...
	return;

    if (i > sp->omega) {
	i = sp->omega;
    } else {
	sp->bend[i - alpha + 1] = 1;
    }

    if (sp->final < i)
	sp->final = i;

    prev = 0;
    curr = bp[i];
    while (1) {
	sp->has[i - alpha][curr] = 1;
	LOHI	*lp = &sp->succ[i - alpha][curr];
	if (lp->lo > prev) lp->lo = prev;
	if (lp->hi < prev) lp->hi = prev;
	if (--i < alpha)
	    break;
	prev = curr + 1;
	curr = bp[i];
    }
}

// Compute initial csets.
// Returns the position at which the product of cset sizes exceeds MAXPARTS.
//  Since this mapping will always be at least as good as rectangular co-ordinates,
//  that position is the first one to try that might do better.

static int
scan_fini(SCAN *sp)
{
    int	    q, prod = 1;

    if (sp->final > sp->omega)
	sp->final = sp->omega;

    for (q = sp->alpha; q <= sp->final; ++q)
    {
	int16_t	*ep = sp->chead;
	scan_cset(sp, q);
	prod *= sp->bend[q] + ep - sp->chead - 1;
	//if (prod > MAXPARTS || (prod == 1 && q == sp->alpha && ++sp->alpha > sp->final))
	if (prod > MAXPARTS || (prod == 1 && ++sp->alpha > sp->final))
	    break;
    }
    DIAG("scan_fini: %d\n", q);
    return q;
}

// Generate the (-1)-terminated ordered set of byte values at position (q).
// These lists are stacked backwards in sp->csets[], deepest first.
// This format is fast to traverse and easy to build incrmentally.

static void
scan_cset(SCAN *sp, int q)
{
    DIAG("scan_cset(%d)\n", q);
    int	    i = 256;
    *--sp->chead = -1;
    while (--i > 0) if (sp->has[q][i]) *--sp->chead = i;
}

// Compute map[pos][byte] for the radix function.
// Returns an upper bound on that function's range.

static int
scan_comp(SCAN *sp, int q, int map[][256])
{
    DIAG("scan_comp(%d)\n", q);
    int	    e, x[4][256];
    int	    *y = x[0], *yn = x[1], *z = x[2], *zn = x[3];
    int16_t   c, *cp = sp->chead;
    *y = *z = *yn = *zn = 0;
    e = sp->bend[q] - 1;

    while ((c = *cp++) >= 0)
	map[q][c] = y[c+1] = z[c+1] = ++e;

    while (e < MAXPARTS && --q >= sp->alpha) {
	LOHI *lp = sp->succ[q];

	e = sp->bend[q] - 1;
	while ((c = *cp++) >= 0) {
	    yn[c+1] = e + 1;
	    map[q][c] = e + 1 - y[lp[c].lo];
	    e = zn[c+1] =  z[lp[c].hi] + map[q][c];
	}

	SWAP(y, yn);
	SWAP(z, zn);
    }

    return  e;
}

static void
scan_dump(SCAN const *sp, int pos)
{
    int         i;
    int16_t const *cp = sp->chead;
    DIAG("scan: alpha:%d omega:%d final:%d\n", sp->alpha, sp->omega, sp->final);

    DIAG("\tkey lengths:");
    for (i = 0; i < 256; ++i) DIAG(!sp->bend[i] ? "" : " %d", i);
    DIAG("\n");

    for (; cp != sp->cstop; ++cp) {
        if (*cp == -1) {
            --pos;
            DIAG("\n");
        } else { 
            DIAG(isprint(*cp) ? " %c" : " %02X", *cp);
            DIAG("(");
            int lo = sp->succ[pos - sp->alpha][*cp].lo;
            int hi = sp->succ[pos - sp->alpha][*cp].hi;
            DIAG(lo == -1 ? "#" : lo == 0 ? "-" : isprint(lo-1) ? "%c" : "%02X", lo - 1);
            if (hi != lo) {
                DIAG(",");
                DIAG(hi == -1 ? "#" : hi == 0 ? "-" : isprint(hi-1) ? "%c" : "%02X", hi - 1);
            }
            DIAG(")");
        }
    }
}
//EOF
