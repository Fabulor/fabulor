#!/usr/bin/env python3

import argparse
import hashlib
import pathlib
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET


BA_NAMESPACE = "http://wixtoolset.org/schemas/v4/BootstrapperApplicationData"
BURN_NAMESPACE = "http://wixtoolset.org/schemas/v4/2008/Burn"
BA_NS = {"ba": BA_NAMESPACE}
BURN_NS = {"burn": BURN_NAMESPACE}

BUNDLE_ID = "Fabulor.Setup.Bundle"
BUNDLE_NAME = "Fabulor Setup"
BUNDLE_UPGRADE_CODE = "D9F4A5C2-7F3B-4F9E-9A21-3C8F6B7E4A10"
MSI_PACKAGE_ID = "FabulorMsi"
MSI_NAME = "Fabulor"
MSI_UPGRADE_CODE = "8F6C0C7E-9A4D-4E4C-9F8C-2B6F5A4E9C11"
EXPECTED_BA_FILES = {
    "BootstrapperApplicationData.xml",
    "BootstrapperExtensionData.xml",
    "Fabulor.BA.deps.json",
    "Fabulor.BA.dll",
    "Fabulor.BA.exe",
    "Fabulor.BA.runtimeconfig.json",
    "WixBundle.ico",
    "WixToolset.BootstrapperApplicationApi.dll",
    "manifest.xml",
    "mbanative.dll",
}
EXPECTED_CHAIN_FILES = {"WixAttachedContainer/Fabulor.msi"}


class ProductionBundleError(Exception):
    pass


def _exactly_one(nodes, description):
    if len(nodes) != 1:
        raise ProductionBundleError(
            f"Expected one {description}, found {len(nodes)}"
        )
    return nodes[0]


def _guid(value):
    return (value or "").strip().strip("{}").upper()


def _sha256_file(path):
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def load_expected_version(props_path):
    try:
        root = ET.parse(props_path).getroot()
    except (OSError, ET.ParseError) as exc:
        raise ProductionBundleError(
            f"Unable to read installer version properties: {exc}"
        ) from exc
    versions = root.findall(".//FabulorSemVer")
    version = _exactly_one(versions, "FabulorSemVer").text
    if not version or not version.strip():
        raise ProductionBundleError("FabulorSemVer is empty")
    return version.strip()


def extract_bundle(wix, bundle, chain_root, ba_root):
    command = [
        str(wix),
        "burn",
        "extract",
        "-acceptEula",
        "wix7",
        str(bundle),
        "-o",
        str(chain_root),
        "-oba",
        str(ba_root),
    ]
    try:
        completed = subprocess.run(
            command,
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            encoding="utf-8",
            errors="replace",
        )
    except OSError as exc:
        raise ProductionBundleError(
            f"Unable to run WiX bundle extraction: {exc}"
        ) from exc
    if completed.returncode != 0:
        raise ProductionBundleError(
            "WiX bundle extraction failed with exit code "
            f"{completed.returncode}:\n{completed.stdout}"
        )


def validate_bootstrapper_data(root, expected_version):
    properties = _exactly_one(
        root.findall("ba:WixBundleProperties", BA_NS),
        "bootstrapper property record",
    )
    if properties.get("BundleId") != BUNDLE_ID:
        raise ProductionBundleError(
            f"Unexpected bundle ID: {properties.get('BundleId')!r}"
        )
    if properties.get("DisplayName") != BUNDLE_NAME:
        raise ProductionBundleError(
            f"Unexpected bundle name: {properties.get('DisplayName')!r}"
        )
    if _guid(properties.get("UpgradeCode")) != BUNDLE_UPGRADE_CODE:
        raise ProductionBundleError(
            f"Unexpected bundle upgrade code: {properties.get('UpgradeCode')!r}"
        )
    if properties.get("Scope") != "perMachine":
        raise ProductionBundleError("Production bundle is not per-machine")

    package = _exactly_one(
        root.findall("ba:WixPackageProperties", BA_NS),
        "bootstrapper package record",
    )
    expected_package = {
        "Package": MSI_PACKAGE_ID,
        "DisplayName": MSI_NAME,
        "PackageType": "Msi",
        "Compressed": "yes",
        "Version": expected_version,
    }
    for attribute, expected in expected_package.items():
        if package.get(attribute) != expected:
            raise ProductionBundleError(
                f"Unexpected MSI package {attribute}: {package.get(attribute)!r}"
            )
    if _guid(package.get("UpgradeCode")) != MSI_UPGRADE_CODE:
        raise ProductionBundleError(
            f"Unexpected MSI upgrade code: {package.get('UpgradeCode')!r}"
        )

    payload = _exactly_one(
        root.findall("ba:WixPayloadProperties", BA_NS),
        "bootstrapper package payload",
    )
    if (
        payload.get("Package") != MSI_PACKAGE_ID
        or payload.get("Payload") != MSI_PACKAGE_ID
        or payload.get("Container") != "WixAttachedContainer"
        or payload.get("Name") != "Fabulor.msi"
    ):
        raise ProductionBundleError("Unexpected embedded MSI payload relationship")
    try:
        payload_size = int(payload.get("Size", ""))
        package_size = int(package.get("PackageSize", ""))
    except ValueError as exc:
        raise ProductionBundleError("Invalid embedded MSI size metadata") from exc
    if payload_size <= 0 or payload_size != package_size:
        raise ProductionBundleError("Embedded MSI size metadata is inconsistent")

    return package.get("ProductCode"), payload_size


