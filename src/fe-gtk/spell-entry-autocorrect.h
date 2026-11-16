/* Optional, local Windows-suggestion correction. No dictionary ownership. */
#ifndef FABULOR_SPELL_ENTRY_AUTOCORRECT_H
#define FABULOR_SPELL_ENTRY_AUTOCORRECT_H
#include <gtk/gtk.h>

typedef gboolean (*FabulorAutoCorrectApprove) (GtkEntry *entry,
	const char *original, const char *replacement, gpointer data);
/* Return an owned UTF-8 suggestion, or NULL. Called only at word boundaries. */
typedef char *(*FabulorAutoCorrectSuggest) (GtkEntry *entry,
	const char *original, gpointer data);
void fabulor_spell_autocorrect_bind (GtkEntry *entry,
	FabulorAutoCorrectApprove approve, FabulorAutoCorrectSuggest suggest, gpointer data);
void fabulor_spell_autocorrect_set_enabled (GtkEntry *entry, gboolean enabled);
void fabulor_spell_autocorrect_reset (GtkEntry *entry);
/* Return an eligible typed word, not a replacement. No fixed typo list. */
char *fabulor_spell_autocorrect_candidate (const char *text, gint cursor,
	gint typed_start, gint *word_start);
gboolean fabulor_spell_autocorrect_valid_replacement (const char *word);
#endif
