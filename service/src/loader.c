#include "app.h"
#include "loader.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#define AES_BLOCK_SIZE 0x10

static struct app_region regions[MAX_PAGE_COUNT];

static int shasum(const char *in, void *out, size_t size) {
    int crypto_service = Xlookup("crypto");
    struct crypto_request req = {0};
    msg_t m = {0};
    req.type = CRYPTO_HASH_SHA;
    req.hash_data = shm_alloc(size);
    req.hash_data_size = size;
    memcpy(PARAM_AT(req.hash_data), in, size);
    int ret = Xpost(crypto_service, 'SECD', &req, sizeof(req));
    if (ret < 0) {
        return ret;
    }
    Xwait(crypto_service, -1, &m);
    if (m.type == 0) {
        memcpy(out, PARAM_FOR(m.from) + m.start, m.size);
#ifdef DEBUG
    } else {
        Xecho(PARAM_FOR(m.from) + m.start);
#endif
    }
    return m.type;
}

static int rsa_decrypt(int cert, const void *in, void *out) {
    int crypto_service = Xlookup("crypto");
    struct crypto_request req = {0};
    msg_t m = {0};
    req.type = CRYPTO_DECRYPT_RSA;
    req.sig_cert_id = cert;
    req.sig_data = shm_alloc(0x100);
    req.sig_data_size = 0x100;
    memcpy(PARAM_AT(req.sig_data), in, 0x100);
    int ret = Xpost(crypto_service, 'SECD', &req, sizeof(req));
    if (ret < 0) {
        return ret;
    }
    Xwait(crypto_service, -1, &m);
    if (m.type == 0) {
        memcpy(out, PARAM_FOR(m.from) + m.start, m.size);
#ifdef DEBUG
    } else {
        Xecho(PARAM_FOR(m.from) + m.start);
#endif
    }
    return m.type;
}

static int aes_decrypt(struct app_page *pg, struct app_page_crypto *ci, const char *in, char *out, size_t size) {
    int crypto_service = Xlookup("crypto");
    struct crypto_request req = {0};
    msg_t m = {0};
    req.type = CRYPTO_DECRYPT_AES;
    req.cipher_key_id = pg->c_key;
    req.cipher_size = 256;
    req.cipher_mode = pg->c_mode;
    req.cipher_key = shm_alloc(sizeof(AES_KEY));
    req.cipher_key_size = sizeof(AES_KEY);
    memcpy(PARAM_AT(req.cipher_key), ci->key, sizeof(AES_KEY));
    req.cipher_data = shm_alloc(size);
    req.cipher_data_size = size;
    memcpy(PARAM_AT(req.cipher_data), in, size);
    req.cipher_iv = shm_alloc(AES_BLOCK_SIZE);
    req.cipher_iv_size = AES_BLOCK_SIZE;
    memcpy(PARAM_AT(req.cipher_iv), ci->iv, AES_BLOCK_SIZE);
    int ret = Xpost(crypto_service, 'SECD', &req, sizeof(req));
    if (ret < 0) {
        return ret;
    }
    Xwait(crypto_service, -1, &m);
    if (m.type == 0) {
        memcpy(out, PARAM_FOR(m.from) + m.start, m.size);
#ifdef DEBUG
    } else {
        Xecho(PARAM_FOR(m.from) + m.start);
#endif
    }
    return m.type;
}

