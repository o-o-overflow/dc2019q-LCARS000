#ifndef _APP_H
#define _APP_H

#include <stdint.h>

#define APP_BLOCK_SIZE 0x4000ULL
#define APP_TEXT_BASE 0x10000000ULL
#define APP_DATA_BASE 0x20000000ULL
#define APP_STACK_END 0xf0000000ULL

#define PARAM_SIZE 0x1000000ULL
#define PARAM_RO 0x30000000ULL
#define PARAM_RW 0x40000000ULL

#define PARAM_FOR(id) (void *)(PARAM_RW + PARAM_SIZE * id)

#define MON_TEXT_BASE 0x100000000ULL

#define REQ_ECHO 0
#define REQ_CHECKIN 1

struct region {
    uint32_t start;
    uint32_t end;
    uint32_t prot;
};

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
} app_t;

struct app_request {
    uint32_t no;
    uint32_t a, b, c, d;
};

struct param_readonly {
    uint32_t magic;
};

void _start();
void __exit(int status);
int _write(int fd, void *buf, uint64_t size);
int _read(int fd, void *buf, uint64_t size);
int _munmap(void *buf, uint64_t size);
int request(uint32_t no, uint32_t a, uint32_t b, uint32_t c, uint32_t d);
int Xecho(const char *str);
int Xcheckin(const char *str);

extern int app_main(void *param);

#endif
