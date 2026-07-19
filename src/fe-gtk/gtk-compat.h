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

enum
{
	FABULOR_GTK_DIALOG_ICON_PIXEL_SIZE = 48
};

static inline GtkWidget *
fabulor_gtk_dialog_icon_new (const gchar *icon_name)
{
	g_return_val_if_fail (icon_name != NULL, NULL);

#if GTK_MAJOR_VERSION >= 4
	GtkWidget *image = gtk_image_new_from_icon_name (icon_name);

	gtk_image_set_pixel_size (GTK_IMAGE (image),
		FABULOR_GTK_DIALOG_ICON_PIXEL_SIZE);
	return image;
#else
	return gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_DIALOG);
#endif
}

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
fabulor_gtk_widget_add_css_class (GtkWidget *widget, const gchar *name)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (name != NULL);

#if GTK_MAJOR_VERSION >= 4
	gtk_widget_add_css_class (widget, name);
#else
	gtk_style_context_add_class (gtk_widget_get_style_context (widget), name);
#endif
}

static inline void
fabulor_gtk_widget_queue_draw_region (GtkWidget *widget, gint x, gint y,
	gint width, gint height)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));

#if GTK_MAJOR_VERSION >= 4
	(void) x;
	(void) y;
	(void) width;
	(void) height;
	gtk_widget_queue_draw (widget);
#else
	gtk_widget_queue_draw_area (widget, x, y, width, height);
#endif
}

static inline gboolean
fabulor_gtk_widget_has_toplevel_focus (GtkWidget *widget)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

#if GTK_MAJOR_VERSION >= 4
	GtkRoot *root = gtk_widget_get_root (widget);
	return GTK_IS_WINDOW (root) && gtk_window_is_active (GTK_WINDOW (root));
#else
	GtkWidget *toplevel = gtk_widget_get_toplevel (widget);
	return GTK_IS_WINDOW (toplevel) &&
		gtk_window_has_toplevel_focus (GTK_WINDOW (toplevel));
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

typedef void (*FabulorGtkPointerMotionFunc) (GtkWidget *widget, gdouble x,
											 gdouble y, gpointer user_data);
typedef void (*FabulorGtkPointerMotionStateFunc) (GtkWidget *widget,
	gdouble x, gdouble y, GdkModifierType state, gpointer user_data);
typedef void (*FabulorGtkPointerLeaveFunc) (GtkWidget *widget,
											gpointer user_data);

typedef struct
{
	FabulorGtkPointerMotionFunc motion_callback;
	FabulorGtkPointerMotionStateFunc motion_state_callback;
	FabulorGtkPointerLeaveFunc leave_callback;
	gpointer user_data;
	guint references;
} FabulorGtkPointerTracking;

static inline void
fabulor_gtk_pointer_tracking_free (gpointer data, GClosure *closure)
{
	FabulorGtkPointerTracking *tracking = data;

	(void) closure;
	tracking->references--;
	if (tracking->references == 0)
		g_free (tracking);
}

#if GTK_MAJOR_VERSION >= 4
static inline void
fabulor_gtk_pointer_motion_cb (GtkEventControllerMotion *controller,
								   gdouble x, gdouble y, gpointer user_data)
{
	FabulorGtkPointerTracking *tracking = user_data;

	if (tracking->motion_state_callback)
		tracking->motion_state_callback (
			gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller)),
			x, y, gtk_event_controller_get_current_event_state (
				GTK_EVENT_CONTROLLER (controller)), tracking->user_data);
	else
		tracking->motion_callback (
			gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller)),
			x, y, tracking->user_data);
}

static inline void
fabulor_gtk_pointer_leave_cb (GtkEventControllerMotion *controller,
								  gpointer user_data)
{
	FabulorGtkPointerTracking *tracking = user_data;

	tracking->leave_callback (
		gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller)),
		tracking->user_data);
}
#else
static inline gboolean
fabulor_gtk_pointer_motion_cb (GtkWidget *widget, GdkEventMotion *event,
								   gpointer user_data)
{
	FabulorGtkPointerTracking *tracking = user_data;

	if (tracking->motion_state_callback)
		tracking->motion_state_callback (widget, event->x, event->y,
			event->state, tracking->user_data);
	else
		tracking->motion_callback (widget, event->x, event->y,
			tracking->user_data);
	return FALSE;
}

static inline gboolean
fabulor_gtk_pointer_leave_cb (GtkWidget *widget, GdkEventCrossing *event,
								  gpointer user_data)
{
	FabulorGtkPointerTracking *tracking = user_data;

	(void) event;
	tracking->leave_callback (widget, tracking->user_data);
	return FALSE;
}
#endif

static inline void
fabulor_gtk_widget_on_pointer_motion_full (GtkWidget *widget,
	FabulorGtkPointerMotionFunc motion_callback,
	FabulorGtkPointerMotionStateFunc motion_state_callback,
	FabulorGtkPointerLeaveFunc leave_callback, gpointer user_data)
{
	FabulorGtkPointerTracking *tracking;

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (motion_callback != NULL || motion_state_callback != NULL);
	g_return_if_fail (leave_callback != NULL);

	tracking = g_new (FabulorGtkPointerTracking, 1);
	tracking->motion_callback = motion_callback;
	tracking->motion_state_callback = motion_state_callback;
	tracking->leave_callback = leave_callback;
	tracking->user_data = user_data;
	tracking->references = 2;

#if GTK_MAJOR_VERSION >= 4
	GtkEventController *controller = gtk_event_controller_motion_new ();

	g_signal_connect_data (controller, "motion",
		G_CALLBACK (fabulor_gtk_pointer_motion_cb), tracking,
		fabulor_gtk_pointer_tracking_free, 0);
	g_signal_connect_data (controller, "leave",
		G_CALLBACK (fabulor_gtk_pointer_leave_cb), tracking,
		fabulor_gtk_pointer_tracking_free, 0);
	gtk_widget_add_controller (widget, controller);
#else
	gtk_widget_add_events (widget,
		GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK);
	g_signal_connect_data (widget, "motion-notify-event",
		G_CALLBACK (fabulor_gtk_pointer_motion_cb), tracking,
		fabulor_gtk_pointer_tracking_free, 0);
	g_signal_connect_data (widget, "leave-notify-event",
		G_CALLBACK (fabulor_gtk_pointer_leave_cb), tracking,
		fabulor_gtk_pointer_tracking_free, 0);
#endif
}

