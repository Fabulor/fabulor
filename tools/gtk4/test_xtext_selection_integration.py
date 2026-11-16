"""Contracts for clipped-prefix message hit testing (native widget suite too)."""
from pathlib import Path
import unittest
ROOT = Path(__file__).resolve().parents[2]

class SelectionIntegration(unittest.TestCase):
    def test_message_hits_skip_the_clipped_prefix(self):
        source = (ROOT/'src/fe-gtk/xtext.c').read_text(encoding='utf-8')
        hit = source.split('gtk_xtext_find_x (', 1)[1].split('static textentry *', 1)[0]
        self.assertIn('subline == 0 && gtk_xtext_prefix_geometry', hit)
        self.assertIn('if (x >= xtext->buffer->indent)', hit)
        self.assertIn('ent->left_len + 1', hit)
        self.assertIn('xtext->buffer->indent,', hit)
        self.assertIn('x = ent->indent + x - prefix.left', hit)
        self.assertIn('indent, 0)', hit)

    def test_range_is_clamped_to_the_subline(self):
        source = (ROOT/'src/fe-gtk/xtext.c').read_text(encoding='utf-8')
        find = source.split('find_x (GtkXText', 1)[1].split('static int\ngtk_xtext_find_x', 1)[0]
        self.assertIn('line_start = MIN (line_end, MAX (line_start, start_offset));', find)
        self.assertIn('segment_start = MAX (meta->off, line_start)', find)
        self.assertIn('fabulor_xtext_layout_index_at_x', find)

    def test_native_fixture_uses_actual_append_and_memory_copy(self):
        source = (ROOT/'tools/gtk4/test-xtext-selection.c').read_text(encoding='utf-8')
        for name in ('gtk_xtext_append_indent', 'gtk_xtext_selection_apply_anchors',
                     'gtk_xtext_selection_get_text', 'test_short_prefix', 'test_plain',
                     'test_wrapped', 'test_unicode', 'test_prefix_identity'):
            self.assertIn(name, source)
        self.assertNotIn('gtk_xtext_set_clip_owner (', source)
        self.assertIn("test('XText clipped-prefix selection'", (ROOT/'tools/gtk4/meson.build').read_text())

if __name__ == '__main__':
    unittest.main()
