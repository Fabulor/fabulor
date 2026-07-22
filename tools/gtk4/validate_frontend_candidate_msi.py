#!/usr/bin/env python3

import argparse
import hashlib
import pathlib
import shutil
import sys
import tempfile
import xml.etree.ElementTree as ET

import validate_frontend_bootstrap
import validate_native_extensions
import validate_runtime_imports
import validate_runtime_msi


NS = validate_runtime_msi.NS
PRODUCT_NAME = "Fabulor GTK4 Frontend Candidate"
UPGRADE_CODE = "B0361915-6035-48B7-B535-8E72AB7493AA"
INSTALL_FOLDER_NAME = "Fabulor GTK4 Candidate"
ROOT_FILES = {
    "cert.pem": "payload",
    "fabulor.exe": "frontend",
    "fabulor-gtk4-frontend.dll": "frontend",
    "libcrypto-3-x64.dll": "payload",
    "libssl-3-x64.dll": "payload",
}
FORBIDDEN_INSTALLER_ELEMENTS = (
    "Environment",
    "RegistryKey",
    "RegistryValue",
    "ServiceControl",
    "ServiceInstall",
    "Shortcut",
)


class FrontendCandidateMsiError(Exception):
    pass


def validate_identity(xml_root):
    packages = xml_root.findall(".//w:Package", NS)
    if len(packages) != 1:
        raise FrontendCandidateMsiError(
            f"Expected one decompiled package, found {len(packages)}"
        )
    package = packages[0]
    if package.get("Name") != PRODUCT_NAME:
        raise FrontendCandidateMsiError(
            f"Unexpected package name: {package.get('Name')!r}"
        )
    upgrade_code = (package.get("UpgradeCode") or "").upper()
    if upgrade_code not in (UPGRADE_CODE, f"{{{UPGRADE_CODE}}}"):
        raise FrontendCandidateMsiError(
            f"Unexpected package upgrade code: {package.get('UpgradeCode')!r}"
        )
    for element_name in FORBIDDEN_INSTALLER_ELEMENTS:
        if package.findall(f".//w:{element_name}", NS):
            raise FrontendCandidateMsiError(
                f"Candidate package contains forbidden {element_name} entries"
            )


def find_install_folder(xml_root):
    directories = [
        node for node in xml_root.findall(".//w:Directory", NS)
        if node.get("Id") == "INSTALLFOLDER"
    ]
    if len(directories) != 1:
        raise FrontendCandidateMsiError(
            f"Expected one INSTALLFOLDER, found {len(directories)}"
        )
    if directories[0].get("Name") != INSTALL_FOLDER_NAME:
        raise FrontendCandidateMsiError(
            f"Unexpected install folder name: {directories[0].get('Name')!r}"
        )
    return directories[0]


def extract_sources(directory):
    sources = {}

    def walk(node, prefix=pathlib.PurePosixPath()):
        for component in node.findall("w:Component", NS):
            for file_node in component.findall("w:File", NS):
                name = file_node.get("Name")
                source = file_node.get("Source")
                if (not name or not source or
                        any(separator in name for separator in ("/", "\\"))):
                    raise FrontendCandidateMsiError(
                        "Candidate MSI contains an invalid file entry"
                    )
                relative = (prefix / name).as_posix()
                if relative in sources:
                    raise FrontendCandidateMsiError(
                        f"Candidate MSI contains duplicate path: {relative}"
                    )
                sources[relative] = source
        for child in node.findall("w:Directory", NS):
            name = child.get("Name")
            if (not name or name in (".", "..") or
                    any(separator in name for separator in ("/", "\\"))):
                raise FrontendCandidateMsiError(
                    "Candidate MSI contains an invalid directory entry"
                )
            walk(child, prefix / name)

    walk(directory)
    return sources


def expected_paths(manifest_path, extension_contract):
    runtime_paths = validate_runtime_msi.load_expected_paths(manifest_path)
    extension_paths = {
        entry["path"] for entry in
        extension_contract["modules"] + extension_contract["data_files"]
    }
    return set(ROOT_FILES) | extension_paths | {
        f"Runtime/GTK4/{relative}" for relative in runtime_paths
    }


