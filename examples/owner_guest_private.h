/**
 * Copyright (c) 2017, 2018 Timothy Savannah
 *   All Rights Reserved, Licensed under terms of Lesser GNU Public License version 2.1
 *
 * owner_guest_private.h - Shared data between examples/owner.c and examples/guest.c
 *
 */

#ifndef _OWNER_GUEST_PRIVATE_H
#define _OWNER_GUEST_PRIVATE_H

static char *DEFAULT_STREAM_NAME = "/test1";

typedef struct {
    char msg[128];
    int numAccess;
    int lastAccessPid;
    
    char nullPad;
} OwnerGuestData;

#endif
