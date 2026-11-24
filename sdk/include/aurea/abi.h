#ifndef AUREA_SDK_ABI_H
#define AUREA_SDK_ABI_H

#include <stdint.h>

/* Runtime / UI ABI identity — independent of OS VERSION string. */
#define AUREA_SDK_VERSION_MAJOR 1u
#define AUREA_SDK_VERSION_MINOR 0u
#define AUREA_SDK_VERSION_PATCH 0u

#define AUREA_SYSCALL_ABI_VERSION 1u
#define AUREA_RUNTIME_ABI_V1      1u
#define AUREA_UI_ABI_V1           1u
#define AUREA_GUI_ABI_V1          1u

/* Wire struct versioning: always set struct_size = sizeof(...). */
typedef struct AureaAbiHeader {
    uint32_t struct_size;
    uint32_t abi_version;
} AureaAbiHeader;

_Static_assert(sizeof(AureaAbiHeader) == 8u, "AureaAbiHeader size");

#endif /* AUREA_SDK_ABI_H */
