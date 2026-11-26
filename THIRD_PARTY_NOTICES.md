# Third-party notices

Aurea OS is my own work, but the Milestone 1.1 image contains software, fonts and game data written by others. These components keep their own licences. They are not Aurea code, and their licences are not changed by being included here.

The full licence texts are in `THIRD_PARTY_NOTICES.txt` on the [Milestone 1.1 release page](../../releases/tag/milestone-1.1).

## Components in the Milestone 1.1 image

### Linux kernel 6.18.49

- **Where:** two guest kernels embedded in the Aurea kernel image: the Linux driver domain (with its loadable modules in the guest's initramfs), and a second build (`6.18.49-aureafs1`, with FUSE built in) for the experimental file-system guest.
- **Licence:** GPL-2.0-only, with the Linux syscall note.
- **Copyright:** Linus Torvalds and the Linux kernel contributors.
- **Build:** unmodified upstream source from kernel.org (`linux-6.18.49.tar.xz`, SHA-256 `ae826f33…a28191`). Both kernels use their own configuration; `-aureafs1` is set through `CONFIG_LOCALVERSION`. Both `.config` files are part of the source archive below.

### Aurea guest modules and driver service

- **Where:** `aurea_driver_net.ko`, `aurea_driver_i2c.ko` and `/bin/aurea-driver-service` in the driver domain's initramfs.
- **Licence:** GPL-2.0-only.
- **Copyright:** 66773. These are my code; they are listed here because they are Linux modules and a Linux user-space program, and their source is in the archive below.

### GNU Bash 5.2.21

- **Where:** `/bin/bash` in the Linux driver domain's initramfs.
- **Licence:** GPL-3.0-or-later.
- **Copyright:** Free Software Foundation, Inc.
- **Build:** the statically linked binary from the Ubuntu package `bash-static` 5.2.21-2ubuntu4, unmodified.

### BusyBox 1.36.1

- **Where:** `/bin/busybox` and its applet links in the Linux driver domain's initramfs.
- **Licence:** GPL-2.0-only.
- **Copyright:** Erik Andersen, Rob Landley, Denys Vlasenko and the BusyBox contributors.
- **Build:** the statically linked binary from the Ubuntu package `busybox-static` 1:1.36.1-6ubuntu3.1, unmodified.

### GNU C Library 2.39

- **Where:** statically linked into Bash, BusyBox and three Aurea helper programs in the initramfs (`aurea-console-session`, `aurea-domain-agent`, `aurea-driver-service`).
- **Licence:** LGPL-2.1-or-later, with additional permissive notices for some parts.
- **Copyright:** Free Software Foundation, Inc. and others.
- **Versions:** Ubuntu `glibc` 2.39-0ubuntu8 (in Bash), 2.39-0ubuntu8.2 (in BusyBox) and 2.39-0ubuntu8.12 (in the three helpers).

### id Tech 1 engine (DOOM source release)

- **Where:** the program in the `Freedoom.run` package (`C:\Programs\phase1\Freedoom.run` in the image).
- **Licence:** GPL-2.0.
- **Copyright:** id Software, Inc. (linuxdoom-1.10); the Aurea port is mine and under the same licence.
- **Build:** ported from id Software's source release. The changes and the Aurea platform layer are in the source archive below.

### Freedoom: Phase 1 0.13.0

- **Where:** the game data in the `Freedoom.run` package.
- **Licence:** BSD-3-Clause.
- **Copyright:** Copyright © 2001-2024 Contributors to the Freedoom project.

### BearSSL

- **Where:** linked into the Browser and NetTest programs.
- **Licence:** MIT.
- **Copyright:** Copyright (c) 2016 Thomas Pornin.

### IBM Plex Sans and IBM Plex Mono

- **Where:** embedded in the Aurea kernel as the interface fonts, shown in Settings as "Aurea UI" and "Aurea Mono". The font files are unmodified.
- **Licence:** SIL Open Font License 1.1.
- **Copyright:** Copyright © 2017 and 2019 IBM Corp., with Reserved Font Name "Plex".

### stb_truetype 1.26

- **Where:** the TrueType rasterizer in the Aurea kernel.
- **Licence:** MIT or public domain (Unlicense), at the user's choice.
- **Copyright:** Copyright (c) 2017 Sean Barrett.

## Source code for GPL and LGPL components

The GNU General Public License and the GNU Lesser General Public License require that the complete corresponding source code be available to anyone who receives the binaries. For Milestone 1.1 it is published next to the image, on the same release page:

**`AureaOS-Milestone1.1-third-party-source.tar`**, containing:

- the Linux 6.18.49 source tarball and the two kernel configurations (driver domain and file-system guest);
- the Ubuntu source packages for `bash` 5.2.21-2ubuntu4, `busybox` 1:1.36.1-6ubuntu3.1, and `glibc` 2.39-0ubuntu8, 2.39-0ubuntu8.2 and 2.39-0ubuntu8.12;
- the source of the Aurea guest modules, the driver service and the two console helpers, with the commands used to build, sign and link them and to assemble the initramfs;
- the id Tech 1 engine port: the upstream source, the scripts that produce the port, the Aurea platform layer, the SDK runtime it links (MIT) and a build script.

The Milestone 1 image and its source archive remain on the [Milestone 1 release page](../../releases/tag/milestone-1).

If anything is missing from an archive, open an issue in this repository and I will provide it.

## Not included

The QEMU firmware (edk2/OVMF) used by `Aurea Live VM.cmd` comes from your own QEMU installation and is not distributed with Aurea OS.
