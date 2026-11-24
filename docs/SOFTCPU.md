# SoftCPU

SoftCPU is an experimental software CPU: an x86-64 interpreter inside the Aurea kernel that executes guest code instruction by instruction, without hardware virtualization.

## Why it exists

The Linux driver domain normally uses Intel VMX. Some hosts cannot provide that to Aurea; Windows QEMU with WHPX is the most common case. SoftCPU lets me study guest execution, paging, faults, interrupts and early Linux bring-up on such hosts, and compare the results with hardware-virtualized runs. It is a development and compatibility research tool.

## Status in Milestone 1

**SoftCPU is not part of the Milestone 1 release build.** Release builds leave the interpreter out; in build 02416, `domain softcpu-exp` reports *experimental interpreter excluded from regular build*. Nothing in the release depends on it.

In my development builds, SoftCPU runs a set of self-tests (basic execution, 4 KiB paging, page-fault recovery, timer interrupts) and can execute the beginning of the Linux kernel's boot under a fixed instruction budget. It does **not** boot Linux to a usable shell, and it is not a substitute for the VMX-based driver domain.

## Performance

SoftCPU is an interpreter and is not optimized. There is no JIT and no binary translation. It is orders of magnitude slower than hardware-assisted execution, and it should not be confused with QEMU's own acceleration (WHPX, KVM) or with running Aurea on real hardware.

I have not published boot-time figures for SoftCPU, because I don't have a measurement for a current build under documented conditions.

## Instruction-set coverage

SoftCPU covers the instructions needed for its self-tests and for the early Linux boot path it has been tested on. I do not claim complete x86-64 coverage, and it does not emulate a full PC.

## Relation to the mainline system

Aurea itself never runs on SoftCPU; it runs on the real (or QEMU-virtualized) CPU. SoftCPU only executes guest code inside Aurea. Progress in SoftCPU experiments does not change what the Milestone 1 desktop can do.
