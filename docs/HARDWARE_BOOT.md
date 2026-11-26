# Booting Aurea OS on Intel hardware

This guide goes from an empty USB drive to a GNU Bash prompt running in Aurea's VMX Linux guest on a real Intel PC.

Hardware boot is experimental. Everything below was tested on one machine:

| | |
|---|---|
| Machine | HP EliteBook 840 G8 |
| CPU | Intel 11th-gen Core (Tiger Lake), 4 cores / 8 threads, VT-x with EPT and unrestricted guest |
| Memory | 16 GB |
| Firmware | HP UEFI, 1920×1080 GOP framebuffer |

| Build | Result on that machine |
|---|---|
| 02388, 02391 | Boot to login and desktop; keyboard works; VMXON/VMXOFF works |
| 02405 | Linux guest ready about 3 s after `domain linux start`; interactive Bash |
| 02412 | Two boots with Linux autostart: VMX guest self-test passed, Linux ready about 40 s after the guest started, interactive Bash including `uname -a` |
| 02416 (Milestone 1 download) | Not tested on hardware. |
| 02450 (Milestone 1.1 download) | **Not tested on hardware yet.** Everything new in 1.1 (SoftCPU, network card driven by Linux) was verified in QEMU only. |

Other Intel machines with VT-x and EPT should behave similarly, but none has been tested, and missing drivers or firmware differences can stop the boot. AMD processors have not been tested: there is no AMD SVM backend, so the Linux guest would have to run on SoftCPU.

## A. Requirements

- **Intel x86-64 CPU** with VT-x. The Linux guest needs these VMX features, which Aurea checks at boot:
  - EPT and unrestricted guest (secondary processor controls);
  - the VMX-preemption timer, external-interrupt exiting and acknowledge-interrupt-on-exit;
  - interrupt-window exiting.

  Most Intel Core processors of the last decade have them, but I have only verified the Tiger Lake machine above. If VMX is not usable, 1.1 runs the guest on [SoftCPU](SOFTCPU.md) instead, which I have only tested in QEMU.
- **For the Linux network path: Intel VT-d** (an IOMMU, listed in the ACPI DMAR table) and a wired network card the guest kernel has a driver for. Without VT-d, Aurea keeps the card and uses its own driver, if it has one. Not tested on hardware.
- **UEFI firmware.** Legacy BIOS/CSM boot is not supported.
- **Secure Boot off.** The Aurea boot loader and kernel are unsigned.
- **Intel Virtualization Technology enabled** in the firmware, and locked as enabled. Firmware normally does both when you turn the option on.
- **Memory:** 1 GB is enough for Aurea and the 128 MiB Linux guest; the test machine had 16 GB.
- **A USB drive** of 256 MB or more that may be erased. The image is 128 MiB.
- **A keyboard** that works without special drivers: the built-in laptop keyboard (PS/2-compatible) or a USB keyboard.
- **A USB mouse.** Laptop touchpads (I2C-HID) do not work yet.

## B. Download

From the [Milestone 1.1 release](../../../releases/tag/milestone-1.1):

- `AureaOS-Milestone1.1-build02450.img`: the boot image
- `AureaOS-Milestone1.1-tools.zip`: `Aurea Live boot.cmd` and `Aurea Live VM.cmd`
- `SHA256SUMS.txt`: checksums

Extract the tools and put the image and `SHA256SUMS.txt` in the same folder.

## C. Prepare the USB drive

On Windows:

1. Plug in the USB drive.
2. Run `Aurea Live boot.cmd` and accept the administrator prompt.
3. The tool checks the image against `SHA256SUMS.txt`, then lists only USB-attached disks. System and boot disks, disks holding Windows or a page file, read-only, offline, too small and larger than 2 TB disks are never offered.
4. Enter the disk number, then type `ERASE <number>` exactly.
5. The tool removes the drive's partitions, writes the image, reads it back and compares the SHA-256. It reports success only if they match.

`"Aurea Live boot.cmd" -ListOnly` shows the eligible drives without writing anything.

On Linux:

```sh
sha256sum -c SHA256SUMS.txt --ignore-missing
lsblk -o NAME,SIZE,MODEL,TRAN          # find the USB drive, e.g. /dev/sdX
sudo dd if=AureaOS-Milestone1.1-build02450.img of=/dev/sdX bs=4M conv=fsync status=progress
```

The image has a GPT sized for 128 MiB, so on a larger drive the backup GPT header is not at the end of the disk. UEFI firmware boots such drives normally.

## D. Configure the firmware

Open the firmware setup (on HP: Esc, then F10) and:

1. Disable **Secure Boot**.
2. Enable **Intel Virtualization Technology (VT-x)**. If there is a separate **VT-d** option, enable it too: 1.1 needs it to hand a network card to the Linux guest.
3. Keep **UEFI** boot mode; disable Legacy/CSM if it is on.
4. Save and power off.

