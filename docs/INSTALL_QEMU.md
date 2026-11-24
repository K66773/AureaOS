# Running Aurea OS in QEMU

QEMU is the recommended way to run Aurea OS Milestone 1. This guide covers Windows and Linux hosts.

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
- From the [release page](../../../releases/tag/milestone-1): `AureaOS-Milestone1-build02416.img` and `SHA256SUMS.txt`, plus `AureaOS-Milestone1-tools.zip` on Windows.

## Windows

Tested with QEMU 11.0.50 on Windows 11, using WHPX acceleration.

1. **Install QEMU.** Download the 64-bit installer from <https://qemu.weilnetz.de/w64/> and install it to the default location (`C:\Program Files\qemu`). The installer includes the UEFI firmware Aurea needs (`share\edk2-x86_64-code.fd`).
2. **Enable hardware acceleration (recommended).** Open *Turn Windows features on or off*, enable **Windows Hypervisor Platform**, and restart. Without it QEMU uses its software emulator (TCG), which works but is much slower: in one test the desktop took about three minutes to appear after login.
3. **Prepare a folder.** Extract `AureaOS-Milestone1-tools.zip`, then put the `.img` file and `SHA256SUMS.txt` in the same folder.
4. **Start Aurea.** Double-click `Aurea Live VM.cmd`. It finds QEMU, checks the image's SHA-256, and opens the VM window.

The launcher accepts options when started from a terminal:

```bat
"Aurea Live VM.cmd" -MemoryMB 2048
"Aurea Live VM.cmd" -Image D:\Downloads\AureaOS-Milestone1-build02416.img
"Aurea Live VM.cmd" -Accel tcg
"Aurea Live VM.cmd" -NoNetwork
"Aurea Live VM.cmd" -DryRun
```

`-DryRun` prints the full QEMU command line without starting anything. If QEMU is installed somewhere unusual, set `AUREA_QEMU` to the full path of `qemu-system-x86_64.exe`. Extra QEMU arguments can be added through the `AUREA_QEMU_EXTRA_ARGS` environment variable.

The launcher keeps a writable copy of the UEFI variable store in `%LOCALAPPDATA%\AureaOS\vm\` and writes the serial console to `%LOCALAPPDATA%\AureaOS\logs\serial-<date>-<time>.log`. That log is the first thing to look at when something goes wrong, and it is useful in bug reports.

The image is opened in snapshot mode. Changes made inside Aurea never reach the `.img` file, and no host disk or USB device is attached.

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
  -machine pc,usb=off -accel kvm -cpu host -smp 1 -m 1024M \
  -device virtio-vga,edid=on,xres=1920,yres=1080 \
  -device piix3-usb-uhci,id=uhci -device usb-tablet,bus=uhci.0,port=1 \
  -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
  -drive if=pflash,format=raw,file=aurea-vars.fd \
  -drive file=AureaOS-Milestone1-build02416.img,format=raw,media=disk,snapshot=on \
  -boot order=c -nic user,model=e1000 -no-reboot \
  -serial file:aurea-serial.log
```

Firmware file names differ between distributions. Fedora uses `/usr/share/edk2/ovmf/OVMF_CODE.fd` and `OVMF_VARS.fd`; Arch uses `/usr/share/edk2/x64/`. Use the non-Secure-Boot variants: the Aurea boot files are unsigned.

### Bash on a Linux host

The `bash` command needs Intel VT-x with EPT inside the guest. On an Intel host with nested virtualization enabled, add `+vmx` to the CPU:

```sh
cat /sys/module/kvm_intel/parameters/nested   # must print Y or 1
# in the command above:  -cpu host,+vmx
```

With that setting the Linux domain reported ready about 32 seconds after boot in my test, and `bash` opened a working shell. AMD hosts were not tested; the Linux domain uses Intel VMX.

## The tested configuration

| Setting | Value | Why |
|---|---|---|
| Machine | `pc` (i440FX), `usb=off` | The configuration Aurea is developed against. USB is added explicitly below. |
| Firmware | edk2 / OVMF, 4 MiB build, no Secure Boot | Aurea boots through UEFI only. |
| Accelerator | WHPX (Windows), KVM (Linux), TCG as fallback | |
| CPU | `qemu64` on Windows, `host` on Linux | |
| vCPUs | 1 | The release configuration starts one CPU; see below. |
| Memory | 1024 MiB | 512 MiB also reached the desktop in one test. |
| Graphics | `virtio-vga`, 1920×1080 through EDID | Aurea uses the UEFI framebuffer. |
| Keyboard | Emulated PS/2 | |
| Mouse | `usb-tablet` on a UHCI controller | Absolute positioning, so the pointer follows the host cursor. |
| Network | `e1000`, user-mode networking | Needed for `http get`. Optional. |
| Disk | The release image, raw, snapshot mode | |
| Reboot | `-no-reboot` | A guest restart ends QEMU instead of looping. |

The serial log is verbose in Milestone 1: the shipped configuration logs at trace level.

## Using Aurea in the VM

- The login screen appears after roughly 30–40 seconds with acceleration. Log in as **`Aura`**, password **`root`**.
- The keyboard behaves as a **German QWERTZ** keyboard regardless of your host layout, because QEMU passes physical key positions. On a US keyboard, Y and Z are swapped and symbols sit in different places. `keyboard layout set ENG` in the Aura terminal switches Aurea to US QWERTY until the next boot.
- Click inside the window to give it keyboard focus. If QEMU grabs the mouse, Ctrl+Alt+G releases it.
- To stop the VM, run `power shutdown` in the Aura terminal or close the QEMU window. Nothing is saved either way. Under KVM, `power shutdown` did not turn the VM off in one test; close the window in that case.

## CPU count and SMP

The release image starts **one CPU**, even if QEMU provides more: its `boot.config` contains `boot.cores=1`. Aurea has SMP code (AP start-up, per-CPU structures, scheduler support), but multi-CPU operation has not been verified for this build, so the release keeps it off.

To experiment anyway, change `boot.cores=1` to `boot.cores=all` in `boot.config` on the image's EFI partition, and start the VM with `-Cpus 2` (Windows launcher) or `-smp 2`. Treat any result as experimental.

## VirtualBox and other hypervisors

VirtualBox, VMware and Hyper-V have **not** been verified with this release. Earlier development builds were tested in VirtualBox, but that says nothing about build 02416. Use QEMU.

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
| `bash` prints `start failed ... bounded-host-irq-controls-unavailable` | The host does not expose the needed VMX features. Expected on Windows; use a Linux host with nested KVM. |
| A red "critical system error" screen | A kernel fault. Close QEMU and start again. The serial log in `%LOCALAPPDATA%\AureaOS\logs` contains the details. |
| The launcher console shows `warning: Ignoring request for interrupt vector 0` | Harmless QEMU message under WHPX. |
