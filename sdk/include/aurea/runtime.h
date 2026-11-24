#ifndef AUREA_SDK_RUNTIME_H
#define AUREA_SDK_RUNTIME_H

#include <stddef.h>
#include <stdint.h>

size_t aurea_strlen(const char *s);
void aurea_memcpy(void *dst, const void *src, size_t n);
void aurea_memset(void *dst, int v, size_t n);
int aurea_memcmp(const void *a, const void *b, size_t n);
void aurea_write_cstr(int fd, const char *s);

/* Decode one UTF-8 codepoint; returns bytes consumed (0 on invalid/incomplete). */
size_t aurea_utf8_prev_len(const char *buf, size_t pos);
size_t aurea_utf8_next_len(const char *buf, size_t len, size_t pos);

#endif /* AUREA_SDK_RUNTIME_H */
