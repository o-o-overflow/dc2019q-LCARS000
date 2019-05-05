#include "app.h"
#include <string.h>
#include <sys/syscall.h>

static void *param;

__attribute__((section(".app_start")))
void _start(void *_param) {
    param = _param;
    _munmap((void *)MON_TEXT_BASE, 0x1000000);
    // TODO munmap stack and all shared libraries
    __exit(app_main(_param));
}

void __exit(int status) {
    asm volatile (
            "syscall\n"
            ::"a"(SYS_exit),"D"(status)
            );
}

int _write(int fd, void *buf, uint64_t size) {
    int res;
    asm volatile (
            "syscall\n"
            :"=a"(res)
            :"a"(SYS_write),"D"(fd),"S"(buf),"d"(size)
            );
    return res;
}

int _read(int fd, void *buf, uint64_t size) {
    int res;
    asm volatile (
            "syscall\n"
            :"=a"(res)
            :"a"(SYS_read),"D"(fd),"S"(buf),"d"(size)
            );
    return res;
}

int _munmap(void *buf, uint64_t size) {
    int res;
    asm volatile (
            "syscall\n"
            :"=a"(res)
            :"a"(SYS_munmap),"D"(buf),"S"(size)
            );
    return res;
}

int request(uint32_t no, uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    struct app_request req = {
        .no = no,
        .a = a,
        .b = b,
        .c = c,
        .d = d,
    };
    _write(0, &req, sizeof(req));
    int32_t res = 0;
    _read(1, &res, sizeof(res));
    return res;
}

int Xecho(const char *str) {
    int len = strlen(str);
    strcpy(param, str);
    return request(REQ_ECHO, 0, len, 0, 0);
}

int Xcheckin(const char *str) {
    memcpy(param, str, 0x10);
    return request(REQ_CHECKIN, 0, 0, 0, 0);
}

int Xlookup(const char *str) {
    memcpy(param, str, 0x10);
    return request(REQ_LOOKUP, 0, 0, 0, 0);
}
