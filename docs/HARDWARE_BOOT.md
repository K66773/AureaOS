# Booting Aurea OS on real hardware

**Hardware boot is experimental.** Aurea OS has its own drivers for only a small set of devices, and it has been tested on exactly one physical machine. Use QEMU unless you specifically want to try real hardware, and use a test machine you can afford to have misbehave.

- [What has been tested](#what-has-been-tested)
- [Requirements](#requirements)
- [Preparing a USB drive](#preparing-a-usb-drive)
- [Booting](#booting)
- [What to expect](#what-to-expect)
- [Storage safety](#storage-safety)
- [Returning to your normal operating system](#returning-to-your-normal-operating-system)
- [Troubleshooting](#troubleshooting)

## What has been tested

| Machine | Build | Result |
|---|---|---|
| HP EliteBook 840 G8 (Intel Tiger Lake, 16 GB) | 02388, 02391 | Boots to login and the desktop; keyboard works; VMX can be enabled. |
| HP EliteBook 840 G8 | 02405 | Linux driver domain starts; Bash ready about 3 seconds after `domain linux start`. |
| HP EliteBook 840 G8 | 02416 (this release) | **Not tested yet.** |
| Any other machine | — | Not tested. |

Those tests used a USB stick with a different partition layout (a 512 MiB EFI partition plus a separate partition for logs) and the same four boot files. The public image, written with `Aurea Live boot.cmd`, contains the same boot files on a single 126 MiB EFI partition. That exact layout has been booted in QEMU, not on hardware.

## Requirements

- **x86-64 PC with UEFI firmware.** Legacy BIOS / CSM boot is not supported.
- **Secure Boot disabled.** The Aurea boot loader and kernel are unsigned.
- **A USB drive of 256 MB or more** that you are prepared to erase. The image is 128 MiB; Aurea writes nothing to the drive while running.
- **For `bash`:** an Intel CPU with VT-x and EPT, with virtualization enabled in the firmware settings. AMD processors are not supported by the Linux driver domain.
- **A keyboard that works without special drivers.** See [What to expect](#what-to-expect).

## Preparing a USB drive

### On Windows: `Aurea Live boot.cmd`

1. Put `Aurea Live boot.cmd`, `aurea-live-boot.ps1`, the `.img` file and `SHA256SUMS.txt` in one folder.
2. Plug in the USB drive.
3. Double-click `Aurea Live boot.cmd` and accept the Windows elevation prompt. Writing a raw disk requires administrator rights.
4. The tool lists the USB drives it is willing to write, with model, size, serial number and drive letters. Disks on other buses (SATA, NVMe, and so on), system and boot disks, disks holding Windows or a page file, read-only and offline disks, disks smaller than the image and disks larger than 2 TB are never offered.
5. Enter the disk number, then type `ERASE <number>` exactly to confirm.
6. The tool removes the drive's partitions, writes the image, reads it back and compares the SHA-256. It reports success only if the read-back matches.

To see which drives would be offered without writing anything, run `"Aurea Live boot.cmd" -ListOnly`.

### On Linux

```sh
sha256sum -c SHA256SUMS.txt --ignore-missing
lsblk -o NAME,SIZE,MODEL,TRAN,MOUNTPOINTS      # identify the USB drive, e.g. /dev/sdX
sudo dd if=AureaOS-Milestone1-build02416.img of=/dev/sdX bs=4M conv=fsync status=progress
```

Double-check the device name. `dd` overwrites whatever you point it at.

### Note on the partition table

The image carries a GPT sized for 128 MiB. On a larger drive, the backup GPT header is therefore not at the end of the disk. UEFI firmware boots such drives normally, and Windows and Linux tools may offer to repair the table. This is expected for raw-written images.

## Booting

1. Shut the computer down completely.
2. Plug in the USB drive.
3. Open the firmware boot menu. Common keys are F9 (HP), F12 (Dell, Lenovo), F11 (MSI), F8 (ASUS) and Esc; check your manufacturer's documentation.
4. Choose the USB drive's **UEFI** entry.
5. A short boot menu appears, then verbose boot output, then the login screen. Log in as `Aura` with password `root`.

## What to expect

| Area | Milestone 1 |
|---|---|
| Graphics | The UEFI framebuffer at the resolution the firmware chooses. No GPU driver, no acceleration, no resolution changes. |
| Keyboard | PS/2 keyboards, including the PS/2-compatible keyboard of most laptops, and USB HID keyboards on xHCI and UHCI controllers. German QWERTZ by default. |
| Pointer | USB mice. **Laptop touchpads do not work**: they are usually I2C-HID devices, and that driver is unfinished. Bring a USB mouse. |
| Storage | USB mass storage (Bulk-Only Transport, 512-byte sectors) is detected. NVMe controllers are detected but not read. AHCI support exists but has not been verified on hardware for this release. Nothing on the USB drive or internal disks is mounted as a drive letter; C: lives in memory. |
| Network | Intel e1000-family adapters only. Wi-Fi is not supported. |
| Audio | AC97 and Intel HD Audio drivers exist but were not tested on hardware for this release. |
| Bluetooth, webcams, card readers | Not supported. |
| Power | `power shutdown` and `power reboot` use ACPI. Sleep and battery status are not supported. |
| Linux driver domain | Starts automatically if VT-x and EPT are available. It does not yet drive any hardware. |

Missing drivers are expected, not bugs. Providing drivers through the Linux driver domain is the focus of the next milestone.

## Storage safety

I cannot guarantee that Aurea OS will never modify an internal disk. What I can tell you:

- The shipped configuration sets `storage.internal_readonly=true`, which makes Aurea register internal block devices as read-only.
- In Milestone 1, Aurea does not mount any disk file system from internal storage; the C: drive is held in memory.
- NVMe drives are only detected, not read or written.
- Aurea has not been audited for storage safety, and bugs are likely.

So: **back up anything important before booting Aurea on a machine**, prefer a dedicated test computer, and do not change `storage.internal_readonly` unless you know exactly what you are doing.

## Returning to your normal operating system

1. In Aura, run `power shutdown`. If the machine does not turn off, hold the power button.
2. Remove the USB drive.
3. Start the computer normally.

Aurea does not change firmware settings or boot entries. If you disabled Secure Boot for Aurea, you can enable it again.

To use the USB drive for files again, delete its partitions in Windows Disk Management (or with `diskpart` → `select disk N` → `clean`) and create a new volume. On Linux, use `wipefs -a /dev/sdX` and then create a new partition table.

## Troubleshooting

| Symptom | What to try |
|---|---|
| The USB drive is not in the boot menu | Enable USB boot in the firmware, disable CSM / legacy mode, and make sure you choose the UEFI entry. |
| "Secure Boot violation" or similar | Disable Secure Boot in the firmware settings. |
| The screen stays black after selecting the drive | Wait a minute: verbose boot can be slow on some framebuffers. If nothing appears, the firmware's graphics output is probably not usable by Aurea. |
| Boot stops with text output and does not reach the login screen | Note the last lines on the screen and report them. Hardware initialization failed on that machine. |
| The keyboard does not respond | Try a USB keyboard in a different port, or the internal keyboard if you used an external one. |
| No mouse pointer movement | Touchpads are not supported. Use a USB mouse. |
| `bash` reports that VMX is unavailable | Enable Intel Virtualization Technology (VT-x) in the firmware. AMD systems are not supported. |
| A red "critical system error" screen | A kernel fault. Note the error name and RIP shown, then restart. |
