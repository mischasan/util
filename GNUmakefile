-include rules.mk
export util ?= .

#---------------- PRIVATE VARS:
# util.progs: programs for non-tap (e.g. perf) tests.
# util.x: test programs requiring no args

#MACOSX does not have procfs, so no maccess.
#MACOSX does not have fmemopen/fopencookie/fwopen, so no tolog

#util.c          = $(addprefix $(util)/, acstr.c bitmat.c bloom.c bndmem.c cessuhash.c concur.c enum.c fnvhash.c maccess.c map.c mm3hash.c msutil.c ordhuff.c phkeys.c rollhash.c rsort.c scan.c sha1.c sha256.c sheap.c sock.c ssearch.c ssearch_dump.c thread.c tolog.c uri.c xmutil.c ymd.c)
util.c          = $(addprefix $(util)/, acstr.c bitmat.c bloom.c bndmem.c cessuhash.c concur.c enum.c fnvhash.c map.c mm3hash.c msutil.c ordhuff.c phkeys.c rollhash.c rsort.c scan.c sha1.c sha256.c sheap.c sock.c ssearch.c ssearch_dump.c thread.c uri.c xmutil.c ymd.c)

util.bin        = $(addprefix $(util)/, concurs gai realpath)
util.scripts    = $(addprefix $(util)/, amok covsum deas envy hist ifdent mkpg pg pgconf pglist pgps proj.fns pspace rules statpg statps tab)

util_t          = bitmat_t bloom_t bndm_t concur_t intsum_t msutil_t phkeys_t scan_t search_t sheap_t sock_t ssesort_t subref_t tran_t ucsutf_t udp_t uri_t xmutil_t
util_x          = bloom_x brk_x fnv_x hash_x map_x ordhuff_x rollhash_x rsort_x sock_x sort_x ssearch_x str_x thread_x

util.xbin       = $(addprefix $(util)/, fnv_x map_x ordhuff_x rsort_x)  # sort_x
util.tbin       = $(addprefix $(util)/, bitmat_t bloom_t bndm_t map_x msutil_t phkeys_t rollhash_x rsort_x scan_t sheap_t sock_t sock_x subref_t thread_x uri_t)
util.test       = $(util.tbin) $(addprefix $(util)/, fnv_t map_t ordhuff_t rsort_t)

# fail: exe brk_x hash_x nolock sock_t sort_x ssearch_x stress str_x udp_t xmutil_t
# fail: sh  concur_t str_t ssearch_t

#---------------- PUBLIC VARS:
all		+= util
clean 	        += $(util)/{match,dump}.tmp $(util)/words $(util)/88.tab $(util)/osho.txt

# Inputs to "make install":
install.bin     += $(util.bin) $(util.scripts)
install.lib     += $(util)/libmsutil.a $(util)/libtap.a
install.include += $(addprefix $(util)/, bloom.h concur.h msutil.h rsort.h sha1.h sha256.h sheap.h sock.h ssearch.h tap.h ubloom.h xmutil.h)

#---------------- PUBLIC TARGETS (see rules.mk):
all             : $(util.bin) $(util.lib) $(util.tbin)
test            : $(util.test:=.pass)

#---------------- PRIVATE RULES:
util.all        : $(util.bin) $(util.lib) $(util.tbin) $(util.xbin)

$(util.bin)             : $(util)/libmsutil.a
$(util.tbin)            : $(util)/libmsutil.a $(util)/libtap.a
$(util.xbin)            : $(util)/libmsutil.a
$(util.test:=.pass)     : PATH := $(util):$(PATH)

$(util)/libmsutil.a     : $(util.c:c=o)
$(util)/libtap.a        : $(util)/tap.o
$(util)/nolock          : LDLIBS += -pthread
$(util)/thread_x        : LDLIBS += -pthread
$(util.test:=.pass)     : PATH := $(util):$(PATH)
$(util)/mac.o           : CFLAGS += -fPIC

#$(util)/railgun7k.o 	: CFLAGS += -funroll -fopenmp
#$(util)/railgun7k.o 	: CPPFLAGS += -D_FILE_OFFSET_BITS=64 -DCommence_OpenMP -D_gcc_mumbo_jumbo_

$(util)/bndm_t.pass     : $(util)/words
$(util)/fnv_t.pass      : $(util)/fnv_x
$(util)/maccess_t.pass  : $(util)/mac.so
$(util)/map_t.pass      : $(util)/map_x
$(util)/ordhuff_t.pass  : $(util)/ordhuff_x
$(util)/rsort_t.pass    : $(util)/rsort_x
$(util)/ssearch_t.pass  : $(util)/ssearch_x
$(util)/str_t.pass      : $(util)/str_x $(util)/88.tab

# source: source files not currently used by any targets.
source += $(util.scripts) $(addprefix $(util)/, .gdbinit aa.c bergstr.c bloom_t.c bndmem.c _concur.h concur.h concur_t.c intsum_t.c jlu32.c railgun7*.c search_t.c *ssearch.h strstr42.asm strstr.S tt.c winstr.c xmm.txt)

# lohuff requires qsort_r (FreeBSD only so far)
# maccess needs 64bit formats.

-include $(util)/*.d
%.i : CPPFLAGS += -I$(util)

# vim: set nowrap :
