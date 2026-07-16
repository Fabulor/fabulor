/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_SERVER_NETWORK_LIST_H
#define FABULOR_SERVER_NETWORK_LIST_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorServerNetworkList FabulorServerNetworkList;

typedef void (*FabulorServerNetworkSelectionFunc) (gpointer identity,
	gpointer user_data);
typedef gboolean (*FabulorServerNetworkEditFunc) (gpointer identity,
	const gchar *new_name, gpointer user_data);

FabulorServerNetworkList *fabulor_server_network_list_new (
	FabulorServerNetworkSelectionFunc selection_func,
	FabulorServerNetworkEditFunc edit_func, gpointer user_data);
void fabulor_server_network_list_free (FabulorServerNetworkList *list);

GtkWidget *fabulor_server_network_list_create_view (
	FabulorServerNetworkList *list, GtkScrolledWindow *scroller);
gboolean fabulor_server_network_list_append (FabulorServerNetworkList *list,
	gpointer identity, const gchar *name, gboolean favorite, gboolean prepend);
void fabulor_server_network_list_clear (FabulorServerNetworkList *list);
guint fabulor_server_network_list_get_n_rows (FabulorServerNetworkList *list);
gpointer fabulor_server_network_list_get_identity_at (
	FabulorServerNetworkList *list, guint position);
gchar *fabulor_server_network_list_dup_name (
	FabulorServerNetworkList *list, gpointer identity);
gboolean fabulor_server_network_list_get_favorite (
	FabulorServerNetworkList *list, gpointer identity, gboolean *favorite);

gpointer fabulor_server_network_list_get_selected (
	FabulorServerNetworkList *list);
gboolean fabulor_server_network_list_select (
	FabulorServerNetworkList *list, gpointer identity);
gboolean fabulor_server_network_list_select_first (
	FabulorServerNetworkList *list);
gboolean fabulor_server_network_list_remove (
	FabulorServerNetworkList *list, gpointer identity);
gboolean fabulor_server_network_list_move (
	FabulorServerNetworkList *list, gpointer identity, gint delta);
gboolean fabulor_server_network_list_set_favorite (
	FabulorServerNetworkList *list, gpointer identity, gboolean favorite);
gboolean fabulor_server_network_list_update_name (
	FabulorServerNetworkList *list, gpointer identity, const gchar *name);
void fabulor_server_network_list_start_editing_selected (
	FabulorServerNetworkList *list);

G_END_DECLS

#endif
