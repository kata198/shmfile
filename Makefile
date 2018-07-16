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

EXAMPLES_ADDITIONAL_FLAGS = -I "${_LIB_PATH}" -Wl,-rpath="${_LIB_PATH}"

INSTALLPREFIX = $(shell echo "${DESTDIR}/${PREFIX}" | sed 's|//|/|g')


##############
## Targets
##########

all: libshmfile.so

debug: libshmfile.so
	make clean; make CFLAGS="${DEBUG_CFLAGS}" LDFLAGS=""

static: libshmfile.a

clean:
	rm -f libshmfile.so examples/owner examples/guest libshmfile.a

distclean: clean

install: libshmfile.so
	mkdir -p "${INSTALLPREFIX}/lib"
	install -m 775 libshmfile.so "${INSTALLPREFIX}/lib"
	mkdir -p "${INSTALLPREFIX}/include"
	install -m 664 shmfile.h "${INSTALLPREFIX}/include"

install_static: libshmfile.a
	mkdir -p "${INSTALLPREFIX}/lib"
	install -m 644 libshmfile.a "${INSTALLPREFIX}/lib"

examples: libshmfile.so examples/owner examples/guest

examples_debug: libshmfile.so examples/owner examples/guest
	make clean; make CFLAGS="${DEBUG_CFLAGS}" LDFLAGS="" examples

libshmfile.a: shmfile.o
	ar rcs libshmfile.a shmfile.o

libshmfile.so: shmfile.o
	gcc shmfile.o ${USE_CFLAGS_LIB} -shared -o libshmfile.so

shmfile.o: shmfile.c shmfile.h
	gcc shmfile.c ${USE_CFLAGS_LIB} -c -o shmfile.o

examples/owner: examples/owner.c examples/owner_guest_private.h shmfile.h 
	gcc examples/owner.c ${USE_CFLAGS_EXEC} ${EXAMPLES_ADDITIONAL_FLAGS} -o examples/owner

examples/guest: examples/guest.c examples/owner_guest_private.h shmfile.h 
	gcc examples/guest.c ${USE_CFLAGS_EXEC} ${EXAMPLES_ADDITIONAL_FLAGS} -o examples/guest

# vim: set ts=4 sw=4 st=4 noexpandtab:
