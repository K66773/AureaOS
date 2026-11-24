#ifndef AUREA_SDK_SYSCALL_IDS_H
#define AUREA_SDK_SYSCALL_IDS_H

/*
 * Public syscall numbers. IDs 0-42 are implemented by the Milestone 1 kernel
 * (build 02412). Calls are made with int $0x80; see aurea/syscall.h.
 */

#define AUREA_SYS_EXIT         0
#define AUREA_SYS_WRITE        1
#define AUREA_SYS_GETPID       2
#define AUREA_SYS_DEBUG_FAULT  3
#define AUREA_SYS_HANDLE_CLOSE 4
#define AUREA_SYS_YIELD        5
#define AUREA_SYS_WAIT         6
#define AUREA_SYS_IPC_CREATE   7
#define AUREA_SYS_IPC_SEND     8
#define AUREA_SYS_IPC_RECV     9
#define AUREA_SYS_OPEN         10
#define AUREA_SYS_READ         11
#define AUREA_SYS_CLOSE        12
#define AUREA_SYS_FILE_WRITE   13
#define AUREA_SYS_ALLOC        14
#define AUREA_SYS_FREE         15
#define AUREA_SYS_PATH_GET     16
#define AUREA_SYS_PATH_JOIN    17
#define AUREA_SYS_WHOAMI       18
#define AUREA_SYS_TIME_MS      19
#define AUREA_SYS_DIALOG       20

#define AUREA_SYS_WINDOW_CREATE  21
#define AUREA_SYS_WINDOW_SHOW    22
#define AUREA_SYS_WINDOW_SET_TITLE 23
#define AUREA_SYS_WINDOW_CLOSE   24
#define AUREA_SYS_EVENT_POLL     25
#define AUREA_SYS_EVENT_WAIT     26
#define AUREA_SYS_WINDOW_RESIZE  27
#define AUREA_SYS_WINDOW_DESTROY 28

#define AUREA_SYS_SOCKET_CREATE   29
#define AUREA_SYS_SOCKET_CONNECT  30
#define AUREA_SYS_SOCKET_SEND     31
#define AUREA_SYS_SOCKET_RECV     32
#define AUREA_SYS_SOCKET_SHUTDOWN 33
#define AUREA_SYS_SOCKET_CLOSE    34
#define AUREA_SYS_DNS_RESOLVE     35
#define AUREA_SYS_CLIPBOARD_GET   36
#define AUREA_SYS_CLIPBOARD_SET   37
#define AUREA_SYS_DIR_LIST        38
#define AUREA_SYS_MKDIR           39
#define AUREA_SYS_REMOVE          40
#define AUREA_SYS_RENAME          41
#define AUREA_SYS_REPLACE         42

#define AUREA_SYS_IMPLEMENTED_MAX 42
#define AUREA_SYS_RESERVED_GUI_MAX 28
#define AUREA_SYS_RESERVED_NET_MAX 35

#endif /* AUREA_SDK_SYSCALL_IDS_H */
