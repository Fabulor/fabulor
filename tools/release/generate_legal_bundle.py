#!/usr/bin/env python3

import argparse
import hashlib
import json
import pathlib
import re
import shutil
import sys
import uuid
import xml.etree.ElementTree as ET


DEFAULT_MANIFEST = pathlib.Path(__file__).parents[2] / "third-party" / "components.json"
DEFAULT_VERSION_PROPS = pathlib.Path(__file__).parents[2] / "installer" / "Directory.Build.props"
BUILD_ROOT = pathlib.Path(__file__).parents[2] / "build"
LICENCE_ROOT_NAME = "licenses"


class LegalBundleError(Exception):
    pass


def _strict_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise LegalBundleError(f"Duplicate JSON field: {key}")
        result[key] = value
    return result


def _safe_relative(value, field):
    if not isinstance(value, str) or not value:
        raise LegalBundleError(f"{field} values must be non-empty strings")
    path = pathlib.PurePosixPath(value)
    if path.is_absolute() or ".." in path.parts or "\\" in value:
        raise LegalBundleError(f"Unsafe {field} path: {value}")
    return path


def _read_json(path):
    try:
        return json.loads(path.read_text(encoding="utf-8"), object_pairs_hook=_strict_object)
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise LegalBundleError(f"Unable to read component manifest {path}: {exc}") from exc


def load_manifest(path):
    manifest = _read_json(path)
    if not isinstance(manifest, dict) or set(manifest) != {
        "schema_version", "product", "components"
    }:
        raise LegalBundleError("Component manifest must contain schema_version, product, and components")
    if manifest["schema_version"] != 1:
        raise LegalBundleError("Unsupported component manifest schema version")

    product = manifest["product"]
    if not isinstance(product, dict) or set(product) != {
        "name", "type", "homepage", "licence"
    }:
        raise LegalBundleError("Invalid product metadata")
    if product["type"] != "application" or not product["homepage"].startswith("https://"):
        raise LegalBundleError("Product metadata must identify an HTTPS application homepage")

    components = manifest["components"]
    if not isinstance(components, list) or not components:
        raise LegalBundleError("components must be a non-empty array")
    required = {
        "id", "name", "version", "type", "scope", "licence",
        "licence_files", "source", "installed_paths",
    }
    optional = {"artifact_paths", "distribution_sha256"}
    ids = []
    for component in components:
        if not isinstance(component, dict):
            raise LegalBundleError("Every component must be an object")
        keys = set(component)
        if not required.issubset(keys) or not keys.issubset(required | optional):
            raise LegalBundleError(f"Invalid fields for component {component.get('id', '<unknown>')}")
        component_id = component["id"]
        if not isinstance(component_id, str) or not re.fullmatch(r"[a-z0-9][a-z0-9.-]*", component_id):
            raise LegalBundleError(f"Invalid component id: {component_id!r}")
        ids.append(component_id)
        for field in ("name", "version", "licence"):
            if not isinstance(component[field], str) or not component[field].strip():
                raise LegalBundleError(f"{component_id}: {field} must be a non-empty string")
        if component["type"] not in {"data", "framework", "library"}:
            raise LegalBundleError(f"{component_id}: unsupported component type")
        if component["scope"] not in {"required", "optional"}:
            raise LegalBundleError(f"{component_id}: unsupported scope")
        if not isinstance(component["source"], str) or not component["source"].startswith("https://"):
            raise LegalBundleError(f"{component_id}: source must be an HTTPS URL")
        for field in ("licence_files", "installed_paths", "artifact_paths"):
            values = component.get(field, [])
            if not isinstance(values, list) or (field == "licence_files" and not values):
                raise LegalBundleError(f"{component_id}: {field} must be an array")
            normalized = [_safe_relative(value, field).as_posix() for value in values]
            if len(set(value.casefold() for value in normalized)) != len(normalized):
                raise LegalBundleError(f"{component_id}: {field} contains duplicates")
        if not component["installed_paths"] and not component.get("artifact_paths"):
            raise LegalBundleError(
                f"{component_id}: installed_paths or artifact_paths must identify shipped content"
            )
        digest = component.get("distribution_sha256")
        if digest is not None and not re.fullmatch(r"[0-9a-f]{64}", digest):
            raise LegalBundleError(f"{component_id}: invalid distribution_sha256")
    if ids != sorted(ids, key=str.casefold):
        raise LegalBundleError("components must be sorted by id")
    if len(set(ids)) != len(ids):
        raise LegalBundleError("component ids must be unique")
    return manifest


