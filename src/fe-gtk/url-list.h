/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_URL_LIST_H
#define FABULOR_URL_LIST_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorUrlList FabulorUrlList;

FabulorUrlList *fabulor_url_list_new (void);
void fabulor_url_list_free (FabulorUrlList *list);
GtkWidget *fabulor_url_list_create_view (FabulorUrlList *list,
	GtkBox *parent, const gchar *title);
GtkWidget *fabulor_url_list_get_view (FabulorUrlList *list);
void fabulor_url_list_clear (FabulorUrlList *list);
void fabulor_url_list_prepend (FabulorUrlList *list, const gchar *url,
	guint limit);
guint fabulor_url_list_get_n_rows (FabulorUrlList *list);
gchar *fabulor_url_list_dup_at (FabulorUrlList *list, guint position);
gchar *fabulor_url_list_dup_selected (FabulorUrlList *list);
gchar *fabulor_url_list_select_and_dup_at_point (FabulorUrlList *list,
	gdouble x, gdouble y);

G_END_DECLS

#endif
