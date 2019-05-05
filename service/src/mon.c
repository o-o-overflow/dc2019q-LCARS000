#include "mon.h"
#include "app.h"
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

static int app_cnt = 0;
static app_t apps[MAX_APP_COUNT];

static int launch(const char *path) {
    int fd = open(path, O_RDONLY);
    struct stat stat;
    if (fd == -1 || fstat(fd, &stat) == -1) {
        return -1;
    }
    int channel_mon[2], channel_app[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, channel_app) == -1
        || socketpair(AF_UNIX, SOCK_STREAM, 0, channel_mon) == -1) {
        return -1;
    }
    int pid = fork();
    if (pid == -1) {
        return -1;
    }
    if (!pid) {
        mprotect((void *)PARAM_RO, PARAM_SIZE, PROT_READ);
        mprotect((void *)PARAM_RW, PARAM_SIZE * MAX_APP_COUNT, PROT_READ);
        mprotect(PARAM_FOR(app_cnt), PARAM_SIZE, PROT_READ | PROT_WRITE);

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
                "mov %2, %%rdi\n"
                "xor %%rbp, %%rbp\n"
                "ret\n"
                ::"g"(APP_STACK_END - 0x10),"g"(APP_TEXT_BASE),"g"(PARAM_FOR(app_cnt)));
        exit(0);
    } else {
        close(channel_app[0]);
        close(channel_mon[1]);
        app_t *app = &apps[app_cnt];
        app->id = app_cnt++;
        app->pid = pid;
        app->tx = channel_mon[0];
        app->rx = channel_app[1];
        app->state = STATE_INIT;
        snprintf(app->name, sizeof(app->name), "app #%d", app->id);
    }
    return 0;
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
    int ret = 0;
    fprintf(stderr, "got request %d (%#x,%#x,%#x,%#x) from %s\n", req.no, req.a, req.b,
            req.c, req.d, app->name);
    switch (req.no) {
        case REQ_ECHO:
            if (req.a < PARAM_SIZE && req.b < PARAM_SIZE && req.a + req.b < PARAM_SIZE) {
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
            ret = -ENOENT;
            for (int i = 0; i < app_cnt; i++) {
                if (apps[i].state != STATE_DEAD && !strcmp(apps[i].name, PARAM_FOR(app->id))) {
                    ret = i;
                    break;
                }
            }
        default:
            ret = -ENOSYS;
            break;
    }
response:
    if (write(app->tx, &ret, sizeof(ret)) != sizeof(ret)) {
        perror("write");
        return -1;
    }
    return 0;
}

static void handler(int signo, siginfo_t *info, void *context) {
    int pid = info->si_pid;
    for (int i = 0; i < app_cnt; i++) {
        if (apps[i].pid == pid) {
            cleanup(&apps[i]);
            fprintf(stderr, "%s exit\n", apps[i].name);
            return ;
        }
    }
    fprintf(stderr, "child %d exit?\n", pid);
}

int main(int argc, char *argv[]) {

    struct param_readonly *pro = mmap((void *)PARAM_RO, PARAM_SIZE, PROT_READ | PROT_WRITE,
            MAP_ANON | MAP_SHARED | MAP_FIXED, -1, 0);
    void *prw = mmap((void *)PARAM_RW, PARAM_SIZE * MAX_APP_COUNT, PROT_READ | PROT_WRITE,
            MAP_ANON | MAP_SHARED | MAP_FIXED, -1, 0);

    struct sigaction act;
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGCHLD, &act, NULL);

    for (int i = 1; i < argc; i++) {
        launch(argv[i]);
    }
    while (1) {
        fd_set set;
        FD_ZERO(&set);
        int mfd = -1;
        for (int i = 0; i < app_cnt; i++) {
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
        for (int i = 0; i < app_cnt; i++) {
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
    for (int i = 0; i < app_cnt; i++) {
        kill(apps[i].pid, 9);
    }
    return 0;
}
