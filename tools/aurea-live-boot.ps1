<#
.SYNOPSIS
    Writes the Aurea OS release image to a USB drive.

.DESCRIPTION
    Lists USB-attached disks, lets you pick one, and writes the Aurea OS disk
    image to it after an explicit confirmation. Everything on the selected
    drive is erased. The written data is read back and compared with the
    image afterwards.

    Only disks on the USB bus are offered. System, boot, read-only, offline
    and too-small disks are refused, as are disks that hold a Windows page
    file or the Windows folder.

.PARAMETER Image
    Path to the Aurea OS .img file. By default the script looks for
    AureaOS-*.img next to itself and in its parent folder.

.PARAMETER ListOnly
    Show the eligible drives and exit without writing anything.
#>
[CmdletBinding()]
param(
    [string]$Image,
    [switch]$ListOnly
)

$ErrorActionPreference = 'Stop'
$MaxDiskBytes = 2TB

function Fail([string]$Message, [string]$Hint) {
    Write-Host ''
    Write-Host "Error: $Message" -ForegroundColor Red
    if ($Hint) { Write-Host $Hint }
    exit 1
}

function Format-Size([uint64]$Bytes) {
    if ($Bytes -ge 1GB) { return ('{0:N1} GB' -f ($Bytes / 1GB)) }
    return ('{0:N0} MB' -f ($Bytes / 1MB))
}

function Find-Image([string]$Requested) {
    if ($Requested) {
        if (-not (Test-Path -LiteralPath $Requested -PathType Leaf)) { Fail "Image not found: $Requested" }
        return (Resolve-Path -LiteralPath $Requested).Path
    }
    $dirs = @($PSScriptRoot, (Split-Path -Parent $PSScriptRoot), (Get-Location).Path) | Select-Object -Unique
    $found = @(foreach ($d in $dirs) { Get-ChildItem -LiteralPath $d -Filter 'AureaOS-*.img' -File -ErrorAction SilentlyContinue })
    $found = @($found | Sort-Object FullName -Unique)
    if ($found.Count -eq 0) {
        Fail 'No Aurea OS image found.' 'Place AureaOS-*.img next to this script or pass -Image <path>.'
    }
    if ($found.Count -gt 1) {
        $list = ($found | ForEach-Object { '  ' + $_.FullName }) -join "`n"
        Fail 'More than one Aurea OS image was found.' "Choose one with -Image <path>:`n$list"
    }
    return $found[0].FullName
}

function Test-Image([string]$Path) {
    $len = (Get-Item -LiteralPath $Path).Length
    if ($len -lt 1MB -or ($len % 512) -ne 0) { Fail "Not a raw disk image: $Path" }
    $fs = [IO.File]::OpenRead($Path)
    try { $buf = New-Object byte[] 1024; [void]$fs.Read($buf, 0, 1024) } finally { $fs.Dispose() }
    if (-not ($buf[510] -eq 0x55 -and $buf[511] -eq 0xAA -and [Text.Encoding]::ASCII.GetString($buf, 512, 8) -eq 'EFI PART')) {
        Fail "No GPT partition table found in $Path" 'Use the AureaOS-*.img file from the release. ISO files cannot be written with this tool.'
    }
    return $len
}

