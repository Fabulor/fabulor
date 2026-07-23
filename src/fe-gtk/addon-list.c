/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "addon-list.h"

#include "gtk-compat.h"

#include "gtk4-list-models.h"

struct _FabulorAddonList
{
	GtkWidget *view;
	FabulorGtk4FlatModelStack *models;
};


typedef struct _FabulorAddonRow FabulorAddonRow;
typedef struct _FabulorAddonRowClass FabulorAddonRowClass;

struct _FabulorAddonRow
{
	GObject parent_instance;
	gchar *name;
	gchar *version;
	gchar *file;
	gchar *description;
	gchar *filepath;
};

struct _FabulorAddonRowClass
{
	GObjectClass parent_class;
};

#define FABULOR_TYPE_ADDON_ROW (fabulor_addon_row_get_type ())
#define FABULOR_ADDON_ROW(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), FABULOR_TYPE_ADDON_ROW, FabulorAddonRow))

G_DEFINE_TYPE (FabulorAddonRow, fabulor_addon_row, G_TYPE_OBJECT)

static void
fabulor_addon_row_finalize (GObject *object)
{
	FabulorAddonRow *row = FABULOR_ADDON_ROW (object);

	g_free (row->name);
	g_free (row->version);
	g_free (row->file);
	g_free (row->description);
	g_free (row->filepath);
	G_OBJECT_CLASS (fabulor_addon_row_parent_class)->finalize (object);
}

static void
fabulor_addon_row_class_init (FabulorAddonRowClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = fabulor_addon_row_finalize;
}

static void
fabulor_addon_row_init (FabulorAddonRow *row)
{
	(void) row;
}

static FabulorAddonRow *
addon_row_new (const gchar *name, const gchar *version, const gchar *file,
			   const gchar *description, const gchar *filepath)
{
	FabulorAddonRow *row = g_object_new (FABULOR_TYPE_ADDON_ROW, NULL);

	row->name = g_strdup (name);
	row->version = g_strdup (version);
	row->file = g_strdup (file);
	row->description = g_strdup (description);
	row->filepath = g_strdup (filepath);
	return row;
}

typedef enum
{
	ADDON_FIELD_NAME,
	ADDON_FIELD_VERSION,
	ADDON_FIELD_FILE,
	ADDON_FIELD_DESCRIPTION
} FabulorAddonField;

static const gchar *
addon_row_field (FabulorAddonRow *row, FabulorAddonField field)
{
	switch (field)
	{
	case ADDON_FIELD_NAME:
		return row->name;
	case ADDON_FIELD_VERSION:
		return row->version;
	case ADDON_FIELD_FILE:
		return row->file;
	case ADDON_FIELD_DESCRIPTION:
		return row->description;
	default:
		return "";
	}
}

static void
addon_factory_setup (GtkSignalListItemFactory *factory, GtkListItem *item,
				 gpointer user_data)
{
	GtkWidget *label = gtk_label_new (NULL);

	(void) factory;
	(void) user_data;
	gtk_label_set_xalign (GTK_LABEL (label), 0.0f);
	gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
	gtk_list_item_set_child (item, label);
}

static void
addon_factory_bind (GtkSignalListItemFactory *factory, GtkListItem *item,
				gpointer user_data)
{
	FabulorAddonRow *row = FABULOR_ADDON_ROW (gtk_list_item_get_item (item));
	GtkLabel *label = GTK_LABEL (gtk_list_item_get_child (item));

	(void) factory;
	gtk_label_set_text (label, addon_row_field (row,
		(FabulorAddonField) GPOINTER_TO_UINT (user_data)));
}

static void
addon_column_append (GtkColumnView *view, const gchar *title,
				 FabulorAddonField field, gboolean expand)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	GtkColumnViewColumn *column;

	g_signal_connect (factory, "setup", G_CALLBACK (addon_factory_setup), NULL);
	g_signal_connect (factory, "bind", G_CALLBACK (addon_factory_bind),
		GUINT_TO_POINTER ((guint) field));
	column = gtk_column_view_column_new (title, factory);
	gtk_column_view_column_set_expand (column, expand);
	gtk_column_view_column_set_resizable (column, TRUE);
	gtk_column_view_append_column (view, column);
	g_object_unref (column);
}

