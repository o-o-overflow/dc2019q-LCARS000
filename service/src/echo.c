#include "app.h"
#include <stdio.h>
#include <string.h>

int app_main() {
    Xcheckin("echo");
    msg_t msg;
    char buf[0x100];
    while (Xwait(-1, -1, &msg) == 0) {
        snprintf(&buf[0], sizeof(buf), "app #%d said: %s\n", msg.from, (char *)(PARAM_FOR(msg.from) + msg.start));
        Xecho(buf);
    }
    return 0;
}
