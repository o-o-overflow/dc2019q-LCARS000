#include "app.h"
#include <stdio.h>
#include <string.h>

int app_main() {
    char buf[0x100] = {0};
    int fd = Xopen("flag3.txt");
    int n = _read(fd, &buf, sizeof(buf));
    Xecho(buf);
    return 0;
}
