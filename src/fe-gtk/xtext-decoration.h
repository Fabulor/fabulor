/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_XTEXT_DECORATION_H
#define FABULOR_XTEXT_DECORATION_H

#include <glib.h>

G_BEGIN_DECLS

typedef union offsets_u
{
	struct offsets_s
	{
		guint16 start;
		guint16 end;
	} o;
	guint32 u;
} offsets_t;

typedef enum
{
	FABULOR_XTEXT_MATCH_START = 1 << 0,
	FABULOR_XTEXT_MATCH_MID = 1 << 1,
	FABULOR_XTEXT_MATCH_END = 1 << 2,
	FABULOR_XTEXT_MATCH_CURRENT = 1 << 3
} FabulorXTextMatch;

typedef struct _FabulorXTextDecoration FabulorXTextDecoration;

FabulorXTextDecoration *fabulor_xtext_decoration_new (void);
void fabulor_xtext_decoration_free (FabulorXTextDecoration *decoration);

gboolean fabulor_xtext_marker_position (gboolean enabled,
	gconstpointer marker_entry, gconstpointer entry, gconstpointer next_entry,
	gint y, gint descent, gint font_size, gint subline_count, gint *render_y);
guint fabulor_xtext_search_match (const GList *marks,
	const GList *current, guint offset);

gboolean fabulor_xtext_decoration_set_hover (
	FabulorXTextDecoration *decoration, gconstpointer entry, gint start,
	gint end);
void fabulor_xtext_decoration_clear_hover (
	FabulorXTextDecoration *decoration);
gboolean fabulor_xtext_decoration_has_hover (
	const FabulorXTextDecoration *decoration);
gboolean fabulor_xtext_decoration_hover_equals (
	const FabulorXTextDecoration *decoration, gconstpointer entry, gint start,
	gint end);
gconstpointer fabulor_xtext_decoration_hover_entry (
	const FabulorXTextDecoration *decoration);
gboolean fabulor_xtext_decoration_hover_contains (
	const FabulorXTextDecoration *decoration, gconstpointer entry, gint offset);
gboolean fabulor_xtext_decoration_hover_starts (
	const FabulorXTextDecoration *decoration, gconstpointer entry, gint offset);
gboolean fabulor_xtext_decoration_hover_ends (
	const FabulorXTextDecoration *decoration, gconstpointer entry, gint offset);

void fabulor_xtext_decoration_begin_hover_render (
	FabulorXTextDecoration *decoration, gboolean clearing);
void fabulor_xtext_decoration_end_hover_render (
	FabulorXTextDecoration *decoration);
gboolean fabulor_xtext_decoration_hover_render_only (
	const FabulorXTextDecoration *decoration);
gboolean fabulor_xtext_decoration_hover_clearing (
	const FabulorXTextDecoration *decoration);
gboolean fabulor_xtext_decoration_hover_inside (
	const FabulorXTextDecoration *decoration);
void fabulor_xtext_decoration_set_hover_inside (
	FabulorXTextDecoration *decoration, gboolean inside);
void fabulor_xtext_decoration_suspend_hover (
	FabulorXTextDecoration *decoration);
void fabulor_xtext_decoration_resume_hover (
	FabulorXTextDecoration *decoration);

G_END_DECLS

#endif
