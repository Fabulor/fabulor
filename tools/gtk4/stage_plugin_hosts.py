#!/usr/bin/env python3

import argparse
import hashlib
import json
import pathlib
import re
import shutil
import stat
import sys


DEFAULT_CONTRACT = pathlib.Path(__file__).with_name(
    "plugin-host-payload-contract.json"
)
MANIFEST_RELATIVE = "Runtime/plugin-host-manifest.json"
REPARSE_POINT_ATTRIBUTE = getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)
SOURCE_NAMES = {"dotnet", "managed", "payload", "python", "tcl"}
ENTRY_KINDS = {"file", "single_glob", "tree"}


class PluginHostStagingError(Exception):
    pass


def _strict_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise PluginHostStagingError(f"Duplicate contract field: {key}")
        result[key] = value
    return result


def _safe_relative(value, label, allow_dot=False):
    if not isinstance(value, str) or not value or "\\" in value:
        raise PluginHostStagingError(f"Invalid {label}: {value!r}")
    path = pathlib.PurePosixPath(value)
    if path.is_absolute() or ".." in path.parts:
        raise PluginHostStagingError(f"Unsafe {label}: {value}")
    if path.as_posix() == "." and not allow_dot:
        raise PluginHostStagingError(f"Invalid {label}: {value!r}")
    return path


def load_contract(path):
    try:
        with path.open("r", encoding="utf-8") as handle:
            contract = json.load(handle, object_pairs_hook=_strict_object)
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise PluginHostStagingError(
            f"Unable to read plugin-host contract {path}: {exc}"
        ) from exc

    required = {
        "schema_version", "platform", "architecture",
        "dotnet_runtime_version", "entries",
    }
    if not isinstance(contract, dict) or set(contract) != required:
        raise PluginHostStagingError("Invalid plugin-host contract fields")
    if (contract["schema_version"] != 1 or contract["platform"] != "windows" or
            contract["architecture"] != "x64"):
        raise PluginHostStagingError("Unsupported plugin-host contract identity")
    version = contract["dotnet_runtime_version"]
    if not isinstance(version, str) or not re.fullmatch(r"\d+\.\d+\.\d+", version):
        raise PluginHostStagingError("Invalid .NET runtime version")
    entries = contract["entries"]
    if not isinstance(entries, list) or not entries:
        raise PluginHostStagingError("Plugin-host entries must be a non-empty array")

    parsed = []
    for entry in entries:
        if not isinstance(entry, dict) or set(entry) != {
                "source", "path", "destination", "kind"}:
            raise PluginHostStagingError("Invalid plugin-host entry fields")
        source = entry["source"]
        kind = entry["kind"]
        if source not in SOURCE_NAMES or kind not in ENTRY_KINDS:
            raise PluginHostStagingError("Invalid plugin-host entry owner or kind")
        source_value = entry["path"].replace("{dotnet_runtime_version}", version)
        destination_value = entry["destination"].replace(
            "{dotnet_runtime_version}", version
        )
        if "{" in source_value or "{" in destination_value:
            raise PluginHostStagingError("Unknown plugin-host contract token")
        if kind == "single_glob":
            if ("/" in source_value or source_value.count("*") != 1 or
                    any(character in source_value for character in "?:[]") or
                    source_value in (".", "..")):
                raise PluginHostStagingError("Invalid single-file glob")
            source_path = source_value
        else:
            source_path = _safe_relative(
                source_value, "source path", allow_dot=(kind == "tree")
            ).as_posix()
        destination = _safe_relative(
            destination_value, "destination path"
        ).as_posix()
        parsed.append({
            "source": source,
            "path": source_path,
            "destination": destination,
            "kind": kind,
        })
    return {
        "schema_version": 1,
        "platform": "windows",
        "architecture": "x64",
        "dotnet_runtime_version": version,
        "entries": parsed,
    }


def _is_reparse(path):
    attributes = getattr(path.lstat(), "st_file_attributes", 0)
    return path.is_symlink() or bool(attributes & REPARSE_POINT_ATTRIBUTE)


