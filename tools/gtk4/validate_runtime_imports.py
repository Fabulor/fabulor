#!/usr/bin/env python3

import argparse
import json
import pathlib
import re
import shutil
import subprocess
import sys


DEPENDENCY_PATTERN = re.compile(r"^\s{4}([^\\/\s]+\.dll)\s*$", re.IGNORECASE)


class RuntimeImportError(Exception):
    pass


def load_contract(path):
    try:
        contract = json.loads(path.read_text(encoding="utf-8"))
        schema_version = contract["schema_version"]
        roots = contract["roots"]
        system_imports = contract["system_imports"]
        system_prefixes = contract["system_import_prefixes"]
        forbidden_prefixes = contract["forbidden_import_prefixes"]
    except (OSError, UnicodeDecodeError, json.JSONDecodeError, KeyError) as exc:
        raise RuntimeImportError(f"Unable to read import contract: {exc}") from exc

    if schema_version != 1 or not isinstance(roots, list):
        raise RuntimeImportError("Unsupported import contract schema")

    parsed_roots = {}
    for entry in roots:
        if not isinstance(entry, dict):
            raise RuntimeImportError("Import contract contains an invalid root")
        path_value = entry.get("path")
        category = entry.get("category")
        if (not isinstance(path_value, str) or not isinstance(category, str) or
                not category):
            raise RuntimeImportError("Import root path and category must be strings")
        relative = pathlib.PurePosixPath(path_value)
        if (relative.is_absolute() or ".." in relative.parts or
                "\\" in path_value or not relative.parts):
            raise RuntimeImportError(f"Import contract contains an unsafe root: {path_value}")
        normalized = relative.as_posix()
        if normalized in parsed_roots:
            raise RuntimeImportError(f"Import contract contains a duplicate root: {normalized}")
        parsed_roots[normalized] = category

    def normalized_strings(values, label):
        if (not isinstance(values, list) or
                any(not isinstance(value, str) or not value for value in values)):
            raise RuntimeImportError(f"Import contract contains invalid {label}")
        lowered = [value.casefold() for value in values]
        if len(lowered) != len(set(lowered)):
            raise RuntimeImportError(f"Import contract contains duplicate {label}")
        return tuple(lowered)

    return {
        "roots": parsed_roots,
        "system_imports": set(normalized_strings(system_imports, "system imports")),
        "system_prefixes": normalized_strings(system_prefixes, "system prefixes"),
        "forbidden_prefixes": normalized_strings(
            forbidden_prefixes, "forbidden prefixes"
        ),
    }


def discover_native_files(runtime_root):
    files = {}
    for path in sorted(runtime_root.rglob("*")):
        if not path.is_file() or path.suffix.casefold() not in (".dll", ".exe"):
            continue
        relative = path.relative_to(runtime_root).as_posix()
        module_name = path.name.casefold()
        if module_name in files:
            raise RuntimeImportError(
                f"Duplicate packaged module basename {path.name}: "
                f"{files[module_name][0]} and {relative}"
            )
        files[module_name] = (relative, path)
    if not files:
        raise RuntimeImportError("Runtime candidate contains no native files")
    return files


def parse_dumpbin_dependencies(output):
    return {
        match.group(1).casefold()
        for line in output.splitlines()
        if (match := DEPENDENCY_PATTERN.match(line)) is not None
    }


def inspect_dependencies(dumpbin, path):
    try:
        completed = subprocess.run(
            [str(dumpbin), "/nologo", "/dependents", str(path)],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            encoding="utf-8",
            errors="replace",
        )
    except OSError as exc:
        raise RuntimeImportError(f"Unable to run dumpbin: {exc}") from exc
    if completed.returncode != 0:
        raise RuntimeImportError(
            f"dumpbin failed for {path} with exit code {completed.returncode}:\n"
            f"{completed.stdout}"
        )
    dependencies = parse_dumpbin_dependencies(completed.stdout)
    if not dependencies:
        raise RuntimeImportError(f"dumpbin reported no dependencies for {path}")
    return dependencies


def validate_graph(files, imports, contract):
    if set(imports) != set(files):
        raise RuntimeImportError("Import inspection does not match packaged native files")
    relative_to_module = {relative: module for module, (relative, _) in files.items()}
    missing_roots = sorted(set(contract["roots"]).difference(relative_to_module))
    if missing_roots:
        raise RuntimeImportError(f"Import roots are missing: {missing_roots}")

    graph = {module: set() for module in files}
    system_seen = set()
    for module, dependencies in imports.items():
        relative = files[module][0]
        for dependency in dependencies:
            if any(dependency.startswith(prefix) for prefix in contract["forbidden_prefixes"]):
                raise RuntimeImportError(
                    f"Forbidden legacy module imported by {relative}: {dependency}"
                )
            if dependency in files:
                graph[module].add(dependency)
                continue
            if (dependency in contract["system_imports"] or
                    any(dependency.startswith(prefix)
                        for prefix in contract["system_prefixes"])):
                system_seen.add(dependency)
                continue
            raise RuntimeImportError(
                f"Unresolved non-system import from {relative}: {dependency}"
            )

    owned = set()
    pending = [relative_to_module[root] for root in contract["roots"]]
    while pending:
        module = pending.pop()
        if module in owned:
            continue
        owned.add(module)
        pending.extend(graph[module].difference(owned))

    unowned = sorted(files[module][0] for module in set(files).difference(owned))
    if unowned:
        raise RuntimeImportError(f"Packaged native files have no import root owner: {unowned}")

    edge_count = sum(len(dependencies) for dependencies in graph.values())
    return edge_count, system_seen


def validate_runtime_imports(runtime_root, dumpbin, contract_path):
    contract = load_contract(contract_path)
    files = discover_native_files(runtime_root)
    imports = {
        module: inspect_dependencies(dumpbin, path)
        for module, (_, path) in files.items()
    }
    edge_count, system_seen = validate_graph(files, imports, contract)
    return len(files), len(contract["roots"]), edge_count, len(system_seen)


def parse_args(argv):
    parser = argparse.ArgumentParser(
        description="Validate the packaged GTK4 native import closure"
    )
    parser.add_argument("--root", type=pathlib.Path, required=True)
    parser.add_argument("--dumpbin", required=True)
    parser.add_argument(
        "--contract",
        type=pathlib.Path,
        default=pathlib.Path(__file__).with_name("runtime-import-contract.json"),
    )
    return parser.parse_args(argv)


def resolve_tool(value):
    candidate = pathlib.Path(value)
    if candidate.is_absolute() or candidate.parent != pathlib.Path("."):
        return candidate.resolve(strict=True)
    discovered = shutil.which(value)
    if discovered is None:
        raise RuntimeImportError(f"Unable to find PE inspection tool: {value}")
    return pathlib.Path(discovered).resolve(strict=True)


def main(argv=None):
    args = parse_args(argv)
    try:
        runtime_root = args.root.resolve(strict=True)
        dumpbin = resolve_tool(args.dumpbin)
        contract = args.contract.resolve(strict=True)
        native_count, root_count, edge_count, system_count = validate_runtime_imports(
            runtime_root, dumpbin, contract
        )
    except (OSError, RuntimeImportError) as exc:
        print(f"GTK4 runtime import validation failed: {exc}", file=sys.stderr)
        return 1

    print(
        "GTK4 runtime imports validated: "
        f"native_files={native_count}, roots={root_count}, "
        f"packaged_edges={edge_count}, system_imports={system_count}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
