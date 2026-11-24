# Limitations of Milestone 1

**Aurea OS Milestone 1 is an experimental developer preview. It is not suitable for daily use.**

**Performance optimization is not the primary focus of Milestone 1.** UI bugs, latency and missing functionality are expected in this release. This page lists what I know about; there are certainly more.

## Known defects

| Defect | Effect | Workaround |
|---|---|---|
| Occasional freeze shortly after login | In one of about ten test sessions of build 02416 the system stopped responding within a minute of login, right after a terminal command: no screen updates, no serial output. Nine further sessions with similar steps ran normally. The cause is not known yet. | Close QEMU and start again. |
| Loader rejects ELF segments that share a page | Programs not linked with the SDK's `linker/user.ld` can fail with `EXEC-0x0120`. | Link with the SDK linker script. |
| `usermode-test` stays in the process list | The kernel's user-mode self-test, run before the first program, remains listed as Running. | Harmless; ignore it. |
| `mount` does not attach file systems | `mount <device> <letter>` only records an entry; the drive cannot be browsed. With exactly three arguments the file system argument is ignored. | None. |
| `power shutdown` under KVM | In one test the VM did not power off. | Close the QEMU window. |
| Windows opened without user action | In one test session Task Manager and Settings were open on the desktop right after login. Not reproduced. | Close them. |
| Dropped keystrokes | Fast typing into the Run dialog lost characters, and a click on its OK button was ignored once. | Start programs from the terminal. |

## Boot performance

Measured on one Windows 11 host with QEMU 11.0 and WHPX: the login screen appeared within 40 seconds of starting QEMU in every run, and the desktop within about 20 seconds after login. Without hardware acceleration (TCG) the desktop took about three minutes after login in one test. Under KVM on Linux the Linux driver domain reported ready about 32 seconds after boot. These are single-host observations, not benchmarks.

The shipped configuration boots in verbose mode and logs at trace level to the serial port, which costs time.

## Graphics and frame pacing

- Everything is drawn in software into the UEFI framebuffer. There is no GPU driver and no acceleration.
- The resolution is whatever the firmware (or QEMU's EDID) provides; it cannot be changed at runtime.
- Window dragging, resizing and full-screen redraws are slow, and frame pacing is uneven.
- Visual glitches and inconsistent window behaviour (for example in property sheets and dialogs) are expected.

## Input

- German QWERTZ is the default layout; US QWERTY is available per session with `keyboard layout set ENG`. No other layouts, no IME, no dead keys.
- Input latency is noticeable, especially while the desktop redraws.
- Laptop touchpads (I2C-HID) do not work.

## User-mode execution

- Native applications run in Ring 3. Only console output and arguments have been exercised from an SDK program in this release.
- The system call interface is small (43 calls) and has no process creation, threads or shared libraries.
- `aurea_free` does not release memory.
- The desktop and system applications run in kernel mode.

## File systems and storage

- The release image keeps C: in memory. Nothing survives a shutdown.
- AureaFS is implemented but not used by the release image.
- USB mass storage is detected but no file system on it is mounted.
- NVMe controllers are detected only. AHCI was not verified for this release.
- File system performance has not been measured for this release.

## Drivers

Native drivers exist for: the UEFI framebuffer, PS/2 keyboard, USB UHCI and xHCI with HID and mass-storage devices, Intel e1000 Ethernet, AHCI, NVMe (detection), AC97 and HD Audio, and ACPI power control. Audio and AHCI were not verified for this release. There is no support for GPUs, Wi-Fi, Bluetooth devices, webcams or card readers, even where commands for those areas exist. The Linux driver domain does not provide drivers yet.

## SMP

The release configuration starts one CPU. SMP code exists, but multi-CPU operation was not verified for build 02416.

## SoftCPU

Not included in the release build. See [SOFTCPU.md](SOFTCPU.md).

## Memory management

1 GiB of guest memory is the tested configuration; 512 MiB reached the desktop in one test. Memory released by applications is not returned to the system until they exit.

## Crash handling

A kernel fault stops the system with a red error screen that shows the error name, fault address and registers. Details are written to the serial log. There is no automatic restart, no crash dump file and no recovery that keeps the session alive.

## Security

Milestone 1 is not a hardened operating system. Do not store sensitive data in it, and do not use it as your primary computing environment.

| Area | State in Milestone 1 |
|---|---|
| Ring 0 / Ring 3 separation | Exists for external applications. The desktop and system applications run in Ring 0. |
| Memory isolation | Per-process address spaces and NX for user stacks. Protection of kernel memory from user code has not been systematically tested. |
| Process permissions | Processes have roles, integrity levels and protection flags; system processes cannot be ended from Task Manager. Not audited. |
| Accounts | The development account `Aura` is an administrator, with a public password (`root`). |
| Password storage | Salted SHA-256, a single iteration. Not a password-hashing function such as Argon2 or bcrypt. |
| File system permissions | Minimal. The RAM file system is not a security boundary. |
| Encryption | None. No storage encryption. |
| Package integrity | HMAC-SHA256 with a fixed, public development key. It detects damaged files, not untrusted authors. |
| Linux driver domain | Runs in a VMX virtual machine with EPT. The boundary has not been audited or tested against a hostile guest. |
| Boot | Unsigned boot loader and kernel; Secure Boot must be off. |
| Network | No firewall. HTTP only; `http get` does not use TLS. |

No malware resistance is claimed, and the planned architecture does not make the current build secure.