def _regular_root(path, label):
    try:
        absolute = path.absolute()
        if _is_reparse(absolute):
            raise PluginHostStagingError(f"{label} is a reparse point: {absolute}")
        resolved = absolute.resolve(strict=True)
    except OSError as exc:
        raise PluginHostStagingError(f"Missing {label}: {path}") from exc
    if not resolved.is_dir():
        raise PluginHostStagingError(f"{label} is not a directory: {resolved}")
    return resolved


def _contained(root, relative, expect_directory):
    candidate = root.joinpath(*pathlib.PurePosixPath(relative).parts)
    try:
        resolved = candidate.resolve(strict=True)
        resolved.relative_to(root)
    except (OSError, ValueError) as exc:
        raise PluginHostStagingError(
            f"Plugin-host source escapes or is missing: {relative}"
        ) from exc
    current = candidate
    while current != root:
        if _is_reparse(current):
            raise PluginHostStagingError(
                f"Plugin-host source crosses a reparse point: {relative}"
            )
        current = current.parent
    if expect_directory != resolved.is_dir():
        kind = "directory" if expect_directory else "file"
        raise PluginHostStagingError(
            f"Plugin-host source is not a {kind}: {relative}"
        )
    return resolved


def _tree_files(root, relative):
    directory = root if relative == "." else _contained(root, relative, True)
    files = []
    for source in sorted(directory.rglob("*")):
        if _is_reparse(source):
            raise PluginHostStagingError(
                f"Plugin-host tree contains a reparse point: {source}"
            )
        if source.is_file():
            files.append((source.relative_to(directory).as_posix(), source))
    if not files:
        raise PluginHostStagingError(f"Plugin-host tree is empty: {relative}")
    return files


def collect_payload(roots, contract):
    resolved_roots = {
        name: _regular_root(path, f"{name} source root")
        for name, path in roots.items()
    }
    selected = {}
    selected_casefold = set()

    def add(destination, source):
        key = destination.casefold()
        if key == MANIFEST_RELATIVE.casefold():
            raise PluginHostStagingError(
                "Plugin-host payload collides with its generated manifest"
            )
        if key in selected_casefold:
            raise PluginHostStagingError(
                f"Plugin-host destination collision: {destination}"
            )
        selected_casefold.add(key)
        selected[destination] = source

    for entry in contract["entries"]:
        root = resolved_roots[entry["source"]]
        kind = entry["kind"]
        if kind == "file":
            add(entry["destination"], _contained(root, entry["path"], False))
        elif kind == "single_glob":
            matches = [path for path in root.glob(entry["path"]) if path.is_file()]
            if len(matches) != 1:
                raise PluginHostStagingError(
                    f"Expected one {entry['path']} under {root}, found {len(matches)}"
                )
            if _is_reparse(matches[0]):
                raise PluginHostStagingError("Plugin-host glob selected a reparse point")
            destination = pathlib.PurePosixPath(entry["destination"]) / matches[0].name
            add(destination.as_posix(), matches[0])
        else:
            for relative, source in _tree_files(root, entry["path"]):
                destination = pathlib.PurePosixPath(entry["destination"]) / relative
                add(destination.as_posix(), source)
    return dict(sorted(selected.items(), key=lambda item: item[0].casefold()))


def _file_record(path, relative):
    digest = hashlib.sha256()
    size = 0
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            size += len(block)
            digest.update(block)
    return {"path": relative, "size_bytes": size, "sha256": digest.hexdigest()}


def stage_plugin_hosts(roots, output, contract):
    selected = collect_payload(roots, contract)
    output = output.absolute()
    if output.exists() and (_is_reparse(output) or not output.is_dir()):
        raise PluginHostStagingError("Plugin-host output is not a regular directory")
    if output.exists() and any(output.iterdir()):
        raise PluginHostStagingError("Plugin-host output is not empty")
    output.mkdir(parents=True, exist_ok=True)
    entries = []
    try:
        for relative, source in selected.items():
            destination = output.joinpath(*pathlib.PurePosixPath(relative).parts)
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, destination)
            entries.append(_file_record(destination, relative))
        manifest = {
            "schema_version": 1,
            "platform": contract["platform"],
            "architecture": contract["architecture"],
            "dotnet_runtime_version": contract["dotnet_runtime_version"],
            "file_count": len(entries),
            "size_bytes": sum(entry["size_bytes"] for entry in entries),
            "files": entries,
        }
        manifest_path = output.joinpath(*pathlib.PurePosixPath(MANIFEST_RELATIVE).parts)
        manifest_path.parent.mkdir(parents=True, exist_ok=True)
        manifest_path.write_text(
            json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8"
        )
    except Exception:
        shutil.rmtree(output, ignore_errors=True)
        raise
    return manifest


