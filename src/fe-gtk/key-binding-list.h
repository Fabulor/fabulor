/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_KEY_BINDING_LIST_H
#define FABULOR_KEY_BINDING_LIST_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorKeyBindingList FabulorKeyBindingList;

typedef enum
{
	FABULOR_KEY_BINDING_ACTION,
	FABULOR_KEY_BINDING_DATA1,
	FABULOR_KEY_BINDING_DATA2
} FabulorKeyBindingField;

typedef struct
{
	gchar *key_label;
	gchar *accelerator;
	gchar *action;
	gchar *data1;
	gchar *data2;
	gboolean custom;
} FabulorKeyBindingRecord;

typedef void (*FabulorKeyBindingSelectionFunc) (const gchar *action,
	gboolean custom, gpointer user_data);
typedef GdkModifierType (*FabulorKeyBindingNormalizeFunc) (
	GdkModifierType modifiers, gpointer user_data);

FabulorKeyBindingList *fabulor_key_binding_list_new (
	FabulorKeyBindingSelectionFunc selection_func,
	FabulorKeyBindingNormalizeFunc normalize_func, gpointer user_data);
void fabulor_key_binding_list_free (FabulorKeyBindingList *list);
GtkWidget *fabulor_key_binding_list_create_view (FabulorKeyBindingList *list,
	GtkBox *parent, const gchar *key_title, const gchar *action_title,
	const gchar *data1_title, const gchar *data2_title,
	const gchar *const *actions, guint n_actions);
void fabulor_key_binding_list_append (FabulorKeyBindingList *list,
	const FabulorKeyBindingRecord *record);
void fabulor_key_binding_list_clear (FabulorKeyBindingList *list);
guint fabulor_key_binding_list_add_custom (FabulorKeyBindingList *list);
gboolean fabulor_key_binding_list_remove_selected (FabulorKeyBindingList *list);
gboolean fabulor_key_binding_list_move_selected (FabulorKeyBindingList *list,
	gint delta);
gboolean fabulor_key_binding_list_set_accelerator_at (
	FabulorKeyBindingList *list, guint position, guint keyval,
	GdkModifierType modifiers);
gboolean fabulor_key_binding_list_set_text_at (FabulorKeyBindingList *list,
	guint position, FabulorKeyBindingField field, const gchar *text);
gboolean fabulor_key_binding_list_set_selected (FabulorKeyBindingList *list,
	guint position);
guint fabulor_key_binding_list_get_n_rows (FabulorKeyBindingList *list);
GPtrArray *fabulor_key_binding_list_dup_all (FabulorKeyBindingList *list);
void fabulor_key_binding_record_free (FabulorKeyBindingRecord *record);

G_END_DECLS

#endif
