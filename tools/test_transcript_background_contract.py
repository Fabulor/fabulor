"""Keep the image background independent of IRC message colour state."""

from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class TranscriptBackgroundContract(unittest.TestCase):
    def test_image_paint_uses_transcript_palette(self):
        source = (ROOT / "src/fe-gtk/xtext.c").read_text(encoding="utf-8")
        paint = source.split("xtext_draw_bg_offset (", 1)[1].split(
            "static inline void", 1
        )[0]
        self.assertIn(
            "fabulor_xtext_background_paint (xtext->background, cr, "
            "&xtext->palette[XTEXT_BG]", paint
        )
        self.assertNotIn("&xtext->bgc", paint)
        self.assertIn("prefs.hex_text_background_dim", paint)

    def test_message_background_colours_are_retained(self):
        source = (ROOT / "src/fe-gtk/xtext.c").read_text(encoding="utf-8")
        self.assertIn("xtext->bgc = xtext->palette[index]", source)
        self.assertIn("xtext_draw_rectangle (xtext, cr, &xtext->bgc", source)


if __name__ == "__main__":
    unittest.main()
