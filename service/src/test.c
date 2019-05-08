#include "app.h"
#include "crypto.h"
#include "certs.h"
#include "certs-test.h"
#include <stdio.h>
#include <string.h>

#define AES_BLOCK_SIZE 0x10

static int echo_service;
static int crypto_service;

static void hexdump(const void *raw, uint32_t size) {
    char buf[0x400];
    for (int i = 0; i < size; i++) {
        sprintf(&buf[i * 2], "%02x", ((uint8_t *)raw)[i]);
    }
    Xecho(buf);
}

static int test_hash(int mode, const void *in, uint32_t size) {
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
        hexdump(PARAM_FOR(m.from) + m.start, m.size);
    } else {
        Xecho(PARAM_FOR(m.from) + m.start);
    }
    return m.type;
}

static int test_aes(int mode, int enc, int64_t key, char *in, uint32_t size, char *iv) {
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
        hexdump(PARAM_FOR(m.from) + m.start, m.size);
    } else {
        Xecho(PARAM_FOR(m.from) + m.start);
    }
    return m.type;
}

static int test_rsa(int cert, const void *in, const uint32_t size) {
    struct crypto_request req = {0};
    msg_t m = {0};
    req.type = CRYPTO_DECRYPT_RSA;
    req.sig_cert_id = cert;
    req.sig_data = shm_alloc(size);
    req.sig_data_size = size;
    memcpy(PARAM_AT(req.sig_data), in, size);
    int ret = Xpost(crypto_service, 'SECD', &req, sizeof(req));
    if (ret < 0) {
        return ret;
    }
    Xwait(crypto_service, -1, &m);
    Xecho(PARAM_FOR(m.from) + m.start);
    return m.type;
}

static void test_echo() {
    const char *msg[] = {
        "hello world",
        "test",
        "0x4141414141",
    };
    for (int i = 0; i < sizeof(msg) / sizeof(msg[0]); i++) {
        Xpost(echo_service, -1, msg[i], strlen(msg[i]) + 1);
    }
}

int app_main() {
    echo_service = Xlookup("echo");
    crypto_service = Xlookup("crypto");
    char zero[0x40] = {0};
    char A[0x10] = {0};
    memset(&A, 'A', sizeof(A));
    test_hash(CRYPTO_HASH_MD5, &zero, 0x10);
    test_hash(CRYPTO_HASH_SHA, &zero, 0x10);
    test_hash(CRYPTO_HASH_SHA256, &zero, 0x10);
    test_aes(CRYPTO_MODE_ECB, 1, CRYPTO_KEY_ROOT, zero, 0x10, NULL);
    test_aes(CRYPTO_MODE_ECB, 0, CRYPTO_KEY_ROOT, zero, 0x10, NULL);
    test_aes(CRYPTO_MODE_ECB, 0, CRYPTO_KEY_PROVISION, zero, 0x10, NULL);
    test_aes(CRYPTO_MODE_ECB, 0, CRYPTO_KEY_SESSION, zero, 0x10, NULL);
    test_aes(CRYPTO_MODE_CBC, 0, CRYPTO_KEY_PROVISION, zero, 0x10, NULL);
    test_aes(CRYPTO_MODE_CBC, 0, CRYPTO_KEY_PROVISION, zero, 0x40, zero);
    test_aes(CRYPTO_MODE_CBC, 0, CRYPTO_KEY_PROVISION, zero, 0x40, A);
    test_aes(CRYPTO_MODE_CBC, 1, CRYPTO_KEY_PROVISION, zero, 0x40, A);
    test_rsa(CRYPTO_CERT_SYSTEM, test_system, test_system_len);
    test_rsa(CRYPTO_CERT_PLATFORM, test_platform, test_platform_len);
    return 0;
}
