#include "aurea/runtime.h"
#include "aurea/syscall.h"

void aurea_write_cstr(int fd, const char *s) {
    aurea_write(fd, s, (long)aurea_strlen(s));
}
