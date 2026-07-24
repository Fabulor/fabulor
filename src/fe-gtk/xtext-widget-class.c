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


void
fabulor_xtext_widget_accessibility_init (GtkWidget *widget,
	const gchar *label)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (label != NULL && *label != '\0');
	gtk_accessible_update_property (GTK_ACCESSIBLE (widget),
		GTK_ACCESSIBLE_PROPERTY_LABEL, label, -1);
}

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
	gtk_widget_class_set_accessible_role (widget_class,
		GTK_ACCESSIBLE_ROLE_LOG);
	widget_class->measure = xtext_widget_measure;
	widget_class->snapshot = xtext_widget_snapshot;
}
