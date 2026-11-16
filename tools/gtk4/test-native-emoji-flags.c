/* Probe the pinned GTK catalogue and the bundled font before switching UI. */
#include <gtk/gtk.h>
#include <stdio.h>
#include "../../src/fe-gtk/emoji-picker.h"

static guint
count_country_labels (GtkWidget *widget)
{
	guint count = 0;
	GtkWidget *child;
	if (GTK_IS_LABEL (widget))
	{
		const gchar *text = gtk_label_get_text (GTK_LABEL (widget));
		if (g_utf8_strlen (text, -1) == 2)
		{
			gunichar a = g_utf8_get_char (text);
			gunichar b = g_utf8_get_char (g_utf8_next_char (text));
			if (a >= 0x1f1e6 && a <= 0x1f1ff && b >= 0x1f1e6 && b <= 0x1f1ff)
				count++;
		}
	}
	for (child = gtk_widget_get_first_child (widget); child;
		child = gtk_widget_get_next_sibling (child))
		count += count_country_labels (child);
	return count;
}

static GtkWidget *
find_label (GtkWidget *widget, const gchar *text)
{
	GtkWidget *child;
	if (GTK_IS_LABEL (widget) && g_strcmp0 (gtk_label_get_text (GTK_LABEL (widget)), text) == 0)
		return widget;
	for (child = gtk_widget_get_first_child (widget); child;
		child = gtk_widget_get_next_sibling (child))
	{
		GtkWidget *found = find_label (child, text);
		if (found)
			return found;
	}
	return NULL;
}

static void
check_attached_chooser_teardown (void)
{
	GtkWidget *window = gtk_window_new ();
	GtkWidget *entry = gtk_entry_new ();
	GtkWidget *chooser;

	gtk_window_set_child (GTK_WINDOW (window), entry);
	gtk_window_present (GTK_WINDOW (window));
	g_assert_true (fabulor_emoji_picker_native_popup (GTK_ENTRY (entry)));
	chooser = g_object_get_data (G_OBJECT (entry), "fabulor-native-emoji-chooser");
	g_assert_true (gtk_widget_get_parent (chooser) == entry);
	g_object_ref (chooser);
	g_object_ref (entry);
	gtk_window_destroy (GTK_WINDOW (window));
	while (g_main_context_iteration (NULL, FALSE)) {}
	g_assert_null (g_object_get_data (G_OBJECT (entry), "fabulor-native-emoji-chooser"));
	g_assert_null (gtk_widget_get_parent (chooser));
	g_object_unref (chooser);
	g_object_unref (entry);
}

