/**
 * Copyright (c) 2017 Timothy Savannah
 *   All Rights Reserved, Licensed under terms of Lesser GNU Public License version 2.1
 *
 * shmfile.c - Definitions and private attributes for shmfile
 *
 */


#define _GNU_SOURCE

#include "shmfile.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef __GNUC__

#define __visibility_hidden  __attribute__( (visibility ("hidden")) )

#else

#define __visibility_hidden

#endif


#ifndef FSHM_QUIET

#define printerr(_x, ...) fprintf(stderr, _x)

#else

#define printerr(_x, ...)

#endif

#define FSHM_COOKIE(_cookie) ( (struct fshm_cookie*) _cookie)


#define IS_FSHM_OWNER(_cookie) !!( FSHM_COOKIE(_cookie)->fshm_flags & FSHM_OWNER )
#define IS_FSHM_GUEST(_cookie) !!( FSHM_COOKIE(_cookie)->fshm_flags & FSHM_GUEST )

#define IS_FSHM_FLAGS_OWNER(_fshm_flags) !!( _fshm_flags & FSHM_OWNER )
#define IS_FSHM_FLAGS_GUEST(_fshm_flags) !!( _fshm_flags & FSHM_GUEST )

__visibility_hidden struct fshm_cookie {
    
    int fd;
    off_t pos;
    char *name;
    int fshm_flags;

};

__visibility_hidden ssize_t _fshm_read(void *cookie, char *buf, size_t size)
{
    struct fshm_cookie *fshmCookie;
    ssize_t ret;

    fshmCookie = FSHM_COOKIE(cookie);

    ret = read(fshmCookie->fd, buf, size);

    return ret;
}

__visibility_hidden ssize_t _fshm_write(void *cookie, const char *buf, size_t size)
{
    struct fshm_cookie *fshmCookie;
    ssize_t ret;

    fshmCookie = FSHM_COOKIE(cookie);

    ret = write(fshmCookie->fd, buf, size);

    return ret;
}

__visibility_hidden int _fshm_seek(void *cookie, off_t *offset, int whence)
{
    struct fshm_cookie *fshmCookie;
    int ret;
    off_t newOffset;

    fshmCookie = FSHM_COOKIE(cookie);

    newOffset = lseek(fshmCookie->fd, *offset, whence);

    if ( newOffset >= 0 )
    {
        *offset = newOffset;
        ret = 0;
    }
    else
    {
        ret = -1;
    }

    return ret;
}

__visibility_hidden int _fshm_close(void *cookie)
{
    struct fshm_cookie *fshmCookie;
    int ret = 0;

    fshmCookie = FSHM_COOKIE(cookie);

    if ( IS_FSHM_OWNER(fshmCookie) )
    {
        ret = shm_unlink(fshmCookie->name);
    }

    free(fshmCookie->name);
    free(fshmCookie);

    return ret;
}

FILE* fshm_open(const char* name, mode_t mode, int fshm_flags)
{
    int fd;
    int oflag;

    char *nameCpy;
    size_t nameLen;
    const char* modeStr;

    struct fshm_cookie *cookie;
    cookie_io_functions_t fshmIoFuncs;

    FILE *fObj;


    if ( ! ( fshm_flags & (FSHM_OWNER | FSHM_GUEST) ) )
    {
        printerr("Error: fshm_open called but flags specified neither FSHM_OWNER nor FSHM_GUEST.\n");
        errno = EINVAL;
        /* TODO: Errno? */
        return NULL;
    }

    if ( !name || name[0] == '\0' )
    {
        printerr("Error: fshm_open called with NULL/empty \"name\" parameter.\n");
        errno = EINVAL;
        /* TODO: Errno? */
        return NULL;
    }

    oflag = O_RDWR;

    if ( fshm_flags & FSHM_OWNER )
    {
        if ( fshm_flags & FSHM_GUEST )
        {
            printerr("Error: fshm_open called with both FSHM_OWNER and FSHM_GUEST\n");
            errno = EINVAL;
            return NULL;
        }
        oflag |= O_CREAT;
        oflag |= O_EXCL;
    }

    fd = shm_open(name, oflag, mode);

    if ( fd < 0 )
        return NULL;

    cookie = malloc( sizeof(struct fshm_cookie) );
    if ( !cookie )
    {
        errno = ENOMEM;
        return NULL;
    }

    nameLen = strlen(name);
    nameCpy = malloc(nameLen + 1);

    strcpy(nameCpy, name);

    cookie->pos = 0;
    cookie->fshm_flags = fshm_flags;
    cookie->fd = fd;
    cookie->name = nameCpy;
    
    fshmIoFuncs.read = _fshm_read;
    fshmIoFuncs.write = _fshm_write;
    fshmIoFuncs.seek = _fshm_seek;
    fshmIoFuncs.close = _fshm_close;


    if ( fshm_flags & FSHM_OWNER )
    {
        modeStr = "w+";
    }
    else
    {
        modeStr = "r+";
    }

    fObj = fopencookie(cookie, modeStr, fshmIoFuncs);
    if ( !fObj )
    {
        _fshm_close(cookie);
        return NULL;
    }

    /* Set the _fileno field so fileno() and then non-f* functions work  */
    fObj->_fileno = fd;

    return fObj;
}

int fshm_force_destroy(const char* name)
{
    return shm_unlink(name);
}

/* vim: set ts=4 sw=4 st=4 expandtab : */