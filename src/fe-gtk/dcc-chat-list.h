/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_DCC_CHAT_LIST_H
#define FABULOR_DCC_CHAT_LIST_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorDccChatList FabulorDccChatList;

typedef struct
{
	gpointer identity;
	const gchar *status;
	const gchar *nick;
	const gchar *received;
	const gchar *sent;
	const gchar *start_time;
	gboolean has_color;
	GdkRGBA color;
} FabulorDccChatSnapshot;

typedef void (*FabulorDccChatSelectionFunc) (guint selected,
	gpointer user_data);
typedef void (*FabulorDccChatActivateFunc) (gpointer identity,
	gpointer user_data);

FabulorDccChatList *fabulor_dcc_chat_list_new (
	FabulorDccChatSelectionFunc selection_func,
	FabulorDccChatActivateFunc activate_func, gpointer user_data);
void fabulor_dcc_chat_list_free (FabulorDccChatList *list);
GtkWidget *fabulor_dcc_chat_list_create_view (FabulorDccChatList *list,
	GtkBox *parent, const gchar *status_title, const gchar *nick_title,
	const gchar *received_title, const gchar *sent_title,
	const gchar *start_time_title);
gboolean fabulor_dcc_chat_list_append (FabulorDccChatList *list,
	const FabulorDccChatSnapshot *snapshot, gboolean prepend);
gboolean fabulor_dcc_chat_list_update (FabulorDccChatList *list,
	const FabulorDccChatSnapshot *snapshot);
gboolean fabulor_dcc_chat_list_remove (FabulorDccChatList *list,
	gpointer identity);
void fabulor_dcc_chat_list_clear (FabulorDccChatList *list);
guint fabulor_dcc_chat_list_get_n_rows (FabulorDccChatList *list);
guint fabulor_dcc_chat_list_get_n_selected (FabulorDccChatList *list);
gboolean fabulor_dcc_chat_list_set_selected (FabulorDccChatList *list,
	guint position, gboolean selected);
gboolean fabulor_dcc_chat_list_select_first (FabulorDccChatList *list);
GPtrArray *fabulor_dcc_chat_list_dup_selected (FabulorDccChatList *list);

G_END_DECLS

#endif