static inline void
fabulor_gtk_widget_on_pointer_motion (GtkWidget *widget,
	FabulorGtkPointerMotionFunc motion_callback,
	FabulorGtkPointerLeaveFunc leave_callback, gpointer user_data)
{
	fabulor_gtk_widget_on_pointer_motion_full (widget, motion_callback, NULL,
		leave_callback, user_data);
}

static inline void
fabulor_gtk_widget_on_pointer_motion_with_state (GtkWidget *widget,
	FabulorGtkPointerMotionStateFunc motion_callback,
	FabulorGtkPointerLeaveFunc leave_callback, gpointer user_data)
{
	fabulor_gtk_widget_on_pointer_motion_full (widget, NULL, motion_callback,
		leave_callback, user_data);
}

static inline void
fabulor_gtk_widget_set_prelight (GtkWidget *widget, gboolean prelight)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));

	if (prelight)
		gtk_widget_set_state_flags (widget, GTK_STATE_FLAG_PRELIGHT, TRUE);
	else
		gtk_widget_unset_state_flags (widget, GTK_STATE_FLAG_PRELIGHT);
}

#if GTK_MAJOR_VERSION >= 4
static inline void
fabulor_gtk_suppress_prelight_enter_cb (GtkEventControllerMotion *controller,
									gdouble x, gdouble y, gpointer user_data)
{
	(void) x;
	(void) y;
	(void) user_data;
	fabulor_gtk_widget_set_prelight (
		gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller)),
		FALSE);
}

static inline void
fabulor_gtk_suppress_prelight_leave_cb (GtkEventControllerMotion *controller,
									gpointer user_data)
{
	(void) user_data;
	fabulor_gtk_widget_set_prelight (
		gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller)),
		FALSE);
}
#else
static inline gboolean
fabulor_gtk_suppress_prelight_cb (GtkWidget *widget, GdkEventCrossing *event,
								  gpointer user_data)
{
	(void) event;
	(void) user_data;
	fabulor_gtk_widget_set_prelight (widget, FALSE);
	return TRUE;
}
#endif

static inline void
fabulor_gtk_widget_suppress_pointer_prelight (GtkWidget *widget)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));

#if GTK_MAJOR_VERSION >= 4
	GtkEventController *controller = gtk_event_controller_motion_new ();

	g_signal_connect (controller, "enter",
		G_CALLBACK (fabulor_gtk_suppress_prelight_enter_cb), NULL);
	g_signal_connect (controller, "leave",
		G_CALLBACK (fabulor_gtk_suppress_prelight_leave_cb), NULL);
	gtk_widget_add_controller (widget, controller);
#else
	g_signal_connect (widget, "enter-notify-event",
		G_CALLBACK (fabulor_gtk_suppress_prelight_cb), NULL);
	g_signal_connect (widget, "leave-notify-event",
		G_CALLBACK (fabulor_gtk_suppress_prelight_cb), NULL);
#endif
}

static inline gboolean
fabulor_gtk_widget_get_descendant_origin (GtkWidget *widget,
									  GtkWidget *descendant,
									  gdouble *x, gdouble *y)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
	g_return_val_if_fail (GTK_IS_WIDGET (descendant), FALSE);
	g_return_val_if_fail (gtk_widget_is_ancestor (descendant, widget), FALSE);
	g_return_val_if_fail (x != NULL, FALSE);
	g_return_val_if_fail (y != NULL, FALSE);

#if GTK_MAJOR_VERSION >= 4
	graphene_point_t source = GRAPHENE_POINT_INIT (0.0f, 0.0f);
	graphene_point_t target;

	if (!gtk_widget_compute_point (descendant, widget, &source, &target))
		return FALSE;
	*x = target.x;
	*y = target.y;
#else
	gint descendant_x;
	gint descendant_y;

	if (!gtk_widget_translate_coordinates (descendant, widget, 0, 0,
		&descendant_x, &descendant_y))
		return FALSE;
	*x = descendant_x;
	*y = descendant_y;
#endif
	return TRUE;
}

static inline gboolean
fabulor_gtk_widget_contains_descendant_point (GtkWidget *widget,
										  GtkWidget *descendant,
										  gdouble x, gdouble y)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
	g_return_val_if_fail (GTK_IS_WIDGET (descendant), FALSE);
	g_return_val_if_fail (gtk_widget_is_ancestor (descendant, widget), FALSE);

#if GTK_MAJOR_VERSION >= 4
	graphene_point_t source = GRAPHENE_POINT_INIT ((float) x, (float) y);
	graphene_point_t target;

	if (!gtk_widget_compute_point (widget, descendant, &source, &target))
		return FALSE;
	return gtk_widget_contains (descendant, target.x, target.y);
#else
	gint descendant_x;
	gint descendant_y;
	GtkAllocation allocation;

	if (!gtk_widget_translate_coordinates (descendant, widget, 0, 0,
		&descendant_x, &descendant_y))
		return FALSE;
	gtk_widget_get_allocation (descendant, &allocation);
	return x >= descendant_x && x < descendant_x + allocation.width &&
		y >= descendant_y && y < descendant_y + allocation.height;
#endif
}

static inline void
fabulor_gtk_widget_set_pointing_cursor (GtkWidget *widget, gboolean pointing)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));

#if GTK_MAJOR_VERSION >= 4
	gtk_widget_set_cursor_from_name (widget, pointing ? "pointer" : NULL);
#else
	GdkWindow *window = gtk_widget_get_window (widget);

	if (window)
	{
		GdkCursor *cursor = NULL;

		if (pointing)
			cursor = gdk_cursor_new_for_display (
				gtk_widget_get_display (widget), GDK_HAND2);
		gdk_window_set_cursor (window, cursor);
		if (cursor)
			g_object_unref (cursor);
	}
