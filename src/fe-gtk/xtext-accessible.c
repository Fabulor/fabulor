/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "xtext-accessible.h"

#include <pango/pango.h>
#include <string.h>

struct _FabulorXTextAccessible
{
	gchar *text;
	guint length;
	gboolean observed;
	FabulorXTextAccessibleRefreshFunc refresh;
	gpointer user_data;
};

static GQuark
xtext_accessible_quark (void)
{
	return g_quark_from_static_string ("fabulor-xtext-accessible");
}

static void
xtext_accessible_ensure_current (FabulorXTextAccessible *accessible)
{
	if (!accessible)
		return;
	accessible->observed = TRUE;
	if (accessible->refresh)
		accessible->refresh (accessible, accessible->user_data);
}

static gchar *
xtext_accessible_bounded_text (const gchar *text)
{
	gchar *valid = g_utf8_make_valid (text ? text : "", -1);
	gsize bytes = strlen (valid);
	gchar *start;
	gchar *line;
	gchar *bounded;

	if (bytes <= FABULOR_XTEXT_ACCESSIBLE_MAX_BYTES)
		return valid;
	start = valid + bytes - FABULOR_XTEXT_ACCESSIBLE_MAX_BYTES;
	while ((*(const guchar *) start & 0xc0) == 0x80)
		start++;
	line = strchr (start, '\n');
	if (line && line[1] != '\0')
		start = line + 1;
	bounded = g_strdup (start);
	g_free (valid);
	return bounded;
}

static void
xtext_accessible_change_init (FabulorXTextAccessibleChange *change)
{
	if (change)
		memset (change, 0, sizeof (*change));
}

FabulorXTextAccessible *
fabulor_xtext_accessible_new (FabulorXTextAccessibleRefreshFunc refresh,
	gpointer user_data)
{
	FabulorXTextAccessible *accessible = g_new0 (FabulorXTextAccessible, 1);

	accessible->text = g_strdup ("");
	accessible->refresh = refresh;
	accessible->user_data = user_data;
	return accessible;
}

void
fabulor_xtext_accessible_free (FabulorXTextAccessible *accessible)
{
	if (!accessible)
		return;
	g_free (accessible->text);
	g_free (accessible);
}

gboolean
fabulor_xtext_accessible_replace (FabulorXTextAccessible *accessible,
	const gchar *text, FabulorXTextAccessibleChange *change)
{
	gunichar *old_chars;
	gunichar *new_chars;
	glong old_length;
	glong new_length;
	glong prefix = 0;
	glong suffix = 0;
	gchar *bounded;

	g_return_val_if_fail (accessible != NULL, FALSE);
	xtext_accessible_change_init (change);
	bounded = xtext_accessible_bounded_text (text);
	if (g_strcmp0 (accessible->text, bounded) == 0)
	{
		g_free (bounded);
		return FALSE;
	}
	old_chars = g_utf8_to_ucs4_fast (accessible->text, -1, &old_length);
	new_chars = g_utf8_to_ucs4_fast (bounded, -1, &new_length);
	while (prefix < old_length && prefix < new_length &&
		old_chars[prefix] == new_chars[prefix])
		prefix++;
	while (suffix < old_length - prefix && suffix < new_length - prefix &&
		old_chars[old_length - suffix - 1] ==
		new_chars[new_length - suffix - 1])
		suffix++;
	if (change)
	{
		change->remove_start = (guint) prefix;
		change->remove_end = (guint) (old_length - suffix);
		change->insert_start = (guint) prefix;
		change->insert_end = (guint) (new_length - suffix);
	}
	g_free (old_chars);
	g_free (new_chars);
	g_free (accessible->text);
	accessible->text = bounded;
	accessible->length = (guint) new_length;
	return TRUE;
}

guint
fabulor_xtext_accessible_length (FabulorXTextAccessible *accessible)
{
	if (!accessible)
		return 0;
	xtext_accessible_ensure_current (accessible);
	return accessible->length;
}

gboolean
fabulor_xtext_accessible_is_observed (FabulorXTextAccessible *accessible)
{
	return accessible && accessible->observed;
}

