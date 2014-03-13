#include <stdint.h>

typedef uint8_t byte;
typedef uint32_t uint;

typedef struct { byte const *ptr; uint len; } MEM;
typedef struct BPSEARCH BPSEARCH;

BPSEARCH *bpsearch_create(MEM const*pattv, uint npatts);
int       bpsearch_scan(BPSEARCH *bp, MEM text, uint *matchnum, uint *matchpos);
void      bpsearch_destroy(BPSEARCH *bp);
