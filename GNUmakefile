-include rules.mk

export util ?= .
#---------------- PRIVATE VARS:
# util.progs: programs for non-tap (e.g. perf) tests.
# util.tpgms: test programs requiring no args

#MACOSX does not have procfs, so no maccess.
util.c          = $(patsubst %,$(util)/%, acstr.c bitmat.c bloom.c bndmem.c cessuhash.c concur.c enum.c fnvhash.c jlu32.c maccess.c map.c mm3hash.c msutil.c ordhuff.c phkeys.c psearch.c psearch_create.c psearch_dump.c psearch_file.c rollhash.c rsort.c scan.c sha1.c sha256.c sheap.c sock.c ssearch.c ssearch_dump.c thread.c tolog.c uri.c xmutil.c ymd.c)
util.progs      = $(patsubst %,$(util)/%, bloom_x brk_x gai hash_x memx nolock sock_x sort_x stress thread_x tt)

#util_tpgms      = bitmat_t bloom_t maccess_t msutil_t scan_t sheap_t sock_t subref_t tolog_t udp_t uri_t xmutil_t
util_tpgms      = bitmat_t bloom_t bndm_t maccess_t msutil_t scan_t sheap_t sock_t subref_t tolog_t udp_t uri_t 
util.tpgms      = $(patsubst %,$(util)/%,      $(util_tpgms) fnv_x map_x ordhuff_x psearch_x psearch_mmap_x rollhash_x rsort_x ssearch_x str_x)
util.test       = $(patsubst %,$(util)/%.pass, $(util_tpgms) fnv_t map_t ordhuff_t psearch_t rsort_t ssearch_t str_t)
util.pgms       = $(patsubst %,$(util)/%, concurs realpath)
util.scripts    = $(patsubst %,$(util)/%, amok covsum deas envy hist ifdent mkpg pg pgconf pglist pgps proj.fns pspace rules statpg statps tab)

#---------------- PUBLIC VARS:
# Inputs to "make install":
util.bin        = $(util.pgms) $(util.scripts)
util.lib        = $(patsubst %,$(util)/%, libmsutil.a libtap.a)
util.include    = $(patsubst %,$(util)/%, concur.h msutil.h psearch.h rsort.h sha1.h sha256.h ssearch.h tap.h xmutil.h)

# Inputs to "make clean cover":
all		+= util
clean 	        += $(util)/{psearch,match,dump}.tmp $(util)/words $(util)/88.tab $(util)/osho.txt
util.cover      = $(patsubst %,$(util)/%, fnvhash.c map.c msutil.c ordhuff.c psearch.c psearch_create.c psearch_dump.c psearch_file.c reffile.c rsort.c scan.c sock.c ssearch.c ssearch_dump.c tolog.c uri.c xmutil.c)

#---------------- PUBLIC TARGETS (see rules.mk):
all             : util.all
test            : util.test
cover           : util.cover
profile         : util.profile
install         : util.install

#---------------- PRIVATE RULES:
util.all        : $(util.bin) $(util.lib) $(util.progs) $(util.tpgms)
util.test       : $(util.test)

$(util.pgms)     : $(util)/libmsutil.a
$(util.progs)   : $(util)/libmsutil.a
# util.tpgms have "#include <tap.h>" in them:
$(util.tpgms)   : CPPFLAGS := -I$(util) $(CPPFLAGS)
$(util.tpgms)   : $(util)/libmsutil.a $(util)/libtap.a
$(util)/aa	: $(util)/libmsutil.a
$(util)/libmsutil.a     : $(util.c:c=o)
$(util)/libtap.a        : $(util)/tap.o
$(util)/nolock          : LDLIBS += -pthread
$(util)/thread_x        : LDLIBS += -pthread
$(util.test)            : PATH := $(util):$(PATH)
$(util)/mac.o           : CFLAGS += -fPIC

#$(util)/railgun7k.o 	: CFLAGS += -funroll -fopenmp
#$(util)/railgun7k.o 	: CPPFLAGS += -D_FILE_OFFSET_BITS=64 -DCommence_OpenMP -D_gcc_mumbo_jumbo_

$(util)/maccess_t.pass  : $(util)/mac.so
$(util)/bndm_t.pass     : $(util)/words
$(util)/fnv_t.pass      : $(util)/fnv_x
$(util)/map_t.pass      : $(util)/map_x
$(util)/ordhuff_t.pass  : $(util)/ordhuff_x
$(util)/psearch_t.pass  : $(util)/psearch_x $(util)/psearch_mmap_x
$(util)/rsort_t.pass    : $(util)/rsort_x
$(util)/ssearch_t.pass  : $(util)/ssearch_x
$(util)/str_t.pass      : $(util)/str_x $(util)/88.tab

# source: source files not currently used by any targets.
source += $(util.scripts) $(patsubst %,$(util)/%, .gdbinit aa.c bergstr.c bloom_t.c bndmem.c _concur.h concur.h concur_t.c intsum_t.c jlu32.c psearch.h psearch_strings.c railgun7*.c search_t.c *ssearch.h strstr42.asm strstr.S winstr.c xmm.txt)

# lohuff requires qsort_r (FreeBSD only so far)
# maccess needs 64bit formats.

-include $(util)/*.d
%.i : CPPFLAGS += -I$(util)

# vim: set nowrap :
