#!/usr/bin/env python3

import argparse
import hashlib
import json
import pathlib
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET


WIX_NAMESPACE = "http://wixtoolset.org/schemas/v4/wxs"
NS = {"w": WIX_NAMESPACE}


class CandidateMsiError(Exception):
    pass


def load_expected_paths(manifest_path):
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        entries = manifest["files"]
        declared_count = manifest["file_count"]
    except (OSError, UnicodeDecodeError, json.JSONDecodeError, KeyError, TypeError) as exc:
        raise CandidateMsiError(f"Unable to read runtime manifest: {exc}") from exc
    if not isinstance(entries, list) or not isinstance(declared_count, int):
        raise CandidateMsiError("Runtime manifest has invalid file metadata")
    paths = []
    for entry in entries:
        if not isinstance(entry, dict) or not isinstance(entry.get("path"), str):
            raise CandidateMsiError("Runtime manifest contains an invalid file entry")
        relative = pathlib.PurePosixPath(entry["path"])
        if relative.is_absolute() or ".." in relative.parts or "\\" in entry["path"]:
            raise CandidateMsiError(f"Runtime manifest contains an unsafe path: {entry['path']}")
        paths.append(relative.as_posix())
    if declared_count != len(paths) or len(paths) != len(set(paths)):
        raise CandidateMsiError("Runtime manifest file count or uniqueness is invalid")
    return set(paths) | {"runtime-manifest.json"}


def load_expected_content(manifest_path):
    try:
        manifest_bytes = manifest_path.read_bytes()
        manifest = json.loads(manifest_bytes.decode("utf-8"))
        entries = manifest["files"]
    except (OSError, UnicodeDecodeError, json.JSONDecodeError, KeyError, TypeError) as exc:
        raise CandidateMsiError(f"Unable to read runtime manifest content: {exc}") from exc
    expected = {}
    for entry in entries:
        try:
            relative = entry["path"]
            size_bytes = entry["size_bytes"]
            sha256 = entry["sha256"]
        except (KeyError, TypeError) as exc:
            raise CandidateMsiError("Runtime manifest contains invalid hashes") from exc
        if (not isinstance(relative, str) or not isinstance(size_bytes, int) or
                not isinstance(sha256, str)):
            raise CandidateMsiError("Runtime manifest contains invalid hashes")
        expected[relative] = (size_bytes, sha256)
    expected["runtime-manifest.json"] = (
        len(manifest_bytes), hashlib.sha256(manifest_bytes).hexdigest()
    )
    return expected


def extract_gtk4_paths(xml_root):
    gtk_directories = [
        node for node in xml_root.findall(".//w:Directory", NS)
        if node.get("Id") == "GTK4DIR"
    ]
    if len(gtk_directories) != 1:
        raise CandidateMsiError(
            f"Expected one decompiled GTK4DIR, found {len(gtk_directories)}"
        )

    paths = []

    def walk(directory, prefix=pathlib.PurePosixPath()):
        for component in directory.findall("w:Component", NS):
            for file_node in component.findall("w:File", NS):
                name = file_node.get("Name")
                if not name or any(separator in name for separator in ("/", "\\")):
                    raise CandidateMsiError("Candidate MSI contains an invalid file name")
                paths.append((prefix / name).as_posix())
        for child in directory.findall("w:Directory", NS):
            name = child.get("Name")
            if not name or name in (".", "..") or any(
                    separator in name for separator in ("/", "\\")):
                raise CandidateMsiError("Candidate MSI contains an invalid directory name")
            walk(child, prefix / name)

    walk(gtk_directories[0])
    if len(paths) != len(set(paths)):
        raise CandidateMsiError("Candidate MSI contains duplicate GTK4 payload paths")
    return set(paths)


def extract_gtk4_sources(xml_root):
    gtk_directories = [
        node for node in xml_root.findall(".//w:Directory", NS)
        if node.get("Id") == "GTK4DIR"
    ]
    if len(gtk_directories) != 1:
        raise CandidateMsiError(
            f"Expected one decompiled GTK4DIR, found {len(gtk_directories)}"
        )
    sources = {}

    def walk(directory, prefix=pathlib.PurePosixPath()):
        for component in directory.findall("w:Component", NS):
            for file_node in component.findall("w:File", NS):
                name = file_node.get("Name")
                source = file_node.get("Source")
                if not name or not source:
                    raise CandidateMsiError("Candidate MSI file is missing name or source")
                relative = (prefix / name).as_posix()
                if relative in sources:
                    raise CandidateMsiError(
                        "Candidate MSI contains duplicate GTK4 payload paths"
                    )
                sources[relative] = source
        for child in directory.findall("w:Directory", NS):
            name = child.get("Name")
            if not name:
                raise CandidateMsiError("Candidate MSI directory is missing a name")
            walk(child, prefix / name)

    walk(gtk_directories[0])
    return sources


