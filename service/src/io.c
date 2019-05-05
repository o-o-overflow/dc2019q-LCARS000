#include "app.h"
#include <stdio.h>
#include <string.h>

int app_main() {
    Xcheckin("io");
    int in = Xopen("/dev/stdin");
    int out = Xopen("/dev/stdout");
    char buf[0x100];
    while (1) {
        int n = _read(in, &buf, sizeof(buf));
        _write(out, &buf, n);
    }
    return 0;
}
