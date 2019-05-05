#include "app.h"
#include <stdio.h>
#include <string.h>

#define MAX_COMMAND_LENGTH 0x100
#define MAX_PARAM_COUNT 8

int app_main() {
    Xcheckin("io");
    int in = Xopen("/dev/stdin");
    int out = Xopen("/dev/stdout");
    char buf[MAX_COMMAND_LENGTH];
    const char *argv[MAX_PARAM_COUNT];

    while (1) {
        int n = read_until(in, &buf, sizeof(buf) - 1, '\n');
        if (n <= 0) {
            break;
        }
        buf[n + 1] = 0;
        char *c = strchr(buf, '\n');
        if (c) {
            *c = '\0';
        }
        int argc = 0;
        argv[argc++] = buf;
        for (char *space = strchr(buf, ' ');
                space != NULL && argc < MAX_PARAM_COUNT;
                space = strchr(space, ' ')) {
            *space = '\0';
            argv[argc++] = ++space;
        }
        for (int i = 0; i < argc; i++) {
            dprintf(out, "argv[%d] = \"%s\"\n", i, argv[i]);
        }
        if (!strcmp(argv[0], "run")) {
            dprintf(out, "run \"%s\" = %d\n", argv[1], Xexec(argv[1]));
        } else if (!strcmp(argv[0], "exit")) {
            break;
        } else {
            dprintf(out, "unrecognized command \"%s\"\n", argv[0]);
        }
    }
    return 0;
}
