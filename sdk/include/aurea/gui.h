#ifndef AUREA_SDK_GUI_H
#define AUREA_SDK_GUI_H

#include <stdint.h>
#include "aurea/abi.h"
#include "aurea/syscall_ids.h"
#include "aurea/status.h"

/*
 * GUI architecture: server-side windows with process-owned opaque handles +
 * event queue. No kernel pointers or app callbacks in the ABI.
 *
 * Client paint/present: aurea_window_present copies UTF-8 text into the
 * owned window surface (bounded). Pixel framebuffers are not exposed.
 */

typedef uint64_t aurea_window_t;

typedef struct AureaWindowCreate {
    uint32_t struct_size;
    uint32_t abi_version;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    char title[64];
} AureaWindowCreate;

_Static_assert(sizeof(AureaWindowCreate) == 88u, "AureaWindowCreate size");

typedef enum AureaEventType {
    AUREA_EVENT_NONE = 0,
    AUREA_EVENT_KEY = 1,
    AUREA_EVENT_TEXT = 2,
    AUREA_EVENT_POINTER = 3,
    AUREA_EVENT_RESIZE = 4,
    AUREA_EVENT_FOCUS = 5,
    AUREA_EVENT_CLOSE = 6,
    AUREA_EVENT_DIALOG = 7
} AureaEventType;

/*
 * AureaEvent.keycode on AUREA_EVENT_KEY — semantic ids from the input layer.
 * These are NOT PS/2 scancodes (e.g. 0x0E/0x1C). Match kernel aurea_key_event_type_t.
 */
#define AUREA_KEYCODE_NONE      0u
#define AUREA_KEYCODE_CHAR      1u
#define AUREA_KEYCODE_BACKSPACE 2u
#define AUREA_KEYCODE_ENTER     3u
#define AUREA_KEYCODE_ESCAPE    4u
#define AUREA_KEYCODE_TAB       5u
#define AUREA_KEYCODE_UP        6u
#define AUREA_KEYCODE_DOWN      7u
#define AUREA_KEYCODE_LEFT      8u
#define AUREA_KEYCODE_RIGHT     9u
#define AUREA_KEYCODE_F4        10u
#define AUREA_KEYCODE_F2        11u
#define AUREA_KEYCODE_F5        12u
#define AUREA_KEYCODE_DELETE    13u
#define AUREA_KEYCODE_HOME      14u
#define AUREA_KEYCODE_END       15u
#define AUREA_KEYCODE_PAGE_UP   16u
#define AUREA_KEYCODE_PAGE_DOWN 17u

#define AUREA_MOD_SHIFT 1u
#define AUREA_MOD_CTRL  2u
#define AUREA_MOD_ALT   4u

#define AUREA_POINTER_MOVE  2u
#define AUREA_POINTER_DOWN  3u
#define AUREA_POINTER_UP    4u
#define AUREA_POINTER_WHEEL 5u

typedef struct AureaEvent {
    uint32_t struct_size;
    uint32_t abi_version;
    uint32_t type;          /* AureaEventType */
    uint32_t flags;
    aurea_window_t window;
    uint64_t sequence;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    uint32_t keycode;       /* AUREA_KEYCODE_* on KEY events */
    uint32_t modifiers;
    char text_utf8[8];      /* decoded text; empty for pure key events */
} AureaEvent;

_Static_assert(sizeof(AureaEvent) == 64u, "AureaEvent size");

#endif /* AUREA_SDK_GUI_H */
