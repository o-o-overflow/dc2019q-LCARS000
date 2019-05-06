#include "app.h"
#include <string.h>
#include <sys/syscall.h>
#include <sys/socket.h>

__attribute__((section(".app_start")))
void _start() {
    _munmap((void *)MON_TEXT_BASE, 0x1000000);
    // TODO munmap stack and all shared libraries
    __exit(app_main());
}

void __exit(int status) {
    asm volatile (
            "syscall\n"
            ::"a"(SYS_exit),"D"(status)
            );
}

int _sendmsg(int fd, const struct msghdr *msg, int flags) {
    int res;
    asm volatile (
            "syscall\n"
            :"=a"(res)
            :"a"(SYS_sendmsg),"D"(fd),"S"(msg),"d"(flags)
            );
    return res;
}

int _recvmsg(int fd, struct msghdr *msg, int flags) {
    int res;
    asm volatile (
            "syscall\n"
            :"=a"(res)
            :"a"(SYS_recvmsg),"D"(fd),"S"(msg),"d"(flags)
            );
    return res;
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
    if (_write(0, &req, sizeof(req)) != sizeof(req)) {
        __exit(1);
    }
    int32_t res = 0;
    if (_read(1, &res, sizeof(res)) != sizeof(res)) {
        __exit(2);
    }
    return res;
}

int read_until(int fd, void *buf, uint64_t size, char delim) {
    int ret;
    char c;
    for (int i = 0; i < size; i++) {
        if ((ret = _read(fd, &c, 1)) <= 0) {
            return ret;
        }
        if (c == delim) {
            ((char *)buf)[i] = 0;
            return i;
        }
        ((char *)buf)[i] = c;
    }
    return size;
}

int Xecho(const char *str) {
    int len = strlen(str);
    strcpy(ARG_FOR(0), str);
    return request(REQ_ECHO, 0, len, 0, 0);
}

int Xcheckin(const char *str) {
    memcpy(ARG_FOR(0), str, 0x10);
    return request(REQ_CHECKIN, 0, 0, 0, 0);
}

int Xlookup(const char *str) {
    memcpy(ARG_FOR(0), str, 0x10);
    return request(REQ_LOOKUP, 0, 0, 0, 0);
}

int Xwait(int from, int type, msg_t *msg) {
    int ret = request(REQ_WAIT, from, type, 0, 0x10);
    if (ret == 0 && msg != NULL) {
        memcpy(msg, ARG_FOR(0), 0x10);
    }
    return ret;
}

int Xpost(int to, int type, const void *buf, uint32_t size) {
    memcpy(ARG_FOR(0), buf, size);
    int ret = request(REQ_POST, to, type, 0, size);
    return ret;
}

int Xopen(const char *str) {
    int len = strlen(str);
    strcpy(ARG_FOR(0), str);
    int ret = request(REQ_OPEN, 0, len + 1, 0, 0);
    if (ret == 0) {
        struct msghdr msg = {0};
        char c;
        struct iovec io = {
            .iov_base = &c,
            .iov_len = 1,
        };
        msg.msg_iov = &io;
        msg.msg_iovlen = 1;

        char c_buffer[256];
        msg.msg_control = c_buffer;
        msg.msg_controllen = sizeof(c_buffer);
        if (_recvmsg(1, &msg, MSG_WAITALL) < 0) {
            __exit(3);
        }
        ret = *(int *)CMSG_DATA(CMSG_FIRSTHDR(&msg));
    }
    return ret;
}

int Xexec(const char *str) {
    int len = strlen(str);
    strcpy(ARG_FOR(0), str);
    return request(REQ_EXEC, 0, len + 1, 0, 0);
}
