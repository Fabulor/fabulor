/* Copyright (C) 2026 Fabulor contributors */
#include <string.h>

#include "window-geometry.h"

typedef struct
{
	GtkWindow *window;
	FabulorWindowGeometryCallback callback;
	gpointer user_data;
	GdkSurface *surface;
	gulong layout_handler;
} FabulorWindowGeometryWatch;

gint
fabulor_pane_clamp_end_size (gint saved_size, gint minimum_size,
	gint pane_width, gint handle_size)
{
	gint available;
	gint normalized;

	available = pane_width - MAX (handle_size, 0);
	if (available < 1)
		return 0;
	normalized = MAX (saved_size, MAX (minimum_size, 1));
	return MIN (normalized, available);
}

gint
fabulor_pane_restore_end_size (gint saved_size, gint fallback_size,
	gint minimum_size, gint pane_width, gint handle_size)
{
	gint available = pane_width - MAX (handle_size, 0);
	gint normalized;

	if (available < 1)
		return 0;

	normalized = fabulor_pane_clamp_end_size (saved_size, minimum_size,
		pane_width, handle_size);
	if (normalized > available / 2)
		return fabulor_pane_clamp_end_size (fallback_size, minimum_size,
			pane_width, handle_size);
	return normalized;
}

void
fabulor_window_geometry_get (GtkWindow *window,
	FabulorWindowGeometry *geometry)
{
	g_return_if_fail (GTK_IS_WINDOW (window));
	g_return_if_fail (geometry != NULL);

	memset (geometry, 0, sizeof (*geometry));
	{
		GdkSurface *surface = gtk_native_get_surface (GTK_NATIVE (window));
		if (GDK_IS_SURFACE (surface))
		{
			geometry->width = gdk_surface_get_width (surface);
			geometry->height = gdk_surface_get_height (surface);
		}
	}
}

static void
window_geometry_layout_cb (GdkSurface *surface, gint width, gint height,
	gpointer user_data)
{
	FabulorWindowGeometryWatch *watch = user_data;
	FabulorWindowGeometry geometry = { 0 };
	(void)surface;
	geometry.width = width;
	geometry.height = height;
	watch->callback (watch->window, &geometry, watch->user_data);
}

static void
window_geometry_detach_surface (FabulorWindowGeometryWatch *watch)
{
	if (!watch->surface)
		return;
	if (watch->layout_handler)
		g_signal_handler_disconnect (watch->surface, watch->layout_handler);
	watch->layout_handler = 0;
	g_clear_object (&watch->surface);
}

static void
window_geometry_attach_surface (FabulorWindowGeometryWatch *watch)
{
	GdkSurface *surface = gtk_native_get_surface (GTK_NATIVE (watch->window));
	window_geometry_detach_surface (watch);
	if (!GDK_IS_SURFACE (surface))
		return;
	watch->surface = g_object_ref (surface);
	watch->layout_handler = g_signal_connect (surface, "layout",
		G_CALLBACK (window_geometry_layout_cb), watch);
}

static void
window_geometry_realize_cb (GtkWidget *widget, gpointer user_data)
{
	(void)widget;
	window_geometry_attach_surface (user_data);
}

static void
window_geometry_unrealize_cb (GtkWidget *widget, gpointer user_data)
{
	(void)widget;
	window_geometry_detach_surface (user_data);
}

static void
window_geometry_watch_free (gpointer data)
{
	FabulorWindowGeometryWatch *watch = data;
	window_geometry_detach_surface (watch);
	g_free (watch);
}

void
fabulor_window_geometry_watch (GtkWindow *window,
	FabulorWindowGeometryCallback callback, gpointer user_data)
{
	GPtrArray *watches;
	FabulorWindowGeometryWatch *watch;

	g_return_if_fail (GTK_IS_WINDOW (window));
	g_return_if_fail (callback != NULL);
	watches = g_object_get_data (G_OBJECT (window),
		"fabulor-window-geometry-watches");
	if (!watches)
	{
		watches = g_ptr_array_new_with_free_func (window_geometry_watch_free);
		g_object_set_data_full (G_OBJECT (window),
			"fabulor-window-geometry-watches", watches,
			(GDestroyNotify)g_ptr_array_unref);
	}
	watch = g_new0 (FabulorWindowGeometryWatch, 1);
	watch->window = window;
	watch->callback = callback;
	watch->user_data = user_data;
	g_ptr_array_add (watches, watch);
	g_signal_connect (window, "realize", G_CALLBACK (window_geometry_realize_cb),
		watch);
	g_signal_connect (window, "unrealize",
		G_CALLBACK (window_geometry_unrealize_cb), watch);
	if (gtk_widget_get_realized (GTK_WIDGET (window)))
		window_geometry_attach_surface (watch);
}
