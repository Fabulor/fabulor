/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "xtext-geometry.h"

gboolean
fabulor_xtext_geometry_init (FabulorXTextGeometry *geometry,
	gint width, gint height)
{
	if (!geometry)
		return FALSE;
	geometry->width = 0;
	geometry->height = 0;
	if (width <= 0 || height <= 0)
		return FALSE;
	geometry->width = width;
	geometry->height = height;
	return TRUE;
}

gboolean
fabulor_xtext_geometry_from_widget (GtkWidget *widget,
	FabulorXTextGeometry *geometry)
{
	if (!GTK_IS_WIDGET (widget))
		return fabulor_xtext_geometry_init (geometry, 0, 0);
	return fabulor_xtext_geometry_init (geometry,
		gtk_widget_get_width (widget), gtk_widget_get_height (widget));
}
