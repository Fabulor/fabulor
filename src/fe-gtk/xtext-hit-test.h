/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_XTEXT_HIT_TEST_H
#define FABULOR_XTEXT_HIT_TEST_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct offlen_s
{
	guint16 off;
	guint16 len;
	guint16 emph;
	guint16 width;
} offlen_t;

typedef struct
{
	gchar *word;
	gint type;
	gint match_start;
	gint match_end;
} FabulorXTextHit;

gboolean fabulor_xtext_hit_test_line (gint y, gint pixel_offset,
	gint font_size, gint scroll_line, gint *line);
gboolean fabulor_xtext_hit_test_separator (gboolean enabled, gint indent,
	gint space_width, gint x);
gboolean fabulor_xtext_hit_test_adjust_match (const GSList *runs,
	gint match_start, gint match_end, gint *entry_offset, gint *entry_length);

void fabulor_xtext_hit_init (FabulorXTextHit *hit, gchar *word, gint type,
	gint match_start, gint match_end);
gboolean fabulor_xtext_hit_has_match (const FabulorXTextHit *hit);
gchar *fabulor_xtext_hit_dup_match (const FabulorXTextHit *hit);

G_END_DECLS

#endif
