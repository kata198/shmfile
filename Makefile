
.PHONY: all clean

_LIB_PATH = /home/media/projects/shmfile

CFLAGS ?= -O2 -s

USE_CFLAGS = ${CFLAGS} -Wall -std=gnu99 

USE_CFLAGS_EXEC = ${USE_CFLAGS} -lrt -L "${_LIB_PATH}" -Wl,-rpath="${_LIB_PATH}" -lshmfile

USE_CFLAGS_LIB = ${USE_CFLAGS} -fPIC -lrt -shared


all: libshmfile.so owner guest

clean:
	rm -f libshmfile.so owner guest


libshmfile.so: shmfile.c shmfile.h
	gcc shmfile.c ${USE_CFLAGS_LIB} -o libshmfile.so

owner: owner.c shmfile.h
	gcc owner.c ${USE_CFLAGS_EXEC} -o owner

guest: guest.c shmfile.h
	gcc guest.c ${USE_CFLAGS_EXEC} -o guest

