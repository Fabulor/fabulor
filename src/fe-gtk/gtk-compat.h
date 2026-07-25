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

G_BEGIN_DECLS

enum
{
	FABULOR_GTK_DIALOG_ICON_PIXEL_SIZE = 48
};

typedef enum
{
	FABULOR_GTK_ICON_SIZE_MENU = 16,
	FABULOR_GTK_ICON_SIZE_LARGE_TOOLBAR = 24
} FabulorGtkIconSize;

typedef enum
{
	FABULOR_GTK_BUTTON_BOX_START,
	FABULOR_GTK_BUTTON_BOX_END,
	FABULOR_GTK_BUTTON_BOX_SPREAD
} FabulorGtkButtonBoxLayout;

static inline GtkIconTheme *
fabulor_gtk_icon_theme_get_default (void)
{
	GdkDisplay *display = gdk_display_get_default ();

	return display ? gtk_icon_theme_get_for_display (display) : NULL;
}

static inline void
fabulor_gtk_icon_theme_add_search_path (GtkIconTheme *theme,
	const gchar *path)
{
	g_return_if_fail (GTK_IS_ICON_THEME (theme));
	g_return_if_fail (path != NULL);

	gtk_icon_theme_add_search_path (theme, path);
}

static inline void
fabulor_gtk_icon_theme_set_name (GtkIconTheme *theme,
	const gchar *theme_name)
{
	g_return_if_fail (GTK_IS_ICON_THEME (theme));
	g_return_if_fail (theme_name != NULL);

	gtk_icon_theme_set_theme_name (theme, theme_name);
}

static inline GtkWidget *
fabulor_gtk_button_box_new (GtkOrientation orientation,
							FabulorGtkButtonBoxLayout layout,
							gint spacing)
{
	g_return_val_if_fail (orientation == GTK_ORIENTATION_HORIZONTAL ||
		orientation == GTK_ORIENTATION_VERTICAL, NULL);
	g_return_val_if_fail (layout >= FABULOR_GTK_BUTTON_BOX_START &&
		layout <= FABULOR_GTK_BUTTON_BOX_SPREAD, NULL);
	g_return_val_if_fail (spacing >= 0, NULL);

	GtkWidget *box = gtk_box_new (orientation, spacing);

	if (layout == FABULOR_GTK_BUTTON_BOX_SPREAD)
		gtk_box_set_homogeneous (GTK_BOX (box), TRUE);
	else if (orientation == GTK_ORIENTATION_HORIZONTAL)
		gtk_widget_set_halign (box, layout == FABULOR_GTK_BUTTON_BOX_END ?
			GTK_ALIGN_END : GTK_ALIGN_START);
	else
		gtk_widget_set_valign (box, layout == FABULOR_GTK_BUTTON_BOX_END ?
			GTK_ALIGN_END : GTK_ALIGN_START);

	return box;
}


static inline gint
fabulor_gtk_icon_size_get_pixels (FabulorGtkIconSize size)
{
	return (gint) size;
}

static inline GtkWidget *
fabulor_gtk_image_new_from_icon_name (const gchar *icon_name,
									 FabulorGtkIconSize size)
{
	GtkWidget *image;

	g_return_val_if_fail (icon_name != NULL, NULL);

	image = gtk_image_new_from_icon_name (icon_name);
	gtk_image_set_pixel_size (GTK_IMAGE (image),
		fabulor_gtk_icon_size_get_pixels (size));
	return image;
}

#define FABULOR_GTK_IMAGE_SOURCE_PIXBUF_DATA "fabulor-gtk-image-source-pixbuf"

static inline GtkWidget *
fabulor_gtk_image_new_from_pixbuf (GdkPixbuf *pixbuf)
{
	GtkWidget *image;

	g_return_val_if_fail (GDK_IS_PIXBUF (pixbuf), NULL);

	GdkTexture *texture = gdk_texture_new_for_pixbuf (pixbuf);

	image = gtk_image_new_from_paintable (GDK_PAINTABLE (texture));
	g_object_unref (texture);
	g_object_set_data_full (G_OBJECT (image),
		FABULOR_GTK_IMAGE_SOURCE_PIXBUF_DATA, g_object_ref (pixbuf),
		(GDestroyNotify) g_object_unref);
	return image;
}

