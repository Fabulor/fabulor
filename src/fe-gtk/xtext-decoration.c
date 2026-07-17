/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "xtext-decoration.h"

struct _FabulorXTextDecoration
{
	gconstpointer hover_entry;
	gint hover_start;
	gint hover_end;
	guint hover_suspend_depth;
	unsigned int hover_render_only : 1;
	unsigned int hover_clearing : 1;
	unsigned int hover_inside : 1;
};

FabulorXTextDecoration *
fabulor_xtext_decoration_new (void)
{
	FabulorXTextDecoration *decoration = g_new0 (FabulorXTextDecoration, 1);
	decoration->hover_start = -1;
	decoration->hover_end = -1;
	return decoration;
}

void
fabulor_xtext_decoration_free (FabulorXTextDecoration *decoration)
{
	g_free (decoration);
}

gboolean
fabulor_xtext_marker_position (gboolean enabled, gconstpointer marker_entry,
	gconstpointer entry, gconstpointer next_entry, gint y, gint descent,
	gint font_size, gint subline_count, gint *render_y)
{
	g_return_val_if_fail (render_y != NULL, FALSE);
	*render_y = 0;

	if (!enabled || marker_entry == NULL || entry == NULL)
		return FALSE;
	if (marker_entry == entry)
		*render_y = y + descent;
	else if (marker_entry == next_entry && next_entry != NULL)
		*render_y = y + descent + font_size * subline_count;
	else
		return FALSE;
	return TRUE;
}

guint
fabulor_xtext_search_match (const GList *marks, const GList *current,
	guint offset)
{
	const GList *item;
	offsets_t range;

	for (item = marks; item; item = item->next)
	{
		guint flags = FABULOR_XTEXT_MATCH_MID;
		range.u = GPOINTER_TO_UINT (item->data);
		if (offset < range.o.start || offset > range.o.end)
			continue;
		if (offset == range.o.start)
			flags |= FABULOR_XTEXT_MATCH_START;
		if (offset == range.o.end)
		{
			const GList *next = item->next;
			if (next)
			{
				range.u = GPOINTER_TO_UINT (next->data);
				if (offset == range.o.start)
					flags |= next == current ?
						FABULOR_XTEXT_MATCH_CURRENT : 0;
				else
					flags |= FABULOR_XTEXT_MATCH_END;
			}
			else
				flags |= FABULOR_XTEXT_MATCH_END;
		}
		else if (item == current)
			flags |= FABULOR_XTEXT_MATCH_CURRENT;
		return flags;
	}
	return 0;
}

gboolean
fabulor_xtext_decoration_set_hover (FabulorXTextDecoration *decoration,
	gconstpointer entry, gint start, gint end)
{
	g_return_val_if_fail (decoration != NULL, FALSE);
	if (entry == NULL || start < 0 || end <= start)
		return FALSE;
	if (decoration->hover_entry == entry &&
		decoration->hover_start == start && decoration->hover_end == end)
		return FALSE;
	decoration->hover_entry = entry;
	decoration->hover_start = start;
	decoration->hover_end = end;
	return TRUE;
}

void
fabulor_xtext_decoration_clear_hover (FabulorXTextDecoration *decoration)
{
	g_return_if_fail (decoration != NULL);
	decoration->hover_entry = NULL;
	decoration->hover_start = -1;
	decoration->hover_end = -1;
	decoration->hover_inside = FALSE;
}

gboolean
fabulor_xtext_decoration_has_hover (const FabulorXTextDecoration *decoration)
{
	return decoration != NULL && decoration->hover_entry != NULL;
}

gboolean
fabulor_xtext_decoration_hover_equals (
	const FabulorXTextDecoration *decoration, gconstpointer entry, gint start,
	gint end)
{
	return decoration != NULL && decoration->hover_entry == entry &&
		decoration->hover_start == start && decoration->hover_end == end;
}

gconstpointer
fabulor_xtext_decoration_hover_entry (const FabulorXTextDecoration *decoration)
{
	return decoration ? decoration->hover_entry : NULL;
}

static gboolean
fabulor_xtext_decoration_hover_active (
	const FabulorXTextDecoration *decoration, gconstpointer entry)
{
	return decoration != NULL && decoration->hover_suspend_depth == 0 &&
		decoration->hover_entry == entry;
}

gboolean
fabulor_xtext_decoration_hover_contains (
	const FabulorXTextDecoration *decoration, gconstpointer entry, gint offset)
{
	return fabulor_xtext_decoration_hover_active (decoration, entry) &&
		decoration->hover_start <= offset && decoration->hover_end > offset;
}

gboolean
fabulor_xtext_decoration_hover_starts (
	const FabulorXTextDecoration *decoration, gconstpointer entry, gint offset)
{
	return fabulor_xtext_decoration_hover_active (decoration, entry) &&
		decoration->hover_start == offset;
}

gboolean
fabulor_xtext_decoration_hover_ends (
	const FabulorXTextDecoration *decoration, gconstpointer entry, gint offset)
{
	return fabulor_xtext_decoration_hover_active (decoration, entry) &&
		decoration->hover_end == offset;
}

void
fabulor_xtext_decoration_begin_hover_render (
	FabulorXTextDecoration *decoration, gboolean clearing)
{
	g_return_if_fail (decoration != NULL);
	decoration->hover_render_only = TRUE;
	decoration->hover_clearing = clearing;
	decoration->hover_inside = FALSE;
}

void
fabulor_xtext_decoration_end_hover_render (FabulorXTextDecoration *decoration)
{
	g_return_if_fail (decoration != NULL);
	decoration->hover_render_only = FALSE;
	decoration->hover_clearing = FALSE;
	decoration->hover_inside = FALSE;
}

gboolean
fabulor_xtext_decoration_hover_render_only (
	const FabulorXTextDecoration *decoration)
{
	return decoration != NULL && decoration->hover_render_only;
}

gboolean
fabulor_xtext_decoration_hover_clearing (
	const FabulorXTextDecoration *decoration)
{
	return decoration != NULL && decoration->hover_clearing;
}

gboolean
fabulor_xtext_decoration_hover_inside (
	const FabulorXTextDecoration *decoration)
{
	return decoration != NULL && decoration->hover_inside;
}

void
fabulor_xtext_decoration_set_hover_inside (
	FabulorXTextDecoration *decoration, gboolean inside)
{
	g_return_if_fail (decoration != NULL);
	decoration->hover_inside = inside;
}

void
fabulor_xtext_decoration_suspend_hover (FabulorXTextDecoration *decoration)
{
	g_return_if_fail (decoration != NULL);
	decoration->hover_suspend_depth++;
}

void
fabulor_xtext_decoration_resume_hover (FabulorXTextDecoration *decoration)
{
	g_return_if_fail (decoration != NULL);
	g_return_if_fail (decoration->hover_suspend_depth > 0);
	decoration->hover_suspend_depth--;
}
