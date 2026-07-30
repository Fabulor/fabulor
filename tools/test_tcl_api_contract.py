#!/usr/bin/env python3

import pathlib
import unittest


REPO_ROOT = pathlib.Path(__file__).resolve().parents[1]
HOST_SOURCE = REPO_ROOT / "src" / "common" / "fabulor-plugin-host.c"
CURRENT_TCL_SURFACES = (
    HOST_SOURCE,
    REPO_ROOT / "samples" / "plugins" / "example.tcl.greeter" / "plugin.tcl",
    REPO_ROOT
    / "samples"
    / "plugins"
    / "simple-tcl-greeter"
    / "simple-tcl-greeter.tcl",
    REPO_ROOT / "docs" / "plugins" / "tcl-plugin-guide.md",
    REPO_ROOT / "docs" / "plugins" / "plugin-schema-and-troubleshooting.md",
)
SHARED_MANIFEST_COMMANDS = (
    "log",
    "send_message",
    "get_user_count",
    "get_user_info",
    "register_callback",
)
TRUSTED_SIMPLE_COMMANDS = (
    "log",
    "print",
    "command",
    "add_user_command",
    "remove_user_command",
    "register_command",
    "getinfo",
    "nickcmp",
    "send_message",
    "get_user_count",
    "get_user_info",
)
RETIRED_NAMESPACE = "zoite" "chat::"


class TclApiContractTests(unittest.TestCase):
    def test_current_surfaces_do_not_expose_zoitechat_namespace(self):
        for path in CURRENT_TCL_SURFACES:
            with self.subTest(path=path):
                self.assertNotIn(
                    RETIRED_NAMESPACE,
                    path.read_text(encoding="utf-8"),
                )

    def test_host_registers_complete_fabulor_namespace(self):
        source = HOST_SOURCE.read_text(encoding="utf-8")
        self.assertIn('namespace eval fabulor {}', source)
        for command in set(SHARED_MANIFEST_COMMANDS + TRUSTED_SIMPLE_COMMANDS):
            with self.subTest(command=command):
                self.assertIn(
                    f'"fabulor::{command}"',
                    source,
                )
        self.assertIn("if (state->simple_addon)", source)
        self.assertIn(
            'else\n\t{\n\t\tfabulor_tcl_runtime.create_command '
            '(state->interp, "fabulor::register_callback"',
            source,
        )

    def test_maintained_samples_use_fabulor_namespace(self):
        for path in CURRENT_TCL_SURFACES[1:3]:
            source = path.read_text(encoding="utf-8")
            with self.subTest(path=path):
                self.assertIn("fabulor::", source)


if __name__ == "__main__":
    unittest.main()
