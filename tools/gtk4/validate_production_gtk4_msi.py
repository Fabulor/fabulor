#!/usr/bin/env python3

import argparse
import pathlib
import sys
import tempfile
import xml.etree.ElementTree as ET

import validate_runtime_msi


NS = validate_runtime_msi.NS
PRODUCT_NAME = "Fabulor"
UPGRADE_CODE = "8F6C0C7E-9A4D-4E4C-9F8C-2B6F5A4E9C11"
REQUIRED_PATHS = {
    "fabulor.exe",
    "fabulor-gtk4-frontend.dll",
    "Plugins/hcchecksum.dll",
    "Plugins/hcexec.dll",
    "Plugins/hcfishlim.dll",
    "Plugins/hcnotifications-winrt.dll",
    "Plugins/hcpython3.dll",
    "Plugins/hcsysinfo.dll",
    "Plugins/hcupd.dll",
    "Runtime/plugin-host-manifest.json",
    "Runtime/GTK4/bin/gtk-4-1.dll",
    "share/doc/fabulor/COPYING",
    "share/doc/NotoColorEmoji/NotoColorEmoji-LICENSE.txt",
    "share/emoji-flags/eu.png",
    "share/fonts/NotoColorEmoji.ttf",
    "share/palettes/Fabulor Dark.hct",
}
REQUIRED_FEATURES = {
    "MainFeature",
    "StartMenuFeature",
    "ShellIntegrationFeature",
    "Gtk4RuntimeFeature",
    "DotNetRuntimeFeature",
    "PythonRuntimeFeature",
    "TclRuntimeFeature",
}
FORBIDDEN_MARKERS = (
    "atk-1.0",
    "gdk-3",
    "gtk-3",
    "gtk3",
)


class ProductionGtk4MsiError(Exception):
    pass


def validate_identity(root):
    packages = root.findall(".//w:Package", NS)
    if len(packages) != 1:
        raise ProductionGtk4MsiError(
            f"Expected one package, found {len(packages)}"
        )
    package = packages[0]
    if package.get("Name") != PRODUCT_NAME:
        raise ProductionGtk4MsiError(
            f"Unexpected package name: {package.get('Name')!r}"
        )
    upgrade_code = (package.get("UpgradeCode") or "").strip("{}").upper()
    if upgrade_code != UPGRADE_CODE:
        raise ProductionGtk4MsiError(
            f"Unexpected upgrade code: {package.get('UpgradeCode')!r}"
        )


def collect_paths(root):
    install_folders = [
        node for node in root.findall(".//w:Directory", NS)
        if node.get("Id") == "INSTALLFOLDER"
    ]
    if len(install_folders) != 1 or install_folders[0].get("Name") != "Fabulor":
        raise ProductionGtk4MsiError("Expected the production Fabulor install root")

    paths = set()

    def walk(directory, prefix=pathlib.PurePosixPath()):
        for component in directory.findall("w:Component", NS):
            for file_node in component.findall("w:File", NS):
                name = file_node.get("Name")
                if not name or any(separator in name for separator in ("/", "\\")):
                    raise ProductionGtk4MsiError("Invalid installed file name")
                relative = (prefix / name).as_posix()
                if relative in paths:
                    raise ProductionGtk4MsiError(
                        f"Duplicate installed path: {relative}"
                    )
                paths.add(relative)
        for child in directory.findall("w:Directory", NS):
            name = child.get("Name")
            if not name or any(separator in name for separator in ("/", "\\")):
                raise ProductionGtk4MsiError("Invalid installed directory name")
            walk(child, prefix / name)

    walk(install_folders[0])
    return paths


def validate_paths(paths):
    folded = {path.casefold(): path for path in paths}
    missing = sorted(
        path for path in REQUIRED_PATHS if path.casefold() not in folded
    )
    forbidden = sorted(
        path for path in paths
        if any(marker in path.casefold() for marker in FORBIDDEN_MARKERS)
    )
    if missing or forbidden:
        raise ProductionGtk4MsiError(
            f"Production payload mismatch; missing={missing}, "
            f"legacy_gtk={forbidden[:20]}"
        )


def validate_features(root):
    features = {
        feature.get("Id") for feature in root.findall(".//w:Feature", NS)
    }
    missing = sorted(REQUIRED_FEATURES.difference(features))
    if missing:
        raise ProductionGtk4MsiError(
            f"Production feature contract is incomplete: {missing}"
        )
    if "LegacyGtkCompatibilityFeature" in features:
        raise ProductionGtk4MsiError("Legacy GTK feature remains in production MSI")


def parse_args(argv):
    parser = argparse.ArgumentParser(
        description="Validate the production Fabulor GTK4 MSI identity and payload"
    )
    parser.add_argument("--wix", type=pathlib.Path, required=True)
    parser.add_argument("--msi", type=pathlib.Path, required=True)
    return parser.parse_args(argv)


def main(argv=None):
    args = parse_args(argv)
    try:
        wix = args.wix.resolve(strict=True)
        msi = args.msi.resolve(strict=True)
        temporary_root = msi.parent / "production-gtk4-msi-validation"
        temporary_root.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=temporary_root) as temporary:
            decompiled = pathlib.Path(temporary) / "product.wxs"
            extraction_root = pathlib.Path(temporary) / "files"
            validate_runtime_msi.decompile_msi(
                wix, msi, decompiled, extraction_root
            )
            root = ET.parse(decompiled).getroot()
            validate_identity(root)
            paths = collect_paths(root)
            validate_paths(paths)
            validate_features(root)
    except (OSError, ET.ParseError, validate_runtime_msi.CandidateMsiError,
            ProductionGtk4MsiError) as exc:
        print(f"Production GTK4 MSI validation failed: {exc}", file=sys.stderr)
        return 1

    print(
        "Production GTK4 MSI validated: "
        f"installed_files={len(paths)}, legacy_gtk_files=0, identity=Fabulor"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
