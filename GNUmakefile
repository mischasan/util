-include rules.mk
export util ?= .

#---------------- PRIVATE VARS:
# util.progs: programs for non-tap (e.g. perf) tests.
# util.tbin: test programs requiring no args

#MACOSX does not have procfs, so no maccess.
util.c          = $(addprefix $(util)/, acstr.c bitmat.c bloom.c bndmem.c bpsearch.c cessuhash.c concur.c enum.c fnvhash.c jlu32.c maccess.c map.c mm3hash.c msutil.c ordhuff.c phkeys.c psearch.c psearch_create.c psearch_dump.c psearch_file.c rollhash.c rsort.c scan.c sha1.c sha256.c sheap.c sock.c ssearch.c ssearch_dump.c thread.c tolog.c uri.c xmutil.c ymd.c)
util.progs      = $(addprefix $(util)/, bloom_x brk_x gai hash_x memx nolock sock_x sort_x stress thread_x)

util.bin        = $(addprefix $(util)/, concurs realpath)
util_tbin       = bitmat_t bloom_t bndm_t maccess_t msutil_t scan_t sheap_t sock_t subref_t tolog_t udp_t uri_t 
util.tbin       = $(addprefix $(util)/, $(util_tbin) fnv_x map_x ordhuff_x psearch_x psearch_mmap_x rollhash_x rsort_x ssearch_x str_x)
util.test       = $(addprefix $(util)/, $(util_tbin) fnv_t map_t ordhuff_t psearch_t rsort_t ssearch_t str_t)
util.scripts    = $(addprefix $(util)/, amok covsum deas envy hist ifdent mkpg pg pgconf pglist pgps proj.fns pspace rules statpg statps tab)

#---------------- PUBLIC VARS:
# Inputs to "make install":
install.bin     += $(util.bin) $(util.scripts)
install.include += $(addprefix $(util)/, concur.h msutil.h psearch.h rsort.h sha1.h sha256.h ssearch.h tap.h xmutil.h)
install.lib     += $(util)/libmsutil.a

# Inputs to "make clean cover":
all		        += util
clean 	        += $(util)/{psearch,match,dump}.tmp $(util)/words $(util)/88.tab $(util)/osho.txt

util.cover      = $(addprefix $(util)/, fnvhash.c map.c msutil.c ordhuff.c psearch.c psearch_create.c psearch_dump.c psearch_file.c reffile.c rsort.c scan.c sock.c ssearch.c ssearch_dump.c tolog.c uri.c xmutil.c)

#---------------- PUBLIC TARGETS (see rules.mk):
all             : $(util.bin) $(util)/libmsutil.a $(util.tbin)
test            : $(util.test:=.pass)

#---------------- PRIVATE RULES:
$(util.bin)     : $(util)/libmsutil.a
$(util.tbin)    : $(util)/libmsutil.a $(util)/libtap.a
$(util)/aa	    : $(util)/libmsutil.a
$(util)/nolock  : $(util)/libmsutil.a

$(util)/libmsutil.a     : $(util.c:c=o)
$(util)/libtap.a        : $(util)/tap.o
$(util)/nolock          : LDLIBS += -pthread
$(util)/thread_x        : LDLIBS += -pthread
$(util.test:=.pass)     : PATH := $(util):$(PATH)
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

$(util)/ordhuff.o $(util)/ordhuff.s : CFLAGS.Linux += -funroll-all-loops -fvariable-expansion-in-unroller

# source: source files not currently used by any targets.
source += $(util.scripts) $(addprefix $(util)/, .gdbinit aa.c bergstr.c bloom_t.c bndmem.c _concur.h concur.h concur_t.c intsum_t.c jlu32.c psearch.h psearch_strings.c railgun7*.c search_t.c *ssearch.h strstr42.asm strstr.S tt.c winstr.c xmm.txt)

# lohuff requires qsort_r (FreeBSD only so far)
# maccess needs 64bit formats.

-include $(util)/*.d
%.i : CPPFLAGS += -I$(util)

# vim: set nowrap :