def load_manifest(path):
    try:
        manifest = json.loads(
            path.read_text(encoding="utf-8"), object_pairs_hook=_strict_object
        )
        files = manifest["files"]
    except (OSError, UnicodeDecodeError, json.JSONDecodeError, KeyError) as exc:
        raise PluginHostStagingError(f"Unable to read plugin-host manifest: {exc}") from exc
    required = {
        "schema_version", "platform", "architecture", "dotnet_runtime_version",
        "file_count", "size_bytes", "files",
    }
    if not isinstance(manifest, dict) or set(manifest) != required:
        raise PluginHostStagingError("Invalid plugin-host manifest fields")
    if (manifest["schema_version"] != 1 or manifest["platform"] != "windows" or
            manifest["architecture"] != "x64" or
            not isinstance(manifest["dotnet_runtime_version"], str) or
            not isinstance(files, list)):
        raise PluginHostStagingError("Invalid plugin-host manifest identity")
    parsed = {}
    parsed_casefold = set()
    total_size = 0
    for entry in files:
        if not isinstance(entry, dict) or set(entry) != {"path", "size_bytes", "sha256"}:
            raise PluginHostStagingError("Invalid plugin-host manifest entry")
        relative = _safe_relative(entry["path"], "manifest path").as_posix()
        size = entry["size_bytes"]
        digest = entry["sha256"]
        if (not isinstance(size, int) or size < 0 or not isinstance(digest, str) or
                not re.fullmatch(r"[0-9a-f]{64}", digest)):
            raise PluginHostStagingError("Invalid plugin-host manifest content record")
        if relative.casefold() == MANIFEST_RELATIVE.casefold():
            raise PluginHostStagingError("Plugin-host manifest lists itself")
        if relative.casefold() in parsed_casefold:
            raise PluginHostStagingError(f"Duplicate plugin-host manifest path: {relative}")
        parsed[relative] = (size, digest)
        parsed_casefold.add(relative.casefold())
        total_size += size
    if manifest["file_count"] != len(parsed) or manifest["size_bytes"] != total_size:
        raise PluginHostStagingError("Plugin-host manifest totals do not match")
    return manifest, parsed


def parse_args(argv):
    parser = argparse.ArgumentParser(
        description="Stage allowlisted Fabulor C#, Python, and Tcl plugin hosts"
    )
    parser.add_argument("--contract", type=pathlib.Path, default=DEFAULT_CONTRACT)
    parser.add_argument("--dotnet-root", type=pathlib.Path, required=True)
    parser.add_argument("--managed-root", type=pathlib.Path, required=True)
    parser.add_argument("--payload-root", type=pathlib.Path, required=True)
    parser.add_argument("--python-root", type=pathlib.Path, required=True)
    parser.add_argument("--tcl-root", type=pathlib.Path, required=True)
    parser.add_argument("--output", type=pathlib.Path, required=True)
    return parser.parse_args(argv)


def main(argv=None):
    args = parse_args(argv)
    try:
        contract = load_contract(args.contract.resolve(strict=True))
        manifest = stage_plugin_hosts({
            "dotnet": args.dotnet_root,
            "managed": args.managed_root,
            "payload": args.payload_root,
            "python": args.python_root,
            "tcl": args.tcl_root,
        }, args.output, contract)
    except (OSError, PluginHostStagingError) as exc:
        print(f"Plugin-host staging failed: {exc}", file=sys.stderr)
        return 1
    print(
        "Plugin hosts staged: "
        f"files={manifest['file_count']}, size_bytes={manifest['size_bytes']}, "
        f"dotnet={manifest['dotnet_runtime_version']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