def validate_paths(expected, actual):
    missing = sorted(expected.difference(actual))
    unexpected = sorted(actual.difference(expected))
    if missing or unexpected:
        raise FrontendCandidateMsiError(
            "Candidate MSI payload differs from the minimal contract; "
            f"missing={missing[:20]}, unexpected={unexpected[:20]}"
        )


def extracted_path(source, extraction_root):
    source_path = pathlib.PureWindowsPath(source)
    if (source_path.is_absolute() or ".." in source_path.parts or
            not source_path.parts or source_path.parts[0].casefold() != "sourcedir"):
        raise FrontendCandidateMsiError(
            f"Unexpected decompiled MSI source path: {source}"
        )
    return extraction_root.joinpath(*source_path.parts[1:])


def file_content(path):
    digest = hashlib.sha256()
    size = 0
    try:
        with path.open("rb") as handle:
            for block in iter(lambda: handle.read(1024 * 1024), b""):
                size += len(block)
                digest.update(block)
    except OSError as exc:
        raise FrontendCandidateMsiError(
            f"Unable to read candidate file {path}: {exc}"
        ) from exc
    return size, digest.hexdigest()


def expected_content(manifest_path, frontend_root, payload_root, enchant_root,
                     extension_contract):
    expected = {
        f"Runtime/GTK4/{relative}": content
        for relative, content in
        validate_runtime_msi.load_expected_content(manifest_path).items()
    }
    for name, source_root in ROOT_FILES.items():
        root = frontend_root if source_root == "frontend" else payload_root
        expected[name] = file_content(root / name)
    extension_roots = {
        "plugins": frontend_root / "plugins",
        "payload": payload_root,
        "enchant": enchant_root,
    }
    for entry in extension_contract["modules"] + extension_contract["data_files"]:
        expected[entry["path"]] = file_content(
            validate_native_extensions.source_path(entry, extension_roots)
        )
    return expected


def validate_content(expected, sources, extraction_root):
    extracted = {}
    for relative, expected_value in expected.items():
        source = sources.get(relative)
        if source is None:
            raise FrontendCandidateMsiError(
                f"Candidate MSI content source is missing: {relative}"
            )
        path = extracted_path(source, extraction_root)
        actual_value = file_content(path)
        if actual_value != expected_value:
            raise FrontendCandidateMsiError(
                f"Candidate MSI content differs for {relative}: "
                f"expected size/hash {expected_value[0]}/{expected_value[1]}, "
                f"got {actual_value[0]}/{actual_value[1]}"
            )
        extracted[relative] = path
    return extracted


def materialize_install_layout(extracted, layout_root):
    for relative, source in extracted.items():
        destination = layout_root.joinpath(*pathlib.PurePosixPath(relative).parts)
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source, destination)


def validate_packaged_bootstrap(layout_root, dumpbin, contract_path):
    launcher = layout_root / "fabulor.exe"
    frontend = layout_root / "fabulor-gtk4-frontend.dll"
    runtime_root = layout_root / "Runtime" / "GTK4"
    contract = validate_runtime_imports.load_contract(contract_path)
    runtime_modules = set(validate_runtime_imports.discover_native_files(runtime_root))
    launcher_imports = validate_runtime_imports.inspect_dependencies(dumpbin, launcher)
    frontend_imports = validate_runtime_imports.inspect_dependencies(dumpbin, frontend)
    validate_frontend_bootstrap.validate_import_sets(
        launcher_imports, frontend_imports, runtime_modules, contract
    )
    exports = validate_frontend_bootstrap.inspect_exports(dumpbin, frontend)
    if validate_frontend_bootstrap.FRONTEND_ENTRY not in exports:
        raise FrontendCandidateMsiError(
            "Packaged GTK4 frontend does not export "
            f"{validate_frontend_bootstrap.FRONTEND_ENTRY}"
        )
    return len(launcher_imports), len(frontend_imports)


