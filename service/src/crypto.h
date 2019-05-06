#ifndef _CRYPTO_H
#define _CRYPTO_H

enum crypto_type {
    CRYPTO_HASH_MD5,
    CRYPTO_HASH_SHA,
    CRYPTO_HASH_SHA256,
    CRYPTO_CIPHER_AES,
    CRYPTO_CIPHER_RSA,
};

struct crypto_request {
    enum crypto_type type;
    union {
        struct {
            uint32_t hash_data;
            uint32_t hash_data_size;
        };
        struct {
            uint32_t cipher_key;
            uint32_t cipher_data;
            uint32_t cipher_data_size;
            uint32_t cipher_iv;
            uint32_t cipher_iv_size;
        };
        // TODO signature
    };
};

#endif
