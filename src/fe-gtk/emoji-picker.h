#ifndef FABULOR_EMOJI_PICKER_H
#define FABULOR_EMOJI_PICKER_H

#include <gtk/gtk.h>

typedef struct _FabulorEmojiPickerPage FabulorEmojiPickerPage;

FabulorEmojiPickerPage *fabulor_emoji_picker_page_new (
	const gunichar *items, gboolean flags);
void fabulor_emoji_picker_page_free (FabulorEmojiPickerPage *page);
gboolean fabulor_emoji_picker_page_claim_load (FabulorEmojiPickerPage *page);
gboolean fabulor_emoji_picker_page_has_flags (
	const FabulorEmojiPickerPage *page);
const gunichar *fabulor_emoji_picker_page_items (
	const FabulorEmojiPickerPage *page);

gchar *fabulor_emoji_picker_flag_sequence (const gchar *code);
gchar *fabulor_emoji_picker_codepoint_sequence (gunichar codepoint);
void fabulor_emoji_picker_viewport_size (gint root_width, gint root_height,
	gint *width, gint *height);

GtkPopover *fabulor_emoji_picker_popover_get (GtkEntry *entry);
GtkPopover *fabulor_emoji_picker_popover_ensure (GtkEntry *entry);

#endif
