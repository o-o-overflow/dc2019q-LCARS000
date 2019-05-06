#ifndef _FS_H
#define _FS_H

#define MAX_FILE_COUNT 16

enum file_perm {
    FILE_RDWR = 1,
    FILE_EXEC = 2,
};

typedef struct file {
    int fd;
    int perm;
    char name[0x20];
} file_t;

file_t *query_file(const char *name);
file_t *open_file(const char *name, int perm);
void append_file(const char *name, int fd, int perm);

#endif
