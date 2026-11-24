<#
.SYNOPSIS
    Starts the Aurea OS release image in QEMU.

.DESCRIPTION
    Looks for QEMU, its bundled x86_64 UEFI firmware and an Aurea OS disk
    image, then boots the image in a QEMU window. The image is opened in
    snapshot mode: nothing the virtual machine writes reaches the image file.
    No host disk or USB device is attached.

.PARAMETER Image
    Path to the Aurea OS .img file. By default the script looks for
    AureaOS-*.img next to itself and in its parent folder.

.PARAMETER MemoryMB
    Guest memory in MiB. Default 1024, minimum 512.

.PARAMETER Cpus
    Number of virtual CPUs. Default 1. The release image starts only one CPU
    unless boot.cores is changed in its boot.config; see docs/INSTALL_QEMU.md.

.PARAMETER Accel
    whpx, tcg or auto (default). auto tries WHPX and falls back to TCG.

.PARAMETER NoNetwork
    Boot without the emulated network adapter.

.PARAMETER DryRun
    Print the QEMU command line and exit.

.NOTES
    Extra QEMU arguments can be appended through the AUREA_QEMU_EXTRA_ARGS
    environment variable, and a specific QEMU binary can be chosen with
    AUREA_QEMU.
#>
[CmdletBinding()]
param(
    [string]$Image,
    [ValidateRange(512, 65536)][int]$MemoryMB = 1024,
    [ValidateRange(1, 64)][int]$Cpus = 1,
    [ValidateSet('auto', 'whpx', 'tcg')][string]$Accel = 'auto',
    [switch]$NoNetwork,
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'

function Fail([string]$Message, [string]$Hint) {
    Write-Host ''
    Write-Host "Error: $Message" -ForegroundColor Red
    if ($Hint) { Write-Host $Hint }
    exit 1
}

function Find-Qemu {
    $candidates = @()
    if ($env:AUREA_QEMU) { $candidates += $env:AUREA_QEMU }
    $onPath = Get-Command 'qemu-system-x86_64.exe' -ErrorAction SilentlyContinue
    if ($onPath) { $candidates += $onPath.Source }
    foreach ($base in @($env:ProgramFiles, ${env:ProgramFiles(x86)}, "$env:LOCALAPPDATA\Programs")) {
        if ($base) { $candidates += (Join-Path $base 'qemu\qemu-system-x86_64.exe') }
    }
    foreach ($c in $candidates) {
        if ($c -and (Test-Path -LiteralPath $c -PathType Leaf)) { return (Resolve-Path -LiteralPath $c).Path }
    }
    return $null
}

function Find-Firmware([string]$QemuExe) {
    $qemuDir = Split-Path -Parent $QemuExe
    $shareDirs = @((Join-Path $qemuDir 'share'), $qemuDir)
    foreach ($d in $shareDirs) {
        $code = Join-Path $d 'edk2-x86_64-code.fd'
        $vars = Join-Path $d 'edk2-i386-vars.fd'
        if ((Test-Path -LiteralPath $code) -and (Test-Path -LiteralPath $vars)) {
            return @{ Code = $code; Vars = $vars }
        }
    }
    return $null
}

function Find-Image([string]$Requested) {
    if ($Requested) {
        if (-not (Test-Path -LiteralPath $Requested -PathType Leaf)) {
            Fail "Image not found: $Requested"
        }
        return (Resolve-Path -LiteralPath $Requested).Path
    }
    $dirs = @($PSScriptRoot, (Split-Path -Parent $PSScriptRoot), (Get-Location).Path) | Select-Object -Unique
    $found = foreach ($d in $dirs) {
        Get-ChildItem -LiteralPath $d -Filter 'AureaOS-*.img' -File -ErrorAction SilentlyContinue
    }
    $found = @($found | Sort-Object FullName -Unique)
    if ($found.Count -eq 0) {
        Fail 'No Aurea OS image found.' ("Download AureaOS-*.img from the release page and place it next to this script,`n" +
            'or pass its location: "Aurea Live VM.cmd" -Image <path-to-image>')
    }
    if ($found.Count -gt 1) {
        $list = ($found | ForEach-Object { '  ' + $_.FullName }) -join "`n"
        Fail 'More than one Aurea OS image was found.' "Choose one with -Image <path>:`n$list"
    }
    return $found[0].FullName
}

function Test-ImageLayout([string]$Path) {
    $len = (Get-Item -LiteralPath $Path).Length
    if ($len -lt 1MB -or ($len % 512) -ne 0) {
        Fail "This does not look like an Aurea OS disk image: $Path" 'Expected a raw disk image whose size is a multiple of 512 bytes.'
    }
    $fs = [IO.File]::OpenRead($Path)
    try {
        $buf = New-Object byte[] 1024
        [void]$fs.Read($buf, 0, 1024)
    } finally { $fs.Dispose() }
    $mbrOk = ($buf[510] -eq 0x55 -and $buf[511] -eq 0xAA)
    $gptOk = ([Text.Encoding]::ASCII.GetString($buf, 512, 8) -eq 'EFI PART')
    if (-not ($mbrOk -and $gptOk)) {
        if ([IO.Path]::GetExtension($Path) -ieq '.iso') {
            Fail 'ISO files are not used by this launcher.' 'Use the AureaOS-*.img disk image instead.'
        }
        Fail "No GPT partition table found in $Path" 'The file may be incomplete. Download it again and compare its SHA-256 checksum.'
    }
}

function Test-Checksum([string]$Path) {
    $dir = Split-Path -Parent $Path
    $sums = Join-Path $dir 'SHA256SUMS.txt'
    if (-not (Test-Path -LiteralPath $sums)) {
        Write-Host 'Checksum file SHA256SUMS.txt not found next to the image; skipping verification.'
        return
    }
    $name = Split-Path -Leaf $Path
    $line = Get-Content -LiteralPath $sums | Where-Object { $_ -match '^\s*([0-9a-fA-F]{64})\s+\*?(.+?)\s*$' -and $Matches[2] -eq $name } | Select-Object -First 1
    if (-not $line) {
        Write-Host "SHA256SUMS.txt has no entry for $name; skipping verification."
        return
    }
    $expected = ($line -split '\s+')[0].ToLowerInvariant()
    Write-Host "Verifying $name ..."
    $actual = (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($actual -ne $expected) {
        Fail "Checksum mismatch for $name." "Expected $expected`nActual   $actual`nDownload the image again."
    }
    Write-Host 'Checksum OK.'
}

$qemu = Find-Qemu
if (-not $qemu) {
    Fail 'QEMU was not found.' ("Install QEMU for Windows (64-bit) from https://qemu.weilnetz.de/w64/ or https://www.qemu.org/download/,`n" +
        'or set AUREA_QEMU to the full path of qemu-system-x86_64.exe.')
}
$fw = Find-Firmware $qemu
if (-not $fw) {
    Fail "UEFI firmware not found next to $qemu" 'Expected share\edk2-x86_64-code.fd and share\edk2-i386-vars.fd from the QEMU installation.'
}
$img = Find-Image $Image
Test-ImageLayout $img
Test-Checksum $img

# The firmware variable store must be writable; keep a private copy per user.
$stateDir = Join-Path $env:LOCALAPPDATA 'AureaOS\vm'
$logDir = Join-Path $env:LOCALAPPDATA 'AureaOS\logs'
New-Item -ItemType Directory -Force -Path $stateDir, $logDir | Out-Null
$vars = Join-Path $stateDir 'efivars.fd'
if (-not (Test-Path -LiteralPath $vars)) { Copy-Item -LiteralPath $fw.Vars -Destination $vars }
$serialLog = Join-Path $logDir ("serial-{0}.log" -f (Get-Date -Format 'yyyyMMdd-HHmmss'))

$accelArgs = switch ($Accel) {
    'whpx' { @('-accel', 'whpx') }
    'tcg' { @('-accel', 'tcg') }
    default { @('-accel', 'whpx', '-accel', 'tcg') }
}

# QEMU splits -drive options on commas; double them in paths.
function Q([string]$p) { $p.Replace(',', ',,') }

$argv = @('-name', 'Aurea OS', '-machine', 'pc,usb=off') + $accelArgs + @(
    '-cpu', 'qemu64',
    '-smp', "$Cpus",
    '-m', "$($MemoryMB)M",
    '-device', 'virtio-vga,edid=on,xres=1920,yres=1080',
    '-device', 'piix3-usb-uhci,id=uhci',
    '-device', 'usb-tablet,bus=uhci.0,port=1',
    '-drive', "if=pflash,format=raw,readonly=on,file=$(Q $fw.Code)",
    '-drive', "if=pflash,format=raw,file=$(Q $vars)",
    '-drive', "file=$(Q $img),format=raw,media=disk,snapshot=on",
    '-boot', 'order=c',
    '-serial', "file:$(Q $serialLog)",
    '-no-reboot',
    '-display', 'sdl,gl=off'
)
if (-not $NoNetwork) { $argv += @('-nic', 'user,model=e1000') }
if ($env:AUREA_QEMU_EXTRA_ARGS) { $argv += ($env:AUREA_QEMU_EXTRA_ARGS -split '\s+' | Where-Object { $_ }) }

Write-Host ''
Write-Host "QEMU:     $qemu"
Write-Host "Image:    $img"
Write-Host "Memory:   $MemoryMB MiB   CPUs: $Cpus   Accelerator: $Accel"
Write-Host "Serial:   $serialLog"
Write-Host ''

if ($DryRun) {
    $quoted = $argv | ForEach-Object { if ($_ -match '\s') { '"' + $_ + '"' } else { $_ } }
    Write-Host ('"' + $qemu + '" ' + ($quoted -join ' '))
    exit 0
}

Write-Host 'Starting Aurea OS. Log in with user Aura and password root.'
Write-Host 'Close the QEMU window or run "power shutdown" in Aura to stop the VM.'
$proc = Start-Process -FilePath $qemu -ArgumentList ($argv | ForEach-Object { if ($_ -match '[\s"]') { '"' + $_.Replace('"', '\"') + '"' } else { $_ } }) -NoNewWindow -PassThru -Wait
if ($proc.ExitCode -ne 0) {
    Fail "QEMU exited with code $($proc.ExitCode)." ("If the window closed straight away, try -Accel tcg. WHPX needs the Windows feature " +
        "'Windows Hypervisor Platform'.`nThe serial log may contain details: $serialLog")
}
