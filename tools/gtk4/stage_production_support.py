#!/usr/bin/env python3

import argparse
import hashlib
import json
import pathlib
import shutil
import stat
import sys


DEFAULT_CONTRACT = pathlib.Path(__file__).with_name(
    "production-support-contract.json"
)
MANIFEST_NAME = "production-support-manifest.json"
SOURCE_NAMES = {"dependency", "python", "repository", "winsparkle"}
REPARSE_POINT_ATTRIBUTE = getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)


class ProductionSupportError(Exception):
    pass


def _strict_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ProductionSupportError(f"Duplicate contract field: {key}")
        result[key] = value
    return result


def _safe_relative(value, label, allow_dot=False):
    if not isinstance(value, str) or not value or "\\" in value:
        raise ProductionSupportError(f"Invalid {label}: {value!r}")
    path = pathlib.PurePosixPath(value)
    if path.is_absolute() or ".." in path.parts:
        raise ProductionSupportError(f"Unsafe {label}: {value}")
    if path.as_posix() == "." and not allow_dot:
        raise ProductionSupportError(f"Invalid {label}: {value!r}")
    return path


def load_contract(path):
    try:
        with path.open("r", encoding="utf-8") as handle:
            contract = json.load(handle, object_pairs_hook=_strict_object)
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ProductionSupportError(
            f"Unable to read production-support contract {path}: {exc}"
        ) from exc
    required = {"schema_version", "platform", "architecture", "entries"}
    if not isinstance(contract, dict) or set(contract) != required:
        raise ProductionSupportError("Invalid production-support contract fields")
    if (contract["schema_version"] != 1 or contract["platform"] != "windows" or
            contract["architecture"] != "x64"):
        raise ProductionSupportError("Unsupported production-support identity")
    entries = contract["entries"]
    if not isinstance(entries, list) or not entries:
        raise ProductionSupportError("Production-support entries must be non-empty")
    parsed = []
    for entry in entries:
        if not isinstance(entry, dict) or set(entry) != {
                "source", "path", "destination", "kind"}:
            raise ProductionSupportError("Invalid production-support entry fields")
        if entry["source"] not in SOURCE_NAMES:
            raise ProductionSupportError("Invalid production-support source")
        if entry["kind"] not in {"file", "single_glob"}:
            raise ProductionSupportError("Invalid production-support entry kind")
        source = _safe_relative(entry["path"], "source path").as_posix()
        destination = _safe_relative(
            entry["destination"], "destination path",
            allow_dot=(entry["kind"] == "single_glob"),
        ).as_posix()
        if entry["kind"] == "single_glob":
            name = pathlib.PurePosixPath(source).name
            if name.count("*") != 1 or any(char in name for char in "?:[]"):
                raise ProductionSupportError("Invalid production-support glob")
        elif "*" in source:
            raise ProductionSupportError("Wildcard requires single_glob kind")
        parsed.append({**entry, "path": source, "destination": destination})
    return {**contract, "entries": parsed}


def _is_reparse(path):
    attributes = getattr(path.lstat(), "st_file_attributes", 0)
    return path.is_symlink() or bool(attributes & REPARSE_POINT_ATTRIBUTE)


def _regular_root(path, label):
    try:
        absolute = path.absolute()
        if _is_reparse(absolute):
            raise ProductionSupportError(f"{label} is a reparse point: {absolute}")
        resolved = absolute.resolve(strict=True)
    except OSError as exc:
        raise ProductionSupportError(f"Missing {label}: {path}") from exc
    if not resolved.is_dir():
        raise ProductionSupportError(f"{label} is not a directory: {resolved}")
    return resolved


def _contained_file(root, relative):
    candidate = root.joinpath(*pathlib.PurePosixPath(relative).parts)
    try:
        resolved = candidate.resolve(strict=True)
        resolved.relative_to(root)
    except (OSError, ValueError) as exc:
        raise ProductionSupportError(
            f"Production-support source escapes or is missing: {relative}"
        ) from exc
    current = candidate
    while current != root:
        if _is_reparse(current):
            raise ProductionSupportError(
                f"Production-support source crosses a reparse point: {relative}"
            )
        current = current.parent
    if not resolved.is_file():
        raise ProductionSupportError(
            f"Production-support source is not a file: {relative}"
        )
    return resolved


