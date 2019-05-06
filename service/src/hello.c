#include "app.h"
#include "crypto.h"
#include <stdio.h>
#include <string.h>

static void hexdump(void *raw, uint32_t size) {
    char buf[0x100];
    for (int i = 0; i < size; i++) {
        sprintf(&buf[i * 2], "%02x", ((uint8_t *)raw)[i]);
    }
    Xecho(buf);
}

int app_main() {
    // test echo service
    int echo_service = Xlookup("echo");
    const char *msg[] = {
        "hello world",
        "test",
        "0x4141414141",
    };
    for (int i = 0; i < sizeof(msg) / sizeof(msg[0]); i++) {
        Xpost(echo_service, -1, msg[i], strlen(msg[i]) + 1);
    }

    // test crypto service
    int crypto_service = Xlookup("crypto");
    struct crypto_request req = {0};
    req.type = CRYPTO_HASH_MD5;
    req.hash_data = shm_alloc(0x10);
    req.hash_data_size = 0x10;
    memset(PARAM_AT(req.hash_data), 0, 0x10);
    Xpost(crypto_service, 'SECD', &req, sizeof(req));
    msg_t m;
    Xwait(crypto_service, -1, &m);
    hexdump(PARAM_FOR(m.from) + m.start, m.size);
    return 0;
}
