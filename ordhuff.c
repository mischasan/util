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

/* Copyright (c) 2005 Mischa Sandberg. All rights reserved.
 *
 * Generate a set of ordered Huffman codes for a given set
 *  of character frequencies (counts).
 *
 * INPUT:   A vector of nonzero character counts;
 *          the sum of counts must not exceed 2^28.
 * OUTPUT:  A vector of codes (right-justified bitstrings),
 *          and a vector of their lengths (in bits).
 *          vBits and vFreq can be the same array.
 *
 * This O(n) algorithm requires no tree. It's a curiosity that
 * produces NEAR-optimal ordered huffman codes, rather than
 * guaranteed optimal (per Garsia-Wachs). On the other hand, 
 * even an "optimal" ordered huffman code is a sub-optimal 
 * Huffman code.
 *
 * The method of computing the tree with bitmask operations
 * is probably worth remembering.
 * NON-LINEAR SCALING
 *  myoh is relatively worse for some flat hyperbolic distributions.
 *  Here, a wider spread for larger vFreq[] values would be better.
 *  You cannot just multiply the higher range by a larger constant;
 *  it just creates a "hole" at the cusp.
 */

#include "msutil.h"

//TODO: vectorize this. Requires __aligned__(16) args

void
ordhuff( 
    int             ncodes,        // length of (all) vectors.
    unsigned const  vFreq[], 
    unsigned        vCode[],
    unsigned        vBits[])
{
    int             i;
    unsigned        scale, sum, z;
    unsigned        prevcode, prevbits, currbits, prevmask, comb;

    for (i = ncodes, sum = vFreq[0]; --i > 0;) sum += vFreq[i];

    /* Calculate initial code widths, as left-justified bitmasks;
     *   e.g 0xF8000000 for a code with length "5".
     * NOTE: 0x7FFFFFFF as a quotient can cause overflows,
     *   i.e. vCode[i] < vCode[i-1].
     */
    scale   = 0x3FFFFFFF / sum;
        //XXX: use a nonlinear scaling
    for (i = ncodes; --i >= 0;)
	vBits[i] = -1 << fls(vFreq[i] * scale);

    /* Calculate initial code values, also left-justified.
     * gcc has a bit of trouble optimizing induction values here.
     */
    prevcode = vCode[0] = 0;
    prevbits = vBits[0];

    for (i = 1; i < ncodes; ++i) {
        currbits = vBits[i];
        vCode[i] = (prevcode - (currbits & prevbits)) & currbits;
        prevcode = vCode[i];
        prevbits = currbits;
    }

    /* Comb out unnecessary bits, equivalent to tree nodes with
     * one child. A bit is combed out, if it is a zero bit in 
     * the current code, and it was either an unnecessary bit 
     * in the previous code, or not part of the previous code 
     * (below the masked bits).
     */
    prevmask = comb = 0;
    for (i = ncodes; --i >= 0;) {
        int             x;  // signed for easy high-bit test
        unsigned        currmask = vBits[i], currcode = vCode[i];

        comb = (~prevmask | comb) & ~currcode & currmask;
        prevmask = currmask;

        // Squeeze out leading useless bits in a tiny loop:
        for (x = comb; x < 0; x <<= 1, currmask <<= 1)
            currcode <<= 1;

        // Squeeze out remaining combed bits in a larger loop:
        for (; x; x &= x - 1, currmask <<= 1)
            currcode += ((x - 1) ^ x) & currcode;

        vBits[i] = currmask, vCode[i] = currcode;
    }

    /* Convert left-justified codes and corresponding bitmasks
     *  to right-justified codes and corresponding lengths.
     * OTOH: probably more useful to leave codes left-justified
     *	and use an int64_t shifter (e.g SHLD %cl, %edx, %eax)
     */
    for (i = ncodes; --i >= 0;) {
        vCode[i] >>= z = bsrl(vBits[i]);
        vBits[i] = 32 - z;
    }
}
