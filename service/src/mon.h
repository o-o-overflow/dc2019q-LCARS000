#ifndef _MON_H
#define _MON_H

#include <stdint.h>
#include "msg.h"

#define MAX_APP_COUNT 16

enum app_state {
    STATE_INIT,
    STATE_IDLE,
    STATE_BUSY,
    STATE_DEAD,
};

typedef struct app_struct {
    int id;
    int pid;
    int tx;
    int rx;
    enum app_state state;
    char name[0x10];
    struct app_request cur_req;
    struct msg *msg;
} app_t;


#endif
