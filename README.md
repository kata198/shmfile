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


The mode specified by guest mappings will be attempted, but if created with lower permissions what the owner set will be used.


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


**fshm\_open** - Opens a stream

FILE\* fshm\_open(const char\* name, mode\_t mode, int fshm\_flags);


This is the "core" function, as fshm allows you to use all the other I/O systems that come standard with libc.


	/**
	 * fshm_open - Open a shared memory and export as a STREAM,
	 *   which will work with both FILE* and int filedes functions
	 *    (e.x.  fwrite and write, fread and read, etc).
	 *
	 *  name - A name which corrosponds to this stream.
	 *
	 *
	 *  mode - An octal representing the "mode" for guest mappings.
	 *           Mode has same meaning as with chmod.
	 *         This mode will be used to determine how FSHM_GUEST
	 *           mappings will be able to access this stream.
	 *
	 *
	 *         A guest may request any mode, but if the requested mode
	 *           exceeds what the owner set, the guest's permissions
	 *           will not be truncated to match the owner's maximum mode.
	 *
	 *
	 *      Examples:
	 *
	 *        Allow current user only to map as guest, read/write:
	 *          0600
	 *
	 *        Allow current user only to map as guest, read-only:
	 *          0400
	 *
	 *        Allow anybody on system to map read/write:
	 *          0777
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

	FILE* fshm_open(const char* name, mode_t mode, int fshm_flags);


**fshm\_force\_destroy** - Forcibly destroys a stream, as a recovery tool.

int  fshm\_force\_destroy(const char\* name);

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


Examples
--------

owner ( from owner.c )  is an example that creates a stream ( as FSHM\_OWNER ), writes some data to it, and leaves it open for a period of time.

guest ( from guest.c )  is an example that connects to an existing stream ( as FSHM\_GUEST ), reads data from it, and prints it to stdout.


These examples use a struct which is given initial values by "owner", and is updated each time "guest" is run.

The examples show both buffered I/O (fwrite and fflush), as well as unbuffered I/O (read and write).

