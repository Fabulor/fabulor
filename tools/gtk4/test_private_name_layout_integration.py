"""Guard the private name-layout implementation alongside the local UI fixes.

These source contracts detect an omitted feature during local reconciliation;
they do not replace the native GTK probe or installed-client acceptance.
"""

from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[2]


class PrivateNameLayoutIntegration(unittest.TestCase):
    def source(self, name):
        return (ROOT / "src/fe-gtk" / name).read_text(encoding="utf-8")

    def test_transcript_keeps_clipping_and_profiling(self):
        source = self.source("xtext.c")
        for contract in (
            "gtk_xtext_prefix_geometry (xtext, ent, &prefix)",
            "xtext->prefix_clip = TRUE",
            "fabulor_xtext_fit_prefix",
            '"transcript-paint"',
            "fabulor_ui_profile_enabled ()",
            "&xtext->palette[XTEXT_BG]",
        ):
            self.assertIn(contract, source)

    def test_clipping_preserves_source_identity(self):
        source = self.source("xtext.c")
        self.assertIn("memcpy (str, left_text, left_len)", source)
        self.assertIn("gtk_xtext_strip_color (ent->str, ent->left_len", source)
        self.assertIn("x = ent->indent + x - prefix.left", source)
        self.assertIn("gtk_tooltip_set_tip_area (tooltip, &area)", source)

    def test_labels_share_live_clipping(self):
        for name in ("user-list-view.c", "channel-tree-view.c", "chanview-tabs.c"):
            self.assertIn("fabulor_clipped_label_init", self.source(name))
        helper = self.source("clipped-label.h")
        self.assertIn("pango_layout_is_ellipsized", helper)
        self.assertIn("gtk_label_get_text", helper)

    def test_previous_cache_and_pane_fixes_remain(self):
        source = self.source("xtext-background.c")
        end_frame = source.split("fabulor_xtext_background_end_frame (", 1)[1]
        end_frame = end_frame.split("\n}", 1)[0]
        self.assertNotIn("xtext_background_clear_cache", end_frame)
        self.assertIn("gtk_widget_get_first_child", self.source("gtk-compat.h"))

    def test_native_probe_exercises_private_helpers(self):
        source = (ROOT / "tools/gtk4/probe.c").read_text(encoding="utf-8")
        self.assertIn("check_width_aware_names (gtk_ready)", source)
        self.assertIn("fabulor_xtext_auto_indent", source)
        self.assertIn("attrs[chars].is_cursor_position", source)
        self.assertIn("fabulor_clipped_label_tip_area", source)


if __name__ == "__main__":
    unittest.main()
