# shmfile
Small shared library to use shared memory as a FILE\* , with interprocess communication in mind.



Basic Design
============

The basic design is that a process is in one of two "groups":

* FSHM\_OWNER - This process creates the stream, and when it closes the stream, the stream will be destroyed.

* FSHM\_GUEST - This process "connects" to an existing stream.


The "stream" is associated by an arbitrary FS-like name, like "/streams/my\_stream"

The stream also has an associated mode, which is an octal following standard unix mode rules  ( think chmod )


Public Functions
----------------


**fshm\_open** - Opens a stream


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


**fshm\_force\_destroy** - Forcibly destroys a stream, as a recovery tool.


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
