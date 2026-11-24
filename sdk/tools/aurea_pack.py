#!/usr/bin/env python3
"""AureaOS AEX / RUN packager (host tool).

Formats:
  aex / run1 — legacy AEX V1 envelope (major=1)
  aex2       — AEX V2 executable (major=2, same layout, independent of RUN2)
  run2       — RUN2 bundle (magic RUN2) with nested AEX image + optional HMAC integrity
"""

from __future__ import annotations

import argparse
import hashlib
import hmac
import struct
import sys
from pathlib import Path

AEX_MAGIC_AURE = 0x41555245
AUREA_RUN2_MAGIC = 0x324E5552
FORMAT_MAJOR_V1 = 1
FORMAT_MAJOR_AEX2 = 2
FORMAT_MINOR = 0
ARCH_X86_64 = 1
IMAGE_ELF64 = 1
ALIGN = 16

ENTRY_MANIFEST = 1
ENTRY_AEX = 2
ENTRY_RESOURCE = 3
ENTRY_INTEGRITY = 4

AEX_FLAG_SIGNED = 0x1
RUN2_FLAG_SIGNED = 0x1
RUN2_FLAG_REQUIRE_SIG = 0x2
SIG_HMAC_SHA256 = 1
# Fixed development key shared with the kernel. The HMAC only detects damaged
# or altered files; anyone with this tool can produce a valid tag, so it says
# nothing about who built a package.
TRUST_KEY = b"AUREA-P08-TEST-ROOT-KEY-v1"


def align_up(value: int, alignment: int) -> int:
    return (value + alignment - 1) // alignment * alignment


def read_manifest(path: Path) -> bytes:
    text = path.read_bytes()
    for line in text.decode("utf-8", errors="replace").splitlines():
        stripped = line.strip()
        if stripped.startswith("#"):
            continue
        if stripped.startswith("kernel_access") and "true" in stripped.split("=", 1)[-1]:
            raise SystemExit("kernel_access=true is not allowed")
        if stripped.startswith("device_access") and "true" in stripped.split("=", 1)[-1]:
            raise SystemExit("device_access=true is not allowed for unsigned packs")
    return text


def validate_elf(data: bytes) -> int:
    if len(data) < 64 or data[:4] != b"\x7fELF":
        raise SystemExit("Invalid ELF image")
    if data[4] != 2 or data[5] != 1:
        raise SystemExit("ELF must be 64-bit little endian")
    if data[18] != 62:
        raise SystemExit("ELF machine must be x86_64")
    entry, = struct.unpack_from("<Q", data, 24)
    if entry >= (1 << 47):
        raise SystemExit("ELF entry must be canonical user address")
    return entry


def checksum_v1(data: bytes) -> int:
    value = 0
    for i, b in enumerate(data):
        if 124 <= i < 128:
            b = 0
        value = (value * 131 + b) & 0xFFFFFFFF
    return value


def checksum_v2(data: bytes) -> int:
    value = 0
    for i, b in enumerate(data):
        if 36 <= i < 40:
            b = 0
        value = (value * 131 + b) & 0xFFFFFFFF
    return value


def build_aex(manifest: bytes, elf: bytes, entry_override: int | None, *, major: int, signed: bool) -> bytes:
    header_size = 128
    manifest_off = align_up(header_size, ALIGN)
    image_off = align_up(manifest_off + len(manifest), ALIGN)
    sig_off = 0
    sig_size = 0
    payload_end = image_off + len(elf)
    if signed:
        sig_off = align_up(payload_end, ALIGN)
        sig_size = 36
        total = align_up(sig_off + sig_size, ALIGN)
    else:
        total = align_up(payload_end, ALIGN)
    entry = entry_override if entry_override is not None else validate_elf(elf)
    flags = AEX_FLAG_SIGNED if signed else 0

    header = bytearray(header_size)
    struct.pack_into("<I", header, 0, AEX_MAGIC_AURE)
    struct.pack_into("<HH", header, 4, major, FORMAT_MINOR)
    struct.pack_into("<I", header, 8, header_size)
    struct.pack_into("<Q", header, 12, total)
    struct.pack_into("<IIII", header, 20, ARCH_X86_64, IMAGE_ELF64, flags, 0)
    struct.pack_into("<QQ", header, 36, manifest_off, len(manifest))
    struct.pack_into("<QQ", header, 52, image_off, len(elf))
    struct.pack_into("<QQQQ", header, 68, 0, 0, 0, 0)
    struct.pack_into("<QQ", header, 100, sig_off, sig_size)
    struct.pack_into("<Q", header, 116, entry)
    struct.pack_into("<I", header, 124, 0)

    blob = bytearray(total)
    blob[0:header_size] = header
    blob[manifest_off:manifest_off + len(manifest)] = manifest
    blob[image_off:image_off + len(elf)] = elf
    if signed:
        digest = hashlib.sha256(bytes(blob[manifest_off:image_off + len(elf)])).digest()
        mac = hmac.new(TRUST_KEY, digest, hashlib.sha256).digest()
        struct.pack_into("<I", blob, sig_off, SIG_HMAC_SHA256)
        blob[sig_off + 4:sig_off + 36] = mac
    struct.pack_into("<I", blob, 124, checksum_v1(bytes(blob)))
    return bytes(blob)


