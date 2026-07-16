/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_IGNORE_LIST_H
#define FABULOR_IGNORE_LIST_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorIgnoreList FabulorIgnoreList;

typedef gboolean (*FabulorIgnoreListRenameFunc) (const gchar *old_mask,
	const gchar *new_mask, guint flags, gpointer user_data);
typedef void (*FabulorIgnoreListFlagsFunc) (const gchar *mask, guint flags,
	gpointer user_data);

FabulorIgnoreList *fabulor_ignore_list_new (
	FabulorIgnoreListRenameFunc rename_func,
	FabulorIgnoreListFlagsFunc flags_func, gpointer user_data);
void fabulor_ignore_list_free (FabulorIgnoreList *list);
GtkWidget *fabulor_ignore_list_create_view (FabulorIgnoreList *list,
	GtkBox *parent, const gchar *mask_title, const gchar *channel_title,
	const gchar *private_title, const gchar *notice_title,
	const gchar *ctcp_title, const gchar *dcc_title,
	const gchar *invite_title, const gchar *unignore_title);
void fabulor_ignore_list_append (FabulorIgnoreList *list, const gchar *mask,
	guint flags, gboolean select);
void fabulor_ignore_list_clear (FabulorIgnoreList *list);
guint fabulor_ignore_list_get_n_rows (FabulorIgnoreList *list);
gchar *fabulor_ignore_list_dup_mask_at (FabulorIgnoreList *list,
	guint position);
guint fabulor_ignore_list_get_flags_at (FabulorIgnoreList *list,
	guint position);
gboolean fabulor_ignore_list_rename_at (FabulorIgnoreList *list,
	guint position, const gchar *new_mask);
gboolean fabulor_ignore_list_set_flag_at (FabulorIgnoreList *list,
	guint position, guint flag, gboolean active);
gchar *fabulor_ignore_list_remove_selected (FabulorIgnoreList *list);
GPtrArray *fabulor_ignore_list_dup_masks (FabulorIgnoreList *list);

G_END_DECLS

#endif
