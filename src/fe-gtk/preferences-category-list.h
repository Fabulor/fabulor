/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_PREFERENCES_CATEGORY_LIST_H
#define FABULOR_PREFERENCES_CATEGORY_LIST_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorPreferencesCategoryList FabulorPreferencesCategoryList;

typedef void (*FabulorPreferencesCategorySelectionFunc) (gint page_index,
	                                                      gpointer user_data);

FabulorPreferencesCategoryList *fabulor_preferences_category_list_new (
	FabulorPreferencesCategorySelectionFunc selection_func, gpointer user_data);
void fabulor_preferences_category_list_free (
	FabulorPreferencesCategoryList *list);

guint fabulor_preferences_category_list_append_category (
	FabulorPreferencesCategoryList *list, const gchar *title);
void fabulor_preferences_category_list_append_page (
	FabulorPreferencesCategoryList *list, guint category_position,
	const gchar *title, gint page_index);
GtkWidget *fabulor_preferences_category_list_create_view (
	FabulorPreferencesCategoryList *list, GtkBox *parent,
	const gchar *column_title);

guint fabulor_preferences_category_list_get_n_categories (
	FabulorPreferencesCategoryList *list);
guint fabulor_preferences_category_list_get_n_pages (
	FabulorPreferencesCategoryList *list);
gboolean fabulor_preferences_category_list_select_page (
	FabulorPreferencesCategoryList *list, gint page_index);
gint fabulor_preferences_category_list_get_selected_page (
	FabulorPreferencesCategoryList *list);

G_END_DECLS

#endif
