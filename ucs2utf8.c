// Convert UTF8 bytevec <-> UCS2LE.
//  As characters are converted, *utf8 and *ucs2 are updated (data++, size--).
//  On return, utf8 points to the remaining MEM for which there was no room in ucs2,
//      or ucs2 points to the remaining unused MEM.
// Reference: http://www.utf8-chartable.de
// Return value is:
//  >0  More left in source MEM.
//  =0  Source MEM all done.
//  <0  Error in source MME.

#include <stdint.h>

typedef uint8_t byte;
typedef uint32_t uint;
typedef struct { byte *data; uint size; } MEM;

translator: (byte *inp, uint leng, byte *out, uint size) -> 
returns: (done, incomplete, error)

uint ucs2_to_utf8(MEM const*ucs2, MEM *utf8);
void xmutf8ucs2(uint8_t const *inp, int len, uint16_t* out);
uint utf8_to_ucs2(MEM const*utf8, MEM *ucs2);
uint ucs2utf8(uint8_t *inp, int inplen, uint8_t *out);
uint utf8ucs2(uint8_t *inp, int inplen, uint8_t *out);

//--------------|---------------|---------------|-------------------
uint
ucs2_to_utf8(MEM const*ucs2, MEM *utf8)
{
    unsigned sym, len;
    byte const *u2data = ucs2->data;
    uint u2left = ucs2->size;

    for (; utf8->size && u2left > 1;) {
#       ifdef __SSE2__
        if (u2left > 15) {
            XMM xin = xmloud(u2data);
            uint16_t h = _mm_movemask_epi8(x);
            if (!h) {   // 16 ASCII chars
                ((XMM*)utf8->data)[0] = _mm_unpacklo_epi8(x, xm_zero);
                ((XMM*)utf8->data)[1] = _mm_unpackhi_epi8(x, xm_zero);
                utf8->data += 16; u2data += 16; u2left -= 16;
                continue;
            } 

            if (!(h & 0x00FF)) { // 8 ASCII chars followed by something else.
                ((XMM*)utf8->data)[0] = _mm_unpacklo_epi8(x, xm_zero);
                utf8->data += 8; u2data += 8; u2left -= 8;  
            }
        }
#       endif//__SSE2__

        sym = u2data[0] | (u2data[1] << 8);
        len = sym < 0x80 ? 1 : sym < 0x800 ? 2 : 3;
        if (len == 0)
            return -1;
        if (len == 3 && sym >= 0xD800 && sym < 0xE000)  //XXX surrogate-pair range
            return -2;
        if (!utf8->data)
            continue;

        switch (len) {
        case 1:
            utf8->data[0] = (uint8_t)sym;
            break;
        case 2:
            utf8->data[0] = 0xC0 | (uint8_t)(sym >> 6);
            utf8->data[1] = 0x80 | (uint8_t)(sym & 0x3F);
            break;
        case 3:
            utf8->data[0] = 0xE0 |  (uint8_t)(sym >> 12);
            utf8->data[1] = 0x80 | ((uint8_t)(sym >> 6) & 0x3F);
            utf8->data[2] = 0x80 | ((uint8_t)(sym) & 0x3F);
            break;
        default:
            return len;    // error
        }
        utf8->data += len;
        utf8->size -= len;
        u2data += 2;
        u2left -= 2;
    }

    return u2left;
}

void
xmutf8ucs2(uint8_t const *inp, int len, uint16_t* out)
{
    while (1) {

        for (;len > 0 && !(*inp & 0x80)); len--) *out++ = *inp++;

        for (;len > 0 && (*inp & 0x80); len--) {
            if (*inp & 0xE0) == 0xE0)
                len -= 3, *outp++ = ((inp[0] & 0x0F) << 12) | (inp[1] & 0x3F) << 6) | (inp[2] & 0x3F);
            else
                len -= 2, *outp++ = ((inp[0] & 0x1F) << 6) | (inp[1] & 0x3F);
        }
    }
}

uint
utf8_to_ucs2(MEM const*utf8, MEM *ucs2)
{
    unsigned sym, len;
    byte const *u8data = utf8->data;
    uint u8left = utf8->size;

    for (; u8left && ucs2->size > 1; u8data += len, u8size -= len, ucs2->size -= 2) {
        sym = u8data[0];

        // A UTF8 symbol beginning with F0 is a 4-byte seq that does not fit in UCS2.
        //  They map to Unicode 10000 starting with Linear B.

        len = sym < 0x80 ? 1 : (sym & 0xE0) == 0xE0 ? 3 : (sym & 0xC0) == 0xC0 ? 2 : 0;
        if (len == 0 || (len > 0 && u8left < len))
            return -1;
        if (!ucs2->data)
            continue;

        if (len == 2)
            sym = ((sym & 0x1F) << 6) | (u8data[1] & 0x3F);
        else if (len == 3)
            sym = ((sym & 0x0F) << 12) | ((u8data[1] & 0x3F) << 6) | (u8data[2] & 0x3F);

        ucs2->data[0] = (uint8_t) sym;
        ucs2->data[1] = (uint8_t)(sym >> 8);
        ucs2->data += 2;
    }

if(tdsTrace>2)fprintf(tdsOut,"%s[%s:%d] \n",__FUNCTION__,__FILE__,__LINE__ );
    return u8left;
}

// TDSCONVERTER versions of the above:
#define HUGE    9999999
uint
ucs2utf8(uint8_t *inp, int inplen, uint8_t *out)
{
    MEM src = { inp, inplen }, dst = { out, HUGE };
    ucs2_to_utf8(&src, &dst);
    int outlen = HUGE - dst.size;
if(tdsTrace>2)fprintf(tdsOut,"%s[%s:%d] inplen=%d outlen=%d\n",__FUNCTION__,__FILE__,__LINE__, inplen, outlen);
    return outlen;
}

uint
utf8ucs2(uint8_t *inp, int inplen, uint8_t *out)
{
    MEM src = { inp, inplen }, dst = { out, HUGE };
    utf8_to_ucs2(&src, &dst);
    int outlen = HUGE - dst.size;
if(tdsTrace>2)fprintf(tdsOut,"%s[%s:%d] inplen=%d outlen=%d\n",__FUNCTION__,__FILE__,__LINE__, inplen, outlen);
    return outlen;
}

END_C
