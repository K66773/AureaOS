#include "aurea/runtime.h"

size_t aurea_strlen(const char *s) {
    size_t n = 0;
    if (!s) {
        return 0;
    }
    while (s[n]) {
        n++;
    }
    return n;
}

void aurea_memcpy(void *dst, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    size_t i;
    for (i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

void aurea_memset(void *dst, int v, size_t n) {
    unsigned char *d = (unsigned char *)dst;
    size_t i;
    for (i = 0; i < n; i++) {
        d[i] = (unsigned char)v;
    }
}

int aurea_memcmp(const void *a, const void *b, size_t n) {
    const unsigned char *x = (const unsigned char *)a;
    const unsigned char *y = (const unsigned char *)b;
    size_t i;
    for (i = 0; i < n; i++) {
        if (x[i] != y[i]) {
            return (int)x[i] - (int)y[i];
        }
    }
    return 0;
}

size_t aurea_utf8_next_len(const char *buf, size_t len, size_t pos) {
    unsigned char c;
    if (!buf || pos >= len) {
        return 0;
    }
    c = (unsigned char)buf[pos];
    if (c < 0x80u) {
        return 1;
    }
    if ((c & 0xE0u) == 0xC0u && pos + 1 < len) {
        return 2;
    }
    if ((c & 0xF0u) == 0xE0u && pos + 2 < len) {
        return 3;
    }
    if ((c & 0xF8u) == 0xF0u && pos + 3 < len) {
        return 4;
    }
    return 1;
}

size_t aurea_utf8_prev_len(const char *buf, size_t pos) {
    size_t i;
    if (!buf || pos == 0) {
        return 0;
    }
    i = pos - 1;
    while (i > 0 && ((unsigned char)buf[i] & 0xC0u) == 0x80u) {
        i--;
    }
    return pos - i;
}
