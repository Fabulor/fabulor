#include "spell-entry-words.h"

#include <string.h>

struct _FabulorSpellWords
{
	gchar *text;
	GArray *ranges;
};

FabulorSpellWords *
fabulor_spell_words_new (const gchar *text, PangoLanguage *language)
{
	FabulorSpellWords *words = g_new0 (FabulorSpellWords, 1);
	PangoLogAttr *attributes;
	glong character_count;
	gint attribute_count;
	gint i;

	words->text = g_utf8_make_valid (text ? text : "", -1);
	words->ranges = g_array_new (FALSE, FALSE,
		sizeof (FabulorSpellWordRange));
	character_count = g_utf8_strlen (words->text, -1);
	if (character_count > G_MAXINT - 1 || strlen (words->text) > G_MAXUINT)
		return words;

	attribute_count = (gint) character_count + 1;
	attributes = g_new0 (PangoLogAttr, attribute_count);
	pango_get_log_attrs (words->text, -1, -1,
		language ? language : pango_language_get_default (), attributes,
		attribute_count);

	for (i = 0; i < attribute_count; i++)
	{
		gint end;
		FabulorSpellWordRange range;
		const gchar *byte_start;
		const gchar *byte_end;

		if (!attributes[i].is_word_start ||
			!attributes[i].is_word_boundary)
			continue;
		for (end = i + 1; end < attribute_count; end++)
		{
			if (attributes[end].is_word_end &&
				attributes[end].is_word_boundary)
				break;
		}
		if (end >= attribute_count)
			break;

		byte_start = g_utf8_offset_to_pointer (words->text, i);
		byte_end = g_utf8_offset_to_pointer (words->text, end);
		range.byte_start = (guint) (byte_start - words->text);
		range.byte_end = (guint) (byte_end - words->text);
		range.character_start = (guint) i;
		range.character_end = (guint) end;
		g_array_append_val (words->ranges, range);
	}

	g_free (attributes);
	return words;
}

void
fabulor_spell_words_free (FabulorSpellWords *words)
{
	if (!words)
		return;
	g_array_unref (words->ranges);
	g_free (words->text);
	g_free (words);
}

guint
fabulor_spell_words_count (const FabulorSpellWords *words)
{
	return words ? words->ranges->len : 0;
}

gboolean
fabulor_spell_words_get (const FabulorSpellWords *words, guint index,
	FabulorSpellWordRange *range)
{
	if (!words || !range || index >= words->ranges->len)
		return FALSE;
	*range = g_array_index (words->ranges, FabulorSpellWordRange, index);
	return TRUE;
}

gboolean
fabulor_spell_words_find_character (const FabulorSpellWords *words,
	guint position, FabulorSpellWordRange *range)
{
	guint i;

	if (!words || !range)
		return FALSE;
	for (i = 0; i < words->ranges->len; i++)
	{
		FabulorSpellWordRange candidate = g_array_index (words->ranges,
			FabulorSpellWordRange, i);
		if (position >= candidate.character_start &&
			position <= candidate.character_end)
		{
			*range = candidate;
			return TRUE;
		}
	}
	return FALSE;
}

gchar *
fabulor_spell_words_dup_word (const FabulorSpellWords *words, guint index)
{
	FabulorSpellWordRange range;

	if (!fabulor_spell_words_get (words, index, &range))
		return NULL;
	return g_strndup (words->text + range.byte_start,
		range.byte_end - range.byte_start);
}
