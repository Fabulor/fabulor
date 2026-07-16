/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_ADDON_LIST_H
#define FABULOR_ADDON_LIST_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorAddonList FabulorAddonList;

FabulorAddonList *fabulor_addon_list_new (void);
void fabulor_addon_list_free (FabulorAddonList *list);
GtkWidget *fabulor_addon_list_create_view (FabulorAddonList *list,
	GtkBox *parent, const gchar *name_title, const gchar *version_title,
	const gchar *file_title, const gchar *description_title);
void fabulor_addon_list_clear (FabulorAddonList *list);
void fabulor_addon_list_append (FabulorAddonList *list, const gchar *name,
	const gchar *version, const gchar *file, const gchar *description,
	const gchar *filepath);
guint fabulor_addon_list_get_n_rows (FabulorAddonList *list);
gboolean fabulor_addon_list_dup_selected (FabulorAddonList *list,
	gchar **name, gchar **filepath);
gchar *fabulor_addon_list_dup_selected_path (FabulorAddonList *list);

G_END_DECLS

#endif
