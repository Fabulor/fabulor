/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "xtext-selection.h"



struct _FabulorXTextSelection
{
	GtkWidget *widget;
	FabulorXTextSelectionTextFunc text_func;
	FabulorXTextSelectionClearFunc clear_func;
	gpointer user_data;
	GdkClipboard *primary;
	GdkContentProvider *provider;
	gulong changed_handler;
	gboolean publishing;
};

gchar *
fabulor_xtext_selection_copy_text (const gchar *text, gssize length)
{
	g_return_val_if_fail (text != NULL, NULL);

	return length < 0 ? g_strdup (text) : g_strndup (text, (gsize) length);
}

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

	GdkDisplay *display = gtk_widget_get_display (widget);

	if (display)
		selection->primary = gdk_display_get_primary_clipboard (display);
	if (selection->primary)
		selection->changed_handler = g_signal_connect (selection->primary,
			"changed", G_CALLBACK (fabulor_xtext_selection_changed), selection);

	return selection;
}

void
fabulor_xtext_selection_free (FabulorXTextSelection *selection)
{
	if (selection == NULL)
		return;

	if (selection->primary && selection->changed_handler)
		g_signal_handler_disconnect (selection->primary,
			selection->changed_handler);
	g_clear_object (&selection->provider);
	g_free (selection);
}

void
fabulor_xtext_selection_publish (FabulorXTextSelection *selection,
	const gchar *text, gint length, guint32 event_time)
{
	g_return_if_fail (selection != NULL);
	g_return_if_fail (text != NULL);

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
}
