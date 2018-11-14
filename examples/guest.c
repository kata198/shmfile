/**
 * Copyright (c) 2017, 2018 Timothy Savannah
 *   All Rights Reserved, Licensed under terms of Lesser GNU Public License version 2.1
 *
 * guest.c - Example of an FSHM_GUEST - 
 *   Inherits and reads data off an existing mapping
 *
 *
 *  Takes up to one optional argument
 *
 *     First, if provided, is an explicit stream name to use.
 *       If not provided, defaults to DEFAULT_STREAM_NAME below.
 *
 *  An "owner" must be running with the same stream name for the guest to be able to read.
 *
 */


#include "shmfile.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "owner_guest_private.h"

void _fshm_printCopyrightVersion(void);

int main(int argc, char* argv[])
{
    FILE *fObj;

    char *streamName;
    OwnerGuestData data = { 0 };

    if ( argc == 1 )
    {
        streamName = DEFAULT_STREAM_NAME;
    }
    else if ( argc == 2 )
    {
        streamName = argv[1];
        if ( strcmp(streamName, "--version") == 0 )
        {
            _fshm_printCopyrightVersion();
            return 0;
        }
    }
    else
    {
        fprintf(stderr, "Wrong number of arguments.\n\nUsage: guest (optional: [stream name])\n");
        return EINVAL;
    }

    printf("Trying to open existing fshm stream at:  \"%s\"\n", streamName);

    /*fObj = fshm_open(streamName, 0775, FSHM_GUEST);*/
    fObj = fshm_guest_open(streamName);
    if(!fObj)
    {
        fprintf(stderr, "Error opening stream! %d: %s\n", errno, strerror(errno));
        return 1;
    }

    read( fileno(fObj), &data, sizeof(OwnerGuestData) );

    printf("Message is: %s\n", data.msg);
    printf("Previous accesses: %d\n", data.numAccess);
    printf("Previous pid: %d\n", data.lastAccessPid);

    data.numAccess += 1;
    data.lastAccessPid = getpid();

    fseek(fObj, 0, SEEK_SET);

    fwrite(&data, sizeof(OwnerGuestData), 1, fObj);
    fflush(fObj);

    write( fileno(fObj), &data, sizeof(OwnerGuestData) );

    printf("\nUpdated dataset.\n");


    fclose(fObj);

    return 0;
}

/* vim: set ts=4 sw=4 st=4 expandtab : */
