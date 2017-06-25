
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
