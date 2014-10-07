# Environment:
#   $BLD        : empty, or one of:   cover  debug  profile
# Makefile vars:
#   all += <module> where <module>.
#   clean += <pathnames>

ifndef RULES_MK
RULES_MK:=1 # Allow repeated "-include".

export LD_LIBRARY_PATH
PS4             = \# # Prefix for "sh -x" output.
SHELL           = /bin/bash

# Import from PREFIX, export to DESTDIR.
PREFIX          ?= /usr/local
DESTDIR         ?= $(PREFIX)
OS              != uname -s

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

#--- *.$(OS):
CFLAGS.Darwin   = 
LDLIBS.FreeBSD  = -lm
LDLIBS.Linux    = -lm

CFLAGS          += -ggdb -MMD -fdiagnostics-show-option -fstack-protector --param ssp-buffer-size=4 -fno-strict-aliasing
CFLAGS          += -Wall -Werror -Wextra -Wcast-align -Wcast-qual -Wformat=2 -Wformat-security -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wno-unknown-pragmas -Wunused -Wwrite-strings
CFLAGS          += -Wno-attributes -Wno-cast-qual -Wno-unknown-pragmas
CFLAGS          += $(CFLAGS.$(BLD)) $(CFLAGS.$(OS))

CXXFLAGS += $(filter-out -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes, $(CFLAGS))

# -D_FORTIFY_SOURCE=2 on some plats rejects any libc call whose return value is ignored.
#   For some calls (system, write) this makes sense. For others (vasprintf), WTF?

CPPFLAGS        += -I$(PREFIX)/include -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE $(CPPFLAGS.$(BLD)) $(CPPFLAGS.$(OS))
LDFLAGS         += -L$(PREFIX)/lib $(LDFLAGS.$(BLD)) $(LDFLAGS.$(OS))
LDLIBS          += $(LDLIBS.$(OS))

#---------------- Explicitly CANCEL EVIL BUILTIN RULES:
%               : %.c 
%               : %.cpp
%.c             : %.l
%.c             : %.y
%.r             : %.l
#----------------
.PHONY          : all clean cover debug gccdefs install profile source tags test
.DEFAULT_GOAL   := all

# $(all) contains subproject names. It can be used in ACTIONS but not RULES,
#   since it accumulates across every "include <submakefile>"
# $(junkfiles) is how to get metachars (commas) through the $(addsuffix...) call.

all             :;@echo "$@ done for BLD='$(BLD)'"
clean           :;@rm -rf $(shell $(MAKE) -nps all install test | sed -n '/^# I/,$${/^[^\#\[%.][^ %]*: /s/:.*//p;}') \
                          $(addsuffix /$(junkfiles), $(all)) $(clean) $(filter %.d,$(MAKEFILE_LIST))
junkfiles       = {gmon.out,tags,*.fail,*.gcda,*.gcno,*.gcov,*.prof}

cover           : BLD := cover
%.cover         : %.test    ; gcov -bcp $($@) | covsum

debug           : BLD := debug
debug           : all

# Expand: translate every occurrence of "${var}" in a file to its env value (or ""):
# $(Expand) template >target
Expand          = perl -pe 's/ (?<!\\) \$${ ([A-Z_][A-Z_0-9]*) } / $$ENV{$$1} || ""/geix'
Install         = if [ "$(strip $2)" ]; then mkdir -p $1; pax -rwpe -s:.*/:: $2 $1; fi

install .PHONY  : $(addprefix install., bin etc include ini lib man1 man3 sbin)
install.man%    :; $(call Install,$(DESTDIR)/man/$(@:install.=),$($@))
install.%       :; $(call Install,$(DESTDIR)/$(@:install.=),$($@))

profile         : BLD := profile
profile         : test    ;@for x in $($*.test:.pass=); do gprof -b $$x >$$x.prof; done

%.test          : $(%.test)
# GMAKE trims leading "./" from $*.; $(*D)/$(*F) restores it.
%.pass          : %         ; rm -f $@; $(exec.$(BLD)) $(*D)/$(*F) >& $*.fail && mv -f $*.fail $@

# To build a .so, "make clean" first, to ensure all .o files compiled with -fPIC
%.so            : CFLAGS := -fPIC $(filter-out $(CFLAGS.cover) $(CFLAGS.profile), $(CFLAGS))
%.so            : %.o       ; $(CC) $(LDFLAGS) -o $@ -shared $< $(LDLIBS)
%.so            : %.a       ; $(CC) $(CFLAGS)  -o $@ -shared -Wl,-whole-archive $< $(LDLIBS) -Wl,-no-whole-archive
%.a             :           ; [ "$^" ] && ar crs $@ $(filter %.o,$^)
%.yy.c          : %.l       ; flex -o $@ $<
%.tab.c 	    : %.y       ; bison $<
%/..            :           ;@mkdir -p $(@D)
%               : %.gz      ; gunzip -c $^ >$@

# Ensure that intermediate files (e.g. the foo.o caused by "foo : foo.c")
#  are not auto-deleted --- causing a re-compile every second "make".
.SECONDARY  	: 

#---------------- TOOLS:
env             :;@($(foreach _,$(env),echo $_=\'$($_)\';):)|sort
# NOTE: "source" MUST be set with "=", not ":=", else MAKE recurses infinitely.
source          = $(filter-out %.d, $(shell $(MAKE) -nps all test cover profile | sed -n '/^. Not a target/{n;/^[^.*][^ ]*:/s/:.*//p;}'))

# gccdefs : all gcc internal #defines.
gccdefs         :;@$(CC) $(CPPFLAGS) -E -dM - </dev/null | cut -c8- | sort
tags            : all; ctags $(filter %.c %.cpp %.h, $(source))
# sh : invoke a shell within the makefile's env:
sh   		:; PS1='$(PS1) [make] ' $(SHELL)
# "make SomeVar." prints $(SomeVar)
%.              :;@echo '$($*)'

# %.I lists all (recursive) #included files; e.g.: "make /usr/include/errno.h.I"
%.I             : %.c       ;@ls -1 2>&- `$(CC)  $(CPPFLAGS) -M $<` ||:
%.I             : %.cpp     ;@ls -1 2>&- `$(CXX) $(CPPFLAGS) -M $<` ||:
%.i             : %.c       ; $(COMPILE.c)   -E -o $@ $<
%.i             : %.cpp     ; $(COMPILE.cpp) -E -o $@ $<
%.s             : %.c       ; $(COMPILE.c)   -S -o $@ $< && deas $@
%.s             : %.cpp     ; $(COMPILE.cpp) -S -o $@ $< && deas $@

endif
# vim: set nowrap :
