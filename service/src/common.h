#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>

#define APP_BLOCK_SIZE 0x4000ULL
#define APP_TEXT_BASE 0x10000000ULL
#define APP_DATA_BASE 0x20000000ULL
#define APP_STACK_END 0xf0000000ULL

#define PARAM_SIZE 0x1000000ULL
#define PARAM_RW 0x30000000ULL
#define PARAM_RO 0x40000000ULL
#define PARAM_LOCAL 0x60000000ULL
#define PARAM_FOR(id) (void *)(PARAM_RO + PARAM_SIZE * (id))

#define PARAM_AT(offset) (void *)(PARAM_RW + offset)

inline int access_ok(uint32_t offset, uint32_t size) {
    return offset < PARAM_SIZE && size < PARAM_SIZE && offset + size < PARAM_SIZE;
}

#define MON_TEXT_BASE 0x100000000ULL

#define REQ_ECHO 0
#define REQ_CHECKIN 1
#define REQ_LOOKUP 2
#define REQ_WAIT 3
#define REQ_POST 4
#define REQ_OPEN 5
#define REQ_EXEC 6
#define REQ_RUNAS 7

inline const char *str_request(int32_t no) {
    switch (no) {
        case REQ_ECHO:
            return "echo";
        case REQ_CHECKIN:
            return "checkin";
        case REQ_LOOKUP:
            return "lookup";
        case REQ_WAIT:
            return "wait";
        case REQ_POST:
            return "post";
        case REQ_OPEN:
            return "open";
        case REQ_EXEC:
            return "exec";
        case REQ_RUNAS:
            return "runas";
        default:
            return "unknown";
    }
}

enum app_ctx {
    CTX_KERNEL,
    CTX_SYSTEM_APP,
    CTX_PLATFORM_APP,
    CTX_UNTRUSTED_APP,
};

struct app_request {
    uint32_t no;
    uint32_t a, b, c, d;
};

typedef struct msg {
    struct msg *next;
    int32_t from;
    int32_t type;
    uint32_t start;
    uint32_t size;
} msg_t;

#endif