static inline GdkPixbuf *
fabulor_gtk_image_get_source_pixbuf (GtkImage *image)
{
	g_return_val_if_fail (GTK_IS_IMAGE (image), NULL);

	return g_object_get_data (G_OBJECT (image),
		FABULOR_GTK_IMAGE_SOURCE_PIXBUF_DATA);
}

static inline void
fabulor_gtk_about_dialog_set_logo_from_pixbuf (GtkAboutDialog *dialog,
	GdkPixbuf *pixbuf)
{
	g_return_if_fail (GTK_IS_ABOUT_DIALOG (dialog));
	g_return_if_fail (GDK_IS_PIXBUF (pixbuf));

	GdkTexture *texture = gdk_texture_new_for_pixbuf (pixbuf);

	gtk_about_dialog_set_logo (dialog, GDK_PAINTABLE (texture));
	g_object_unref (texture);
}

static inline GtkWidget *
fabulor_gtk_button_new_with_icon_and_mnemonic (const gchar *label,
											   const gchar *icon_name,
											   FabulorGtkIconSize size)
{
	GtkWidget *button;
	GtkWidget *image;

	g_return_val_if_fail (label != NULL, NULL);
	g_return_val_if_fail (icon_name != NULL, NULL);

	GtkWidget *box;
	GtkWidget *label_widget;

	button = gtk_button_new ();
	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
	image = fabulor_gtk_image_new_from_icon_name (icon_name, size);
	label_widget = gtk_label_new_with_mnemonic (label);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label_widget), button);
	gtk_box_append (GTK_BOX (box), image);
	gtk_box_append (GTK_BOX (box), label_widget);
	gtk_button_set_child (GTK_BUTTON (button), box);

	return button;
}

static inline GtkWidget *
fabulor_gtk_radio_button_new_with_mnemonic (GtkWidget *group_member,
											const gchar *label)
{
	g_return_val_if_fail (label != NULL, NULL);

	GtkWidget *button;

	g_return_val_if_fail (group_member == NULL ||
		GTK_IS_CHECK_BUTTON (group_member), NULL);
	button = gtk_check_button_new_with_mnemonic (label);
	if (group_member)
		gtk_check_button_set_group (GTK_CHECK_BUTTON (button),
			GTK_CHECK_BUTTON (group_member));
	else
		gtk_check_button_set_active (GTK_CHECK_BUTTON (button), TRUE);
	return button;
}

static inline gboolean
fabulor_gtk_check_button_get_active (GtkWidget *button)
{
	g_return_val_if_fail (GTK_IS_CHECK_BUTTON (button), FALSE);
	return gtk_check_button_get_active (GTK_CHECK_BUTTON (button));
}

static inline void
fabulor_gtk_check_button_set_active (GtkWidget *button, gboolean active)
{
	g_return_if_fail (GTK_IS_CHECK_BUTTON (button));
	gtk_check_button_set_active (GTK_CHECK_BUTTON (button), active);
}

static inline void
fabulor_gtk_combo_box_set_single_column (GtkComboBox *combo_box)
{
	g_return_if_fail (GTK_IS_COMBO_BOX (combo_box));

	/* GTK4 combo popups already present their items in one column. */
	(void) combo_box;
}

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static inline GtkEntry *
fabulor_gtk_combo_box_get_entry (GtkComboBox *combo_box)
{
	GtkWidget *child;

	g_return_val_if_fail (GTK_IS_COMBO_BOX (combo_box), NULL);

	child = gtk_combo_box_get_child (combo_box);

	return GTK_IS_ENTRY (child) ? GTK_ENTRY (child) : NULL;
}
G_GNUC_END_IGNORE_DEPRECATIONS

static inline void
fabulor_gtk_label_set_wrap (GtkLabel *label, gboolean wrap)
{
	g_return_if_fail (GTK_IS_LABEL (label));

	gtk_label_set_wrap (label, wrap);
}

static inline const gchar *
fabulor_gtk_entry_get_text (GtkEntry *entry)
{
	g_return_val_if_fail (GTK_IS_ENTRY (entry), NULL);

	return gtk_editable_get_text (GTK_EDITABLE (entry));
}

static inline void
fabulor_gtk_entry_set_text (GtkEntry *entry, const gchar *text)
{
	g_return_if_fail (GTK_IS_ENTRY (entry));
	g_return_if_fail (text != NULL);

	gtk_editable_set_text (GTK_EDITABLE (entry), text);
}

