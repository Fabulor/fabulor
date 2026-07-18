#!/usr/bin/env python3

import pathlib
import tempfile
import unittest

import validate_runtime_imports


def contract(roots=None):
    return {
        "roots": roots or {"bin/gtk-4-1.dll": "frontend-runtime"},
        "system_imports": {"kernel32.dll"},
        "system_prefixes": ("api-ms-win-",),
        "forbidden_prefixes": ("gtk-3", "libglib-"),
    }


def files(*paths):
    return {
        pathlib.PurePosixPath(path).name.casefold(): (path, pathlib.Path(path))
        for path in paths
    }


class ValidateRuntimeImportsTests(unittest.TestCase):
    def test_owned_packaged_closure_passes(self):
        native_files = files("bin/gtk-4-1.dll", "bin/glib-2.0-0.dll")
        imports = {
            "gtk-4-1.dll": {"glib-2.0-0.dll", "kernel32.dll"},
            "glib-2.0-0.dll": {"api-ms-win-crt-runtime-l1-1-0.dll"},
        }
        edge_count, system = validate_runtime_imports.validate_graph(
            native_files, imports, contract()
        )
        self.assertEqual(edge_count, 1)
        self.assertEqual(len(system), 2)

    def test_unresolved_import_is_rejected(self):
        native_files = files("bin/gtk-4-1.dll")
        with self.assertRaisesRegex(
                validate_runtime_imports.RuntimeImportError, "Unresolved"):
            validate_runtime_imports.validate_graph(
                native_files, {"gtk-4-1.dll": {"missing.dll"}}, contract()
            )

    def test_forbidden_legacy_family_is_rejected(self):
        native_files = files("bin/gtk-4-1.dll")
        with self.assertRaisesRegex(
                validate_runtime_imports.RuntimeImportError, "Forbidden legacy"):
            validate_runtime_imports.validate_graph(
                native_files, {"gtk-4-1.dll": {"gtk-3-0.dll"}}, contract()
            )

    def test_unowned_packaged_module_is_rejected(self):
        native_files = files("bin/gtk-4-1.dll", "bin/unused.dll")
        imports = {
            "gtk-4-1.dll": {"kernel32.dll"},
            "unused.dll": {"kernel32.dll"},
        }
        with self.assertRaisesRegex(
                validate_runtime_imports.RuntimeImportError, "no import root owner"):
            validate_runtime_imports.validate_graph(native_files, imports, contract())

    def test_duplicate_packaged_basename_is_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = pathlib.Path(temporary)
            (root / "bin").mkdir()
            (root / "lib").mkdir()
            (root / "bin" / "same.dll").touch()
            (root / "lib" / "SAME.DLL").touch()
            with self.assertRaisesRegex(
                    validate_runtime_imports.RuntimeImportError, "Duplicate"):
                validate_runtime_imports.discover_native_files(root)

    def test_missing_root_is_rejected(self):
        native_files = files("bin/glib-2.0-0.dll")
        with self.assertRaisesRegex(
                validate_runtime_imports.RuntimeImportError, "roots are missing"):
            validate_runtime_imports.validate_graph(
                native_files,
                {"glib-2.0-0.dll": {"kernel32.dll"}},
                contract(),
            )

    def test_incomplete_inspection_is_rejected(self):
        native_files = files("bin/gtk-4-1.dll", "bin/glib-2.0-0.dll")
        with self.assertRaisesRegex(
                validate_runtime_imports.RuntimeImportError, "inspection"):
            validate_runtime_imports.validate_graph(
                native_files,
                {"gtk-4-1.dll": {"kernel32.dll"}},
                contract(),
            )

    def test_dumpbin_dependency_parser_is_case_insensitive(self):
        output = """Image has the following dependencies:\n\n    KERNEL32.dll\n    glib-2.0-0.DLL\n\n  Summary\n"""
        self.assertEqual(
            validate_runtime_imports.parse_dumpbin_dependencies(output),
            {"kernel32.dll", "glib-2.0-0.dll"},
        )


if __name__ == "__main__":
    unittest.main()
