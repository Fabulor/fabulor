#!/usr/bin/env python3

import pathlib
import re
import unittest


REPO_ROOT = pathlib.Path(__file__).resolve().parents[1]
PANE_SOURCES = (
    REPO_ROOT / "src" / "fe-gtk" / "chanview-tree.c",
    REPO_ROOT / "src" / "fe-gtk" / "userlistgui.c",
)


class PaneDragContractTests(unittest.TestCase):
    def test_panes_do_not_start_internal_drags(self):
        for path in PANE_SOURCES:
            with self.subTest(path=path.name):
                source = path.read_text(encoding="utf-8")
                self.assertNotIn(
                    "fabulor_gtk_widget_enable_internal_drag_source",
                    source,
                )

    def test_locked_userlist_uses_configured_nick_width(self):
        source = (REPO_ROOT / "src" / "fe-gtk" / "maingui.c").read_text(
            encoding="utf-8"
        )

        self.assertRegex(
            source,
            re.compile(
                r"if\s*\(\s*!prefs\.hex_gui_ulist_resizable\s*\)\s*"
                r"return\s+mg_userlist_fallback_width\s*\(\s*\)\s*;"
            ),
        )
        self.assertIn(
            "gui->pane_right_size = mg_userlist_restore_width ();",
            source,
        )
        self.assertIn(
            "prefs.hex_gui_pane_right_size = right_size;",
            source,
        )
        self.assertIn(
            "gtk_paned_set_position (pane, locked_position);",
            source,
        )


if __name__ == "__main__":
    unittest.main()
