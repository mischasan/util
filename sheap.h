#ifndef SHEAP_H
#define SHEAP_H

// A score-heap manipulates a limited ordered set of (int*).
//  It uses (int*), not (int), because the int can then be
//  a struct field, and a pointer to the containing struct
//  can be computed from the pointer to the field.

#define MAKE_field_TO_struct(FLD,STR) \
static inline typeof(STR)* FLD##_TO_##STR(typeof(((STR*)0)->FLD)*fldp) \
{ return (STR*)((char*)(fldp) - (intptr_t)&((STR*)0)->val); }

typedef int SCORE;
#define SCORE_MIN (-__INT_MAX__)

typedef struct SHEAP_s SHEAP;

// sheap_head and sheap_pull will ASSERT if called on an empty SHEAP.
// sheap_push will ASSERT if called on a full SHEAP.

SHEAP* sheap_create(int size);
void   sheap_destroy(SHEAP*);
int    sheap_size(SHEAP const*);
int    sheap_count(SHEAP const*);
SCORE  sheap_head(SHEAP const*);
SCORE* sheap_pull(SHEAP*);
void   sheap_push(SHEAP*, SCORE*);

void   sheap_dump(SHEAP const*, FILE*);

#endif//SHEAP_H
