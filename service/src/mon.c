#define _GNU_SOURCE
#include "mon.h"
#include "msg.h"
#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

static app_t apps[MAX_APP_COUNT];

static int launch(int fd) {
    struct stat stat;
    if (fd == -1 || fstat(fd, &stat) == -1 || stat.st_size == 0) {
        return -EINVAL;
    }
    int channel_mon[2], channel_app[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, channel_app) == -1
        || socketpair(AF_UNIX, SOCK_STREAM, 0, channel_mon) == -1) {
        return -errno;
    }
    int app_id = -1;
    for (int i = 0; i < MAX_APP_COUNT; i++) {
        if (apps[i].state == STATE_DEAD) {
            app_id = i;
            break;
        }
    }
    if (app_id == -1) {
        return -EBUSY;
    }
    int pid = fork();
    if (pid == -1) {
        return -errno;
    }
    if (!pid) {
        mremap(PARAM_FOR(app_id), PARAM_SIZE, PARAM_SIZE, MREMAP_FIXED | MREMAP_MAYMOVE, PARAM_RW);
        mprotect((void *)PARAM_RO, PARAM_SIZE * MAX_APP_COUNT, PROT_READ);

        mmap((void *)(APP_DATA_BASE), APP_BLOCK_SIZE, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANON | MAP_FIXED, -1, 0);

        mmap((void *)(APP_STACK_END - APP_BLOCK_SIZE), APP_BLOCK_SIZE, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANON | MAP_FIXED, -1, 0);

        mmap((void *)(APP_TEXT_BASE), stat.st_size, PROT_READ | PROT_EXEC,
                MAP_PRIVATE | MAP_FIXED, fd, 0);
        close(channel_app[1]);
        close(channel_mon[0]);
        dup2(channel_app[0], 0);
        dup2(channel_mon[1], 1);
        for (int i = 2; i < 1024; i++) {
            close(i);
        }
        // TODO setup seccomp
        asm volatile (
                "movq %0, %%rsp\n"
                "pushq %1\n"
                "xor %%rbp, %%rbp\n"
                "ret\n"
                ::"g"(APP_STACK_END - 0x8),"g"(APP_TEXT_BASE));
        exit(0);
    } else {
        close(channel_app[0]);
        close(channel_mon[1]);
        app_t *app = &apps[app_id];
        app->id = app_id;
        app->pid = pid;
        app->tx = channel_mon[0];
        app->rx = channel_app[1];
        app->state = STATE_IDLE;
        app->msg = NULL;
        snprintf(app->name, sizeof(app->name), "app #%d", app->id);
    }
    return app_id;
}

static void cleanup(app_t *app) {
    close(app->tx);
    app->tx = -1;
    app->state = STATE_DEAD;
}

static int read_all(int fd, void *buf, size_t total) {
    int i = 0;
    for (i; i < total; ) {
        int c = read(fd, buf + i, total - i);
        if (c == -1) {
            if (errno != -EINTR) {
                return -1;
            }
        } else if (c == 0) {
            break;
        } else {
            i += c;
        }
    }
    return i;
}

static app_t *query_app(const char *name) {
    for (int i = 0; i < MAX_APP_COUNT; i++) {
        if (apps[i].state != STATE_DEAD && !strcmp(apps[i].name, name)) {
            return &apps[i];
        }
    }
    return NULL;
}

static app_t *get_app(int id) {
    if (id >= 0 && id < MAX_APP_COUNT && apps[id].state != STATE_DEAD) {
        return &apps[id];
    }
    return NULL;
}

static int copy_from_app(int id, uint32_t offset, uint32_t size, void *buf, uint32_t max_size) {
    if (!access_ok(offset, size)) {
        return -1;
    }
    if (size > max_size) {
        size = max_size;
    }
    memcpy(buf, PARAM_FOR(id) + offset, size);
    return size;
}