static inline void
fabulor_gtk_entry_set_width_chars (GtkEntry *entry, gint width_chars)
{
	g_return_if_fail (GTK_IS_ENTRY (entry));
	g_return_if_fail (width_chars >= -1);

	gtk_editable_set_width_chars (GTK_EDITABLE (entry), width_chars);
}

static inline GtkWidget *
fabulor_gtk_dialog_icon_new (const gchar *icon_name)
{
	g_return_val_if_fail (icon_name != NULL, NULL);

	GtkWidget *image = gtk_image_new_from_icon_name (icon_name);

	gtk_image_set_pixel_size (GTK_IMAGE (image),
		FABULOR_GTK_DIALOG_ICON_PIXEL_SIZE);
	return image;
}

static inline gint
fabulor_gtk_margin_with_padding (gint margin, guint padding)
{
	if (padding > (guint) (G_MAXINT - margin))
		return G_MAXINT;

	return margin + (gint) padding;
}

#define FABULOR_GTK_WINDOW_INSET_DATA "fabulor-gtk-window-inset"

static inline void
fabulor_gtk_widget_set_uniform_margin (GtkWidget *widget, guint inset)
{
	gint margin = inset > G_MAXINT ? G_MAXINT : (gint) inset;

	gtk_widget_set_margin_start (widget, margin);
	gtk_widget_set_margin_end (widget, margin);
	gtk_widget_set_margin_top (widget, margin);
	gtk_widget_set_margin_bottom (widget, margin);
}

static inline void
fabulor_gtk_container_set_uniform_inset (GtkWidget *widget, guint inset)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));

	if (GTK_IS_WINDOW (widget))
	{
		GtkWidget *child = gtk_window_get_child (GTK_WINDOW (widget));
		guint *stored_inset = g_new (guint, 1);

		*stored_inset = inset;
		g_object_set_data_full (G_OBJECT (widget),
			FABULOR_GTK_WINDOW_INSET_DATA, stored_inset, g_free);
		if (child)
			fabulor_gtk_widget_set_uniform_margin (child, inset);
	}
	else
	{
		fabulor_gtk_widget_set_uniform_margin (widget, inset);
	}
}

static inline void
fabulor_gtk_box_append (GtkBox *box, GtkWidget *child, gboolean expand,
						gboolean fill, guint padding)
{
	g_return_if_fail (GTK_IS_BOX (box));
	g_return_if_fail (GTK_IS_WIDGET (child));

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
}

static inline void
fabulor_gtk_horizontal_box_append_trailing (GtkBox *box, GtkWidget *child)
{
	g_return_if_fail (GTK_IS_BOX (box));
	g_return_if_fail (GTK_IS_WIDGET (child));
	g_return_if_fail (gtk_orientable_get_orientation (GTK_ORIENTABLE (box)) ==
					  GTK_ORIENTATION_HORIZONTAL);

	gtk_widget_set_hexpand (child, TRUE);
	gtk_widget_set_halign (child, GTK_ALIGN_END);
	gtk_box_append (box, child);
}

static inline void
fabulor_gtk_box_insert_before_trailing (GtkBox *box, GtkWidget *child,
										 GtkWidget *trailing)
{
	g_return_if_fail (GTK_IS_BOX (box));
	g_return_if_fail (GTK_IS_WIDGET (child));
	g_return_if_fail (GTK_IS_WIDGET (trailing));
	g_return_if_fail (gtk_widget_get_parent (trailing) == GTK_WIDGET (box));

	GtkWidget *previous = gtk_widget_get_prev_sibling (trailing);

	if (previous)
		gtk_box_insert_child_after (box, child, previous);
	else
		gtk_box_prepend (box, child);
}

static inline void
fabulor_gtk_box_append_trailing_pair (GtkBox *box, GtkWidget *leading,
									 GtkWidget *trailing)
{
	g_return_if_fail (GTK_IS_BOX (box));
	g_return_if_fail (GTK_IS_WIDGET (leading));
	g_return_if_fail (GTK_IS_WIDGET (trailing));

	/* Keep this pair at the box end without changing call-site ordering. */
	gtk_widget_set_halign (GTK_WIDGET (box), GTK_ALIGN_END);
	gtk_box_append (box, leading);
	gtk_box_append (box, trailing);
}

