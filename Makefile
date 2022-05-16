.MAKEFLAGS: -r -m share/mk

# targets
all::  mkdir .WAIT dep .WAIT prog
dep::
gen::
prog::
test:: all
clean::

# things to override
CC     ?= cc
BUILD  ?= build
PREFIX ?= /usr/local

# ${unix} is an arbitrary variable set by sys.mk
.if defined(unix)
.BEGIN::
	@echo "We don't use sys.mk; run ${MAKE} with -r" >&2
	@false
.endif

# layout
SUBDIR += tests/c89-common
SUBDIR += tests/c89-hosted
SUBDIR += tests/c89-library
SUBDIR += tests/c89-regression
SUBDIR += tests/c89-fuzz
SUBDIR += tests/posix.1-2008
#SUBDIR += tests/tdf

#INCDIR += include

.include <subdir.mk>

.for test in ${TEST}

test::
	${BUILD}/bin/${test:R}

PROG += ${test:R}
SRC  += tests/${test}
DIR  += ${BUILD}/bin/${test:H}

${BUILD}/bin/${test:R}: ${BUILD}/tests/${test:R}.o

.endfor

.include <obj.mk>
.include <dep.mk>
.include <ar.mk>
.include <so.mk>
.include <prog.mk>
.include <mkdir.mk>
.include <install.mk>
.include <clean.mk>

