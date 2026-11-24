#ifndef AUREA_SDK_UI_H
#define AUREA_SDK_UI_H

#include <stddef.h>
#include <stdint.h>
#include "aurea/gui.h"

/*
 * Ring-3 AUI1 present protocol. Kernel maps ops to theme colors; apps do not
 * invent product-specific palettes. Hit-testing stays in user space.
 */

#define AUREA_UI_OP_FILL     1u
#define AUREA_UI_OP_LABEL    2u
#define AUREA_UI_OP_BUTTON   3u
#define AUREA_UI_OP_TEXTBOX  4u
#define AUREA_UI_OP_CARET    5u
#define AUREA_UI_OP_TEXT     6u
#define AUREA_UI_OP_HLINE    7u
/* Version byte stays 1. Ops 8–10 carry RGB in the unused flag/pad bytes.
 * Kernels that do not know them skip the opcode. Ops 1–7 are unchanged. */
#define AUREA_UI_OP_FILL_RGB 8u
#define AUREA_UI_OP_TEXT_RGB 9u
#define AUREA_UI_OP_STROKE   10u

#define AUREA_UI_FILL_BG       0u
#define AUREA_UI_FILL_TOOLBAR  1u
#define AUREA_UI_FILL_VIEWPORT 2u
#define AUREA_UI_FILL_STATUS   3u
#define AUREA_UI_FILL_ACCENT   4u
#define AUREA_UI_FILL_SEL      5u
#define AUREA_UI_FILL_MENU     6u

#define AUREA_UI_BTN_HOVER    1u
#define AUREA_UI_BTN_PRESS    2u
#define AUREA_UI_BTN_DISABLED 4u
#define AUREA_UI_BTN_ACCENT   8u

#define AUREA_UI_CHAR_W 8
#define AUREA_UI_LINE_H 16

typedef struct AureaUiRect {
    int x;
    int y;
    int w;
    int h;
} AureaUiRect;

typedef struct AureaUiBuilder {
    uint8_t *buf;
    size_t cap;
    size_t cmd_end;
    size_t str_used;
    uint16_t ncmd;
    int failed;
} AureaUiBuilder;

void aurea_ui_begin(AureaUiBuilder *b, void *buf, size_t cap);
void aurea_ui_fill(AureaUiBuilder *b, int x, int y, int w, int h, uint8_t role);
void aurea_ui_hline(AureaUiBuilder *b, int x, int y, int w);
void aurea_ui_label(AureaUiBuilder *b, int x, int y, int w, int h, const char *text, int muted);
void aurea_ui_button(AureaUiBuilder *b, int x, int y, int w, int h, const char *text, uint8_t flags);
void aurea_ui_textbox(AureaUiBuilder *b, int x, int y, int w, int h, const char *text, int focused);
void aurea_ui_caret(AureaUiBuilder *b, int x, int y, int h);
void aurea_ui_text(AureaUiBuilder *b, int x, int y, int w, int h, const char *text, uint8_t flags);
/* RGB is 0x00RRGGBB. Packed as B in flags, R in pad0, G in pad1. */
void aurea_ui_fill_rgb(AureaUiBuilder *b, int x, int y, int w, int h, uint32_t rgb);
void aurea_ui_text_rgb(AureaUiBuilder *b, int x, int y, int w, int h, const char *text, uint32_t rgb);
void aurea_ui_stroke(AureaUiBuilder *b, int x, int y, int w, int h, uint32_t rgb);
int aurea_ui_finish(AureaUiBuilder *b, size_t *out_len);

int aurea_ui_hit(int x, int y, int w, int h, int px, int py);
int aurea_ui_ptr_down(const AureaEvent *ev);
int aurea_ui_ptr_move(const AureaEvent *ev);
int aurea_ui_ptr_wheel(const AureaEvent *ev);

#endif /* AUREA_SDK_UI_H */
