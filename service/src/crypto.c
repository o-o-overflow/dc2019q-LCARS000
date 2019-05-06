#include "app.h"
#include "crypto.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#define WC_NO_HARDEN
#define WOLFSSL_AES_DIRECT
#include <wolfssl/wolfcrypt/hash.h>
#include <wolfssl/wolfcrypt/aes.h>

static AES_KEY keys[4];

// hack for wolfssl
void *__memcpy_chk(void *dst, const void *src, uint64_t size, uint64_t max_size) {
    return memcpy(dst, src, size);
}

int app_main() {
    Xcheckin("crypto", -1);

    // register keys
    memset(&keys, 0, sizeof(keys));
    // ROOT KEY: load from filesystem
    int fd = Xopen("root.key");
    if (fd >= 0) {
        read_all(fd, &keys[0], sizeof(AES_KEY));
        _close(fd);
    }
    // PROVISION KEY: hardcoded
    memset(&keys[1], 'O', sizeof(AES_KEY));
    // USER KEY: user controlled
    memset(&keys[2], 'U', sizeof(AES_KEY));
    // SESSION KEY: generated for this session
    fd = Xopen("/dev/urandom");
    if (fd >= 0) {
        read_all(fd, &keys[3], sizeof(AES_KEY));
        _close(fd);
    }

    Xrunas(CTX_UNTRUSTED_APP);

    msg_t msg;
    struct crypto_request req;
    uint8_t buf[0x100];
    while (Xwait(-1, 'SECD', &msg) == 0) {
        int ret = 0;
        void *result = "";
        uint32_t length = 0;
        ret = -EINVAL;
        if (msg.size < sizeof(req)) {
            result = "truncated";
            goto end;
        }
        memcpy(&req, PARAM_FOR(msg.from) + msg.start, sizeof(req));
        switch (req.type) {
            case CRYPTO_HASH_MD5:
                if (!access_ok(req.hash_data, req.hash_data_size)) {
                    ret = -EFAULT;
                    result = "access";
                } else {
                    wc_Md5Hash(PARAM_FOR(msg.from) + req.hash_data, req.hash_data_size, &buf[0]);
                    ret = 0;
                    result = &buf;
                    length = 16;
                }
                break;
            case CRYPTO_HASH_SHA:
                if (!access_ok(req.hash_data, req.hash_data_size)) {
                    ret = -EFAULT;
                    result = "access";
                } else {
                    wc_ShaHash(PARAM_FOR(msg.from) + req.hash_data, req.hash_data_size, &buf[0]);
                    ret = 0;
                    result = &buf;
                    length = 20;
                }
                break;
            case CRYPTO_HASH_SHA256:
                if (!access_ok(req.hash_data, req.hash_data_size)) {
                    ret = -EFAULT;
                    result = "access";
                } else {
                    wc_Sha256Hash(PARAM_FOR(msg.from) + req.hash_data, req.hash_data_size, &buf[0]);
                    ret = 0;
                    result = &buf;
                    length = 32;
                }
                break;
            case CRYPTO_ENCRYPT_AES:
            case CRYPTO_DECRYPT_AES:
                if (!access_ok(req.cipher_data, req.cipher_data_size)) {
                    ret = -EFAULT;
                    result = "access";
                } else if (req.cipher_size != 256 || req.cipher_data_size & (AES_BLOCK_SIZE - 1)) {
                    ret = -EINVAL;
                    result = "size";
                } else if (req.cipher_key_id >= 4) {
                    result = "key";
                } else {
                    uint32_t a = shm_alloc(req.cipher_data_size);
                    if (a == -1) {
                        result = "shmem";
                        ret = -ENOMEM;
                        break;
                    }
                    if (req.cipher_key_id == CRYPTO_KEY_USER) {
                        if (req.cipher_key_size != sizeof(AES_KEY)
                        || !access_ok(req.cipher_key, req.cipher_key_size)) {
                            ret = -EFAULT;
                            result = "userkey";
                            break;
                        }
                        memcpy(&keys[CRYPTO_KEY_USER], PARAM_FOR(msg.from) + req.cipher_key, sizeof(AES_KEY));
                    } else if ((req.cipher_key_id == CRYPTO_KEY_ROOT
                                || req.cipher_key_id == CRYPTO_KEY_SESSION)
                            && req.type == CRYPTO_ENCRYPT_AES) {
                        // root/session keys are unknown to user, they are
                        // not alloed in encryption
                        ret = -EPERM;
                        result = "denied";
                        break;
                    }
                    Aes aes;
                    wc_AesInit(&aes, NULL, INVALID_DEVID);
                    uint8_t IV[AES_BLOCK_SIZE] = {0};
                    if (req.cipher_mode == CRYPTO_MODE_CBC) {
                        if (req.cipher_iv_size != sizeof(IV)
                        || !access_ok(req.cipher_iv, req.cipher_iv_size)) {
                            ret = -EFAULT;
                            result = "iv";
                            break;
                        }
                        memcpy(&IV, PARAM_FOR(msg.from) + req.cipher_iv, sizeof(IV));
                    }
                    int dir = req.type == CRYPTO_ENCRYPT_AES ? AES_ENCRYPTION : AES_DECRYPTION;
                    if (wc_AesSetKey(&aes, keys[req.cipher_key_id], sizeof(AES_KEY), &IV[0], dir) != 0) {
                        ret = -EINVAL;
                        result = "bad func arg";
                        break;
                    }
                    switch (req.cipher_mode) {
                        case CRYPTO_MODE_ECB:
                            for (int i = 0; i < req.cipher_data_size; i += AES_BLOCK_SIZE) {
                                if (dir == AES_ENCRYPTION) {
                                    wc_AesEncryptDirect(&aes, PARAM_AT(a) + i, PARAM_FOR(msg.from) + req.cipher_data + i);
                                } else {
                                    wc_AesDecryptDirect(&aes, PARAM_AT(a) + i, PARAM_FOR(msg.from) + req.cipher_data + i);
                                }
                            }
                            ret = 0;
                            break;
                        case CRYPTO_MODE_CBC:
                            if (dir == AES_ENCRYPTION) {
                                wc_AesCbcEncrypt(&aes, PARAM_AT(a), PARAM_FOR(msg.from) + req.cipher_data, req.cipher_data_size);
                            } else {
                                wc_AesCbcDecrypt(&aes, PARAM_AT(a), PARAM_FOR(msg.from) + req.cipher_data, req.cipher_data_size);
                            }
                            ret = 0;
                            break;
                        default:
                            result = "mode";
                            break;
                    }
                    if (ret == 0) {
                        result = PARAM_AT(a);
                        length = req.cipher_data_size;
                    }
                }
                break;
            default:
                result = "unsupported";
                break;
        }
end:
        if (ret != 0) {
            length = strlen(result) + 1;
        }
        Xpost(msg.from, ret, result, length);
    }
    return 0;
}