static inline void
fabulor_gtk_box_reorder_child (GtkBox *box, GtkWidget *child, gint position)
{
	g_return_if_fail (GTK_IS_BOX (box));
	g_return_if_fail (GTK_IS_WIDGET (child));
	g_return_if_fail (gtk_widget_get_parent (child) == GTK_WIDGET (box));
	g_return_if_fail (position >= 0);

	GtkWidget *sibling = NULL;
	GtkWidget *candidate;
	gint index = 0;

	for (candidate = gtk_widget_get_first_child (GTK_WIDGET (box)); candidate;
		 candidate = gtk_widget_get_next_sibling (candidate))
	{
		if (candidate == child)
			continue;
		if (index++ >= position)
			break;
		sibling = candidate;
	}
	gtk_box_reorder_child_after (box, child, sibling);
}

static inline void
fabulor_gtk_box_remove_child (GtkBox *box, GtkWidget *child)
{
	g_return_if_fail (GTK_IS_BOX (box));
	g_return_if_fail (GTK_IS_WIDGET (child));
	g_return_if_fail (gtk_widget_get_parent (child) == GTK_WIDGET (box));

	gtk_box_remove (box, child);
}

static inline GtkWidget *
fabulor_gtk_content_surface_new (gboolean visible_background)
{
	(void) visible_background;
	return gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
}

static inline void
fabulor_gtk_content_surface_set_child (GtkWidget *surface, GtkWidget *child)
{
	g_return_if_fail (GTK_IS_WIDGET (surface));
	g_return_if_fail (GTK_IS_WIDGET (child));

	g_return_if_fail (GTK_IS_BOX (surface));
	gtk_box_append (GTK_BOX (surface), child);
}

static inline void
fabulor_gtk_list_box_row_set_child (GtkListBoxRow *row, GtkWidget *child)
{
	g_return_if_fail (GTK_IS_LIST_BOX_ROW (row));
	g_return_if_fail (GTK_IS_WIDGET (child));

	gtk_list_box_row_set_child (row, child);
}

static inline void
fabulor_gtk_list_box_append (GtkListBox *list, GtkWidget *row)
{
	g_return_if_fail (GTK_IS_LIST_BOX (list));
	g_return_if_fail (GTK_IS_WIDGET (row));

	gtk_list_box_append (list, row);
}

static inline void
fabulor_gtk_widget_add_css_class (GtkWidget *widget, const gchar *name)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (name != NULL);

	gtk_widget_add_css_class (widget, name);
}

static inline PangoFontDescription *
fabulor_gtk_widget_dup_font_description (GtkWidget *widget)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

	{
		PangoContext *context = gtk_widget_get_pango_context (widget);
		const PangoFontDescription *description;

		if (!context)
			return NULL;
		description = pango_context_get_font_description (context);
		return description ? pango_font_description_copy (description) : NULL;
	}
}

static inline void
fabulor_gtk_button_set_flat (GtkButton *button)
{
	g_return_if_fail (GTK_IS_BUTTON (button));

	gtk_widget_add_css_class (GTK_WIDGET (button), "flat");
}

static inline void
fabulor_gtk_button_set_always_show_image (GtkButton *button, gboolean always)
{
	g_return_if_fail (GTK_IS_BUTTON (button));

	(void) always;
}

static inline GtkWidget *
fabulor_gtk_icon_button_new (const gchar *icon_name)
{
	g_return_val_if_fail (icon_name != NULL, NULL);

	return gtk_button_new_from_icon_name (icon_name);
}

static inline void
fabulor_gtk_widget_set_accessible_label (GtkWidget *widget,
										 const gchar *label)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (label != NULL);

	gtk_accessible_update_property (GTK_ACCESSIBLE (widget),
		GTK_ACCESSIBLE_PROPERTY_LABEL, label, -1);
}

static inline void
fabulor_gtk_widget_queue_draw_region (GtkWidget *widget, gint x, gint y,
	gint width, gint height)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));

	(void) x;
	(void) y;
	(void) width;
	(void) height;
	gtk_widget_queue_draw (widget);
}

static inline gboolean
fabulor_gtk_widget_has_toplevel_focus (GtkWidget *widget)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

	GtkRoot *root = gtk_widget_get_root (widget);
	return GTK_IS_WINDOW (root) && gtk_window_is_active (GTK_WINDOW (root));
}

