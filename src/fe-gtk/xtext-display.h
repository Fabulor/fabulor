/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_XTEXT_DISPLAY_H
#define FABULOR_XTEXT_DISPLAY_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct
{
	gint ascent;
	gint descent;
	gint line_height;
} FabulorXTextFontMetrics;

gboolean fabulor_xtext_font_metrics_init (gint pango_ascent,
	gint pango_descent, gint pango_height, gint pango_scale,
	gboolean prefer_ink_height, FabulorXTextFontMetrics *metrics);
gboolean fabulor_xtext_inline_image_size (gint line_height,
	gint scale_factor, gint *logical_width, gint *logical_height,
	gint *device_width, gint *device_height);
gboolean fabulor_xtext_device_to_logical (gint device_pixels,
	gint scale_factor, gint *logical_pixels);
gint fabulor_xtext_scale_factor (gint scale_factor);
void fabulor_xtext_decoration_positions (gint baseline, gint ascent,
	gint line_height, gint *strike_y, gint *underline_y);

G_END_DECLS

#endif
