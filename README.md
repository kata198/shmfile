# shmfile
Small shared library to use shared memory as a FILE\* , with interprocess communication in mind.



Basic Design
============

**Overview**

shmfile is desgined in an "owner/guest" format.

One process is the *owner* [FSHM\_OWMER] of a shared mapping (given by a string name). It creates the mapping, and can destroy it.


Other processes then can map that same string name as *guest* [FSHM\_GUEST].


The guest only needs to know the unique name that the owner picked to have access to the data.


**Permissions**

Permissions for who can map the same shm as FSHM\_GUEST are specified by the octal: "mode".

These follow the same rules as chmod. For example, same user only and read-only would be 0400. Same user r/w, all other users r/o, 0644


You specify the mode when the *owner* creates the shmfile, which will later determine whomelse on the system may have access.


The guest opening the already-created shmfile ( using FSHM\_GUEST ) may specify different permissions than those which the FSHM\_OWNER set on the shmfile, but they cannot exceed the owner's permissions. If such is attempted, for example if owner sets mode to 0644 and guest tries to open the stream with 0666, the guest's mode will be forced to 0644.


**Usage**


The *fshm\_open* function returns a FILE\* object, with a valid integer file descriptor [ fd ].

You can read/write the data using either the buffered I/O functions (like fread, fwrite, fseek) and flushing with fflush, OR

the unbuffered I/O functions (like read, write, lseek).


The size is completely automatic, you do not need to allocate an underlying buffer -- any data written to it and flushed will
expand the buffer automatically.


There is no built-in locking, so it's suggested if you are going to have multiple processes writing to the stream, that you either use fixed-size sections for each process,

or implement your own locking.


Public Functions
----------------

The following functions make up the public API of shmfile.


**fshm\_create** - Create a new shmfile stream (convenience/helper function for fshm\_open with fshm\_flags as FSHM\_OWNER)

FILE\* fshm\_create(const char \*name, mode\_t mode)

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


**fshm\_guest\_open** - Opens an existing shmfile stream (convenience/helper function for fshm\_open with fshm\_flags as FSHM\_GUEST)


FILE \*fshm\_guest\_open(const char \*name);

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

**fshm\_open** - Opens a stream

FILE\* fshm\_open(const char \*name, mode\_t mode, int fshm\_flags);


This is the "core" function, as fshm allows you to use all the other I/O systems that come standard with libc.


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

**fshm\_chgrp** - Change the group "owner" of a shmfile stream

int fshm\_chgrp(FILE \*fshm\_file, gid\_t group)

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


**fshm\_force\_destroy** - Forcibly destroys a stream, as a recovery tool.

int  fshm\_force\_destroy(const char \*name);

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


**shmfile\_get\_version** - Gets version info on libshmfile

void shmfile\_get\_version(char \*major, char \*minor, char \*patchlevel, const char \*\*extra);

	/**
	 * shmfile_get_version - Get the version info on this version of shmfile
	 *
	 *    Sets the value of *major, *minor, *patchlevel, and *extra to the numeric version numbers
	 */


**Defined Constants**

*FSHM\_OWNER* - Used as a flag with fshm\_open to denote that the current process and user

will create and own the shmfile stream.

*FSHM\_GUEST* - Used as a flag with fshm\_open to denote that we want to map an existing

shmfile stream.



Examples
--------

examples/owner ( from examples/owner.c )  is an example that creates a stream ( as FSHM\_OWNER ),
 writes some data to it, and leaves it open for a period of time.

examples/guest ( from examples/guest.c )  is an example that connects to an existing stream ( as FSHM\_GUEST ),
 reads data from it, and prints it to stdout.


These examples test useing a struct as data which is given initial values by "owner", and is updated each time "guest" is run.

The examples show both buffered I/O (fwrite and fflush), as well as unbuffered I/O (read and write).

Optionally, the fshm\_chgrp method will be used if you uncomment and set the SET\_GID\_MACRO at the top of examples/owner.c


