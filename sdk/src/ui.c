#include "aurea/ui.h"
#include "aurea/runtime.h"

static void ui_fail(AureaUiBuilder *b) {
    if (b) {
        b->failed = 1;
    }
}

void aurea_ui_begin(AureaUiBuilder *b, void *buf, size_t cap) {
    if (!b) {
        return;
    }
    aurea_memset(b, 0, sizeof(*b));
    b->buf = (uint8_t *)buf;
    b->cap = cap;
    b->cmd_end = 8;
    b->str_used = cap; /* strings grow downward from cap */
    if (!buf || cap < 64u) {
        b->failed = 1;
        return;
    }
    aurea_memset(buf, 0, cap);
    b->buf[0] = 'A';
    b->buf[1] = 'U';
    b->buf[2] = 'I';
    b->buf[3] = '1';
    b->buf[4] = 1;
}

static uint16_t ui_put_str(AureaUiBuilder *b, const char *text, uint16_t *len_out) {
    size_t n;
    size_t i;
    size_t at;
    if (!b || b->failed) {
        return 0;
    }
    if (!text) {
        text = "";
    }
    n = aurea_strlen(text);
    if (n > 240u) {
        n = 240u;
    }
    if (b->cmd_end + 16u + n + 1u > b->str_used) {
        ui_fail(b);
        return 0;
    }
    at = b->str_used - (n + 1u);
    for (i = 0; i < n; i++) {
        b->buf[at + i] = (uint8_t)text[i];
    }
    b->buf[at + n] = 0;
    b->str_used = at;
    if (len_out) {
        *len_out = (uint16_t)n;
    }
    return (uint16_t)at;
}

static void ui_cmd_rgb(AureaUiBuilder *b, uint8_t op, uint8_t flags, uint8_t r, uint8_t g,
                       int x, int y, int w, int h, const char *text) {
    uint16_t off = 0;
    uint16_t n = 0;
    uint8_t *p;
    if (!b || b->failed) {
        return;
    }
    if (b->cmd_end + 16u > b->str_used) {
        ui_fail(b);
        return;
    }
    if (text && text[0]) {
        off = ui_put_str(b, text, &n);
        if (b->failed) {
            return;
        }
    }
    p = b->buf + b->cmd_end;
    p[0] = op;
    p[1] = flags;
    p[2] = r;
    p[3] = g;
    p[4] = (uint8_t)((uint16_t)x & 0xffu);
    p[5] = (uint8_t)(((uint16_t)x >> 8) & 0xffu);
    p[6] = (uint8_t)((uint16_t)y & 0xffu);
    p[7] = (uint8_t)(((uint16_t)y >> 8) & 0xffu);
    p[8] = (uint8_t)((uint16_t)w & 0xffu);
    p[9] = (uint8_t)(((uint16_t)w >> 8) & 0xffu);
    p[10] = (uint8_t)((uint16_t)h & 0xffu);
    p[11] = (uint8_t)(((uint16_t)h >> 8) & 0xffu);
    p[12] = (uint8_t)(off & 0xffu);
    p[13] = (uint8_t)((off >> 8) & 0xffu);
    p[14] = (uint8_t)(n & 0xffu);
    p[15] = (uint8_t)((n >> 8) & 0xffu);
    b->cmd_end += 16u;
    b->ncmd++;
}

static void ui_cmd(AureaUiBuilder *b, uint8_t op, uint8_t flags, int x, int y, int w, int h,
                   const char *text) {
    ui_cmd_rgb(b, op, flags, 0, 0, x, y, w, h, text);
}

void aurea_ui_fill(AureaUiBuilder *b, int x, int y, int w, int h, uint8_t role) {
    ui_cmd(b, AUREA_UI_OP_FILL, role, x, y, w, h, 0);
}

void aurea_ui_hline(AureaUiBuilder *b, int x, int y, int w) {
    ui_cmd(b, AUREA_UI_OP_HLINE, 0, x, y, w, 1, 0);
}

void aurea_ui_label(AureaUiBuilder *b, int x, int y, int w, int h, const char *text, int muted) {
    ui_cmd(b, AUREA_UI_OP_LABEL, muted ? 1u : 0u, x, y, w, h, text);
}

void aurea_ui_button(AureaUiBuilder *b, int x, int y, int w, int h, const char *text, uint8_t flags) {
    ui_cmd(b, AUREA_UI_OP_BUTTON, flags, x, y, w, h, text);
}

void aurea_ui_textbox(AureaUiBuilder *b, int x, int y, int w, int h, const char *text, int focused) {
    ui_cmd(b, AUREA_UI_OP_TEXTBOX, focused ? 1u : 0u, x, y, w, h, text);
}

void aurea_ui_caret(AureaUiBuilder *b, int x, int y, int h) {
    ui_cmd(b, AUREA_UI_OP_CARET, 0, x, y, 2, h, 0);
}

void aurea_ui_text(AureaUiBuilder *b, int x, int y, int w, int h, const char *text, uint8_t flags) {
    ui_cmd(b, AUREA_UI_OP_TEXT, flags, x, y, w, h, text);
}

static void ui_split_rgb(uint32_t rgb, uint8_t *r, uint8_t *g, uint8_t *b) {
    *r = (uint8_t)((rgb >> 16) & 0xffu);
    *g = (uint8_t)((rgb >> 8) & 0xffu);
    *b = (uint8_t)(rgb & 0xffu);
}

void aurea_ui_fill_rgb(AureaUiBuilder *b, int x, int y, int w, int h, uint32_t rgb) {
    uint8_t r, g, bl;
    ui_split_rgb(rgb, &r, &g, &bl);
    ui_cmd_rgb(b, AUREA_UI_OP_FILL_RGB, bl, r, g, x, y, w, h, 0);
}

void aurea_ui_text_rgb(AureaUiBuilder *b, int x, int y, int w, int h, const char *text, uint32_t rgb) {
    uint8_t r, g, bl;
    ui_split_rgb(rgb, &r, &g, &bl);
    ui_cmd_rgb(b, AUREA_UI_OP_TEXT_RGB, bl, r, g, x, y, w, h, text);
}

void aurea_ui_stroke(AureaUiBuilder *b, int x, int y, int w, int h, uint32_t rgb) {
    uint8_t r, g, bl;
    ui_split_rgb(rgb, &r, &g, &bl);
    ui_cmd_rgb(b, AUREA_UI_OP_STROKE, bl, r, g, x, y, w, h, 0);
}

int aurea_ui_finish(AureaUiBuilder *b, size_t *out_len) {
    if (!b || b->failed || !b->buf) {
        return -1;
    }
    b->buf[6] = (uint8_t)(b->ncmd & 0xffu);
    b->buf[7] = (uint8_t)((b->ncmd >> 8) & 0xffu);
    if (out_len) {
        *out_len = b->cap;
    }
    return 0;
}

int aurea_ui_hit(int x, int y, int w, int h, int px, int py) {
    return px >= x && py >= y && px < x + w && py < y + h;
}

int aurea_ui_ptr_down(const AureaEvent *ev) {
    return ev && ev->type == AUREA_EVENT_POINTER && ev->flags == AUREA_POINTER_DOWN;
}

int aurea_ui_ptr_move(const AureaEvent *ev) {
    return ev && ev->type == AUREA_EVENT_POINTER && ev->flags == AUREA_POINTER_MOVE;
}

int aurea_ui_ptr_wheel(const AureaEvent *ev) {
    return ev && ev->type == AUREA_EVENT_POINTER && ev->flags == AUREA_POINTER_WHEEL;
}
