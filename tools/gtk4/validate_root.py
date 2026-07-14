#!/usr/bin/env python3

import argparse
import hashlib
import json
import pathlib
import re
import struct
import sys


DEFAULT_CONTRACT = pathlib.Path(__file__).with_name("dependency-contract.json")
PE_MACHINES = {
    0x014C: "x86",
    0x8664: "x64",
    0xAA64: "arm64",
}
GTK3_MARKERS = (
    "bin/gtk-3-0.dll",
    "bin/libgtk-3-0.dll",
    "include/gtk-3.0/gtk/gtk.h",
    "lib/gtk-3.lib",
)


class ValidationError(Exception):
    pass


def _strict_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValidationError(f"Duplicate contract field: {key}")
        result[key] = value
    return result


def load_contract(path):
    try:
        with path.open("r", encoding="utf-8") as handle:
            contract = json.load(handle, object_pairs_hook=_strict_object)
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ValidationError(f"Unable to read GTK4 contract {path}: {exc}") from exc

    required_keys = {
        "schema_version",
        "platform",
        "architecture",
        "gtk_version",
        "glib_version",
        "source",
        "required_files",
    }
    if set(contract) != required_keys:
        missing = sorted(required_keys.difference(contract))
        unknown = sorted(set(contract).difference(required_keys))
        raise ValidationError(
            f"Invalid contract fields; missing={missing}, unknown={unknown}"
        )
    if contract["schema_version"] != 1:
        raise ValidationError("Unsupported GTK4 contract schema version")
    if contract["platform"] != "windows" or contract["architecture"] != "x64":
        raise ValidationError("The initial GTK4 contract must target Windows x64")

    source_keys = {"file_name", "url", "size_bytes", "sha256"}
    source = contract["source"]
    if not isinstance(source, dict) or set(source) != source_keys:
        raise ValidationError("Invalid GTK4 source contract")
    if not isinstance(source["size_bytes"], int) or source["size_bytes"] <= 0:
        raise ValidationError("GTK4 archive size must be a positive integer")
    if not re.fullmatch(r"[0-9a-f]{64}", source["sha256"]):
        raise ValidationError("GTK4 archive SHA-256 must be 64 lowercase hex digits")

    required_files = contract["required_files"]
    if not isinstance(required_files, list) or not required_files:
        raise ValidationError("GTK4 required_files must be a non-empty array")
    if len(required_files) != len(set(required_files)):
        raise ValidationError("GTK4 required_files contains duplicates")
    for relative in required_files:
        if not isinstance(relative, str) or not relative:
            raise ValidationError("GTK4 required_files entries must be non-empty strings")
        pure = pathlib.PurePosixPath(relative)
        if pure.is_absolute() or ".." in pure.parts or "\\" in relative:
            raise ValidationError(f"Unsafe GTK4 required path: {relative}")

    return contract


def _sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def validate_archive(path, contract):
    archive = path.resolve(strict=True)
    if not archive.is_file():
        raise ValidationError(f"GTK4 archive is not a regular file: {archive}")

    source = contract["source"]
    actual_size = archive.stat().st_size
    if actual_size != source["size_bytes"]:
        raise ValidationError(
            f"GTK4 archive size mismatch: expected {source['size_bytes']}, got {actual_size}"
        )
    actual_hash = _sha256(archive)
    if actual_hash != source["sha256"]:
        raise ValidationError(
            f"GTK4 archive SHA-256 mismatch: expected {source['sha256']}, got {actual_hash}"
        )

    return {
        "archive": str(archive),
        "archive_bytes": actual_size,
        "archive_sha256": actual_hash,
    }


def _required_file(root, relative):
    candidate = root.joinpath(*pathlib.PurePosixPath(relative).parts)
    try:
        resolved = candidate.resolve(strict=True)
    except OSError as exc:
        raise ValidationError(f"Missing GTK4 dependency file: {relative}") from exc
    try:
        resolved.relative_to(root)
    except ValueError as exc:
        raise ValidationError(f"GTK4 dependency path escapes its root: {relative}") from exc
    if not resolved.is_file():
        raise ValidationError(f"GTK4 dependency path is not a file: {relative}")
    return resolved


def _define_int(path, name):
    pattern = re.compile(rf"^\s*#\s*define\s+{re.escape(name)}\s+\(?([0-9]+)\)?\s*$")
    try:
        for line in path.read_text(encoding="utf-8").splitlines():
            match = pattern.match(line)
            if match:
                return int(match.group(1))
    except (OSError, UnicodeDecodeError) as exc:
        raise ValidationError(f"Unable to read version header {path}: {exc}") from exc
    raise ValidationError(f"Missing integer definition {name} in {path}")


def _header_version(path, prefix):
    return ".".join(
        str(_define_int(path, f"{prefix}_{part}_VERSION"))
        for part in ("MAJOR", "MINOR", "MICRO")
    )


