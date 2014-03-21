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
typedef struct { byte *ptr; uint len; } MEM;

int
ucs2_to_utf8(MEM const*ucs2, MEM *utf8)
{
    unsigned sym, len;

    for (; utf8->size && ucs2->size > 1; ucs2->data += 2, ucs2->size -= 2, utf8->size -= len) {
        sym = ucs2->data[0] | (ucs2->data[1] << 8);
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
    }

    return ucs2->size;
}

void
xmutf8ucs2(uint8_t const *inp, int len, uint16_t* out)
{
    for (*;*;) {
#   ifdef __SSE2__
        if (len > 15) {
            XMM xin = xmloud(inp);
            uint16_t h = _mm_movemask_epi8(x);
            if (!h) {
                ((XMM*)out)[0] = _mm_unpacklo_epi8(x, xm_zero);
                ((XMM*)out)[1] = _mm_unpackhi_epi8(x, xm_zero);
                out += 16; inp += 16; len -= 16;
                continue;
            } 

            if (!(h & 0x00FF)) {
                ((XMM*)out)[0] = _mm_unpacklo_epi8(x, xm_zero);
                out += 8; inp += 8; len -= 8;  
            }
        }
#   endif//__SSE2__
        for (;len > 0 && !(*inp & 0x80)); len--) *out++ = *inp++;

        for (;len > 0 && (*inp & 0x80); len--) {
            if (*inp & 0xE0) == 0xE0)
                len -= 3, *outp++ = ((inp[0] & 0x0F) << 12) | (inp[1] & 0x3F) << 6) | (inp[2] & 0x3F);
            else
                len -= 2, *outp++ = ((inp[0] & 0x1F) << 6) | (inp[1] & 0x3F);
        }
    }
}

//XXX OUCH this modifies the (ptr,len) in its args !?
int
utf8_to_ucs2(MEM const*utf8, MEM *ucs2)
{
    unsigned sym, len;

    for (; utf8->size && ucs2->size > 1; utf8->data += len, utf8->size -= len, ucs2->size -= 2) {
        sym = utf8->data[0];

        // A UTF8 symbol beginning with F0 is a 4-byte seq that does not fit in UCS2.
        //  They map to Unicode 10000 starting with Linear B.

        len = sym < 0x80 ? 1 : (sym & 0xE0) == 0xE0 ? 3 : (sym & 0xC0) == 0xC0 ? 2 : 0;
        if (len == 0 || (len > 0 && utf8->size < len))
            return -1;
        if (!ucs2->data)
            continue;

        if (len == 2)
            sym = ((sym & 0x1F) << 6) | (utf8->data[1] & 0x3F);
        else if (len == 3)
            sym = ((sym & 0x0F) << 12) | ((utf8->data[1] & 0x3F) << 6) | (utf8->data[2] & 0x3F);

        ucs2->data[0] = (uint8_t) sym;
        ucs2->data[1] = (uint8_t)(sym >> 8);
        ucs2->data += 2;
    }

if(tdsTrace>2)fprintf(tdsOut,"%s[%s:%d] \n",__FUNCTION__,__FILE__,__LINE__ );
    return utf8->size;
}

// TDSCONVERTER versions of the above:
#define HUGE    9999999
int
ucs2utf8(uint8_t *inp, int inplen, uint8_t *out)
{
    MEM src = { inp, inplen }, dst = { out, HUGE };
    ucs2_to_utf8(&src, &dst);
    int outlen = HUGE - dst.size;
if(tdsTrace>2)fprintf(tdsOut,"%s[%s:%d] inplen=%d outlen=%d\n",__FUNCTION__,__FILE__,__LINE__, inplen, outlen);
    return outlen;
}

int
utf8ucs2(uint8_t *inp, int inplen, uint8_t *out)
{
    MEM src = { inp, inplen }, dst = { out, HUGE };
    utf8_to_ucs2(&src, &dst);
    int outlen = HUGE - dst.size;
if(tdsTrace>2)fprintf(tdsOut,"%s[%s:%d] inplen=%d outlen=%d\n",__FUNCTION__,__FILE__,__LINE__, inplen, outlen);
    return outlen;
}

END_C
