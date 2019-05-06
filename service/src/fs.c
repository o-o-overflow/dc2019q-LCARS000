#define _GNU_SOURCE
#include "fs.h"
#include "mon.h"
#include <string.h>
#include <sys/mman.h>

static int file_cnt = 0;
static file_t files[MAX_FILE_COUNT];

static const char *trim_path(const char *path) {
    while (*path == '/' || *path == '.') path++;
    return path;
}

static int default_role(enum app_ctx ctx) {
    switch (ctx) {
        case CTX_KERNEL:
            // files loaded by kernel can be accessed by everyone
            return FILE_ROLE_GLOBAL_DATA;
        case CTX_SYSTEM_APP:
            return FILE_ROLE_SYSTEM_DATA;
        case CTX_PLATFORM_APP:
            return FILE_ROLE_PLATFORM_DATA;
        case CTX_UNTRUSTED_APP:
        default:
            return FILE_ROLE_UNTRUSTED_DATA;
    }
}

static int is_allowed(enum app_ctx ctx, enum file_role role) {
    switch (ctx) {
        case CTX_KERNEL:
        case CTX_SYSTEM_APP:
            return 1;
        case CTX_PLATFORM_APP:
            return role >= FILE_ROLE_PLATFORM_DATA;
        case CTX_UNTRUSTED_APP:
            return role >= FILE_ROLE_GLOBAL_DATA;
        default:
            return 0;
    }
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

file_t *open_file(int ctx, const char *name, int perm) {
    file_t file;
    strncpy(file.name, trim_path(name), sizeof(file.name));
    file_t *f = query_file(file.name);
    if (f != NULL) {
        if ((perm & f->perm) && is_allowed(ctx, f->role)) {
            return f;
        } else {
            return NULL;
        }
    }
    return append_file(ctx, file.name, memfd_create(file.name, 0), FILE_RDWR);
}

file_t *append_file(int ctx, const char *name, int fd, int perm) {
    if (fd != -1 && file_cnt < MAX_FILE_COUNT) {
        strncpy(files[file_cnt].name, trim_path(name), sizeof(files[file_cnt].name));
        files[file_cnt].perm = perm;
        files[file_cnt].fd = fd;
        files[file_cnt].role = default_role(ctx);
        return &files[file_cnt++];
    } else {
        return NULL;
    }
}
