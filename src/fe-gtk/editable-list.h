/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_EDITABLE_LIST_H
#define FABULOR_EDITABLE_LIST_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorEditableList FabulorEditableList;

typedef enum
{
	FABULOR_EDITABLE_LIST_NAME,
	FABULOR_EDITABLE_LIST_COMMAND
} FabulorEditableListField;

typedef struct
{
	gchar *name;
	gchar *command;
} FabulorEditableListRecord;

FabulorEditableList *fabulor_editable_list_new (void);
void fabulor_editable_list_free (FabulorEditableList *list);
GtkWidget *fabulor_editable_list_create_view (FabulorEditableList *list,
	GtkBox *parent, const gchar *name_title, const gchar *command_title);
void fabulor_editable_list_append (FabulorEditableList *list,
	const gchar *name, const gchar *command);
void fabulor_editable_list_add_empty (FabulorEditableList *list);
gboolean fabulor_editable_list_remove_selected (FabulorEditableList *list);
gboolean fabulor_editable_list_move_selected (FabulorEditableList *list,
	gint delta);
gboolean fabulor_editable_list_set_text_at (FabulorEditableList *list,
	guint position, FabulorEditableListField field, const gchar *text);
gboolean fabulor_editable_list_set_selected (FabulorEditableList *list,
	guint position);
guint fabulor_editable_list_get_n_rows (FabulorEditableList *list);
GPtrArray *fabulor_editable_list_dup_all (FabulorEditableList *list);
void fabulor_editable_list_record_free (FabulorEditableListRecord *record);

G_END_DECLS

#endif
