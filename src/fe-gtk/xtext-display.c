/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "xtext-display.h"

gboolean
fabulor_xtext_font_metrics_init (gint pango_ascent, gint pango_descent,
	gint pango_height, gint pango_scale, gboolean prefer_ink_height,
	FabulorXTextFontMetrics *metrics)
{
	gint ink_height;

	g_return_val_if_fail (metrics != NULL, FALSE);
	metrics->ascent = 0;
	metrics->descent = 0;
	metrics->line_height = 0;
	if (pango_ascent < 0 || pango_descent < 0 || pango_scale <= 0)
		return FALSE;

	metrics->ascent = pango_ascent / pango_scale;
	metrics->descent = pango_descent / pango_scale;
	ink_height = metrics->ascent + metrics->descent;
	if (pango_height >= 0)
		metrics->line_height = (pango_height / pango_scale) + 1;
	if (prefer_ink_height || metrics->line_height <= 0)
		metrics->line_height = ink_height;
	return metrics->line_height > 0;
}

gint
fabulor_xtext_scale_factor (gint scale_factor)
{
	return scale_factor > 0 ? scale_factor : 1;
}

gboolean
fabulor_xtext_inline_image_size (gint line_height, gint scale_factor,
	gint *logical_width, gint *logical_height, gint *device_width,
	gint *device_height)
{
	gint height;
	gint width;
	gint scale = fabulor_xtext_scale_factor (scale_factor);

	if (logical_width)
		*logical_width = 0;
	if (logical_height)
		*logical_height = 0;
	if (device_width)
		*device_width = 0;
	if (device_height)
		*device_height = 0;
	if (line_height <= 0)
		return FALSE;

	height = CLAMP (line_height - 2, 14, 64);
	width = (height * 4) / 3;
	if (scale > G_MAXINT / width || scale > G_MAXINT / height)
		return FALSE;
	if (logical_width)
		*logical_width = width;
	if (logical_height)
		*logical_height = height;
	if (device_width)
		*device_width = width * scale;
	if (device_height)
		*device_height = height * scale;
	return TRUE;
}

gboolean
fabulor_xtext_device_to_logical (gint device_pixels, gint scale_factor,
	gint *logical_pixels)
{
	gint scale = fabulor_xtext_scale_factor (scale_factor);

	g_return_val_if_fail (logical_pixels != NULL, FALSE);
	*logical_pixels = 0;
	if (device_pixels < 0 || device_pixels > G_MAXINT - (scale - 1))
		return FALSE;
	*logical_pixels = (device_pixels + scale - 1) / scale;
	return TRUE;
}

gint
fabulor_xtext_layout_text_width (PangoLayout *layout, const gchar *text,
	gint length)
{
	gint width = 0;

	if (!layout || !text || length <= 0)
		return 0;
	pango_layout_set_text (layout, text, length);
	pango_layout_get_pixel_size (layout, &width, NULL);
	return MAX (width, 0);
}

gint
fabulor_xtext_layout_index_at_x (PangoLayout *layout, const gchar *text,
	gint length, gint x)
{
	const gchar *position;
	gint index = 0;
	gint trailing = 0;
	gint width;

	if (!layout || !text || length <= 0 || x <= 0)
		return 0;

	pango_layout_set_text (layout, text, length);
	pango_layout_get_pixel_size (layout, &width, NULL);
	if (x >= width)
		return length;
	if (!pango_layout_xy_to_index (layout, x * PANGO_SCALE, 0,
		&index, &trailing))
		return x < 0 ? 0 : length;

	index = CLAMP (index, 0, length);
	position = text + index;
	while (trailing > 0 && position < text + length)
	{
		position = g_utf8_next_char (position);
		trailing--;
	}
	return (gint) MIN (position - text, length);
}

void
fabulor_xtext_decoration_positions (gint baseline, gint ascent,
	gint line_height, gint *strike_y, gint *underline_y)
{
	if (strike_y)
		*strike_y = baseline - ascent + (line_height / 2);
	if (underline_y)
		*underline_y = baseline + 1;
}
