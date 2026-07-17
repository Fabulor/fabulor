/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_XTEXT_WIDGET_CLASS_H
#define FABULOR_XTEXT_WIDGET_CLASS_H

#include <gtk/gtk.h>

#include "xtext-render-target.h"

G_BEGIN_DECLS

typedef struct
{
	gint x;
	gint y;
	gint width;
	gint height;
	gint baseline;
} FabulorXTextAllocation;

typedef struct
{
	void (*realize) (GtkWidget *widget);
	void (*unrealize) (GtkWidget *widget);
	void (*allocate) (GtkWidget *widget,
		const FabulorXTextAllocation *allocation);
	void (*render) (GtkWidget *widget, const GdkRectangle *area,
		cairo_t *context);
	FabulorXTextRenderTarget *(*get_render_target) (GtkWidget *widget);
} FabulorXTextWidgetCallbacks;

void fabulor_xtext_widget_measure (GtkOrientation orientation,
	gint *minimum, gint *natural, gint *minimum_baseline,
	gint *natural_baseline);
gboolean fabulor_xtext_widget_width_changed (gint previous_width,
	gint width);
void fabulor_xtext_widget_class_install (GtkWidgetClass *widget_class,
	const FabulorXTextWidgetCallbacks *callbacks);
void fabulor_xtext_widget_accessibility_init (GtkWidget *widget,
	const gchar *label);

G_END_DECLS

#endif
