/* Copyright (C) 2026 Fabulor contributors */
#ifndef FABULOR_WINDOW_GEOMETRY_H
#define FABULOR_WINDOW_GEOMETRY_H

#include <gtk/gtk.h>

typedef struct
{
	gint width;
	gint height;
	gboolean has_position;
	gint x;
	gint y;
} FabulorWindowGeometry;

typedef void (*FabulorWindowGeometryCallback) (GtkWindow *window,
	const FabulorWindowGeometry *geometry, gpointer user_data);

void fabulor_window_geometry_get (GtkWindow *window,
	FabulorWindowGeometry *geometry);
void fabulor_window_geometry_watch (GtkWindow *window,
	FabulorWindowGeometryCallback callback, gpointer user_data);
gint fabulor_pane_clamp_end_size (gint saved_size, gint minimum_size,
	gint pane_width, gint handle_size);
gint fabulor_pane_restore_end_size (gint saved_size, gint fallback_size,
	gint minimum_size, gint pane_width, gint handle_size);

#endif
