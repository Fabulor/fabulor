#ifndef FABULOR_SPELL_ENTRY_WORDS_H
#define FABULOR_SPELL_ENTRY_WORDS_H

#include <pango/pango.h>

typedef struct _FabulorSpellWords FabulorSpellWords;

typedef struct
{
	guint byte_start;
	guint byte_end;
	guint character_start;
	guint character_end;
} FabulorSpellWordRange;

FabulorSpellWords *fabulor_spell_words_new (const gchar *text,
	PangoLanguage *language);
void fabulor_spell_words_free (FabulorSpellWords *words);
guint fabulor_spell_words_count (const FabulorSpellWords *words);
gboolean fabulor_spell_words_get (const FabulorSpellWords *words,
	guint index, FabulorSpellWordRange *range);
gboolean fabulor_spell_words_find_character (const FabulorSpellWords *words,
	guint position, FabulorSpellWordRange *range);
gchar *fabulor_spell_words_dup_word (const FabulorSpellWords *words,
	guint index);

#endif
