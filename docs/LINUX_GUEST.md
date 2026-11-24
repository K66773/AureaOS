# The Linux guest: VMX, Bash and the road to a driver domain

Milestone 1 is about one thing: Zero Core can boot a Linux kernel inside a hardware-virtualized guest that it controls, and you can use GNU Bash in that guest from the Aura terminal. This page explains how that works today and what is still missing before it becomes a driver domain.

Three different things are easy to mix up:

1. **A Linux guest boots.** Done in Milestone 1, on real Intel hardware and under QEMU/KVM.
2. **GNU Bash runs inside that guest.** Done in Milestone 1, in the same configurations.
3. **Linux device drivers provide services to Aurea.** Not done. This is the next milestone.

![GNU Bash in the Aura terminal, QEMU/KVM](../assets/screenshots/bash-linux-domain.png)

## Why Intel VMX

Aurea needs a lot of hardware drivers that I cannot write from scratch in reasonable time. Linux has them. The plan is to run Linux as a guest under Zero Core and let it drive selected devices, without making Linux the operating system's kernel. That needs hardware virtualization: the guest must run at native speed, but every access to memory and devices outside its assignment must stop in Zero Core.

Aurea programs Intel VMX directly. There is no KVM, Xen or other host hypervisor underneath. The VMX code uses:

- **EPT** (Extended Page Tables), so the guest's physical memory is a region that Zero Core allocated and maps;
- **unrestricted guest**, so Linux can start in 32-bit protected mode as it expects;
- **external-interrupt exiting**, **acknowledge-interrupt-on-exit** and the **VMX-preemption timer**, so host interrupts and time slices stay with Zero Core while the guest runs (each guest run is bounded to a few milliseconds);
- **interrupt-window exiting**, to inject interrupts into the guest when it can take them.

If one of these is missing, the guest does not start, and Aura says which check failed. AMD SVM is not implemented.

## How Zero Core starts the guest

1. At boot, Zero Core reads the VMX capability MSRs, executes VMXON and VMXOFF as a self-test, and runs a tiny guest that executes CPUID and two HLT instructions (hardware diagnostics H03–H05).
2. With `linux.autostart=true` (the default in the release image), the domain manager allocates 128 MiB of guest memory, maps it through EPT, and loads the Linux kernel and initramfs that are embedded in the Aurea kernel image.
3. Zero Core enters the guest at the kernel's 32-bit entry point with the boot parameters filled in, and runs it in bounded slices from its own scheduler.
4. When the initramfs reports ready, the shell is available to Aura.

`domain linux status`, `domain linux start` and `domain linux stop` control the guest by hand.

## What the guest is

| | Build 02416 |
|---|---|
| Kernel | Linux 6.18.49, unmodified source, small guest configuration |
| Command line | `console=ttyS0,115200 earlycon=uart,io,0x3f8,115200n8 panic=-1 aurea_domain=1 noapic nolapic pci=off clocksource=tsc tsc=reliable no_timer_check` |
| CPUs | 1 |
| Memory | 128 MiB |
| Initramfs | GNU Bash 5.2.21 (static), BusyBox 1.36.1 with all applets, two small Aurea helpers, an init script |
| Emulated hardware | 8259 interrupt controller, 8254 timer, MC146818 clock, two 16550 serial ports |
| Not present | PCI, disks, network, graphics, USB, any physical device |

All emulated devices are implemented in Zero Core. Linux sees no real hardware at all.

The source code for the kernel, Bash, BusyBox, the C library and the helpers is on the release page (`AureaOS-Milestone1-third-party-source.tar`).

## Where Bash runs and how Aura talks to it

Bash runs as an ordinary process inside the Linux guest, as root, on the guest's first serial port (`ttyS0`). A helper in the initramfs (`aurea-console-session`) starts it as the session leader, so Bash has a real controlling terminal with job control.

Zero Core's emulated serial port carries the terminal stream between the guest and the Aura window:

- Keystrokes from Aura go into the guest's `ttyS0`.
- Bash's output is decoded by a terminal emulator in Aura (colors, cursor movement, line editing).
- A second serial port (`ttyS1`) carries control messages only: session start and end, window size (so `stty size` and full-screen programs match the Aura window) and redraw requests.

`bash` in Aura attaches the window to that session. `exit` or Ctrl+D ends the shell and returns to Aura; the guest's init starts a new shell for the next `bash`. Ctrl+] detaches and leaves the shell running.

## What the guest can and cannot do

The guest can run the programs in its initramfs: Bash built-ins and the BusyBox tools. Everything it writes stays in its own memory.

It cannot see Aurea's files, the network, disks or any physical device. It cannot run programs that are not in the initramfs, and there is no package manager. Linux applications are not integrated into Aurea; this is a shell inside an isolated guest, not a Linux compatibility layer.

## Verified configurations

| Configuration | Build | Evidence |
|---|---|---|
| HP EliteBook 840 G8, Intel Tiger Lake | 02412 | Two physical boots: H03–H05 passed, Linux ready about 40 s after the guest started (autostart during boot), interactive Bash, `uname -a` |
| HP EliteBook 840 G8 | 02405 | Linux ready 3.3 s after `domain linux start`, interactive Bash |
| QEMU 8.2 with KVM, nested VMX (`-cpu host,+vmx`), 1 and 4 vCPUs | 02416 | Linux ready about 32 s after boot, `bash`, `uname -sr`, `bash --version`, return to Aura |
| QEMU 11 with WHPX on Windows | 02416 | Not available. WHPX does not offer the interrupt and preemption-timer controls listed above, and `bash` reports `bounded-host-irq-controls-unavailable`. |

The current download (02416) has not been booted on real hardware yet.

## Why this is not yet a driver domain

A driver domain needs things Milestone 1 does not have:

- **Device assignment.** Zero Core would pass a real device (its PCI configuration, MMIO ranges, interrupts and DMA) to the guest, with DMA confined to the guest's memory. Today the guest has no PCI bus at all.
- **A service path back to Aurea.** The driver's result, such as a block device, a network interface or input events, has to reach Aurea through a defined channel. Today only a terminal stream and a few control messages cross the boundary.
- **Resource mediation and recovery.** Zero Core has to decide which device belongs to whom, notice a driver crash and restart the guest without taking Aurea down.

The next milestone works on exactly these points. Until real device operations go through the guest, I will describe it as a Linux guest, not as a working driver domain.

## The file-system guest

The Aurea kernel also carries a second Linux build (`6.18.49-aureafs1`, with FUSE built in) for an experiment with file-system access through a guest. It is not used in normal operation in Milestone 1.