def read_version(path):
    try:
        root = ET.parse(path).getroot()
    except (OSError, ET.ParseError) as exc:
        raise LegalBundleError(f"Unable to read version properties {path}: {exc}") from exc
    values = [node.text.strip() for node in root.iter("FabulorSemVer") if node.text]
    if len(values) != 1 or not re.fullmatch(r"\d+\.\d+\.\d+(?:[-+][0-9A-Za-z.-]+)?", values[0]):
        raise LegalBundleError("Directory.Build.props must contain one valid FabulorSemVer")
    return values[0]


def validate_licences(manifest, licence_root):
    try:
        root = licence_root.resolve(strict=True)
    except OSError as exc:
        raise LegalBundleError(f"Licence root does not exist: {licence_root}") from exc
    evidence = {}
    for component in manifest["components"]:
        for relative in component["licence_files"]:
            path = root.joinpath(*pathlib.PurePosixPath(relative).parts)
            try:
                resolved = path.resolve(strict=True)
                resolved.relative_to(root)
            except (OSError, ValueError) as exc:
                raise LegalBundleError(
                    f"{component['id']}: missing or unsafe licence evidence {relative}"
                ) from exc
            if not resolved.is_file() or resolved.is_symlink():
                raise LegalBundleError(f"{component['id']}: licence evidence is not a regular file")
            data = resolved.read_bytes()
            if not data:
                raise LegalBundleError(f"{component['id']}: empty licence evidence {relative}")
            evidence[relative] = (resolved, hashlib.sha256(data).hexdigest())
    return dict(sorted(evidence.items(), key=lambda item: item[0].casefold()))


def _component_bom_ref(component):
    return f"component:{component['id']}@{component['version']}"


def build_sbom(manifest, product_version, manifest_digest, evidence):
    product_ref = f"pkg:github/Fabulor/fabulor@{product_version}"
    components = []
    for component in manifest["components"]:
        item = {
            "type": component["type"],
            "bom-ref": _component_bom_ref(component),
            "name": component["name"],
            "version": component["version"],
            "scope": component["scope"],
            "licenses": [{"expression": component["licence"]}],
            "externalReferences": [{"type": "vcs", "url": component["source"]}],
            "properties": [
                {"name": "fabulor:component-id", "value": component["id"]},
                {"name": "fabulor:installed-paths", "value": ";".join(component["installed_paths"])},
                {"name": "fabulor:licence-files", "value": ";".join(component["licence_files"])},
            ],
        }
        if component.get("artifact_paths"):
            item["properties"].append({
                "name": "fabulor:artifact-paths",
                "value": ";".join(component["artifact_paths"]),
            })
        if "distribution_sha256" in component:
            item["hashes"] = [{"alg": "SHA-256", "content": component["distribution_sha256"]}]
        components.append(item)

    serial_seed = f"{product_ref}:{manifest_digest}"
    serial = uuid.uuid5(uuid.NAMESPACE_URL, serial_seed)
    return {
        "$schema": "https://cyclonedx.org/schema/bom-1.6.schema.json",
        "bomFormat": "CycloneDX",
        "specVersion": "1.6",
        "serialNumber": f"urn:uuid:{serial}",
        "version": 1,
        "metadata": {
            "component": {
                "type": "application",
                "bom-ref": product_ref,
                "name": manifest["product"]["name"],
                "version": product_version,
                "licenses": [{"expression": manifest["product"]["licence"]}],
                "externalReferences": [
                    {"type": "website", "url": manifest["product"]["homepage"]}
                ],
                "properties": [
                    {"name": "fabulor:component-manifest-sha256", "value": manifest_digest},
                    {"name": "fabulor:licence-evidence-count", "value": str(len(evidence))},
                ],
            }
        },
        "components": components,
        "dependencies": [
            {"ref": product_ref, "dependsOn": [_component_bom_ref(c) for c in manifest["components"]]}
        ],
    }