static int app_load(const char *file, const char **err, struct app_info *info) {
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
    int region_cnt = 0;
    char tmpbuf[0x1000];
    for (int i = 0; i < header.pages; i++) {
        struct app_page pg = {0};
        struct app_page_crypto c = {0};
        struct app_page_sig sig = {0};
        ret = read_all(fd, &pg, sizeof(pg));
        if (ret < 0) {
            *err = "page";
            goto fail;
        }
        if (pg.flags & PAGE_SIGNED) {
            if ((pg.s_cert != CRYPTO_CERT_SYSTEM && pg.s_cert != CRYPTO_CERT_PLATFORM)) {
                ret = -EINVAL;
                *err = "sig param";
                goto fail;
            }
            if (!(pg.flags & PAGE_ENCRYPTED) || pg.s_cert != pg.c_key) {
                // signed pages must be encrypted
                // cert should matches decryption key
                ret = -EINVAL;
                *err = "mismatch";
                goto fail;
            }
            ret = read_all(fd, &sig, sizeof(sig));
            if (ret < 0) {
                *err = "sig info";
                goto fail;
            }
        }
        if (pg.flags & PAGE_ENCRYPTED) {
            if ((pg.c_mode != CRYPTO_MODE_ECB && pg.c_mode != CRYPTO_MODE_CBC)
                    || pg.c_key > CRYPTO_KEY_SESSION) {
                ret = -EINVAL;
                *err = "crypto param";
                goto fail;
            }
            ret = read_all(fd, &c, sizeof(c));
            if (ret < 0) {
                *err = "crypto info";
                goto fail;
            }
        }
        ret = -EINVAL;
        // alignment
        if ((pg.start & 0xfff) || pg.size > 0x1000 || pg.size == 0
            || ((pg.flags & PAGE_ENCRYPTED) && (pg.size & (AES_BLOCK_SIZE - 1)))) {
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
        if (pg.flags & PAGE_SIGNED) {
            uint8_t hash1[20], hash2[20];
            ret = shasum(tmpbuf, &hash1, pg.size);
            if (ret < 0) {
                *err = "hash";
                goto fail;
            }
            ret = rsa_decrypt(pg.s_cert, sig.sig, &hash2);
            if (ret < 0) {
                *err = "decrypt";
                goto fail;
            }
            if (memcmp(hash1, hash2, 20)) {
                ret = -EINVAL;
                *err = "verify";
                goto fail;
            }
        }
        if (pg.flags & PAGE_ENCRYPTED) {
            ret = aes_decrypt(&pg, &c, tmpbuf, page, pg.size);
            if (ret < 0) {
                *err = "decrypt";
                goto fail;
            }
        } else {
            memcpy(page, tmpbuf, pg.size);
        }
        if ((pg.flags & (PAGE_ALL)) != PAGE_WRITE) {
            ret = _mprotect(page, 0x1000, pg.flags & PAGE_ALL);
            if (ret != 0) {
                *err = "mprotect";
                goto fail;
            }
        }
        if (pg.flags & PAGE_EXEC) {
            if (!(pg.flags & PAGE_SIGNED)) {
                // unsigned pages are untrusted
                info->ctx = CTX_UNTRUSTED_APP;
            } else {
                // drop privilege level if necessary
                if (pg.s_cert == CRYPTO_CERT_SYSTEM) {
                    if (info->ctx < CTX_SYSTEM_APP) {
                        info->ctx = CTX_SYSTEM_APP;
                    }
                } else if (pg.s_cert == CRYPTO_CERT_PLATFORM) {
                    if (info->ctx < CTX_PLATFORM_APP) {
                        info->ctx = CTX_PLATFORM_APP;
                    }
                }
            }
            if (info->entry == 0) {
                info->entry = pg.start;
            }
        }
        regions[region_cnt].start = pg.start;
        regions[region_cnt].end = pg.start + 0x1000;
        regions[region_cnt].size = pg.size;
        region_cnt++;
    }
    if (info->entry == 0) {
        // no executable page
        ret = -ENOEXEC;
        *err = "noexec";
        goto fail;
    }
    Xcheckin(header.name, -1);
    *err = "ok";
    return 0;
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
        struct app_info info = {0};
        strncpy(name, PARAM_FOR(msg.from) + msg.start, sizeof(name));
        if (msg.size < sizeof(name)) {
            name[msg.size] = 0;
        }
        int ret = app_load(name, &err, &info);
        Xpost(msg.from, ret, err, strlen(err) + 1);
        if (ret == 0) {
            Xrunas(info.ctx);
            ((void (*)())((uint64_t)info.entry))();
        }
    }
    return 0;
}
