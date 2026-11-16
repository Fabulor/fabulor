#include "emoji-picker.h"

#include <string.h>
#include <pango/pangocairo.h>

#define FABULOR_EMOJI_POPOVER_DATA "fabulor-emoji-popover"
#define FABULOR_EMOJI_VIEWPORT_COMPACT_WIDTH 320
#define FABULOR_EMOJI_VIEWPORT_COMPACT_HEIGHT 240
#define FABULOR_EMOJI_VIEWPORT_MAX_WIDTH 520
#define FABULOR_EMOJI_VIEWPORT_MAX_HEIGHT 320
#define FABULOR_EMOJI_VIEWPORT_HORIZONTAL_INSET 48
#define FABULOR_EMOJI_VIEWPORT_VERTICAL_INSET 140

struct _FabulorEmojiPickerPage
{
	const gunichar *items;
	gboolean flags;
	gboolean loaded;
};

FabulorEmojiPickerPage *
fabulor_emoji_picker_page_new (const gunichar *items, gboolean flags)
{
	FabulorEmojiPickerPage *page = g_new0 (FabulorEmojiPickerPage, 1);

	page->items = items;
	page->flags = flags;
	return page;
}

void
fabulor_emoji_picker_page_free (FabulorEmojiPickerPage *page)
{
	g_free (page);
}

gboolean
fabulor_emoji_picker_page_claim_load (FabulorEmojiPickerPage *page)
{
	if (!page || page->loaded)
		return FALSE;
	page->loaded = TRUE;
	return TRUE;
}

gboolean
fabulor_emoji_picker_page_has_flags (const FabulorEmojiPickerPage *page)
{
	return page ? page->flags : FALSE;
}

const gunichar *
fabulor_emoji_picker_page_items (const FabulorEmojiPickerPage *page)
{
	return page ? page->items : NULL;
}

gchar *
fabulor_emoji_picker_flag_sequence (const gchar *code)
{
	GString *sequence;
	gint i;

	if (!code || strlen (code) != 2 ||
		!g_ascii_isalpha (code[0]) || !g_ascii_isalpha (code[1]))
		return NULL;

	sequence = g_string_sized_new (9);
	for (i = 0; i < 2; i++)
	{
		gunichar regional = 0x1F1E6 + (g_ascii_toupper (code[i]) - 'A');
		gchar utf8[7];
		gint length = g_unichar_to_utf8 (regional, utf8);

		g_string_append_len (sequence, utf8, length);
	}
	return g_string_free (sequence, FALSE);
}

gchar *
fabulor_emoji_picker_codepoint_sequence (gunichar codepoint)
{
	gchar utf8[7];
	gint length;

	if (!g_unichar_validate (codepoint) || codepoint == 0)
		return NULL;
	length = g_unichar_to_utf8 (codepoint, utf8);
	return g_strndup (utf8, length);
}

gboolean
fabulor_emoji_picker_flag_matches (const gchar *code,
	const gchar *country_name, const gchar *query)
{
	gchar *folded_query;
	gchar *folded_code;
	gchar *folded_name;
	gboolean matches;

	if (!query || *query == '\0')
		return TRUE;
	if (!code || !country_name)
		return FALSE;

	folded_query = g_utf8_casefold (query, -1);
	g_strstrip (folded_query);
	if (*folded_query == '\0')
	{
		g_free (folded_query);
		return TRUE;
	}

	folded_code = g_utf8_casefold (code, -1);
	folded_name = g_utf8_casefold (country_name, -1);
	matches = strstr (folded_code, folded_query) != NULL ||
		strstr (folded_name, folded_query) != NULL;
	g_free (folded_name);
	g_free (folded_code);
	g_free (folded_query);
	return matches;
}

void
fabulor_emoji_picker_viewport_size (gint root_width, gint root_height,
	gint *width, gint *height)
{
	gint available_width = root_width - FABULOR_EMOJI_VIEWPORT_HORIZONTAL_INSET;
	gint available_height = root_height - FABULOR_EMOJI_VIEWPORT_VERTICAL_INSET;

	if (width)
		*width = CLAMP (available_width,
			FABULOR_EMOJI_VIEWPORT_COMPACT_WIDTH,
			FABULOR_EMOJI_VIEWPORT_MAX_WIDTH);
	if (height)
		*height = CLAMP (available_height,
			FABULOR_EMOJI_VIEWPORT_COMPACT_HEIGHT,
			FABULOR_EMOJI_VIEWPORT_MAX_HEIGHT);
}

static void
fabulor_emoji_picker_popover_free (GtkPopover *popover)
{
	if (gtk_widget_get_parent (GTK_WIDGET (popover)))
		gtk_widget_unparent (GTK_WIDGET (popover));
	g_object_unref (popover);
}

static void
fabulor_emoji_native_picked (GtkEmojiChooser *chooser, const gchar *text,
	GtkEntry *entry)
{
	GtkEditable *editable = GTK_EDITABLE (entry);
	gint start, end, position;
	if (!gtk_editable_get_editable (editable))
		return;
	if (gtk_editable_get_selection_bounds (editable, &start, &end))
		gtk_editable_delete_text (editable, start, end);
	position = gtk_editable_get_position (editable);
	gtk_editable_insert_text (editable, text, -1, &position);
	gtk_popover_popdown (GTK_POPOVER (chooser));
	gtk_entry_grab_focus_without_selecting (entry);
	gtk_editable_set_position (editable, position);
}

