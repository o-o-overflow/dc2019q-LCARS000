#include "scmp.h"
#include "policy.h"
#include <errno.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>

int load_policy(enum app_ctx ctx) {
    struct filter {
        uint64_t size;
        const void *bpf;
    } filter = {0};
    switch (ctx) {
        case CTX_KERNEL:
            break;
        case CTX_SYSTEM_APP:
            filter.size = system_bpf_len / 8;
            filter.bpf = system_bpf;
            break;
        case CTX_PLATFORM_APP:
            filter.size = platform_bpf_len / 8;
            filter.bpf = platform_bpf;
            break;
        case CTX_UNTRUSTED_APP:
            filter.size = untrusted_bpf_len / 8;
            filter.bpf = untrusted_bpf;
            break;
        default:
            return -EINVAL;
    }
    prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
    if (filter.size != 0) {
        return prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &filter);
    } else {
        return 0;
    }
}
