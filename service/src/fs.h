#ifndef _FS_H
#define _FS_H

#define MAX_FILE_COUNT 16

enum file_perm {
    FILE_RDWR = 1,
    FILE_EXEC = 2,
};

enum file_role {
    FILE_ROLE_SYSTEM_DATA,
    FILE_ROLE_PLATFORM_DATA,
    FILE_ROLE_UNTRUSTED_DATA,
    FILE_ROLE_GLOBAL_DATA,
};

typedef struct file {
    int fd;
    enum file_perm perm;
    enum file_role role;
    char name[0x20];
} file_t;

file_t *query_file(const char *name);
file_t *open_file(int ctx, const char *name, int perm);
file_t *append_file(int ctx, const char *name, int fd, int perm);

#endif
