# Aura command reference

Aura is the Aurea OS command environment. Open it from the taskbar (the `>_` icon), from the desktop icon *Aura Shell*, or from the Start menu.

This reference covers build 02416. Status values:

- **Verified**: I ran the command in the release build and it behaved as described.
- **Registered**: the command is registered and its syntax is taken from the command registry, but it was not individually tested for this release.

## Basics

- `help` lists the command categories. `help <category>` lists a category, `help command <name>` shows one command, and `commands --all` lists everything.
- Paths use drive letters, for example `C:/Temp/file.txt`. Aura prints paths with backslashes (`C:\Temp\file.txt`). The prompt shows the current directory: `Aura C:/>`.
- `history` lists the commands entered in this session.
- The keyboard follows the German QWERTZ layout unless you run `keyboard layout set ENG`.
- Each command has a required permission level. The development account `aura` is an administrator.

## Aura commands and Bash commands

Aura commands are part of Aurea OS. Some share names with Unix tools (`ls`, `cd`, `ps`, `kill`), but they are separate implementations with their own options.

GNU Bash is different: `bash` starts a shell inside the Linux driver domain (see the README's [GNU Bash section](../README.md#gnu-bash-and-linux-integration)). Inside it, the prompt changes to `root@linux:~#`, and commands are the Bash built-ins and BusyBox tools of a small Linux system. They act on the Linux guest's own in-memory file system, not on Aurea's C: drive.

| Key | In a Bash session |
|---|---|
| `exit` or Ctrl+D | End Bash and return to Aura |
| Ctrl+] or Ctrl+Shift+J | Return to Aura and leave Bash running; `bash` reattaches |
| Ctrl+C | Copy if text is selected, otherwise interrupt the running program |
| Ctrl+V | Paste |

Bash needs VT-x with EPT and is not available in Windows QEMU.

## Everyday commands

