#include "app.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define ECHO "echo.sys"
#define CRYPTO "crypto.sys"
#define LOADER "loader.sys"
#define MAX_COMMAND_LENGTH 0x100
#define MAX_PARAM_COUNT 8

static int download(int fin, const char *file, uint32_t size) {
    if (size == 0 || size > 0x10000) {
        return -EFBIG;
    }
    int fout = Xopen(file);
    if (fout < 0) {
        return -EBADF;
    } else {
        char buf[0x800];
        for (int i = 0; i < size; ) {
            uint32_t this_read = sizeof(buf);
            if (this_read > size - i) {
                this_read = size - i;
            }
            int l = _read(fin, &buf, this_read);
            if (l <= 0) {
                break;
            }
            _write(fout, &buf, l);
            i += l;
        }
    }
    return fout;
}

int app_main() {
    Xcheckin("init", 0);

    int in = Xopen("/dev/stdin");
    int out = Xopen("/dev/stdout");

    // launch services
    Xexec(ECHO, CTX_SYSTEM_APP);
    Xexec(CRYPTO, CTX_KERNEL);

    // drop privileges
    Xrunas(CTX_SYSTEM_APP);
    Xcheckin("io", CTX_SYSTEM_APP);

    char buf[MAX_COMMAND_LENGTH];
    const char *argv[MAX_PARAM_COUNT];
    while (1) {
        int n = read_until(in, &buf, sizeof(buf) - 1, '\n');
        if (n <= 0) {
            break;
        }
        buf[n] = 0;
        char *c = strchr(buf, '\n');
        if (c) {
            *c = '\0';
        }
        if (strstr(buf, "txt")) {
            break;
        }
        int argc = 0;
        argv[argc++] = buf;
        for (char *space = strchr(buf, ' ');
                space != NULL && argc < MAX_PARAM_COUNT;
                space = strchr(space, ' ')) {
            *space = '\0';
            argv[argc++] = ++space;
        }
#ifdef DEBUG
        for (int i = 0; i < argc; i++) {
            dprintf(out, "argv[%d] = \"%s\"\n", i, argv[i]);
        }
#endif
        if (!strcmp(argv[0], "run")) {
            int ret = Xexec(argv[1], CTX_SYSTEM_APP);
            if (ret == -EACCES) {
                int ldr = Xexec(LOADER, CTX_SYSTEM_APP);
                dprintf(out, "loading at #%d...\n", ldr);
                if (ldr >= 0 && (ret = Xpost(ldr, 'load', argv[1], strlen(argv[1]) + 1)) == 0) {
                    msg_t msg = {0};
                    ret = Xwait(ldr, -1, &msg);
                    dprintf(out, "loading result %d (%s)...\n", msg.type,
                            PARAM_FOR(msg.from) + msg.start);
                }
            }
            dprintf(out, "run \"%s\" = %d\n", argv[1], ret);
        } else if (!strcmp(argv[0], "download")) {
            if (argc < 3) {
                dprintf(out, "invalid arguments\n");
            } else {
                int fd = download(in, argv[1], atoi(argv[2]));
                dprintf(out, "download \"%s\" = %d\n", argv[1], fd);
                _close(fd);
            }
#ifdef DEBUG
        } else if (!strcmp(argv[0], "open")) {
            dprintf(out, "open \"%s\" = %d\n", argv[1], Xopen(argv[1]));
#endif
        } else if (!strcmp(argv[0], "exit")) {
            dprintf(out, "bye\n");
            break;
        } else {
            dprintf(out, "unrecognized command \"%s\"\n", argv[0]);
        }
    }
    return 0;
}
