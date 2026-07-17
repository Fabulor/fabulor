/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_XTEXT_SCROLL_COPY_H
#define FABULOR_XTEXT_SCROLL_COPY_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct
{
	gint source_y;
	gint destination_y;
	gint copy_height;
	gint damage_y;
	gint damage_height;
} FabulorXTextScrollCopy;

gboolean fabulor_xtext_scroll_copy_plan (gint overlap, gint height,
	gint font_size, gint descent, gboolean native_capture,
	FabulorXTextScrollCopy *plan);

G_END_DECLS

#endif