| Command | Purpose | Syntax | Status | Notes |
|---|---|---|---|---|
| `help` | Show command help | `help [command\|category]` | Verified | Lists categories and usage. |
| `aurea` | System information | `aurea -info` | Verified | `aurea -info` and `aurea --info` print system and hardware information. |
| `smp` | Symmetric multiprocessing | `smp status \| smp selftest \| smp timers \| smp idle \| smp start \| smp test \| smp context-cert [short\|scale\|full] \| smp inject-mask <cpu>` | Verified | `smp status` verified with one and with four CPUs. |
| `commands` | List registered commands | `commands [--all] [category]` | Registered |  |
| `clear` | Clear the screen | `clear` | Verified |  |
| `echo` | Print text | `echo <text...>` | Verified |  |
| `history` | Show command history | `history` | Registered |  |
| `exit` | Exit shell session | `exit` | Registered |  |
| `whoami` | Execution identity | `whoami` | Verified | Prints `Aura (ADMIN)` for the development account. |
| `version` | Show version | `version` | Verified | Prints `AureaOS 0.02.0 (02416)`. |
| `about` | About AureaOS | `about` | Verified |  |
| `date` | Show date | `date` | Verified | Prints the date as `YYYY-MM-DD`. |
| `time` | Show time | `time \| time status` | Registered |  |
| `uptime` | Show uptime | `uptime` | Registered |  |
| `sysinfo` | System summary | `sysinfo` | Registered |  |
| `keyboard` | Keyboard status and layout | `keyboard status \| keyboard layout \| keyboard layout set GER\|ENG` | Verified | `keyboard layout set ENG` switched to US QWERTY in testing; not saved. |
| `dir` | List directory | `dir [--tree\|-T\|-R] [path]` | Verified |  |
| `ls` | List directory | `ls [--tree\|-T\|-R] [path]` | Verified | Same as `dir`. |
| `cd` | Change directory | `cd <path> \| cd .. \| cd -` | Registered |  |
| `pwd` | Print working directory | `pwd` | Registered |  |
| `mkdir` | Create directory | `mkdir <path>` | Verified |  |
| `rmdir` | Remove directory | `rmdir <path>` | Registered |  |
| `touch` | Create empty file | `touch <path>` | Registered |  |
| `write` | Write file text | `write <file> <text>` | Verified | Writes the first word of the text only; quoting was not tested. |
| `type` | Display file contents | `type <file>` | Verified |  |
| `cp` | Copy path | `cp <src> <dst>` | Verified |  |
| `mv` | Move/rename | `mv <src> <dst>` | Registered |  |
| `rm` | Remove path | `rm <path>` | Verified |  |
| `stat` | Path metadata | `stat <path>` | Registered |  |
| `ps` | Process list | `ps` | Verified |  |
| `tasklist` | Process list | `tasklist [--details]` | Registered |  |
| `kill` | Stop process | `kill <pid>` | Registered |  |
| `app` | Application management | `app <list\|running\|info\|start\|stop> ...` | Verified | `app list` verified. |
| `run` | Run executable | `run [-d\|--diag] <path> [--] [args...]` | Verified | Verified with SDK-built `.run` and `.aex` files and the bundled sample, with and without arguments. |
| `explorer` | Start Explorer | `explorer` | Registered |  |
| `settings` | Settings | `settings \| settings debug layout on\|off` | Registered |  |
| `taskmgr` | Task Manager | `taskmgr` | Verified |  |
| `errorcenter` | Error Center | `errorcenter` | Verified |  |
| `devmgr` | Device Manager | `devmgr` | Registered |  |
| `terminal` | Start terminal | `terminal` | Registered |  |
| `bash` | Linux shell | `bash` | Verified | Works under QEMU/KVM with nested VMX; fails with a clear message under WHPX. |
| `linux` | Linux console | `linux [start\|exec <cmd>\|stop\|status]` | Registered |  |
| `net` | Network stack | `net list\|status\|health\|wireless\|dhcp renew` | Verified | `net status` verified. |
| `ip` | IP addresses and routes | `ip addr\|route` | Verified | `ip addr` verified. |
| `ping` | ICMP echo | `ping [-n count] [-t] [--timeout ms] <ipv4\|hostname>` | Registered |  |
| `dns` | DNS resolver | `dns lookup <hostname> \| dns set <ipv4>` | Registered |  |
| `http` | HTTP download | `http get <host\|ipv4> <port> <path> <out_path>` | Verified | `http get` verified: byte-exact transfer, terminal keeps working afterwards. |
| `disk` | Block storage | `disk [list\|status\|info <name>\|partitions <name>\|probe <name>\|diskpart ...]` | Verified | `disk list` and `disk partitions <name>` verified. |
| `mount` | Mount manager | `mount list \| mount <device> <D:> [raw]` | Verified | Records a mount entry only; no file system is attached. With exactly three arguments the file system argument is ignored. |
| `users` | List user accounts | `users` | Registered |  |
| `passwd` | Set user password | `passwd <user> [password]` | Registered |  |
| `logout` | End interactive session | `logout` | Registered |  |
| `lock` | Lock interactive session | `lock` | Registered |  |
| `power` | Power service | `power status \| power reboot\|shutdown\|halt` | Verified | `power shutdown` ended the VM under WHPX; under KVM the VM did not power off in one test. |

## Complete registry

Every command registered in build 02416, grouped as `help` groups them. *Registered* means the command exists and its usage line is taken from the registry; it was not individually exercised for this release. Commands in **Diagnostics** and many in **System**, **Display** and **Input** are development tools: they print internal state, and some run stress tests or trigger deliberate faults (`panic-test`).

