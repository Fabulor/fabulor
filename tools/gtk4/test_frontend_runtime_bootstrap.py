#!/usr/bin/env python3

import pathlib
import unittest
import xml.etree.ElementTree as ET

import validate_frontend_bootstrap


REPOSITORY_ROOT = pathlib.Path(__file__).resolve().parents[2]
PROPS_PATH = REPOSITORY_ROOT / "win32" / "fabulor.props"
PROJECT_PATH = REPOSITORY_ROOT / "src" / "fe-gtk" / "fe-gtk.vcxproj"
MAIN_PATH = REPOSITORY_ROOT / "src" / "common" / "zoitechat.c"
LAUNCHER_PATH = REPOSITORY_ROOT / "tools" / "gtk4" / "gtk4-launcher.c"
LAUNCHER_PROJECT_PATH = REPOSITORY_ROOT / "tools" / "gtk4" / "gtk4-launcher.vcxproj"
MSBUILD_NAMESPACE = {"msbuild": "http://schemas.microsoft.com/developer/msbuild/2003"}


class FrontendRuntimeBootstrapTests(unittest.TestCase):
    def test_gtk4_profile_enables_bootstrap(self):
        tree = ET.parse(PROPS_PATH)
        definitions = tree.findall(".//msbuild:OwnFlags", MSBUILD_NAMESPACE)
        self.assertEqual(len(definitions), 1)
        self.assertIsNone(definitions[0].get("Condition"))
        self.assertIn("FABULOR_GTK4_FRONTEND_MODULE", definitions[0].text or "")

    def test_gtk4_frontend_is_a_separate_module(self):
        tree = ET.parse(PROJECT_PATH)
        configurations = tree.findall(
            ".//msbuild:ConfigurationType", MSBUILD_NAMESPACE
        )
        targets = tree.findall(".//msbuild:TargetName", MSBUILD_NAMESPACE)
        self.assertEqual([node.text for node in configurations], ["DynamicLibrary"])
        self.assertEqual([node.text for node in targets], ["fabulor-gtk4-frontend"])

    def test_frontend_exports_launcher_entry(self):
        source = MAIN_PATH.read_text(encoding="utf-8")
        self.assertIn("__declspec(dllexport) int\nfabulor_frontend_main", source)

    def test_launcher_configures_runtime_before_loading_frontend(self):
        source = LAUNCHER_PATH.read_text(encoding="utf-8")
        configure = source.index("fabulor_win32_configure_gtk4_runtime")
        load_frontend = source.index("LoadLibraryExW")
        self.assertLess(configure, load_frontend)
        self.assertNotIn("#include <gtk", source.casefold())
        self.assertNotIn("#include <glib", source.casefold())

        tree = ET.parse(LAUNCHER_PROJECT_PATH)
        compile_paths = {
            node.get("Include", "").replace("/", "\\").casefold()
            for node in tree.findall(".//msbuild:ClCompile", MSBUILD_NAMESPACE)
            if node.get("Include")
        }
        self.assertEqual(
            compile_paths,
            {"gtk4-launcher.c", "..\\..\\src\\common\\win32-gtk4-runtime.c"},
        )

    def test_binary_import_boundary_accepts_owned_modules(self):
        contract = {
            "system_imports": {"kernel32.dll"},
            "system_prefixes": ("api-ms-win-",),
            "forbidden_prefixes": ("gtk-3", "libgtk-"),
        }
        validate_frontend_bootstrap.validate_import_sets(
            {"kernel32.dll"},
            {"gtk-4-1.dll", "glib-2.0-0.dll", "libssl-3-x64.dll"},
            {"gtk-4-1.dll", "glib-2.0-0.dll"},
            contract,
        )

    def test_binary_import_boundary_rejects_launcher_runtime_import(self):
        contract = {
            "system_imports": {"kernel32.dll"},
            "system_prefixes": ("api-ms-win-",),
            "forbidden_prefixes": ("gtk-3",),
        }
        with self.assertRaises(validate_frontend_bootstrap.FrontendBootstrapError):
            validate_frontend_bootstrap.validate_import_sets(
                {"kernel32.dll", "glib-2.0-0.dll"},
                {"gtk-4-1.dll"},
                {"gtk-4-1.dll", "glib-2.0-0.dll"},
                contract,
            )


if __name__ == "__main__":
    unittest.main()