static inline GtkWindow *
fabulor_gtk_widget_get_root_window (GtkWidget *widget)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

	GtkRoot *root = gtk_widget_get_root (widget);
	return GTK_IS_WINDOW (root) ? GTK_WINDOW (root) : NULL;
}

static inline void
fabulor_gtk_copy_text_to_clipboards (GtkWidget *widget, const gchar *text)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (text != NULL);

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

static inline void
fabulor_gtk_widget_on_pointer_enter (GtkWidget *widget,
								 FabulorGtkWidgetInteractionFunc callback,
								 gpointer user_data)
{
	FabulorGtkWidgetInteraction *interaction;

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (callback != NULL);
	interaction = fabulor_gtk_widget_interaction_new (callback, user_data);

	GtkEventController *controller = gtk_event_controller_motion_new ();

	g_signal_connect_data (controller, "enter",
		G_CALLBACK (fabulor_gtk_pointer_enter_cb), interaction,
		fabulor_gtk_widget_interaction_free, 0);
	gtk_widget_add_controller (widget, controller);
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

	GtkEventController *controller = gtk_event_controller_focus_new ();

	g_signal_connect_data (controller, entering ? "enter" : "leave",
		G_CALLBACK (fabulor_gtk_focus_change_cb), interaction,
		fabulor_gtk_widget_interaction_free, 0);
	gtk_widget_add_controller (widget, controller);
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

	GtkEventController *controller = gtk_event_controller_motion_new ();

	g_signal_connect_data (controller, "motion",
		G_CALLBACK (fabulor_gtk_pointer_motion_cb), tracking,
		fabulor_gtk_pointer_tracking_free, 0);
	g_signal_connect_data (controller, "leave",
		G_CALLBACK (fabulor_gtk_pointer_leave_cb), tracking,
		fabulor_gtk_pointer_tracking_free, 0);
	gtk_widget_add_controller (widget, controller);
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

static inline void
fabulor_gtk_widget_suppress_pointer_prelight (GtkWidget *widget)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));

	GtkEventController *controller = gtk_event_controller_motion_new ();

	g_signal_connect (controller, "enter",
		G_CALLBACK (fabulor_gtk_suppress_prelight_enter_cb), NULL);
	g_signal_connect (controller, "leave",
		G_CALLBACK (fabulor_gtk_suppress_prelight_leave_cb), NULL);
	gtk_widget_add_controller (widget, controller);
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

	graphene_point_t source = GRAPHENE_POINT_INIT (0.0f, 0.0f);
	graphene_point_t target;

	if (!gtk_widget_compute_point (descendant, widget, &source, &target))
		return FALSE;
	*x = target.x;
	*y = target.y;
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

	graphene_point_t source = GRAPHENE_POINT_INIT ((float) x, (float) y);
	graphene_point_t target;

	if (!gtk_widget_compute_point (widget, descendant, &source, &target))
		return FALSE;
	return gtk_widget_contains (descendant, target.x, target.y);
}

static inline void
fabulor_gtk_widget_set_pointing_cursor (GtkWidget *widget, gboolean pointing)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));

	gtk_widget_set_cursor_from_name (widget, pointing ? "pointer" : NULL);
}

static inline void
fabulor_gtk_text_view_set_pointing_cursor (GtkTextView *text_view,
										   gboolean pointing)
{
	g_return_if_fail (GTK_IS_TEXT_VIEW (text_view));

	gtk_widget_set_cursor_from_name (GTK_WIDGET (text_view),
		pointing ? "pointer" : "text");
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

	GtkGesture *gesture = gtk_gesture_click_new ();

	gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), 0);
	g_signal_connect_data (gesture, "released",
		G_CALLBACK (fabulor_gtk_click_released_cb), interaction,
		fabulor_gtk_click_interaction_free, 0);
	gtk_widget_add_controller (widget, GTK_EVENT_CONTROLLER (gesture));
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
	{
		GtkGesture *gesture = gtk_gesture_click_new ();
		gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (gesture), 0);
		g_signal_connect_data (gesture, "pressed",
			G_CALLBACK (fabulor_gtk_multi_click_pressed_cb), interaction,
			fabulor_gtk_multi_click_interaction_free, 0);
		gtk_widget_add_controller (widget, GTK_EVENT_CONTROLLER (gesture));
	}
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

	GtkDragSource *controller = gtk_drag_source_new ();

	gtk_drag_source_set_actions (controller, fabulor_gtk_internal_drag_action (kind));
	g_object_set_data_full (G_OBJECT (controller),
		"fabulor-internal-drag-source", source, g_free);
	g_signal_connect (controller, "prepare",
		G_CALLBACK (fabulor_gtk_internal_drag_prepare_cb), source);
	g_signal_connect (controller, "drag-begin",
		G_CALLBACK (fabulor_gtk_internal_drag_begin_cb), source);
	gtk_widget_add_controller (widget, GTK_EVENT_CONTROLLER (controller));
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

	GtkEventController *controller = gtk_event_controller_key_new ();

	gtk_event_controller_set_propagation_phase (controller, GTK_PHASE_BUBBLE);
	g_signal_connect_data (controller, "key-pressed",
		G_CALLBACK (fabulor_gtk_key_pressed_cb), interaction,
		fabulor_gtk_key_interaction_free, 0);
	gtk_widget_add_controller (widget, controller);
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

