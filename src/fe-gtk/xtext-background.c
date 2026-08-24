/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "xtext-background.h"

#define XTEXT_BACKGROUND_MAX_DIMENSION 8192

struct _FabulorXTextBackground
{
	cairo_surface_t *source;
	cairo_surface_t *cache;
	gint cache_width;
	gint cache_height;
	gint cache_tile_x;
	gint cache_tile_y;
	gint cache_dim_percent;
};

static void
fabulor_xtext_background_clear_cache (FabulorXTextBackground *background)
{
	if (background->cache)
	{
		cairo_surface_destroy (background->cache);
		background->cache = NULL;
	}
	background->cache_width = 0;
	background->cache_height = 0;
	background->cache_tile_x = 0;
	background->cache_tile_y = 0;
	background->cache_dim_percent = 0;
}

static void
fabulor_xtext_background_fill (cairo_t *context,
	const XTextColor *color, gint x, gint y, gint width, gint height)
{
	cairo_save (context);
	cairo_set_source_rgba (context, color->red, color->green, color->blue,
		color->alpha);
	cairo_set_operator (context, CAIRO_OPERATOR_SOURCE);
	cairo_rectangle (context, (double) x, (double) y, (double) width,
		(double) height);
	cairo_fill (context);
	cairo_restore (context);
}

static gboolean
fabulor_xtext_background_build_cache (FabulorXTextBackground *background,
	const FabulorXTextGeometry *geometry, const XTextColor *fallback,
	gint tile_x, gint tile_y, gint dim_percent)
{
	cairo_t *context;
	gint width = geometry->width;
	gint height = geometry->height;

	if (background->cache && background->cache_width == width &&
		background->cache_height == height &&
		background->cache_tile_x == tile_x &&
		background->cache_tile_y == tile_y &&
		background->cache_dim_percent == dim_percent)
		return TRUE;

	fabulor_xtext_background_clear_cache (background);
	background->cache = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
		width, height);
	if (cairo_surface_status (background->cache) != CAIRO_STATUS_SUCCESS)
	{
		fabulor_xtext_background_clear_cache (background);
		return FALSE;
	}

	context = cairo_create (background->cache);
	if (cairo_status (context) != CAIRO_STATUS_SUCCESS)
	{
		cairo_destroy (context);
		fabulor_xtext_background_clear_cache (background);
		return FALSE;
	}
	if (cairo_surface_get_type (background->source) == CAIRO_SURFACE_TYPE_IMAGE)
	{
		gint source_width = cairo_image_surface_get_width (background->source);
		gint source_height = cairo_image_surface_get_height (background->source);

		if (source_width > 0 && source_height > 0)
		{
			double scale_x = (double) width / (double) source_width;
			double scale_y = (double) height / (double) source_height;
			double scale = MAX (scale_x, scale_y);
			double draw_width = source_width * scale;
			double draw_height = source_height * scale;
			double draw_x = ((double) width - draw_width) / 2.0;
			double draw_y = ((double) height - draw_height) / 2.0;

			fabulor_xtext_background_fill (context, fallback, 0, 0,
				width, height);
			cairo_save (context);
			cairo_rectangle (context, 0.0, 0.0, (double) width,
				(double) height);
			cairo_clip (context);
			cairo_translate (context, draw_x, draw_y);
			cairo_scale (context, scale, scale);
			cairo_set_source_surface (context, background->source, 0.0, 0.0);
			cairo_pattern_set_extend (cairo_get_source (context),
				CAIRO_EXTEND_PAD);
			cairo_rectangle (context, 0.0, 0.0, (double) source_width,
				(double) source_height);
			cairo_fill (context);
			cairo_restore (context);
		}
		else
		{
			cairo_set_source_surface (context, background->source,
				(double) tile_x, (double) tile_y);
			cairo_pattern_set_extend (cairo_get_source (context),
				CAIRO_EXTEND_REPEAT);
			cairo_paint (context);
		}
	}
	else
	{
		cairo_set_source_surface (context, background->source,
			(double) tile_x, (double) tile_y);
		cairo_pattern_set_extend (cairo_get_source (context),
			CAIRO_EXTEND_REPEAT);
		cairo_paint (context);
	}
	if (dim_percent > 0)
	{
		double opacity = (double) CLAMP (dim_percent, 0, 100) / 100.0;

		cairo_set_source_rgba (context, fallback->red, fallback->green,
			fallback->blue, opacity);
		cairo_set_operator (context, CAIRO_OPERATOR_OVER);
		cairo_paint (context);
	}
	cairo_destroy (context);

	background->cache_width = width;
	background->cache_height = height;
	background->cache_tile_x = tile_x;
	background->cache_tile_y = tile_y;
	background->cache_dim_percent = dim_percent;
	return TRUE;
}

FabulorXTextBackground *
fabulor_xtext_background_new (void)
{
	return g_new0 (FabulorXTextBackground, 1);
}

void
fabulor_xtext_background_free (FabulorXTextBackground *background)
{
	if (background == NULL)
		return;
	fabulor_xtext_background_clear_cache (background);
	if (background->source)
		cairo_surface_destroy (background->source);
	g_free (background);
}

void
fabulor_xtext_background_set_surface (FabulorXTextBackground *background,
	cairo_surface_t *surface)
{
	g_return_if_fail (background != NULL);

	fabulor_xtext_background_clear_cache (background);
	if (background->source)
		cairo_surface_destroy (background->source);
	background->source = surface ? cairo_surface_reference (surface) : NULL;
}

gboolean
fabulor_xtext_background_has_surface (
	const FabulorXTextBackground *background)
{
	return background != NULL && background->source != NULL;
}

void
fabulor_xtext_background_begin_frame (FabulorXTextBackground *background)
{
	g_return_if_fail (background != NULL);
	fabulor_xtext_background_clear_cache (background);
}

void
fabulor_xtext_background_end_frame (FabulorXTextBackground *background)
{
	g_return_if_fail (background != NULL);
	fabulor_xtext_background_clear_cache (background);
}

void
fabulor_xtext_background_paint (FabulorXTextBackground *background,
	cairo_t *context, const XTextColor *fallback,
	const FabulorXTextGeometry *geometry, gint x, gint y, gint width,
	gint height, gint tile_x, gint tile_y, gint dim_percent)
{
	g_return_if_fail (background != NULL);
	g_return_if_fail (context != NULL);
	g_return_if_fail (fallback != NULL);
	g_return_if_fail (geometry != NULL);

	if (width <= 0 || height <= 0)
		return;
	if (background->source == NULL ||
		cairo_surface_status (background->source) != CAIRO_STATUS_SUCCESS ||
		geometry->width < 1 || geometry->height < 1 ||
		geometry->width > XTEXT_BACKGROUND_MAX_DIMENSION ||
		geometry->height > XTEXT_BACKGROUND_MAX_DIMENSION ||
		!fabulor_xtext_background_build_cache (background, geometry,
			fallback, tile_x, tile_y, dim_percent))
	{
		fabulor_xtext_background_fill (context, fallback, x, y, width,
			height);
		return;
	}

	cairo_set_source_surface (context, background->cache, 0.0, 0.0);
	cairo_rectangle (context, (double) x, (double) y, (double) width,
		(double) height);
	cairo_fill (context);
}
