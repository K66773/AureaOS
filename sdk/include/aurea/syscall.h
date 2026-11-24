#ifndef AUREA_SDK_SYSCALL_H
#define AUREA_SDK_SYSCALL_H

#include <stdint.h>
#include <stddef.h>
#include "aurea/syscall_ids.h"
#include "aurea/status.h"
#include "aurea/dialog.h"
#include "aurea/file.h"
#include "aurea/known_paths.h"
#include "aurea/gui.h"

typedef struct AureaWhoAmI {
    uint32_t user_id;
    uint32_t role;
    uint32_t session_id;
    char username[32];
} AureaWhoAmI;

_Static_assert(sizeof(AureaWhoAmI) == 44u, "AureaWhoAmI size");

static inline long aurea_syscall4(long num, long a0, long a1, long a2, long a3) {
    long ret;
    register long r10 __asm__("r10") = a3;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "D"(a0), "S"(a1), "d"(a2), "r"(r10)
        : "rcx", "r11", "memory");
    return ret;
}

static inline void aurea_write(int fd, const char *buf, long len) {
    (void)aurea_syscall4(AUREA_SYS_WRITE, fd, (long)buf, len, 0);
}

static inline void aurea_exit(int code) {
    (void)aurea_syscall4(AUREA_SYS_EXIT, code, 0, 0, 0);
}

static inline long aurea_getpid(void) {
    return aurea_syscall4(AUREA_SYS_GETPID, 0, 0, 0, 0);
}

static inline long aurea_open(const char *path, long flags) {
    return aurea_syscall4(AUREA_SYS_OPEN, (long)path, flags, 0, 0);
}

static inline long aurea_read(long handle, void *buf, long len) {
    return aurea_syscall4(AUREA_SYS_READ, handle, (long)buf, len, 0);
}

static inline long aurea_file_write(long handle, const void *buf, long len) {
    return aurea_syscall4(AUREA_SYS_FILE_WRITE, handle, (long)buf, len, 0);
}

static inline long aurea_close(long handle) {
    return aurea_syscall4(AUREA_SYS_CLOSE, handle, 0, 0, 0);
}

/* Negative syscall results are status codes (see aurea/status.h), not pointers. */
static inline void *aurea_alloc(long size) {
    long ret = aurea_syscall4(AUREA_SYS_ALLOC, size, 0, 0, 0);
    if (ret < 0) {
        return NULL;
    }
    return (void *)ret;
}

/* Unmap is not implemented. The syscall status is returned as-is
 * (AUREA_ERR_UNSUPPORTED until ownership tracking exists). Not a successful free. */
static inline long aurea_free(void *ptr) {
    return aurea_syscall4(AUREA_SYS_FREE, (long)ptr, 0, 0, 0);
}

static inline long aurea_path_get(long path_id, char *buf, long cap) {
    return aurea_syscall4(AUREA_SYS_PATH_GET, path_id, (long)buf, cap, 0);
}

static inline long aurea_path_join(const char *base, const char *rel, char *out, long cap) {
    return aurea_syscall4(AUREA_SYS_PATH_JOIN, (long)base, (long)rel, (long)out, cap);
}

static inline long aurea_whoami(AureaWhoAmI *out) {
    return aurea_syscall4(AUREA_SYS_WHOAMI, (long)out, 0, 0, 0);
}

static inline long aurea_time_ms(void) {
    return aurea_syscall4(AUREA_SYS_TIME_MS, 0, 0, 0, 0);
}

/* GUI wrappers — implemented when kernel dispatches IDs 21–28. */
static inline long aurea_window_create(const AureaWindowCreate *args, aurea_window_t *out) {
    if (!args || !out || args->struct_size < sizeof(AureaWindowCreate) ||
        args->abi_version != AUREA_GUI_ABI_V1) {
        return AUREA_ERR_INVALID;
    }
    return aurea_syscall4(AUREA_SYS_WINDOW_CREATE, (long)args, (long)out, 0, 0);
}

static inline long aurea_window_show(aurea_window_t win) {
    return aurea_syscall4(AUREA_SYS_WINDOW_SHOW, (long)win, 0, 0, 0);
}

/* Present UTF-8 client text into the window surface (ownership stays with app). */
static inline long aurea_window_present(aurea_window_t win, const void *text, long len) {
    if (len < 0) {
        return AUREA_ERR_INVALID;
    }
    return aurea_syscall4(AUREA_SYS_WINDOW_SHOW, (long)win, (long)text, len, 1);
}

