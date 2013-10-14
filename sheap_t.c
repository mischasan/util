#include <msutil.h>
#include <tap.h>
#include "sheap.h"

typedef struct { char const *name; SCORE val; } NAMEVAL;
// Define function: NAMEVAL *val_TO_NAMEVAL(SCORE*)
MAKE_field_TO_struct(val, NAMEVAL)

int
main(void)
{
    plan_tests(1);
    setvbuf(stdout, 0, _IOLBF, 0);

    SHEAP *shp = sheap_create(10);
    ok(shp, "created");
    
    NAMEVAL some[22] = {
        { "alpha",    31 },
        { "beta",     41 },
        { "gamma",    59 },
        { "delta",    26 },
        { "epsilon",  53 },
        { "zeta",     58 },
        { "eta",      97 },
        { "theta",    93 },
        { "iota",     23 },
        { "kappa",    84 },
        { "lambda",   26 },
        { "mu",       43 },
        { "nu",       38 },
        { "xi",       32 },
        { "omicron",  79 },
        { "pi",       59 },
        { "rho",      28 },
        { "sigma",    84 },
        { "tau",      19 },
        { "phi",      71 },
        { "chi",      69 },
        { "psi",      19 }
    };

    int i;
    printf("push x 3\n");
    for (i = 0; i < 3; sheap_push(shp, &some[i++].val));
    sheap_dump(shp, stderr);

    for (i = 0; i < 3; i++)
        printf("pull %d\n", *sheap_pull(shp)), sheap_dump(shp, stderr);
    
    // Select the top (10) entries from a stream of 10+12, with dups.
    for (i = 0; i < 22; ++i) {
        if (i < sheap_size(shp)) {
            printf("push %d %s\n", some[i].val, some[i].name);
            sheap_push(shp, &some[i].val);
        } else if (some[i].val <= sheap_head(shp)) {
            printf("skip %d %s < %d \n",
                    some[i].val, some[i].name, sheap_head(shp));
        } else {
            // Only the first (10) elements of input are "saved" in "RAM".
            NAMEVAL *nvp = val_TO_NAMEVAL(sheap_pull(shp));   // extract lowest element
            printf("pull %d %s\n", nvp->val, nvp->name);
            *nvp = some[i]; 
            printf("push %d %s\n", nvp->val, nvp->name);
            sheap_push(shp, &nvp->val);
        }
    } 

    puts("# sheap_dump:");
    sheap_dump(shp, stderr);
    puts("# NAMEVAL[0..9]:");
    for (i = 0; i < 10; ++i)
        printf( " [%d]\t%d %s\n", i, some[i].val, some[i].name);

    puts("# pull x 10:");
    while (sheap_count(shp)) {
        NAMEVAL *nvp = val_TO_NAMEVAL(sheap_pull(shp));
        printf("\t%d %s\n", nvp->val, nvp->name);
    }

    sheap_dump(shp, stderr);
    sheap_destroy(shp);

    return exit_status();
}
