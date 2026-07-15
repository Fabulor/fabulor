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
fabulor_gtk_horizontal_box_append_trailing (GtkBox *box, GtkWidget *child)
{
	g_return_if_fail (GTK_IS_BOX (box));
	g_return_if_fail (GTK_IS_WIDGET (child));
	g_return_if_fail (gtk_orientable_get_orientation (GTK_ORIENTABLE (box)) ==
					  GTK_ORIENTATION_HORIZONTAL);

#if GTK_MAJOR_VERSION >= 4
	gtk_widget_set_hexpand (child, TRUE);
	gtk_widget_set_halign (child, GTK_ALIGN_END);
	gtk_box_append (box, child);
#else
	gtk_box_pack_end (box, child, FALSE, FALSE, 0);
#endif
}

static inline void
fabulor_gtk_box_insert_before_trailing (GtkBox *box, GtkWidget *child,
										 GtkWidget *trailing)
{
	g_return_if_fail (GTK_IS_BOX (box));
	g_return_if_fail (GTK_IS_WIDGET (child));
	g_return_if_fail (GTK_IS_WIDGET (trailing));
	g_return_if_fail (gtk_widget_get_parent (trailing) == GTK_WIDGET (box));

#if GTK_MAJOR_VERSION >= 4
	GtkWidget *previous = gtk_widget_get_prev_sibling (trailing);

	if (previous)
		gtk_box_insert_child_after (box, child, previous);
	else
		gtk_box_prepend (box, child);
#else
	gtk_box_pack_start (box, child, FALSE, FALSE, 0);
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
fabulor_gtk_box_remove_child (GtkBox *box, GtkWidget *child)
{
	g_return_if_fail (GTK_IS_BOX (box));
	g_return_if_fail (GTK_IS_WIDGET (child));
	g_return_if_fail (gtk_widget_get_parent (child) == GTK_WIDGET (box));

#if GTK_MAJOR_VERSION >= 4
	gtk_box_remove (box, child);
#else
	gtk_widget_destroy (child);
#endif
}

static inline void
fabulor_gtk_copy_text_to_clipboards (GtkWidget *widget, const gchar *text)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (text != NULL);

#if GTK_MAJOR_VERSION >= 4
	GdkDisplay *display = gtk_widget_get_display (widget);
	GdkClipboard *clipboard;
	GdkClipboard *primary;

	if (!display)
		return;

	clipboard = gdk_display_get_clipboard (display);
	primary = gdk_display_get_primary_clipboard (display);
	gdk_clipboard_set_text (clipboard, text);
	if (primary && primary != clipboard)
		gdk_clipboard_set_text (primary, text);
#else
	GtkWidget *window = gtk_widget_get_toplevel (widget);
	GtkClipboard *clipboard;
	GtkClipboard *primary;

	if (!gtk_widget_is_toplevel (window))
		return;

	clipboard = gtk_widget_get_clipboard (window, GDK_SELECTION_CLIPBOARD);
	primary = gtk_widget_get_clipboard (window, GDK_SELECTION_PRIMARY);
	gtk_clipboard_set_text (clipboard, text, -1);
	if (primary != clipboard)
		gtk_clipboard_set_text (primary, text, -1);
#endif
}

typedef void (*FabulorGtkWidgetInteractionFunc) (GtkWidget *widget,
													gpointer user_data);

typedef struct
{
	FabulorGtkWidgetInteractionFunc callback;
	gpointer user_data;
} FabulorGtkWidgetInteraction;

static inline FabulorGtkWidgetInteraction *
fabulor_gtk_widget_interaction_new (FabulorGtkWidgetInteractionFunc callback,
									gpointer user_data)
{
	FabulorGtkWidgetInteraction *interaction;

	interaction = g_new (FabulorGtkWidgetInteraction, 1);
	interaction->callback = callback;
	interaction->user_data = user_data;
	return interaction;
}

static inline void
fabulor_gtk_widget_interaction_free (gpointer data, GClosure *closure)
{
	(void) closure;
	g_free (data);
}

#if GTK_MAJOR_VERSION >= 4
static inline void
fabulor_gtk_pointer_enter_cb (GtkEventControllerMotion *controller,
								gdouble x, gdouble y, gpointer user_data)
{
	FabulorGtkWidgetInteraction *interaction = user_data;

	(void) x;
	(void) y;
	interaction->callback (gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller)),
						   interaction->user_data);
}

static inline void
fabulor_gtk_focus_change_cb (GtkEventControllerFocus *controller,
							gpointer user_data)
{
	FabulorGtkWidgetInteraction *interaction = user_data;

	interaction->callback (gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller)),
						   interaction->user_data);
}
#else
static inline gboolean
fabulor_gtk_pointer_enter_cb (GtkWidget *widget, GdkEventCrossing *event,
								gpointer user_data)
{
	FabulorGtkWidgetInteraction *interaction = user_data;

	(void) event;
	interaction->callback (widget, interaction->user_data);
	return FALSE;
}

static inline gboolean
fabulor_gtk_focus_change_cb (GtkWidget *widget, GdkEventFocus *event,
							gpointer user_data)
{
	FabulorGtkWidgetInteraction *interaction = user_data;

	(void) event;
	interaction->callback (widget, interaction->user_data);
	return FALSE;
}
#endif

