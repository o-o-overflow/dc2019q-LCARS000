#include "app.h"
#include "loader.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

static struct app_region regions[MAX_PAGE_COUNT];

static int64_t app_load(const char *file, const char **err) {
    int fd = Xopen(file);
    if (fd < 0) {
        return fd;
    }
    struct app_header header = {0};
    int ret = read_all(fd, &header, sizeof(header));
    if (ret < 0) {
        *err = "open";
        return ret;
    }
    if (ret != sizeof(header)) {
        *err = "truncated";
        return -EINVAL;
    }
    if (header.magic != 'LIFE'
        || header.pages == 0
        || header.pages > MAX_PAGE_COUNT) {
        *err = "format";
        return -EINVAL;
    }
    header.name[sizeof(header.name) - 1] = 0;
    int64_t entry = -1;
    int region_cnt = 0;
    char *tmpbuf = PARAM_AT(shm_alloc(0x1000));
    for (int i = 0; i < header.pages; i++) {
        struct app_page pg = {0};
        ret = read_all(fd, &pg, sizeof(pg));
        if (ret < 0) {
            *err = "page";
            goto fail;
            return ret;
        }
        ret = -EINVAL;
        // alignment
        if ((pg.start & 0xfff) || pg.size > 0x1000 || pg.size == 0) {
            *err = "aglinment";
            goto fail;
        }
        // overflow
        if ((pg.start + pg.size <= pg.start)) {
            *err = "overflow";
            goto fail;
        }
        // W^X
        if ((pg.flags & (PAGE_WRITE | PAGE_EXEC)) == (PAGE_WRITE | PAGE_EXEC)) {
            *err = "w^x";
            goto fail;
        }
        // FIXME check pg.start == 0
        uint64_t start = pg.start;
        uint64_t end = start + pg.size;
        // check overlaps
        for (int j = 0; j < region_cnt; j++) {
            if (!(start >= regions[j].end || end <= regions[j].start)) {
                *err = "overlap";
                goto fail;
            }
        }
        uint64_t flags = MAP_FIXED | MAP_ANON | MAP_PRIVATE;
        char *page = _mmap((void *)start, 0x1000, (uint64_t)PROT_WRITE, flags, -1, 0);
        if ((int64_t)page < 0) {
            *err = "mmap";
            ret = (int64_t)page;
            goto fail;
        }
        ret = read_all(fd, tmpbuf, pg.size);
        if (ret < 0) {
            *err = "truncated";
            goto fail;
        }
        memcpy(page, tmpbuf, pg.size);
        if ((pg.flags & (PAGE_ALL)) != PAGE_WRITE) {
            ret = _mprotect(page, 0x1000, pg.flags & PAGE_ALL);
            if (ret != 0) {
                *err = "mprotect";
                goto fail;
            }
        }
        if (entry < 0) {
            entry = pg.start;
        }
        regions[region_cnt].start = pg.start;
        regions[region_cnt].end = pg.start + 0x1000;
        regions[region_cnt].size = pg.size;
        region_cnt++;
    }
    Xcheckin(header.name);
    *err = "ok";
    return entry;
fail:
    for (int i = 0; i < region_cnt; i++) {
        _munmap((void *)regions[i].start, regions[i].size);
    }
    return ret;
}

int app_main() {
    msg_t msg;
    char name[0x20];
    const char *err = "";
    while (Xwait(-1, 'load', &msg) == 0) {
        strncpy(name, PARAM_FOR(msg.from) + msg.start, sizeof(name));
        if (msg.size < sizeof(name)) {
            name[msg.size] = 0;
        }
        int64_t entry = app_load(name, &err);
        Xpost(msg.from, entry, err, strlen(err) + 1);
        if (entry > 0) {
            ((void (*)())entry)();
        }
    }
    return 0;
}
