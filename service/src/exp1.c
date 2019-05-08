#include "app.h"
#include <stdio.h>
#include <string.h>

int app_main() {
    int fd = Xopen("flag1.papp");
    uint32_t a = shm_alloc(0x10000);
    int len = _read(fd, PARAM_AT(a), 0x10000);
    Xecho("BEGIN\n");
    request(REQ_ECHO, a, len, 0, 0);
    Xecho("END\n");
    return 0;
}
