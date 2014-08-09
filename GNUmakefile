-include rules.mk
export util ?= .

#---------------- PRIVATE VARS:
# util.progs: programs for non-tap (e.g. perf) tests.
# util.x: test programs requiring no args

#MACOSX does not have procfs, so no maccess.
#MACOSX does not have fmemopen/fopencookie/fwopen, so no tolog

#util.c          = $(addprefix $(util)/, acstr.c bitmat.c bloom.c bndmem.c bpsearch.c cessuhash.c concur.c enum.c fnvhash.c maccess.c map.c mm3hash.c msutil.c ordhuff.c phkeys.c psearch.c psearch_create.c psearch_dump.c psearch_file.c rollhash.c rsort.c scan.c sha1.c sha256.c sheap.c sock.c ssearch.c ssearch_dump.c thread.c tolog.c uri.c xmutil.c ymd.c)
util.c          = $(addprefix $(util)/, acstr.c bitmat.c bloom.c bndmem.c bpsearch.c cessuhash.c concur.c enum.c fnvhash.c map.c mm3hash.c msutil.c ordhuff.c phkeys.c psearch.c psearch_create.c psearch_dump.c psearch_file.c rollhash.c rsort.c scan.c sha1.c sha256.c sheap.c sock.c ssearch.c ssearch_dump.c thread.c uri.c xmutil.c ymd.c)

util.bin        = $(addprefix $(util)/, concurs gai realpath)
util.scripts    = $(addprefix $(util)/, amok covsum deas envy hist ifdent mkpg pg pgconf pglist pgps proj.fns pspace rules statpg statps tab)

#util_t          = bitmat_t bloom_t bndm_t concur_t intsum_t maccess_t msutil_t phkeys_t scan_t search_t sheap_t sock_t ssesort_t subref_t tolog_t tran_t ucsutf_t udp_t uri_t xmutil_t
util_t          = bitmat_t bloom_t bndm_t concur_t intsum_t msutil_t phkeys_t scan_t search_t sheap_t sock_t ssesort_t subref_t tran_t ucsutf_t udp_t uri_t xmutil_t
util_x          = bloom_x brk_x fnv_x hash_x map_x ordhuff_x psearch_x psearch_mmap_x rollhash_x rsort_x sock_x sort_x ssearch_x str_x thread_x   aa nolock stress tt

util.tbin       = $(addprefix $(util)/, $(util_t) $(util_x))
util.test       = $(util.tbin) $(addprefix $(util)/, fnv_t map_t ordhuff_t psearch_t rsort_t ssearch_t str_t)


#---------------- PUBLIC VARS:
all		+= util
clean 	        += $(util)/{psearch,match,dump}.tmp $(util)/words $(util)/88.tab $(util)/osho.txt

# Inputs to "make install":
install.bin     = $(util.bin) $(util.scripts)
install.lib     = $(util)/libmsutil.a $(util)/libtap.a
install.include = $(addprefix $(util)/, bloom.h bpsearch.h concur.h msutil.h psearch.h rsort.h sha1.h sha256.h sheap.h sock.h ssearch.h tap.h ubloom.h xmutil.h)

#---------------- PUBLIC TARGETS (see rules.mk):
all             : $(util.bin) $(util.lib) $(util.progs) $(util.x)
test            : $(util.test:=.pass)

#---------------- PRIVATE RULES:
util.all        : $(util.bin) $(util.lib) $(util.tbin)

$(util.bin)             : $(util)/libmsutil.a
$(util.tbin)            : $(util)/libmsutil.a $(util)/libtap.a
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
$(util)/psearch_t.pass  : $(util)/psearch_x $(util)/psearch_mmap_x
$(util)/rsort_t.pass    : $(util)/rsort_x
$(util)/ssearch_t.pass  : $(util)/ssearch_x
$(util)/str_t.pass      : $(util)/str_x $(util)/88.tab

# source: source files not currently used by any targets.
source += $(util.scripts) $(addprefix $(util)/, .gdbinit aa.c bergstr.c bloom_t.c bndmem.c _concur.h concur.h concur_t.c intsum_t.c jlu32.c psearch.h psearch_strings.c railgun7*.c search_t.c *ssearch.h strstr42.asm strstr.S tt.c winstr.c xmm.txt)

# lohuff requires qsort_r (FreeBSD only so far)
# maccess needs 64bit formats.

-include $(util)/*.d
%.i : CPPFLAGS += -I$(util)

# vim: set nowrap :