def build_aex_v1(manifest: bytes, elf: bytes, entry_override: int | None) -> bytes:
    return build_aex(manifest, elf, entry_override, major=FORMAT_MAJOR_V1, signed=False)


def build_aex_v2(manifest: bytes, elf: bytes, entry_override: int | None, *, signed: bool = False) -> bytes:
    return build_aex(manifest, elf, entry_override, major=FORMAT_MAJOR_AEX2, signed=signed)


def build_run_v2(
    manifest: bytes,
    aex_image: bytes,
    entry_image_name: str,
    *,
    signed: bool = False,
    require_sig: bool = False,
    resources: list[tuple[str, bytes]] | None = None,
) -> bytes:
    """RUN2 directory order: manifest, one AEX image, optional resources, integrity.

    HMAC covers every non-integrity payload in that order. Packages with no
    resources keep the previous manifest||aex digest.
    """
    header_size = 40
    resources = list(resources or [])
    if len(resources) + (3 if signed else 2) > 64:
        raise SystemExit("too many RUN2 entries")
    entry_count = 2 + len(resources) + (1 if signed else 0)
    entries_off = align_up(header_size, 8)
    name_blobs = [b"manifest.toml", entry_image_name.encode("utf-8")]
    payloads: list[tuple[int, bytes]] = [
        (ENTRY_MANIFEST, manifest),
        (ENTRY_AEX, aex_image),
    ]
    for name, data in resources:
        raw_name = name.encode("utf-8")
        if not raw_name or len(raw_name) > 192:
            raise SystemExit(f"bad resource name {name!r}")
        name_blobs.append(raw_name)
        payloads.append((ENTRY_RESOURCE, data))
    if signed:
        name_blobs.append(b"integrity.hmac")
    names = b"".join(name_blobs)
    names_off = align_up(entries_off + entry_count * 40, 8)
    cursor = align_up(names_off + len(names), 8)
    data_off: list[int] = []
    for _typ, payload in payloads:
        data_off.append(cursor)
        cursor = align_up(cursor + len(payload), 8)
    integ_off = cursor if signed else 0
    total = align_up((integ_off + 36) if signed else cursor, 8)

    flags = 0
    if signed:
        flags |= RUN2_FLAG_SIGNED
    if require_sig:
        flags |= RUN2_FLAG_REQUIRE_SIG

    blob = bytearray(total)
    struct.pack_into("<IHH", blob, 0, AUREA_RUN2_MAGIC, 2, FORMAT_MINOR)
    struct.pack_into("<I", blob, 8, header_size)
    struct.pack_into("<Q", blob, 12, total)
    struct.pack_into("<II", blob, 20, entry_count, flags)
    struct.pack_into("<Q", blob, 28, entries_off)
    struct.pack_into("<II", blob, 36, 0, 0)

    def pack_entry(base: int, noff: int, nsz: int, typ: int, doff: int, dsz: int) -> None:
        struct.pack_into("<Q", blob, base + 0, noff)
        struct.pack_into("<I", blob, base + 8, nsz)
        struct.pack_into("<I", blob, base + 12, typ)
        struct.pack_into("<Q", blob, base + 16, doff)
        struct.pack_into("<Q", blob, base + 24, dsz)
        struct.pack_into("<II", blob, base + 32, 0, 0)

    name_at = names_off
    for index, (typ, payload) in enumerate(payloads):
        raw_name = name_blobs[index]
        pack_entry(entries_off + index * 40, name_at, len(raw_name), typ, data_off[index], len(payload))
        blob[name_at:name_at + len(raw_name)] = raw_name
        blob[data_off[index]:data_off[index] + len(payload)] = payload
        name_at += len(raw_name)
    if signed:
        raw_name = name_blobs[-1]
        pack_entry(entries_off + len(payloads) * 40, name_at, len(raw_name), ENTRY_INTEGRITY, integ_off, 36)
        blob[name_at:name_at + len(raw_name)] = raw_name
        body = hashlib.sha256(b"".join(payload for _typ, payload in payloads)).digest()
        mac = hmac.new(TRUST_KEY, body, hashlib.sha256).digest()
        struct.pack_into("<I", blob, integ_off, SIG_HMAC_SHA256)
        blob[integ_off + 4:integ_off + 36] = mac
    struct.pack_into("<I", blob, 36, checksum_v2(bytes(blob)))
    return bytes(blob)