def validate_paths(expected, actual):
    missing = sorted(expected.difference(actual))
    unexpected = sorted(actual.difference(expected))
    if missing or unexpected:
        raise CandidateMsiError(
            "Candidate MSI GTK4 payload differs from the runtime manifest; "
            f"missing={missing[:20]}, unexpected={unexpected[:20]}"
        )


def _sha256_file(path):
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def validate_content(expected, sources, extraction_root):
    for relative, (expected_size, expected_hash) in expected.items():
        source = sources.get(relative)
        if source is None:
            raise CandidateMsiError(f"Candidate MSI content source is missing: {relative}")
        source_path = pathlib.PureWindowsPath(source)
        if (source_path.is_absolute() or ".." in source_path.parts or
                not source_path.parts or source_path.parts[0].casefold() != "sourcedir"):
            raise CandidateMsiError(f"Unexpected decompiled MSI source path: {source}")
        extracted = extraction_root.joinpath(*source_path.parts[1:])
        try:
            actual_size = extracted.stat().st_size
            actual_hash = _sha256_file(extracted)
        except OSError as exc:
            raise CandidateMsiError(
                f"Unable to read extracted candidate file {relative}: {exc}"
            ) from exc
        if actual_size != expected_size or actual_hash != expected_hash:
            raise CandidateMsiError(
                f"Candidate MSI content differs for {relative}: "
                f"expected size/hash {expected_size}/{expected_hash}, "
                f"got {actual_size}/{actual_hash}"
            )


def decompile_msi(wix, msi, output, extraction_root):
    command = [
        str(wix),
        "msi",
        "decompile",
        str(msi),
        "-o",
        str(output),
        "-sui",
        "-sct",
        "-x",
        str(extraction_root),
    ]
    try:
        completed = subprocess.run(
            command, check=False, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            text=True, encoding="utf-8", errors="replace"
        )
    except OSError as exc:
        raise CandidateMsiError(f"Unable to run WiX decompiler: {exc}") from exc
    if completed.returncode != 0:
        raise CandidateMsiError(
            f"WiX MSI decompile failed with exit code {completed.returncode}:\n"
            f"{completed.stdout}"
        )


def parse_args(argv):
    parser = argparse.ArgumentParser(
        description="Validate a candidate MSI GTK4 payload against its manifest"
    )
    parser.add_argument("--wix", type=pathlib.Path, required=True)
    parser.add_argument("--msi", type=pathlib.Path, required=True)
    parser.add_argument("--manifest", type=pathlib.Path, required=True)
    return parser.parse_args(argv)


def main(argv=None):
    args = parse_args(argv)
    try:
        wix = args.wix.resolve(strict=True)
        msi = args.msi.resolve(strict=True)
        manifest = args.manifest.resolve(strict=True)
        expected = load_expected_paths(manifest)
        expected_content = load_expected_content(manifest)
        temporary_root = msi.parent / "candidate-msi-validation"
        temporary_root.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=temporary_root) as temporary:
            decompiled = pathlib.Path(temporary) / "candidate.wxs"
            extraction_root = pathlib.Path(temporary) / "files"
            decompile_msi(wix, msi, decompiled, extraction_root)
            xml_root = ET.parse(decompiled).getroot()
            actual = extract_gtk4_paths(xml_root)
            sources = extract_gtk4_sources(xml_root)
            validate_paths(expected, actual)
            validate_content(expected_content, sources, extraction_root)
    except (OSError, ET.ParseError, CandidateMsiError) as exc:
        print(f"GTK4 candidate MSI validation failed: {exc}", file=sys.stderr)
        return 1

    print(
        "GTK4 candidate MSI validated: "
        f"manifest_entries={len(expected) - 1}, installed_entries={len(actual)}, "
        "content_hashes=verified"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