static inline gboolean
fabulor_gtk_scroll_cb (GtkEventControllerScroll *controller, gdouble dx,
					   gdouble dy, gpointer user_data)
{
	FabulorGtkScrollInteraction *interaction = user_data;

	return interaction->callback (
		gtk_event_controller_get_widget (GTK_EVENT_CONTROLLER (controller)),
		dx, dy, interaction->user_data);
}

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

	GtkEventController *controller = gtk_event_controller_scroll_new (
		GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES);

	gtk_event_controller_set_propagation_phase (controller, GTK_PHASE_CAPTURE);
	g_signal_connect_data (controller, "scroll",
		G_CALLBACK (fabulor_gtk_scroll_cb), interaction,
		fabulor_gtk_scroll_interaction_free, 0);
	gtk_widget_add_controller (widget, controller);
}

static inline GtkWidget *
fabulor_gtk_window_new (void)
{
	return gtk_window_new ();
}

static inline void
fabulor_gtk_window_set_child (GtkWindow *window, GtkWidget *child)
{
	g_return_if_fail (GTK_IS_WINDOW (window));
	g_return_if_fail (GTK_IS_WIDGET (child));

	guint *inset;

	gtk_window_set_child (window, child);
	inset = g_object_get_data (G_OBJECT (window), FABULOR_GTK_WINDOW_INSET_DATA);
	if (inset)
		fabulor_gtk_widget_set_uniform_margin (child, *inset);
}

static inline void
fabulor_gtk_window_set_role (GtkWindow *window, const gchar *role)
{
	g_return_if_fail (GTK_IS_WINDOW (window));
	g_return_if_fail (role != NULL);

	(void) role;
}

static inline gboolean
fabulor_gtk_window_minimize (GtkWindow *window)
{
	g_return_val_if_fail (GTK_IS_WINDOW (window), FALSE);

	{
		GdkSurface *surface = gtk_native_get_surface (GTK_NATIVE (window));

		if (!GDK_IS_TOPLEVEL (surface))
			return FALSE;
		return gdk_toplevel_minimize (GDK_TOPLEVEL (surface));
	}
}

static inline void
fabulor_gtk_window_set_urgent (GtkWindow *window, gboolean urgent)
{
	g_return_if_fail (GTK_IS_WINDOW (window));

	/* GTK4 deliberately has no urgency-hint API. */
	(void) urgent;
}

static inline void
fabulor_gtk_window_set_wm_class (GtkWindow *window, const gchar *name,
	const gchar *class_name)
{
	g_return_if_fail (GTK_IS_WINDOW (window));
	g_return_if_fail (name != NULL);
	g_return_if_fail (class_name != NULL);

	/* GTK4 derives application identity from the process/application setup. */
	(void) name;
	(void) class_name;
}

static inline void
fabulor_gtk_window_resize (GtkWindow *window, gint width, gint height)
{
	g_return_if_fail (GTK_IS_WINDOW (window));

	gtk_window_set_default_size (window, width, height);
}

static inline void
fabulor_gtk_widget_set_can_default (GtkWidget *widget, gboolean can_default)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));

	(void) can_default;
}

static inline void
fabulor_gtk_window_set_default_widget (GtkWindow *window, GtkWidget *widget)
{
	g_return_if_fail (GTK_IS_WINDOW (window));
	g_return_if_fail (GTK_IS_WIDGET (widget));

	gtk_window_set_default_widget (window, widget);
}