def validate_burn_manifest(root, expected_version, product_code):
    registration = _exactly_one(
        root.findall("burn:Registration", BURN_NS),
        "Burn registration",
    )
    expected_registration = {
        "BundleId": BUNDLE_ID,
        "Version": expected_version,
        "Scope": "perMachine",
    }
    for attribute, expected in expected_registration.items():
        if registration.get(attribute) != expected:
            raise ProductionBundleError(
                f"Unexpected Burn registration {attribute}: "
                f"{registration.get(attribute)!r}"
            )
    if _guid(registration.get("PrimaryUpgradeCode")) != BUNDLE_UPGRADE_CODE:
        raise ProductionBundleError("Burn registration upgrade identity changed")

    related = _exactly_one(
        root.findall("burn:RelatedBundle", BURN_NS),
        "related-bundle upgrade record",
    )
    if (
        _guid(related.get("Code")) != BUNDLE_UPGRADE_CODE
        or related.get("Action") != "Upgrade"
    ):
        raise ProductionBundleError("Bundle upgrade relationship is invalid")

    package = _exactly_one(
        root.findall("burn:Chain/burn:MsiPackage", BURN_NS),
        "Burn MSI chain package",
    )
    if (
        package.get("Id") != MSI_PACKAGE_ID
        or package.get("ProductCode") != product_code
        or package.get("Version") != expected_version
        or _guid(package.get("UpgradeCode")) != MSI_UPGRADE_CODE
        or package.get("Scope") != "perMachine"
    ):
        raise ProductionBundleError("Burn MSI chain identity is inconsistent")
    payload_refs = package.findall("burn:PayloadRef", BURN_NS)
    if (
        len(payload_refs) != 1
        or payload_refs[0].get("Id") != MSI_PACKAGE_ID
    ):
        raise ProductionBundleError("Burn MSI chain payload reference is invalid")

    payload = _exactly_one(
        [
            node
            for node in root.findall("burn:Payload", BURN_NS)
            if node.get("Id") == MSI_PACKAGE_ID
        ],
        "Burn embedded MSI payload",
    )
    if (
        payload.get("FilePath") != "Fabulor.msi"
        or payload.get("Packaging") != "embedded"
        or payload.get("Container") != "WixAttachedContainer"
    ):
        raise ProductionBundleError("Burn MSI payload is not embedded as expected")


def _relative_files(root):
    return {
        path.relative_to(root).as_posix()
        for path in root.rglob("*")
        if path.is_file()
    }


def validate_extracted_files(ba_root, chain_root):
    actual_ba = _relative_files(ba_root)
    actual_chain = _relative_files(chain_root)
    if actual_ba != EXPECTED_BA_FILES:
        raise ProductionBundleError(
            "Bootstrapper application payload differs; "
            f"missing={sorted(EXPECTED_BA_FILES - actual_ba)}, "
            f"unexpected={sorted(actual_ba - EXPECTED_BA_FILES)}"
        )
    if actual_chain != EXPECTED_CHAIN_FILES:
        raise ProductionBundleError(
            "Bundle chain payload differs; "
            f"missing={sorted(EXPECTED_CHAIN_FILES - actual_chain)}, "
            f"unexpected={sorted(actual_chain - EXPECTED_CHAIN_FILES)}"
        )


def validate_embedded_msi(msi, embedded_msi, declared_size):
    try:
        published_size = msi.stat().st_size
        embedded_size = embedded_msi.stat().st_size
        published_hash = _sha256_file(msi)
        embedded_hash = _sha256_file(embedded_msi)
    except OSError as exc:
        raise ProductionBundleError(f"Unable to compare release MSI files: {exc}") from exc
    if (
        published_size != declared_size
        or embedded_size != declared_size
        or published_hash != embedded_hash
    ):
        raise ProductionBundleError(
            "Embedded MSI does not match the separately published Fabulor.msi"
        )
    return published_hash


def parse_args(argv):
    parser = argparse.ArgumentParser(
        description="Validate the production Fabulor bootstrapper and MSI pair"
    )
    parser.add_argument("--wix", type=pathlib.Path, required=True)
    parser.add_argument("--bundle", type=pathlib.Path, required=True)
    parser.add_argument("--msi", type=pathlib.Path, required=True)
    parser.add_argument("--version-props", type=pathlib.Path, required=True)
    return parser.parse_args(argv)


def main(argv=None):
    args = parse_args(argv)
    try:
        wix = args.wix.resolve(strict=True)
        bundle = args.bundle.resolve(strict=True)
        msi = args.msi.resolve(strict=True)
        version_props = args.version_props.resolve(strict=True)
        expected_version = load_expected_version(version_props)
        temporary_root = bundle.parent / "production-bundle-validation"
        temporary_root.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=temporary_root) as temporary:
            temporary = pathlib.Path(temporary)
            chain_root = temporary / "chain"
            ba_root = temporary / "ba"
            extract_bundle(wix, bundle, chain_root, ba_root)
            validate_extracted_files(ba_root, chain_root)
            ba_data = ET.parse(
                ba_root / "BootstrapperApplicationData.xml"
            ).getroot()
            product_code, declared_size = validate_bootstrapper_data(
                ba_data, expected_version
            )
            manifest = ET.parse(ba_root / "manifest.xml").getroot()
            validate_burn_manifest(manifest, expected_version, product_code)
            digest = validate_embedded_msi(
                msi,
                chain_root / "WixAttachedContainer" / "Fabulor.msi",
                declared_size,
            )
    except (OSError, ET.ParseError, ProductionBundleError) as exc:
        print(f"Production bundle validation failed: {exc}", file=sys.stderr)
        return 1

    print(
        "Production bundle validated: "
        f"version={expected_version}, chain_packages=1, "
        f"embedded_msi_sha256={digest}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
