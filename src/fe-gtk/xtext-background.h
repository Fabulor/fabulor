/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_XTEXT_BACKGROUND_H
#define FABULOR_XTEXT_BACKGROUND_H

#include <cairo.h>
#include <glib.h>

#include "xtext-color.h"
#include "xtext-geometry.h"

G_BEGIN_DECLS

typedef struct _FabulorXTextBackground FabulorXTextBackground;

FabulorXTextBackground *fabulor_xtext_background_new (void);
void fabulor_xtext_background_free (FabulorXTextBackground *background);
void fabulor_xtext_background_set_surface (FabulorXTextBackground *background,
	cairo_surface_t *surface);
gboolean fabulor_xtext_background_has_surface (
	const FabulorXTextBackground *background);
void fabulor_xtext_background_begin_frame (FabulorXTextBackground *background);
void fabulor_xtext_background_end_frame (FabulorXTextBackground *background);
void fabulor_xtext_background_paint (FabulorXTextBackground *background,
	cairo_t *context, const XTextColor *fallback,
	const FabulorXTextGeometry *geometry, gint x, gint y, gint width,
	gint height, gint tile_x, gint tile_y, gint dim_percent);

G_END_DECLS

#endif
