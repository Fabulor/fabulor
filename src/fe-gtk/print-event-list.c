/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "print-event-list.h"

#include "gtk-compat.h"

struct _FabulorPrintEventList
{
	GtkWidget *event_view;
	GtkWidget *help_view;
	FabulorPrintEventEditFunc edit_func;
	FabulorPrintEventSelectionFunc selection_func;
	gpointer callback_data;
#if GTK_MAJOR_VERSION >= 4
	GListStore *event_store;
	GListStore *help_store;
	GtkSingleSelection *event_selection;
#else
	GtkListStore *event_store;
	GtkListStore *help_store;
#endif
};

#if GTK_MAJOR_VERSION >= 4

typedef struct _FabulorPrintEventRow FabulorPrintEventRow;
typedef struct _FabulorPrintEventRowClass FabulorPrintEventRowClass;
typedef struct _FabulorPrintEventHelpRow FabulorPrintEventHelpRow;
typedef struct _FabulorPrintEventHelpRowClass FabulorPrintEventHelpRowClass;

struct _FabulorPrintEventRow
{
	GObject parent_instance;
	gchar *event_name;
	gchar *text;
	gint signal_index;
};

struct _FabulorPrintEventRowClass { GObjectClass parent_class; };

struct _FabulorPrintEventHelpRow
{
	GObject parent_instance;
	gint number;
	gchar *description;
};

struct _FabulorPrintEventHelpRowClass { GObjectClass parent_class; };

#define FABULOR_TYPE_PRINT_EVENT_ROW (fabulor_print_event_row_get_type ())
#define FABULOR_PRINT_EVENT_ROW(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), \
	FABULOR_TYPE_PRINT_EVENT_ROW, FabulorPrintEventRow))
#define FABULOR_TYPE_PRINT_EVENT_HELP_ROW \
	(fabulor_print_event_help_row_get_type ())
#define FABULOR_PRINT_EVENT_HELP_ROW(object) (G_TYPE_CHECK_INSTANCE_CAST ( \
	(object), FABULOR_TYPE_PRINT_EVENT_HELP_ROW, FabulorPrintEventHelpRow))

G_DEFINE_TYPE (FabulorPrintEventRow, fabulor_print_event_row, G_TYPE_OBJECT)
G_DEFINE_TYPE (FabulorPrintEventHelpRow, fabulor_print_event_help_row,
	G_TYPE_OBJECT)

static void
fabulor_print_event_row_finalize (GObject *object)
{
	FabulorPrintEventRow *row = FABULOR_PRINT_EVENT_ROW (object);
	g_free (row->event_name);
	g_free (row->text);
	G_OBJECT_CLASS (fabulor_print_event_row_parent_class)->finalize (object);
}

static void
fabulor_print_event_row_class_init (FabulorPrintEventRowClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = fabulor_print_event_row_finalize;
}

static void fabulor_print_event_row_init (FabulorPrintEventRow *row)
{
	(void) row;
}

static void
fabulor_print_event_help_row_finalize (GObject *object)
{
	g_free (FABULOR_PRINT_EVENT_HELP_ROW (object)->description);
	G_OBJECT_CLASS (fabulor_print_event_help_row_parent_class)->finalize (object);
}

static void
fabulor_print_event_help_row_class_init (FabulorPrintEventHelpRowClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = fabulor_print_event_help_row_finalize;
}

static void fabulor_print_event_help_row_init (FabulorPrintEventHelpRow *row)
{
	(void) row;
}

static FabulorPrintEventRow *
print_event_row_new (const gchar *event_name, const gchar *text,
	gint signal_index)
{
	FabulorPrintEventRow *row = g_object_new (FABULOR_TYPE_PRINT_EVENT_ROW, NULL);
	row->event_name = g_strdup (event_name ? event_name : "");
	row->text = g_strdup (text ? text : "");
	row->signal_index = signal_index;
	return row;
}

static FabulorPrintEventRow *
print_event_row_at (FabulorPrintEventList *list, guint position)
{
	return g_list_model_get_item (G_LIST_MODEL (list->event_store), position);
}

typedef enum
{
	PRINT_EVENT_CELL_EVENT,
	PRINT_EVENT_CELL_TEXT,
	PRINT_EVENT_CELL_NUMBER,
	PRINT_EVENT_CELL_DESCRIPTION
} PrintEventCell;

typedef struct
{
	FabulorPrintEventList *owner;
	PrintEventCell cell;
} PrintEventFactoryData;

