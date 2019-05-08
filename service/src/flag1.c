#include "app.h"
#include "crypto.h"
#include "flag1.h"
#include <stdio.h>
#include <string.h>

#define AES_BLOCK_SIZE 0x10

static int crypto_service;

static void hexencode(void *out, const void *raw, uint32_t size) {
    for (int i = 0; i < size; i++) {
        sprintf(out + i * 2, "%02x", ((uint8_t *)raw)[i]);
    }
    ((char *)out)[size * 2] = 0;
}

static int hash(int mode, const void *in, void *out, uint32_t size) {
    struct crypto_request req = {0};
    msg_t m = {0};
    req.type = mode;
    req.hash_data = shm_alloc(size);
    req.hash_data_size = size;
    memcpy(PARAM_AT(req.hash_data), in, size);
    int ret = Xpost(crypto_service, 'SECD', &req, sizeof(req));
    if (ret < 0) {
        return ret;
    }
    Xwait(crypto_service, -1, &m);
    if (m.type == 0) {
        hexencode(out, PARAM_FOR(m.from) + m.start, m.size);
    }
    return m.type;
}

static int aes(int mode, int enc, int64_t key, const char *in, void *out, uint32_t size, char *iv) {
    struct crypto_request req = {0};
    msg_t m = {0};
    req.type = enc ? CRYPTO_ENCRYPT_AES: CRYPTO_DECRYPT_AES;
    req.cipher_size = 256;
    if (key < 4) {
        req.cipher_key_id = key;
    } else {
        req.cipher_key_id = CRYPTO_KEY_USER;
        req.cipher_key = shm_alloc(sizeof(AES_KEY));
        memcpy(PARAM_AT(req.cipher_key), (void *)key, sizeof(AES_KEY));
    }
    req.cipher_mode = mode;
    req.cipher_data = shm_alloc(size);
    req.cipher_data_size = size;
    memcpy(PARAM_AT(req.cipher_data), in, size);
    if (iv != NULL) {
        req.cipher_iv = shm_alloc(AES_BLOCK_SIZE);
        req.cipher_iv_size = AES_BLOCK_SIZE;
        memcpy(PARAM_AT(req.cipher_iv), iv, AES_BLOCK_SIZE);
    }
    int ret = Xpost(crypto_service, 'SECD', &req, sizeof(req));
    if (ret < 0) {
        return ret;
    }
    Xwait(crypto_service, -1, &m);
    if (m.type == 0) {
        hexencode(out, PARAM_FOR(m.from) + m.start, m.size);
    } else {
        Xecho(PARAM_FOR(m.from) + m.start);
    }
    return m.type;
}

int app_main() {
    crypto_service = Xlookup("crypto");
    char buf[0x100];
    char H[0x100];
    hash(CRYPTO_HASH_MD5, &flag1_txt, &H, flag1_txt_len);
    snprintf(&buf[0], sizeof(buf), "md5: %s\n", &H[0]);
    Xecho(&buf[0]);
    hash(CRYPTO_HASH_SHA, &flag1_txt, &H, flag1_txt_len);
    snprintf(&buf[0], sizeof(buf), "sha1: %s\n", &H[0]);
    Xecho(&buf[0]);
    hash(CRYPTO_HASH_SHA256, &flag1_txt, &H, flag1_txt_len);
    snprintf(&buf[0], sizeof(buf), "sha256: %s\n", &H[0]);
    Xecho(&buf[0]);
    aes(CRYPTO_MODE_ECB, 1, CRYPTO_KEY_SESSION, &flag1_txt[0], &H[0], (flag1_txt_len + 0xf) / 0x10 * 0x10, NULL);
    snprintf(&buf[0], sizeof(buf), "encrypted: %s\n", &H[0]);
    Xecho(&buf[0]);
    return 0;
}
