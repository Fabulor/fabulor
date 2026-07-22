#!/usr/bin/env python3

import pathlib
import sys
import unittest


sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import validate_native_extensions as extensions


def contract(required=None):
    return {
        "modules": [{
            "path": "plugins/example.dll",
            "source": "plugins",
            "required_imports": set(required or ()),
        }],
        "data_files": [],
        "application_imports": {"libcrypto-3-x64.dll"},
    }


SYSTEM = {
    "system_imports": {"kernel32.dll"},
    "system_prefixes": ("api-ms-win-",),
    "forbidden_prefixes": ("gtk-3", "gdk-3"),
}


class NativeExtensionTests(unittest.TestCase):
    def test_owned_runtime_application_and_system_imports_pass(self):
        edges = extensions.validate_import_graph(
            contract({"gtk-4-1.dll"}),
            {"example.dll": {
                "gtk-4-1.dll", "libcrypto-3-x64.dll", "kernel32.dll"
            }},
            {"gtk-4-1.dll"}, SYSTEM,
        )
        self.assertEqual(edges, 2)

    def test_gtk3_import_is_rejected(self):
        with self.assertRaisesRegex(extensions.NativeExtensionError, "legacy"):
            extensions.validate_import_graph(
                contract(), {"example.dll": {"gtk-3-vs17.dll"}}, set(), SYSTEM
            )

    def test_unresolved_import_is_rejected(self):
        with self.assertRaisesRegex(extensions.NativeExtensionError, "unresolved"):
            extensions.validate_import_graph(
                contract(), {"example.dll": {"mystery.dll"}}, set(), SYSTEM
            )

    def test_missing_required_import_is_rejected(self):
        with self.assertRaisesRegex(extensions.NativeExtensionError, "required"):
            extensions.validate_import_graph(
                contract({"gtk-4-1.dll"}),
                {"example.dll": {"kernel32.dll"}}, {"gtk-4-1.dll"}, SYSTEM,
            )


if __name__ == "__main__":
    unittest.main()
