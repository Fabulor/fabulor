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

#if GTK_MAJOR_VERSION >= 4
static inline gint
fabulor_gtk_margin_with_padding (gint margin, guint padding)
{
	if (padding > (guint) (G_MAXINT - margin))
		return G_MAXINT;

	return margin + (gint) padding;
}
#endif

static inline void
fabulor_gtk_box_append (GtkBox *box, GtkWidget *child, gboolean expand,
						gboolean fill, guint padding)
{
	g_return_if_fail (GTK_IS_BOX (box));
	g_return_if_fail (GTK_IS_WIDGET (child));

#if GTK_MAJOR_VERSION >= 4
	if (gtk_orientable_get_orientation (GTK_ORIENTABLE (box)) ==
		GTK_ORIENTATION_HORIZONTAL)
	{
		if (expand)
			gtk_widget_set_hexpand (child, TRUE);
		if (!fill && gtk_widget_get_halign (child) == GTK_ALIGN_FILL)
			gtk_widget_set_halign (child, GTK_ALIGN_CENTER);
		gtk_widget_set_margin_start (child,
			fabulor_gtk_margin_with_padding (gtk_widget_get_margin_start (child), padding));
		gtk_widget_set_margin_end (child,
			fabulor_gtk_margin_with_padding (gtk_widget_get_margin_end (child), padding));
	}
	else
	{
		if (expand)
			gtk_widget_set_vexpand (child, TRUE);
		if (!fill && gtk_widget_get_valign (child) == GTK_ALIGN_FILL)
			gtk_widget_set_valign (child, GTK_ALIGN_CENTER);
		gtk_widget_set_margin_top (child,
			fabulor_gtk_margin_with_padding (gtk_widget_get_margin_top (child), padding));
		gtk_widget_set_margin_bottom (child,
			fabulor_gtk_margin_with_padding (gtk_widget_get_margin_bottom (child), padding));
	}

	gtk_box_append (box, child);
#else
	gtk_box_pack_start (box, child, expand, fill, padding);
#endif
}

static inline void
fabulor_gtk_box_append_trailing_pair (GtkBox *box, GtkWidget *leading,
									 GtkWidget *trailing)
{
	g_return_if_fail (GTK_IS_BOX (box));
	g_return_if_fail (GTK_IS_WIDGET (leading));
	g_return_if_fail (GTK_IS_WIDGET (trailing));

#if GTK_MAJOR_VERSION >= 4
	/* GTK3 pack_end reverses call order and keeps this pair at the box end. */
	gtk_widget_set_halign (GTK_WIDGET (box), GTK_ALIGN_END);
	gtk_box_append (box, leading);
	gtk_box_append (box, trailing);
#else
	gtk_box_pack_end (box, trailing, FALSE, FALSE, 0);
	gtk_box_pack_end (box, leading, FALSE, FALSE, 0);
#endif
}

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
fabulor_gtk_widget_reveal_tree (GtkWidget *widget)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));

#if GTK_MAJOR_VERSION >= 4
	/* GTK4 descendants are visible by default once the completed root is shown. */
	gtk_widget_set_visible (widget, TRUE);
#else
	gtk_widget_show_all (widget);
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

static inline void
fabulor_gtk_dialog_destroy_on_response (GtkDialog *dialog, gint response_id,
										gpointer user_data)
{
	(void) response_id;
	(void) user_data;

	fabulor_gtk_window_destroy (GTK_WINDOW (dialog));
}

G_END_DECLS

#endif
