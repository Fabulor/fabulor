#!/usr/bin/env python3

import argparse
import hashlib
import json
import pathlib
import re
import shutil
import stat
import sys


DEFAULT_CONTRACT = pathlib.Path(__file__).with_name("runtime-payload-contract.json")
MANIFEST_NAME = "runtime-manifest.json"
REPARSE_POINT_ATTRIBUTE = getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)


class StagingError(Exception):
    pass


def _strict_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise StagingError(f"Duplicate contract field: {key}")
        result[key] = value
    return result


def _safe_relative(value, field):
    if not isinstance(value, str) or not value:
        raise StagingError(f"{field} entries must be non-empty strings")
    path = pathlib.PurePosixPath(value)
    if path.is_absolute() or ".." in path.parts or "\\" in value:
        raise StagingError(f"Unsafe {field} path: {value}")
    return path


def load_contract(path):
    try:
        with path.open("r", encoding="utf-8") as handle:
            contract = json.load(handle, object_pairs_hook=_strict_object)
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise StagingError(f"Unable to read runtime contract {path}: {exc}") from exc

    required = {
        "schema_version",
        "platform",
        "architecture",
        "source_contract",
        "files",
        "trees",
        "text_normalizations",
        "forbidden_suffixes",
    }
    if not isinstance(contract, dict) or set(contract) != required:
        missing = sorted(required.difference(contract))
        unknown = sorted(set(contract).difference(required))
        raise StagingError(
            f"Invalid runtime contract fields; missing={missing}, unknown={unknown}"
        )
    if contract["schema_version"] != 1:
        raise StagingError("Unsupported runtime contract schema version")
    if contract["platform"] != "windows" or contract["architecture"] != "x64":
        raise StagingError("The runtime contract must target Windows x64")

    _safe_relative(contract["source_contract"], "source_contract")
    for field in ("files", "trees"):
        values = contract[field]
        if not isinstance(values, list) or not values:
            raise StagingError(f"{field} must be a non-empty array")
        normalized = [_safe_relative(value, field).as_posix() for value in values]
        if normalized != sorted(normalized, key=str.casefold):
            raise StagingError(f"{field} must be sorted case-insensitively")
        if len({value.casefold() for value in normalized}) != len(normalized):
            raise StagingError(f"{field} contains duplicate paths")

    normalizations = contract["text_normalizations"]
    if not isinstance(normalizations, dict):
        raise StagingError("text_normalizations must be an object")
    for relative, operation in normalizations.items():
        _safe_relative(relative, "text_normalizations")
        if relative not in contract["files"]:
            raise StagingError(f"Normalization target is not an exact file: {relative}")
        if operation != "gdk-pixbuf-loader-cache":
            raise StagingError(f"Unsupported text normalization: {operation}")

    suffixes = contract["forbidden_suffixes"]
    if not isinstance(suffixes, list) or not suffixes:
        raise StagingError("forbidden_suffixes must be a non-empty array")
    if suffixes != sorted(suffixes) or len(suffixes) != len(set(suffixes)):
        raise StagingError("forbidden_suffixes must be sorted and unique")
    if any(not isinstance(value, str) or not value.startswith(".") for value in suffixes):
        raise StagingError("forbidden_suffixes entries must begin with a dot")
    return contract


def _contained(root, relative, expect_directory):
    candidate = root.joinpath(*pathlib.PurePosixPath(relative).parts)
    try:
        resolved = candidate.resolve(strict=True)
    except OSError as exc:
        raise StagingError(f"Missing runtime payload path: {relative}") from exc
    try:
        resolved.relative_to(root)
    except ValueError as exc:
        raise StagingError(f"Runtime payload path escapes its root: {relative}") from exc
    current = candidate
    while current != root:
        attributes = getattr(current.lstat(), "st_file_attributes", 0)
        if current.is_symlink() or attributes & REPARSE_POINT_ATTRIBUTE:
            raise StagingError(f"Runtime payload path crosses a reparse point: {relative}")
        current = current.parent
    if expect_directory != resolved.is_dir():
        kind = "directory" if expect_directory else "file"
        raise StagingError(f"Runtime payload path is not a {kind}: {relative}")
    return resolved


