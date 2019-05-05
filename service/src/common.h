#ifndef _COMMON_H
#define _COMMON_H

#define APP_BLOCK_SIZE 0x4000ULL
#define APP_TEXT_BASE 0x10000000ULL
#define APP_DATA_BASE 0x20000000ULL
#define APP_STACK_END 0xf0000000ULL

#define PARAM_SIZE 0x1000000ULL
#define PARAM_RW 0x30000000ULL
#define PARAM_RO 0x40000000ULL
#define PARAM_FOR(id) (void *)(PARAM_RO + PARAM_SIZE * id)

#define ARG_SIZE 0x100000
#define ARG_FOR(id) (void *)(PARAM_RW + ARG_SIZE * id)

#define MON_TEXT_BASE 0x100000000ULL

#define REQ_ECHO 0
#define REQ_CHECKIN 1
#define REQ_LOOKUP 2
#define REQ_WAIT 3
#define REQ_POST 4
#define REQ_OPEN 5

struct app_request {
    uint32_t no;
    uint32_t a, b, c, d;
};

typedef struct msg {
    int32_t from;
    int32_t type;
    uint32_t start;
    uint32_t size;
    struct msg *next;
} msg_t;

#endif
