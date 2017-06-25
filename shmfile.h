#ifndef _SHM_FILE_H
#define _SHM_FILE_H
/**
 * Copyright (c) 2017 Timothy Savannah
 *   All Rights Reserved, Licensed under terms of Lesser GNU Public License version 2.1
 *
 * shmfile.h - Header for shmfile
 *
 *    https://github.com/kata198/shmfile
 *
 * Allows mapping shared-memory in an owner/guest fashion, usable through standard
 *   file I/O functions
 *
 */


/*#if defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS != 64
#error "_FILE_OFFSET_BITS must be set to 64. Currently defined to something else."
#endif

#define _FILE_OFFSET_BITS 64

#define _GNU_SOURCE
*/

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#if defined (__cplusplus)
extern "C" {
#endif


/***************************
 *  Options for fshm_flags
 ************/

#define FSHM_OWNER 1  /* FSHM_OWNER - This process will create and manage this stream. */
#define FSHM_GUEST 2  /* FSHM_GUEST - This process will inherit an existing stream */


/*******************
 * PUBLIC API
 *************/

/**
 * fshm_open - Open a shared memory and export as a STREAM, 
 *   which will work with both FILE* and int filedes functions
 *    (e.x.  fwrite and write, fread and read, etc).
 *
 *  name - A name which corrosponds to this stream.
 *
 *  mode - An octal representing the "mode", same as chmod
 *
 *  fshm_flags - A list of flags OR'd together
 *
 *     FSHM_OWNER  - Specifies that this call will create the stream
 *                    specified by #name . When the owner closes the stream,
 *                    it will be destroyed.
 *
 *     FSHM_GUEST  - Specifies that this call will open an existing stream
 *                    specified by #name . An FSHM_OWNER must have created
 *                    this stream already, or the call will fail.
 *                    Closing the stream as a guest will not destroy the file.
 *
 *  Return:
 *
 *     A FILE* object on success, otherwise NULL and errno is set.
 *
 *  NOTES:
 *
 *     The idea here is to simply allow interprocess I/O that exists
 *      only in memory. One process creates the mapping ( FSHM_OWNER )
 *      and several other processes can inherit that mapping,
 *      and share data between them.
 *
 *     There is a real FD associated with this FILE (unlike, open_memstream, for example).
 *       The same position pointers are used for the fd-functions (like read, write) as
 *       the FILE functions (like fread, fwrite).
 *
 *     No buffering occurs (because it's to memory), so read/fread act the same.
 *
 *     You can use this object anywhere a FILE* is allowed, or anywhere an fd is allowed
 *      by calling fileno(X) where X is the FILE* returned by this function.
 *
 */
FILE* fshm_open(const char* name, mode_t mode, int fshm_flags);

/**
 * fshm_force_destroy - Forcibly destroy the shared memory region associated
 *                       with #name.
 *
 *  name - The name associated with this stream
 *
 *
 *   Normal operation does not require this, however, if a process opens
 *     a fshm stream as FSHM_OWNER, and does not close it before terminating,
 *     the stream will maintain stuck open without an owner.
 *
 *   The owner should test for this, and forcibly destroy if required.
 *
 *   Like so:
 *
 *   FILE *fObj;
 *
 *   errno = 0;
 *   fObj = fshm_open("/example1", 0775, FSHM_OWNER);
 *
 *   if ( !fObj && errno == EEXIST )
 *   {
 *      fshm_force_destroy("/example1");
 *      fObj = fshm_open("/example1", 0775, FSHM_OWNER);
 *   }
 *
 *
 *  RETURN:
 *    0 on success, -1 on error.
 */
int  fshm_force_destroy(const char* name);

#if defined (__cplusplus)
} /* extern "C" */
#endif

#endif

/* vim: set ts=4 sw=4 st=4 expandtab : */