#endif
}

static inline void
fabulor_gtk_text_view_set_pointing_cursor (GtkTextView *text_view,
										   gboolean pointing)
{
	g_return_if_fail (GTK_IS_TEXT_VIEW (text_view));

#if GTK_MAJOR_VERSION >= 4
	gtk_widget_set_cursor_from_name (GTK_WIDGET (text_view),
		pointing ? "pointer" : "text");
#else
	GdkWindow *text_window = gtk_text_view_get_window (
		text_view, GTK_TEXT_WINDOW_TEXT);

	if (text_window)
	{
		GdkCursor *cursor = gdk_cursor_new_for_display (
			gdk_window_get_display (text_window),
			pointing ? GDK_HAND2 : GDK_XTERM);

		gdk_window_set_cursor (text_window, cursor);
		g_object_unref (cursor);
	}
#endif
}

typedef gboolean (*FabulorGtkClickFunc) (GtkWidget *widget, guint button,
										 gdouble x, gdouble y,
										 GdkModifierType state,
										 gpointer user_data);

typedef struct
{
	FabulorGtkClickFunc callback;
	gpointer user_data;
} FabulorGtkClickInteraction;

static inline void
fabulor_gtk_click_interaction_free (gpointer data, GClosure *closure)
{
	(void) closure;
	g_free (data);
}

#if GTK_MAJOR_VERSION >= 4
static inline void
fabulor_gtk_click_released_cb (GtkGestureClick *gesture, gint n_press,
								   gdouble x, gdouble y, gpointer user_data)
{
	FabulorGtkClickInteraction *interaction = user_data;
	GtkEventController *controller = GTK_EVENT_CONTROLLER (gesture);

	(void) n_press;
	if (interaction->callback (
			gtk_event_controller_get_widget (controller),
			gtk_gesture_single_get_current_button (GTK_GESTURE_SINGLE (gesture)),
			x, y, gtk_event_controller_get_current_event_state (controller),
			interaction->user_data))
	{
		gtk_gesture_set_state (GTK_GESTURE (gesture),
			GTK_EVENT_SEQUENCE_CLAIMED);
	}
}
#else
static inline gboolean
fabulor_gtk_click_released_cb (GtkWidget *widget, GdkEventButton *event,
								   gpointer user_data)
{
	FabulorGtkClickInteraction *interaction = user_data;

	return interaction->callback (widget, event->button, event->x, event->y,
		event->state, interaction->user_data);
}
#endif

static inline void
fabulor_gtk_widget_on_click_released (GtkWidget *widget,
								  FabulorGtkClickFunc callback,
								  gpointer user_data)
{
	FabulorGtkClickInteraction *interaction;

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (callback != NULL);

	interaction = g_new (FabulorGtkClickInteraction, 1);
	interaction->callback = callback;
	interaction->user_data = user_data;

#if GTK_MAJOR_VERSION >= 4
	GtkGesture *gesture = gtk_gesture_click_new ();

	gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), 0);
	g_signal_connect_data (gesture, "released",
		G_CALLBACK (fabulor_gtk_click_released_cb), interaction,
		fabulor_gtk_click_interaction_free, 0);
	gtk_widget_add_controller (widget, GTK_EVENT_CONTROLLER (gesture));
#else
	gtk_widget_add_events (widget,
		GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	g_signal_connect_data (widget, "button-release-event",
		G_CALLBACK (fabulor_gtk_click_released_cb), interaction,
		fabulor_gtk_click_interaction_free, 0);
#endif
}

typedef gboolean (*FabulorGtkMultiClickFunc) (GtkWidget *widget,
											  guint button, guint n_press,
											  gdouble x, gdouble y,
											  GdkModifierType state,
											  gpointer user_data);

typedef struct
{
	FabulorGtkMultiClickFunc callback;
	gpointer user_data;
} FabulorGtkMultiClickInteraction;

static inline void
fabulor_gtk_multi_click_interaction_free (gpointer data, GClosure *closure)
{
	(void) closure;
	g_free (data);
}

#if GTK_MAJOR_VERSION >= 4
static inline void
fabulor_gtk_multi_click_pressed_cb (GtkGestureClick *gesture, gint n_press,
	gdouble x, gdouble y, gpointer user_data)
{
	FabulorGtkMultiClickInteraction *interaction = user_data;
	GtkEventController *controller = GTK_EVENT_CONTROLLER (gesture);

	if (interaction->callback (gtk_event_controller_get_widget (controller),
		gtk_gesture_single_get_current_button (GTK_GESTURE_SINGLE (gesture)),
		(guint) n_press, x, y,
		gtk_event_controller_get_current_event_state (controller),
		interaction->user_data))
	{
		gtk_gesture_set_state (GTK_GESTURE (gesture),
			GTK_EVENT_SEQUENCE_CLAIMED);
	}
}
#else
static inline gboolean
fabulor_gtk_multi_click_pressed_cb (GtkWidget *widget,
	GdkEventButton *event, gpointer user_data)
{
	FabulorGtkMultiClickInteraction *interaction = user_data;
	guint n_press = event->type == GDK_2BUTTON_PRESS ? 2 :
		event->type == GDK_3BUTTON_PRESS ? 3 : 1;

	return interaction->callback (widget, event->button, n_press,
		event->x, event->y, event->state, interaction->user_data);
}
#endif

static inline void
fabulor_gtk_widget_on_multi_click (GtkWidget *widget,
	FabulorGtkMultiClickFunc callback, gpointer user_data)
{
	FabulorGtkMultiClickInteraction *interaction;

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (callback != NULL);
	interaction = g_new (FabulorGtkMultiClickInteraction, 1);
	interaction->callback = callback;
	interaction->user_data = user_data;
#if GTK_MAJOR_VERSION >= 4
	{
		GtkGesture *gesture = gtk_gesture_click_new ();
		gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), 0);
		g_signal_connect_data (gesture, "pressed",
			G_CALLBACK (fabulor_gtk_multi_click_pressed_cb), interaction,
			fabulor_gtk_multi_click_interaction_free, 0);
		gtk_widget_add_controller (widget, GTK_EVENT_CONTROLLER (gesture));
	}
