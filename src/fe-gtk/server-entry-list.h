/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_SERVER_ENTRY_LIST_H
#define FABULOR_SERVER_ENTRY_LIST_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorServerEntryList FabulorServerEntryList;

typedef enum
{
	FABULOR_SERVER_ENTRY_PRIMARY,
	FABULOR_SERVER_ENTRY_SECONDARY
} FabulorServerEntryField;

typedef void (*FabulorServerEntrySelectionFunc) (gpointer identity,
	gpointer user_data);
typedef gboolean (*FabulorServerEntryEditFunc) (gpointer identity,
	FabulorServerEntryField field, const gchar *new_text, gpointer user_data);

FabulorServerEntryList *fabulor_server_entry_list_new (
	gboolean has_secondary, FabulorServerEntrySelectionFunc selection_func,
	FabulorServerEntryEditFunc edit_func, gpointer user_data);
void fabulor_server_entry_list_free (FabulorServerEntryList *list);
GtkWidget *fabulor_server_entry_list_create_view (
	FabulorServerEntryList *list, GtkScrolledWindow *scroller,
	const gchar *primary_title, const gchar *secondary_title,
	gboolean headers_visible);

gboolean fabulor_server_entry_list_append (FabulorServerEntryList *list,
	gpointer identity, const gchar *primary, const gchar *secondary);
void fabulor_server_entry_list_clear (FabulorServerEntryList *list);
guint fabulor_server_entry_list_get_n_rows (FabulorServerEntryList *list);
gpointer fabulor_server_entry_list_get_identity_at (
	FabulorServerEntryList *list, guint position);
gchar *fabulor_server_entry_list_dup_text (FabulorServerEntryList *list,
	gpointer identity, FabulorServerEntryField field);

gpointer fabulor_server_entry_list_get_selected (FabulorServerEntryList *list);
gboolean fabulor_server_entry_list_select (FabulorServerEntryList *list,
	gpointer identity);
gboolean fabulor_server_entry_list_remove (FabulorServerEntryList *list,
	gpointer identity);
gboolean fabulor_server_entry_list_move (FabulorServerEntryList *list,
	gpointer identity, gint delta);
gboolean fabulor_server_entry_list_update (FabulorServerEntryList *list,
	gpointer identity, FabulorServerEntryField field, const gchar *text);
void fabulor_server_entry_list_start_editing_selected (
	FabulorServerEntryList *list);

G_END_DECLS

#endif
