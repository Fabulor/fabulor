/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_USER_LIST_VIEW_H
#define FABULOR_USER_LIST_VIEW_H

#include <gtk/gtk.h>

#include "user-list-model.h"

G_BEGIN_DECLS

GtkWidget *fabulor_user_list_view_new (gboolean compact,
	gboolean show_hosts, gint *nick_width, gint *host_width);
void fabulor_user_list_view_set_model (GtkWidget *view,
	FabulorUserListModel *model);
FabulorUserListModel *fabulor_user_list_view_get_model (GtkWidget *view);

gfloat fabulor_user_list_view_get_scroll_value (GtkWidget *view);
void fabulor_user_list_view_set_scroll_value (GtkWidget *view, gfloat value);

GPtrArray *fabulor_user_list_view_dup_selected_users (GtkWidget *view);
gboolean fabulor_user_list_view_is_user_selected (GtkWidget *view,
	gpointer user);
gboolean fabulor_user_list_view_select_user (GtkWidget *view, gpointer user,
	gboolean toggle, gboolean clear_others, gboolean scroll_to);
gpointer fabulor_user_list_view_get_user_at_position (GtkWidget *view,
	gdouble x, gdouble y);
gboolean fabulor_user_list_view_select_at_position (GtkWidget *view,
	gdouble x, gdouble y, gboolean clear_others);
void fabulor_user_list_view_unselect_all (GtkWidget *view);

G_END_DECLS

#endif
