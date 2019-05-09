#ifndef _MON_H
#define _MON_H

#include <stdint.h>
#include "msg.h"

#define MAX_APP_COUNT 16

enum app_state {
    STATE_DEAD = 0,
    STATE_IDLE,
    STATE_BUSY,
};

typedef struct app_struct {
    int id;
    int pid;
    int tx;
    int rx;
    uint32_t ctx;
    uint32_t role;
    uint32_t critical;
    enum app_state state;
    char name[0x10];
    struct app_request cur_req;
    struct msg *msg;
} app_t;


#endif