#else
	gtk_widget_add_events (widget, GDK_BUTTON_PRESS_MASK);
	g_signal_connect_data (widget, "button-press-event",
		G_CALLBACK (fabulor_gtk_multi_click_pressed_cb), interaction,
		fabulor_gtk_multi_click_interaction_free, 0);
#endif
}

typedef gboolean (*FabulorGtkFileDropFunc) (GtkWidget *widget, gdouble x,
											gdouble y, const gchar *uri_list,
											gpointer user_data);
typedef gboolean (*FabulorGtkFileDropMotionFunc) (GtkWidget *widget,
												  gdouble x, gdouble y,
												  gpointer user_data);
typedef void (*FabulorGtkFileDropLeaveFunc) (GtkWidget *widget,
											gpointer user_data);

typedef struct
{
	FabulorGtkFileDropFunc callback;
	FabulorGtkFileDropMotionFunc motion_callback;
	FabulorGtkFileDropLeaveFunc leave_callback;
	gpointer user_data;
	gboolean active;
} FabulorGtkFileDropInteraction;

static inline void
fabulor_gtk_file_drop_finish (FabulorGtkFileDropInteraction *interaction,
							  GtkWidget *widget)
{
	if (interaction->active && interaction->leave_callback)
		interaction->leave_callback (widget, interaction->user_data);
	interaction->active = FALSE;
}

#if GTK_MAJOR_VERSION >= 4
static inline GdkDragAction
fabulor_gtk_file_drag_motion_cb (GtkDropTarget *target, gdouble x,
								 gdouble y, gpointer user_data)
{
	FabulorGtkFileDropInteraction *interaction = user_data;
	const GValue *value = gtk_drop_target_get_value (target);
	GtkWidget *widget = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (target));

	if (!value || !G_VALUE_HOLDS (value, GDK_TYPE_FILE_LIST) ||
		!interaction->motion_callback ||
		!interaction->motion_callback (widget, x, y, interaction->user_data))
	{
		interaction->active = FALSE;
		return 0;
	}

	interaction->active = TRUE;
	{
		GdkDragAction actions = gtk_drop_target_get_actions (target);

		if (actions & GDK_ACTION_COPY)
			return GDK_ACTION_COPY;
		if (actions & GDK_ACTION_MOVE)
			return GDK_ACTION_MOVE;
		if (actions & GDK_ACTION_LINK)
			return GDK_ACTION_LINK;
	}
	return 0;
}

static inline void
fabulor_gtk_file_drag_leave_cb (GtkDropTarget *target, gpointer user_data)
{
	FabulorGtkFileDropInteraction *interaction = user_data;

	if (interaction->active && interaction->leave_callback)
		interaction->leave_callback (
			gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (target)),
			interaction->user_data);
	interaction->active = FALSE;
}

static inline gboolean
fabulor_gtk_file_drop_cb (GtkDropTarget *target, const GValue *value,
						  gdouble x, gdouble y, gpointer user_data)
{
	FabulorGtkFileDropInteraction *interaction = user_data;
	GdkFileList *file_list = g_value_get_boxed (value);
	GString *uris;
	GSList *files;

	if (!file_list)
		return FALSE;

	uris = g_string_new (NULL);
	for (files = gdk_file_list_get_files (file_list); files; files = files->next)
	{
		gchar *uri = g_file_get_uri (G_FILE (files->data));

		if (uri)
		{
			g_string_append (uris, uri);
			g_string_append (uris, "\r\n");
			g_free (uri);
		}
	}

	if (uris->len > 0)
	{
		gboolean handled = interaction->callback (
			gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (target)),
			x, y, uris->str, interaction->user_data);

		fabulor_gtk_file_drop_finish (interaction,
			gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (target)));
		g_string_free (uris, TRUE);
		return handled;
	}

	fabulor_gtk_file_drop_finish (interaction,
		gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (target)));
	g_string_free (uris, TRUE);
	return FALSE;
}
#else
static inline gboolean
fabulor_gtk_drag_context_has_target (GdkDragContext *context,
									 const gchar *expected_name)
{
	GList *target;

	if (!context || !expected_name)
		return FALSE;

	for (target = gdk_drag_context_list_targets (context); target;
		 target = target->next)
	{
		gchar *target_name = gdk_atom_name (GDK_POINTER_TO_ATOM (target->data));
		gboolean matches = g_strcmp0 (target_name, expected_name) == 0;

		g_free (target_name);
		if (matches)
			return TRUE;
	}

	return FALSE;
}

static inline gboolean
fabulor_gtk_file_drag_motion_cb (GtkWidget *widget, GdkDragContext *context,
								 gint x, gint y, guint32 time,
								 gpointer user_data)
{
	FabulorGtkFileDropInteraction *interaction = user_data;
	GdkDragAction action;

	if (!fabulor_gtk_drag_context_has_target (context, "text/uri-list") ||
		!interaction->motion_callback ||
		!interaction->motion_callback (widget, x, y, interaction->user_data))
	{
		interaction->active = FALSE;
		return FALSE;
	}

	interaction->active = TRUE;
	action = gdk_drag_context_get_suggested_action (context);
	gdk_drag_status (context, action, time);
	return TRUE;
}

static inline void
fabulor_gtk_file_drag_leave_cb (GtkWidget *widget, GdkDragContext *context,
								guint32 time, gpointer user_data)
{
	FabulorGtkFileDropInteraction *interaction = user_data;

	(void) context;
	(void) time;
	if (interaction->active && interaction->leave_callback)
		interaction->leave_callback (widget, interaction->user_data);
	interaction->active = FALSE;
}

