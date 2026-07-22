#!/usr/bin/env python3

import argparse
import json
import pathlib
import sys

import validate_runtime_imports


class NativeExtensionError(Exception):
    pass


def _safe_relative(value, label):
    if not isinstance(value, str):
        raise NativeExtensionError(f"{label} must be a string")
    relative = pathlib.PurePosixPath(value)
    if (not value or relative.is_absolute() or ".." in relative.parts or
            "\\" in value):
        raise NativeExtensionError(f"{label} contains an unsafe path: {value!r}")
    return relative.as_posix()


def load_contract(path):
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
        modules = value["modules"]
        data_files = value["data_files"]
        application_imports = value["application_imports"]
    except (OSError, UnicodeDecodeError, json.JSONDecodeError, KeyError) as exc:
        raise NativeExtensionError(f"Unable to read extension contract: {exc}") from exc
    if value.get("schema_version") != 1:
        raise NativeExtensionError("Unsupported extension contract schema")
    if not isinstance(modules, list) or not isinstance(data_files, list):
        raise NativeExtensionError("Extension contract file lists are invalid")

    parsed_modules = []
    paths = set()
    basenames = set()
    for module in modules:
        if not isinstance(module, dict):
            raise NativeExtensionError("Extension contract module is invalid")
        relative = _safe_relative(module.get("path"), "Module path")
        source = module.get("source")
        imports = module.get("required_imports")
        if source not in ("plugins", "payload", "enchant"):
            raise NativeExtensionError(f"Invalid source owner for {relative}")
        if (not isinstance(imports, list) or
                any(not isinstance(item, str) or not item for item in imports)):
            raise NativeExtensionError(f"Invalid required imports for {relative}")
        normalized_imports = tuple(item.casefold() for item in imports)
        basename = pathlib.PurePosixPath(relative).name.casefold()
        if relative in paths or basename in basenames:
            raise NativeExtensionError(f"Duplicate extension module: {relative}")
        if len(normalized_imports) != len(set(normalized_imports)):
            raise NativeExtensionError(f"Duplicate required import for {relative}")
        paths.add(relative)
        basenames.add(basename)
        parsed_modules.append({
            "path": relative,
            "source": source,
            "required_imports": set(normalized_imports),
        })

    parsed_data = []
    for entry in data_files:
        if not isinstance(entry, dict):
            raise NativeExtensionError("Extension contract data file is invalid")
        relative = _safe_relative(entry.get("path"), "Data path")
        source = entry.get("source")
        if source not in ("plugins", "payload", "enchant") or relative in paths:
            raise NativeExtensionError(f"Invalid data file contract: {relative}")
        paths.add(relative)
        parsed_data.append({"path": relative, "source": source})

    if (not isinstance(application_imports, list) or
            any(not isinstance(item, str) or not item
                for item in application_imports)):
        raise NativeExtensionError("Extension application imports are invalid")
    normalized_application = {item.casefold() for item in application_imports}
    if len(normalized_application) != len(application_imports):
        raise NativeExtensionError("Extension application imports contain duplicates")
    return {
        "modules": parsed_modules,
        "data_files": parsed_data,
        "application_imports": normalized_application,
    }


def source_path(entry, roots):
    root = roots[entry["source"]]
    relative = pathlib.PurePosixPath(entry["path"])
    if entry["source"] == "plugins":
        relative = pathlib.PurePosixPath(relative.name)
    return root.joinpath(*relative.parts)


def resolve_contract_files(contract, roots):
    resolved = {}
    for entry in contract["modules"] + contract["data_files"]:
        path = source_path(entry, roots)
        if not path.is_file():
            raise NativeExtensionError(
                f"Missing native extension contract file: {entry['path']} ({path})"
            )
        resolved[entry["path"]] = path
    return resolved


def validate_import_graph(contract, imports, runtime_modules, system_contract):
    contract_modules = {
        pathlib.PurePosixPath(entry["path"]).name.casefold(): entry
        for entry in contract["modules"]
    }
    if set(imports) != set(contract_modules):
        raise NativeExtensionError("Inspected modules do not match extension contract")
    known_modules = set(runtime_modules) | set(contract_modules) | set(
        contract["application_imports"]
    )

    def is_system(module):
        return (module in system_contract["system_imports"] or
                any(module.startswith(prefix)
                    for prefix in system_contract["system_prefixes"]))

    edge_count = 0
    for module, dependencies in imports.items():
        entry = contract_modules[module]
        missing_required = sorted(entry["required_imports"].difference(dependencies))
        if missing_required:
            raise NativeExtensionError(
                f"{entry['path']} is missing required imports: {missing_required}"
            )
        for dependency in dependencies:
            if any(dependency.startswith(prefix)
                   for prefix in system_contract["forbidden_prefixes"]):
                raise NativeExtensionError(
                    f"{entry['path']} imports forbidden legacy module {dependency}"
                )
            if dependency in known_modules:
                edge_count += 1
            elif not is_system(dependency):
                raise NativeExtensionError(
                    f"{entry['path']} has unresolved import {dependency}"
                )
    return edge_count


def validate_native_extensions(contract, roots, runtime_root, dumpbin,
                               system_contract):
    resolved = resolve_contract_files(contract, roots)
    runtime_modules = set(validate_runtime_imports.discover_native_files(runtime_root))
    imports = {}
    for entry in contract["modules"]:
        module = pathlib.PurePosixPath(entry["path"]).name.casefold()
        imports[module] = validate_runtime_imports.inspect_dependencies(
            dumpbin, resolved[entry["path"]]
        )
    edge_count = validate_import_graph(
        contract, imports, runtime_modules, system_contract
    )
    return len(imports), len(contract["data_files"]), edge_count


def parse_args(argv):
    parser = argparse.ArgumentParser(
        description="Validate GTK4 candidate native extensions and Enchant"
    )
    parser.add_argument("--plugins-root", type=pathlib.Path, required=True)
    parser.add_argument("--payload-root", type=pathlib.Path, required=True)
    parser.add_argument("--enchant-root", type=pathlib.Path, required=True)
    parser.add_argument("--runtime-root", type=pathlib.Path, required=True)
    parser.add_argument("--dumpbin", required=True)
    parser.add_argument(
        "--contract", type=pathlib.Path,
        default=pathlib.Path(__file__).with_name("native-extension-contract.json"),
    )
    parser.add_argument(
        "--system-contract", type=pathlib.Path,
        default=pathlib.Path(__file__).with_name("runtime-import-contract.json"),
    )
    return parser.parse_args(argv)


def main(argv=None):
    args = parse_args(argv)
    try:
        contract = load_contract(args.contract.resolve(strict=True))
        roots = {
            "plugins": args.plugins_root.resolve(strict=True),
            "payload": args.payload_root.resolve(strict=True),
            "enchant": args.enchant_root.resolve(strict=True),
        }
        runtime_root = args.runtime_root.resolve(strict=True)
        dumpbin = validate_runtime_imports.resolve_tool(args.dumpbin)
        system_contract = validate_runtime_imports.load_contract(
            args.system_contract.resolve(strict=True)
        )
        module_count, data_count, edge_count = validate_native_extensions(
            contract, roots, runtime_root, dumpbin, system_contract
        )
    except (OSError, NativeExtensionError,
            validate_runtime_imports.RuntimeImportError) as exc:
        print(f"GTK4 native extension validation failed: {exc}", file=sys.stderr)
        return 1
    print(
        "GTK4 native extensions validated: "
        f"modules={module_count}, data_files={data_count}, "
        f"owned_import_edges={edge_count}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
