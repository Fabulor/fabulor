/* Copyright (C) 2026 Fabulor contributors */
#include <string.h>

#include "window-geometry.h"

typedef struct
{
	GtkWindow *window;
	FabulorWindowGeometryCallback callback;
	gpointer user_data;
#if GTK_MAJOR_VERSION >= 4
	GdkSurface *surface;
	gulong layout_handler;
#endif
} FabulorWindowGeometryWatch;

void
fabulor_window_geometry_get (GtkWindow *window,
	FabulorWindowGeometry *geometry)
{
	g_return_if_fail (GTK_IS_WINDOW (window));
	g_return_if_fail (geometry != NULL);

	memset (geometry, 0, sizeof (*geometry));
#if GTK_MAJOR_VERSION >= 4
	{
		GdkSurface *surface = gtk_native_get_surface (GTK_NATIVE (window));
		if (GDK_IS_SURFACE (surface))
		{
			geometry->width = gdk_surface_get_width (surface);
			geometry->height = gdk_surface_get_height (surface);
		}
	}
#else
	gtk_window_get_size (window, &geometry->width, &geometry->height);
	gtk_window_get_position (window, &geometry->x, &geometry->y);
	geometry->has_position = TRUE;
#endif
}

#if GTK_MAJOR_VERSION >= 4
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
#else
static gboolean
window_geometry_configure_cb (GtkWidget *widget, GdkEventConfigure *event,
	gpointer user_data)
{
	FabulorWindowGeometryWatch *watch = user_data;
	FabulorWindowGeometry geometry;
	(void)widget;
	fabulor_window_geometry_get (watch->window, &geometry);
	if (event->width > 0)
		geometry.width = event->width;
	if (event->height > 0)
		geometry.height = event->height;
	watch->callback (watch->window, &geometry, watch->user_data);
	return FALSE;
}
#endif

static void
window_geometry_watch_free (gpointer data)
{
	FabulorWindowGeometryWatch *watch = data;
#if GTK_MAJOR_VERSION >= 4
	window_geometry_detach_surface (watch);
#endif
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
#if GTK_MAJOR_VERSION >= 4
	g_signal_connect (window, "realize", G_CALLBACK (window_geometry_realize_cb),
		watch);
	g_signal_connect (window, "unrealize",
		G_CALLBACK (window_geometry_unrealize_cb), watch);
	if (gtk_widget_get_realized (GTK_WIDGET (window)))
		window_geometry_attach_surface (watch);
#else
	g_signal_connect (window, "configure-event",
		G_CALLBACK (window_geometry_configure_cb), watch);
#endif
}