typedef struct
{
	FabulorPrintEventList *owner;
	GtkEditableLabel *label;
	FabulorPrintEventRow *row;
	gboolean editing;
	gboolean blocked;
} PrintEventEditBinding;

static void
print_event_editing_changed (GtkEditableLabel *label, GParamSpec *pspec,
	gpointer user_data)
{
	PrintEventEditBinding *binding = user_data;
	gboolean editing = gtk_editable_label_get_editing (label);
	const gchar *text;

	(void) pspec;
	if (!binding->blocked && binding->row && binding->editing && !editing)
	{
		guint position;
		text = gtk_editable_get_text (GTK_EDITABLE (label));
		if (!g_list_store_find (binding->owner->event_store, binding->row,
			&position) || !fabulor_print_event_list_edit_at (binding->owner,
			position, text))
		{
			binding->blocked = TRUE;
			gtk_editable_set_text (GTK_EDITABLE (label), binding->row->text);
			binding->blocked = FALSE;
		}
	}
	binding->editing = editing;
}

static void
print_event_edit_binding_free (gpointer data)
{
	PrintEventEditBinding *binding = data;
	g_clear_object (&binding->row);
	g_free (binding);
}

static void
print_event_factory_setup (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	PrintEventFactoryData *data = user_data;
	GtkWidget *label;

	(void) factory;
	if (data->cell == PRINT_EVENT_CELL_TEXT)
	{
		PrintEventEditBinding *binding = g_new0 (PrintEventEditBinding, 1);
		label = gtk_editable_label_new (NULL);
		binding->owner = data->owner;
		binding->label = GTK_EDITABLE_LABEL (label);
		g_signal_connect (label, "notify::editing",
			G_CALLBACK (print_event_editing_changed), binding);
		g_object_set_data_full (G_OBJECT (item), "fabulor-print-event-binding",
			binding, print_event_edit_binding_free);
	}
	else
		label = gtk_label_new (NULL);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_hexpand (label, TRUE);
	gtk_list_item_set_child (item, label);
}

static void
print_event_factory_bind (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	PrintEventFactoryData *data = user_data;
	GtkWidget *label = gtk_list_item_get_child (item);
	gpointer object = gtk_list_item_get_item (item);
	const gchar *text = "";
	gchar *number = NULL;

	(void) factory;
	if (data->cell == PRINT_EVENT_CELL_EVENT)
		text = FABULOR_PRINT_EVENT_ROW (object)->event_name;
	else if (data->cell == PRINT_EVENT_CELL_TEXT)
	{
		PrintEventEditBinding *binding = g_object_get_data (G_OBJECT (item),
			"fabulor-print-event-binding");
		g_set_object (&binding->row, FABULOR_PRINT_EVENT_ROW (object));
		binding->blocked = TRUE;
		gtk_editable_set_text (GTK_EDITABLE (label), binding->row->text);
		binding->blocked = FALSE;
		binding->editing = gtk_editable_label_get_editing (binding->label);
		return;
	}
	else if (data->cell == PRINT_EVENT_CELL_NUMBER)
	{
		number = g_strdup_printf ("%d", FABULOR_PRINT_EVENT_HELP_ROW (object)->number);
		text = number;
	}
	else
		text = FABULOR_PRINT_EVENT_HELP_ROW (object)->description;
	gtk_label_set_text (GTK_LABEL (label), text);
	g_free (number);
}

static void
print_event_factory_unbind (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	PrintEventFactoryData *data = user_data;
	(void) factory;
	if (data->cell == PRINT_EVENT_CELL_TEXT)
	{
		PrintEventEditBinding *binding = g_object_get_data (G_OBJECT (item),
			"fabulor-print-event-binding");
		g_clear_object (&binding->row);
	}
}

static GtkColumnViewColumn *
print_event_column_new (FabulorPrintEventList *list, const gchar *title,
	PrintEventCell cell, gboolean expand)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	PrintEventFactoryData *data = g_new (PrintEventFactoryData, 1);
	GtkColumnViewColumn *column;
	data->owner = list;
	data->cell = cell;
	g_object_set_data_full (G_OBJECT (factory), "fabulor-print-event-factory",
		data, g_free);
	g_signal_connect (factory, "setup", G_CALLBACK (print_event_factory_setup), data);
	g_signal_connect (factory, "bind", G_CALLBACK (print_event_factory_bind), data);
	g_signal_connect (factory, "unbind", G_CALLBACK (print_event_factory_unbind), data);
	column = gtk_column_view_column_new (title ? title : "", factory);
	gtk_column_view_column_set_expand (column, expand);
	gtk_column_view_column_set_resizable (column, TRUE);
	return column;
}

