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
#						Also installs include headers and man pages
#
#      install_static - Installs libshmfile.a into $DESTDIR/$PREFIX/lib
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


SHARED_LIB_FILES = libshmfile.so
STATIC_LIB_FILES = libshmfile.a

MAN3_FILES = man/fshm_open.3.gz

# ALL_FILES - The "all" standard build (shared lib and man page)
#   static is handled separate
ALL_FILES = ${SHARED_LIB_FILES} ${MAN3_FILES}

EXAMPLE_FILES = examples/owner examples/guest

##############
## Targets
##########

all: ${ALL_FILES}

debug: ${ALL_FILES}
	make clean; make CFLAGS="${DEBUG_CFLAGS}" LDFLAGS=""

static: ${STATIC_LIB_FILES}

clean:
	rm -f ${SHARED_LIB_FILES} ${STATIC_LIB_FILES} ${MAN3_FILES} examples/owner examples/guest shmfile.o

distclean: clean

install: ${ALL_FILES}
	# Install shared lib
	mkdir -p "${INSTALLPREFIX}/lib"
	install -m 775 libshmfile.so "${INSTALLPREFIX}/lib"
	# Install include headers
	mkdir -p "${INSTALLPREFIX}/include"
	install -m 664 shmfile.h "${INSTALLPREFIX}/include"
	# Install man pages
	mkdir -p "${INSTALLPREFIX}/share/man3"
	install -m 644 man/fshm_open.3.gz "${INSTALLPREFIX}/share/man/man3"
	# Symlink alternate names for fshm_open.3.gz man page
	ln -sf fshm_open.3.gz fshm_create.3.gz
	ln -sf fshm_open.3.gz fshm_guest_open.3.gz
	ln -sf fshm_open.3.gz fshm_force_destroy.3.gz
	mv fshm_create.3.gz fshm_guest_open.3.gz fshm_force_destroy.3.gz "${INSTALLPREFIX}/share/man/man3"


install_static: ${STATIC_LIB_FILES}
	mkdir -p "${INSTALLPREFIX}/lib"
	install -m 644 libshmfile.a "${INSTALLPREFIX}/lib"

examples: ${SHARED_LIB_FILES} ${EXAMPLE_FILES}

examples_debug: ${SHARED_LIB_FILES} ${EXAMPLE_FILES}
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


man/fshm_open.3.gz: man/fshm_open.3
	cat man/fshm_open.3 | gzip -c > man/fshm_open.3.gz

# vim: set ts=4 sw=4 st=4 noexpandtab:
