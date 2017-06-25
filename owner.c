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

#define DEFAULT_SLEEP_TIME 10

static char *DEFAULT_STREAM_NAME = "/test1";


int main(int argc, char* argv[])
{
    FILE *fObj;

    char *data;
    char *streamName;
    int sleepTime;

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
    fObj = fshm_open(streamName, 0775, FSHM_OWNER);
    if (!fObj)
    {
        if ( errno == EEXIST )
        {
            fprintf(stderr, "Existing stream, forcing close.\n");
            fshm_force_destroy(streamName);
            fObj = fshm_open(streamName, 0775, FSHM_OWNER);
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

    fprintf(fObj, "Hello World\n");

    fseek(fObj, 0, SEEK_SET);

    data = malloc(200);

    fread(data, 1, 200, fObj);

    printf("Data is: %s\n", data);


    sleep(sleepTime);

    fclose(fObj);
    free(data);


    return 0;
}

/* vim: set ts=4 sw=4 st=4 expandtab : */
