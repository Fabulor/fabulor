/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "xtext-selection.h"

#if GTK_MAJOR_VERSION < 4 && defined(GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#endif

#if GTK_MAJOR_VERSION < 4
enum
{
	TARGET_UTF8_STRING,
	TARGET_STRING,
	TARGET_TEXT,
	TARGET_COMPOUND_TEXT
};

static const GtkTargetEntry selection_targets[] = {
	{ "UTF8_STRING", 0, TARGET_UTF8_STRING },
	{ "STRING", 0, TARGET_STRING },
	{ "TEXT", 0, TARGET_TEXT },
	{ "COMPOUND_TEXT", 0, TARGET_COMPOUND_TEXT }
};
#endif

struct _FabulorXTextSelection
{
	GtkWidget *widget;
	FabulorXTextSelectionTextFunc text_func;
	FabulorXTextSelectionClearFunc clear_func;
	gpointer user_data;
#if GTK_MAJOR_VERSION >= 4
	GdkClipboard *primary;
	GdkContentProvider *provider;
	gulong changed_handler;
	gboolean publishing;
#else
	gulong realize_handler;
	gulong clear_handler;
	gulong get_handler;
#endif
};

gchar *
fabulor_xtext_selection_copy_text (const gchar *text, gssize length)
{
	g_return_val_if_fail (text != NULL, NULL);

	return length < 0 ? g_strdup (text) : g_strndup (text, (gsize) length);
}

#if GTK_MAJOR_VERSION >= 4
static void
fabulor_xtext_selection_changed (GdkClipboard *clipboard, gpointer user_data)
{
	FabulorXTextSelection *selection = user_data;

	if (selection->publishing || selection->provider == NULL ||
		gdk_clipboard_get_content (clipboard) == selection->provider)
		return;

	g_clear_object (&selection->provider);
	if (selection->clear_func)
		selection->clear_func (selection->widget, selection->user_data);
}
#else
static void
fabulor_xtext_selection_install_targets (GtkWidget *widget)
{
	if (gtk_widget_get_window (widget) == NULL)
		return;

	gtk_selection_add_targets (widget, GDK_SELECTION_PRIMARY,
		(GtkTargetEntry *) selection_targets,
		(gint) G_N_ELEMENTS (selection_targets));
}

static void
fabulor_xtext_selection_realize (GtkWidget *widget, gpointer user_data)
{
	(void) user_data;
	fabulor_xtext_selection_install_targets (widget);
}

static gboolean
fabulor_xtext_selection_clear (GtkWidget *widget, GdkEventSelection *event,
	gpointer user_data)
{
	FabulorXTextSelection *selection = user_data;
	(void) event;

	if (selection->clear_func)
		selection->clear_func (widget, selection->user_data);
	return TRUE;
}

static void
fabulor_xtext_selection_get (GtkWidget *widget,
	GtkSelectionData *selection_data, guint info, guint time,
	gpointer user_data)
{
	FabulorXTextSelection *selection = user_data;
	gchar *text;
	guchar *converted;
	gint length;
	gsize converted_length;
	(void) time;

	text = selection->text_func (widget, &length, selection->user_data);
	if (text == NULL)
		return;

	switch (info)
	{
	case TARGET_UTF8_STRING:
		gtk_selection_data_set_text (selection_data, text, length);
		break;
	case TARGET_TEXT:
	case TARGET_COMPOUND_TEXT:
#ifdef GDK_WINDOWING_X11
		{
			GdkWindow *window = gtk_widget_get_window (widget);
			GdkDisplay *display;
			GdkAtom encoding;
			gint format;
			gint new_length;

			if (window == NULL)
				break;
			display = gdk_window_get_display (window);
			if (display == NULL)
				break;
			gdk_x11_display_string_to_compound_text (display, text,
				&encoding, &format, &converted, &new_length);
			gtk_selection_data_set (selection_data, encoding, format,
				converted, new_length);
			gdk_x11_free_compound_text (converted);
		}
		break;
#endif
	default:
		converted = (guchar *) g_locale_from_utf8 (text, length, NULL,
			&converted_length, NULL);
		if (converted)
		{
			gtk_selection_data_set (selection_data,
				GDK_SELECTION_TYPE_STRING, 8, converted,
				(gint) converted_length);
			g_free (converted);
		}
	}

	g_free (text);
}
#endif

