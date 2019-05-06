#define _GNU_SOURCE
#include "fs.h"
#include <string.h>
#include <sys/mman.h>

static int file_cnt = 0;
static file_t files[MAX_FILE_COUNT];

static const char *trim_path(const char *path) {
    while (*path == '/' || *path == '.') path++;
    return path;
}

file_t *query_file(const char *name) {
    name = trim_path(name);
    for (int i = 0; i < file_cnt; i++) {
        if (files[i].fd != -1 && !strcmp(files[i].name, name)) {
            return &files[i];
        }
    }
    return NULL;
}

file_t *open_file(const char *name, int perm) {
    file_t file;
    strncpy(file.name, trim_path(name), sizeof(file.name));
    file_t *f = query_file(file.name);
    if (f != NULL) {
        if (perm & f->perm) {
            return f;
        } else {
            return NULL;
        }
    }
    return append_file(file.name, memfd_create(file.name, 0), FILE_RDWR);
}

file_t *append_file(const char *name, int fd, int perm) {
    if (fd != -1 && file_cnt < MAX_FILE_COUNT) {
        strncpy(files[file_cnt].name, trim_path(name), sizeof(files[file_cnt].name));
        files[file_cnt].perm = perm;
        files[file_cnt].fd = fd;
        return &files[file_cnt++];
    } else {
        return NULL;
    }
}
