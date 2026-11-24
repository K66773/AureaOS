#ifndef AUREA_SDK_STATUS_H
#define AUREA_SDK_STATUS_H

/* Public subset of the kernel status codes. Negative syscall results use these values. */
#define AUREA_OK                 0
#define AUREA_ERR_INVALID       -1
#define AUREA_ERR_NOT_FOUND     -2
#define AUREA_ERR_NO_MEMORY     -3
#define AUREA_ERR_IO            -4
#define AUREA_ERR_UNSUPPORTED   -5
#define AUREA_ERR_EXISTS        -6
#define AUREA_ERR_NOT_IMPL      -7
#define AUREA_ERR_DENIED        -8
#define AUREA_ERR_FAULT         -9
#define AUREA_ERR_EOF           -10
#define AUREA_ERR_TIMEOUT       -11
#define AUREA_ERR_BUSY          -12
#define AUREA_ERR_CONFIG        -13
#define AUREA_ERR_DEPENDENCY    -14
#define AUREA_ERR_HARDWARE_NOT_FOUND -15
#define AUREA_ERR_STATE         -16
#define AUREA_ERR_VERIFY        -17
#define AUREA_ERR_CANCELLED     -18

/* Maps to AUREA_ERR_STATE when no graphical interactive session. */
#define AUREA_UI_UNAVAILABLE    AUREA_ERR_STATE

#endif /* AUREA_SDK_STATUS_H */