def inspect_blob(data: bytes) -> None:
    if len(data) < 4:
        raise SystemExit("truncated")
    magic, = struct.unpack_from("<I", data, 0)
    if magic == AUREA_RUN2_MAGIC:
        major, minor = struct.unpack_from("<HH", data, 4)
        total, = struct.unpack_from("<Q", data, 12)
        count, flags = struct.unpack_from("<II", data, 20)
        entries_off, = struct.unpack_from("<Q", data, 28)
        csum_file, = struct.unpack_from("<I", data, 36)
        csum_calc = checksum_v2(data)
        print("kind: RUN_V2")
        print(f"format: {major}.{minor}")
        print(f"total_size: {total}")
        print(f"entry_count: {count}")
        print(f"flags: 0x{flags:x}")
        print(f"entries_offset: {entries_off}")
        print(f"checksum: {'ok' if csum_file == csum_calc else 'bad'}")
        print(f"signed: {'yes' if flags & RUN2_FLAG_SIGNED else 'no'}")
        return
    if magic != AEX_MAGIC_AURE:
        raise SystemExit("Not an AEX / RUN2 container")
    major, minor = struct.unpack_from("<HH", data, 4)
    total, = struct.unpack_from("<Q", data, 12)
    flags, = struct.unpack_from("<I", data, 28)
    manifest_off, manifest_size = struct.unpack_from("<QQ", data, 36)
    image_off, image_size = struct.unpack_from("<QQ", data, 52)
    sig_off, sig_size = struct.unpack_from("<QQ", data, 100)
    entry, = struct.unpack_from("<Q", data, 116)
    csum_file, = struct.unpack_from("<I", data, 124)
    csum_calc = checksum_v1(data)
    kind = "AEX_V2" if major == FORMAT_MAJOR_AEX2 else "AEX_V1_OR_RUN_V1_LEGACY"
    print(f"kind: {kind}")
    print(f"format: {major}.{minor}")
    print(f"total_size: {total}")
    print(f"flags: 0x{flags:x}")
    print(f"manifest_size: {manifest_size}")
    print(f"image_size: {image_size}")
    print(f"signature_size: {sig_size}")
    print(f"entry_point: 0x{entry:x}")
    print(f"checksum: {'ok' if csum_file == csum_calc else 'bad'}")


def inject_deployment(manifest: bytes, mode: str | None, installer: str | None) -> bytes:
    """Ensure [deployment] mode/installer keys exist for the three SDK variants."""
    if not mode and not installer:
        return manifest
    text = manifest.decode("utf-8", errors="replace")
    lines = text.splitlines()
    # Drop existing deployment section keys we own.
    out: list[str] = []
    in_dep = False
    for line in lines:
        stripped = line.strip()
        if stripped.startswith("[") and stripped.endswith("]"):
            in_dep = stripped == "[deployment]"
            if in_dep:
                continue
            out.append(line)
            continue
        if in_dep:
            key = stripped.split("=", 1)[0].strip() if "=" in stripped else ""
            if key in ("mode", "installer", "interactive", "allow_unattended"):
                continue
            if stripped == "" or stripped.startswith("#"):
                continue
            # leftover unknown keys stay under a re-emitted section later
            continue
        out.append(line)
    while out and out[-1].strip() == "":
        out.pop()
    out.append("")
    out.append("[deployment]")
    if mode:
        out.append(f'mode = "{mode}"')
    if installer:
        out.append(f'installer = "{installer}"')
    if mode == "install" and installer == "cli":
        out.append("interactive = true")
        out.append("allow_unattended = true")
    out.append("")
    return ("\n".join(out) + "\n").encode("utf-8")