GBytes *
fabulor_xtext_accessible_contents (FabulorXTextAccessible *accessible,
	guint start, guint end)
{
	const gchar *begin;
	const gchar *finish;

	if (!accessible)
		return g_bytes_new_static ("", 0);
	xtext_accessible_ensure_current (accessible);
	start = MIN (start, accessible->length);
	if (end == G_MAXUINT)
		end = accessible->length;
	end = CLAMP (end, start, accessible->length);
	begin = g_utf8_offset_to_pointer (accessible->text, start);
	finish = g_utf8_offset_to_pointer (begin, end - start);
	return g_bytes_new (begin, finish - begin);
}

static void
xtext_accessible_line_range (FabulorXTextAccessible *accessible, guint offset,
	guint *start, guint *end)
{
	const gchar *at = g_utf8_offset_to_pointer (accessible->text, offset);
	const gchar *begin = at;
	const gchar *finish;

	while (begin > accessible->text && begin[-1] != '\n')
		begin = g_utf8_prev_char (begin);
	finish = strchr (at, '\n');
	if (finish)
		finish++;
	else
		finish = accessible->text + strlen (accessible->text);
	*start = (guint) g_utf8_pointer_to_offset (accessible->text, begin);
	*end = (guint) g_utf8_pointer_to_offset (accessible->text, finish);
}

static void
xtext_accessible_boundary_range (FabulorXTextAccessible *accessible,
	guint offset, gboolean sentence, guint *start, guint *end)
{
	PangoLogAttr *attrs = g_new0 (PangoLogAttr, accessible->length + 1);
	guint i;

	pango_get_log_attrs (accessible->text, -1, -1, NULL, attrs,
		accessible->length + 1);
	*start = 0;
	for (i = offset; i > 0; i--)
	{
		if ((sentence && attrs[i].is_sentence_start) ||
			(!sentence && attrs[i].is_word_start))
		{
			*start = i;
			break;
		}
	}
	*end = accessible->length;
	for (i = MAX (offset + 1, *start + 1); i <= accessible->length; i++)
	{
		if ((sentence && attrs[i].is_sentence_start) ||
			(!sentence && attrs[i].is_word_start))
		{
			*end = i;
			break;
		}
	}
	g_free (attrs);
}

GBytes *
fabulor_xtext_accessible_contents_at (FabulorXTextAccessible *accessible,
	guint offset, FabulorXTextAccessibleGranularity granularity,
	guint *start, guint *end)
{
	guint range_start = 0;
	guint range_end = 0;

	if (!accessible)
	{
		if (start)
			*start = 0;
		if (end)
			*end = 0;
		return g_bytes_new_static ("", 0);
	}
	xtext_accessible_ensure_current (accessible);
	offset = MIN (offset, accessible->length);
	if (offset < accessible->length)
	{
		switch (granularity)
		{
		case FABULOR_XTEXT_ACCESSIBLE_CHARACTER:
			range_start = offset;
			range_end = offset + 1;
			break;
		case FABULOR_XTEXT_ACCESSIBLE_WORD:
			xtext_accessible_boundary_range (accessible, offset, FALSE,
				&range_start, &range_end);
			break;
		case FABULOR_XTEXT_ACCESSIBLE_SENTENCE:
			xtext_accessible_boundary_range (accessible, offset, TRUE,
				&range_start, &range_end);
			break;
		case FABULOR_XTEXT_ACCESSIBLE_LINE:
		case FABULOR_XTEXT_ACCESSIBLE_PARAGRAPH:
			xtext_accessible_line_range (accessible, offset, &range_start,
				&range_end);
			break;
		default:
			break;
		}
	}
	if (start)
		*start = range_start;
	if (end)
		*end = range_end;
	return fabulor_xtext_accessible_contents (accessible, range_start,
		range_end);
}

void
fabulor_xtext_accessible_attach (GtkWidget *widget,
	FabulorXTextAccessible *accessible)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_object_set_qdata (G_OBJECT (widget), xtext_accessible_quark (), accessible);
}

void
fabulor_xtext_accessible_notify (GtkWidget *widget,
	const FabulorXTextAccessibleChange *change)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (change != NULL);
#if GTK_MAJOR_VERSION >= 4
	g_return_if_fail (GTK_IS_ACCESSIBLE_TEXT (widget));
	if (change->remove_end > change->remove_start)
		gtk_accessible_text_update_contents (GTK_ACCESSIBLE_TEXT (widget),
			GTK_ACCESSIBLE_TEXT_CONTENT_CHANGE_REMOVE, change->remove_start,
			change->remove_end);
	if (change->insert_end > change->insert_start)
		gtk_accessible_text_update_contents (GTK_ACCESSIBLE_TEXT (widget),
			GTK_ACCESSIBLE_TEXT_CONTENT_CHANGE_INSERT, change->insert_start,
			change->insert_end);
	gtk_accessible_text_update_caret_position (GTK_ACCESSIBLE_TEXT (widget));