static int handle_request(app_t *app) {
    struct app_request req = {0};
    int c = read_all(app->rx, &req, sizeof(req));
    if (c == 0) {
        // ignore empty request
        return 0;
    } else if (c != sizeof(req)) {
        perror("read");
        return -1;
    }
    app->cur_req = req;
    int ret = 0;
    int fd = -1;
    char name[0x20];
    fprintf(stderr, "got request %d (%#x,%#x,%#x,%#x) from %s\n", req.no, req.a, req.b,
            req.c, req.d, app->name);
    switch (req.no) {
        case REQ_ECHO:
            if (access_ok(req.a, req.b)) {
                ret = write(1, PARAM_FOR(app->id) + req.a, req.b);
            } else {
                ret = -EINVAL;
            }
            break;
        case REQ_CHECKIN:
            strncpy(app->name, PARAM_FOR(app->id), sizeof(app->name));
            ret = 0;
            break;
        case REQ_LOOKUP:
            {
                app_t *a = query_app(PARAM_FOR(app->id));
                ret = a == NULL ? -ENOENT : a->id;
                break;
            }
        case REQ_WAIT:
            if (!access_ok(req.c, req.d)) {
                ret = -EINVAL;
                break;
            }
            accept_msg(app);
            goto done;
        case REQ_POST:
            if (!access_ok(req.c, req.d)) {
                ret = -EINVAL;
                break;
            }
            app_t *a = get_app(req.a);
            if (a == NULL) {
                ret = -ENOENT;
                break;
            }
            if (query_msg(app, app->id, req.b) != NULL) {
                ret = -EBUSY;
                break;
            }
            msg_t *msg = calloc(1, sizeof(msg_t));
            msg->from = app->id;
            msg->type = req.b;
            msg->start = req.c;
            msg->size = req.d;
            msg->next = NULL;
            append_msg(a, msg);
            if (a->cur_req.no == REQ_WAIT) {
                // try push the new message
                accept_msg(a);
            }
            ret = 0;
            break;
        case REQ_OPEN:
            if (access_ok(req.a, req.b)
                    && copy_from_app(app->id, req.a, req.b, &name, 0x20)) {
                name[0x1f] = 0;
                file_t *f = open_file(name, FILE_RDWR);
                if (f != NULL) {
                    fd = f->fd;
                    ret = 0;
                } else {
                    if (query_file(PARAM_FOR(app->id)) != NULL) {
                        // exists but not writable
                        ret = -EACCES;
                    } else {
                        ret = -ENOENT;
                    }
                }
            } else {
                ret = -EINVAL;
            }
            break;
        case REQ_EXEC:
            if (access_ok(req.a, req.b)
                    && copy_from_app(app->id, req.a, req.b, &name, 0x20)) {
                name[0x1f] = 0;
                file_t *f = open_file(name, FILE_EXEC);
                if (f != NULL) {
                    ret = launch(f->fd);
                } else {
                    if (query_file(PARAM_FOR(app->id)) != NULL) {
                        // exists but not executable
                        ret = -EACCES;
                    } else {
                        ret = -ENOENT;
                    }
                }
            } else {
                ret = -EINVAL;
            }
            break;
        default:
            ret = -ENOSYS;
            break;
    }
response:
    app->cur_req.no = -1;
    if (write(app->tx, &ret, sizeof(ret)) != sizeof(ret)) {
        perror("write");
        return -1;
    }
    if (fd != -1) {
        lseek(fd, 0, SEEK_SET);
        struct msghdr msg = {0};
        char buf[CMSG_SPACE(sizeof(fd))] = {0};
        struct iovec iovec = {
            .iov_base = "",
            .iov_len  = 1,
        };
        msg.msg_iov = &iovec;
        msg.msg_iovlen = 1;
        msg.msg_control = buf;
        msg.msg_controllen = sizeof(buf);
        struct cmsghdr * cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
        *(int *)CMSG_DATA(cmsg) = fd;
        msg.msg_controllen = CMSG_SPACE(sizeof(fd));
        if (sendmsg(app->tx, &msg, 0) == -1) {
            perror("sendmsg");
            return -1;
        }
    }
done:
    return 0;
}

static void handler(int signo, siginfo_t *info, void *context) {
    int pid = info->si_pid;
    for (int i = 0; i < MAX_APP_COUNT; i++) {
        if (apps[i].pid == pid) {
            cleanup(&apps[i]);
            fprintf(stderr, "%s exit %#x\n", apps[i].name, info->si_status);
            return ;
        }
    }
    fprintf(stderr, "child %d exit %#x?\n", pid, info->si_status);
}

int main(int argc, char *argv[]) {

    void *param = mmap((void *)PARAM_RO, PARAM_SIZE * MAX_APP_COUNT, PROT_READ | PROT_WRITE,
            MAP_ANON | MAP_SHARED | MAP_FIXED, -1, 0);

    struct sigaction act;
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGCHLD, &act, NULL);

    append_file("/dev/stdin", 0, FILE_RDWR);
    append_file("/dev/stdout", 1, FILE_RDWR);

    int init = -1;
    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        if (fd != -1) {
            append_file(argv[i], fd, FILE_EXEC);
            if (init == -1) {
                init = fd;
            }
        }
    }

    launch(init);

    while (1) {
        fd_set set;
        FD_ZERO(&set);
        int mfd = -1;
        for (int i = 0; i < MAX_APP_COUNT; i++) {
            if (apps[i].state != STATE_DEAD) {
                int fd = apps[i].rx;
                if (fd != -1) {
                    FD_SET(fd, &set);
                    if (fd > mfd) {
                        mfd = fd;
                    }
                }
            }
        }
        if (mfd == -1) {
            break;
        }
        if (select(mfd + 1, &set, NULL, NULL, NULL) == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("select");
            break;
        }
        for (int i = 0; i < MAX_APP_COUNT; i++) {
            if (apps[i].state != STATE_DEAD) {
                int fd = apps[i].rx;
                if (fd != -1 && FD_ISSET(fd, &set)) {
                    if (handle_request(&apps[i]) < 0) {
                        fprintf(stderr, "failed to handle request from app #%d\n", apps[i].id);
                        kill(apps[i].pid, 9);
                        cleanup(&apps[i]);
                    }
                }
            }
        }
    }
    signal(SIGCHLD, SIG_IGN);
    for (int i = 0; i < MAX_APP_COUNT; i++) {
        if (apps[i].state != STATE_DEAD && apps[i].pid != 0) {
            kill(apps[i].pid, 9);
        }
    }
    return 0;
}
