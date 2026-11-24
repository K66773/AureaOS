#ifndef AUREA_SDK_DIALOG_H
#define AUREA_SDK_DIALOG_H

#include <stdint.h>

#define AUREA_DIALOG_INFO          0
#define AUREA_DIALOG_WARNING       1
#define AUREA_DIALOG_ERROR         2
#define AUREA_DIALOG_CONFIRM       3
#define AUREA_DIALOG_QUESTION      4
#define AUREA_DIALOG_INPUT         5
#define AUREA_DIALOG_SAVE_CHANGES  6

#define AUREA_DIALOG_OK        1
#define AUREA_DIALOG_CANCEL    2
#define AUREA_DIALOG_YES       3
#define AUREA_DIALOG_NO        4
#define AUREA_DIALOG_ACCEPTED  5
#define AUREA_DIALOG_DISMISSED 6
#define AUREA_DIALOG_ERROR_RES 7

typedef struct AureaDialogIo {
    uint32_t result;
    char input[64];
} AureaDialogIo;

_Static_assert(sizeof(AureaDialogIo) == 68u, "AureaDialogIo size");

#endif /* AUREA_SDK_DIALOG_H */