#else
	(void) change;
#endif
}

#if GTK_MAJOR_VERSION >= 4

static FabulorXTextAccessible *
xtext_accessible_from_interface (GtkAccessibleText *self)
{
	FabulorXTextAccessible *accessible = g_object_get_qdata (G_OBJECT (self),
		xtext_accessible_quark ());

	if (accessible)
		(void) fabulor_xtext_accessible_length (accessible);
	return accessible;
}

static GBytes *
xtext_accessible_get_contents (GtkAccessibleText *self, guint start, guint end)
{
	return fabulor_xtext_accessible_contents (
		xtext_accessible_from_interface (self), start, end);
}

static GBytes *
xtext_accessible_get_contents_at (GtkAccessibleText *self, guint offset,
	GtkAccessibleTextGranularity granularity, guint *start, guint *end)
{
	return fabulor_xtext_accessible_contents_at (
		xtext_accessible_from_interface (self), offset,
		(FabulorXTextAccessibleGranularity) granularity, start, end);
}

static guint
xtext_accessible_get_caret_position (GtkAccessibleText *self)
{
	return fabulor_xtext_accessible_length (
		xtext_accessible_from_interface (self));
}

static gboolean
xtext_accessible_get_selection (GtkAccessibleText *self, gsize *n_ranges,
	GtkAccessibleTextRange **ranges)
{
	(void) xtext_accessible_from_interface (self);
	*n_ranges = 0;
	if (ranges)
		*ranges = NULL;
	return FALSE;
}

static gboolean
xtext_accessible_get_attributes (GtkAccessibleText *self, guint offset,
	gsize *n_ranges, GtkAccessibleTextRange **ranges, gchar ***names,
	gchar ***values)
{
	(void) xtext_accessible_from_interface (self);
	(void) offset;
	*n_ranges = 0;
	if (ranges)
		*ranges = NULL;
	if (names)
		*names = NULL;
	if (values)
		*values = NULL;
	return FALSE;
}

static void
xtext_accessible_get_default_attributes (GtkAccessibleText *self,
	gchar ***names, gchar ***values)
{
	(void) xtext_accessible_from_interface (self);
	if (names)
		*names = NULL;
	if (values)
		*values = NULL;
}

static gboolean
xtext_accessible_get_extents (GtkAccessibleText *self, guint start, guint end,
	graphene_rect_t *extents)
{
	(void) xtext_accessible_from_interface (self);
	(void) start;
	(void) end;
	(void) extents;
	return FALSE;
}

static gboolean
xtext_accessible_get_offset (GtkAccessibleText *self,
	const graphene_point_t *point, guint *offset)
{
	(void) xtext_accessible_from_interface (self);
	(void) point;
	if (offset)
		*offset = 0;
	return FALSE;
}

static gboolean
xtext_accessible_set_caret_position (GtkAccessibleText *self, guint offset)
{
	(void) xtext_accessible_from_interface (self);
	(void) offset;
	return FALSE;
}

static gboolean
xtext_accessible_set_selection (GtkAccessibleText *self, gsize index,
	GtkAccessibleTextRange *range)
{
	(void) xtext_accessible_from_interface (self);
	(void) index;
	(void) range;
	return FALSE;
}

void
fabulor_xtext_accessible_text_interface_init (GtkAccessibleTextInterface *iface)
{
	iface->get_contents = xtext_accessible_get_contents;
	iface->get_contents_at = xtext_accessible_get_contents_at;
	iface->get_caret_position = xtext_accessible_get_caret_position;
	iface->get_selection = xtext_accessible_get_selection;
	iface->get_attributes = xtext_accessible_get_attributes;
	iface->get_default_attributes = xtext_accessible_get_default_attributes;
	iface->get_extents = xtext_accessible_get_extents;
	iface->get_offset = xtext_accessible_get_offset;
	iface->set_caret_position = xtext_accessible_set_caret_position;
	iface->set_selection = xtext_accessible_set_selection;
}

#endif
