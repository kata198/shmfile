#############################################
##  Makefile for shmfile
#
#
#    TARGETS:
#
#      all (default) - Builds libshmfile.so, and the example programs: owner and guest
#
#      debug         - Build libshmfile.so with debug flags
#
#      static        - Build libshmfile.so as a static lib
#
#      install       - Installs libshmfile.so into $DESTDIR/$PREFIX/lib . So default is /usr/lib,
#                        to install to /usr/local instead, do:      make install PREFIX="/usr/local"
#
#                      To install to a package, do:      make install DESTDIR="${pkgdir}"
#
#      clean         - Remove all compiled stuff
#
#      distclean     - Return to a completely fresh state
#
#    INFLUENIAL VARIABLES:
#
#      CFLAGS  - Defines the CFLAGS to be used for compiling
#
#      LDFLAGS - Appended to CFLAGS for compiling.
#
#      DESTDIR - For installing, this should either be blank, or an alternate root (like $RPM_BUILD_ROOT or $pkgdir)
#
#      PREFIX  - The prefix, default "/usr".  Maybe you want "/usr/local",  or "$HOME" for a local install.
#
#############################################


.PHONY: all install clean distclean debug static

# _LIB_PATH - Used as a hack so owner/guest programs work without doing an install
_LIB_PATH = $(shell pwd)


##############
## Variables
###########

CFLAGS ?= -O2 -s

LDFLAGS ?= -Wl,-O1,--sort-common

DEBUG_CFLAGS = -Og -ggdb3

USE_CFLAGS = ${CFLAGS} -Wall -std=gnu99 ${LDFLAGS}

USE_CFLAGS_EXEC = ${USE_CFLAGS} -L "${_LIB_PATH}" -Wl,-rpath="${_LIB_PATH}" -lshmfile

USE_CFLAGS_LIB = ${USE_CFLAGS} -fPIC -lrt -shared

DESTDIR ?=

PREFIX ?= /usr


INSTALLPREFIX = $(shell echo "${DESTDIR}/${PREFIX}" | sed 's|//|/|g')


##############
## Targets
##########

all: libshmfile.so owner guest

debug: libshmfile.so owner guest
	make clean; make CFLAGS="${DEBUG_CFLAGS}" LDFLAGS=""

static: libshmfile.a

clean:
	rm -f libshmfile.so owner guest libshmfile.a

distclean: clean

install: libshmfile.so
	mkdir -p "${INSTALLPREFIX}/lib"
	install -m 775 libshmfile.so "${INSTALLPREFIX}/lib"
	mkdir -p "${INSTALLPREFIX}/include"
	install -m 664 shmfile.h "${INSTALLPREFIX}/include"

libshmfile.a: shmfile.c shmfile.h
	gcc shmfile.c ${USE_CFLAGS_LIB} -shared -o libshmfile.a

libshmfile.so: shmfile.c shmfile.h
	gcc shmfile.c ${USE_CFLAGS_LIB} -o libshmfile.so

owner: owner.c shmfile.h owner_guest_private.h
	gcc owner.c ${USE_CFLAGS_EXEC} -o owner

guest: guest.c shmfile.h owner_guest_private.h
	gcc guest.c ${USE_CFLAGS_EXEC} -o guest

