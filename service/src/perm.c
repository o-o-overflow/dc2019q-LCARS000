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

int app_main() {
    io_service = Xlookup("io");
    DEBUG("io service = %d\n", io_service);
    echo_service = Xlookup("echo");
    DEBUG("echo service = %d\n", echo_service);
    crypto_service = Xlookup("crypto");
    DEBUG("crypto service = %d\n", crypto_service);
    DEBUG("exec platform apps = %d\n", Xexec("echo.sys", CTX_UNTRUSTED_APP));
    DEBUG("call echo service = %d\n", Xpost(echo_service, -1, "echo echo", 10));
    DEBUG("open system file = %d\n", Xopen("root.key"));
    DEBUG("open system file = %d\n", Xopen("root.key"));
    DEBUG("open platform file = %d\n", Xopen("perm.uapp"));
    DEBUG("open global file = %d\n", Xopen("svc.uapp"));
    DEBUG("open untrusted file = %d\n", Xopen("not exists"));
    DEBUG("open flag1.papp = %d\n", Xopen("flag1.papp"));
    DEBUG("open flag22.txt = %d\n", Xopen("flag22.txt"));
    DEBUG("open flag333.txt = %d\n", Xopen("flag333.txt"));
    Xecho("test sys_close\n"); // sys_close is forbidden in untrusted
    _close(123);
    Xecho("test sys_mprotect\n"); // sys_mprotect is forbidden in platform
    _mprotect(NULL, 0, 0);
    Xecho("runas untrusted\n"); // only system/kernel are allowed to reload seccomp policy
    Xrunas(CTX_UNTRUSTED_APP);
    return 0;
}
