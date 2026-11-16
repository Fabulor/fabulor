#!/usr/bin/env python3
"""Source contracts for opt-in UI timing; not performance benchmarks."""

import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]


class UiPerformanceProfileTests(unittest.TestCase):
    def test_transcript_paint_timing_is_opt_in_and_covers_frame_cleanup(self):
        source = (ROOT / "src/fe-gtk/xtext.c").read_text(encoding="utf-8")
        render = source.split(
            "gtk_xtext_render (GtkWidget *widget,", 1
        )[1].split("\nstatic void\ngtk_xtext_paint", 1)[0]
        self.assertIn(
            "gint64 started = fabulor_ui_profile_enabled () ?\n"
            "\t\tg_get_monotonic_time () : 0;",
            render,
        )
        self.assertIn(
            'if (started)\n\t\tfabulor_ui_profile_log ("transcript-paint",',
            render,
        )
        self.assertLess(
            render.index("fabulor_xtext_background_end_frame"),
            render.index('fabulor_ui_profile_log ("transcript-paint"'),
        )
        self.assertLess(
            render.index("xtext->render_target, old_cr"),
            render.index('fabulor_ui_profile_log ("transcript-paint"'),
        )
        self.assertIn('"total_us=%" G_GINT64_FORMAT', render)
        self.assertIn("xtext->buffer->num_lines", render)
        self.assertNotIn("sess->channel", render)
        self.assertNotIn("ent->str", render)


if __name__ == "__main__":
    unittest.main()
