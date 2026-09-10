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
        self.assertNotIn(
            'g_signal_connect (G_OBJECT (gui->hpane_right), "size-allocate",',
            source,
        )
        for signal in (
            '"notify::maximized"',
            '"notify::fullscreened"',
            '"notify::scale-factor"',
        ):
            self.assertIn(signal, source)
        layout_callback = source.split(
            "mg_rightpane_window_layout_cb", 1
        )[1].split("mg_add_pane_signals", 1)[0]
        self.assertIn("if (!prefs.hex_gui_ulist_resizable)", layout_callback)
        self.assertIn("mg_schedule_rightpane_restore (gui);", layout_callback)

        populate = source.split("mg_populate (session *sess)", 1)[1].split(
            "mg_bring_tofront_sess", 1
        )[0]
        model_attach = populate.index("mg_populate_userlist (sess);")
        locked_restore = populate.index(
            "if (!prefs.hex_gui_ulist_resizable)", model_attach
        )
        self.assertGreater(locked_restore, model_attach)
        self.assertIn(
            "mg_schedule_rightpane_restore (gui);",
            populate[locked_restore:],
        )

        schedule = source.split(
            "mg_schedule_rightpane_restore (session_gui *gui)", 1
        )[1].split("mg_rightpane_window_layout_cb", 1)[0]
        immediate_restore = schedule.index("mg_restore_rightpane (")
        tick_registration = schedule.index("gtk_widget_add_tick_callback")
        self.assertLess(immediate_restore, tick_registration)


if __name__ == "__main__":
    unittest.main()
