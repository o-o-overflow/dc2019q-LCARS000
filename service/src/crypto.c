#include "app.h"
#include "crypto.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#define WC_NO_HARDEN
#include <wolfssl/wolfcrypt/hash.h>

int app_main() {
    Xcheckin("crypto");
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
                    result = "access";
                } else {
                    ret = wc_Md5Hash(PARAM_FOR(msg.from) + req.hash_data, req.hash_data_size, &buf[0]);
                    result = &buf;
                    length = 16;
                }
                break;
            case CRYPTO_HASH_SHA:
                if (!access_ok(req.hash_data, req.hash_data_size)) {
                    result = "access";
                } else {
                    ret = wc_ShaHash(PARAM_FOR(msg.from) + req.hash_data, req.hash_data_size, &buf[0]);
                    result = &buf;
                    length = 20;
                }
                break;
            case CRYPTO_HASH_SHA256:
                if (!access_ok(req.hash_data, req.hash_data_size)) {
                    result = "access";
                } else {
                    ret = wc_Sha256Hash(PARAM_FOR(msg.from) + req.hash_data, req.hash_data_size, &buf[0]);
                    result = &buf;
                    length = 32;
                }
                break;
            default:
                break;
        }
end:
        Xpost(msg.from, ret, result, length);
    }
    return 0;
}
