#include "gnu-c.h"
#include "app.h"
#include <string.h>

void *malloc(size_t sz) {
    return PARAM_AT(shm_alloc(sz));
}

void free(void *ptr) {
}

void *realloc(void *ptr, size_t sz) {
    void *nptr = malloc(sz);
    if (ptr != NULL) {
        memcpy(nptr, ptr, sz);
    }
    return nptr;
}

void *__memcpy_chk(void *dst, const void *src, size_t size, size_t max_size) {
    return memcpy(dst, src, size);
}

void *__memset_chk(void *s, int c, size_t n, size_t max_size) {
    return memset(s, c, n);
}
