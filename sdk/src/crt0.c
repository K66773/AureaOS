#include "aurea/syscall.h"

extern int aurea_app_main(int argc, char **argv);

/* Entered with RSP = argc frame from aex_loader (not via CALL). */
__attribute__((naked)) void _start(void) {
    __asm__ volatile(
        "movq (%%rsp), %%rdi\n\t"
        "leaq 8(%%rsp), %%rsi\n\t"
        "call aurea_app_main\n\t"
        "movq %%rax, %%rdi\n\t"
        "movq $0, %%rax\n\t" /* AUREA_SYS_EXIT */
        "int $0x80\n\t"
        "1: hlt\n\t"
        "jmp 1b"
        :
        :
        : "memory");
}
