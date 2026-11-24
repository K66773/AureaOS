#ifndef AUREA_SDK_SHA256_H
#define AUREA_SDK_SHA256_H

#include <stddef.h>
#include <stdint.h>

typedef struct AureaSha256Ctx {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    uint32_t datalen;
} AureaSha256Ctx;

void aurea_sha256_init(AureaSha256Ctx *ctx);
void aurea_sha256_update(AureaSha256Ctx *ctx, const void *data, size_t len);
void aurea_sha256_final(AureaSha256Ctx *ctx, uint8_t out[32]);
void aurea_sha256(const void *data, size_t len, uint8_t out[32]);
void aurea_sha256_hex(const uint8_t digest[32], char out[65]);

#endif