function Get-ImageHash([string]$Path) {
    Write-Host 'Calculating the image checksum ...'
    $actual = (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
    $sums = Join-Path (Split-Path -Parent $Path) 'SHA256SUMS.txt'
    $name = Split-Path -Leaf $Path
    $expected = $null
    if (Test-Path -LiteralPath $sums) {
        foreach ($line in Get-Content -LiteralPath $sums) {
            if ($line -match '^\s*([0-9a-fA-F]{64})\s+\*?(.+?)\s*$' -and $Matches[2] -eq $name) { $expected = $Matches[1].ToLowerInvariant() }
        }
    }
    if ($expected) {
        if ($expected -ne $actual) { Fail "Checksum mismatch for $name." "Expected $expected`nActual   $actual`nDownload the image again." }
        Write-Host 'Checksum matches SHA256SUMS.txt.'
    } else {
        Write-Host "SHA256SUMS.txt with an entry for $name was not found next to the image." -ForegroundColor Yellow
        Write-Host "Image SHA-256: $actual"
        Write-Host 'Compare it with the value on the release page before continuing.'
        $answer = Read-Host 'Type YES to continue without the checksum file'
        if ($answer -cne 'YES') { Fail 'Cancelled.' }
    }
    return $actual
}

function Get-WindowsDiskNumbers {
    $numbers = New-Object 'System.Collections.Generic.HashSet[int]'
    $letters = @($env:SystemDrive.TrimEnd(':'))
    try {
        Get-CimInstance Win32_PageFileUsage -ErrorAction Stop | ForEach-Object { $letters += $_.Name.Substring(0, 1) }
    } catch { }
    foreach ($l in ($letters | Select-Object -Unique)) {
        $p = Get-Partition -DriveLetter $l -ErrorAction SilentlyContinue
        if ($p) { [void]$numbers.Add([int]$p.DiskNumber) }
    }
    return $numbers
}

function Get-Candidates([uint64]$ImageBytes) {
    $windowsDisks = Get-WindowsDiskNumbers
    $result = @()
    foreach ($d in (Get-Disk | Sort-Object Number)) {
        $reason = $null
        if ($d.BusType -ne 'USB') { $reason = "bus type $($d.BusType)" }
        elseif ($d.IsSystem -or $d.IsBoot) { $reason = 'system or boot disk' }
        elseif ($windowsDisks.Contains([int]$d.Number)) { $reason = 'holds Windows or a page file' }
        elseif ($d.IsReadOnly) { $reason = 'read-only' }
        elseif ($d.OperationalStatus -ne 'Online') { $reason = "status $($d.OperationalStatus)" }
        elseif ($d.Size -lt $ImageBytes) { $reason = 'smaller than the image' }
        elseif ($d.Size -gt $MaxDiskBytes) { $reason = 'larger than 2 TB' }
        $letters = @(Get-Partition -DiskNumber $d.Number -ErrorAction SilentlyContinue |
            Where-Object { $_.DriveLetter } | ForEach-Object { "$($_.DriveLetter):" }) -join ' '
        $result += [pscustomobject]@{
            Number = [int]$d.Number; Name = $d.FriendlyName; Serial = ($d.SerialNumber -as [string]).Trim()
            Size = [uint64]$d.Size; Style = $d.PartitionStyle; Letters = $letters; Reason = $reason
        }
    }
    return $result
}

function Show-Disks($Disks) {
    Write-Host ''
    Write-Host 'USB drives that can be written:'
    $ok = @($Disks | Where-Object { -not $_.Reason })
    if ($ok.Count -eq 0) { Write-Host '  (none)' }
    foreach ($d in $ok) {
        Write-Host ('  Disk {0,-3} {1,-34} {2,9}  {3,-4} {4}' -f $d.Number, $d.Name, (Format-Size $d.Size), $d.Style, $d.Letters)
        if ($d.Serial) { Write-Host ('            serial {0}' -f $d.Serial) }
    }
    $skipped = @($Disks | Where-Object { $_.Reason -and $_.Reason -notlike 'bus type*' })
    if ($skipped.Count -gt 0) {
        Write-Host ''
        Write-Host 'USB drives that are not offered:'
        foreach ($d in $skipped) {
            Write-Host ('  Disk {0,-3} {1,-34} {2,9}  {3}' -f $d.Number, $d.Name, (Format-Size $d.Size), $d.Reason)
        }
    }
    return $ok
}

function Write-ImageToDisk([string]$Path, [int]$DiskNumber, [uint64]$Length) {
    $device = "\\.\PhysicalDrive$DiskNumber"
    $chunk = 4MB
    $src = [IO.File]::OpenRead($Path)
    $dst = New-Object IO.FileStream($device, [IO.FileMode]::Open, [IO.FileAccess]::ReadWrite, [IO.FileShare]::ReadWrite, 4096, [IO.FileOptions]::WriteThrough)
    try {
        $buf = New-Object byte[] $chunk
        [uint64]$done = 0
        while ($done -lt $Length) {
            # Raw disk writes must be whole sectors. The image length is a multiple
            # of 512 (checked in Test-Image), so fill each chunk completely.
            $want = [int][Math]::Min([uint64]$chunk, $Length - $done)
            $n = 0
            while ($n -lt $want) {
                $r = $src.Read($buf, $n, $want - $n)
                if ($r -le 0) { throw 'Unexpected end of image file.' }
                $n += $r
            }
            $dst.Write($buf, 0, $n)
            $done += [uint64]$n
            Write-Progress -Activity "Writing to disk $DiskNumber" -Status (Format-Size $done) -PercentComplete ([int](100 * $done / $Length))
        }
        $dst.Flush($true)
    } finally {
        Write-Progress -Activity "Writing to disk $DiskNumber" -Completed
        $dst.Dispose(); $src.Dispose()
    }
}

function Get-DiskPrefixHash([int]$DiskNumber, [uint64]$Length) {
    $device = "\\.\PhysicalDrive$DiskNumber"
    $sha = [Security.Cryptography.SHA256]::Create()
    $fs = New-Object IO.FileStream($device, [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::ReadWrite, 4096, [IO.FileOptions]::None)
    try {
        $buf = New-Object byte[] 4MB
        [uint64]$done = 0
        while ($done -lt $Length) {
            $want = [int][Math]::Min([uint64]$buf.Length, $Length - $done)
            $n = $fs.Read($buf, 0, $want)
            if ($n -le 0) { throw 'Read-back ended early.' }
            [void]$sha.TransformBlock($buf, 0, $n, $null, 0)
            $done += [uint64]$n
            Write-Progress -Activity "Verifying disk $DiskNumber" -Status (Format-Size $done) -PercentComplete ([int](100 * $done / $Length))
        }
        [void]$sha.TransformFinalBlock($buf, 0, 0)
    } finally {
        Write-Progress -Activity "Verifying disk $DiskNumber" -Completed
        $fs.Dispose()
    }
    return (($sha.Hash | ForEach-Object { $_.ToString('x2') }) -join '')
}

$principal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
if (-not $ListOnly -and -not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Fail 'Administrator rights are required to write a USB drive.' 'Run "Aurea Live boot.cmd" again and accept the Windows elevation prompt.'
}

$img = Find-Image $Image
$imgBytes = [uint64](Test-Image $img)
Write-Host "Image: $img ($(Format-Size $imgBytes))"

$disks = Get-Candidates $imgBytes
$eligible = @(Show-Disks $disks)
if ($ListOnly) { exit 0 }
if ($eligible.Count -eq 0) {
    Fail 'No suitable USB drive found.' 'Connect a USB drive that you can erase, then run this tool again.'
}

$imgHash = Get-ImageHash $img

Write-Host ''
$sel = Read-Host 'Enter the disk number to write (or press Enter to cancel)'
if ([string]::IsNullOrWhiteSpace($sel)) { Fail 'Cancelled.' }
$num = 0
if (-not [int]::TryParse($sel.Trim(), [ref]$num)) { Fail "Not a disk number: $sel" }
$target = $eligible | Where-Object { $_.Number -eq $num }
if (-not $target) { Fail "Disk $num is not one of the drives listed above." }

Write-Host ''
Write-Host 'All data on this drive will be erased:' -ForegroundColor Yellow
Write-Host ('  Disk {0}: {1}, {2}{3}' -f $target.Number, $target.Name, (Format-Size $target.Size),
    $(if ($target.Serial) { ", serial $($target.Serial)" } else { '' }))
if ($target.Letters) { Write-Host "  Volumes: $($target.Letters)" }
$confirm = Read-Host "Type ERASE $($target.Number) to continue"
if ($confirm -cne "ERASE $($target.Number)") { Fail 'Cancelled. Nothing was written.' }

# Re-read the disk right before writing so a swapped or re-enumerated drive is caught.
$now = Get-Disk -Number $target.Number -ErrorAction SilentlyContinue
if (-not $now -or $now.BusType -ne 'USB' -or $now.IsSystem -or $now.IsBoot -or [uint64]$now.Size -ne $target.Size -or
    ($now.SerialNumber -as [string]).Trim() -ne $target.Serial) {
    Fail 'The selected drive changed since it was listed. Nothing was written.'
}

Write-Host ''
Write-Host 'Removing the existing partitions ...'
if ($now.PartitionStyle -ne 'RAW') {
    Clear-Disk -Number $target.Number -RemoveData -RemoveOEM -Confirm:$false
}
Write-Host 'Writing the image ...'
try {
    Write-ImageToDisk -Path $img -DiskNumber $target.Number -Length $imgBytes
} catch {
    Fail "Writing failed: $($_.Exception.Message)" 'The drive is now incomplete. Run the tool again or reformat the drive in Windows.'
}
Write-Host 'Verifying ...'
$diskHash = Get-DiskPrefixHash -DiskNumber $target.Number -Length $imgBytes
Update-Disk -Number $target.Number -ErrorAction SilentlyContinue
if ($diskHash -ne $imgHash) {
    Fail 'Verification failed: the data on the drive does not match the image.' "Image $imgHash`nDrive $diskHash"
}
Write-Host ''
Write-Host "Done. Disk $($target.Number) now holds the Aurea OS image and the read-back matches." -ForegroundColor Green
Write-Host 'Boot it through your firmware boot menu in UEFI mode. See docs/HARDWARE_BOOT.md.'
Write-Host 'To use the drive normally again, delete its partitions in Disk Management and create a new volume.'