### Applications

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `app` | Application management | `app <list\|running\|info\|start\|stop> ...` | Verified |
| `aurea` | System information | `aurea -info` | Verified |
| `container` | Classify/inspect AEX/RUN | `container classify\|inspect <path>` | Registered |
| `devmgr` | Device Manager | `devmgr` | Registered |
| `devmgr.run` | Start Device Manager | `devmgr.run` | Registered |
| `errorcenter` | Error Center | `errorcenter` | Verified |
| `errorcenter.run` | Start Error Center | `errorcenter.run` | Registered |
| `explorer` | Start Explorer | `explorer` | Registered |
| `explorer.run` | Start Explorer | `explorer.run` | Registered |
| `horizon` | Horizon desktop shell | `horizon <start\|stop\|restart\|status>` | Registered |
| `horizon.run` | Start Horizon | `horizon.run` | Registered |
| `installer` | Package installer UI | `installer [auto] <path>` | Registered |
| `open` | Open by association | `open <path>` | Registered |
| `pkg` | Package manager | `pkg inspect\|install\|list\|info\|verify\|repair\|uninstall\|launch <...>` | Registered |
| `run` | Run executable | `run [-d\|--diag] <path> [--] [args...]` | Verified |
| `settings` | Settings | `settings \| settings debug layout on\|off` | Registered |
| `settings.run` | Start Settings | `settings.run` | Registered |
| `taskmgr` | Task Manager | `taskmgr` | Verified |
| `taskmgr.run` | Start Task Manager | `taskmgr.run` | Registered |
| `terminal` | Start terminal | `terminal` | Registered |
| `terminal.run` | Start Aura Shell | `terminal.run` | Registered |

### Audio

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `audio` | Audio stack | `audio list\|status\|health\|test\|volume <0-100>\|mute\|unmute\|stats` | Registered |

### Bluetooth

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `bluetooth` | Bluetooth foundation | `bluetooth list\|status\|health\|scan\|pair <id>\|connect <id>\|disconnect <id>\|forget <id>` | Registered |

### Cache

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `cache` | Cache management | `cache <status\|list\|clear> [name]` | Registered |

### Configuration

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `config` | Configuration store | `config <list\|get\|set> ...` | Registered |
| `reg` | Configuration registry | `reg <list\|query> ...` | Registered |
| `sysconf` | System configuration | `sysconf <get\|set\|delete\|list\|status\|type> ...` | Registered |

### Desktop

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `desktop` | Desktop shell | `desktop <status\|restart>` | Registered |
| `window` | Window management | `window <list\|info\|close> ...` | Registered |

### Diagnostics

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `bootui` | BootDisplay diagnostics | `bootui status\|handoff-test\|fallback-test\|api-probe` | Registered |
| `bringup` | Generic hardware bring-up inventory | `bringup` | Registered |
| `command` | Command registry | `command verify\|list [--all]\|info <name>\|test <name>\|audit` | Registered |
| `crash` | Crash reports | `crash list \| crash info <pid>` | Registered |
| `diagnose` | Diagnostics | `diagnose <system\|logs>` | Registered |
| `dmesg` | Kernel log dump | `dmesg [--level <lvl>] [--facility <cat>] [--since <ms>] [--clear]` | Registered |
| `domain` | Driver domain primitives | `domain status\|vmm-probe\|softcpu-exp\|independent\|linux status\|start\|stop\|restart\|console\|cert [case] [cycles]` | Verified |
| `error` | Error Center / codes | `error list\|show <id>\|stats\|clear\|info <code>\|last\|test warning\|process\|driver\|service\|critical-sim\|storm` | Registered |
| `event` | Event service | `event list\|watch [FACILITY]\|stats\|clear\|emit-test [count]` | Registered |
| `font` | Font diagnostics | `font status\|report\|test\|corpus\|cache-bench\|scale-cycle [N] \| font scale <pct> \| font fallback on\|off \| font ab on\|off \| font reset-stats` | Registered |
| `fps` | FPS overlay | `fps \| fps -s \| fps status \| fps detailed` | Registered |
| `frame` | Frame timing | `frame timing` | Registered |
| `frame-test` | Frame scheduler test | `frame-test start 60\|30\|10 \| stop \| status` | Registered |
| `graphics` | Graphics statistics | `graphics stats [reset] \| graphics overlay on\|off \| graphics dirty-view on\|off \| graphics test atomic-present \| graphics cache status\|clear` | Registered |
| `hwdiag` | Native hardware diagnostic | `hwdiag [vmxon\|msrtest]` | Registered |
| `kernel-watch` | Kernel Watcher | `kernel-watch status\|incidents\|incident <id>\|cpus\|workers\|processes\|devices\|services` | Registered |
| `log` | Kernel log control | `log status \| log stats \| log show [level] [category] \| log clear \| log level ...` | Registered |
| `profiler` | Graphics profiler | `profiler [status\|reset]` | Registered |
| `resource` | Resource diagnostics | `resource status \| resource stress <cycles> \| resource stress status \| resource stress results \| resource fault-test <surface\|process>` | Registered |
| `runtime` | Runtime authority snapshot | `runtime context` | Registered |
| `ui` | UI diagnostics | `ui hitboxes on\|off \| ui perf on\|off\|status` | Registered |
| `ui-perf` | UI performance | `ui-perf baseline\|status\|trace \| ui-perf recovery status\|clear\|test` | Registered |
| `zerocore-probe` | ZeroCore authority probe | `zerocore-probe` | Registered |

