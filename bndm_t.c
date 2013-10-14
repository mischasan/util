#include <tap.h>
#include "msutil.h"

int main(int argc, char **argv)
{
    plan_tests(1);
    char dflt[] = "one", *patt = argc > 1 ? argv[1] : dflt;
    char const *file = argc > 2 ? argv[2] : "words";
    FILE *fp = fopen(file, "r");
    if (!fp) return fprintf(stderr, "bndm_t: cannot open %s\n", file);
    int patlen = strlen(patt), lines = 0, goods = 0, hits = 0;
    char buf[999];

    while (fgets(buf, sizeof buf, fp)) {
        lines++;
        char *cp = strstr(buf, patt);
        hits += !!cp;
        if (cp == bndmem(buf, strlen(buf) - 1, patt, patlen)) ++goods;
        else fprintf(stderr, "# failed: %s", buf);
    }
    ok(goods == lines, "strstr == bndmem %d/%d times (%d hits)", goods, lines, hits);

    return exit_status();
}
