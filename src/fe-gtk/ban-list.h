/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_BAN_LIST_H
#define FABULOR_BAN_LIST_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorBanList FabulorBanList;
typedef void (*FabulorBanListSelectionFunc) (guint selected,
	gpointer user_data);

FabulorBanList *fabulor_ban_list_new (
	FabulorBanListSelectionFunc selection_func, gpointer user_data);
void fabulor_ban_list_free (FabulorBanList *list);
GtkWidget *fabulor_ban_list_create_view (FabulorBanList *list,
	GtkBox *parent, const gchar *type_title, const gchar *mask_title,
	const gchar *from_title, const gchar *date_title);
void fabulor_ban_list_append (FabulorBanList *list, guint mode,
	const gchar *type, const gchar *mask, const gchar *from,
	const gchar *date);
void fabulor_ban_list_clear (FabulorBanList *list);
guint fabulor_ban_list_get_n_rows (FabulorBanList *list);
guint fabulor_ban_list_get_n_selected (FabulorBanList *list);
void fabulor_ban_list_select_all (FabulorBanList *list);
gboolean fabulor_ban_list_set_selected (FabulorBanList *list,
	guint position, gboolean selected);
void fabulor_ban_list_invert_selection (FabulorBanList *list);
GPtrArray *fabulor_ban_list_dup_masks (FabulorBanList *list, guint mode,
	gboolean selected);
gboolean fabulor_ban_list_select_at_point (FabulorBanList *list,
	gdouble x, gdouble y);
gchar *fabulor_ban_list_dup_selected_mask (FabulorBanList *list);
gchar *fabulor_ban_list_dup_selected_entry (FabulorBanList *list,
	const gchar *format);

G_END_DECLS

#endif
