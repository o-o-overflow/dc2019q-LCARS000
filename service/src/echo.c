#include "app.h"
#include <stdio.h>
#include <string.h>

int app_main() {
    Xcheckin("echo", -1);
    int fd = Xopen("/dev/stdout");
    Xrunas(CTX_PLATFORM_APP);
    msg_t msg;
    char buf[0x100];
    while (Xwait(-1, -1, &msg) == 0) {
        snprintf(&buf[0], sizeof(buf), "app #%d said: %s\n", msg.from, (char *)(PARAM_FOR(msg.from) + msg.start));
        _write(fd, buf, strlen(buf));
    }
    return 0;
}