typedef struct
{
	gint x;
	gint y;
} FabulorGtkWindowPlacement;

static inline void
fabulor_gtk_window_position_at_pointer (GtkWindow *window)
{
	g_return_if_fail (GTK_IS_WINDOW (window));

}

static inline void
fabulor_gtk_window_position_center (GtkWindow *window)
{
	g_return_if_fail (GTK_IS_WINDOW (window));

}

static inline void
fabulor_gtk_window_position_center_on_parent (GtkWindow *window)
{
	g_return_if_fail (GTK_IS_WINDOW (window));

}

static inline void
fabulor_gtk_window_move (GtkWindow *window, gint x, gint y)
{
	g_return_if_fail (GTK_IS_WINDOW (window));

	(void) x;
	(void) y;
}

static inline gboolean
fabulor_gtk_window_get_position (GtkWindow *window, gint *x, gint *y)
{
	g_return_val_if_fail (GTK_IS_WINDOW (window), FALSE);
	g_return_val_if_fail (x != NULL, FALSE);
	g_return_val_if_fail (y != NULL, FALSE);

	*x = 0;
	*y = 0;
	return FALSE;
}

static inline void
fabulor_gtk_window_set_dialog_hint (GtkWindow *window)
{
	g_return_if_fail (GTK_IS_WINDOW (window));

}

static inline void
fabulor_gtk_window_placement_capture (GtkWindow *window,
	FabulorGtkWindowPlacement *placement)
{
	g_return_if_fail (GTK_IS_WINDOW (window));
	g_return_if_fail (placement != NULL);

	fabulor_gtk_window_get_position (window, &placement->x, &placement->y);
}

static inline void
fabulor_gtk_window_placement_restore (GtkWindow *window,
	const FabulorGtkWindowPlacement *placement)
{
	g_return_if_fail (GTK_IS_WINDOW (window));
	g_return_if_fail (placement != NULL);

}

static inline GtkWidget *
fabulor_gtk_scrolled_window_new (void)
{
	return gtk_scrolled_window_new ();
}

static inline void
fabulor_gtk_scrolled_window_set_child (GtkScrolledWindow *window,
										GtkWidget *child)
{
	g_return_if_fail (GTK_IS_SCROLLED_WINDOW (window));
	g_return_if_fail (GTK_IS_WIDGET (child));

	gtk_scrolled_window_set_child (window, child);
}

static inline void
fabulor_gtk_scrolled_window_set_framed (GtkScrolledWindow *window,
										gboolean framed)
{
	g_return_if_fail (GTK_IS_SCROLLED_WINDOW (window));

	if (framed)
		gtk_widget_add_css_class (GTK_WIDGET (window), "frame");
	else
		gtk_widget_remove_css_class (GTK_WIDGET (window), "frame");
}

static inline void
fabulor_gtk_paned_set_start_child (GtkPaned *paned, GtkWidget *child,
								  gboolean resize, gboolean shrink)
{
	g_return_if_fail (GTK_IS_PANED (paned));

	gtk_paned_set_start_child (paned, child);
	gtk_paned_set_resize_start_child (paned, resize);
	gtk_paned_set_shrink_start_child (paned, shrink);
}

static inline void
fabulor_gtk_paned_set_end_child (GtkPaned *paned, GtkWidget *child,
								gboolean resize, gboolean shrink)
{
	g_return_if_fail (GTK_IS_PANED (paned));

	gtk_paned_set_end_child (paned, child);
	gtk_paned_set_resize_end_child (paned, resize);
	gtk_paned_set_shrink_end_child (paned, shrink);
}

static inline GtkWidget *
fabulor_gtk_paned_get_start_child (GtkPaned *paned)
{
	g_return_val_if_fail (GTK_IS_PANED (paned), NULL);

	return gtk_paned_get_start_child (paned);
}

static inline GtkWidget *
fabulor_gtk_paned_get_end_child (GtkPaned *paned)
{
	g_return_val_if_fail (GTK_IS_PANED (paned), NULL);

	return gtk_paned_get_end_child (paned);
}

