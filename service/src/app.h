#ifndef _APP_H
#define _APP_H

#include <stdint.h>
#include <sys/socket.h>

#include "common.h"

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
int Xopen(const char *str);

extern int app_main();

#endif
