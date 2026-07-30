#!/usr/bin/env python3

import pathlib
import unittest


REPO_ROOT = pathlib.Path(__file__).resolve().parents[1]
PUBLIC_HEADER = REPO_ROOT / "src" / "common" / "fabulor-plugin.h"
HOST_HEADER = REPO_ROOT / "src" / "common" / "fabulor-plugin-host.h"
VERSION_SCRIPT = REPO_ROOT / "src" / "version-script"
PLUGIN_DEFINITIONS = tuple((REPO_ROOT / "plugins").glob("*/*.def"))
NATIVE_CONSUMERS = (
    REPO_ROOT / "src" / "common" / "plugin.c",
    REPO_ROOT / "plugins" / "python" / "generate_plugin.py",
    REPO_ROOT / "plugins" / "fishlim" / "plugin_fabulor.c",
)
CURRENT_GUIDES = (
    REPO_ROOT / "docs" / "plugins" / "plugin-schema-and-troubleshooting.md",
    REPO_ROOT / "docs" / "cleanup" / "repository-cleanup-plan.md",
)
PUBLIC_FUNCTIONS = (
    "fabulor_hook_command",
    "fabulor_hook_server",
    "fabulor_hook_print",
    "fabulor_hook_timer",
    "fabulor_unhook",
    "fabulor_print",
    "fabulor_command",
    "fabulor_get_info",
    "fabulor_list_get",
    "fabulor_pluginpref_set_str",
)
RETIRED_PREFIX = "zoite" "chat"


class NativePluginApiContractTests(unittest.TestCase):
    def test_public_header_exposes_only_fabulor_contract(self):
        source = PUBLIC_HEADER.read_text(encoding="utf-8")
        for function_name in PUBLIC_FUNCTIONS:
            with self.subTest(function_name=function_name):
                self.assertIn(function_name, source)
        self.assertIn("typedef struct _fabulor_plugin fabulor_plugin;", source)
        self.assertIn("FABULOR_EAT_FABULOR", source)
        self.assertNotIn(f"{RETIRED_PREFIX}_", source.lower())
        self.assertFalse(
            (REPO_ROOT / "src" / "common" / f"{RETIRED_PREFIX}-plugin.h").exists()
        )

    def test_loader_and_bundled_plugins_use_fabulor_entry_points(self):
        loader = NATIVE_CONSUMERS[0].read_text(encoding="utf-8")
        self.assertIn('"fabulor_plugin_init"', loader)
        self.assertIn('"fabulor_plugin_deinit"', loader)

        for definition in PLUGIN_DEFINITIONS:
            source = definition.read_text(encoding="utf-8")
            with self.subTest(definition=definition):
                self.assertIn("fabulor_plugin_init", source)
                self.assertNotIn(f"{RETIRED_PREFIX}_plugin_", source.lower())

    def test_exports_bridges_and_current_guides_have_no_retired_api(self):
        surfaces = (
            HOST_HEADER,
            VERSION_SCRIPT,
            *NATIVE_CONSUMERS,
            *CURRENT_GUIDES,
        )
        for path in surfaces:
            source = path.read_text(encoding="utf-8")
            with self.subTest(path=path):
                self.assertNotIn(f"{RETIRED_PREFIX}_plugin", source.lower())
                self.assertNotIn(f"{RETIRED_PREFIX}api", source.lower())

        exports = VERSION_SCRIPT.read_text(encoding="utf-8")
        for function_name in PUBLIC_FUNCTIONS:
            with self.subTest(function_name=function_name):
                self.assertIn(function_name, exports)


if __name__ == "__main__":
    unittest.main()