def validate_deployment(mode: str | None, installer: str | None) -> None:
    if mode is None and installer is None:
        return
    if mode not in (None, "standalone", "install"):
        raise SystemExit(f"invalid --deployment {mode!r}")
    if installer not in (None, "none", "cli", "gui"):
        raise SystemExit(f"invalid --installer {installer!r}")
    if mode == "standalone" and installer not in (None, "none"):
        raise SystemExit("standalone deployment requires --installer none")
    if mode == "install" and installer in (None, "none"):
        raise SystemExit("install deployment requires --installer cli|gui")


def main() -> int:
    parser = argparse.ArgumentParser(description="AureaOS AEX/RUN packager")
    parser.add_argument("--manifest", type=Path, help="Manifest path")
    parser.add_argument("--image", type=Path, help="ELF64 or nested .aex (for run2)")
    parser.add_argument("--output", type=Path, help="Output path")
    parser.add_argument("--entry", type=lambda x: int(x, 0), help="Optional ELF entry override")
    parser.add_argument(
        "--format",
        choices=("aex", "aex2", "run1", "run2"),
        default="aex",
        help="aex/run1=V1 envelope; aex2=AEX V2; run2=RUN2 bundle",
    )
    parser.add_argument("--entry-image", default="images/x86_64/app.aex",
                        help="Nested AEX entry name for run2")
    parser.add_argument("--sign", action="store_true", help="Attach HMAC-SHA256 integrity")
    parser.add_argument("--require-signature", action="store_true",
                        help="Mark RUN2 as signature-required")
    parser.add_argument(
        "--deployment",
        choices=("standalone", "install"),
        help="SDK deployment mode written into [deployment] manifest section",
    )
    parser.add_argument(
        "--installer",
        choices=("none", "cli", "gui"),
        help="Installer frontend: none (portable), cli, or gui",
    )
    parser.add_argument(
        "--resource",
        action="append",
        default=[],
        help="RUN2 resource as name=path (for example icon.bmp=icons/icon-32.bmp)",
    )
    parser.add_argument("--inspect", type=Path, help="Inspect container")
    parser.add_argument("--verify", type=Path, help="Verify container checksum")
    args = parser.parse_args()

    if args.inspect:
        inspect_blob(args.inspect.read_bytes())
        return 0

    if args.verify:
        data = args.verify.read_bytes()
        inspect_blob(data)
        magic, = struct.unpack_from("<I", data, 0)
        if magic == AUREA_RUN2_MAGIC:
            csum_file, = struct.unpack_from("<I", data, 36)
            if csum_file != checksum_v2(data):
                raise SystemExit("Checksum verification failed")
        else:
            csum_file, = struct.unpack_from("<I", data, 124)
            if csum_file != checksum_v1(data):
                raise SystemExit("Checksum verification failed")
        print("verify: ok")
        return 0

    if not args.manifest or not args.image or not args.output:
        parser.error("--manifest, --image, and --output are required")

    validate_deployment(args.deployment, args.installer)
    manifest = read_manifest(args.manifest)
    if args.deployment or args.installer:
        mode = args.deployment
        installer = args.installer
        if mode == "standalone" and not installer:
            installer = "none"
        if mode == "install" and not installer:
            raise SystemExit("--deployment install requires --installer cli|gui")
        if installer == "none" and not mode:
            mode = "standalone"
        if installer in ("cli", "gui") and not mode:
            mode = "install"
        validate_deployment(mode, installer)
        manifest = inject_deployment(manifest, mode, installer)

    image = args.image.read_bytes()
    if args.format == "run2":
        if image[:4] == b"\x7fELF":
            nested = build_aex_v2(manifest, image, args.entry, signed=args.sign)
        else:
            nested = image
        resources: list[tuple[str, bytes]] = []
        for item in args.resource:
            if "=" not in item:
                raise SystemExit(f"--resource expects name=path, got {item!r}")
            res_name, res_path = item.split("=", 1)
            resources.append((res_name, Path(res_path).read_bytes()))
        out = build_run_v2(
            manifest,
            nested,
            args.entry_image,
            signed=args.sign,
            require_sig=args.require_signature,
            resources=resources,
        )
    elif args.format == "aex2":
        out = build_aex_v2(manifest, image, args.entry, signed=args.sign)
    else:
        out = build_aex_v1(manifest, image, args.entry)
    args.output.write_bytes(out)
    dep = ""
    if args.deployment or args.installer:
        dep = f" deployment={args.deployment or mode} installer={args.installer or installer}"
    print(f"Wrote {args.output} ({len(out)} bytes) format={args.format}{dep}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
