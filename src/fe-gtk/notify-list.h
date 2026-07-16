/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_NOTIFY_LIST_H
#define FABULOR_NOTIFY_LIST_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorNotifyList FabulorNotifyList;

typedef struct
{
	gpointer owner;
	gpointer server_data;
	const gchar *owner_name;
	const gchar *display_name;
	const gchar *status;
	const gchar *network;
	const gchar *last_seen;
	const GdkRGBA *foreground;
} FabulorNotifyListRow;

typedef void (*FabulorNotifyListSelectionChangedFunc) (gpointer user_data);

FabulorNotifyList *fabulor_notify_list_new (
	FabulorNotifyListSelectionChangedFunc selection_changed,
	gpointer user_data);
void fabulor_notify_list_free (FabulorNotifyList *list);
GtkWidget *fabulor_notify_list_create_view (FabulorNotifyList *list,
	GtkBox *parent, const gchar *name_title, const gchar *status_title,
	const gchar *network_title, const gchar *last_seen_title);

void fabulor_notify_list_begin_update (FabulorNotifyList *list);
gboolean fabulor_notify_list_append (FabulorNotifyList *list,
	const FabulorNotifyListRow *row);
void fabulor_notify_list_end_update (FabulorNotifyList *list);

guint fabulor_notify_list_get_n_rows (FabulorNotifyList *list);
gboolean fabulor_notify_list_has_selection (FabulorNotifyList *list);
gchar *fabulor_notify_list_dup_selected_name (FabulorNotifyList *list);
gpointer fabulor_notify_list_get_selected_server_data (
	FabulorNotifyList *list);
gboolean fabulor_notify_list_select_identity (FabulorNotifyList *list,
	gpointer owner, gpointer server_data);

G_END_DECLS

#endif