def collect_payload(root, contract):
    try:
        root = root.resolve(strict=True)
    except OSError as exc:
        raise StagingError(f"GTK4 runtime root does not exist: {root}") from exc
    if not root.is_dir():
        raise StagingError(f"GTK4 runtime root is not a directory: {root}")

    selected = {}
    selected_casefold = set()
    for relative in contract["files"]:
        selected[relative] = _contained(root, relative, False)
        selected_casefold.add(relative.casefold())
    for tree in contract["trees"]:
        directory = _contained(root, tree, True)
        for source in sorted(directory.rglob("*")):
            attributes = getattr(source.lstat(), "st_file_attributes", 0)
            if source.is_symlink() or attributes & REPARSE_POINT_ATTRIBUTE:
                relative = source.relative_to(root).as_posix()
                raise StagingError(f"Runtime payload contains a reparse point: {relative}")
            if source.is_file():
                relative = source.relative_to(root).as_posix()
                if relative.casefold() in selected_casefold:
                    raise StagingError(f"Runtime payload path selected more than once: {relative}")
                selected[relative] = source
                selected_casefold.add(relative.casefold())

    forbidden = tuple(value.casefold() for value in contract["forbidden_suffixes"])
    rejected = [relative for relative in selected if relative.casefold().endswith(forbidden)]
    if rejected:
        raise StagingError(f"Runtime payload selects forbidden files: {', '.join(rejected)}")
    return dict(sorted(selected.items(), key=lambda item: item[0].casefold()))


def _normalized_bytes(relative, source, contract):
    operation = contract["text_normalizations"].get(relative)
    data = source.read_bytes()
    if operation is None:
        return data
    try:
        text = data.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise StagingError(f"Normalization target is not UTF-8: {relative}") from exc
    if operation == "gdk-pixbuf-loader-cache":
        text, count = re.subn(
            r"(?m)^# LoaderDir = .*$",
            "# LoaderDir is resolved relative to the Fabulor GTK4 runtime root",
            text,
        )
        if count != 1:
            raise StagingError(f"Unexpected GDK pixbuf loader cache format: {relative}")
        return text.encode("utf-8")
    raise StagingError(f"Unsupported text normalization: {operation}")


def _sha256(data):
    return hashlib.sha256(data).hexdigest()


def stage_runtime(root, output, contract, contract_path):
    selected = collect_payload(root, contract)
    output = output.absolute()
    if output.exists():
        attributes = getattr(output.lstat(), "st_file_attributes", 0)
        if (not output.is_dir() or output.is_symlink() or
                attributes & REPARSE_POINT_ATTRIBUTE):
            raise StagingError(f"Runtime staging output is not a regular directory: {output}")
    if output.exists() and any(output.iterdir()):
        raise StagingError(f"Runtime staging output is not empty: {output}")

    source_contract_path = contract_path.parent / contract["source_contract"]
    try:
        source_contract = json.loads(source_contract_path.read_text(encoding="utf-8"))
        source_identity = source_contract["source"]["sha256"]
    except (OSError, UnicodeDecodeError, json.JSONDecodeError, KeyError, TypeError) as exc:
        raise StagingError(f"Unable to read source dependency identity: {exc}") from exc
    if not isinstance(source_identity, str) or not re.fullmatch(r"[0-9a-f]{64}", source_identity):
        raise StagingError("Source dependency identity is not a lowercase SHA-256")

    output.mkdir(parents=True, exist_ok=True)

    entries = []
    try:
        for relative, source in selected.items():
            data = _normalized_bytes(relative, source, contract)
            destination = output.joinpath(*pathlib.PurePosixPath(relative).parts)
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(data)
            entries.append(
                {"path": relative, "size_bytes": len(data), "sha256": _sha256(data)}
            )

        manifest = {
            "schema_version": 1,
            "platform": contract["platform"],
            "architecture": contract["architecture"],
            "source_archive_sha256": source_identity,
            "file_count": len(entries),
            "size_bytes": sum(entry["size_bytes"] for entry in entries),
            "files": entries,
        }
        manifest_path = output / MANIFEST_NAME
        manifest_path.write_text(
            json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8"
        )
    except Exception:
        shutil.rmtree(output, ignore_errors=True)
        raise
    return manifest


def parse_args(argv):
    parser = argparse.ArgumentParser(description="Stage the allowlisted Fabulor GTK4 runtime")
    parser.add_argument("--contract", type=pathlib.Path, default=DEFAULT_CONTRACT)
    parser.add_argument("--root", type=pathlib.Path, required=True)
    parser.add_argument("--output", type=pathlib.Path)
    parser.add_argument("--validate-only", action="store_true")
    args = parser.parse_args(argv)
    if args.validate_only == (args.output is not None):
        parser.error("select exactly one of --validate-only or --output")
    return args


def main(argv=None):
    args = parse_args(argv)
    try:
        contract_path = args.contract.resolve(strict=True)
        contract = load_contract(contract_path)
        if args.validate_only:
            selected = collect_payload(args.root, contract)
            print(f"GTK4 runtime payload validated: file_count={len(selected)}")
        else:
            manifest = stage_runtime(args.root, args.output, contract, contract_path)
            print(
                "GTK4 runtime staged: "
                f"file_count={manifest['file_count']}, size_bytes={manifest['size_bytes']}, "
                f"output={args.output.resolve()}"
            )
    except (OSError, StagingError) as exc:
        print(f"GTK4 runtime staging failed: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