def build_notices(manifest, product_version, manifest_digest, evidence):
    lines = [
        "# Fabulor Third-Party Notices",
        "",
        f"Release version: `{product_version}`  ",
        f"Component manifest SHA-256: `{manifest_digest}`",
        "",
        "Fabulor redistributes the components below. The accompanying `licenses`",
        "directory contains the verbatim licence and notice evidence identified",
        "for each component. This inventory is generated from the release manifest.",
        "",
        "| Component | Version | Scope | Licence | Shipped location(s) | Evidence |",
        "| --- | --- | --- | --- | --- | --- |",
    ]
    for component in manifest["components"]:
        locations = [f"installed: `{value}`" for value in component["installed_paths"]]
        locations.extend(
            f"release artefact: `{value}`" for value in component.get("artifact_paths", [])
        )
        paths = "<br>".join(locations)
        files = "<br>".join(f"`licenses/{value}`" for value in component["licence_files"])
        lines.append(
            f"| [{component['name']}]({component['source']}) | `{component['version']}` | "
            f"{component['scope']} | `{component['licence']}` | {paths} | {files} |"
        )
    lines.extend(["", "## Evidence Checksums", "", "| File | SHA-256 |", "| --- | --- |"])
    for relative, (_, digest) in evidence.items():
        lines.append(f"| `licenses/{relative}` | `{digest}` |")
    lines.append("")
    return "\n".join(lines)


def generate(manifest_path, version_props, licence_root, output):
    manifest_bytes = manifest_path.read_bytes()
    manifest_digest = hashlib.sha256(manifest_bytes).hexdigest()
    manifest = load_manifest(manifest_path)
    version = read_version(version_props)
    evidence = validate_licences(manifest, licence_root)

    output = output.resolve()
    build_root = BUILD_ROOT.resolve()
    try:
        relative_output = output.relative_to(build_root)
    except ValueError as exc:
        raise LegalBundleError(f"Output must be beneath the repository build directory: {output}") from exc
    if not relative_output.parts:
        raise LegalBundleError("Output cannot be the repository build directory itself")
    if output.exists() and (not output.is_dir() or output.is_symlink()):
        raise LegalBundleError(f"Output is not a regular directory: {output}")
    if output.exists():
        shutil.rmtree(output)
    output.mkdir(parents=True)
    try:
        licence_output = output / LICENCE_ROOT_NAME
        licence_output.mkdir()
        for relative, (source, _) in evidence.items():
            destination = licence_output.joinpath(*pathlib.PurePosixPath(relative).parts)
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, destination)

        sbom = build_sbom(manifest, version, manifest_digest, evidence)
        sbom_name = f"Fabulor-{version}.cdx.json"
        (output / sbom_name).write_text(
            json.dumps(sbom, indent=2, ensure_ascii=True) + "\n", encoding="utf-8"
        )
        (output / "THIRD-PARTY-NOTICES.md").write_text(
            build_notices(manifest, version, manifest_digest, evidence), encoding="utf-8"
        )
    except Exception:
        shutil.rmtree(output, ignore_errors=True)
        raise
    return version, len(manifest["components"]), len(evidence), sbom_name


def parse_args(argv):
    parser = argparse.ArgumentParser(description="Generate Fabulor's installed legal bundle and SBOM")
    parser.add_argument("--manifest", type=pathlib.Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--version-props", type=pathlib.Path, default=DEFAULT_VERSION_PROPS)
    parser.add_argument("--licence-root", type=pathlib.Path)
    parser.add_argument("--output", type=pathlib.Path, required=True)
    args = parser.parse_args(argv)
    if args.licence_root is None:
        args.licence_root = args.manifest.parent / LICENCE_ROOT_NAME
    return args


def main(argv=None):
    args = parse_args(argv)
    try:
        version, components, files, sbom_name = generate(
            args.manifest.resolve(strict=True),
            args.version_props.resolve(strict=True),
            args.licence_root,
            args.output,
        )
        print(
            f"Legal bundle generated: version={version}, components={components}, "
            f"licence_files={files}, sbom={sbom_name}"
        )
        return 0
    except (OSError, LegalBundleError) as exc:
        print(f"Legal bundle generation failed: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
