#!/usr/bin/env python3

import pathlib
import re
import unittest


REPO_ROOT = pathlib.Path(__file__).resolve().parents[1]
HOST_HEADER = REPO_ROOT / "src" / "common" / "fabulor-plugin-host.h"
HOST_SOURCE = REPO_ROOT / "src" / "common" / "fabulor-plugin-host.c"
PLUGIN_SOURCE = REPO_ROOT / "src" / "common" / "plugin.c"
PYTHON_MANIFEST = REPO_ROOT / "plugins" / "python" / "_fabulor_manifest.py"
CSHARP_CONTEXT = (
    REPO_ROOT
    / "src"
    / "managed"
    / "Fabulor.PluginAbstractions"
    / "FabulorContext.cs"
)
SCHEMA_GUIDE = (
    REPO_ROOT / "docs" / "plugins" / "plugin-schema-and-troubleshooting.md"
)

SHARED_OPERATIONS = {
    "get_user_count",
    "get_user_info",
    "log",
    "register_callback",
    "send_message",
}
SUPPORTED_CAPABILITIES = {
    "events.command",
    "events.message",
    "events.print",
    "events.server",
    "messages.write",
    "session.read",
}
RETIRED_MANIFEST_CAPABILITIES = {
    "commands.execute",
    "commands.manage",
    "events.timer",
    "events.unload",
    "preferences.read",
    "preferences.write",
    "ui.write",
}


class ManifestApiBoundaryTests(unittest.TestCase):
    def test_native_fabulor_api_is_the_compact_shared_surface(self):
        source = HOST_HEADER.read_text(encoding="utf-8")
        api_match = re.search(
            r"typedef struct _fabulor_api\s*\{(?P<body>.*?)\}\s*FabulorAPI;",
            source,
            re.DOTALL,
        )
        self.assertIsNotNone(api_match)
        body = api_match.group("body")
        for operation in ("send_message", "log", "get_user_count", "get_user_info"):
            self.assertRegex(body, rf"\(\*{operation}\)")
        self.assertNotIn("command", body)
        self.assertNotIn("preference", body)

    def test_python_manifest_exports_only_shared_operations(self):
        source = PYTHON_MANIFEST.read_text(encoding="utf-8")
        exported_match = re.search(
            r"module\.__all__\s*=\s*\[(?P<body>.*?)\]",
            source,
            re.DOTALL,
        )
        self.assertIsNotNone(exported_match)
        exports = set(re.findall(r"'([a-z_]+)'", exported_match.group("body")))
        self.assertEqual(exports, SHARED_OPERATIONS)

    def test_csharp_context_exposes_only_shared_operations(self):
        source = CSHARP_CONTEXT.read_text(encoding="utf-8")
        methods = set(
            re.findall(
                r"public\s+(?:void|bool|int|FabulorUserInfo)\s+([A-Z][A-Za-z]+)\s*\(",
                source,
            )
        )
        self.assertEqual(
            methods,
            {
                "GetUserCount",
                "GetUserInfo",
                "Log",
                "RegisterCallback",
                "SendMessage",
            },
        )

    def test_manifest_capability_registry_matches_supported_surface(self):
        source = HOST_SOURCE.read_text(encoding="utf-8")
        known_match = re.search(
            r"fabulor_capability_is_known.*?known\[\]\s*=\s*\{(?P<body>.*?)\};",
            source,
            re.DOTALL,
        )
        self.assertIsNotNone(known_match)
        capabilities = set(re.findall(r'"([a-z]+\.[a-z]+)"', known_match.group("body")))
        self.assertEqual(capabilities, SUPPORTED_CAPABILITIES)

    def test_native_boundary_repeats_input_validation(self):
        header = HOST_HEADER.read_text(encoding="utf-8")
        source = PLUGIN_SOURCE.read_text(encoding="utf-8")
        for limit in (
            "FABULOR_PLUGIN_LOG_TEXT_MAX",
            "FABULOR_PLUGIN_MESSAGE_TARGET_MAX",
            "FABULOR_PLUGIN_MESSAGE_TEXT_MAX",
        ):
            with self.subTest(limit=limit):
                self.assertIn(f"#define {limit}", header)
                self.assertIn(limit, source)
        self.assertGreaterEqual(source.count("g_utf8_validate"), 3)
        self.assertIn("strchr (text, '\\r')", source)
        self.assertIn("strchr (text, '\\n')", source)

    def test_current_schema_does_not_advertise_unreachable_capabilities(self):
        guide = SCHEMA_GUIDE.read_text(encoding="utf-8")
        for capability in RETIRED_MANIFEST_CAPABILITIES:
            with self.subTest(capability=capability):
                self.assertNotIn(f"`{capability}`", guide)

    def test_callback_dispatch_binds_session_and_returns_consumption(self):
        header = HOST_HEADER.read_text(encoding="utf-8")
        source = PLUGIN_SOURCE.read_text(encoding="utf-8")
        self.assertIn("FABULOR_CALLBACK_CONSUME = 1", header)
        self.assertIn("FABULOR_PLUGIN_CALLBACK_RESULTS_API_VERSION 2U", header)
        self.assertIn("fabulor_plugin_api_set_session (sess);", source)
        self.assertIn("|| manifest_consumed", source)


if __name__ == "__main__":
    unittest.main()