### Display

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `compositor` | Compositor diagnostics | `compositor dirty\|status` | Registered |
| `display` | Display diagnostics | `display status \| display pixel-format \| display test \| display driver \| display stats \| display perf \| display scenario run\|status\|abort \| display frame-rate [25\|30\|60\|120\|uncapped\|unlock] \| display vsync-status` | Registered |
| `icon` | Icon cache | `icon cache clear\|reload\|status \| icon status` | Registered |
| `popup` | Popup pipeline | `popup status` | Registered |
| `surface` | Surface diagnostics | `surface list \| surface inspect <id> \| surface test` | Registered |
| `wallpaper` | Wallpaper pipeline | `wallpaper status \| wallpaper mode <Fill\|Fit\|Center\|Stretch\|Tile> \| wallpaper import <path> \| wallpaper cache clear\|reload\|status` | Registered |

### Files

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `cd` | Change directory | `cd <path> \| cd .. \| cd -` | Registered |
| `cp` | Copy path | `cp <src> <dst>` | Verified |
| `create` | Create file or folder | `create <path> \| create --folder <name>` | Registered |
| `dir` | List directory | `dir [--tree\|-T\|-R] [path]` | Verified |
| `directory` | Directory operations | `directory <create\|delete> <path>` | Registered |
| `file` | File operations | `file <list\|info\|read\|create\|write\|pattern-write\|hash\|size\|delete\|copy\|move> ...` | Registered |
| `ls` | List directory | `ls [--tree\|-T\|-R] [path]` | Verified |
| `mkdir` | Create directory | `mkdir <path>` | Verified |
| `mv` | Move/rename | `mv <src> <dst>` | Registered |
| `pwd` | Print working directory | `pwd` | Registered |
| `rm` | Remove path | `rm <path>` | Verified |
| `rmdir` | Remove directory | `rmdir <path>` | Registered |
| `stat` | Path metadata | `stat <path>` | Registered |
| `touch` | Create empty file | `touch <path>` | Registered |
| `type` | Display file contents | `type <file>` | Verified |
| `write` | Write file text | `write <file> <text>` | Verified |

### Input

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `cursor` | Cursor status | `cursor status \| cursor debug on\|off` | Registered |
| `input` | Input diagnostics | `input status \| input selftest \| input test \| input debug pointer on\|off \| input debug widgets \| input pointer status\|stats\|raw on\|off\|trace[ on\|off]\|event stats \| input hid layout\|descriptor \| input capture status \| input event stats \| input backend status \| input report stats \| input transition stats` | Registered |
| `keyboard` | Keyboard status and layout | `keyboard status \| keyboard layout \| keyboard layout set GER\|ENG` | Verified |
| `pointer` | Pointer input | `pointer status \| events \| backend \| devices \| capture \| reset \| pointer debug on \| pointer debug off \| pointer test` | Registered |
| `usb` | USB stack | `usb list \| usb tree \| usb device <id> \| usb rescan` | Registered |

