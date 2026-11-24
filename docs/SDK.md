# Aurea SDK

The SDK in [`sdk/`](../sdk) is the public interface for writing native Aurea OS applications. It targets ABI version 1 as implemented by Milestone 1 (build 02416).

> **Status: Partial.** The SDK builds, and SDK-built console programs run in the release image: in my tests they received their arguments, printed to the Aura window and exited cleanly, repeatedly in one session. Only the console path has been exercised from an SDK program; windows, files, sockets and the clipboard are implemented in the kernel but untested from the SDK. Treat it as an early interface that will change.

- [What is in the SDK](#what-is-in-the-sdk)
- [Required tools](#required-tools)
- [A first application](#a-first-application)
- [Building](#building)
- [Packaging](#packaging)
- [Transferring and running](#transferring-and-running)
- [Available interfaces](#available-interfaces)
- [The manifest](#the-manifest)
- [Unsupported features](#unsupported-features)
- [Known user-mode limitations](#known-user-mode-limitations)

## What is in the SDK

| Path | Contents |
|---|---|
| `include/aurea/` | Public headers: system call numbers, status codes, files, paths, windows and events, UI drawing protocol, sockets and DNS, dialogs, SHA-256 |
| `include/aurea_user.h` | Convenience header that pulls in the basics |
| `src/` | `crt0.c` (program entry), a small runtime (`strlen`, `memcpy` …), `memcpy`/`memset` symbols for the compiler, SHA-256, UI command builder |
| `linker/user.ld` | Linker script; applications are loaded at `0x400000` |
| `tools/aurea_pack.py` | Packager for `.aex` and `.run` files |
| `examples/hello/` | Console example |
| `Makefile` | Builds every example into `out/` |

The SDK contains no Aurea kernel source and does not depend on any.

## Required tools

- **clang** and **ld.lld** (LLVM 14 or newer; tested with LLVM 18)
- **GNU make** and **ar**
- **Python 3.8+** for the packager

Tested on Ubuntu 24.04 under WSL 2:

```sh
sudo apt install clang lld make binutils python3
```

On Windows, use WSL. A native Windows LLVM toolchain should work in principle (`--target=x86_64-unknown-none` is host-independent), but I have not tested it.

## A first application

Applications are freestanding C. There is no C standard library: no `printf`, no `malloc`, no `stdio.h`. The entry point is `aurea_app_main`, called by `crt0` with the usual `argc`/`argv`. Its return value becomes the exit code.

```c
#include "aurea/runtime.h"
#include "aurea/syscall.h"

int aurea_app_main(int argc, char **argv) {
    aurea_write_cstr(1, "Hello from an Aurea SDK application.\r\n");
    for (int i = 1; i < argc; i++) {
        aurea_write_cstr(1, "arg: ");
        aurea_write_cstr(1, argv[i]);
        aurea_write_cstr(1, "\r\n");
    }
    return 0;
}
```

The complete example, which also prints its process ID, is [`sdk/examples/hello/hello.c`](../sdk/examples/hello/hello.c). Use `\r\n` line endings for console output.

## Building

```sh
cd sdk
make              # builds every folder under examples/
make APP=hello    # builds one
```

Output goes to `sdk/out/`: `hello.elf`, `hello.aex` and `hello.run`. To build your own application, create `sdk/examples/<name>/` with your `.c` files and an `app.toml`, and run `make APP=<name>`.

The compiler flags matter. Code must be freestanding, non-PIC, without a red zone and without stack protector calls:

```
--target=x86_64-unknown-none -ffreestanding -fno-stack-protector -fno-pic -mno-red-zone -nostdlib
```

## Packaging

`aurea_pack.py` wraps the linked ELF and the manifest:

| Format | Command | Use |
|---|---|---|
| AEX (v1) | `--format aex` | Plain executable. |
| AEX (v2) | `--format aex2` | Same layout, newer header version. |
| RUN2 | `--format run2 --deployment standalone --installer none` | Bundle with the AEX image, manifest and optional resources such as icons. |

`--sign` adds an HMAC-SHA256 integrity tag. **This is not a signature in the security sense**: the key is a fixed development key built into the packager and the kernel, so anyone can produce a valid tag. It detects damaged files. Aurea grants signed packages some additional permissions, such as file writes; that is a development convenience, not a trust decision.

`python3 tools/aurea_pack.py --inspect out/hello.run` prints a package's header and checks its checksum.

## Transferring and running

Milestone 1 has no persistent storage, no USB file import and no package store, so there is one way to get a file into a running system: download it over the emulated network.

1. Start the VM with networking (the default with `Aurea Live VM.cmd`).
2. On the host, serve the output folder:
   ```sh
   cd sdk/out
   python3 -m http.server 8000
   ```
3. In the Aura terminal:
   ```
   http get 10.0.2.2 8000 /hello.run C:/hello.run
   run C:/hello.run alpha beta
   ```

`10.0.2.2` is the host as seen from QEMU's user-mode network. `http get` prints the size and SHA-256 of the received file, which you can compare with `sha256sum` on the host. In my tests the transfer was byte-exact.

Expected result in the Aura window:

```
Aura C:/> run C:/hello.run alpha beta
Hello from an Aurea SDK application.
pid: 19
arg: alpha
arg: beta
Process exited with code 0.
```

While a program runs, the terminal waits for it. Console output (`aurea_write` on fd 1) is collected and shown in the Aura window when the program ends; it is also copied to the serial log. `argv[0]` is the program path and the arguments follow from `argv[1]`. Programs placed on C: are gone after a restart.

`run -d <path>` (or `--diag`) prints detailed loader diagnostics when a launch fails.

### Linker script

Use `linker/user.ld` as shipped. It puts the whole program into one loadable segment. The Milestone 1 loader cannot load ELF files whose segments share a memory page; a default lld link of the same objects is rejected with `Error EXEC-0x0120: Executable mapping failed.`

## Available interfaces

System calls use `int $0x80`: `rax` holds the number, `rdi`, `rsi`, `rdx` and `r10` the arguments; `rcx` and `r11` are clobbered. Negative results are status codes from `aurea/status.h` (`AUREA_ERR_*`). The headers wrap every call in an inline function.

| IDs | Area | Wrappers |
|---|---|---|
| 0–2 | Process | `aurea_exit`, `aurea_write` (fd 1 = console), `aurea_getpid` |
| 5 | Scheduling | `aurea_yield` |
| 10–13 | Files | `aurea_open`, `aurea_read`, `aurea_file_write`, `aurea_close` |
| 14–15 | Memory | `aurea_alloc`; `aurea_free` returns `AUREA_ERR_UNSUPPORTED` (memory is released when the process ends) |
| 16–17 | Paths | `aurea_path_get` (known folders such as Documents and Temp), `aurea_path_join` |
| 18–19 | Identity, time | `aurea_whoami`, `aurea_time_ms` |
| 20 | Dialogs | `aurea_dialog_show` |
| 21–28 | Windows and events | `aurea_window_create`, `_show`, `_set_title`, `_present`, `_resize`, `_close`, `_destroy`, `aurea_event_poll`, `aurea_event_wait` |
| 29–35 | Network | `aurea_socket_create`, `_connect`, `_send`, `_recv`, `_shutdown`, `_close`, `aurea_dns_resolve` (TCP over IPv4) |
| 36–37 | Clipboard | `aurea_clipboard_get`, `aurea_clipboard_set` |
| 38–42 | Directories | `aurea_dir_list`, `aurea_mkdir`, `aurea_remove`, `aurea_rename`, `aurea_replace_file` |

IDs 3, 4 and 6–9 (a test fault, handle close, wait and IPC) are defined in `syscall_ids.h` but have no SDK wrappers and are not documented here.

Windows are server-side: an application owns an opaque handle, receives keyboard, text, pointer, resize and close events, and sends drawing commands in the AUI1 format built with `aurea/ui.h` (`aurea_ui_fill`, `aurea_ui_label`, `aurea_ui_button`, `aurea_ui_textbox`, …). Applications cannot write the framebuffer directly. Colours come from the system theme unless the RGB variants are used.

These interfaces match the build 02416 kernel. Only the console path (`aurea_write`, `aurea_getpid`, `aurea_exit`) has been exercised in this release; the others are implemented but were not tested from an SDK application in Milestone 1.

## The manifest

Every application has an `app.toml`. The example's manifest:

```toml
application_id = "dev.example.hello"
display_name = "Hello"
version = "1.0.0"
author = "Example"
description = "Prints a greeting, its PID and its arguments"
architecture = "x86_64"
runtime = "aurea-native"
aurea_api_version = "0.1"
aurea_abi_version = 1
entry = "0x400000"
application_role = "user"

[limits]
memory_bytes = 16777216
max_threads = 1
max_windows = 0
max_handles = 16
cpu_priority = 3

[permissions]
system_info = true
console_write = true
window_create = false
filesystem_read = false
filesystem_write = false
network_connect = false
device_query = false
device_access = false
process_spawn = false
kernel_access = false
```

Request only the permissions you need. The packager refuses `kernel_access = true`, and `device_access = true` for unsigned packages. `process_spawn` is accepted but there is no system call that uses it.

## Unsupported features

- No C or C++ standard library, no `printf`, no heap allocator beyond `aurea_alloc`.
- No threads, no process creation, no signals.
- No shared libraries; every application is a single static image.
- No direct hardware or framebuffer access.
- No persistent storage in the release image.
- No installer workflow that has been verified in Milestone 1 (`pkg` and `installer` commands exist but were not tested).
- No code signing.

## Known user-mode limitations

- The loader requires one segment per page (see [Linker script](#linker-script)).
- After the first `run`, the kernel's own user-mode self-test stays listed as `usermode-test` in `ps`.
- The Ring 3 environment has been smoke-tested (user-mode execution, system calls, stack writes, NX on the stack), but protection of kernel memory against user code has not been systematically tested.
- `aurea_free` does not release memory.
- Error reporting for failed launches is detailed with `run -d`, but the normal message is short.
- The desktop applications that ship with Aurea are not SDK applications; they run inside the kernel.
