#include <stddef.h>

#include "aurea/runtime.h"

void *memcpy(void *dst, const void *src, size_t n) {
    aurea_memcpy(dst, src, n);
    return dst;
}

void *memset(void *dst, int value, size_t n) {
    aurea_memset(dst, value, n);
    return dst;
}

int memcmp(const void *a, const void *b, size_t n) {
    return aurea_memcmp(a, b, n);
}

void *memmove(void *dst, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    size_t i;

    if (d == s || n == 0) {
        return dst;
    }
    if (d < s || d >= s + n) {
        aurea_memcpy(dst, src, n);
        return dst;
    }
    for (i = n; i > 0; i--) {
        d[i - 1] = s[i - 1];
    }
    return dst;
}

size_t strlen(const char *s) {
    return aurea_strlen(s);
}
