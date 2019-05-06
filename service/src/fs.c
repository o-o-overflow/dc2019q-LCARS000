#define _GNU_SOURCE
#include "fs.h"
#include <string.h>
#include <sys/mman.h>

static int file_cnt = 0;
static file_t files[MAX_FILE_COUNT];

file_t *query_file(const char *name) {
    for (int i = 0; i < file_cnt; i++) {
        if (files[i].fd != -1 && !strcmp(files[i].name, name)) {
            return &files[i];
        }
    }
    return NULL;
}

file_t *open_file(const char *name, int perm) {
    file_t file;
    strncpy(file.name, name, sizeof(file.name));
    file_t *f = query_file(file.name);
    if (f != NULL) {
        if (perm & f->perm) {
            return f;
        } else {
            return NULL;
        }
    }
    if (strchr(file.name, '/') || strchr(file.name, '.')) {
        return NULL;
    }
    file.fd = memfd_create(file.name, 0);
    file.perm = FILE_RDWR;
    if (file.fd != -1) {
        files[file_cnt] = file;
        return &files[file_cnt++];
    } else {
        return NULL;
    }
}

void append_file(const char *name, int fd, int perm) {
    if (fd != -1 && file_cnt < MAX_FILE_COUNT) {
        strncpy(files[file_cnt].name, name, sizeof(files[file_cnt].name));
        files[file_cnt].perm = perm;
        files[file_cnt++].fd = fd;
    }
}