int main (int argc, char **argv)
{
	GtkWidget *chooser, *window, *entry;
	GtkEditable *delegate;
	gint cursor;
	gboolean handled;
	GtkEventController *keys;
	GBytes *bytes;
	GVariant *catalogue, *item;
	GVariantIter iter;
	guint flags = 0, failures = 0;
	gint largest_ink_width = 0, largest_ink_height = 0;
	g_assert_cmpint (argc, ==, 2);
	fabulor_emoji_picker_set_font_file (argv[1]);
	gtk_init ();
	/* Themes may set a font on the emoji label itself. An inherited font on
	 * the popover does not override that, regardless of provider priority. */
	{
		GtkCssProvider *theme = gtk_css_provider_new ();
		gtk_css_provider_load_from_string (theme,
			"* { font-family: \"JetBrainsMono Nerd Font Mono\", \"Segoe UI Emoji\"; }"
			".emoji-size-stress, .emoji-size-stress * { font-size: 32px; }");
		gtk_style_context_add_provider_for_display (gdk_display_get_default (),
			GTK_STYLE_PROVIDER (theme), GTK_STYLE_PROVIDER_PRIORITY_USER);
		g_object_unref (theme);
	}
	window = gtk_window_new ();
	entry = gtk_entry_new ();
	gtk_widget_add_css_class (entry, "emoji-size-stress");
	gtk_window_set_child (GTK_WINDOW (window), entry);
	gtk_window_present (GTK_WINDOW (window));
	fabulor_emoji_picker_bind_entry (GTK_ENTRY (entry));
	fabulor_emoji_picker_bind_entry (GTK_ENTRY (entry));
	delegate = gtk_editable_get_delegate (GTK_EDITABLE (entry));
	g_signal_emit_by_name (delegate, "insert-emoji");
	chooser = g_object_get_data (G_OBJECT (entry), "fabulor-native-emoji-chooser");
	g_assert_true (GTK_IS_EMOJI_CHOOSER (chooser));
	g_assert_nonnull (pango_font_map_get_family (gtk_widget_get_font_map (chooser),
		"Noto Color Emoji"));
	g_assert_true (fabulor_emoji_picker_native_popup (GTK_ENTRY (entry)));
	g_assert_true (chooser == g_object_get_data (G_OBJECT (entry), "fabulor-native-emoji-chooser"));
	keys = g_object_get_data (G_OBJECT (entry), "fabulor-emoji-action-bound");
	g_assert_cmpint (gtk_event_controller_get_propagation_phase (keys), ==, GTK_PHASE_CAPTURE);
	g_signal_emit_by_name (keys, "key-pressed", GDK_KEY_semicolon, 0, GDK_CONTROL_MASK, &handled);
	g_assert_true (handled);
	g_signal_emit_by_name (keys, "key-pressed", GDK_KEY_period, 0, GDK_CONTROL_MASK, &handled);
	g_assert_true (handled);
	g_signal_emit_by_name (keys, "key-pressed", GDK_KEY_A, 0, GDK_CONTROL_MASK, &handled);
	g_assert_false (handled);
	g_assert_true (chooser == g_object_get_data (G_OBJECT (entry), "fabulor-native-emoji-chooser"));
	/* Exercise selected-text replacement and Unicode cursor positioning. */
	gtk_editable_set_text (GTK_EDITABLE (entry), "AxxB");
	gtk_editable_select_region (GTK_EDITABLE (entry), 1, 3);
	g_signal_emit_by_name (chooser, "emoji-picked", "\360\237\207\246\360\237\207\272");
	g_assert_cmpstr (gtk_editable_get_text (GTK_EDITABLE (entry)), ==,
		"A\360\237\207\246\360\237\207\272B");
	cursor = gtk_editable_get_position (GTK_EDITABLE (entry));
	g_assert_cmpint (cursor, ==, 3);
	gtk_editable_set_editable (GTK_EDITABLE (entry), FALSE);
	g_assert_false (fabulor_emoji_picker_native_popup (GTK_ENTRY (entry)));
	g_signal_emit_by_name (chooser, "emoji-picked", "X");
	g_assert_cmpint (gtk_editable_get_position (GTK_EDITABLE (entry)), ==, cursor);
	gtk_editable_set_editable (GTK_EDITABLE (entry), TRUE);
	fabulor_emoji_picker_native_popup (GTK_ENTRY (entry));
	/* Allow frame-clock style/layout updates after the idle catalogue load;
	 * an empty nonblocking main-context queue does not mean a frame ran. */
	{
		gint frame;
		for (frame = 0; frame < 50; frame++)
		{
			while (g_main_context_iteration (NULL, FALSE)) {}
			g_usleep (10000);
		}
	}
	/* The pinned template must retain a short scrolling viewport even
	 * when the input entry's theme uses oversized text. */
	{
		GObject *viewport = gtk_widget_get_template_child (chooser,
			GTK_TYPE_EMOJI_CHOOSER, "scrolled_window");
		GdkSurface *surface = gtk_native_get_surface (GTK_NATIVE (chooser));
		g_assert_true (GTK_IS_SCROLLED_WINDOW (viewport));
		g_assert_cmpint (gtk_scrolled_window_get_max_content_height (
			GTK_SCROLLED_WINDOW (viewport)), <=, 220);
		g_assert_cmpint (gtk_widget_get_height (chooser), >, 0);
		g_assert_cmpint (gtk_widget_get_height (chooser), <=, 480);
		g_assert_nonnull (surface);
		printf ("chooser size: %d x %d logical pixels, surface %.2f, widget scale %d\n",
			gtk_widget_get_width (chooser), gtk_widget_get_height (chooser),
			gdk_surface_get_scale (surface), gtk_widget_get_scale_factor (chooser));
	}
	/* Inspect each country label's own layout and selected font below. */
	bytes = g_resources_lookup_data ("/org/gtk/libgtk/emoji/en.data", 0, NULL);
	if (!bytes) return 2;
	/* Test-only contract for our pinned GTK 4.22.4 catalogue. */
	catalogue = g_variant_ref_sink (g_variant_new_from_bytes (
		G_VARIANT_TYPE ("a(aussasasu)"), bytes, FALSE));
	g_variant_iter_init (&iter, catalogue);
	while ((item = g_variant_iter_next_value (&iter)))
	{
		GVariant *codes = g_variant_get_child_value (item, 0);
		guint32 a, b;
		if (g_variant_n_children (codes) == 2)
		{
			g_variant_get_child (codes, 0, "u", &a);
			g_variant_get_child (codes, 1, "u", &b);
			if (a >= 0x1f1e6 && a <= 0x1f1ff && b >= 0x1f1e6 && b <= 0x1f1ff)
			{
				gchar text[16];
				gint length = g_unichar_to_utf8 (a, text);
				PangoLayoutIter *runs;
				gint glyphs = 0;
				GtkWidget *flag_label;
				PangoLayout *flag_layout;
				PangoRectangle ink;
				length += g_unichar_to_utf8 (b, text + length);
				text[length] = 0;
				flag_label = find_label (chooser, text);
				g_assert_nonnull (flag_label);
				/* Inspect the rendered label, not its ancestor's font context. */
				flag_layout = gtk_label_get_layout (GTK_LABEL (flag_label));
				g_assert_true (pango_context_get_font_map (pango_layout_get_context (flag_layout)) ==
					gtk_widget_get_font_map (chooser));
				pango_layout_get_pixel_extents (flag_layout, &ink, NULL);
				largest_ink_width = MAX (largest_ink_width, ink.width);
				largest_ink_height = MAX (largest_ink_height, ink.height);
				if (ink.width <= 0 || ink.height <= 0 || ink.width > 48 || ink.height > 48)
					failures++;
				runs = pango_layout_get_iter (flag_layout);
				do {
					PangoLayoutRun *run = pango_layout_iter_get_run_readonly (runs);
					if (run)
					{
						PangoFontDescription *actual = pango_font_describe (run->item->analysis.font);
						const gchar *family = pango_font_description_get_family (actual);
						glyphs += run->glyphs->num_glyphs;
						if (g_strcmp0 (family, "Noto Color Emoji") != 0)
						{
							printf ("Flag font mismatch: %s\n", family);
							failures++;
						}
						pango_font_description_free (actual);
					}
				} while (pango_layout_iter_next_run (runs));
				pango_layout_iter_free (runs);
				printf ("%c%c glyphs=%d unknown=%d\n", 'A' + a - 0x1f1e6,
					'A' + b - 0x1f1e6, glyphs, pango_layout_get_unknown_glyphs_count (flag_layout));
				flags++;
				if (glyphs != 1 || pango_layout_get_unknown_glyphs_count (flag_layout)) failures++;
			}
		}
		g_variant_unref (codes);
		g_variant_unref (item);
	}
	printf ("flags=%u failures=%u visible_country_labels=%u max ink=%dx%d logical pixels\n",
		flags, failures, count_country_labels (chooser), largest_ink_width, largest_ink_height);
	g_assert_cmpuint (count_country_labels (chooser), >=, flags);
	g_assert_cmpuint (flags, >=, 253);
	g_variant_unref (catalogue);
	g_bytes_unref (bytes);
	/* The cache still owns the chooser if it loses its widget parent.
	 * Unrealizing the entry must clear the cache and release that reference. */
	g_object_add_weak_pointer (G_OBJECT (chooser), (gpointer *) &chooser);
	gtk_popover_popdown (GTK_POPOVER (chooser));
	gtk_widget_unparent (chooser);
	g_assert_nonnull (chooser);
	g_object_ref (entry);
	gtk_window_destroy (GTK_WINDOW (window));
	while (g_main_context_iteration (NULL, FALSE)) {}
	g_assert_null (g_object_get_data (G_OBJECT (entry), "fabulor-native-emoji-chooser"));
	g_assert_null (chooser);
	g_object_unref (entry);
	check_attached_chooser_teardown ();
	return flags == 0 || failures != 0;
}
