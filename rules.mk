# Environment:
#   $BLD        : empty, or one of:   cover  debug  profile
# Makefile vars:
#   all += <module> where <module>.
#   clean += <pathnames>

ifndef RULES_MK
RULES_MK:=1 # Allow repeated "-include".

PS4             := \# # Prefix for "sh -x" output.
export LD_LIBRARY_PATH PS4

# Import from PREFIX, export to DESTDIR.
PREFIX          ?= /usr/local
DESTDIR         ?= $(PREFIX)
OSName          := $(shell uname -s)
OSNAME          := $(shell uname -s | tr '[a-z]' '[A-Z]')

# HACK CentOS 5 comes with gcc 4.1, gcc 4.4 requires a special command
#CC              = /usr/bin/gcc44

#--- *.$(BLD):
# -O level > 2 makes gcc 4.2 go weird on MacOSX
CFLAGS.         = -O2
CFLAGS.cover    = --coverage -DNDEBUG
LDFLAGS.cover   = --coverage

CFLAGS.debug    = -O0 -Wno-uninitialized
CPPFLAGS.debug  = -UNDEBUG

CFLAGS.profile  = -pg -DNDEBUG
LDFLAGS.profile = -pg
# PROFILE tests get stats on syscalls in their .pass files.
exec.profile	= strace -cf

#--- *.$(OSName):
CFLAGS.Darwin   = 
LDLIBS.FreeBSD  = -lm
LDLIBS.Linux    = -ldl -lm -lresolv

# Before gcc 4.5, -Wno-unused-result was unknown and causes an error.
Wno-unused-result := $(shell $(CC) -dumpversion | awk '$$0 >= 4.5 {print "-Wno-unused-result"}')

# XXX -funsigned-char would save time.
CFLAGS          += -ggdb -MMD -fdiagnostics-show-option -fstack-protector --param ssp-buffer-size=4 -fno-strict-aliasing
#CFLAGS          += -Wall -Werror -Wextra -Wcast-align -Wcast-qual -Wformat=2 -Wformat-security -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wno-unknown-pragmas -Wunused -Wwrite-strings
CFLAGS          += -Wno-attributes -Wno-cast-qual -Wno-unknown-pragmas $(Wno-unused-result)
CFLAGS          += $(CFLAGS.$(BLD)) $(CFLAGS.$(OSName))

CXXFLAGS += $(filter-out -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes, $(CFLAGS))

# -D_FORTIFY_SOURCE=2 on some plats rejects any libc call whose return value is ignored.
#   For some calls (system, write) this makes sense. For others (vasprintf), WTF?

CPPFLAGS        += -I$(PREFIX)/include -DPLATFORM_$(OSNAME) -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE $(CPPFLAGS.$(BLD)) $(CPPFLAGS.$(OSName))
LDFLAGS         += -L$(PREFIX)/lib $(LDFLAGS.$(BLD)) $(LDFLAGS.$(OSName))
LDLIBS          += $(LDLIBS.$(OSName))

#---------------- Explicitly CANCEL EVIL BUILTIN RULES:
%               : %.c 
%               : %.cpp
%.c             : %.l
%.c             : %.y
%.r             : %.l
#----------------
.PHONY          : all clean cover debug gccdefs install profile source tags test
.DEFAULT_GOAL   := all

# $(all) contains all subproject names. It can be used in ACTIONS but not RULES,
# since it accumulates across "include */GNUmakefile"'s.

# All $(BLD) types use the same pathnames for binaries.
# To switch from release to debug, first "make clean".
# To extract and save exports, "make install DESTDIR=rel".

all             :;@echo "$@ done for BLD='$(BLD)'"
clean           :;@rm -rf $(shell $(MAKE) -nps all test cover profile | sed -n '/^# I/,$${/^[^\#\[%.][^ %]*: /s/:.*//p;}') \
			  $(clean)  $(foreach A,$(all), $($A)/{gmon.out,tags,*.fail,*.gcda,*.gcno,*.gcov,*.prof}) \
                          $(filter %.d,$(MAKEFILE_LIST))

cover           : BLD := cover
%.cover         : %.test    ; gcov -bcp $($@) | covsum

debug           : BLD := debug
debug           : all

# Expand: translate every occurrence of "${var}" in a file to its env value (or ""):
# $(Expand) template >target
Expand          = perl -pe 's/ (?<!\\) \$${ ([A-Z_][A-Z_0-9]*) } / $$ENV{$$1} || ""/geix'