def _pkg_config_value(path, key, separator):
    prefix = f"{key}{separator}"
    try:
        for line in path.read_text(encoding="utf-8").splitlines():
            if line.startswith(prefix):
                return line[len(prefix) :].strip()
    except (OSError, UnicodeDecodeError) as exc:
        raise ValidationError(f"Unable to read pkg-config file {path}: {exc}") from exc
    raise ValidationError(f"Missing {key}{separator} in {path}")


def _pe_architecture(path):
    try:
        with path.open("rb") as handle:
            dos_header = handle.read(64)
            if len(dos_header) != 64 or dos_header[:2] != b"MZ":
                raise ValidationError(f"Invalid PE DOS header: {path}")
            pe_offset = struct.unpack_from("<I", dos_header, 0x3C)[0]
            handle.seek(pe_offset)
            pe_header = handle.read(6)
    except OSError as exc:
        raise ValidationError(f"Unable to inspect PE architecture for {path}: {exc}") from exc

    if len(pe_header) != 6 or pe_header[:4] != b"PE\0\0":
        raise ValidationError(f"Invalid PE header: {path}")
    machine = struct.unpack_from("<H", pe_header, 4)[0]
    return PE_MACHINES.get(machine, f"unknown-0x{machine:04x}")


def validate_root(path, contract):
    try:
        root = path.resolve(strict=True)
    except OSError as exc:
        raise ValidationError(f"GTK4 root does not exist: {path}") from exc
    if not root.is_dir():
        raise ValidationError(f"GTK4 root is not a directory: {root}")

    required = {
        relative: _required_file(root, relative)
        for relative in contract["required_files"]
    }
    mixed = [relative for relative in GTK3_MARKERS if root.joinpath(*relative.split("/")).exists()]
    if mixed:
        raise ValidationError(f"GTK4 root contains GTK3 build markers: {', '.join(mixed)}")

    gtk_version = _header_version(
        required["include/gtk-4.0/gtk/gtkversion.h"], "GTK"
    )
    glib_config = required["lib/glib-2.0/include/glibconfig.h"]
    glib_version = _header_version(glib_config, "GLIB")
    pointer_size = _define_int(glib_config, "GLIB_SIZEOF_VOID_P")
    gtk_architecture = _pe_architecture(required["bin/gtk-4-1.dll"])
    glib_architecture = _pe_architecture(required["bin/glib-2.0-0.dll"])
    if gtk_architecture != glib_architecture:
        raise ValidationError(
            "GTK and GLib runtime DLL architectures do not match: "
            f"gtk={gtk_architecture}, glib={glib_architecture}"
        )
    architecture = gtk_architecture

    gtk_pc = required["lib/pkgconfig/gtk4.pc"]
    glib_pc = required["lib/pkgconfig/glib-2.0.pc"]
    gtk_pc_version = _pkg_config_value(gtk_pc, "Version", ":")
    glib_pc_version = _pkg_config_value(glib_pc, "Version", ":")
    gtk_host = _pkg_config_value(gtk_pc, "gtk_host", "=")
    pkg_config_prefix = _pkg_config_value(gtk_pc, "prefix", "=")

    expected = {
        "gtk": contract["gtk_version"],
        "glib": contract["glib_version"],
        "architecture": contract["architecture"],
    }
    actual = {
        "gtk": gtk_version,
        "glib": glib_version,
        "architecture": architecture,
    }
    if actual != expected:
        raise ValidationError(f"GTK4 root identity mismatch: expected {expected}, got {actual}")
    if gtk_pc_version != gtk_version or glib_pc_version != glib_version:
        raise ValidationError(
            "GTK4 pkg-config versions do not match their installed headers"
        )
    if gtk_host != "x86_64-windows" or pointer_size != 8:
        raise ValidationError(
            f"GTK4 root is not consistently x64: gtk_host={gtk_host}, pointer_size={pointer_size}"
        )

    return {
        "root": str(root),
        "platform": contract["platform"],
        "architecture": architecture,
        "gtk_version": gtk_version,
        "glib_version": glib_version,
        "pkg_config_prefix": pkg_config_prefix,
        "required_file_count": len(required),
    }


def parse_args(argv):
    parser = argparse.ArgumentParser(description="Validate the pinned Fabulor GTK4 dependency")
    parser.add_argument("--contract", type=pathlib.Path, default=DEFAULT_CONTRACT)
    parser.add_argument("--archive", type=pathlib.Path)
    parser.add_argument("--root", type=pathlib.Path)
    parser.add_argument("--json", action="store_true", dest="as_json")
    args = parser.parse_args(argv)
    if args.archive is None and args.root is None:
        parser.error("at least one of --archive or --root is required")
    return args


def main(argv=None):
    args = parse_args(argv)
    try:
        contract = load_contract(args.contract.resolve(strict=True))
        result = {}
        if args.archive is not None:
            result.update(validate_archive(args.archive, contract))
        if args.root is not None:
            result.update(validate_root(args.root, contract))
    except (OSError, ValidationError) as exc:
        print(f"GTK4 validation failed: {exc}", file=sys.stderr)
        return 1

    if args.as_json:
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        details = ", ".join(f"{key}={value}" for key, value in result.items())
        print(f"GTK4 dependency validated: {details}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
