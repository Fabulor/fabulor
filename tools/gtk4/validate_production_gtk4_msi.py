#!/usr/bin/env python3

import argparse
import json
import pathlib
import re
import sys
import tempfile
import xml.etree.ElementTree as ET

import validate_runtime_msi


NS = validate_runtime_msi.NS
PRODUCT_NAME = "Fabulor"
PRODUCT_PUBLISHER = "Fabulor"
UPGRADE_CODE = "8F6C0C7E-9A4D-4E4C-9F8C-2B6F5A4E9C11"
REQUIRED_PATHS = {
    "fabulor.exe",
    "fabulor-gtk4-frontend.dll",
    "readme.url",
    "Plugins/hcchecksum.dll",
    "Plugins/hcexec.dll",
    "Plugins/hcfishlim.dll",
    "Plugins/hcnotifications-winrt.dll",
    "Plugins/hcpython3.dll",
    "Plugins/hcsysinfo.dll",
    "Runtime/plugin-host-manifest.json",
    "Runtime/GTK4/bin/gtk-4-1.dll",
    "share/adwaita-icons-attribution.txt",
    "share/doc/fabulor/Licence.md",
    "share/doc/fabulor/third-party/THIRD-PARTY-NOTICES.md",
    "share/doc/fabulor/third-party/licenses/openssl-copyright",
    "share/doc/NotoColorEmoji/NotoColorEmoji-LICENSE.txt",
    "share/emoji-flags/eu.png",
    "share/fonts/NotoColorEmoji.ttf",
    "share/gtkpref.png",
    "share/music.png",
    "share/palettes/Fabulor Dark.hct",
    "share/system.png",
    "share/xml/iso-codes/iso_3166.xml",
    "share/xml/iso-codes/iso_639.xml",
}
REQUIRED_FEATURES = {
    "MainFeature",
    "StartMenuFeature",
    "DesktopShortcutFeature",
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
FORBIDDEN_PATHS = {
    "plugins/hcupd.dll",
    "winsparkle.dll",
    "share/download.png",
}
FORBIDDEN_FEATURES = {
    "LegacyGtkCompatibilityFeature",
    "UpdatePluginFeature",
}
DEFAULT_COMPONENT_MANIFEST = pathlib.Path(__file__).parents[2] / "third-party" / "components.json"
DEFAULT_VERSION_PROPS = pathlib.Path(__file__).parents[2] / "installer" / "Directory.Build.props"


class ProductionGtk4MsiError(Exception):
    pass


def load_product_version(path):
    try:
        root = ET.parse(path).getroot()
    except (OSError, ET.ParseError) as exc:
        raise ProductionGtk4MsiError(
            f"Unable to read installer version properties {path}: {exc}"
        ) from exc
    versions = [node.text.strip() for node in root.iter("FabulorSemVer") if node.text]
    if len(versions) != 1 or not re.fullmatch(r"\d+\.\d+\.\d+", versions[0]):
        raise ProductionGtk4MsiError(
            "Directory.Build.props must contain one numeric FabulorSemVer"
        )
    return versions[0]


def validate_identity(root, product_version):
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
    if package.get("Manufacturer") != PRODUCT_PUBLISHER:
        raise ProductionGtk4MsiError(
            f"Unexpected package manufacturer: {package.get('Manufacturer')!r}"
        )
    if package.get("Version") != product_version:
        raise ProductionGtk4MsiError(
            f"Unexpected package version: {package.get('Version')!r}"
        )
    upgrade_code = (package.get("UpgradeCode") or "").strip("{}").upper()
    if upgrade_code != UPGRADE_CODE:
        raise ProductionGtk4MsiError(
            f"Unexpected upgrade code: {package.get('UpgradeCode')!r}"
        )
    properties = {
        node.get("Id"): node.get("Value")
        for node in root.findall(".//w:Property", NS)
    }
    if properties.get("ARPCONTACT") != PRODUCT_PUBLISHER:
        raise ProductionGtk4MsiError(
            f"Unexpected ARP contact: {properties.get('ARPCONTACT')!r}"
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


def validate_paths(paths, product_version):
    required_paths = REQUIRED_PATHS | {
        f"share/doc/fabulor/third-party/Fabulor-{product_version}.cdx.json"
    }
    folded = {path.casefold(): path for path in paths}
    missing = sorted(
        path for path in required_paths if path.casefold() not in folded
    )
    forbidden = sorted(
        path for path in paths
        if any(marker in path.casefold() for marker in FORBIDDEN_MARKERS)
    )
    forbidden_paths = sorted(
        folded[path.casefold()]
        for path in FORBIDDEN_PATHS
        if path.casefold() in folded
    )
    if missing or forbidden or forbidden_paths:
        raise ProductionGtk4MsiError(
            f"Production payload mismatch; missing={missing}, "
            f"legacy_gtk={forbidden[:20]}, retired={forbidden_paths}"
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
    forbidden = sorted(FORBIDDEN_FEATURES.intersection(features))
    if forbidden:
        raise ProductionGtk4MsiError(
            f"Retired production features remain in MSI: {forbidden}"
        )


def load_component_inventory(manifest_path):
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        return manifest["components"]
    except (OSError, UnicodeDecodeError, json.JSONDecodeError, KeyError, TypeError) as exc:
        raise ProductionGtk4MsiError(
            f"Unable to read component inventory {manifest_path}: {exc}"
        ) from exc


def validate_component_inventory(paths, manifest_path):
    components = load_component_inventory(manifest_path)
    folded_paths = {path.casefold() for path in paths}
    missing = []
    for component in components:
        component_id = component.get("id", "<unknown>")
        for installed_path in component.get("installed_paths", []):
            candidate = installed_path.replace("\\", "/").strip("/").casefold()
            prefix = candidate + "/"
            if candidate not in folded_paths and not any(
                path.startswith(prefix) for path in folded_paths
            ):
                missing.append(f"{component_id}:{installed_path}")
    if missing:
        raise ProductionGtk4MsiError(
            f"Component inventory paths missing from MSI: {sorted(missing)}"
        )


def validate_component_artifacts(artifacts, manifest_path):
    components = load_component_inventory(manifest_path)
    expected = {
        path.casefold()
        for component in components
        for path in component.get("artifact_paths", [])
    }
    supplied = {path.name.casefold() for path in artifacts}
    missing = sorted(expected.difference(supplied))
    if missing:
        raise ProductionGtk4MsiError(
            f"Component inventory release artefacts were not supplied: {missing}"
        )


def parse_args(argv):
    parser = argparse.ArgumentParser(
        description="Validate the production Fabulor GTK4 MSI identity and payload"
    )
    parser.add_argument("--wix", type=pathlib.Path, required=True)
    parser.add_argument("--msi", type=pathlib.Path, required=True)
    parser.add_argument("--bootstrapper", type=pathlib.Path, required=True)
    parser.add_argument(
        "--component-manifest",
        type=pathlib.Path,
        default=DEFAULT_COMPONENT_MANIFEST,
    )
    parser.add_argument(
        "--version-props",
        type=pathlib.Path,
        default=DEFAULT_VERSION_PROPS,
    )
    return parser.parse_args(argv)


def main(argv=None):
    args = parse_args(argv)
    try:
        wix = args.wix.resolve(strict=True)
        msi = args.msi.resolve(strict=True)
        bootstrapper = args.bootstrapper.resolve(strict=True)
        component_manifest = args.component_manifest.resolve(strict=True)
        version_props = args.version_props.resolve(strict=True)
        product_version = load_product_version(version_props)
        temporary_root = msi.parent / "production-gtk4-msi-validation"
        temporary_root.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=temporary_root) as temporary:
            decompiled = pathlib.Path(temporary) / "product.wxs"
            extraction_root = pathlib.Path(temporary) / "files"
            validate_runtime_msi.decompile_msi(
                wix, msi, decompiled, extraction_root
            )
            root = ET.parse(decompiled).getroot()
            validate_identity(root, product_version)
            paths = collect_paths(root)
            validate_paths(paths, product_version)
            validate_features(root)
            validate_component_inventory(paths, component_manifest)
            validate_component_artifacts([bootstrapper], component_manifest)
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