static inline void
fabulor_gtk_widget_on_pointer_enter (GtkWidget *widget,
								 FabulorGtkWidgetInteractionFunc callback,
								 gpointer user_data)
{
	FabulorGtkWidgetInteraction *interaction;

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (callback != NULL);
	interaction = fabulor_gtk_widget_interaction_new (callback, user_data);

#if GTK_MAJOR_VERSION >= 4
	GtkEventController *controller = gtk_event_controller_motion_new ();

	g_signal_connect_data (controller, "enter",
		G_CALLBACK (fabulor_gtk_pointer_enter_cb), interaction,
		fabulor_gtk_widget_interaction_free, 0);
	gtk_widget_add_controller (widget, controller);
#else
	g_signal_connect_data (widget, "enter-notify-event",
		G_CALLBACK (fabulor_gtk_pointer_enter_cb), interaction,
		fabulor_gtk_widget_interaction_free, 0);
#endif
}

static inline void
fabulor_gtk_widget_on_focus_change (GtkWidget *widget, gboolean entering,
								FabulorGtkWidgetInteractionFunc callback,
								gpointer user_data)
{
	FabulorGtkWidgetInteraction *interaction;

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (callback != NULL);
	interaction = fabulor_gtk_widget_interaction_new (callback, user_data);

#if GTK_MAJOR_VERSION >= 4
	GtkEventController *controller = gtk_event_controller_focus_new ();

	g_signal_connect_data (controller, entering ? "enter" : "leave",
		G_CALLBACK (fabulor_gtk_focus_change_cb), interaction,
		fabulor_gtk_widget_interaction_free, 0);
	gtk_widget_add_controller (widget, controller);
#else
	const gchar *legacy_signal = entering
		? "focus-in-event"
		: "focus-out-event";

	g_signal_connect_data (widget, legacy_signal,
		G_CALLBACK (fabulor_gtk_focus_change_cb), interaction,
		fabulor_gtk_widget_interaction_free, 0);
#endif
}

static inline void
fabulor_gtk_widget_on_focus_enter (GtkWidget *widget,
							   FabulorGtkWidgetInteractionFunc callback,
							   gpointer user_data)
{
	fabulor_gtk_widget_on_focus_change (widget, TRUE, callback, user_data);
}

static inline void
fabulor_gtk_widget_on_focus_leave (GtkWidget *widget,
							   FabulorGtkWidgetInteractionFunc callback,
							   gpointer user_data)
{
	fabulor_gtk_widget_on_focus_change (widget, FALSE, callback, user_data);
}

typedef gboolean (*FabulorGtkScrollFunc) (GtkWidget *widget, gdouble dx,
										  gdouble dy, gpointer user_data);

typedef struct
{
	FabulorGtkScrollFunc callback;
	gpointer user_data;
} FabulorGtkScrollInteraction;

static inline void
fabulor_gtk_scroll_interaction_free (gpointer data, GClosure *closure)
{
	(void) closure;
	g_free (data);
}

#if GTK_MAJOR_VERSION >= 4
static inline gboolean
fabulor_gtk_scroll_cb (GtkEventControllerScroll *controller, gdouble dx,
					   gdouble dy, gpointer user_data)
{
	FabulorGtkScrollInteraction *interaction = user_data;

	return interaction->callback (
		gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller)),
		dx, dy, interaction->user_data);
}
#else
static inline gboolean
fabulor_gtk_scroll_cb (GtkWidget *widget, GdkEventScroll *event,
					   gpointer user_data)
{
	FabulorGtkScrollInteraction *interaction = user_data;
	gdouble dx = 0.0;
	gdouble dy = 0.0;

	if (event->direction != GDK_SCROLL_SMOOTH ||
		!gdk_event_get_scroll_deltas ((GdkEvent *) event, &dx, &dy))
	{
		switch (event->direction)
		{
		case GDK_SCROLL_UP:
			dy = -1.0;
			break;
		case GDK_SCROLL_DOWN:
			dy = 1.0;
			break;
		case GDK_SCROLL_LEFT:
			dx = -1.0;
			break;
		case GDK_SCROLL_RIGHT:
			dx = 1.0;
			break;
		default:
			break;
		}
	}

	return interaction->callback (widget, dx, dy, interaction->user_data);
}
#endif

static inline void
fabulor_gtk_widget_on_scroll (GtkWidget *widget,
							  FabulorGtkScrollFunc callback,
							  gpointer user_data)
{
	FabulorGtkScrollInteraction *interaction;

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (callback != NULL);
	interaction = g_new (FabulorGtkScrollInteraction, 1);
	interaction->callback = callback;
	interaction->user_data = user_data;

#if GTK_MAJOR_VERSION >= 4
	GtkEventController *controller = gtk_event_controller_scroll_new (
		GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES);

	gtk_event_controller_set_propagation_phase (controller, GTK_PHASE_CAPTURE);
	g_signal_connect_data (controller, "scroll",
		G_CALLBACK (fabulor_gtk_scroll_cb), interaction,
		fabulor_gtk_scroll_interaction_free, 0);
	gtk_widget_add_controller (widget, controller);
#else
	gtk_widget_add_events (widget, GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK);
	g_signal_connect_data (widget, "scroll-event",
		G_CALLBACK (fabulor_gtk_scroll_cb), interaction,
		fabulor_gtk_scroll_interaction_free, 0);
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
