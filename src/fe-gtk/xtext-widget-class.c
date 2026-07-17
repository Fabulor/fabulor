/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "xtext-widget-class.h"

#include "xtext-geometry.h"

#define XTEXT_MINIMUM_WIDTH 200
#define XTEXT_MINIMUM_HEIGHT 90

static GQuark
xtext_widget_callbacks_quark (void)
{
	return g_quark_from_static_string ("fabulor-xtext-widget-callbacks");
}

static const FabulorXTextWidgetCallbacks *
xtext_widget_callbacks (GtkWidget *widget)
{
	GType type = G_OBJECT_TYPE (widget);
	const FabulorXTextWidgetCallbacks *callbacks;
	while (type != G_TYPE_INVALID)
	{
		callbacks = g_type_get_qdata (type, xtext_widget_callbacks_quark ());
		if (callbacks)
			return callbacks;
		type = g_type_parent (type);
	}
	return NULL;
}

void
fabulor_xtext_widget_measure (GtkOrientation orientation,
	gint *minimum, gint *natural, gint *minimum_baseline,
	gint *natural_baseline)
{
	gint size = orientation == GTK_ORIENTATION_HORIZONTAL ?
		XTEXT_MINIMUM_WIDTH : XTEXT_MINIMUM_HEIGHT;
	if (minimum)
		*minimum = size;
	if (natural)
		*natural = size;
	if (minimum_baseline)
		*minimum_baseline = -1;
	if (natural_baseline)
		*natural_baseline = -1;
}

gboolean
fabulor_xtext_widget_width_changed (gint previous_width, gint width)
{
	return previous_width != width;
}

static void
xtext_widget_realize (GtkWidget *widget)
{
	const FabulorXTextWidgetCallbacks *callbacks =
		xtext_widget_callbacks (widget);
	callbacks->realize (widget);
}

static void
xtext_widget_unrealize (GtkWidget *widget)
{
	const FabulorXTextWidgetCallbacks *callbacks =
		xtext_widget_callbacks (widget);
	callbacks->unrealize (widget);
}

#if GTK_MAJOR_VERSION >= 4

static void
xtext_widget_measure (GtkWidget *widget, GtkOrientation orientation,
	gint for_size, gint *minimum, gint *natural, gint *minimum_baseline,
	gint *natural_baseline)
{
	(void) widget;
	(void) for_size;
	fabulor_xtext_widget_measure (orientation, minimum, natural,
		minimum_baseline, natural_baseline);
}

static void
xtext_widget_size_allocate (GtkWidget *widget, gint width, gint height,
	gint baseline)
{
	const FabulorXTextWidgetCallbacks *callbacks =
		xtext_widget_callbacks (widget);
	FabulorXTextAllocation allocation = { 0, 0, width, height, baseline };
	callbacks->allocate (widget, &allocation);
}

static void
xtext_widget_snapshot (GtkWidget *widget, GtkSnapshot *snapshot)
{
	const FabulorXTextWidgetCallbacks *callbacks =
		xtext_widget_callbacks (widget);
	FabulorXTextGeometry geometry;
	FabulorXTextRenderTarget *target;
	GdkRectangle area;
	cairo_t *context;

	if (!fabulor_xtext_geometry_from_widget (widget, &geometry))
		return;
	target = callbacks->get_render_target (widget);
	if (!target)
		return;
	context = fabulor_xtext_render_target_begin_snapshot (target, snapshot,
		geometry.width, geometry.height);
	if (!context)
		return;
	area.x = 0;
	area.y = 0;
	area.width = geometry.width;
	area.height = geometry.height;
	callbacks->render (widget, &area, context);
	fabulor_xtext_render_target_end_snapshot (target, context);
}

#else

static void
xtext_widget_get_preferred_width (GtkWidget *widget, gint *minimum,
	gint *natural)
{
	(void) widget;
	fabulor_xtext_widget_measure (GTK_ORIENTATION_HORIZONTAL, minimum,
		natural, NULL, NULL);
}

static void
xtext_widget_get_preferred_height (GtkWidget *widget, gint *minimum,
	gint *natural)
{
	(void) widget;
	fabulor_xtext_widget_measure (GTK_ORIENTATION_VERTICAL, minimum,
		natural, NULL, NULL);
}

static void
xtext_widget_get_preferred_height_for_width (GtkWidget *widget, gint width,
	gint *minimum, gint *natural)
{
	(void) widget;
	(void) width;
	fabulor_xtext_widget_measure (GTK_ORIENTATION_VERTICAL, minimum,
		natural, NULL, NULL);
}

static void
xtext_widget_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	const FabulorXTextWidgetCallbacks *callbacks =
		xtext_widget_callbacks (widget);
	FabulorXTextAllocation xtext_allocation = {
		allocation->x, allocation->y, allocation->width, allocation->height, -1
	};
	callbacks->allocate (widget, &xtext_allocation);
}

static gboolean
xtext_widget_draw (GtkWidget *widget, cairo_t *context)
{
	const FabulorXTextWidgetCallbacks *callbacks =
		xtext_widget_callbacks (widget);
	GdkRectangle area;

	if (!gdk_cairo_get_clip_rectangle (context, &area))
	{
		GtkAllocation allocation;
		gtk_widget_get_allocation (widget, &allocation);
		area.x = 0;
		area.y = 0;
		area.width = allocation.width;
		area.height = allocation.height;
	}
	callbacks->render (widget, &area, context);
	return FALSE;
}

#endif

void
fabulor_xtext_widget_class_install (GtkWidgetClass *widget_class,
	const FabulorXTextWidgetCallbacks *callbacks)
{
	g_return_if_fail (GTK_IS_WIDGET_CLASS (widget_class));
	g_return_if_fail (callbacks && callbacks->realize && callbacks->unrealize &&
		callbacks->allocate && callbacks->render &&
		callbacks->get_render_target);

	/* The caller supplies one static callback table for the widget class. */
	g_type_set_qdata (G_TYPE_FROM_CLASS (widget_class),
		xtext_widget_callbacks_quark (), (gpointer) callbacks);
	widget_class->realize = xtext_widget_realize;
	widget_class->unrealize = xtext_widget_unrealize;
	widget_class->size_allocate = xtext_widget_size_allocate;
#if GTK_MAJOR_VERSION >= 4
	widget_class->measure = xtext_widget_measure;
	widget_class->snapshot = xtext_widget_snapshot;
#else
	widget_class->draw = xtext_widget_draw;
	widget_class->get_preferred_width = xtext_widget_get_preferred_width;
	widget_class->get_preferred_height = xtext_widget_get_preferred_height;
	widget_class->get_preferred_height_for_width =
		xtext_widget_get_preferred_height_for_width;
#endif
}