### Memory

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `meminfo` | Memory information | `meminfo` | Registered |
| `memory` | Memory statistics | `memory [status]` | Registered |

### Network

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `dns` | DNS resolver | `dns lookup <hostname> \| dns set <ipv4>` | Registered |
| `http` | HTTP download | `http get <host\|ipv4> <port> <path> <out_path>` | Verified |
| `ip` | IP addresses and routes | `ip addr\|route` | Verified |
| `net` | Network stack | `net list\|status\|health\|wireless\|dhcp renew` | Verified |
| `ping` | ICMP echo | `ping [-n count] [-t] [--timeout ms] <ipv4\|hostname>` | Registered |
| `socket` | Socket smoke test | `socket test` | Registered |
| `tcp` | TCP echo test | `tcp echo-test <ipv4> <port> <bytes>` | Registered |

### Performance

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `perf` | Subsystem profilers | `perf status\|reset\|start\|stop\|snapshot\|report\|scenario\|cpu\|scheduler\|...` | Registered |
| `performance` | Performance metrics | `performance <status\|cpu\|process\|memory>` | Registered |

### Processes

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `kill` | Stop process | `kill <pid>` | Registered |
| `process` | Process management | `process <list\|info\|stats\|stop\|suspend\|resume\|tree\|metrics\|reaper status> ...` | Registered |
| `ps` | Process list | `ps` | Verified |
| `taskinfo` | Process details | `taskinfo <pid>` | Registered |
| `tasklist` | Process list | `tasklist [--details]` | Registered |

### Services

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `service` | Service management | `service <list\|status\|info\|start\|stop\|restart\|enable\|disable> ...` | Registered |

### Shell

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `clear` | Clear the screen | `clear` | Verified |
| `commands` | List registered commands | `commands [--all] [category]` | Registered |
| `echo` | Print text | `echo <text...>` | Verified |
| `environment` | Show environment | `environment` | Registered |
| `exit` | Exit shell session | `exit` | Registered |
| `help` | Show command help | `help [command\|category]` | Verified |
| `history` | Show command history | `history` | Registered |
| `jobwait` | Deterministic job delay | `jobwait <milliseconds>` | Registered |
| `whoami` | Execution identity | `whoami` | Verified |

### Storage

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `blk` | Block I/O | `blk read <name> <lba> \| blk writetest <name> <lba> \| blk flush <name> \| blk cache` | Registered |
| `disk` | Block storage | `disk [list\|status\|info <name>\|partitions <name>\|probe <name>\|diskpart ...]` | Verified |
| `logs` | Logs persistence | `logs [status\|bind\|flush\|session <id>\|note serial\|smp\|linux\|device <text>]` | Registered |
| `mount` | Mount manager | `mount list \| mount <device> <D:> [raw]` | Verified |
| `umount` | Unmount volume | `umount <D:> [--force]` | Registered |