static inline long aurea_window_set_title(aurea_window_t win, const char *title) {
    return aurea_syscall4(AUREA_SYS_WINDOW_SET_TITLE, (long)win, (long)title, 0, 0);
}

static inline long aurea_window_close(aurea_window_t win) {
    return aurea_syscall4(AUREA_SYS_WINDOW_CLOSE, (long)win, 0, 0, 0);
}

static inline long aurea_window_destroy(aurea_window_t win) {
    return aurea_syscall4(AUREA_SYS_WINDOW_DESTROY, (long)win, 0, 0, 0);
}

static inline long aurea_window_resize(aurea_window_t win, long w, long h) {
    return aurea_syscall4(AUREA_SYS_WINDOW_RESIZE, (long)win, w, h, 0);
}

static inline long aurea_yield(void) {
    return aurea_syscall4(AUREA_SYS_YIELD, 0, 0, 0, 0);
}

static inline long aurea_event_poll(AureaEvent *ev) {
    if (!ev || ev->struct_size < sizeof(AureaEvent)) {
        return AUREA_ERR_INVALID;
    }
    return aurea_syscall4(AUREA_SYS_EVENT_POLL, (long)ev, 0, 0, 0);
}

static inline long aurea_event_wait(AureaEvent *ev, long timeout_ms) {
    if (!ev || ev->struct_size < sizeof(AureaEvent)) {
        return AUREA_ERR_INVALID;
    }
    return aurea_syscall4(AUREA_SYS_EVENT_WAIT, (long)ev, timeout_ms, 0, 0);
}

static inline long aurea_dialog_show(long type, const char *title, const char *message,
                                     AureaDialogIo *io) {
    long st = aurea_syscall4(AUREA_SYS_DIALOG, type, (long)title, (long)message, (long)io);
    if (st != 0) {
        return st;
    }
    for (;;) {
        AureaEvent ev;
        size_t i;
        unsigned char *p;
        p = (unsigned char *)&ev;
        for (i = 0; i < sizeof(ev); i++) {
            p[i] = 0;
        }
        ev.struct_size = (uint32_t)sizeof(ev);
        ev.abi_version = AUREA_GUI_ABI_V1;
        st = aurea_event_wait(&ev, 50);
        if (ev.type == AUREA_EVENT_DIALOG) {
            return 0;
        }
        if (st != 0 && st != AUREA_ERR_TIMEOUT && ev.type != AUREA_EVENT_NONE) {
            continue;
        }
    }
}

typedef struct AureaDirListEntry {
    uint32_t is_dir;
    char name[64];
} AureaDirListEntry;

_Static_assert(sizeof(AureaDirListEntry) == 68u, "AureaDirListEntry size");

static inline long aurea_clipboard_get(char *buf, long cap) {
    if (!buf || cap < 1) {
        return AUREA_ERR_INVALID;
    }
    return aurea_syscall4(AUREA_SYS_CLIPBOARD_GET, (long)buf, cap, 0, 0);
}

static inline long aurea_clipboard_set(const char *buf, long len) {
    return aurea_syscall4(AUREA_SYS_CLIPBOARD_SET, (long)buf, len, 0, 0);
}

static inline long aurea_dir_list(const char *path, AureaDirListEntry *ents, long max_ents,
                                  uint32_t *out_count) {
    if (!path || !ents || !out_count || max_ents < 1) {
        return AUREA_ERR_INVALID;
    }
    return aurea_syscall4(AUREA_SYS_DIR_LIST, (long)path, (long)ents, max_ents, (long)out_count);
}

static inline long aurea_mkdir(const char *path) {
    if (!path) {
        return AUREA_ERR_INVALID;
    }
    return aurea_syscall4(AUREA_SYS_MKDIR, (long)path, 0, 0, 0);
}

static inline long aurea_remove(const char *path, long flags) {
    if (!path) {
        return AUREA_ERR_INVALID;
    }
    return aurea_syscall4(AUREA_SYS_REMOVE, (long)path, flags, 0, 0);
}

static inline long aurea_rename(const char *old_path, const char *new_path) {
    if (!old_path || !new_path) {
        return AUREA_ERR_INVALID;
    }
    return aurea_syscall4(AUREA_SYS_RENAME, (long)old_path, (long)new_path, 0, 0);
}

/* Publish a fully written staging file as target. Kernel VFS replace, not a copy. */
static inline long aurea_replace_file(const char *staging, const char *target) {
    if (!staging || !target) {
        return AUREA_ERR_INVALID;
    }
    return aurea_syscall4(AUREA_SYS_REPLACE, (long)staging, (long)target, 0, 0);
}

#endif /* AUREA_SDK_SYSCALL_H */