static FabulorAddonRow *
addon_selected_row (FabulorAddonList *list)
{
	GtkSelectionModel *selection =
		fabulor_gtk4_flat_model_stack_get_selection (list->models);

	if (!GTK_IS_SINGLE_SELECTION (selection))
		return NULL;
	return FABULOR_ADDON_ROW (gtk_single_selection_get_selected_item (
		GTK_SINGLE_SELECTION (selection)));
}


FabulorAddonList *
fabulor_addon_list_new (void)
{
	FabulorAddonList *list = g_new0 (FabulorAddonList, 1);

	list->models = fabulor_gtk4_flat_model_stack_new (FABULOR_TYPE_ADDON_ROW,
		NULL, FABULOR_GTK4_SELECTION_SINGLE);
	if (!list->models)
	{
		g_free (list);
		return NULL;
	}
	return list;
}

void
fabulor_addon_list_free (FabulorAddonList *list)
{
	if (!list)
		return;
	fabulor_gtk4_flat_model_stack_free (list->models);
	g_free (list);
}

GtkWidget *
fabulor_addon_list_create_view (FabulorAddonList *list, GtkBox *parent,
								const gchar *name_title,
								const gchar *version_title,
								const gchar *file_title,
								const gchar *description_title)
{
	g_return_val_if_fail (list != NULL, NULL);
	g_return_val_if_fail (GTK_IS_BOX (parent), NULL);
	g_return_val_if_fail (list->view == NULL, NULL);

	{
		GtkWidget *scroller = gtk_scrolled_window_new ();
		GtkSelectionModel *selection =
			fabulor_gtk4_flat_model_stack_get_selection (list->models);

		list->view = gtk_column_view_new (GTK_SELECTION_MODEL (
			g_object_ref (selection)));
		addon_column_append (GTK_COLUMN_VIEW (list->view), name_title,
			ADDON_FIELD_NAME, TRUE);
		addon_column_append (GTK_COLUMN_VIEW (list->view), version_title,
			ADDON_FIELD_VERSION, FALSE);
		addon_column_append (GTK_COLUMN_VIEW (list->view), file_title,
			ADDON_FIELD_FILE, FALSE);
		addon_column_append (GTK_COLUMN_VIEW (list->view), description_title,
			ADDON_FIELD_DESCRIPTION, TRUE);
		fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroller),
			list->view);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_widget_set_hexpand (scroller, TRUE);
		gtk_widget_set_vexpand (scroller, TRUE);
		fabulor_gtk_box_append (parent, scroller, TRUE, TRUE, 0);
	}
	return list->view;
}

void
fabulor_addon_list_clear (FabulorAddonList *list)
{
	g_return_if_fail (list != NULL);
	fabulor_gtk4_flat_model_stack_clear (list->models);
}

void
fabulor_addon_list_append (FabulorAddonList *list, const gchar *name,
					   const gchar *version, const gchar *file,
					   const gchar *description, const gchar *filepath)
{
	g_return_if_fail (list != NULL);

	{
		FabulorAddonRow *row = addon_row_new (name, version, file, description,
			filepath);
		fabulor_gtk4_flat_model_stack_append (list->models, row);
		g_object_unref (row);
	}
}

guint
fabulor_addon_list_get_n_rows (FabulorAddonList *list)
{
	g_return_val_if_fail (list != NULL, 0);
	return g_list_model_get_n_items (G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_store (list->models)));
}

gboolean
fabulor_addon_list_dup_selected (FabulorAddonList *list, gchar **name,
								 gchar **filepath)
{
	g_return_val_if_fail (list != NULL, FALSE);
	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (filepath != NULL, FALSE);
	*name = NULL;
	*filepath = NULL;

	{
		FabulorAddonRow *row = addon_selected_row (list);
		if (!row)
			return FALSE;
		*name = g_strdup (row->name);
		*filepath = g_strdup (row->filepath);
	}
	return TRUE;
}

gchar *
fabulor_addon_list_dup_selected_path (FabulorAddonList *list)
{
	gchar *name;
	gchar *filepath;

	if (!fabulor_addon_list_dup_selected (list, &name, &filepath))
		return NULL;
	g_free (name);
	return filepath;
}
