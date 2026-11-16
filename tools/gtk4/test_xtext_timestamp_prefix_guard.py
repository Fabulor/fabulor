"""Guard timestamp-only paints from nickname-prefix processing.

The timestamp renderer temporarily points an entry at a short stamp string
when timestamp copying is enabled, while retaining the original left_len.
These are source contracts, not installed visual or pixel-rendering tests.
"""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]


class TimestampPrefixGuard(unittest.TestCase):
    def test_timestamp_only_pass_cannot_enter_prefix_geometry(self):
        source = (ROOT / "src/fe-gtk/xtext.c").read_text(encoding="utf-8")
        render = source.split("gtk_xtext_render_str (GtkXText * xtext", 1)[1]
        expression = render.split("clipped_prefix =", 1)[1].split(";", 1)[0]
        self.assertRegex(expression, r"!left_only\s*&&\s*offset\s*==\s*0\s*&&")
        self.assertIn("gtk_xtext_prefix_geometry (xtext, ent, &prefix)", expression)

    def test_timestamp_substitution_is_an_explicit_left_only_pass(self):
        source = (ROOT / "src/fe-gtk/xtext.c").read_text(encoding="utf-8")
        stamp = source.split("gtk_xtext_render_stamp (GtkXText * xtext", 1)[1]
        stamp = stamp.split("/* render a single line", 1)[0]
        self.assertIn("if (xtext->mark_stamp)", stamp)
        self.assertIn("ent->str = text", stamp)
        self.assertRegex(stamp, r"win_width,\s*2,\s*line,\s*TRUE,\s*&xsize")
        self.assertIn("memcpy (ent, &tmp_ent, sizeof (tmp_ent))", stamp)

    def test_normal_message_clipping_remains(self):
        source = (ROOT / "src/fe-gtk/xtext.c").read_text(encoding="utf-8")
        self.assertIn("xtext->prefix_clip = TRUE", source)
        self.assertIn("prefix.left + prefix.visible_width", source)
        self.assertIn("xtext->buffer->indent - xtext->space_width", source)


if __name__ == "__main__":
    unittest.main()
