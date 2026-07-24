/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_XTEXT_RENDER_TARGET_H
#define FABULOR_XTEXT_RENDER_TARGET_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorXTextRenderTarget FabulorXTextRenderTarget;

FabulorXTextRenderTarget *fabulor_xtext_render_target_new (void);
void fabulor_xtext_render_target_free (FabulorXTextRenderTarget *target);

void fabulor_xtext_render_target_set_surface (
	FabulorXTextRenderTarget *target, cairo_surface_t *surface);

cairo_t *fabulor_xtext_render_target_exchange_context (
	FabulorXTextRenderTarget *target, cairo_t *context);
gboolean fabulor_xtext_render_target_has_active_context (
	FabulorXTextRenderTarget *target);
cairo_t *fabulor_xtext_render_target_create_context (
	FabulorXTextRenderTarget *target);

cairo_t *fabulor_xtext_render_target_begin_snapshot (
	FabulorXTextRenderTarget *target, GtkSnapshot *snapshot,
	gint width, gint height);
void fabulor_xtext_render_target_end_snapshot (
	FabulorXTextRenderTarget *target, cairo_t *context);

G_END_DECLS

#endif
