/**
 * Copyright (c) 2017 Timothy Savannah
 *   All Rights Reserved, Licensed under terms of Lesser GNU Public License version 2.1
 *
 * owner.c - Example of an FSHM_OWNER - 
 *   Creates an fshm mapping and writes data to it.
 *
 *   "guest" is the corrosponding FSHM_GUEST
 *
 *
 * Takes up to two arguments:
 *  
 *  First, if provided, is the stream name. Defaults to DEFAULT_STREAM_NAME below.
 *
 *  Second is number of seconds to sleep before destroying stream. During this sleep time
 *    guests will be able to access the written data.
 *
 */

#include "shmfile.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "owner_guest_private.h"

#define DEFAULT_SLEEP_TIME 10

/*
  SET_GID_TO - If defined, will change the group of the shmfile mapping
                   to the provided group id
*/
/*
#define SET_GID_TO 1003
*/


int main(int argc, char* argv[])
{
    FILE *fObj;

    char *streamName;
    int sleepTime;
    unsigned char vmajor, vminor, vpatchlevel;
    const char* vextra;

    OwnerGuestData data = { 0 };

    shmfile_get_version(&vmajor, &vminor, &vpatchlevel, &vextra);

    printf("shmfile version: %hhu.%hhu.%hhu%s\n", vmajor, vminor, vpatchlevel, vextra);

    if ( argc == 1 )
    {
        sleepTime = DEFAULT_SLEEP_TIME;
        streamName = DEFAULT_STREAM_NAME;
    }
    else if ( argc == 2 )
    {
        sleepTime = DEFAULT_SLEEP_TIME;
        streamName = argv[1];
    }
    else if ( argc == 3 )
    {
        sleepTime = atoi(argv[2]);
        streamName = argv[1];
    }
    else
    {
        fprintf(stderr, "Wrong number of arguments.\n\nUsage: owner (optional: [stream name] [sleep time])\n");
        return EINVAL;
    }

    printf("Trying to create fshm stream at:  \"%s\"\n\n", streamName);

    errno = 0;
    /*fObj = fshm_open(streamName, 0770, FSHM_OWNER);*/
    fObj = fshm_create(streamName, 0770);
    if (!fObj)
    {
        if ( errno == EEXIST )
        {
            fprintf(stderr, "-- Existing stream, forcing close.\n\n");
            fshm_force_destroy(streamName);
            fObj = fshm_open(streamName, 0770, FSHM_OWNER);
        }
        else
        {
            goto handle_fobj_null;
        }
    }

    if (!fObj) {
handle_fobj_null:
        fprintf(stderr, "Error! %d: %s\n", errno, strerror(errno));
        return 1;
    }

    #ifdef SET_GID_TO
      char *strError;
      if ( fshm_chgrp(fObj, SET_GID_TO) != 0 )
      {
          strError = strerror(errno);
          fprintf(stderr, "fshm_chgrp failed to change gid=%d with errno=%d. %s\n", \
            SET_GID_TO, errno, strError
          );
      }
      else
      {
          printf("fshm_chgrp successfully changed gid to %d\n", SET_GID_TO);
      }
    #endif

    printf("Writing data...\n\n");

    strcpy(data.msg, "Hello World!\n");
    data.numAccess = 0;
    data.lastAccessPid = getpid();

    write( fileno(fObj), &data, sizeof(OwnerGuestData) );

    printf("Waiting %d seconds before closing stream.\n\n", sleepTime);

    sleep(sleepTime);

    printf("Removing stream..\n");
    fclose(fObj);


    return 0;
}

/* vim: set ts=4 sw=4 st=4 expandtab : */
