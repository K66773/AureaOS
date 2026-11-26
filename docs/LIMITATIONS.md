# Limitations of Milestone 1.1

**Aurea OS Milestone 1.1 is an experimental developer preview. It is not suitable for daily use.**

**Build 02450 has been tested in QEMU only.** Nothing on this page, good or bad, has been checked on a physical machine for this build. UI bugs, latency and missing functionality are expected. This page lists what I know about; there are certainly more.

## Known defects

Seen with build 02450:

| Defect | Effect | Workaround |
|---|---|---|
| Driver-service messages in Bash | The first `bash` shows the guest driver service's start-up lines (module requests, bound devices, an `aurea-i2c` probe error) above the prompt. | Ignore them, or press Enter. |
| `http get` into protected folders | Writing to the root of C: fails with `http get FAIL status=-4 http=200`: the download worked, the file could not be written there. | Save into your user folder, for example `C:/Users/Aura/file.html`. |
| The guest's I2C module fails to load in QEMU | `aurea_driver_i2c` finds no touchpad controller and reports `No such device`. | Expected without an I2C touchpad. |

Seen with Milestone 1 (build 02416) and not rechecked:

| Defect | Effect | Workaround |
|---|---|---|
| Occasional freeze shortly after login | In one of about ten sessions of build 02416 the system stopped responding within a minute of login. It did not happen in twelve sessions of 02450, but the cause was never found, so I can't rule it out. | Close QEMU and start again. |
| Loader rejects ELF segments that share a page | Programs not linked with the SDK's `linker/user.ld` can fail with `EXEC-0x0120`. | Link with the SDK linker script. |
| `mount` does not attach file systems | `mount <device> <letter>` only records an entry; the drive cannot be browsed. | None. |
| `power shutdown` under KVM | In one test the VM did not power off. | Close the QEMU window. |
| Dropped keystrokes | Fast typing into the Run dialog lost characters. | Start programs from the terminal. |

## Boot performance

With the 1.1 launcher on one Windows 11 host (QEMU 11.0, WHPX, one virtual CPU), the desktop's main loop started about 34 seconds after QEMU started, in every one of ten sessions. The Linux guest on SoftCPU was ready 2.5–2.7 seconds after the domain started. Under nested KVM, Linux on VMX was ready 11–17 seconds after boot. These are single-host observations, not benchmarks.

The shipped configuration logs at trace level to the serial port, which costs time.

## Graphics and frame pacing

- Everything is drawn in software into the UEFI framebuffer. There is no GPU driver and no acceleration.
- The resolution is whatever the firmware (or QEMU's EDID) provides; it cannot be changed at runtime.
- Window dragging, resizing and full-screen redraws are slow, and frame pacing is uneven.
- Visual glitches and inconsistent window behaviour are expected.

## Input

- German QWERTZ is the default layout; US QWERTY is available per session with `keyboard layout set ENG`. No other layouts, no IME, no dead keys.
- Input latency is noticeable, especially while the desktop redraws.
- Laptop touchpads (I2C-HID) do not work.

## Programs

- Native applications run in Ring 3. Browser, Notepad, Snake Arena and the Freedoom package (install and start) were exercised with build 02450.
- Only one native program runs at a time.
- The desktop and system applications run in kernel mode.

## File systems and storage

- The release image keeps C: in memory. Nothing survives a shutdown, including installed packages.
- AureaFS is implemented but not used by the release image.
- USB mass storage is detected but no file system on it is mounted.
- File system performance has not been measured for this release.

## Drivers

- **Through Linux:** network only. In 02450 the Linux guest drives an assigned network card when an Intel IOMMU (VT-d) is present; verified with QEMU's `e1000` under WHPX and KVM. Wi-Fi, storage, input and audio do not go through Linux yet.
- **Native:** the UEFI framebuffer, PS/2 keyboard, USB UHCI and xHCI with HID and mass-storage devices, Intel e1000 Ethernet (used when no IOMMU is present), AHCI, NVMe (detection), AC97 and HD Audio, and ACPI power control. Audio was not verified for this release.
- There is no support for GPUs, Wi-Fi, Bluetooth devices, webcams or card readers, even where commands or settings for those areas exist.

## SMP

The release configuration starts one CPU (`boot.cores=1`). Milestone 1 ran on four virtual CPUs in QEMU with `boot.cores=all`; I have not retested that with 02450. Multi-core operation has not been tested on hardware or in long runs.

## SoftCPU

Included since 1.1, used for the Linux guest when VMX is not usable. Its translation cache is not W^X, and its isolation has not been audited. See [SOFTCPU.md](SOFTCPU.md).

## Memory management

1 GiB of guest memory is the tested configuration. Memory released by applications is not returned to the system until they exit.

## Crash handling

A kernel fault stops the system with a red error screen that shows the error name, fault address and registers. Details are written to the serial log. There is no automatic restart and no crash dump file.

## Security

Milestone 1.1 is not a hardened operating system. Do not store sensitive data in it, and do not use it as your primary computing environment.

| Area | State in Milestone 1.1 |
|---|---|
| Ring 0 / Ring 3 separation | Exists for external applications. The desktop and system applications run in Ring 0. |
| Memory isolation | Per-process address spaces and NX for user stacks. Protection of kernel memory from user code has not been systematically tested. |
| Process permissions | Processes have roles, integrity levels and protection flags. Not audited. |
| Accounts | The development account `aura` is an administrator, with a public password (`root`). |
| Password storage | Salted SHA-256, a single iteration. Not a password-hashing function such as Argon2 or bcrypt. |
| Encryption | None. No storage encryption. |
| Package integrity | Packages carry an integrity check with a development key. It detects damaged files, not untrusted authors. |
| Linux guest | Runs on VMX with EPT, or on SoftCPU. An assigned device's DMA is confined by the IOMMU. None of it has been audited or tested against a hostile guest. |
| Boot | Unsigned boot loader and kernel; Secure Boot must be off. |
| Network | No firewall. `http get` uses plain HTTP. TLS was not verified for this release. |

No malware resistance is claimed, and the planned architecture does not make the current build secure.
