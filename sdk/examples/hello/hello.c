#include "aurea/runtime.h"
#include "aurea/syscall.h"

int aurea_app_main(int argc, char **argv) {
    aurea_write_cstr(1, "Hello from an Aurea SDK application.\r\n");
    long pid = aurea_getpid();
    char line[48] = "pid: ";
    size_t n = aurea_strlen(line);
    char digits[20];
    size_t d = 0;
    if (pid < 0) {
        pid = 0;
    }
    do {
        digits[d++] = (char)('0' + (pid % 10));
        pid /= 10;
    } while (pid > 0 && d < sizeof(digits));
    while (d > 0) {
        line[n++] = digits[--d];
    }
    line[n++] = '\r';
    line[n++] = '\n';
    line[n] = '\0';
    aurea_write_cstr(1, line);
    for (int i = 1; i < argc; i++) {
        aurea_write_cstr(1, "arg: ");
        aurea_write_cstr(1, argv[i]);
        aurea_write_cstr(1, "\r\n");
    }
    return 0;
}