def _contained_directory(root, relative):
    candidate = root.joinpath(*pathlib.PurePosixPath(relative).parts)
    try:
        resolved = candidate.resolve(strict=True)
        resolved.relative_to(root)
    except (OSError, ValueError) as exc:
        raise ProductionSupportError(
            f"Production-support directory escapes or is missing: {relative}"
        ) from exc
    current = candidate
    while current != root:
        if _is_reparse(current):
            raise ProductionSupportError(
                f"Production-support directory crosses a reparse point: {relative}"
            )
        current = current.parent
    if not resolved.is_dir():
        raise ProductionSupportError(
            f"Production-support source is not a directory: {relative}"
        )
    return resolved


def collect_payload(roots, contract):
    resolved_roots = {
        name: _regular_root(path, f"{name} source root")
        for name, path in roots.items()
    }
    selected = {}
    folded = set()
    for entry in contract["entries"]:
        root = resolved_roots[entry["source"]]
        destination = entry["destination"]
        if entry["kind"] == "single_glob":
            relative = pathlib.PurePosixPath(entry["path"])
            parent = _contained_directory(root, relative.parent.as_posix())
            matches = sorted(path for path in parent.glob(relative.name) if path.is_file())
            if len(matches) != 1 or _is_reparse(matches[0]):
                raise ProductionSupportError(
                    f"Expected one regular {entry['path']}, found {len(matches)}"
                )
            source = matches[0]
            destination = (pathlib.PurePosixPath(destination) / source.name).as_posix()
        else:
            source = _contained_file(root, entry["path"])
        key = destination.casefold()
        if key == MANIFEST_NAME.casefold() or key in folded:
            raise ProductionSupportError(
                f"Production-support destination collision: {destination}"
            )
        folded.add(key)
        selected[destination] = source
    return dict(sorted(selected.items(), key=lambda item: item[0].casefold()))


def _record(path, relative):
    digest = hashlib.sha256()
    size = 0
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            size += len(block)
            digest.update(block)
    return {"path": relative, "size_bytes": size, "sha256": digest.hexdigest()}


def stage(roots, output, contract):
    selected = collect_payload(roots, contract)
    output = output.absolute()
    if output.exists() and (_is_reparse(output) or not output.is_dir()):
        raise ProductionSupportError("Production-support output is not regular")
    if output.exists() and any(output.iterdir()):
        raise ProductionSupportError("Production-support output is not empty")
    output.mkdir(parents=True, exist_ok=True)
    records = []
    try:
        for relative, source in selected.items():
            destination = output.joinpath(*pathlib.PurePosixPath(relative).parts)
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, destination)
            records.append(_record(destination, relative))
        manifest = {
            "schema_version": 1,
            "platform": "windows",
            "architecture": "x64",
            "file_count": len(records),
            "size_bytes": sum(record["size_bytes"] for record in records),
            "files": records,
        }
        output.joinpath(MANIFEST_NAME).write_text(
            json.dumps(manifest, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
    except Exception:
        shutil.rmtree(output, ignore_errors=True)
        raise
    return manifest


def parse_args(argv):
    parser = argparse.ArgumentParser(
        description="Stage the allowlisted non-GTK production support payload"
    )
    parser.add_argument("--contract", type=pathlib.Path, default=DEFAULT_CONTRACT)
    parser.add_argument("--dependency-root", type=pathlib.Path, required=True)
    parser.add_argument("--python-build-root", type=pathlib.Path, required=True)
    parser.add_argument("--repository-root", type=pathlib.Path, required=True)
    parser.add_argument("--winsparkle-root", type=pathlib.Path, required=True)
    parser.add_argument("--output", type=pathlib.Path, required=True)
    return parser.parse_args(argv)


def main(argv=None):
    args = parse_args(argv)
    try:
        contract = load_contract(args.contract.resolve(strict=True))
        manifest = stage({
            "dependency": args.dependency_root,
            "python": args.python_build_root,
            "repository": args.repository_root,
            "winsparkle": args.winsparkle_root,
        }, args.output, contract)
    except (OSError, ProductionSupportError) as exc:
        print(f"Production-support staging failed: {exc}", file=sys.stderr)
        return 1
    print(
        "Production support staged: "
        f"files={manifest['file_count']}, bytes={manifest['size_bytes']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
