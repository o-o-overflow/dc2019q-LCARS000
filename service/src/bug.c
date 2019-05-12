#include "app.h"
#include "crypto.h"
#include <stdio.h>
#include <string.h>

static int io_service;
static int echo_service;
static int crypto_service;

static char buf[0x100];

#define DEBUG(...) do { \
    snprintf(&buf[0], sizeof(buf), __VA_ARGS__); \
    Xecho(buf); \
} while (0)

static int xopen(const char *str) {
    int len = strlen(str) + 1;
    uint32_t a = shm_alloc(len);
    strcpy(PARAM_AT(a), str);
    int ret = request(REQ_OPEN, a, len, 0, 0);
    if (ret == 0) {
        char c;
        _read(1, &c, 1);
    }
    return ret;
}

int app_main() {
    for (int i = 0; i < 1024; i++) {
        char buf[0x20];
        snprintf(&buf[0], sizeof(buf), "file%d", i);
        DEBUG("open %s = %d\n", buf, xopen(&buf[0]));
    }
    while (1);
    return 0;
}