### System

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `about` | About AureaOS | `about` | Verified |
| `acpi` | ACPI tables | `acpi madt \| tables \| fadt \| mcfg` | Registered |
| `apic` | APIC controllers | `apic status` | Registered |
| `authlogin` | Authenticate and start interactive session | `authlogin <user> [password]` | Registered |
| `bash` | Linux shell | `bash` | Verified |
| `boot-policy` | ZEROcore boot policy alias | `boot-policy` | Registered |
| `cpu` | Per-CPU structures | `cpu info <id>` | Registered |
| `cpuinfo` | CPU information | `cpuinfo` | Registered |
| `date` | Show date | `date` | Verified |
| `device` | Device manager | `device list \| tree \| info <id> \| resources <id> \| rescan \| restart <id> \| enable <id> \| disable <id> \| events` | Registered |
| `devices` | Device inventory | `devices` | Registered |
| `devlogin` | DevAutoLogin interactive session | `devlogin` | Registered |
| `driver` | Driver manager | `driver list\|tree\|graphics hardware\|candidates\|status\|set <auto\|vboxsvga\|vmsvga\|gop-recovery>\|restart\|...\|driver <type> list\|status\|candidates\|restart` | Registered |
| `driverpkg` | Driver package manager | `driverpkg inspect\|install\|stage\|remove\|verify <path> [--force]` | Registered |
| `hellopolicy` | Evaluate Hello AutoLogin policy | `hellopolicy` | Registered |
| `jmp` | jump alias | `jmp <console\|horizon\|wm>` | Registered |
| `jump` | Switch foreground presentation | `jump <console\|horizon\|wm>` | Registered |
| `linux` | Linux console | `linux [start\|exec <cmd>\|stop\|status]` | Registered |
| `lock` | Lock interactive session | `lock` | Registered |
| `logout` | End interactive session | `logout` | Registered |
| `mouse` | Mouse diagnostics | `mouse status \| mouse test \| mouse debug on \| mouse debug off` | Registered |
| `passwd` | Set user password | `passwd <user> [password]` | Registered |
| `pci` | PCI bus | `pci list` | Registered |
| `platform` | Platform discovery | `platform [resources]` | Registered |
| `power` | Power service | `power status \| power reboot\|shutdown\|halt` | Verified |
| `sched` | Kernel thread scheduler | `sched status \| audit \| threads \| trace [cpu] \| migration on\|off \| steal on\|off` | Registered |
| `scheduler` | Kernel thread scheduler | `scheduler status \| audit \| threads \| trace [cpu] \| migration on\|off \| steal on\|off` | Registered |
| `session` | Session status and lifecycle tests | `session <subcommand>` | Registered |
| `sessions` | List lifecycle sessions | `sessions [presentation]` | Registered |
| `sh` | Linux console alias | `sh [start\|exec <cmd>\|stop\|status]` | Registered |
| `smp` | Symmetric multiprocessing | `smp status \| smp selftest \| smp timers \| smp idle \| smp start \| smp test \| smp context-cert [short\|scale\|full] \| smp inject-mask <cpu>` | Verified |
| `start` | ZEROcore start alias | `start <component>` | Registered |
| `status` | ZEROcore status alias | `status [component]` | Registered |
| `sysinfo` | System summary | `sysinfo` | Registered |
| `system` | System control | `system <status\|info\|version\|uptime\|reboot\|shutdown\|halt>` | Registered |
| `time` | Show time | `time \| time status` | Registered |
| `timezone` | Timezone | `timezone status \| timezone set Europe/Berlin\|UTC` | Registered |
| `unlock` | Unlock interactive session | `unlock [credential]` | Registered |
| `uptime` | Show uptime | `uptime` | Registered |
| `user` | User account management | `user <info\|create\|delete\|enable\|disable> ...` | Registered |
| `users` | List user accounts | `users` | Registered |
| `verify` | ZEROcore verify | `verify <component\|all>` | Registered |
| `version` | Show version | `version` | Verified |
| `wm` | Window manager diagnostics | `wm status \| wm surfaces \| wm dirty \| wm scene \| wm capture \| wm redraw [window-id] \| wm background status` | Registered |
| `zc-bootcfg` | ZEROcore boot policy | `zc-bootcfg` | Registered |
| `zc-services` | ZEROcore services | `zc-services` | Registered |
| `zc-start` | ZEROcore start component | `zc-start <component>` | Registered |
| `zc-status` | ZEROcore component status | `zc-status [component]` | Registered |
| `zc-verify` | ZEROcore verify | `zc-verify <component\|all>` | Registered |
| `zerocore` | Restore ZeroCore Console | `zerocore` | Registered |

### Tasks

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `task` | Task management | `task <list\|info\|stop\|start> ...` | Registered |
| `taskkill` | Stop application | `taskkill <pid>` | Registered |

### UI

| Command | Purpose | Usage | Status |
|---|---|---|---|
| `dialog` | Dialog system | `dialog status \| dialog test [info\|warning\|error\|question\|confirm\|input\|progress\|progress-indeterminate\|buttons\|credential\|stress]` | Registered |
