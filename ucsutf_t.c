#include "msutil.h"
#include "tap.h"

typedef struct { byte *ptr; uint len; } MEM;

int ucs2_to_utf8(MEM const*ucs2, MEM *utf8);
int utf8_to_ucs2(MEM const*utf8, MEM *ucs2);

void Test(char const *name, uint8_t*utf, int utflen, uint8_t*ucs, int ucslen);
void Dump(uint8_t*data, int leng);
void MEMcat(MEM *dst, MEM *src);

uint8_t utf_all[99], ucs_all[99];
MEM m_utf = { utf_all, 0 }, m_ucs = { ucs_all, 0 };

int main(void)
{
    int ntests = 9;
    plan_tests(ntests * 6);

    uint8_t utf_u[] = { 0x75 }, ucs_u[] = { 0x75, 0x00 };
    Test("'u'", utf_u, sizeof utf_u, ucs_u, sizeof ucs_u);

    uint8_t utf_cent[] = { 0xC2, 0xA2 }, ucs_cent[] = { 0xA2, 0x00 };
    Test("cent", utf_cent, sizeof utf_cent, ucs_cent, sizeof ucs_cent);

    uint8_t utf_thorn[] = { 0xC3, 0xBE }, ucs_thorn[] = { 0xFE, 0x00 };
    Test("thorn", utf_thorn, sizeof utf_thorn, ucs_thorn, sizeof ucs_thorn);

    uint8_t utf_tick[] = { 0xDF, 0xB4 }, ucs_tick[] = { 0xF4, 0x07 };
    Test("tick", utf_tick, sizeof utf_tick, ucs_tick, sizeof ucs_tick);

    uint8_t utf_alaf[] = { 0xe0, 0xA0, 0x80 }, ucs_alaf[] = { 0x00, 0x08 };
    Test("alaf", utf_alaf, sizeof utf_alaf, ucs_alaf, sizeof ucs_alaf);

    uint8_t utf_nul[] = { 0x00 }, ucs_nul[] = { 0x00, 0x00 };
    Test("nul", utf_nul, sizeof utf_nul, ucs_nul, sizeof ucs_nul);

    uint8_t utf_tm[] = { 0xE2, 0x84, 0xA2 }, ucs_tm[] = { 0x22, 0x21 };
    Test("tm", utf_tm, sizeof utf_tm, ucs_tm, sizeof ucs_tm);

    uint8_t utf_ring[] = { 0xEF, 0xBF, 0xAE }, ucs_ring[] = { 0xEE, 0xFF };
    Test("ring", utf_ring, sizeof utf_ring, ucs_ring, sizeof ucs_ring);

    Test("all", utf_all, m_utf.size, ucs_all, m_ucs.size);

    fprintf(stderr, "TODO: failure cases\n");

    return exit_status();
}

void
Test(char const *name, uint8_t*utf, int utflen, uint8_t*ucs, int ucslen)
{
    printf("#----- Test %s\n", name);
    printf("# utf:"); Dump(utf, utflen);
    printf("# ucs:"); Dump(ucs, ucslen);

    uint8_t out[99];

    MEM dst = { out, sizeof out };
    MEM src = { utf, utflen };
    if (utf != utf_all) MEMcat(&m_utf, &src);

    int rc = utf8_to_ucs2(&src, &dst);
    is(rc, 0, "ucs done");
    if (is(dst.data - out, ucslen, "ucs2 length") && !ok(!memcmp(out, ucs, ucslen), "ucs2 value"))
        printf("# actual: "), Dump(out, ucslen);

    dst = (MEM){ out, sizeof out };
    src = (MEM){ ucs, ucslen };
    if (ucs != ucs_all) MEMcat(&m_ucs, &src);

    rc = ucs2_to_utf8(&src, &dst);
    is(rc, 0, "utf done");
    if (is(dst.data - out, utflen, "utf8 length") && !ok(!memcmp(out, utf, utflen), "utf8 value"))
        printf("# actual: "), Dump(out, ucslen);
}

void
Dump(uint8_t *data, int leng)
{
    while (leng-- > 0) printf(" %02X", *data++);
    putchar('\n');
}

void MEMcat(MEM *dst, MEM *src)
{
    memcpy(dst->data + dst->size, src->data, src->size);
    dst->size += src->size;
}