static inline void
fabulor_gtk_file_drop_cb (GtkWidget *widget, GdkDragContext *context,
						  gint x, gint y, GtkSelectionData *selection_data,
						  guint info, guint32 time, gpointer user_data)
{
	FabulorGtkFileDropInteraction *interaction = user_data;
	gint length = gtk_selection_data_get_length (selection_data);
	gchar *target_name;
	gchar *uri_list;

	(void) context;
	(void) info;
	(void) time;

	target_name = gdk_atom_name (gtk_selection_data_get_target (selection_data));
	if (!target_name || strcmp (target_name, "text/uri-list") != 0 || length <= 0)
	{
		g_free (target_name);
		return;
	}
	g_free (target_name);

	uri_list = g_strndup ((const gchar *) gtk_selection_data_get_data (selection_data),
		length);
	interaction->callback (widget, x, y, uri_list, interaction->user_data);
	fabulor_gtk_file_drop_finish (interaction, widget);
	g_free (uri_list);
}
#endif

static inline void
fabulor_gtk_widget_on_file_drop_full (GtkWidget *widget,
								  GdkDragAction actions,
								  FabulorGtkFileDropFunc callback,
								  FabulorGtkFileDropMotionFunc motion_callback,
								  FabulorGtkFileDropLeaveFunc leave_callback,
								  gpointer user_data)
{
	FabulorGtkFileDropInteraction *interaction;

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (callback != NULL);

	interaction = g_new (FabulorGtkFileDropInteraction, 1);
	interaction->callback = callback;
	interaction->motion_callback = motion_callback;
	interaction->leave_callback = leave_callback;
	interaction->user_data = user_data;
	interaction->active = FALSE;

#if GTK_MAJOR_VERSION >= 4
	GtkDropTarget *target = gtk_drop_target_new (GDK_TYPE_FILE_LIST, actions);

	gtk_drop_target_set_preload (target, motion_callback != NULL);
	g_object_set_data_full (G_OBJECT (target),
		"fabulor-file-drop-interaction", interaction, g_free);
	if (motion_callback)
		g_signal_connect (target, "motion",
			G_CALLBACK (fabulor_gtk_file_drag_motion_cb), interaction);
	if (leave_callback)
		g_signal_connect (target, "leave",
			G_CALLBACK (fabulor_gtk_file_drag_leave_cb), interaction);
	g_signal_connect (target, "drop",
		G_CALLBACK (fabulor_gtk_file_drop_cb), interaction);
	gtk_widget_add_controller (widget, GTK_EVENT_CONTROLLER (target));
#else
	gtk_drag_dest_add_uri_targets (widget);
	g_object_set_data_full (G_OBJECT (widget),
		"fabulor-file-drop-interaction", interaction, g_free);
	if (motion_callback)
		g_signal_connect (widget, "drag-motion",
			G_CALLBACK (fabulor_gtk_file_drag_motion_cb), interaction);
	if (leave_callback)
		g_signal_connect (widget, "drag-leave",
			G_CALLBACK (fabulor_gtk_file_drag_leave_cb), interaction);
	g_signal_connect (widget, "drag-data-received",
		G_CALLBACK (fabulor_gtk_file_drop_cb), interaction);
#endif
}

static inline void
fabulor_gtk_widget_on_file_drop (GtkWidget *widget, GdkDragAction actions,
								 FabulorGtkFileDropFunc callback,
								 gpointer user_data)
{
	fabulor_gtk_widget_on_file_drop_full (widget, actions, callback,
		NULL, NULL, user_data);
}

typedef enum
{
	FABULOR_GTK_INTERNAL_DRAG_NONE = 0,
	FABULOR_GTK_INTERNAL_DRAG_CHANNEL_VIEW = 1,
	FABULOR_GTK_INTERNAL_DRAG_USER_LIST = 2
} FabulorGtkInternalDragKind;

#define FABULOR_GTK_INTERNAL_DRAG_ACCEPT(kind) (1u << (guint) (kind))

typedef GdkPixbuf *(*FabulorGtkInternalDragIconFunc) (GtkWidget *widget,
													 gpointer user_data);
typedef gboolean (*FabulorGtkInternalDragMotionFunc) (GtkWidget *widget,
													  FabulorGtkInternalDragKind kind,
													  gdouble x, gdouble y,
													  gpointer user_data);
typedef void (*FabulorGtkInternalDragLeaveFunc) (GtkWidget *widget,
												 FabulorGtkInternalDragKind kind,
												 gpointer user_data);
typedef gboolean (*FabulorGtkInternalDragDropFunc) (GtkWidget *widget,
													FabulorGtkInternalDragKind kind,
													gdouble x, gdouble y,
													gpointer user_data);

typedef struct
{
	FabulorGtkInternalDragKind kind;
	FabulorGtkInternalDragIconFunc icon_callback;
	gpointer user_data;
} FabulorGtkInternalDragSource;

typedef struct
{
	guint accepted_kinds;
	FabulorGtkInternalDragMotionFunc motion_callback;
	FabulorGtkInternalDragLeaveFunc leave_callback;
	FabulorGtkInternalDragDropFunc drop_callback;
	gpointer user_data;
	FabulorGtkInternalDragKind active_kind;
} FabulorGtkInternalDropTarget;

static inline void
fabulor_gtk_internal_drag_finish (FabulorGtkInternalDropTarget *target,
								  GtkWidget *widget)
{
	if (target->active_kind != FABULOR_GTK_INTERNAL_DRAG_NONE &&
		target->leave_callback)
		target->leave_callback (widget, target->active_kind, target->user_data);
	target->active_kind = FABULOR_GTK_INTERNAL_DRAG_NONE;
}

static inline GdkDragAction
fabulor_gtk_internal_drag_action (FabulorGtkInternalDragKind kind)
{
	return kind == FABULOR_GTK_INTERNAL_DRAG_USER_LIST ?
		GDK_ACTION_MOVE : GDK_ACTION_COPY;
}

static inline gboolean
fabulor_gtk_internal_drag_kind_is_accepted (FabulorGtkInternalDragKind kind,
											guint accepted_kinds)
{
	return kind != FABULOR_GTK_INTERNAL_DRAG_NONE &&
		(accepted_kinds & FABULOR_GTK_INTERNAL_DRAG_ACCEPT (kind)) != 0;
}

