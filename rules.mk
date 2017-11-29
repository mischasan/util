# Environment:
#   $BLD        : empty, or one of:   cover  debug  profile
#   CFLAGS_     : final-override member of CFLAGS
# Makefile vars:
#   all   += <module>
#   clean += <pathnames>
#   tags  += <external source files>
# NOTE: Simba uses gmake (only) on non-WINDOWS platforms (only) i.e. Unix.
#   So rules.mk can be Unix-"specific", while

#YYY Simba Makefile_FLAGS.mak refers to ${DMFLAGS} and ${EVALFLAGS}

ifndef RULES_MK
RULES_MK := 1

export LD_LIBRARY_PATH
# HP-UX gmake does not grok "!="
OS              = $(shell uname -s)
PS4             = \# # Prefix for "sh -x" output.
SHELL           = bash
SPACE           = $` # " "

# Import from PREFIX, export to DESTDIR.
PREFIX          ?= /usr/local
DESTDIR         ?= ${PREFIX}

# HACK CentOS 5 comes with gcc 4.1, gcc 4.4 is /usr/lib/gcc44
CC              = gcc

# Theses are all gcc-specific; change them to (e.g.) CFLAGS.gcc.debug
#XXX: xlc, SunOS/cc, HP-UX/cc are

#--- *.${BLD}:
# -O level > 2 makes gcc 4.2 go weird on MacOSX
# For gcc 4.5+, use: -O3 -flto --- link-time optimization including inlining.

CFLAGS.         = -O3
CFLAGS.cover    = --coverage -DNDEBUG
LDFLAGS.cover   = --coverage
CFLAGS.debug    = -O0 -Wno-uninitialized
CPPFLAGS.debug  = -UNDEBUG
CFLAGS.profile  = -pg -DNDEBUG
LDFLAGS.profile = -pg

# PROFILE tests get stats on syscalls appended to their .pass files.
#   Darwin spells strace "dtruss".
exec.profile    = $(shell which dtruss strace) -cf

#--- *.${OS}:
#XXX HP-UX gcc 4.2.3 does not grok -fstack-protector. Sigh.
#CFLAGS.AIX      = -fstack-protector --param ssp-buffer-size=4
CFLAGS.Darwin   = ${CFLAGS.AIX}
CFLAGS.HP-UX    = 
CFLAGS.Linux    = ${CFLAGS.AIX}
CFLAGS.SunOS    = ${CFLAGS.AIX}

#NOTE: defining SIZEOF_LONG_INT suppresses inclusion of UNIXODBC files in Simba ThirdParty/odbcheaders/*.h . Otherwise it is just silly.
CPPFLAGS        += -DSIZEOF_LONG_INT=8

LDLIBS.FreeBSD  = -lm
LDLIBS.Linux    = -lm

# -fPIC allows all .o's to be built into .so's.
CFLAGS          += -g -MMD -fPIC -fdiagnostics-show-option -fno-strict-aliasing

# We have code that throws warnings all over :-(
#CFLAGS         += -Wall -Werror -Wextra -Wcast-align -Wcast-qual -Wformat=2 -Wformat-security -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wunused -Wwrite-strings
CFLAGS          += -Wall         -Wextra -Wcast-align -Wcast-qual -Wformat=2 -Wformat-security -Wmissing-prototypes -Wnested-externs -Wpointer-arith                   -Wshadow -Wstrict-prototypes -Wunused -Wwrite-strings

CFLAGS          += -Wno-attributes -Wno-cast-qual -Wno-unknown-pragmas -Wno-unused-parameter
CFLAGS          += ${CFLAGS_} ${CFLAGS.${BLD}} ${CFLAGS.${OS}} ${CFLAGS.${BLD}.${OS}} ${_CFLAGS}

#CPPFLAGS        += -I${PREFIX}/include -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE ${CPPFLAGS.${BLD}} ${CPPFLAGS.${OS}}
CPPFLAGS        += -I${PREFIX}/include -D_GNU_SOURCE ${CPPFLAGS.${BLD}} ${CPPFLAGS.${OS}}
CXXFLAGS        += $(filter-out -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes, ${CFLAGS})

LDFLAGS         += -L${PREFIX}/lib ${LDFLAGS.${BLD}} ${LDFLAGS.${OS}}
LDLIBS          += ${LDLIBS.${OS}} -lstdc++
#LDLIBS.Linux    += /usr/lib/libstdc++.so.6
#LDLIBS.Darwin   += -lstdc++.6

#---------------- Explicitly CANCEL THESE EVIL BUILTIN RULES:
%               : %.c
%               : %.cpp
%.c             : %.l
%.c             : %.y
%.r             : %.l
#----------------
.PHONY          : all clean cover debug env gccdefs install profile sh source tags test
.DEFAULT_GOAL   := all

# ${all} contains subproject names. It can be used in ACTIONS but not RULES,
#   since it accumulates across every "include <submakefile>"
# ${junkfiles} is how to get metachars (commas) through the ${addsuffix...} call.
SPACE := #
COMMA := ,
junkfiles       = {gmon.out,tags,*.[di],*.fail,*.gcda,*.gcno,*.gcov,*.prof}

all             :;@echo "$@ done for BLD='${BLD}'"
clean           :;@( ${MAKE} -nps all install test | sed -n '/^# I/,$${/^[^\#\[%.][^ %]*: /s/:.*//p;}'; \
                    echo $(addsuffix /${junkfiles}, $(foreach d,${all},${$d})) ${clean} $(filter %.d,${MAKEFILE_LIST}) ) | xargs rm -rf
# using all=dirs not all=names : {$(subst ${SPACE},${COMMA},$(strip ${all}))}/${junkfiles} ${clean} $(filter %.d,${MAKEFILE_LIST}) ) | xargs rm -rf


cover           : BLD := cover
%.cover         : %.test    ; gcov -bcp ${$@} | covsum

# Expand: translate every occurrence of "${var}" in a file to its env value (or ""):
#   E.G.   ${Expand} foo.tmpl >foo.ini
Expand          = perl -pe 's/ (?<!\\) \$${ ([A-Z_][A-Z_0-9]*) } / $$ENV{$$1} || ""/geix'
Install         = $(if $2, mkdir -p $1; pax -rwpe -s:.*/:: $2 $1)

# install.% cannot be .PHONY because there is no pattern expansion of phony targets.
install         : $(addprefix install., bin etc ini man1 man3 sbin) \
                            ; $(call Install,${DESTDIR}/include,$(filter %.h,$^) $(filter %.hpp,$^)) \
                            ; $(call Install,${DESTDIR}/lib,$(filter %.a,$^) $(filter %.so,$^) $(filter %.dylib,$^))

install.man%    :           ; $(call Install,${DESTDIR}/man/$(subst install.,,$@),$^)
install.%       :           ; $(call Install,${DESTDIR}/$(subst install.,,$@),$^)

profile         : BLD := profile
profile         : test      ;@for x in ${$*.test:.pass=}; do gprof -b $$x >$$x.prof; done

%.test          : ${%.test}

# GMAKE trims leading "./" from $* ; ${*D}/${*F} restores it, so no need to fiddle with $PATH.
%.pass          : %         ; rm -f $@; ${exec.${BLD}} ${*D}/${*F} >& $*.fail && mv -f $*.fail $@

%.so            : %.o       ; ${CC} ${LDFLAGS} -o $@ -shared $< ${LDLIBS}
%.so            : %.a       ; ${CC} ${CFLAGS}  -o $@ -shared -Wl,-whole-archive $< ${LDLIBS} -Wl,-no-whole-archive
%.a             :           ; $(if $(filter %.o,$^), ar crs $@ $(filter %.o,$^))
%.yy.c          : %.l       ; flex -o $@ $<
%.tab.c         : %.y       ; bison $<
%/..            :           ;@mkdir -p ${@D}
%               : %.gz      ; gunzip -c $^ >$@

# Ensure that intermediate files (e.g. the foo.o caused by "foo : foo.c")
#  are not auto-deleted --- causing a re-compile every second "make".
.SECONDARY      :

#---------------- Unix tools: should be converted into scripts.
# defs - list gcc's builtin macros
defs            :;@${CC} ${CPPFLAGS} -E -dM - </dev/null | cut -c8- | sort

# env - environment relevant to the make. "sort -u" because env entries may not be unique -- "env +=" in multiple makefiles.
env             :;@($(foreach _,${env},echo $_=${$_};):) | sort -u

# sh - invoke a shell within the makefile's env:
sh              :;@PS1='${PS1} [make] ' ${SHELL}

# source - list files used and not built by the "make". Explicitly filters out "*.d" files.
#TODO: filter out .PHONY targets properly. /^install/d is a hack.
source          :;$(if $^, ls$^;) ${MAKE} -nps all test cover profile | sed -n '/^. Not a target/{ n; /^install/d; /^[^ ]*\.d:/!{ /^[^.*][^ ]*:/s/:.*//p; }; }' | sort -u

# NOTE: "make tags" BEFORE "make all" is incomplete because *.h dependencies are only in *.d files.
tags            :; ctags $(filter %.c %.cpp %.h, ${source})

# "make SomeVar." prints ${SomeVar}
%.              :;@echo '${$*}'

# %.I lists all (recursive) #included files; e.g.: "make /usr/include/errno.h.I"
%.I             : %.c       ;@ls -1 2>&- `${CC}  ${CPPFLAGS} -M $<` | sort -u
%.I             : %.cpp     ;@ls -1 2>&- `${CXX} ${CPPFLAGS} -M $<` | sort -u
%.i             : %.c       ; ${COMPILE.c}   -E -o $@ $<
%.i             : %.cpp     ; ${COMPILE.cpp} -E -o $@ $<
%.s             : %.c       ; $(filter-out -Werror,${COMPILE.c}) -DNDEBUG -S -o $@ $< && deas $@
%.s             : %.cpp     ; ${COMPILE.cpp} -S -o $@ $< && deas $@

endif
# vim: set nowrap :
