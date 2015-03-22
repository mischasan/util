include $(word 1,${RULES} rules.mk)
_ := util
. = $(word 1,${$_} .)

#---------------- PRIVATE VARS:
# util.progs: programs for non-tap (e.g. perf) tests.
# util.x: test programs requiring no args

#MACOSX does not have procfs, so no maccess.
#MACOSX does not have fmemopen/fopencookie/fwopen, so no tolog

#$_.c         = $(addprefix $./, acstr.c bitmat.c bloom.c bndmem.c cessuhash.c concur.c enum.c fnvhash.c maccess.c map.c mm3hash.c msutil.c ordhuff.c phkeys.c rollhash.c rsort.c scan.c sha1.c sha256.c sheap.c sock.c ssearch.c ssearch_dump.c thread.c tolog.c uri.c xmutil.c ymd.c)
$_.c          = $(addprefix $./, acstr.c bitmat.c bloom.c bndmem.c cessuhash.c concur.c enum.c fnvhash.c           map.c mm3hash.c msutil.c ordhuff.c phkeys.c rollhash.c rsort.c scan.c sha1.c sha256.c sheap.c sock.c ssearch.c ssearch_dump.c thread.c         uri.c xmutil.c ymd.c)

$_.bin        = $(addprefix $./, concurs gai realpath)
$_.scripts    = $(addprefix $./, amok covsum deas envy hist ifdent mkpg pg pgconf pglist pgps proj.fns pspace rules statpg statps tab)

#util_t         = bitmat_t bloom_t bndm_t concur_t intsum_t msutil_t phkeys_t scan_t search_t sheap_t sock_t ssesort_t subref_t tran_t ucsutf_t udp_t uri_t xmutil_t
util_t          = bitmat_t bloom_t bndm_t concur_t intsum_t msutil_t phkeys_t scan_t search_t sheap_t sock_t ssesort_t subref_t tran_t udp_t uri_t xmutil_t
util_x          = bloom_x brk_x fnv_x hash_x map_x ordhuff_x rollhash_x rsort_x sock_x sort_x ssearch_x str_x thread_x

#$_.xbin      = $(addprefix $./, fnv_x map_x ordhuff_x rsort_x sort_x)
$_.xbin       = $(addprefix $./, fnv_x map_x ordhuff_x rsort_x)
#$_.tbin      = $(addprefix $./, bitmat_t bloom_t bndm_t map_x msutil_t phkeys_t rollhash_x rsort_x scan_t sheap_t sock_t sock_x subref_t thread_x ucsutf_t uri_t)
$_.tbin       = $(addprefix $./, bitmat_t bloom_t bndm_t map_x msutil_t phkeys_t rollhash_x rsort_x scan_t sheap_t sock_t sock_x subref_t thread_x uri_t)
_.test       = ${$_.tbin} $(addprefix $./, fnv_t map_t ordhuff_t rsort_t)

# fail: exe brk_x hash_x nolock sock_t sort_x ssearch_x stress str_x udp_t xmutil_t
# fail: sh  concur_t str_t ssearch_t

#---------------- PUBLIC VARS:
all		+= $_
clean 	        += $./{match,dump}.tmp $./words $./88.tab $./osho.txt
# source: files that should be sent with the build package but are not (yet) in any dependency.
source          += $./tap.[ch]

# Inputs to "make install":
install.bin     += ${$_.bin} ${$_.scripts}
install.lib     += $./libmsutil.a
install.include += $(addprefix $./, bloom.h concur.h msutil.h rsort.h sha1.h sha256.h sheap.h sock.h ssearch.h tap.h ubloom.h xmutil.h)

#---------------- PUBLIC TARGETS (see rules.mk):
all             : ${$_.bin} ${$_.lib} ${$_.tbin} ${$_.xbin}
test            : ${_.test:=.pass}

#---------------- PRIVATE RULES:
${$_.bin}             : $./libmsutil.a
${$_.tbin}            : $./libmsutil.a $./tap.o 
${$_.xbin}            : $./libmsutil.a
${_.test:=.pass}     : PATH := $.:${PATH}

$./libmsutil.a     : ${$_.c:c=o}
$./nolock          : LDLIBS += -pthread
$./xmem            : LDLIBS += -lstdc++

$./thread_x        : LDLIBS += -pthread
${_.test:=.pass}     : PATH := $.:${PATH}
$./mac.o           : CFLAGS += -fPIC

#$./railgun7k.o 	: CFLAGS += -funroll -fopenmp
#$./railgun7k.o 	: CPPFLAGS += -D_FILE_OFFSET_BITS=64 -DCommence_OpenMP -D_gcc_mumbo_jumbo_

# Test that require aux files:
$./bndm_t.pass     : $./words
$./maccess_t.pass  : $./mac.so
$./str_t.pass      : $./88.tab

# Various scripts depend on standalone executables:
$./fnv_t.pass      : $./fnv_x
$./map_t.pass      : $./map_x
$./ordhuff_t.pass  : $./ordhuff_x
$./rsort_t.pass    : $./rsort_x
$./ssearch_t.pass  : $./ssearch_x
$./str_t.pass      : $./str_x

# source: source files not currently used by any targets.
source += ${$_.scripts} $(addprefix $./, .gdbinit aa.c bergstr.c bloom_t.c bndmem.c _concur.h concur.h concur_t.c intsum_t.c jlu32.c maccess.c railgun7*.c search_t.c *ssearch.h sort_x.c strstr42.asm strstr.S tt.c ucsutf_t.c winstr.c xmm.txt)

# lohuff requires qsort_r (FreeBSD only so far)
# maccess needs 64bit formats.

-include $./*.d
%.i : CPPFLAGS += -I$.

# vim: set nowrap :
