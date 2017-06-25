/**
 * Copyright (c) 2017 Timothy Savannah
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

static char *DEFAULT_STREAM_NAME = "/test1";

int main(int argc, char* argv[])
{
    FILE *fObj;

    char *data;
    char *streamName;

    if ( argc == 1 )
    {
        streamName = DEFAULT_STREAM_NAME;
    }
    else if ( argc == 2 )
    {
        streamName = argv[1];
    }
    else
    {
        fprintf(stderr, "Wrong number of arguments.\n\nUsage: guest (optional: [stream name])\n");
        return EINVAL;
    }

    printf("Trying to open existing fshm stream at:  \"%s\"\n", streamName);

    fObj = fshm_open(streamName, 0775, FSHM_GUEST);
    if(!fObj)
    {
        fprintf(stderr, "Error opening stream! %d: %s\n", errno, strerror(errno));
        return 1;
    }

    data = malloc(200);

    read( fileno(fObj), data, 200 );

    printf("Data is: %s\n", data);

    fclose(fObj);
    free(data);


    return 0;
}

/* vim: set ts=4 sw=4 st=4 expandtab : */
