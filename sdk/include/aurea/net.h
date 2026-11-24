#ifndef AUREA_SDK_NET_H
#define AUREA_SDK_NET_H

#include <stdint.h>
#include "aurea/abi.h"
#include "aurea/syscall.h"

/*
 * Public user socket / DNS ABI (IDs 29–35).
 * Implemented in kernel; requires network_connect capability.
 */

#define AUREA_AF_INET     2
#define AUREA_SOCK_STREAM 1
#define AUREA_SOCK_DGRAM  2

typedef uint64_t aurea_sock_t;

typedef struct AureaSockAddrIn {
    uint32_t struct_size;
    uint32_t abi_version;
    uint32_t addr; /* host-order IPv4 */
    uint16_t port; /* host-order */
    uint16_t reserved;
} AureaSockAddrIn;

_Static_assert(sizeof(AureaSockAddrIn) == 16u, "AureaSockAddrIn size");

typedef struct AureaDnsQuery {
    uint32_t struct_size;
    uint32_t abi_version;
    char hostname[256];
    uint32_t addr_out;
    int32_t status;
} AureaDnsQuery;

_Static_assert(sizeof(AureaDnsQuery) == 272u, "AureaDnsQuery size");

static inline void aurea_sockaddr_in_init(AureaSockAddrIn *a, uint32_t addr, uint16_t port) {
    if (!a) {
        return;
    }
    a->struct_size = (uint32_t)sizeof(AureaSockAddrIn);
    a->abi_version = AUREA_RUNTIME_ABI_V1;
    a->addr = addr;
    a->port = port;
    a->reserved = 0;
}

static inline long aurea_socket_create(long domain, long type, long proto, aurea_sock_t *out) {
    if (!out) {
        return AUREA_ERR_INVALID;
    }
    return aurea_syscall4(AUREA_SYS_SOCKET_CREATE, domain, type, proto, (long)out);
}

static inline long aurea_socket_connect(aurea_sock_t sock, const AureaSockAddrIn *addr) {
    if (!addr || addr->struct_size < sizeof(AureaSockAddrIn)) {
        return AUREA_ERR_INVALID;
    }
    return aurea_syscall4(AUREA_SYS_SOCKET_CONNECT, (long)sock, (long)addr, 0, 0);
}

static inline long aurea_socket_send(aurea_sock_t sock, const void *buf, long len) {
    return aurea_syscall4(AUREA_SYS_SOCKET_SEND, (long)sock, (long)buf, len, 0);
}

static inline long aurea_socket_recv(aurea_sock_t sock, void *buf, long len) {
    return aurea_syscall4(AUREA_SYS_SOCKET_RECV, (long)sock, (long)buf, len, 0);
}

static inline long aurea_socket_shutdown(aurea_sock_t sock, long how) {
    return aurea_syscall4(AUREA_SYS_SOCKET_SHUTDOWN, (long)sock, how, 0, 0);
}

static inline long aurea_socket_close(aurea_sock_t sock) {
    return aurea_syscall4(AUREA_SYS_SOCKET_CLOSE, (long)sock, 0, 0, 0);
}

static inline long aurea_dns_resolve(AureaDnsQuery *q) {
    if (!q || q->struct_size < sizeof(AureaDnsQuery) || q->abi_version != AUREA_RUNTIME_ABI_V1) {
        return AUREA_ERR_INVALID;
    }
    return aurea_syscall4(AUREA_SYS_DNS_RESOLVE, (long)q, 0, 0, 0);
}

#endif /* AUREA_SDK_NET_H */