static void
fabulor_emoji_native_style (GdkDisplay *display)
{
	GtkCssProvider *provider;
	if (g_object_get_data (G_OBJECT (display), "fabulor-emoji-native-style"))
		return;
	provider = gtk_css_provider_new ();
	/* Scope the bundled font to the chooser, not the transcript or input.
	 * Segoe's regional indicators can otherwise appear as two letters and
	 * GTK can discard wider fallback sequences during catalogue population. */
	gtk_css_provider_load_from_string (provider,
		".fabulor-emoji-chooser, .fabulor-emoji-chooser * { font-family: \"Noto Color Emoji\", \"Segoe UI\", sans-serif; }");
	gtk_style_context_add_provider_for_display (display,
		GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_USER + 1);
	g_object_set_data_full (G_OBJECT (display), "fabulor-emoji-native-style",
		provider, g_object_unref);
}

gboolean
fabulor_emoji_picker_native_popup (GtkEntry *entry)
{
	GtkWidget *chooser;
	if (!GTK_IS_ENTRY (entry) ||
		!gtk_editable_get_editable (GTK_EDITABLE (entry)))
		return FALSE;
	chooser = g_object_get_data (G_OBJECT (entry), "fabulor-native-emoji-chooser");
	if (!chooser)
	{
		fabulor_emoji_native_style (gtk_widget_get_display (GTK_WIDGET (entry)));
		PangoFontMap *font_map;
		chooser = gtk_emoji_chooser_new ();
		/* A Win32 font map cannot discover the private bundled Noto font.
		 * Select the packaged Fontconfig/FreeType backend only for this
		 * chooser; CSS alone cannot change the backend's font inventory. */
		font_map = pango_cairo_font_map_new_for_font_type (CAIRO_FONT_TYPE_FT);
		if (font_map)
		{
			gtk_widget_set_font_map (chooser, font_map);
			g_object_unref (font_map);
		}
		else
			g_warning ("Emoji chooser: packaged FreeType font map is unavailable");
		gtk_widget_add_css_class (chooser, "fabulor-emoji-chooser");
		gtk_widget_set_parent (chooser, GTK_WIDGET (entry));
		gtk_popover_set_autohide (GTK_POPOVER (chooser), TRUE);
		gtk_popover_set_position (GTK_POPOVER (chooser), GTK_POS_TOP);
		g_signal_connect_object (chooser, "emoji-picked",
			G_CALLBACK (fabulor_emoji_native_picked), entry, 0);
		g_object_set_data_full (G_OBJECT (entry), "fabulor-native-emoji-chooser",
			chooser, (GDestroyNotify) fabulor_emoji_picker_popover_free);
	}
	gtk_popover_popup (GTK_POPOVER (chooser));
	return TRUE;
}

static void
fabulor_emoji_native_action (GtkWidget *delegate, GtkEntry *entry)
{
	/* Also handle callers which emit the legacy action signal directly. */
	g_signal_stop_emission_by_name (delegate, "insert-emoji");
	fabulor_emoji_picker_native_popup (entry);
}

static gboolean
fabulor_emoji_native_key (GtkEventControllerKey *controller, guint keyval,
	guint keycode, GdkModifierType state, GtkEntry *entry)
{
	(void) controller;
	(void) keycode;
	if ((state & gtk_accelerator_get_default_mod_mask ()) != GDK_CONTROL_MASK ||
		(keyval != GDK_KEY_semicolon && keyval != GDK_KEY_period))
		return FALSE;
	return fabulor_emoji_picker_native_popup (entry);
}

void
fabulor_emoji_picker_bind_entry (GtkEntry *entry)
{
	GtkEditable *delegate;
	GtkEventController *keys;
	if (!GTK_IS_ENTRY (entry) ||
		g_object_get_data (G_OBJECT (entry), "fabulor-emoji-action-bound"))
		return;
	delegate = gtk_editable_get_delegate (GTK_EDITABLE (entry));
	if (!delegate || !g_signal_lookup ("insert-emoji", G_OBJECT_TYPE (delegate)))
		return;
	g_signal_connect_object (delegate, "insert-emoji",
		G_CALLBACK (fabulor_emoji_native_action), entry, 0);
	/* GTK 4.22's keyboard action calls its implementation directly, not
	 * the legacy signal, so capture both shortcuts before the delegate. */
	keys = gtk_event_controller_key_new ();
	gtk_event_controller_set_propagation_phase (keys, GTK_PHASE_CAPTURE);
	g_signal_connect_object (keys, "key-pressed",
		G_CALLBACK (fabulor_emoji_native_key), entry, 0);
	gtk_widget_add_controller (GTK_WIDGET (entry), keys);
	g_object_set_data (G_OBJECT (entry), "fabulor-emoji-action-bound", keys);
}

GtkPopover *
fabulor_emoji_picker_popover_get (GtkEntry *entry)
{
	if (!entry)
		return NULL;
	return g_object_get_data (G_OBJECT (entry), FABULOR_EMOJI_POPOVER_DATA);
}

GtkPopover *
fabulor_emoji_picker_popover_ensure (GtkEntry *entry)
{
	GtkPopover *popover;

	if (!entry)
		return NULL;
	popover = fabulor_emoji_picker_popover_get (entry);
	if (popover)
		return popover;

	popover = GTK_POPOVER (gtk_popover_new ());
	gtk_widget_set_parent (GTK_WIDGET (popover), GTK_WIDGET (entry));
	g_object_ref_sink (popover);
	g_object_set_data_full (G_OBJECT (entry), FABULOR_EMOJI_POPOVER_DATA,
		popover, (GDestroyNotify) fabulor_emoji_picker_popover_free);
	gtk_popover_set_position (popover, GTK_POS_TOP);
	gtk_popover_set_autohide (popover, TRUE);
	return popover;
}