FabulorXTextSelection *
fabulor_xtext_selection_new (GtkWidget *widget,
	FabulorXTextSelectionTextFunc text_func,
	FabulorXTextSelectionClearFunc clear_func, gpointer user_data)
{
	FabulorXTextSelection *selection;

	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
	g_return_val_if_fail (text_func != NULL, NULL);

	selection = g_new0 (FabulorXTextSelection, 1);
	selection->widget = widget;
	selection->text_func = text_func;
	selection->clear_func = clear_func;
	selection->user_data = user_data;

#if GTK_MAJOR_VERSION >= 4
	GdkDisplay *display = gtk_widget_get_display (widget);

	if (display)
		selection->primary = gdk_display_get_primary_clipboard (display);
	if (selection->primary)
		selection->changed_handler = g_signal_connect (selection->primary,
			"changed", G_CALLBACK (fabulor_xtext_selection_changed), selection);
#else
	selection->clear_handler = g_signal_connect (widget,
		"selection-clear-event", G_CALLBACK (fabulor_xtext_selection_clear),
		selection);
	selection->get_handler = g_signal_connect (widget, "selection-get",
		G_CALLBACK (fabulor_xtext_selection_get), selection);
	if (gtk_widget_get_realized (widget))
		fabulor_xtext_selection_install_targets (widget);
	else
		selection->realize_handler = g_signal_connect (widget, "realize",
			G_CALLBACK (fabulor_xtext_selection_realize), selection);
#endif

	return selection;
}

void
fabulor_xtext_selection_free (FabulorXTextSelection *selection)
{
	if (selection == NULL)
		return;

#if GTK_MAJOR_VERSION >= 4
	if (selection->primary && selection->changed_handler)
		g_signal_handler_disconnect (selection->primary,
			selection->changed_handler);
	g_clear_object (&selection->provider);
#else
	if (selection->realize_handler)
		g_signal_handler_disconnect (selection->widget,
			selection->realize_handler);
	if (selection->clear_handler)
		g_signal_handler_disconnect (selection->widget,
			selection->clear_handler);
	if (selection->get_handler)
		g_signal_handler_disconnect (selection->widget,
			selection->get_handler);
#endif
	g_free (selection);
}

void
fabulor_xtext_selection_publish (FabulorXTextSelection *selection,
	const gchar *text, gint length, guint32 event_time)
{
	g_return_if_fail (selection != NULL);
	g_return_if_fail (text != NULL);

#if GTK_MAJOR_VERSION >= 4
	GdkDisplay *display = gtk_widget_get_display (selection->widget);
	GdkClipboard *clipboard;
	GdkContentProvider *provider;
	GValue value = G_VALUE_INIT;
	(void) event_time;

	if (display == NULL)
		return;

	g_value_init (&value, G_TYPE_STRING);
	g_value_take_string (&value,
		fabulor_xtext_selection_copy_text (text, length));
	provider = gdk_content_provider_new_for_value (&value);
	g_value_unset (&value);

	g_clear_object (&selection->provider);
	selection->provider = provider;
	clipboard = gdk_display_get_clipboard (display);
	selection->publishing = TRUE;
	gdk_clipboard_set_content (clipboard, provider);
	if (selection->primary && selection->primary != clipboard)
		gdk_clipboard_set_content (selection->primary, provider);
	selection->publishing = FALSE;
#else
	GtkClipboard *clipboard = gtk_widget_get_clipboard (selection->widget,
		GDK_SELECTION_CLIPBOARD);

	gtk_clipboard_set_text (clipboard, text, length);
	gtk_selection_owner_set (selection->widget, GDK_SELECTION_PRIMARY,
		event_time);
	gtk_selection_owner_set (selection->widget, GDK_SELECTION_SECONDARY,
		event_time);
#endif
}
