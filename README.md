# AureaOS

AureaOS is an independently developed, 64-bit operating system for x86-64 PCs. It has its own kernel, the Horizon desktop, the Aura command environment, AureaFS, and native application interfaces. AureaOS is not a Linux distribution; its optional Linux environment runs as a separate virtualized guest.

## Download

[**AureaOS 0.02.0-alpha — build 02402**](https://github.com/K66773/AureaOS/releases/tag/build-02402)

The release contains an ISO image, a UEFI boot-file archive, component notices, and SHA-256 checksums. This repository currently publishes the project overview and binary releases. Source code and the SDK are not included in these uploads.

## This build

- Horizon desktop and Aura terminal.
- An optional Linux 6.18.49 guest with GNU Bash 5.2, using Intel VT-x and EPT.
- A serial terminal with cursor movement, colors, scrolling, keyboard editing, and a controlling TTY for Bash job control.
- Persistent USB boot and diagnostic logs on the supported partition layout.

This is an early development preview. Hardware support is incomplete, and successful emulation does not establish support for every PC. The terminal changes in this build still need a physical HP retest. See the release notes for the checks performed and known limits.

## Boot requirements

Use x86-64 UEFI firmware. The binaries are unsigned, so firmware must permit unsigned boot. The optional Linux guest also requires Intel VT-x and EPT enabled in firmware.

The supplied configuration uses one executing CPU and keeps internal block devices read-only. The UEFI ZIP contains boot files, not a partitioning tool or an installer. Its README describes the tested USB layout: a 512 MiB FAT32 EFI System Partition and a separate 2 GiB FAT32 partition named `Logs`, with volume label `LOGS`.

Verify the downloaded files against `SHA256SUMS.txt` before preparing boot media. Writing an image can erase the selected drive.

## Linux console

On a fresh RAM filesystem, the development login is `Aura` with password `root`. Open Aura and run `bash` to start or attach the Linux console.

- `Ctrl+C` interrupts a foreground Linux job.
- `Ctrl+D` ends Bash; guest init starts a new shell session.
- `Ctrl+]` or `Ctrl+Shift+J` returns to Aura while keeping the Linux session running.

The serial terminal is currently fixed at 80 columns by 24 rows. The guest uses a small initramfs with Bash and BusyBox; it is not a complete Linux installation. For example, `ip` is not installed.

## Component notices

The release includes `THIRD_PARTY_NOTICES.txt` for bundled components. AureaOS and its bundled components should not be assumed to share one license.