## E. Boot Aurea

1. Plug in the USB drive and open the boot menu (HP: F9; Dell and Lenovo: F12; ASUS: F8; MSI: F11).
2. Choose the USB drive's UEFI entry.
3. A short boot menu appears, then the kernel's boot screen with its log and progress bars. On the test machine the login screen came up about 30 seconds after the kernel started.
4. Log in as `aura` with password `root`.
5. Open the Aura terminal from the taskbar (`>_`).

## F. Start the Linux guest

The release image sets `linux.autostart=true`, so the guest starts by itself during boot. Its state:

```
Aura C:/> domain linux status
```

Then open the shell:

```
Aura C:/> bash
```

If Linux is still starting, `bash` prints `Linux is still starting...` and attaches when the shell is ready. On the test machine, with autostart, that took about 40 seconds from the guest's start. If autostart is turned off or the guest was stopped, start it by hand:

```
Aura C:/> domain linux start
```

If VMX is missing or disabled, 1.1 runs the guest on SoftCPU; the boot log says `Linux vCPU: SoftCPU` instead of `Linux vCPU: VMX`.

## G. Verify Linux

```
root@linux:~# uname -a
Linux (none) 6.18.49 #2 SMP PREEMPT_DYNAMIC Wed Oct  7 17:32:56 CEST 2026 x86_64 GNU/Linux
root@linux:~# bash --version
GNU bash, version 5.2.21(1)-release (x86_64-pc-linux-gnu)
root@linux:~# whoami
root
```

The `uname -a` line is the real output from the HP run of build 02412; `bash --version` is from QEMU/KVM. The guest has Bash plus the BusyBox tools (`ls`, `cat`, `ps`, `mkdir` and so on). It has no package manager, no disk and no access to Aurea's C: drive. If a network card was assigned to it, Linux drives the card for Aurea but has no IP address of its own. Files you create live in the guest's memory.

To see whether the network card went to Linux, run `net status` in Aura. With the Linux path it says `driver: Linux <driver> on PCI <id>, DMA isolated by the IOMMU`.

## H. Return to Aura

| Key or command | Effect |
|---|---|
| `exit` or Ctrl+D | Ends Bash and returns to Aura. The guest starts a fresh shell for the next `bash`. |
| Ctrl+] or Ctrl+Shift+J | Returns to Aura and leaves Bash running. `bash` reattaches to the same shell. |
| Ctrl+C | Copies if text is selected, otherwise interrupts the running Linux program. |

## I. Shut down

```
Aura C:/> power shutdown
```

If the machine does not power off, hold the power button. Remove the USB drive and the computer boots its normal operating system again; Aurea changes no firmware settings or boot entries. You can turn Secure Boot back on.

To reuse the USB drive, delete its partitions in Windows Disk Management (or `diskpart`, `select disk N`, `clean`) and create a new volume.

## J. Troubleshooting

| Problem | What to check |
|---|---|
| The boot log says `Linux vCPU: SoftCPU` on an Intel machine | Intel Virtualization Technology is off in the firmware, or the CPU lacks one of the features in section A. `hwdiag` in Aura prints the VMX capability checks; `hwdiag vmxon` also tries VMXON and VMXOFF. Linux still runs, on SoftCPU. |
| Virtualization option is enabled but VMX still fails | Some firmware needs a full power-off, not a restart, after changing it. |
| AMD processor | Not tested. There is no AMD SVM backend; the Linux guest would run on SoftCPU. |
| Linux does not become ready | `domain linux status` shows the guest's state. Try `domain linux stop`, then `domain linux start`. |
| Bash prompt does not appear after `bash` | Wait for the `Linux is still starting...` phase to finish, or detach with Ctrl+] and run `bash` again. |
| Screen stays black after choosing the USB drive | Secure Boot may still be on, or the firmware's graphics output is not usable by Aurea. |
| Boot stops in the log screen | A driver failed on this hardware. Note the last lines and report them. |
| Keyboard does not respond | Try the built-in keyboard, or a USB keyboard in another port. |
| No mouse pointer movement | Touchpads are not supported yet; use a USB mouse. |
| Internal disks | Aurea mounts nothing from internal disks. The shipped configuration marks them read-only (`storage.internal_readonly=true`), and NVMe drives are only detected. I still recommend a test machine and a backup. |
| `net status` does not mention Linux | No VT-d, VT-d disabled in the firmware, or no supported wired card. Wi-Fi does not go through Linux yet. |
| Wi-Fi, sound, Bluetooth | Not supported on hardware in Milestone 1.1. |

Logs are not kept on the USB drive in this image. If you report a problem, a photo of the screen with the last messages helps most.
