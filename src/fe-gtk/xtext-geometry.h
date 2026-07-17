/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_XTEXT_GEOMETRY_H
#define FABULOR_XTEXT_GEOMETRY_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct
{
	gint width;
	gint height;
} FabulorXTextGeometry;

gboolean fabulor_xtext_geometry_init (FabulorXTextGeometry *geometry,
	gint width, gint height);
gboolean fabulor_xtext_geometry_from_widget (GtkWidget *widget,
	FabulorXTextGeometry *geometry);

G_END_DECLS

#endif
