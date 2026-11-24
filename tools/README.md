# Tools

| File | Purpose |
|---|---|
| `Aurea Live VM.cmd` | Starts the Aurea OS image in QEMU on Windows. See [docs/INSTALL_QEMU.md](../docs/INSTALL_QEMU.md). |
| `Aurea Live boot.cmd` | Writes the image to a USB drive. Erases the selected drive. See [docs/HARDWARE_BOOT.md](../docs/HARDWARE_BOOT.md). |
| `aurea-live-vm.ps1`, `aurea-live-boot.ps1` | The PowerShell scripts behind the two `.cmd` files. Keep them in the same folder. |

Both tools look for `AureaOS-*.img` in their own folder and its parent, and verify it against `SHA256SUMS.txt` when that file is present.
