/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "xtext-hit-test.h"

#include <string.h>

gboolean
fabulor_xtext_hit_test_line (gint y, gint pixel_offset, gint font_size,
	gint scroll_line, gint *line)
{
	g_return_val_if_fail (line != NULL, FALSE);
	*line = 0;
	if (font_size <= 0 || pixel_offset < 0 || scroll_line < 0)
		return FALSE;
	if (y < 0)
		y -= font_size;
	*line = ((y + pixel_offset) / font_size) + scroll_line;
	return *line >= 0;
}

gboolean
fabulor_xtext_hit_test_separator (gboolean enabled, gint indent,
	gint space_width, gint x)
{
	gint separator_x;

	if (!enabled || indent <= 0 || space_width < 0)
		return FALSE;
	separator_x = indent - ((space_width + 1) / 2);
	return x >= separator_x - 1 && x <= separator_x + 1;
}

static gint
fabulor_xtext_hit_test_adjustment (const GSList *runs, gint match_offset)
{
	const GSList *item;
	gint consumed = 0;
	gint adjustment = 0;

	for (item = runs; item; item = item->next)
	{
		const offlen_t *run = item->data;
		if (run == NULL)
			continue;
		adjustment = (gint) run->off - consumed;
		consumed += run->len;
		if (match_offset < consumed)
			break;
	}
	return adjustment;
}

gboolean
fabulor_xtext_hit_test_adjust_match (const GSList *runs, gint match_start,
	gint match_end, gint *entry_offset, gint *entry_length)
{
	gint64 adjusted_start;
	gint64 adjusted_end;

	g_return_val_if_fail (entry_offset != NULL, FALSE);
	g_return_val_if_fail (entry_length != NULL, FALSE);
	if (match_start < 0 || match_end <= match_start || *entry_offset < 0 ||
		*entry_length <= 0)
		return FALSE;

	adjusted_start = (gint64) match_start +
		fabulor_xtext_hit_test_adjustment (runs, match_start);
	adjusted_end = (gint64) match_end +
		fabulor_xtext_hit_test_adjustment (runs, match_end);
	if (adjusted_start < 0 || adjusted_end <= adjusted_start ||
		adjusted_start > G_MAXINT || adjusted_end > G_MAXINT ||
		adjusted_end > *entry_length ||
		(gint64) *entry_offset + adjusted_end > G_MAXINT)
		return FALSE;

	*entry_offset += (gint) adjusted_start;
	*entry_length = (gint) (adjusted_end - adjusted_start);
	return TRUE;
}

void
fabulor_xtext_hit_init (FabulorXTextHit *hit, gchar *word, gint type,
	gint match_start, gint match_end)
{
	gsize word_length;

	g_return_if_fail (hit != NULL);
	hit->word = word;
	hit->type = type;
	hit->match_start = 0;
	hit->match_end = 0;
	if (word == NULL || type <= 0)
		return;

	word_length = strlen (word);
	if (match_start < 0 || match_end <= match_start ||
		(gsize) match_end > word_length)
	{
		hit->type = 0;
		return;
	}
	hit->match_start = match_start;
	hit->match_end = match_end;
}

gboolean
fabulor_xtext_hit_has_match (const FabulorXTextHit *hit)
{
	return hit != NULL && hit->word != NULL && hit->type > 0 &&
		hit->match_start >= 0 && hit->match_end > hit->match_start &&
		(gsize) hit->match_end <= strlen (hit->word);
}

gchar *
fabulor_xtext_hit_dup_match (const FabulorXTextHit *hit)
{
	if (!fabulor_xtext_hit_has_match (hit))
		return NULL;
	return g_strndup (hit->word + hit->match_start,
		(gsize) (hit->match_end - hit->match_start));
}