static void
print_event_selected_changed (GtkSingleSelection *selection,
	GParamSpec *pspec, gpointer user_data)
{
	FabulorPrintEventList *list = user_data;
	guint position = gtk_single_selection_get_selected (selection);
	(void) pspec;
	if (list->selection_func)
		list->selection_func (position == GTK_INVALID_LIST_POSITION ? -1 :
			fabulor_print_event_list_get_signal_at (list, position),
			list->callback_data);
}

#else

enum { EVENT_NAME_COLUMN, EVENT_TEXT_COLUMN, EVENT_SIGNAL_COLUMN, N_EVENT_COLUMNS };
enum { HELP_NUMBER_COLUMN, HELP_DESCRIPTION_COLUMN, N_HELP_COLUMNS };

static void
print_event_gtk3_edited (GtkCellRendererText *renderer, gchar *path_string,
	gchar *new_text, gpointer user_data)
{
	FabulorPrintEventList *list = user_data;
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	gint *indices = gtk_tree_path_get_indices (path);
	(void) renderer;
	if (indices)
		fabulor_print_event_list_edit_at (list, (guint) indices[0], new_text);
	gtk_tree_path_free (path);
}

static void
print_event_gtk3_selection_changed (GtkTreeSelection *selection,
	gpointer user_data)
{
	FabulorPrintEventList *list = user_data;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gint signal_index = -1;
	if (gtk_tree_selection_get_selected (selection, &model, &iter))
		gtk_tree_model_get (model, &iter, EVENT_SIGNAL_COLUMN, &signal_index, -1);
	if (list->selection_func)
		list->selection_func (signal_index, list->callback_data);
}

#endif

FabulorPrintEventList *
fabulor_print_event_list_new (FabulorPrintEventEditFunc edit_func,
	FabulorPrintEventSelectionFunc selection_func, gpointer user_data)
{
	FabulorPrintEventList *list = g_new0 (FabulorPrintEventList, 1);
	list->edit_func = edit_func;
	list->selection_func = selection_func;
	list->callback_data = user_data;
#if GTK_MAJOR_VERSION >= 4
	list->event_store = g_list_store_new (FABULOR_TYPE_PRINT_EVENT_ROW);
	list->help_store = g_list_store_new (FABULOR_TYPE_PRINT_EVENT_HELP_ROW);
	list->event_selection = gtk_single_selection_new (
		G_LIST_MODEL (g_object_ref (list->event_store)));
	g_signal_connect (list->event_selection, "notify::selected",
		G_CALLBACK (print_event_selected_changed), list);
#else
	list->event_store = gtk_list_store_new (N_EVENT_COLUMNS, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_INT);
	list->help_store = gtk_list_store_new (N_HELP_COLUMNS, G_TYPE_INT,
		G_TYPE_STRING);
#endif
	return list;
}

void
fabulor_print_event_list_free (FabulorPrintEventList *list)
{
	if (!list)
		return;
#if GTK_MAJOR_VERSION >= 4
	g_clear_object (&list->event_selection);
#endif
	g_clear_object (&list->event_store);
	g_clear_object (&list->help_store);
	g_free (list);
}

