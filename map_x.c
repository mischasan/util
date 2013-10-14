#include "msutil.h"
#include <tap.h>

const char *keyv[] = {
	"alpha","beta",	"gamma","delta","epsilon","zeta",
	"eta",	"theta","iota",	"kappa","lambda","mu",
};
#define	NKEYS	(int)(sizeof(keyv)/sizeof(*keyv))
const char *valv[] = {
	"zero",	"one",	"two",	"three","four",	"five",
	"six",	"seven","eight","nine",	"ten",	"eleven"
};

int
main(int argc, char **argv)
{
    int	i, j;
    char	*skey, *sval;

    plan_tests(5*NKEYS + 3 + 2*(argc - 1));
    KVS	sh = kvs_create();

    fputs("# pass 1: add 11 kv pairs\n", stderr);
    for (i = 0; i < NKEYS-1; ++i) {
	kvs_set(sh, keyv[i], valv[i+1]);
	ok(kvs_count(sh) == i+1, "adding (%s => %s) makes count: %d", 
	    keyv[i], valv[i+1], kvs_count(sh));
    }
    for (i = 0; i < NKEYS-1; ++i) {
	const char *vp = kvs_get(sh, keyv[i]);
	ok(vp && !strcmp(vp, valv[i+1]), "%s => %s", keyv[i], vp);
    }

    fputs("# pass 2: add 12 kv pairs, with different values\n", stderr);
    for (i = 0; i < NKEYS; ++i) {
	kvs_set(sh, keyv[i], valv[i]);
    }

    for (i = 0; i < NKEYS; ++i) {
	const char *vp = kvs_get(sh, keyv[i]);
	ok(vp && !strcmp(vp, valv[i]), "%s => %s", keyv[i], valv[i]);
    }

    ok(kvs_count(sh) == NKEYS, "kvs_count %d", kvs_count(sh));

    for (i = 0, kvs_start(sh);  kvs_next(sh, &skey, &sval); ++i) {
	for (j = 0; j < NKEYS && strcmp(keyv[j], skey); ++j);
	ok(j < NKEYS && !strcmp(sval, valv[j]), "scan finds (%s => %s)", skey, sval);
    }
    ok(i == NKEYS, "scan finds %d keys", i);

    fputs("# Delete 3 keys\n", stderr);
    kvs_del(sh, keyv[0]);
    kvs_del(sh, keyv[2]);
    kvs_del(sh, keyv[3]);

    ok(!kvs_get(sh, keyv[0]), "%s deleted ok", keyv[0]);
    ok(!kvs_get(sh, keyv[2]), "%s deleted ok", keyv[2]);
    ok(!kvs_get(sh, keyv[3]), "%s deleted ok", keyv[3]);
    ok(kvs_count(sh) == NKEYS - 3, "kvs_count %d", kvs_count(sh));
    for (i = 0; i < NKEYS; ++i) {
	if (i == 0 || i == 2 || i == 3) continue;
	ok(!strcmp(valv[i], kvs_get(sh, keyv[i])), "%s still ok", keyv[i]);
    }
    ok(kvs_count(sh) == 9, "count is %d", kvs_count(sh));

    fputs("# Delete all keys, including 3 already deleted\n", stderr);
    for (i = 0; i < NKEYS; ++i) {
	kvs_del(sh, keyv[i]);
    }    
    ok(kvs_count(sh) == 0, "final count is %d", kvs_count(sh));

    kvs_destroy(sh);
    while (*++argv) {
	char    val[9], buf[999];;
	int	    num = 0;

	FILE    *fp = fopen(*argv, "r");
	if (!fp) {
	    fprintf(stderr, "t.map: %s: cannot open\n", *argv);
	    continue;
	}

	sh = kvs_create();

	while (fgets(buf, sizeof(buf), fp)) {
	    sprintf(val, "%d", num++);
	    kvs_set(sh, buf, val);
	}

	ok(kvs_count(sh) == num, "kvs_count %d matches file count %d", 
	    kvs_count(sh), num);

	rewind(fp);
	num = 0;
	int	    found = 0;
	while (fgets(buf, sizeof(buf), fp)) {
	    sprintf(val, "%d", num++);
	    const char *ret = kvs_get(sh,buf);

	    if (!strcmp(ret, val)) {
		++found;
	    } else {
		fprintf(stderr, "retrieved %s, not %s, for key %s", 
			ret ? ret : "<NULL>", val, buf);
	    }
	}
	ok(found == num, "found %d of %d values in kvs",  found, num);
	kvs_destroy(sh);
    }

    return exit_status();
}
