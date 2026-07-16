/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_DCC_TRANSFER_LIST_H
#define FABULOR_DCC_TRANSFER_LIST_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorDccTransferList FabulorDccTransferList;

typedef struct
{
	gpointer identity;
	gboolean upload;
	const gchar *status;
	const gchar *file;
	const gchar *size;
	const gchar *position;
	const gchar *percentage;
	const gchar *speed;
	const gchar *eta;
	const gchar *nick;
	gboolean has_color;
	GdkRGBA color;
} FabulorDccTransferSnapshot;

typedef void (*FabulorDccTransferSelectionFunc) (gpointer user_data);
typedef void (*FabulorDccTransferActivateFunc) (gpointer identity,
	gpointer user_data);

FabulorDccTransferList *fabulor_dcc_transfer_list_new (
	FabulorDccTransferSelectionFunc selection_func,
	FabulorDccTransferActivateFunc activate_func, gpointer user_data);
void fabulor_dcc_transfer_list_free (FabulorDccTransferList *list);
GtkWidget *fabulor_dcc_transfer_list_create_view (
	FabulorDccTransferList *list, GtkBox *parent, const gchar *status_title,
	const gchar *file_title, const gchar *size_title,
	const gchar *position_title, const gchar *percentage_title,
	const gchar *speed_title, const gchar *eta_title,
	const gchar *nick_title);
gboolean fabulor_dcc_transfer_list_append (FabulorDccTransferList *list,
	const FabulorDccTransferSnapshot *snapshot, gboolean prepend);
gboolean fabulor_dcc_transfer_list_update (FabulorDccTransferList *list,
	const FabulorDccTransferSnapshot *snapshot);
gboolean fabulor_dcc_transfer_list_remove (FabulorDccTransferList *list,
	gpointer identity);
void fabulor_dcc_transfer_list_clear (FabulorDccTransferList *list);
guint fabulor_dcc_transfer_list_get_n_rows (FabulorDccTransferList *list);
guint fabulor_dcc_transfer_list_get_n_selected (FabulorDccTransferList *list);
gboolean fabulor_dcc_transfer_list_select_first (FabulorDccTransferList *list);
gboolean fabulor_dcc_transfer_list_set_selected (FabulorDccTransferList *list,
	guint position, gboolean selected);
gpointer fabulor_dcc_transfer_list_get_first_selected (
	FabulorDccTransferList *list);
GPtrArray *fabulor_dcc_transfer_list_dup_selected (
	FabulorDccTransferList *list);
GPtrArray *fabulor_dcc_transfer_list_dup_all (FabulorDccTransferList *list);

G_END_DECLS

#endif