#if GTK_MAJOR_VERSION >= 4
static inline FabulorGtkInternalDragKind
fabulor_gtk_internal_drag_kind_from_value (const GValue *value)
{
	gpointer payload;

	if (!value || !G_VALUE_HOLDS (value, G_TYPE_POINTER))
		return FABULOR_GTK_INTERNAL_DRAG_NONE;

	payload = g_value_get_pointer (value);
	if (payload == GUINT_TO_POINTER (FABULOR_GTK_INTERNAL_DRAG_CHANNEL_VIEW))
		return FABULOR_GTK_INTERNAL_DRAG_CHANNEL_VIEW;
	if (payload == GUINT_TO_POINTER (FABULOR_GTK_INTERNAL_DRAG_USER_LIST))
		return FABULOR_GTK_INTERNAL_DRAG_USER_LIST;
	return FABULOR_GTK_INTERNAL_DRAG_NONE;
}

static inline GdkContentProvider *
fabulor_gtk_internal_drag_prepare_cb (GtkDragSource *controller,
									  gdouble x, gdouble y,
									  gpointer user_data)
{
	FabulorGtkInternalDragSource *source = user_data;

	(void) controller;
	(void) x;
	(void) y;
	return gdk_content_provider_new_typed (G_TYPE_POINTER,
		GUINT_TO_POINTER (source->kind));
}

static inline void
fabulor_gtk_internal_drag_begin_cb (GtkDragSource *controller, GdkDrag *drag,
									gpointer user_data)
{
	GtkWidget *widget = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller));
	GdkPaintable *paintable = gtk_widget_paintable_new (widget);

	(void) drag;
	(void) user_data;
	gtk_drag_source_set_icon (controller, paintable, 0, 0);
	g_object_unref (paintable);
}

static inline GdkDragAction
fabulor_gtk_internal_drag_motion_cb (GtkDropTarget *controller,
									gdouble x, gdouble y,
									gpointer user_data)
{
	FabulorGtkInternalDropTarget *target = user_data;
	FabulorGtkInternalDragKind kind = fabulor_gtk_internal_drag_kind_from_value (
		gtk_drop_target_get_value (controller));
	GtkWidget *widget = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller));

	if (!fabulor_gtk_internal_drag_kind_is_accepted (kind, target->accepted_kinds) ||
		!target->motion_callback ||
		!target->motion_callback (widget, kind, x, y, target->user_data))
	{
		target->active_kind = FABULOR_GTK_INTERNAL_DRAG_NONE;
		return 0;
	}

	target->active_kind = kind;
	return fabulor_gtk_internal_drag_action (kind);
}

static inline void
fabulor_gtk_internal_drag_leave_cb (GtkDropTarget *controller,
									gpointer user_data)
{
	FabulorGtkInternalDropTarget *target = user_data;

	if (target->active_kind != FABULOR_GTK_INTERNAL_DRAG_NONE &&
		target->leave_callback)
	{
		target->leave_callback (
			gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller)),
			target->active_kind, target->user_data);
	}
	target->active_kind = FABULOR_GTK_INTERNAL_DRAG_NONE;
}

static inline gboolean
fabulor_gtk_internal_drag_drop_cb (GtkDropTarget *controller,
								  const GValue *value, gdouble x,
								  gdouble y, gpointer user_data)
{
	FabulorGtkInternalDropTarget *target = user_data;
	FabulorGtkInternalDragKind kind = fabulor_gtk_internal_drag_kind_from_value (value);
	GtkWidget *widget = gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller));
	gboolean handled;

	handled = fabulor_gtk_internal_drag_kind_is_accepted (kind, target->accepted_kinds) &&
		target->drop_callback (widget, kind, x, y, target->user_data);
	fabulor_gtk_internal_drag_finish (target, widget);
	return handled;
}
#else
static inline const gchar *
fabulor_gtk_internal_drag_target_name (FabulorGtkInternalDragKind kind)
{
	if (kind == FABULOR_GTK_INTERNAL_DRAG_CHANNEL_VIEW)
		return "ZOITECHAT_CHANVIEW";
	if (kind == FABULOR_GTK_INTERNAL_DRAG_USER_LIST)
		return "ZOITECHAT_USERLIST";
	return NULL;
}

static inline FabulorGtkInternalDragKind
fabulor_gtk_internal_drag_kind_from_context (GdkDragContext *context)
{
	if (fabulor_gtk_drag_context_has_target (context, "ZOITECHAT_CHANVIEW"))
		return FABULOR_GTK_INTERNAL_DRAG_CHANNEL_VIEW;
	if (fabulor_gtk_drag_context_has_target (context, "ZOITECHAT_USERLIST"))
		return FABULOR_GTK_INTERNAL_DRAG_USER_LIST;
	return FABULOR_GTK_INTERNAL_DRAG_NONE;
}

static inline void
fabulor_gtk_internal_drag_begin_cb (GtkWidget *widget,
									GdkDragContext *context,
									gpointer user_data)
{
	FabulorGtkInternalDragSource *source = user_data;
	GdkPixbuf *icon;

	if (!source->icon_callback)
		return;
	icon = source->icon_callback (widget, source->user_data);
	if (!icon)
		return;
	gtk_drag_set_icon_pixbuf (context, icon, 0, 0);
	g_object_unref (icon);
}

static inline gboolean
fabulor_gtk_internal_drag_motion_cb (GtkWidget *widget,
									GdkDragContext *context,
									gint x, gint y, guint32 time,
									gpointer user_data)
{
	FabulorGtkInternalDropTarget *target = user_data;
	FabulorGtkInternalDragKind kind = fabulor_gtk_internal_drag_kind_from_context (context);

	if (!fabulor_gtk_internal_drag_kind_is_accepted (kind, target->accepted_kinds) ||
		!target->motion_callback ||
		!target->motion_callback (widget, kind, x, y, target->user_data))
	{
		target->active_kind = FABULOR_GTK_INTERNAL_DRAG_NONE;
		return FALSE;
	}

	target->active_kind = kind;
	gdk_drag_status (context, fabulor_gtk_internal_drag_action (kind), time);
	return TRUE;
}

