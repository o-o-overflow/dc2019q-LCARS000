#ifndef _APP_H
#define _APP_H

#include <stdint.h>
#include <sys/socket.h>

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

enum app_state {
    STATE_INIT,
    STATE_IDLE,
    STATE_BUSY,
    STATE_DEAD,
};

typedef struct msg {
    int32_t from;
    int32_t type;
    uint32_t start;
    uint32_t size;
    struct msg *next;
} msg_t;

struct app_request {
    uint32_t no;
    uint32_t a, b, c, d;
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

void _start();
void __exit(int status);
int _sendmsg(int fd, const struct msghdr *msg, int flags);
int _recvmsg(int fd, struct msghdr *msg, int flags);
int _write(int fd, void *buf, uint64_t size);
int _read(int fd, void *buf, uint64_t size);
int _munmap(void *buf, uint64_t size);
int request(uint32_t no, uint32_t a, uint32_t b, uint32_t c, uint32_t d);
int Xecho(const char *str);
int Xcheckin(const char *str);
int Xlookup(const char *str);
int Xwait(int from, int type, msg_t *msg);
int Xpost(int to, int type, const void *buf, uint32_t size);

extern int app_main();

#endif
