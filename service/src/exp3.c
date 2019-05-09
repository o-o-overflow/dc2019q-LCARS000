#include "app.h"
#include <stdio.h>
#include <string.h>

int app_main() {
    int ldr = 3; // failed loader
    msg_t msg = {0};
    Xpost(ldr, 'load', "2", 2);
    // Xwait(ldr, -1, &msg);
    // Xecho(PARAM_FOR(msg.from) + msg.start);
    Xpost(ldr, 'load', "3", 2);
    // Xwait(ldr, -1, &msg);
    // Xecho(PARAM_FOR(msg.from) + msg.start);
    return 0;
}