static inline void
fabulor_gtk_internal_drag_leave_cb (GtkWidget *widget,
									GdkDragContext *context, guint32 time,
									gpointer user_data)
{
	FabulorGtkInternalDropTarget *target = user_data;

	(void) context;
	(void) time;
	if (target->active_kind != FABULOR_GTK_INTERNAL_DRAG_NONE &&
		target->leave_callback)
		target->leave_callback (widget, target->active_kind, target->user_data);
	target->active_kind = FABULOR_GTK_INTERNAL_DRAG_NONE;
}

static inline gboolean
fabulor_gtk_internal_drag_drop_cb (GtkWidget *widget,
								  GdkDragContext *context, gint x,
								  gint y, guint32 time, gpointer user_data)
{
	FabulorGtkInternalDropTarget *target = user_data;
	FabulorGtkInternalDragKind kind = fabulor_gtk_internal_drag_kind_from_context (context);
	gboolean handled;

	(void) time;
	handled = fabulor_gtk_internal_drag_kind_is_accepted (kind, target->accepted_kinds) &&
		target->drop_callback (widget, kind, x, y, target->user_data);
	fabulor_gtk_internal_drag_finish (target, widget);
	return handled;
}
#endif

static inline void
fabulor_gtk_widget_enable_internal_drag_source (
	GtkWidget *widget, FabulorGtkInternalDragKind kind,
	FabulorGtkInternalDragIconFunc icon_callback, gpointer user_data)
{
	FabulorGtkInternalDragSource *source;

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (kind == FABULOR_GTK_INTERNAL_DRAG_CHANNEL_VIEW ||
		kind == FABULOR_GTK_INTERNAL_DRAG_USER_LIST);

	source = g_new (FabulorGtkInternalDragSource, 1);
	source->kind = kind;
	source->icon_callback = icon_callback;
	source->user_data = user_data;

#if GTK_MAJOR_VERSION >= 4
	GtkDragSource *controller = gtk_drag_source_new ();

	gtk_drag_source_set_actions (controller, fabulor_gtk_internal_drag_action (kind));
	g_object_set_data_full (G_OBJECT (controller),
		"fabulor-internal-drag-source", source, g_free);
	g_signal_connect (controller, "prepare",
		G_CALLBACK (fabulor_gtk_internal_drag_prepare_cb), source);
	g_signal_connect (controller, "drag-begin",
		G_CALLBACK (fabulor_gtk_internal_drag_begin_cb), source);
	gtk_widget_add_controller (widget, GTK_EVENT_CONTROLLER (controller));
#else
	GtkTargetEntry entry = {
		(gchar *) fabulor_gtk_internal_drag_target_name (kind),
		GTK_TARGET_SAME_APP,
		(guint) kind
	};

	gtk_drag_source_set (widget, GDK_BUTTON1_MASK, &entry, 1,
		fabulor_gtk_internal_drag_action (kind));
	g_object_set_data_full (G_OBJECT (widget),
		"fabulor-internal-drag-source", source, g_free);
	g_signal_connect (widget, "drag-begin",
		G_CALLBACK (fabulor_gtk_internal_drag_begin_cb), source);
#endif
}

static inline void
fabulor_gtk_widget_enable_internal_drop_target (
	GtkWidget *widget, guint accepted_kinds,
	FabulorGtkInternalDragMotionFunc motion_callback,
	FabulorGtkInternalDragLeaveFunc leave_callback,
	FabulorGtkInternalDragDropFunc drop_callback, gpointer user_data)
{
	FabulorGtkInternalDropTarget *target;

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (accepted_kinds != 0);
	g_return_if_fail (drop_callback != NULL);

	target = g_new0 (FabulorGtkInternalDropTarget, 1);
	target->accepted_kinds = accepted_kinds;
	target->motion_callback = motion_callback;
	target->leave_callback = leave_callback;
	target->drop_callback = drop_callback;
	target->user_data = user_data;

#if GTK_MAJOR_VERSION >= 4
	GtkDropTarget *controller = gtk_drop_target_new (G_TYPE_POINTER,
		GDK_ACTION_MOVE | GDK_ACTION_COPY);

	gtk_drop_target_set_preload (controller, TRUE);
	g_object_set_data_full (G_OBJECT (controller),
		"fabulor-internal-drop-target", target, g_free);
	if (motion_callback)
		g_signal_connect (controller, "motion",
			G_CALLBACK (fabulor_gtk_internal_drag_motion_cb), target);
	if (leave_callback)
		g_signal_connect (controller, "leave",
			G_CALLBACK (fabulor_gtk_internal_drag_leave_cb), target);
	g_signal_connect (controller, "drop",
		G_CALLBACK (fabulor_gtk_internal_drag_drop_cb), target);
	gtk_widget_add_controller (widget, GTK_EVENT_CONTROLLER (controller));
#else
	GtkTargetEntry entries[2];
	guint entry_count = 0;

	if (fabulor_gtk_internal_drag_kind_is_accepted (
			FABULOR_GTK_INTERNAL_DRAG_CHANNEL_VIEW, accepted_kinds))
	{
		entries[entry_count].target = "ZOITECHAT_CHANVIEW";
		entries[entry_count].flags = GTK_TARGET_SAME_APP;
		entries[entry_count++].info = FABULOR_GTK_INTERNAL_DRAG_CHANNEL_VIEW;
	}
	if (fabulor_gtk_internal_drag_kind_is_accepted (
			FABULOR_GTK_INTERNAL_DRAG_USER_LIST, accepted_kinds))
	{
		entries[entry_count].target = "ZOITECHAT_USERLIST";
		entries[entry_count].flags = GTK_TARGET_SAME_APP;
		entries[entry_count++].info = FABULOR_GTK_INTERNAL_DRAG_USER_LIST;
	}

	gtk_drag_dest_set (widget, GTK_DEST_DEFAULT_ALL, entries, entry_count,
		GDK_ACTION_MOVE | GDK_ACTION_COPY);
	g_object_set_data_full (G_OBJECT (widget),
		"fabulor-internal-drop-target", target, g_free);
	if (motion_callback)
		g_signal_connect (widget, "drag-motion",
			G_CALLBACK (fabulor_gtk_internal_drag_motion_cb), target);
	if (leave_callback)
		g_signal_connect (widget, "drag-leave",
			G_CALLBACK (fabulor_gtk_internal_drag_leave_cb), target);
	g_signal_connect (widget, "drag-drop",
		G_CALLBACK (fabulor_gtk_internal_drag_drop_cb), target);
#endif
}

