#include "emoji-picker.h"

#include <string.h>

#define FABULOR_EMOJI_POPOVER_DATA "fabulor-emoji-popover"

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

static void
fabulor_emoji_picker_popover_free (GtkPopover *popover)
{
#if GTK_MAJOR_VERSION >= 4
	if (gtk_widget_get_parent (GTK_WIDGET (popover)))
		gtk_widget_unparent (GTK_WIDGET (popover));
#else
	gtk_widget_destroy (GTK_WIDGET (popover));
#endif
	g_object_unref (popover);
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

#if GTK_MAJOR_VERSION >= 4
	popover = GTK_POPOVER (gtk_popover_new ());
	gtk_widget_set_parent (GTK_WIDGET (popover), GTK_WIDGET (entry));
#else
	popover = GTK_POPOVER (gtk_popover_new (GTK_WIDGET (entry)));
#endif
	g_object_ref_sink (popover);
	g_object_set_data_full (G_OBJECT (entry), FABULOR_EMOJI_POPOVER_DATA,
		popover, (GDestroyNotify) fabulor_emoji_picker_popover_free);
	gtk_popover_set_position (popover, GTK_POS_TOP);
#if GTK_MAJOR_VERSION >= 4
	gtk_popover_set_autohide (popover, TRUE);
#else
	gtk_popover_set_modal (popover, TRUE);
	gtk_popover_set_transitions_enabled (popover, FALSE);
#endif
	return popover;
}