# $(call Install,TGTDIR,SRCFILES):
Install         = if [ "$(strip $2)" ]; then mkdir -p $1; pax -rw -pe -s:.*/:: $2 $1; fi

ToUpper = $(shell echo $1 | tr '[a-z]' '[A-Z]')

# If you believe in magic vars, e.g. "myutil.bin = prog1 prog2 prog3"
# causing "myutil.install" to copy those files to $(DESTDIR)/bin
# then here's your automagic "install":
#XXX use global vars (bin lib ...) to which all subprojects += ...
%.install       : %.all $(%.bin) $(%.etc) $(%.include) $(%.ini) $(%.lib) $(%.sbin) \
                ;@$(call Install,$(DESTDIR)/bin,    $($*.bin))  \
                ; $(call Install,$(DESTDIR)/etc,    $($*.etc))  \
                ; $(call Install,$(DESTDIR)/ini,    $($*.ini))  \
                ; $(call Install,$(DESTDIR)/lib,    $($*.lib))  \
                ; $(call Install,$(DESTDIR)/sbin,   $($*.sbin)) \
                ; $(call Install,$(DESTDIR)/include,$($*.include))

#$(DESTDIR)/bin/%	: %; $(call Install,$(@D),$<)
#$(DESTDIR)/etc/%	: %; $(call Install,$(@D),$<)
#$(DESTDIR)/include/% 	: %; $(call Install,$(@D),$<)
#$(DESTDIR)/ini/%	: %; $(call Install,$(@D),$<)
#$(DESTDIR)/lib/% 	: %; $(call Install,$(@D),$<)
#$(DESTDIR)/sbin/% 	: %; $(call Install,$(@D),$<)

profile         : BLD := profile
%.profile       : %.test    ;@for x in $($*.test:.pass=); do gprof -b $$x >$$x.prof; done

%.test          : $(%.test)
# GMAKE trims leading "./" from $*.; $(*D)/$(*F) restores it.
%.pass          : %         ; rm -f $@; $(exec.$(BLD)) $(*D)/$(*F) >& $*.fail && mv -f $*.fail $@

# To build a .so, "make clean" first, to ensure all .o files compiled with -fPIC
%.so            : CFLAGS := -fPIC $(filter-out $(CFLAGS.cover) $(CFLAGS.profile), $(CFLAGS))
%.so            : %.o       ; $(CC) $(LDFLAGS) -o $@ -shared $< $(LDLIBS)
%.so            : %.a       ; $(CC) $(CFLAGS)  -o $@ -shared -Wl,-whole-archive $< $(LDLIBS)
%.a             :           ; [ "$^" ] && ar crs $@ $(filter %.o,$^)
%.yy.c          : %.l       ; flex -o $@ $<
%.tab.c 	: %.y       ; bison $<
%/..            :           ;@mkdir -p $(@D)
%               : %.gz      ; gunzip -c $^ >$@

# Ensure that intermediate files (e.g. the foo.o caused by "foo : foo.c")
#  are not auto-deleted --- causing a re-compile every second "make".
.SECONDARY  	: 

#---------------- TOOLS:
# NOTE: "source" MUST be set with "=", not ":=", else MAKE recurses infinitely.
source          = $(filter-out %.d, $(shell $(MAKE) -nps all test cover profile | sed -n '/^. Not a target/{n;/^[^.*][^ ]*:/s/:.*//p;}'))

# gccdefs : all gcc internal #defines.
gccdefs         :;@$(CC) $(CPPFLAGS) -E -dM - </dev/null | cut -c8- | sort
tags            : all; ctags $(filter %.c %.cpp %.h, $(source))
# sh : invoke a shell within the makefile's env:
sh   		:; PS1='$(PS1) [make] ' $(SHELL)

# %.I lists all (recursive) #included files; e.g.: "make /usr/include/errno.h.I"
%.I             : %.c       ;@ls -1 2>&- `$(CC) $(CPPFLAGS) -M $<` ||:
%.I             : %.cpp     ;@ls -1 2>&- `$(CXX) $(CPPFLAGS) -M $<` ||:
%.i             : %.c       ; $(COMPILE.c) -E -o $@ $<
%.i             : %.cpp     ; $(COMPILE.cpp) -E -o $@ $<
%.s             : %.c       ; $(COMPILE.c) -S -o $@ $< && deas $@
%.s             : %.cpp     ; $(COMPILE.cpp) -S -o $@ $< && deas $@

# "make SomeVar." prints $(SomeVar)
%.              :;@echo '$($*)'

endif
# vim: set nowrap :