gboolean
fabulor_print_event_list_create_views (FabulorPrintEventList *list,
	GtkPaned *parent, const gchar *event_title, const gchar *text_title,
	const gchar *number_title, const gchar *description_title)
{
	GtkWidget *event_scroller;
	GtkWidget *help_scroller;
	g_return_val_if_fail (list && GTK_IS_PANED (parent) && !list->event_view,
		FALSE);
#if GTK_MAJOR_VERSION >= 4
	event_scroller = gtk_scrolled_window_new ();
	help_scroller = gtk_scrolled_window_new ();
	list->event_view = gtk_column_view_new (GTK_SELECTION_MODEL (g_object_ref (
		list->event_selection)));
	gtk_column_view_append_column (GTK_COLUMN_VIEW (list->event_view),
		print_event_column_new (list, event_title, PRINT_EVENT_CELL_EVENT, FALSE));
	gtk_column_view_append_column (GTK_COLUMN_VIEW (list->event_view),
		print_event_column_new (list, text_title, PRINT_EVENT_CELL_TEXT, TRUE));
	gtk_column_view_set_show_row_separators (GTK_COLUMN_VIEW (list->event_view), TRUE);
	list->help_view = gtk_column_view_new (GTK_SELECTION_MODEL (
		gtk_no_selection_new (G_LIST_MODEL (g_object_ref (list->help_store)))));
	gtk_column_view_append_column (GTK_COLUMN_VIEW (list->help_view),
		print_event_column_new (list, number_title, PRINT_EVENT_CELL_NUMBER, FALSE));
	gtk_column_view_append_column (GTK_COLUMN_VIEW (list->help_view),
		print_event_column_new (list, description_title,
			PRINT_EVENT_CELL_DESCRIPTION, TRUE));
#else
	{
		GtkCellRenderer *renderer;
		GtkTreeViewColumn *column;
		event_scroller = gtk_scrolled_window_new (NULL, NULL);
		help_scroller = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (event_scroller), GTK_SHADOW_IN);
		gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (help_scroller), GTK_SHADOW_IN);
		list->event_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (g_object_ref (list->event_store)));
		gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (list->event_view), TRUE);
		gtk_tree_view_set_enable_search (GTK_TREE_VIEW (list->event_view), TRUE);
		gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (list->event_view), GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);
		g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (list->event_view)), "changed",
			G_CALLBACK (print_event_gtk3_selection_changed), list);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (list->event_view), EVENT_NAME_COLUMN,
			event_title, renderer, "text", EVENT_NAME_COLUMN, NULL);
		column = gtk_tree_view_get_column (GTK_TREE_VIEW (list->event_view), EVENT_NAME_COLUMN);
		gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
		gtk_tree_view_column_set_resizable (column, TRUE);
		gtk_tree_view_column_set_min_width (column, 100);
		renderer = gtk_cell_renderer_text_new ();
		g_object_set (renderer, "editable", TRUE, NULL);
		g_signal_connect (renderer, "edited", G_CALLBACK (print_event_gtk3_edited), list);
		gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (list->event_view), EVENT_TEXT_COLUMN,
			text_title, renderer, "text", EVENT_TEXT_COLUMN, NULL);
		list->help_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (g_object_ref (list->help_store)));
		gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (list->help_view), TRUE);
		gtk_tree_view_set_enable_search (GTK_TREE_VIEW (list->help_view), FALSE);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (list->help_view), HELP_NUMBER_COLUMN,
			number_title, renderer, "text", HELP_NUMBER_COLUMN, NULL);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (list->help_view), HELP_DESCRIPTION_COLUMN,
			description_title, renderer, "text", HELP_DESCRIPTION_COLUMN, NULL);
		column = gtk_tree_view_get_column (GTK_TREE_VIEW (list->help_view), HELP_NUMBER_COLUMN);
		gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	}
#endif
	gtk_widget_set_size_request (event_scroller, -1, 250);
	gtk_widget_set_can_focus (list->help_view, FALSE);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (event_scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (help_scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (event_scroller), list->event_view);
	fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (help_scroller), list->help_view);
	fabulor_gtk_paned_set_start_child (parent, event_scroller, TRUE, TRUE);
	fabulor_gtk_paned_set_end_child (parent, help_scroller, TRUE, TRUE);
#if GTK_MAJOR_VERSION < 4
	gtk_widget_show_all (event_scroller);
	gtk_widget_show_all (help_scroller);
#endif
	return TRUE;
}

void
fabulor_print_event_list_append_event (FabulorPrintEventList *list,
	const gchar *event_name, const gchar *text, gint signal_index)
{
	g_return_if_fail (list != NULL);
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorPrintEventRow *row = print_event_row_new (event_name, text, signal_index);
		g_list_store_append (list->event_store, row);
		g_object_unref (row);
	}
#else
	{
		GtkTreeIter iter;
		gtk_list_store_append (list->event_store, &iter);
		gtk_list_store_set (list->event_store, &iter, EVENT_NAME_COLUMN,
			event_name ? event_name : "", EVENT_TEXT_COLUMN, text ? text : "",
			EVENT_SIGNAL_COLUMN, signal_index, -1);
	}
#endif
}

void
fabulor_print_event_list_clear_events (FabulorPrintEventList *list)
{
	g_return_if_fail (list != NULL);
#if GTK_MAJOR_VERSION >= 4
	g_list_store_remove_all (list->event_store);
#else
	gtk_list_store_clear (list->event_store);
#endif
}

void
fabulor_print_event_list_append_help (FabulorPrintEventList *list,
	gint number, const gchar *description)
{
	g_return_if_fail (list != NULL);
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorPrintEventHelpRow *row = g_object_new (FABULOR_TYPE_PRINT_EVENT_HELP_ROW, NULL);
		row->number = number;
		row->description = g_strdup (description ? description : "");
		g_list_store_append (list->help_store, row);
		g_object_unref (row);
	}
