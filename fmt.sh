#!/bin/sh
trap 'Q=$?; rm -f a.out; exit $Q' 0
gcc -xc $CFLAGS - <<SRC
#include <stdio.h>
#include <sys/types.h>
#define FMT(CODE,TYPE) printf("#define F"#CODE" \"%s\"\n", sizeof(TYPE)==sizeof(int) ? "" : sizeof(TYPE)==sizeof(long) ? "l" : "ll")
int main(void) { FMT(OF, off_t); FMT(SZ, size_t); FMT(64, int64_t); return 0; }
SRC
./a.out
