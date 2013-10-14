#ifndef UBLOOM_H
#define UBLOOM_H
#include <stdint.h>     // uint32_t etc.

// ubloom uses a fixed (K=5, M/N=16) bloom filter.

typedef struct ubloom UBLOOM;
typedef uint32_t UHASH[6];

void    ubloom_create(char const *filename, int maxrecs);
void    ubloom_destroy(char const *filename);

UBLOOM *ubloom_open(char const *filename, int mode);
void    ubloom_close(UBLOOM*);

int     ubloom_test(UBLOOM const*, UHASH const hash);

void    ubloom_add(UBLOOM*, UHASH const hash);
void    ubloom_del(UBLOOM*, UHASH const hash);

void    ubloom_addv(UBLOOM*, UHASH const hash[], int nhashes);
void    ubloom_delv(UBLOOM*, UHASH const hash[], int nhashes);

#endif//UBLOOM_H