#else
	{
		GtkTreeIter iter;
		gtk_list_store_append (list->help_store, &iter);
		gtk_list_store_set (list->help_store, &iter, HELP_NUMBER_COLUMN, number,
			HELP_DESCRIPTION_COLUMN, description ? description : "", -1);
	}
#endif
}

void
fabulor_print_event_list_clear_help (FabulorPrintEventList *list)
{
	g_return_if_fail (list != NULL);
#if GTK_MAJOR_VERSION >= 4
	g_list_store_remove_all (list->help_store);
#else
	gtk_list_store_clear (list->help_store);
#endif
}

guint fabulor_print_event_list_get_n_events (FabulorPrintEventList *list)
{
	g_return_val_if_fail (list != NULL, 0);
#if GTK_MAJOR_VERSION >= 4
	return g_list_model_get_n_items (G_LIST_MODEL (list->event_store));
#else
	return (guint) gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list->event_store), NULL);
#endif
}

guint fabulor_print_event_list_get_n_help (FabulorPrintEventList *list)
{
	g_return_val_if_fail (list != NULL, 0);
#if GTK_MAJOR_VERSION >= 4
	return g_list_model_get_n_items (G_LIST_MODEL (list->help_store));
#else
	return (guint) gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list->help_store), NULL);
#endif
}

gint
fabulor_print_event_list_get_signal_at (FabulorPrintEventList *list,
	guint position)
{
	gint signal_index = -1;
	g_return_val_if_fail (list != NULL, -1);
	if (position >= fabulor_print_event_list_get_n_events (list))
		return -1;
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorPrintEventRow *row = print_event_row_at (list, position);
		signal_index = row->signal_index;
		g_object_unref (row);
	}
#else
	{
		GtkTreeIter iter;
		gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->event_store), &iter, NULL, (gint) position);
		gtk_tree_model_get (GTK_TREE_MODEL (list->event_store), &iter, EVENT_SIGNAL_COLUMN, &signal_index, -1);
	}
#endif
	return signal_index;
}

gboolean
fabulor_print_event_list_edit_at (FabulorPrintEventList *list,
	guint position, const gchar *text)
{
	gint signal_index;
	g_return_val_if_fail (list != NULL, FALSE);
	if (position >= fabulor_print_event_list_get_n_events (list))
		return FALSE;
	signal_index = fabulor_print_event_list_get_signal_at (list, position);
	if (list->edit_func && !list->edit_func (signal_index, text ? text : "",
		list->callback_data))
		return FALSE;
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorPrintEventRow *row = print_event_row_at (list, position);
		g_free (row->text);
		row->text = g_strdup (text ? text : "");
		g_object_unref (row);
	}
#else
	{
		GtkTreeIter iter;
		gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->event_store), &iter, NULL, (gint) position);
		gtk_list_store_set (list->event_store, &iter, EVENT_TEXT_COLUMN, text ? text : "", -1);
	}
#endif
	return TRUE;
}

gboolean
fabulor_print_event_list_select_at (FabulorPrintEventList *list,
	guint position)
{
	g_return_val_if_fail (list != NULL, FALSE);
	if (position >= fabulor_print_event_list_get_n_events (list))
		return FALSE;
#if GTK_MAJOR_VERSION >= 4
	return gtk_selection_model_select_item (GTK_SELECTION_MODEL (list->event_selection), position, TRUE);
#else
	if (list->event_view)
	{
		GtkTreePath *path = gtk_tree_path_new_from_indices ((gint) position, -1);
		gtk_tree_view_set_cursor (GTK_TREE_VIEW (list->event_view), path, NULL, FALSE);
		gtk_tree_path_free (path);
		return TRUE;
	}
	if (list->selection_func)
		list->selection_func (fabulor_print_event_list_get_signal_at (list, position), list->callback_data);
	return TRUE;
#endif
}

gchar *
fabulor_print_event_list_dup_text_at (FabulorPrintEventList *list,
	guint position)
{
	gchar *text = NULL;
	g_return_val_if_fail (list != NULL, NULL);
	if (position >= fabulor_print_event_list_get_n_events (list))
		return NULL;
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorPrintEventRow *row = print_event_row_at (list, position);
		text = g_strdup (row->text);
		g_object_unref (row);
	}
#else
	{
		GtkTreeIter iter;
		gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->event_store), &iter, NULL, (gint) position);
		gtk_tree_model_get (GTK_TREE_MODEL (list->event_store), &iter, EVENT_TEXT_COLUMN, &text, -1);
	}
#endif
	return text;
}
