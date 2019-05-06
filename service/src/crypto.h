#ifndef _CRYPTO_H
#define _CRYPTO_H

typedef uint8_t AES_KEY[0x20];

enum crypto_type {
    CRYPTO_HASH_MD5,
    CRYPTO_HASH_SHA,
    CRYPTO_HASH_SHA256,
    CRYPTO_ENCRYPT_AES,
    CRYPTO_DECRYPT_AES,
    CRYPTO_CIPHER_RSA,
};

enum crypto_key {
    CRYPTO_KEY_ROOT,
    CRYPTO_KEY_PROVISION,
    CRYPTO_KEY_USER,
    CRYPTO_KEY_SESSION,
};

enum crypto_mode {
    CRYPTO_MODE_ECB,
    CRYPTO_MODE_CBC,
};

struct crypto_request {
    enum crypto_type type;
    union {
        struct {
            uint32_t hash_data;
            uint32_t hash_data_size;
        };
        struct {
            uint8_t cipher_key_id;
            uint8_t cipher_mode;
            uint16_t cipher_size;
            uint32_t cipher_data;
            uint32_t cipher_data_size;
            uint32_t cipher_key;
            uint32_t cipher_key_size;
            uint32_t cipher_iv;
            uint32_t cipher_iv_size;
        };
        // TODO signature
    };
};

#endif
