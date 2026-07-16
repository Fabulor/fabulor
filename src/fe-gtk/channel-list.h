/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_CHANNEL_LIST_H
#define FABULOR_CHANNEL_LIST_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorChannelList FabulorChannelList;

typedef struct
{
	gpointer identity;
	const gchar *channel;
	guint users;
	const gchar *topic;
	const gchar *collation_key;
} FabulorChannelListSnapshot;

typedef struct
{
	gchar *channel;
	guint users;
	gchar *topic;
} FabulorChannelListRecord;

typedef enum
{
	FABULOR_CHANNEL_LIST_CHANNEL,
	FABULOR_CHANNEL_LIST_TOPIC
} FabulorChannelListTextField;

typedef void (*FabulorChannelListActivateFunc) (gpointer user_data);

FabulorChannelList *fabulor_channel_list_new (
	FabulorChannelListActivateFunc activate_func, gpointer user_data);
void fabulor_channel_list_free (FabulorChannelList *list);
GtkWidget *fabulor_channel_list_create_view (FabulorChannelList *list,
	GtkBox *parent, const gchar *channel_title, const gchar *users_title,
	const gchar *topic_title, gint channel_width, gint users_width,
	gint topic_width);
GtkWidget *fabulor_channel_list_get_view (FabulorChannelList *list);
gboolean fabulor_channel_list_append (FabulorChannelList *list,
	const FabulorChannelListSnapshot *snapshot);
void fabulor_channel_list_clear (FabulorChannelList *list);
void fabulor_channel_list_resort (FabulorChannelList *list);
guint fabulor_channel_list_get_n_rows (FabulorChannelList *list);
gboolean fabulor_channel_list_set_selected (FabulorChannelList *list,
	guint position, gboolean selected);
gboolean fabulor_channel_list_select_at_point (FabulorChannelList *list,
	gdouble x, gdouble y);
GPtrArray *fabulor_channel_list_dup_selected_text (FabulorChannelList *list,
	FabulorChannelListTextField field);
gchar *fabulor_channel_list_dup_first_selected_channel (
	FabulorChannelList *list);
GPtrArray *fabulor_channel_list_dup_all (FabulorChannelList *list);
void fabulor_channel_list_get_column_widths (FabulorChannelList *list,
	gint *channel_width, gint *users_width, gint *topic_width);
void fabulor_channel_list_record_free (FabulorChannelListRecord *record);

G_END_DECLS

#endif
