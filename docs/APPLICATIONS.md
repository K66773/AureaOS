# Installing and running programs

This page describes how programs get into Aurea OS Milestone 1 and how they are started. The procedure is a developer workflow: there is no installer or store, and nothing persists across a restart.

> **Milestone 1.1:** this page was written for Milestone 1 (build 02416) and has not been fully rechecked against build 02450. Build 02450 also contains preinstalled programs in `C:\Programs` (Browser, Notepad, Snake Arena) and a Freedoom package that opens the package installer; `http get` must write into your user folder, such as `C:/Users/Aura`. For what changed, see the [README](../README.md), [LINUX_GUEST.md](LINUX_GUEST.md) and [LIMITATIONS.md](LIMITATIONS.md).

## Status

| Step | Status in build 02416 |
|---|---|
| Building an application with the SDK | Verified |
| Transferring a file with `http get` | Verified: byte-exact transfer, SHA-256 matches |
| Running a program with `run` | Verified: SDK-built `.run` and `.aex` files and the bundled sample, with and without arguments |
| Installing with `pkg install` or `installer` | Not verified |
| Persistence of installed programs | Not available: the release image keeps C: in memory |

## Executable formats

| Extension | Format | Notes |
|---|---|---|
| `.aex` | Aurea executable, version 1 or 2 | An ELF64 image plus its manifest. |
| `.run` | RUN2 bundle | Contains an `.aex`, the manifest, optional resources (icons) and an optional integrity tag. |

Both are produced by `sdk/tools/aurea_pack.py`; see [SDK.md](SDK.md#packaging). `container classify <path>` and `container inspect <path>` in Aura show what the system thinks a file is.

## Where programs live

| Folder | Contents |
|---|---|
| `C:\Programs` | Programs. The image includes one sample, `C:\Programs\hello\hello.run`. |
| `C:\Users` | User folders. |
| `C:\Temp` | Scratch space. |

In the release image all of these are in memory and start out the same at every boot.

## Getting a file into Aurea

Milestone 1 cannot read files from USB drives or host folders: USB storage devices are detected, but no file system is mounted from them. The one working path is a download over QEMU's emulated network:

1. On the host, in the folder with your files:
   ```sh
   python3 -m http.server 8000
   ```
2. In the Aura terminal:
   ```
   http get 10.0.2.2 8000 /hello.run C:/hello.run
   ```
   Arguments: host (an IPv4 address or a name), port, path on the server, destination path in Aurea. The command reports `status=200`, the byte count and the SHA-256 of what it received.

`http get` makes a plain HTTP/1.0 request. The emulated network adapter must be present (the launcher adds it unless you pass `-NoNetwork`).

## Running a program

```
run C:/Programs/hello/hello.run
run C:/hello.run first second
run -d C:/hello.run
```

Arguments after the path are passed to the program; `--` ends `run`'s own options. `-d` or `--diag` prints the full loader diagnostics if the launch fails. The Start menu's *Run…* dialog can also start programs, but it dropped typed characters in my tests; prefer the terminal.

While a program runs, the terminal waits for it. Its console output is collected and printed in the Aura window when the program ends, followed by `Process exited with code N.`

## Package commands

Aura registers `pkg` (inspect, install, list, info, verify, repair, uninstall, launch) and `installer` (a guided installer for `.run` bundles built with `--deployment install`). These commands exist, but I have not verified their workflows in this release, and any installation would be lost at shutdown anyway. They are listed in [COMMANDS.md](COMMANDS.md).

There is no graphical installer, package repository or store.

## Permissions

A program receives the permissions requested in its manifest, limited by policy. File writes (`filesystem_write`) require a package with an integrity tag (`--sign`). Packages cannot request kernel access, and unsigned packages cannot request device access. The development account is an administrator, which does not change what a program itself may do.

## What is planned

- Persistent storage on AureaFS, so that installed programs survive a restart.
- Reading files from USB drives and shared host folders.
- A working package installer.
- Real code signing.
