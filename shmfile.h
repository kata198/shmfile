#ifndef _SHM_FILE_H
#define _SHM_FILE_H
/**
 * Copyright (c) 2017, 2018 Timothy Savannah
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
 **************************/

#define FSHM_OWNER 1  /* FSHM_OWNER - This process will create and manage this stream. */
#define FSHM_GUEST 2  /* FSHM_GUEST - This process will inherit an existing stream */


/*******************
 * PUBLIC API
 *************/

/**
 * fshm_create - Create a new shmfile (as FSHM_OWNER)
 *                 with a given mode.
 *
 *
 *  name - A unique name which corrosponds to this stream.
 *          Guests will use this name as a reference to map
 *           this shmfile stream.
 *
 *  mode - An octal representing the "mode" for guest mappings.
 *           Mode has same meaning as with chmod.
 *         This mode will be used to determine how FSHM_GUEST
 *           mappings will be able to access this stream.
 *
 *         See the 'fshm_open' function for more information
 *          on mode.
 *
 *
 *  RETURN VALUE - 
 *                 A pointer to a FILE structure which may be used
 *                  with standard io functions (e.x. fread, fprintf, fseek)
 *
 *                 On error, this function will return NULL and `errno' will be set
 *
 *  NOTES -
 *                 * This is the same as calling fshm_open(name, mode, FSHM_OWNER)
 *
 *                 * See fshm_open for more info
 */
FILE* fshm_create(const char *name, mode_t mode);


/**
 * fshm_guest_open - Maps an existing shmfile stream (as FSHM_GUEST)
 *                    using the given name for reference.
 *
 *
 *  name - The name associated with an already-created fshmfile stream
 *
 *
 *  RETURN VALUE -
 *                 A pointer to a FILE structure which may be used
 *                  with standard io functions (e.x. fread, fprintf, fseek)
 *
 *                 On error, this function will return NULL and `errno' will be set
 *
 *  NOTES -
 *                 * This is the same as calling fshm_open(name, 0, FSHM_GUEST)
 *
 *                 * See fshm_open for more info
 */
FILE *fshm_guest_open(const char *name);


/**
 * fshm_open - Open a shared memory and export as a STREAM,
 *   which will work with both FILE* and int filedes functions
 *    (e.x.  fwrite and write, fread and read, etc).
 *
 *  name - A unique name which corrosponds to this stream.
 *            If flags contain FSHM_OWNER, we attempt to create
 *             a stream using this name.
 *            Guests who wish to map this shmfile will provide
 *             this same name to reference which shmfile stream
 *             to map.
 *
 *  mode - An octal representing the "mode" for guest mappings.
 *           Mode has same meaning as with chmod.
 *         This mode will be used to determine how FSHM_GUEST
 *           mappings will be able to access this stream.
 *
 *         This field only has meaning when creating the shmfile ( FSHM_OWNER ).
 *          For an FSHM_GUEST open of an existing shmfile stream, this can be
 *            set to 0.
 *
 *         Columns:
 *
 *          - First column from left is always 0 for now (no sticky/setguid bit meaning)
 *          - Second column from left is permissions for owner (user who creates shmfile)
 *          - Third column from the left is permissions for users who share primary
 *            group with creating user
 *          - Fourth column from the left are permissions for any user which
 *            does NOT fall into any categories listed above
 *
 *         Values:
 *
 *          For each column, the number is derived by starting with 0
 *            and following these rules:
 *
 *           - Add 4 to allow guest mapping and read for represented set
 *           - Add 2 to allow writing to the mapping
 *           - Add 1 to allow execution
 *
 *
 *  Examples - Some examples of various #mode[s] and their meaning
 *
 *        Allow current user only to map as guest, read/write:
 *           0600
 *
 *        Allow current user only to map as guest, read-only:
 *           0400
 *
 *        Allow current user to map as guest, read and write,
 *          allow a different user belonging to same primary group
 *          to map as guest read-only:
 *           0640
 *
 *        Allow anybody on system to map read/write:
 *           0777
 *
 *
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
 *
 *     You may choose to use either the buffered variants ( like fwrite, fread),
 *       or the unbuffered variants (like write, read).
 *
 *     Keep in mind that if you use the buffered functions, you must call fflush to
 *       sync the changes.
 *
 *
 *     You can use this object anywhere a FILE* is allowed, or anywhere an fd is allowed
 *      by calling fileno(X) where X is the FILE* returned by this function.
 *
 */
FILE* fshm_open(const char *name, mode_t mode, int fshm_flags);


/**
 * fshm_chgrp - Change the group that owns this shmfile.
 *
 *      You must be the the same user as the FSHM_OWNER of the given stream,
 *        or root in order to change the gid assigned to the shmfile.
 *
 *      The shmfile created by fshm_open is assigned the gid
 *        matching the primary group of the creating user.
 *
 *      The purpose is to allow guests in a different group access rights to
 *        this shmfile ( based on the group bits in the mode set with fshm_open ),
 *        but not just open up access for everybody ( using the "other" bits in mode ).
 *
 *      As non-root, you may change the group to any groups to which you are a member.
 *        This includes your primary gid as well as all supplementry groups to which you belong.
 *        As root, you may change the shmfile's group to any group existing on this system.
 *
 *
 *  fshm_file - A FILE* object as returned by fshm_open.
 *                 You must be the FSHM_OWNER of this shmfile to change permissions.
 *
 *
 *  group     - new gid to replace current group on this shmfile.
 *
 *                  If you are not root, this must be either your primary gid
 *                    or a group contained in your list of supplementry groups.
 *
 *
 *  RETURN VALUE -
 *                   0:  Success
 *                  -1:  Failure (and errno will be set)
 */
int fshm_chgrp(FILE *fshm_file, gid_t group);


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
int  fshm_force_destroy(const char *name);

/**
 * shmfile_get_version - Get the version info on this version of shmfile
 *
 *    Sets the value of *major, *minor, *patchlevel, and *extra to the numeric version numbers
 */
void shmfile_get_version(char *major, char *minor, char *patchlevel, const char **extra);


#if defined (__cplusplus)
} /* extern "C" */
#endif

#endif

/* vim: set ts=4 sw=4 st=4 expandtab : */
