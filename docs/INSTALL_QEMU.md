# Running Aurea OS in QEMU

QEMU is the recommended way to run Aurea OS Milestone 1.1. This guide covers Windows and Linux hosts.

- [Requirements](#requirements)
- [Windows](#windows)
- [Linux](#linux)
- [The tested configuration](#the-tested-configuration)
- [Using Aurea in the VM](#using-aurea-in-the-vm)
- [CPU count and SMP](#cpu-count-and-smp)
- [VirtualBox and other hypervisors](#virtualbox-and-other-hypervisors)
- [Troubleshooting](#troubleshooting)

## Requirements

- An x86-64 host. Aurea is an x86-64 operating system; ARM hosts would need full emulation and were not tested.
- 2 GB of free memory for the VM (1 GiB guest memory plus QEMU overhead).
- From the [release page](../../../releases/tag/milestone-1.1): `AureaOS-Milestone1.1-build02450.img` and `SHA256SUMS.txt`, plus `AureaOS-Milestone1.1-tools.zip` on Windows.

## Windows

Tested with QEMU 11.0.50 on Windows 11, using WHPX acceleration. Under WHPX the Linux guest runs on [SoftCPU](SOFTCPU.md); `bash` works, and so does the network card driven by Linux.

1. **Install QEMU.** Download the 64-bit installer from <https://qemu.weilnetz.de/w64/> and install it to the default location (`C:\Program Files\qemu`). The installer includes the UEFI firmware Aurea needs (`share\edk2-x86_64-code.fd`).
2. **Enable hardware acceleration (recommended).** Open *Turn Windows features on or off*, enable **Windows Hypervisor Platform**, and restart. Without it QEMU uses its software emulator (TCG), which works but is much slower: in one test the desktop took about three minutes to appear after login.
3. **Prepare a folder.** Extract `AureaOS-Milestone1.1-tools.zip`, then put the `.img` file and `SHA256SUMS.txt` in the same folder.
4. **Start Aurea.** Double-click `Aurea Live VM.cmd`. It finds QEMU, checks the image's SHA-256, and opens the VM window.

The launcher accepts options when started from a terminal:

```bat
"Aurea Live VM.cmd" -MemoryMB 2048
"Aurea Live VM.cmd" -Image D:\Downloads\AureaOS-Milestone1.1-build02450.img
"Aurea Live VM.cmd" -Accel tcg
"Aurea Live VM.cmd" -NoNetwork
"Aurea Live VM.cmd" -DryRun
```

`-DryRun` prints the full QEMU command line without starting anything. If QEMU is installed somewhere unusual, set `AUREA_QEMU` to the full path of `qemu-system-x86_64.exe`. Extra QEMU arguments can be added through the `AUREA_QEMU_EXTRA_ARGS` environment variable.

The launcher keeps a writable copy of the UEFI variable store in `%LOCALAPPDATA%\AureaOS\vm\` and writes the serial console to `%LOCALAPPDATA%\AureaOS\logs\serial-<date>-<time>.log`. That log is the first thing to look at when something goes wrong, and it is useful in bug reports.

The image is opened in snapshot mode. Changes made inside Aurea never reach the `.img` file, and no host disk or USB device is attached.

The 1.1 launcher differs from the Milestone 1 one: it starts a `q35` machine with an emulated Intel IOMMU (`intel-iommu,intremap=off`), runs WHPX with `kernel-irqchip=off`, and gives the guest CPU SSSE3, SSE4.1/4.2 and POPCNT. The IOMMU is what lets Aurea assign the network card to the Linux guest. `-NoNetwork` removes the card, and with it the device the Linux driver would drive.

## Linux

Tested with QEMU 8.2.2 and OVMF from Ubuntu 24.04 (under WSL 2), with KVM.

Install QEMU and the UEFI firmware:

```sh
sudo apt install qemu-system-x86 ovmf        # Debian, Ubuntu
sudo dnf install qemu-system-x86 edk2-ovmf   # Fedora
```

Check the image and start it:

```sh
sha256sum -c SHA256SUMS.txt --ignore-missing
cp /usr/share/OVMF/OVMF_VARS_4M.fd aurea-vars.fd

qemu-system-x86_64 \
  -machine q35 -accel kvm -cpu host -smp 1 -m 1024M \
  -device intel-iommu,intremap=off \
  -device virtio-vga,edid=on,xres=1920,yres=1080 \
  -device piix3-usb-uhci,id=uhci -device usb-tablet,bus=uhci.0,port=1 \
  -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
  -drive if=pflash,format=raw,file=aurea-vars.fd \
  -drive file=AureaOS-Milestone1.1-build02450.img,format=raw,media=disk,snapshot=on \
  -boot order=c -nic user,model=e1000 -no-reboot \
  -serial file:aurea-serial.log
```

Firmware file names differ between distributions. Fedora uses `/usr/share/edk2/ovmf/OVMF_CODE.fd` and `OVMF_VARS.fd`; Arch uses `/usr/share/edk2/x64/`. Use the non-Secure-Boot variants: the Aurea boot files are unsigned.

### Bash on a Linux host

On a Linux host, Aurea uses VMX for the Linux guest when the virtual CPU offers it, and SoftCPU otherwise. With nested virtualization on an Intel host, `-cpu host` passes VMX through:

```sh
cat /sys/module/kvm_intel/parameters/nested   # must print Y or 1
# -cpu host then includes VMX; -cpu host,+vmx asks for it explicitly
```

With nested VMX, build 02450 reported the Linux domain ready 11–17 seconds after boot in my two tests (one of them with exactly the command above), with the network card bound by Linux and DHCP done. Nested VM exits are expensive, which is why this takes longer than SoftCPU under WHPX; VMX is the path meant for real hardware. AMD hosts were not tested.

## The tested configuration

| Setting | Value | Why |
|---|---|---|
| Machine | `q35` with `intel-iommu,intremap=off` | The IOMMU is required for assigning the network card to the Linux guest. |
| Firmware | edk2 / OVMF, 4 MiB build, no Secure Boot | Aurea boots through UEFI only. |
| Accelerator | WHPX with `kernel-irqchip=off` (Windows), KVM (Linux), TCG as fallback | |
| CPU | `qemu64,+ssse3,+sse4.1,+sse4.2,+popcnt` on Windows, `host` on Linux | |
| vCPUs | 1 | The release configuration starts one CPU; see below. |
| Memory | 1024 MiB | 512 MiB also reached the desktop in one test. |
| Graphics | `virtio-vga`, 1920×1080 through EDID | Aurea uses the UEFI framebuffer. |
| Keyboard | Emulated PS/2 | |
| Mouse | `usb-tablet` on a UHCI controller | Absolute positioning, so the pointer follows the host cursor. |
| Network | `e1000`, user-mode networking | Driven by Linux in the guest when the IOMMU is present. Optional. |
| Disk | The release image, raw, snapshot mode | |
| Reboot | `-no-reboot` | A guest restart ends QEMU instead of looping. |

The serial log is verbose: the shipped configuration logs at trace level.

## Using Aurea in the VM

- The login screen appears after roughly 30–40 seconds with acceleration. Log in as **`aura`**, password **`root`**.
- The keyboard behaves as a **German QWERTZ** keyboard regardless of your host layout, because QEMU passes physical key positions. On a US keyboard, Y and Z are swapped and symbols sit in different places. `keyboard layout set ENG` in the Aura terminal switches Aurea to US QWERTY until the next boot.
- Click inside the window to give it keyboard focus. If QEMU grabs the mouse, Ctrl+Alt+G releases it.
- To stop the VM, run `power shutdown` in the Aura terminal or close the QEMU window. Nothing is saved either way. Under KVM, `power shutdown` did not turn the VM off in one test; close the window in that case.

## CPU count and SMP

The release image starts **one CPU**, even if QEMU provides more: `boot.config` on the image contains `boot.cores=1`. I kept that default because multi-core operation has not been tested on hardware yet.

In Milestone 1, four cores worked in QEMU with build 02416. With `boot.cores=all` and four virtual CPUs, all four came online and were scheduled, Task Manager showed live per-core load, native programs ran, and under KVM the Linux guest started and Bash worked. I tested this under WHPX and KVM for a few minutes at a time, not in long runs, and not again with build 02450.

To try it, change the setting in the image's EFI partition. With mtools (Linux or WSL):

```sh
for f in boot.config Aurea/boot.config simply.config; do
  mcopy -n -i AureaOS-Milestone1.1-build02450.img@@1M ::/$f cfg && \
  sed -i 's/^boot.cores=1/boot.cores=all/' cfg && \
  mcopy -o -i AureaOS-Milestone1.1-build02450.img@@1M cfg ::/$f
done
```

Then start the VM with `"Aurea Live VM.cmd" -Cpus 4` or `-smp 4`. Changing the image changes its checksum, so the launcher will report a mismatch with `SHA256SUMS.txt`; keep a copy of the original.

## VirtualBox and other hypervisors

VirtualBox, VMware and Hyper-V have **not** been verified with this release. Earlier development builds were tested in VirtualBox, but that says nothing about build 02450. Use QEMU.

## Troubleshooting

| Symptom | What to try |
|---|---|
| `QEMU was not found` | Install QEMU for Windows or set `AUREA_QEMU` to the full path of `qemu-system-x86_64.exe`. |
| `UEFI firmware not found` | The QEMU installation is incomplete. Reinstall it; the `share` folder must contain `edk2-x86_64-code.fd` and `edk2-i386-vars.fd`. |
| `Checksum mismatch` | The download is damaged. Download the image again. |
| The window opens and closes immediately | Run `"Aurea Live VM.cmd" -Accel tcg`. If that works, WHPX is missing or blocked; check that *Windows Hypervisor Platform* is enabled. |
| A UEFI shell or boot manager appears instead of Aurea | The firmware did not find the boot disk. Check that you used the `.img` file, not an ISO, and that it is complete. |
| Very slow boot | QEMU is using TCG. Enable Windows Hypervisor Platform (Windows) or KVM (Linux: `ls -l /dev/kvm`). |
| The keyboard types the wrong characters | Aurea expects German QWERTZ. Run `keyboard layout set ENG`. |
| `net status` shows Aurea's own e1000 driver instead of `Linux e1000` | QEMU has no IOMMU, so the card was not assigned to Linux. Use the 1.1 launcher, or add `-machine q35 -device intel-iommu,intremap=off`. |
| A red "critical system error" screen | A kernel fault. Close QEMU and start again. The serial log in `%LOCALAPPDATA%\AureaOS\logs` contains the details. |
| The launcher console shows `warning: Ignoring request for interrupt vector 0` | Harmless QEMU message under WHPX. |
