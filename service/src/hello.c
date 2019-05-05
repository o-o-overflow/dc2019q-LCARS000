#include "app.h"
#include <stdio.h>
#include <string.h>

int app_main() {
    int echo_service = Xlookup("echo");
    const char *msg[] = {
        "hello world",
        "test",
        "0x4141414141",
    };
    for (int i = 0; i < sizeof(msg) / sizeof(msg[0]); i++) {
        Xpost(echo_service, -1, msg[i], strlen(msg[i]) + 1);
    }
    return 0;
}
