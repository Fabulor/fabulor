/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "xtext-render-target.h"

struct _FabulorXTextRenderTarget
{
	cairo_surface_t *surface;
	cairo_t *active_context;
};

FabulorXTextRenderTarget *
fabulor_xtext_render_target_new (void)
{
	return g_new0 (FabulorXTextRenderTarget, 1);
}

void
fabulor_xtext_render_target_free (FabulorXTextRenderTarget *target)
{
	if (!target)
		return;
	g_warn_if_fail (target->active_context == NULL);
	g_free (target);
}

void
fabulor_xtext_render_target_set_surface (FabulorXTextRenderTarget *target,
	cairo_surface_t *surface)
{
	g_return_if_fail (target != NULL);
	target->surface = surface;
}


cairo_t *
fabulor_xtext_render_target_exchange_context (
	FabulorXTextRenderTarget *target, cairo_t *context)
{
	cairo_t *previous;
	g_return_val_if_fail (target != NULL, NULL);
	previous = target->active_context;
	target->active_context = context;
	return previous;
}

gboolean
fabulor_xtext_render_target_has_active_context (
	FabulorXTextRenderTarget *target)
{
	g_return_val_if_fail (target != NULL, FALSE);
	return target->active_context != NULL;
}

cairo_t *
fabulor_xtext_render_target_create_context (FabulorXTextRenderTarget *target)
{
	g_return_val_if_fail (target != NULL, NULL);
	if (target->surface)
		return cairo_create (target->surface);
	if (target->active_context)
		return cairo_reference (target->active_context);
	return NULL;
}

cairo_t *
fabulor_xtext_render_target_begin_snapshot (FabulorXTextRenderTarget *target,
	GtkSnapshot *snapshot, gint width, gint height)
{
	graphene_rect_t bounds;
	cairo_t *context;
	g_return_val_if_fail (target && GTK_IS_SNAPSHOT (snapshot), NULL);
	g_return_val_if_fail (width > 0 && height > 0, NULL);
	g_return_val_if_fail (target->active_context == NULL, NULL);
	graphene_rect_init (&bounds, 0.0f, 0.0f, (float) width, (float) height);
	context = gtk_snapshot_append_cairo (snapshot, &bounds);
	target->active_context = context;
	return context;
}

void
fabulor_xtext_render_target_end_snapshot (FabulorXTextRenderTarget *target,
	cairo_t *context)
{
	g_return_if_fail (target && context);
	g_return_if_fail (target->active_context == context);
	target->active_context = NULL;
	cairo_destroy (context);
}