static inline gint
fabulor_gtk_paned_get_handle_size (GtkPaned *paned)
{
	g_return_val_if_fail (GTK_IS_PANED (paned), 0);

	GtkWidget *end_child = gtk_paned_get_end_child (paned);
	gint handle_size;

	if (!end_child)
		return 0;

	if (gtk_orientable_get_orientation (GTK_ORIENTABLE (paned)) ==
		GTK_ORIENTATION_HORIZONTAL)
	{
		handle_size = gtk_widget_get_width (GTK_WIDGET (paned)) -
			gtk_paned_get_position (paned) - gtk_widget_get_width (end_child);
	}
	else
	{
		handle_size = gtk_widget_get_height (GTK_WIDGET (paned)) -
			gtk_paned_get_position (paned) - gtk_widget_get_height (end_child);
	}
	return MAX (handle_size, 0);
}

static inline gint
fabulor_gtk_widget_get_allocated_width (GtkWidget *widget)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), 0);

	return gtk_widget_get_width (widget);
}

static inline gboolean
fabulor_gtk_layout_retain_and_detach_child (GtkWidget *child)
{
	GtkWidget *parent;

	g_return_val_if_fail (GTK_IS_WIDGET (child), FALSE);

	parent = gtk_widget_get_parent (child);
	if (!parent)
		return FALSE;
	g_return_val_if_fail (GTK_IS_PANED (parent) || GTK_IS_GRID (parent),
		FALSE);

	if (GTK_IS_PANED (parent))
	{
		GtkPaned *paned = GTK_PANED (parent);

		if (gtk_paned_get_start_child (paned) == child)
		{
			g_object_ref (child);
			gtk_paned_set_start_child (paned, NULL);
		}
		else if (gtk_paned_get_end_child (paned) == child)
		{
			g_object_ref (child);
			gtk_paned_set_end_child (paned, NULL);
		}
		else
		{
			g_return_val_if_reached (FALSE);
		}
	}
	else
	{
		g_object_ref (child);
		gtk_grid_remove (GTK_GRID (parent), child);
	}

	return TRUE;
}

static inline void
fabulor_gtk_frame_set_child (GtkFrame *frame, GtkWidget *child)
{
	g_return_if_fail (GTK_IS_FRAME (frame));
	g_return_if_fail (GTK_IS_WIDGET (child));

	gtk_frame_set_child (frame, child);
}

static inline void
fabulor_gtk_frame_set_outlined (GtkFrame *frame)
{
	g_return_if_fail (GTK_IS_FRAME (frame));

}

static inline void
fabulor_gtk_button_set_child (GtkButton *button, GtkWidget *child)
{
	g_return_if_fail (GTK_IS_BUTTON (button));
	g_return_if_fail (GTK_IS_WIDGET (child));

	gtk_button_set_child (button, child);
}

static inline GtkWidget *
fabulor_gtk_button_get_child (GtkButton *button)
{
	g_return_val_if_fail (GTK_IS_BUTTON (button), NULL);

	return gtk_button_get_child (button);
}

static inline void
fabulor_gtk_overlay_set_child (GtkOverlay *overlay, GtkWidget *child)
{
	g_return_if_fail (GTK_IS_OVERLAY (overlay));
	g_return_if_fail (GTK_IS_WIDGET (child));

	gtk_overlay_set_child (overlay, child);
}

static inline void
fabulor_gtk_popover_set_child (GtkPopover *popover, GtkWidget *child)
{
	g_return_if_fail (GTK_IS_POPOVER (popover));
	g_return_if_fail (GTK_IS_WIDGET (child));

	gtk_popover_set_child (popover, child);
}

static inline void
fabulor_gtk_widget_reveal_tree (GtkWidget *widget)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));

	/* GTK4 descendants are visible by default once the completed root is shown. */
	gtk_widget_set_visible (widget, TRUE);
}

static inline void
fabulor_gtk_widget_hide_until_explicitly_shown (GtkWidget *widget)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));

	gtk_widget_set_visible (widget, FALSE);
}


static inline void
fabulor_gtk_widget_reveal_children (GtkWidget *widget)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));

	GtkWidget *child;

	for (child = gtk_widget_get_first_child (widget); child;
		 child = gtk_widget_get_next_sibling (child))
	{
		gtk_widget_set_visible (child, TRUE);
	}
}

static inline void
fabulor_gtk_window_destroy (GtkWindow *window)
{
	g_return_if_fail (GTK_IS_WINDOW (window));

	gtk_window_destroy (window);
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