typedef gboolean (*FabulorGtkKeyFunc) (GtkWidget *widget, guint keyval,
									   GdkModifierType state,
									   gpointer user_data);

typedef struct
{
	FabulorGtkKeyFunc callback;
	gpointer user_data;
} FabulorGtkKeyInteraction;

static inline void
fabulor_gtk_key_interaction_free (gpointer data, GClosure *closure)
{
	(void) closure;
	g_free (data);
}

#if GTK_MAJOR_VERSION >= 4
static inline gboolean
fabulor_gtk_key_pressed_cb (GtkEventControllerKey *controller, guint keyval,
							guint keycode, GdkModifierType state,
							gpointer user_data)
{
	FabulorGtkKeyInteraction *interaction = user_data;

	(void) keycode;
	return interaction->callback (
		gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller)),
		keyval, state, interaction->user_data);
}
#else
static inline gboolean
fabulor_gtk_key_pressed_cb (GtkWidget *widget, GdkEventKey *event,
							gpointer user_data)
{
	FabulorGtkKeyInteraction *interaction = user_data;

	return interaction->callback (widget, event->keyval, event->state,
		interaction->user_data);
}
#endif

static inline void
fabulor_gtk_widget_on_key_pressed (GtkWidget *widget,
								FabulorGtkKeyFunc callback,
								gpointer user_data)
{
	FabulorGtkKeyInteraction *interaction;

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (callback != NULL);

	interaction = g_new (FabulorGtkKeyInteraction, 1);
	interaction->callback = callback;
	interaction->user_data = user_data;

#if GTK_MAJOR_VERSION >= 4
	GtkEventController *controller = gtk_event_controller_key_new ();

	gtk_event_controller_set_propagation_phase (controller, GTK_PHASE_BUBBLE);
	g_signal_connect_data (controller, "key-pressed",
		G_CALLBACK (fabulor_gtk_key_pressed_cb), interaction,
		fabulor_gtk_key_interaction_free, 0);
	gtk_widget_add_controller (widget, controller);
#else
	g_signal_connect_data (widget, "key-press-event",
		G_CALLBACK (fabulor_gtk_key_pressed_cb), interaction,
		fabulor_gtk_key_interaction_free, 0);
#endif
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

typedef struct
{
	gint x;
	gint y;
#if GTK_MAJOR_VERSION < 4
	GdkScreen *screen;
#endif
} FabulorGtkWindowPlacement;

static inline void
fabulor_gtk_window_position_at_pointer (GtkWindow *window)
{
	g_return_if_fail (GTK_IS_WINDOW (window));

#if GTK_MAJOR_VERSION < 4
	gtk_window_set_position (window, GTK_WIN_POS_MOUSE);
#endif
}

static inline void
fabulor_gtk_window_position_center (GtkWindow *window)
{
	g_return_if_fail (GTK_IS_WINDOW (window));

#if GTK_MAJOR_VERSION < 4
	gtk_window_set_position (window, GTK_WIN_POS_CENTER);
#endif
}

static inline void
fabulor_gtk_window_position_center_on_parent (GtkWindow *window)
{
	g_return_if_fail (GTK_IS_WINDOW (window));

#if GTK_MAJOR_VERSION < 4
	gtk_window_set_position (window, GTK_WIN_POS_CENTER_ON_PARENT);
#endif
}

static inline void
fabulor_gtk_window_move (GtkWindow *window, gint x, gint y)
{
	g_return_if_fail (GTK_IS_WINDOW (window));

#if GTK_MAJOR_VERSION >= 4
	(void) x;
	(void) y;
#else
	gtk_window_move (window, x, y);
#endif
}

static inline gboolean
fabulor_gtk_window_get_position (GtkWindow *window, gint *x, gint *y)
{
	g_return_val_if_fail (GTK_IS_WINDOW (window), FALSE);
	g_return_val_if_fail (x != NULL, FALSE);
	g_return_val_if_fail (y != NULL, FALSE);

#if GTK_MAJOR_VERSION >= 4
	*x = 0;
	*y = 0;
	return FALSE;
#else
	gtk_window_get_position (window, x, y);
	return TRUE;
#endif
}

static inline void
fabulor_gtk_window_placement_capture (GtkWindow *window,
	FabulorGtkWindowPlacement *placement)
{
	g_return_if_fail (GTK_IS_WINDOW (window));
	g_return_if_fail (placement != NULL);

	fabulor_gtk_window_get_position (window, &placement->x, &placement->y);
#if GTK_MAJOR_VERSION < 4
	placement->screen = gtk_window_get_screen (window);
#endif
}

static inline void
fabulor_gtk_window_placement_restore (GtkWindow *window,
	const FabulorGtkWindowPlacement *placement)
{
	g_return_if_fail (GTK_IS_WINDOW (window));
	g_return_if_fail (placement != NULL);

#if GTK_MAJOR_VERSION < 4
	if (placement->screen)
		gtk_window_set_screen (window, placement->screen);
	gtk_window_move (window, placement->x, placement->y);
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
fabulor_gtk_paned_set_start_child (GtkPaned *paned, GtkWidget *child,
								  gboolean resize, gboolean shrink)
{
	g_return_if_fail (GTK_IS_PANED (paned));

#if GTK_MAJOR_VERSION >= 4
	(void) resize;
	(void) shrink;
	gtk_paned_set_start_child (paned, child);
#else
	gtk_paned_pack1 (paned, child, resize, shrink);
#endif
}

static inline void
fabulor_gtk_paned_set_end_child (GtkPaned *paned, GtkWidget *child,
								gboolean resize, gboolean shrink)
{
	g_return_if_fail (GTK_IS_PANED (paned));

#if GTK_MAJOR_VERSION >= 4
	(void) resize;
	(void) shrink;
	gtk_paned_set_end_child (paned, child);
#else
	gtk_paned_pack2 (paned, child, resize, shrink);
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
