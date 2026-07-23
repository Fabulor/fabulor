/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_USER_LIST_MODEL_H
#define FABULOR_USER_LIST_MODEL_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorUserListModel FabulorUserListModel;

typedef gint (*FabulorUserListCompareFunc) (gconstpointer left_user,
	gconstpointer right_user, gpointer user_data);

typedef struct
{
	gpointer user;
	GdkPixbuf *icon;
	const gchar *prefix_markup;
	const gchar *nick_markup;
	const gchar *hostname;
	const GdkRGBA *foreground;
} FabulorUserListRow;

enum
{
	FABULOR_USER_LIST_COLUMN_ICON,
	FABULOR_USER_LIST_COLUMN_PREFIX,
	FABULOR_USER_LIST_COLUMN_NICK,
	FABULOR_USER_LIST_COLUMN_HOST,
	FABULOR_USER_LIST_COLUMN_USER,
	FABULOR_USER_LIST_COLUMN_FOREGROUND,
	FABULOR_USER_LIST_N_COLUMNS
};

FabulorUserListModel *fabulor_user_list_model_new (
	FabulorUserListCompareFunc compare, gpointer compare_data,
	gboolean descending);
void fabulor_user_list_model_free (FabulorUserListModel *model);

gboolean fabulor_user_list_model_insert (FabulorUserListModel *model,
	const FabulorUserListRow *row);
gboolean fabulor_user_list_model_update (FabulorUserListModel *model,
	const FabulorUserListRow *row, gboolean sort_changed);
gboolean fabulor_user_list_model_remove (FabulorUserListModel *model,
	gpointer user);
void fabulor_user_list_model_clear (FabulorUserListModel *model);

guint fabulor_user_list_model_get_n_rows (FabulorUserListModel *model);
gpointer fabulor_user_list_model_get_user_at (FabulorUserListModel *model,
	guint position);

GListModel *fabulor_user_list_model_get_list_model (
	FabulorUserListModel *model);
GtkSelectionModel *fabulor_user_list_model_get_selection (
	FabulorUserListModel *model);
gpointer fabulor_user_list_model_get_item_user (gpointer item);
GdkPixbuf *fabulor_user_list_model_get_item_icon (gpointer item);
const gchar *fabulor_user_list_model_get_item_prefix (gpointer item);
const gchar *fabulor_user_list_model_get_item_nick (gpointer item);
const gchar *fabulor_user_list_model_get_item_host (gpointer item);
const GdkRGBA *fabulor_user_list_model_get_item_foreground (gpointer item);

G_END_DECLS

#endif
