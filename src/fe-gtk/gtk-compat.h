/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_GTK_COMPAT_H
#define FABULOR_GTK_COMPAT_H

#include <gtk/gtk.h>

#if GTK_MAJOR_VERSION != 3 && GTK_MAJOR_VERSION != 4
#error Fabulor GTK compatibility helpers support GTK 3 and GTK 4 only.
#endif

G_BEGIN_DECLS

static inline void
fabulor_gtk_window_set_child (GtkWindow *window, GtkWidget *child)
{
	g_return_if_fail (GTK_IS_WINDOW (window));
	g_return_if_fail (GTK_IS_WIDGET (child));

#if GTK_MAJOR_VERSION >= 4
	gtk_window_set_child (window, child);
#else
	gtk_container_add (GTK_CONTAINER (window), child);
#endif
}

static inline void
fabulor_gtk_scrolled_window_set_child (GtkScrolledWindow *window,
										GtkWidget *child)
{
	g_return_if_fail (GTK_IS_SCROLLED_WINDOW (window));
	g_return_if_fail (GTK_IS_WIDGET (child));

#if GTK_MAJOR_VERSION >= 4
	gtk_scrolled_window_set_child (window, child);
#else
	gtk_container_add (GTK_CONTAINER (window), child);
#endif
}

static inline void
fabulor_gtk_frame_set_child (GtkFrame *frame, GtkWidget *child)
{
	g_return_if_fail (GTK_IS_FRAME (frame));
	g_return_if_fail (GTK_IS_WIDGET (child));

#if GTK_MAJOR_VERSION >= 4
	gtk_frame_set_child (frame, child);
#else
	gtk_container_add (GTK_CONTAINER (frame), child);
#endif
}

static inline void
fabulor_gtk_button_set_child (GtkButton *button, GtkWidget *child)
{
	g_return_if_fail (GTK_IS_BUTTON (button));
	g_return_if_fail (GTK_IS_WIDGET (child));

#if GTK_MAJOR_VERSION >= 4
	gtk_button_set_child (button, child);
#else
	gtk_container_add (GTK_CONTAINER (button), child);
#endif
}

static inline void
fabulor_gtk_overlay_set_child (GtkOverlay *overlay, GtkWidget *child)
{
	g_return_if_fail (GTK_IS_OVERLAY (overlay));
	g_return_if_fail (GTK_IS_WIDGET (child));

#if GTK_MAJOR_VERSION >= 4
	gtk_overlay_set_child (overlay, child);
#else
	gtk_container_add (GTK_CONTAINER (overlay), child);
#endif
}

static inline void
fabulor_gtk_popover_set_child (GtkPopover *popover, GtkWidget *child)
{
	g_return_if_fail (GTK_IS_POPOVER (popover));
	g_return_if_fail (GTK_IS_WIDGET (child));

#if GTK_MAJOR_VERSION >= 4
	gtk_popover_set_child (popover, child);
#else
	gtk_container_add (GTK_CONTAINER (popover), child);
#endif
}

static inline void
fabulor_gtk_window_destroy (GtkWindow *window)
{
	g_return_if_fail (GTK_IS_WINDOW (window));

#if GTK_MAJOR_VERSION >= 4
	gtk_window_destroy (window);
#else
	gtk_widget_destroy (GTK_WIDGET (window));
#endif
}

G_END_DECLS

#endif
