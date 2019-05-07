#ifndef _GNU_C_H
#define _GNU_C_H

#include <stdint.h>

typedef uint64_t size_t;
typedef void *FILE;

void *malloc(size_t sz);
void free(void *ptr);
void *realloc(void *ptr, size_t sz);

void *__memcpy_chk(void *dst, const void *src, size_t size, size_t max_size);
void *__memset_chk(void *s, char c, size_t n, size_t max_size);
#endif
