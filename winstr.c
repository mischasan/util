#ifndef STR_X
#   include <stdint.h>
#   include <string.h>
#   ifdef __linux__
#       include <byteswap.h>
#   endif
char *winstr(char *tgt, char *pat);
#endif

// memdiff: return 1 if src and dst differ, 0 otherwise
//  Note that this may OVERREAD memory.

static int
memdiff(char *dst, char *src, int len)
{
    while (len >= (int)sizeof(long)) {
        if (*(long*)src != *(long*)dst)
            return 1;
        len -= sizeof(long), src += sizeof(long), dst += sizeof(long);
    }

#   if __SIZEOF_SIZE_T__ == 8
    if (len >= (int)sizeof(uint32_t)) {
        if (*(uint32_t*)src != *(uint32_t*)dst)
            return 1;
        len -= sizeof(uint32_t), src += sizeof(uint32_t), dst += sizeof(uint32_t);
    }
#   endif

    if (len >= (int)sizeof(short)) {
        if (*(short*)src != *(short*)dst)
            return 1;
        len -= sizeof(short), src += sizeof(short), dst += sizeof(short);
    }

    return len && *src != *dst;
}

#define SHLB(X,B)   ((X) << ((B) * 8))

// Partial-word winstrX. Note masked-off/ignored high bytes.
#define WINSTR_PART(BYTES,UTYPE,X01,X80) \
static char *winstr##BYTES(char *tgt, char pat[BYTES]) {\
    UTYPE head = *(UTYPE*)pat, wind = *(UTYPE*)tgt;\
    if ((wind - (UTYPE)X01) & ~wind & (UTYPE)X80 & ~SHLB((UTYPE)-1, BYTES)) return NULL;\
    while (1) {\
        if (!((head ^ wind) & ~SHLB((UTYPE)-1, BYTES))) return tgt;\
        if (!(wind & SHLB((UTYPE)0xFF, BYTES))) return NULL;\
        wind = *(UTYPE*)++tgt;\
    }\
}

// Full-word winstrX. Note changed order of tests in loop body.
#define WINSTR_FULL(BYTES,UTYPE,X01,X80) \
static char *winstr##BYTES(char *tgt, char pat[BYTES]) {\
    UTYPE head = *(UTYPE*)pat, wind = *(UTYPE*)tgt;\
    if ((wind - X01) & ~wind & (UTYPE)X80) return NULL;\
    while (1) {\
        if (head == wind) return tgt;\
        wind = *(UTYPE*)++tgt;\
        if (!(wind & SHLB((UTYPE)0xFF, BYTES - 1))) return NULL;\
    }\
}

// winstr2 is academic, since sse2str will always be faster.
static char *
winstr2(char *tgt, char pat[2])
{
    uint16_t head = *(uint16_t*)pat, wind = *(uint16_t*)tgt;
    if (!(uint8_t)wind) return NULL;
    while (1) {
        if (!(wind & 0xFF00)) return NULL;
        if (wind == head) return tgt;
        wind = *(uint16_t*)++tgt;
    }
}

// Winstr3 can't use the same trick as railgun7,
// because it also needs to test for a null byte.
WINSTR_PART(3, uint32_t, 0x01010101, 0x80808080)
WINSTR_FULL(4, uint32_t, 0x01010101, 0x80808080)
#if __SIZEOF_SIZE_T__ == 8
#   define x01 0x0101010101010101L
#   define x80 0x8080808080808080L
WINSTR_PART(5, uint64_t, x01, x80)
WINSTR_PART(6, uint64_t, x01, x80)
WINSTR_PART(7, uint64_t, x01, x80)
WINSTR_FULL(8, uint64_t, x01, x80)
#else // == 4
#   define x01 0x01010101
#   define x80 0x80808080
#endif

typedef __SIZE_TYPE__ UINT;
static char *
winstrm(char *tgt, char *pat, int len)
{
    UINT head = *(UINT*)pat, wind = *(UINT*)tgt;
    if ((wind - x01) & ~wind & x80)
        return NULL;

    while (1) {
        if (head == wind && !memdiff(tgt+sizeof(UINT), pat+sizeof(UINT), len-sizeof(UINT)))
            return tgt;
        wind = *(UINT*)++tgt;
        if (!(wind & SHLB(0xFFL, sizeof(UINT) - 1)))
            return NULL;
    }
}

char *
winstr(char *tgt, char *pat)
{
    unsigned     len = strlen(pat);
    switch (len) {
    case  0: return tgt;
    case  1: return strchr( tgt,*pat);
    case  2: return winstr2(tgt, pat);
    case  3: return winstr3(tgt, pat);
    case  4: return winstr4(tgt, pat);
#   if __SIZEOF_SIZE_T__ == 8
    case  5: return winstr5(tgt, pat);
    case  6: return winstr6(tgt, pat);
    case  7: return winstr7(tgt, pat);
    case  8: return winstr8(tgt, pat);
#   endif
    default: return winstrm(tgt, pat, len);
    }
}