def parse_args(argv):
    parser = argparse.ArgumentParser(
        description="Validate the minimal GTK4 frontend candidate MSI"
    )
    parser.add_argument("--wix", type=pathlib.Path, required=True)
    parser.add_argument("--msi", type=pathlib.Path, required=True)
    parser.add_argument("--manifest", type=pathlib.Path, required=True)
    parser.add_argument("--frontend-root", type=pathlib.Path, required=True)
    parser.add_argument("--payload-root", type=pathlib.Path, required=True)
    parser.add_argument("--enchant-root", type=pathlib.Path, required=True)
    parser.add_argument("--layout-output", type=pathlib.Path)
    parser.add_argument("--dumpbin", required=True)
    parser.add_argument(
        "--extension-contract",
        type=pathlib.Path,
        default=pathlib.Path(__file__).with_name("native-extension-contract.json"),
    )
    parser.add_argument(
        "--contract",
        type=pathlib.Path,
        default=pathlib.Path(__file__).with_name("runtime-import-contract.json"),
    )
    return parser.parse_args(argv)


def main(argv=None):
    args = parse_args(argv)
    try:
        wix = args.wix.resolve(strict=True)
        msi = args.msi.resolve(strict=True)
        manifest = args.manifest.resolve(strict=True)
        frontend_root = args.frontend_root.resolve(strict=True)
        payload_root = args.payload_root.resolve(strict=True)
        enchant_root = args.enchant_root.resolve(strict=True)
        dumpbin = validate_runtime_imports.resolve_tool(args.dumpbin)
        contract = args.contract.resolve(strict=True)
        layout_output = args.layout_output.resolve() if args.layout_output else None
        if layout_output and layout_output.exists():
            raise FrontendCandidateMsiError(
                f"Layout output must not already exist: {layout_output}"
            )
        extension_contract = validate_native_extensions.load_contract(
            args.extension_contract.resolve(strict=True)
        )
        expected = expected_paths(manifest, extension_contract)
        expected_hashes = expected_content(
            manifest, frontend_root, payload_root, enchant_root,
            extension_contract
        )
        temporary_root = msi.parent / "frontend-candidate-msi-validation"
        temporary_root.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=temporary_root) as temporary:
            temporary = pathlib.Path(temporary)
            decompiled = temporary / "candidate.wxs"
            extraction_root = temporary / "files"
            validate_runtime_msi.decompile_msi(
                wix, msi, decompiled, extraction_root
            )
            xml_root = ET.parse(decompiled).getroot()
            validate_identity(xml_root)
            sources = extract_sources(find_install_folder(xml_root))
            validate_paths(expected, set(sources))
            extracted = validate_content(
                expected_hashes, sources, extraction_root
            )
            layout_root = temporary / "installed-layout"
            materialize_install_layout(extracted, layout_root)
            launcher_count, frontend_count = validate_packaged_bootstrap(
                layout_root, dumpbin, contract
            )
            module_count, data_count, extension_edges = (
                validate_native_extensions.validate_native_extensions(
                    extension_contract,
                    {
                        "plugins": layout_root / "plugins",
                        "payload": layout_root,
                        "enchant": layout_root,
                    },
                    layout_root / "Runtime" / "GTK4", dumpbin,
                    validate_runtime_imports.load_contract(contract),
                )
            )
            if layout_output:
                shutil.copytree(layout_root, layout_output)
    except (OSError, ET.ParseError, validate_runtime_msi.CandidateMsiError,
            validate_runtime_imports.RuntimeImportError,
            validate_frontend_bootstrap.FrontendBootstrapError,
            validate_native_extensions.NativeExtensionError,
            FrontendCandidateMsiError) as exc:
        print(f"GTK4 frontend candidate MSI validation failed: {exc}", file=sys.stderr)
        return 1

    print(
        "GTK4 frontend candidate MSI validated: "
        f"files={len(expected)}, content_hashes=verified, "
        f"launcher_imports={launcher_count}, frontend_imports={frontend_count}, "
        f"extension_modules={module_count}, extension_data={data_count}, "
        f"extension_edges={extension_edges}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
