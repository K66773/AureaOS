# SoftCPU

SoftCPU runs the Linux guest's code without hardware virtualization. It is part of the Aurea kernel and, since Milestone 1.1, part of the release image.

## Why it exists

The Linux guest normally runs on Intel VMX, programmed directly by Zero Core. Some hosts cannot give Aurea the VMX features it needs. QEMU with WHPX on Windows is the most common case: in Milestone 1, `bash` simply failed there. SoftCPU closes that gap, so the Linux guest, Bash and the Linux-driven network card work on such hosts as well.

## When it is used

At domain start, Zero Core checks the CPU. With usable VMX, the guest runs on VMX; without it, on SoftCPU. If VMX fails to start, or the guest faults under VMX before Linux is ready, Zero Core switches to SoftCPU instead of giving up. I tested that switch by injecting VMX failures in a development build under nested KVM, not with build 02450. The boot log says which one is running:

```
[DOMAIN] Linux vCPU: SoftCPU
[SOFTCPU] Linux ready after 2523 ms: ... rate=1150 M/s
```

The guest is the same in both cases: same kernel, same initramfs, same serial console, the same device assignment and network path. Aura cannot tell the difference except by speed.

## How it works

SoftCPU translates guest x86-64 code into host x86-64 code in blocks and caches the translations. Guest memory goes through its own page-table walk and a software TLB, interrupts and exceptions are delivered between blocks, and the emulated devices are the same ones the VMX path uses. A single-step executor is kept as a reference; in development builds I run both side by side and compare their state after every block.

It runs on the host CPU in bounded slices from Aurea's scheduler, so the desktop keeps running while Linux boots.

## Performance

In my tests of build 02450 under QEMU/WHPX on one Windows 11 machine (ten sessions, one virtual CPU), Linux was ready 2.5–2.7 seconds after the domain started, at about 1.1–1.2 billion guest instructions per second. Under nested KVM, the VMX path took 8–14 seconds for the same step, because nested VM exits are expensive there; on real hardware there are no nested exits. These are measurements on one host, not benchmarks.

The first version of SoftCPU was an interpreter and needed several minutes to reach a shell. The translator replaced it for the guest.

## Limits

- x86-64 coverage is what the Linux guest and its programs need. I do not claim complete instruction-set coverage.
- The translation cache is writable and executable kernel memory. It is not W^X yet.
- Some exception, self-modifying-code and privilege edge cases have not been tested systematically.
- SSE and x87 instructions are executed with the host's own floating-point unit.
- The guest's isolation under SoftCPU has not been audited.

## Relation to the rest of the system

Aurea itself never runs on SoftCPU; it runs on the real (or QEMU-virtualized) CPU. SoftCPU only executes the Linux guest.
