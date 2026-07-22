#!/usr/bin/env python3

import argparse
import pathlib
import re
import subprocess
import sys

import validate_runtime_imports


EXPORT_PATTERN = re.compile(r"^\s+\d+\s+[0-9A-F]+\s+[0-9A-F]+\s+(\S+)", re.IGNORECASE)
FRONTEND_ENTRY = "fabulor_frontend_main"
APPLICATION_IMPORTS = {"libcrypto-3-x64.dll", "libssl-3-x64.dll"}


class FrontendBootstrapError(Exception):
    pass


def inspect_exports(dumpbin, path):
    completed = subprocess.run(
        [str(dumpbin), "/nologo", "/exports", str(path)],
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    if completed.returncode != 0:
        raise FrontendBootstrapError(
            f"dumpbin export inspection failed for {path}:\n{completed.stdout}"
        )
    return {
        match.group(1)
        for line in completed.stdout.splitlines()
        if (match := EXPORT_PATTERN.match(line)) is not None
    }


def validate_import_sets(launcher_imports, frontend_imports, runtime_modules, contract):
    system_imports = contract["system_imports"]
    system_prefixes = contract["system_prefixes"]

    def is_system(module):
        return module in system_imports or any(
            module.startswith(prefix) for prefix in system_prefixes
        )

    launcher_non_system = sorted(
        module for module in launcher_imports if not is_system(module)
    )
    if launcher_non_system:
        raise FrontendBootstrapError(
            f"GTK4 launcher has non-system imports: {launcher_non_system}"
        )

    if "gtk-4-1.dll" not in frontend_imports:
        raise FrontendBootstrapError("GTK4 frontend does not import gtk-4-1.dll")
    forbidden = sorted(
        module
        for module in frontend_imports
        if any(module.startswith(prefix) for prefix in contract["forbidden_prefixes"])
    )
    if forbidden:
        raise FrontendBootstrapError(f"GTK4 frontend has legacy imports: {forbidden}")

    unresolved = sorted(
        module
        for module in frontend_imports
        if module not in runtime_modules
        and module not in APPLICATION_IMPORTS
        and not is_system(module)
    )
    if unresolved:
        raise FrontendBootstrapError(
            f"GTK4 frontend has unresolved candidate imports: {unresolved}"
        )


def parse_args(argv):
    parser = argparse.ArgumentParser(
        description="Validate the GTK4 launcher/frontend bootstrap boundary"
    )
    parser.add_argument("--launcher", type=pathlib.Path, required=True)
    parser.add_argument("--frontend", type=pathlib.Path, required=True)
    parser.add_argument("--runtime-root", type=pathlib.Path, required=True)
    parser.add_argument("--dumpbin", required=True)
    parser.add_argument(
        "--contract",
        type=pathlib.Path,
        default=pathlib.Path(__file__).with_name("runtime-import-contract.json"),
    )
    return parser.parse_args(argv)


def main(argv=None):
    args = parse_args(argv)
    try:
        launcher = args.launcher.resolve(strict=True)
        frontend = args.frontend.resolve(strict=True)
        runtime_root = args.runtime_root.resolve(strict=True)
        dumpbin = validate_runtime_imports.resolve_tool(args.dumpbin)
        contract = validate_runtime_imports.load_contract(args.contract.resolve(strict=True))
        runtime_modules = set(
            validate_runtime_imports.discover_native_files(runtime_root)
        )
        launcher_imports = validate_runtime_imports.inspect_dependencies(dumpbin, launcher)
        frontend_imports = validate_runtime_imports.inspect_dependencies(dumpbin, frontend)
        validate_import_sets(
            launcher_imports, frontend_imports, runtime_modules, contract
        )
        exports = inspect_exports(dumpbin, frontend)
        if FRONTEND_ENTRY not in exports:
            raise FrontendBootstrapError(
                f"GTK4 frontend does not export {FRONTEND_ENTRY}"
            )
    except (OSError, validate_runtime_imports.RuntimeImportError,
            FrontendBootstrapError) as exc:
        print(f"GTK4 frontend bootstrap validation failed: {exc}", file=sys.stderr)
        return 1

    print(
        "GTK4 frontend bootstrap validated: "
        f"launcher_imports={len(launcher_imports)}, "
        f"frontend_imports={len(frontend_imports)}, entry={FRONTEND_ENTRY}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
