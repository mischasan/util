include $(word 1,${RULES} rules.mk)
~ := util
. := $(word 1,${$~} .)

#---------------- PRIVATE VARS:
# util.progs: programs for non-tap (e.g. perf) tests.
# util.x: test programs requiring no args

#Darwin does not have procfs, so no maccess.
#Darwin does not have fmemopen/fopencookie/fwopen, so no tolog

$~not.Darwin        = maccess.c tolog.c
$~c                 = $(addprefix $./, $(filter-out ${$~not.${OS}}, acstr.c bitmat.c bloom.c bndmem.c cessuhash.c concur.c enum.c fnvhash.c maccess.c map.c mm3hash.c msutil.c ordhuff.c phkeys.c rollhash.c rsort.c scan.c sha1.c sha256.c sheap.c sock.c ssearch.c ssearch_dump.c thread.c tolog.c uri.c xmutil.c ymd.c))
$~bin               = $(addprefix $./, concurs gai realpath)
$~scripts           = $(addprefix $./, amok covsum deas envy hist ifdent mkpg pg pgconf pglist pgps proj.fns pspace rules statpg statps tab)

# fail: FAILING TESTS XXX
# tbin: compiled test pgms
# xbin: pms used by test scripts
# test: tbin plus scripts using xbin

$~FAIL              = concur_t intsum_t search_t ssearch_t str_t ucsutf_t udp_t xmutil_t
$~tbin              = $(addprefix $./, $(filter-out ${$~FAIL}, bitmat_t bloom_t bndm_t concur_t intsum_t map_x msutil_t phkeys_t rollhash_x rsort_x scan_t search_t sheap_t sock_t sock_x subref_t thread_x ucsutf_t uri_t))
$~xbin              = $(addprefix $./, fnv_x ordhuff_x sort_x ssearch_x stress str_x)
$~test              = ${$~tbin} $(addprefix $./, $(filter-out ${$~FAIL}, fnv_t map_t ordhuff_t rsort_t))

#---------------- PUBLIC VARS:
clean               += $./{match,dump}.tmp

# Inputs to "make install":
install             : $(addprefix $./, libmsutil.a bloom.h concur.h msutil.h rsort.h sha1.h sha256.h sheap.h sock.h ssearch.h tap.h ubloom.h xmutil.h)
install.bin         : ${$~bin} ${$~scripts}

#---------------- PUBLIC TARGETS (see rules.mk):
all                 : ${$~bin} ${$~lib} ${$~tbin} ${$~xbin}
test                : ${$~test:=.pass}

#---------------- PRIVATE RULES:
${$~bin}            : $./libmsutil.a
${$~tbin} ${$~xbin} : $./libmsutil.a $./tap.o

$./libmsutil.a      : ${$~c:c=o}
$./nolock           : LDLIBS += -pthread
$./xmem             : LDLIBS += -lstdc++
$./thread_x         : LDLIBS += -pthread
$./mac.o            : CFLAGS += -fPIC

# Test that require aux files:
$./bndm_t.pass      : $./words
$./maccess_t.pass   : $./mac.so
$./str_t.pass       : $./88.tab $./osho.txt

# ~test script tests depend on $~xbin programs:
$./fnv_t.pass       : $./fnv_x
$./map_t.pass       : $./map_x
$./ordhuff_t.pass   : $./ordhuff_x
$./rsort_t.pass     : $./rsort_x
$./ssearch_t.pass   : $./ssearch_x
$./str_t.pass       : $./str_x

# source: source files not currently used by any targets.
# Note that "make source" will no be correct until AFTER "make all", which generates all the *.d files, which in turn add the *.h files to the source list.
source += ${$~scripts} $(addprefix $./, .gdbinit aa.c bergstr.c bloom_t.c bndmem.c _concur.h concur.h concur_t.c intsum_t.c jlu32.c maccess.c search_t.c *ssearch.h sort_x.c strstr42.asm strstr.S tt.c ucsutf_t.c winstr.c xmm.txt)

# lohuff requires qsort_r (FreeBSD only so far)
# maccess needs 64bit formats.

-include $./*.d
%.i                 : CPPFLAGS += -I$.

# vim: set nowrap :
