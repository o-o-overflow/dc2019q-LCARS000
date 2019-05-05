#ifndef _FS_H
#define _FS_H

#define MAX_FILE_COUNT 16

typedef struct file {
    int fd;
    char name[0x20];
} file_t;

file_t *query_file(const char *name);
file_t *open_file(const char *name);
void append_file(const char *name, int fd);

#endif
