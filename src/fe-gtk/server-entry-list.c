/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "server-entry-list.h"

#include "gtk-compat.h"

#if GTK_MAJOR_VERSION >= 4
#include "gtk4-list-models.h"
#endif

struct _FabulorServerEntryList
{
	GtkWidget *view;
	gboolean has_secondary;
	gpointer pending_edit_identity;
	FabulorServerEntrySelectionFunc selection_func;
	FabulorServerEntryEditFunc edit_func;
	gpointer callback_data;
#if GTK_MAJOR_VERSION >= 4
	FabulorGtk4FlatModelStack *models;
	gulong selection_id;
	GHashTable *primary_bindings;
#else
	GtkListStore *store;
#endif
};

#if GTK_MAJOR_VERSION >= 4

typedef struct _FabulorServerEntryRow FabulorServerEntryRow;
typedef struct _FabulorServerEntryRowClass FabulorServerEntryRowClass;

struct _FabulorServerEntryRow
{
	GObject parent_instance;
	gpointer identity;
	gchar *primary;
	gchar *secondary;
};

struct _FabulorServerEntryRowClass
{
	GObjectClass parent_class;
};

enum
{
	PROP_SERVER_ENTRY_ROW_0,
	PROP_SERVER_ENTRY_ROW_PRIMARY,
	PROP_SERVER_ENTRY_ROW_SECONDARY,
	N_SERVER_ENTRY_ROW_PROPERTIES
};

static GParamSpec *server_entry_row_properties[N_SERVER_ENTRY_ROW_PROPERTIES];

#define FABULOR_TYPE_SERVER_ENTRY_ROW (fabulor_server_entry_row_get_type ())
#define FABULOR_SERVER_ENTRY_ROW(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), FABULOR_TYPE_SERVER_ENTRY_ROW, \
	 FabulorServerEntryRow))

G_DEFINE_TYPE (FabulorServerEntryRow, fabulor_server_entry_row, G_TYPE_OBJECT)

static void
fabulor_server_entry_row_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	FabulorServerEntryRow *row = FABULOR_SERVER_ENTRY_ROW (object);
	if (property_id == PROP_SERVER_ENTRY_ROW_PRIMARY)
		g_value_set_string (value, row->primary);
	else if (property_id == PROP_SERVER_ENTRY_ROW_SECONDARY)
		g_value_set_string (value, row->secondary);
	else
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
fabulor_server_entry_row_finalize (GObject *object)
{
	FabulorServerEntryRow *row = FABULOR_SERVER_ENTRY_ROW (object);
	g_free (row->primary);
	g_free (row->secondary);
	G_OBJECT_CLASS (fabulor_server_entry_row_parent_class)->finalize (object);
}

static void
fabulor_server_entry_row_class_init (FabulorServerEntryRowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = fabulor_server_entry_row_get_property;
	object_class->finalize = fabulor_server_entry_row_finalize;
	server_entry_row_properties[PROP_SERVER_ENTRY_ROW_PRIMARY] =
		g_param_spec_string ("primary", "Primary", "Primary entry text", NULL,
			G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	server_entry_row_properties[PROP_SERVER_ENTRY_ROW_SECONDARY] =
		g_param_spec_string ("secondary", "Secondary", "Secondary entry text",
			NULL, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties (object_class,
		N_SERVER_ENTRY_ROW_PROPERTIES, server_entry_row_properties);
}

static void
fabulor_server_entry_row_init (FabulorServerEntryRow *row)
{
	(void) row;
}

static FabulorServerEntryRow *
server_entry_row_new (gpointer identity, const gchar *primary,
	const gchar *secondary)
{
	FabulorServerEntryRow *row = g_object_new (FABULOR_TYPE_SERVER_ENTRY_ROW,
		NULL);
	row->identity = identity;
	row->primary = g_strdup (primary ? primary : "");
	row->secondary = g_strdup (secondary ? secondary : "");
	return row;
}

static GListStore *
server_entry_store (FabulorServerEntryList *list)
{
	return fabulor_gtk4_flat_model_stack_get_store (list->models);
}

static GtkSingleSelection *
server_entry_selection (FabulorServerEntryList *list)
{
	return GTK_SINGLE_SELECTION (
		fabulor_gtk4_flat_model_stack_get_selection (list->models));
}

static FabulorServerEntryRow *
server_entry_row_at (FabulorServerEntryList *list, guint position)
{
	return g_list_model_get_item (G_LIST_MODEL (server_entry_store (list)),
		position);
}

static void
server_entry_selection_changed (GtkSingleSelection *selection,
	GParamSpec *pspec, gpointer user_data)
{
	FabulorServerEntryList *list = user_data;
	FabulorServerEntryRow *row = FABULOR_SERVER_ENTRY_ROW (
		gtk_single_selection_get_selected_item (selection));
	(void) pspec;
	if (list->selection_func)
		list->selection_func (row ? row->identity : NULL, list->callback_data);
}

typedef struct
{
	FabulorServerEntryList *owner;
	FabulorServerEntryField field;
	GtkEditableLabel *label;
	FabulorServerEntryRow *row;
	gulong notify_id;
	gboolean editing;
	gboolean blocked;
} ServerEntryBinding;

typedef struct
{
	FabulorServerEntryList *owner;
	FabulorServerEntryField field;
} ServerEntryFactoryData;

static const gchar *
server_entry_row_text (FabulorServerEntryRow *row,
	FabulorServerEntryField field)
{
	return field == FABULOR_SERVER_ENTRY_PRIMARY ? row->primary :
		row->secondary;
}

static void
server_entry_binding_refresh (ServerEntryBinding *binding)
{
	if (!binding->row)
		return;
	binding->blocked = TRUE;
	gtk_editable_set_text (GTK_EDITABLE (binding->label),
		server_entry_row_text (binding->row, binding->field));
	binding->blocked = FALSE;
}

static void
server_entry_row_changed (GObject *object, GParamSpec *pspec,
	gpointer user_data)
{
	ServerEntryBinding *binding = user_data;
	GParamSpec *expected = binding->field == FABULOR_SERVER_ENTRY_PRIMARY ?
		server_entry_row_properties[PROP_SERVER_ENTRY_ROW_PRIMARY] :
		server_entry_row_properties[PROP_SERVER_ENTRY_ROW_SECONDARY];
	(void) object;
	if (!binding->blocked && pspec == expected)
		server_entry_binding_refresh (binding);
}

static void
server_entry_editing_changed (GtkEditableLabel *label, GParamSpec *pspec,
	gpointer user_data)
{
	ServerEntryBinding *binding = user_data;
	gboolean editing = gtk_editable_label_get_editing (label);
	(void) pspec;
	if (!binding->blocked && binding->row && binding->editing && !editing)
	{
		const gchar *text = gtk_editable_get_text (GTK_EDITABLE (label));
		if (binding->owner->edit_func && binding->owner->edit_func (
			binding->row->identity, binding->field, text,
			binding->owner->callback_data))
			fabulor_server_entry_list_update (binding->owner,
				binding->row->identity, binding->field, text);
		else
			server_entry_binding_refresh (binding);
	}
	binding->editing = editing;
}

static void
server_entry_binding_free (gpointer data)
{
	ServerEntryBinding *binding = data;
	if (binding->row)
	{
		if (binding->field == FABULOR_SERVER_ENTRY_PRIMARY)
			g_hash_table_remove (binding->owner->primary_bindings,
				binding->row->identity);
		if (binding->notify_id)
			g_signal_handler_disconnect (binding->row, binding->notify_id);
	}
	g_clear_object (&binding->row);
	g_free (binding);
}

static void
server_entry_factory_setup (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	ServerEntryFactoryData *data = user_data;
	ServerEntryBinding *binding = g_new0 (ServerEntryBinding, 1);
	GtkWidget *label = gtk_editable_label_new (NULL);
	(void) factory;
	binding->owner = data->owner;
	binding->field = data->field;
	binding->label = GTK_EDITABLE_LABEL (label);
	gtk_widget_set_hexpand (label, TRUE);
	gtk_widget_set_halign (label, GTK_ALIGN_FILL);
	g_signal_connect (label, "notify::editing",
		G_CALLBACK (server_entry_editing_changed), binding);
	gtk_list_item_set_child (item, label);
	g_object_set_data_full (G_OBJECT (item), "fabulor-server-entry-binding",
		binding, server_entry_binding_free);
}

static void
server_entry_factory_bind (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	ServerEntryBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-server-entry-binding");
	(void) factory;
	(void) user_data;
	binding->row = g_object_ref (FABULOR_SERVER_ENTRY_ROW (
		gtk_list_item_get_item (item)));
	binding->notify_id = g_signal_connect (binding->row,
		binding->field == FABULOR_SERVER_ENTRY_PRIMARY ? "notify::primary" :
		"notify::secondary", G_CALLBACK (server_entry_row_changed), binding);
	if (binding->field == FABULOR_SERVER_ENTRY_PRIMARY)
		g_hash_table_insert (binding->owner->primary_bindings,
			binding->row->identity, binding);
	server_entry_binding_refresh (binding);
	binding->editing = gtk_editable_label_get_editing (binding->label);
	if (binding->field == FABULOR_SERVER_ENTRY_PRIMARY &&
		binding->row->identity == binding->owner->pending_edit_identity)
	{
		binding->owner->pending_edit_identity = NULL;
		gtk_editable_label_start_editing (binding->label);
	}
}

static void
server_entry_factory_unbind (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	ServerEntryBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-server-entry-binding");
	(void) factory;
	(void) user_data;
	if (binding->row)
	{
		if (binding->field == FABULOR_SERVER_ENTRY_PRIMARY)
			g_hash_table_remove (binding->owner->primary_bindings,
				binding->row->identity);
		if (binding->notify_id)
			g_signal_handler_disconnect (binding->row, binding->notify_id);
	}
	binding->notify_id = 0;
	g_clear_object (&binding->row);
}

static GtkColumnViewColumn *
server_entry_column_new (FabulorServerEntryList *list, const gchar *title,
	FabulorServerEntryField field)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	GtkColumnViewColumn *column;
	ServerEntryFactoryData *data = g_new (ServerEntryFactoryData, 1);
	data->owner = list;
	data->field = field;
	g_object_set_data_full (G_OBJECT (factory),
		"fabulor-server-entry-factory-data", data, g_free);
	g_signal_connect (factory, "setup",
		G_CALLBACK (server_entry_factory_setup), data);
	g_signal_connect (factory, "bind", G_CALLBACK (server_entry_factory_bind),
		data);
	g_signal_connect (factory, "unbind",
		G_CALLBACK (server_entry_factory_unbind), data);
	column = gtk_column_view_column_new (title, factory);
	gtk_column_view_column_set_expand (column, TRUE);
	return column;
}

#else

enum
{
	SERVER_ENTRY_COLUMN_PRIMARY,
	SERVER_ENTRY_COLUMN_SECONDARY,
	SERVER_ENTRY_COLUMN_EDITABLE,
	SERVER_ENTRY_COLUMN_IDENTITY,
	N_SERVER_ENTRY_COLUMNS
};

typedef struct
{
	FabulorServerEntryList *owner;
	FabulorServerEntryField field;
} ServerEntryRendererData;

static void
server_entry_selection_changed (GtkTreeSelection *selection,
	gpointer user_data)
{
	FabulorServerEntryList *list = user_data;
	GtkTreeIter iter;
	gpointer identity = NULL;
	if (gtk_tree_selection_get_selected (selection, NULL, &iter))
		gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
			SERVER_ENTRY_COLUMN_IDENTITY, &identity, -1);
	if (list->selection_func)
		list->selection_func (identity, list->callback_data);
}

static void
server_entry_renderer_data_free (gpointer data, GClosure *closure)
{
	(void) closure;
	g_free (data);
}

static void
server_entry_edited (GtkCellRendererText *renderer, gchar *path_text,
	gchar *new_text, gpointer user_data)
{
	ServerEntryRendererData *data = user_data;
	GtkTreePath *path = gtk_tree_path_new_from_string (path_text);
	GtkTreeIter iter;
	gpointer identity = NULL;
	(void) renderer;
	if (path && gtk_tree_model_get_iter (GTK_TREE_MODEL (data->owner->store),
		&iter, path))
		gtk_tree_model_get (GTK_TREE_MODEL (data->owner->store), &iter,
			SERVER_ENTRY_COLUMN_IDENTITY, &identity, -1);
	if (identity && data->owner->edit_func && data->owner->edit_func (identity,
		data->field, new_text, data->owner->callback_data))
		fabulor_server_entry_list_update (data->owner, identity, data->field,
			new_text);
	if (path)
		gtk_tree_path_free (path);
}

static void
server_entry_append_column (FabulorServerEntryList *list,
	const gchar *title, FabulorServerEntryField field)
{
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
	ServerEntryRendererData *data = g_new (ServerEntryRendererData, 1);
	data->owner = list;
	data->field = field;
	g_signal_connect_data (renderer, "edited", G_CALLBACK (server_entry_edited),
		data, server_entry_renderer_data_free, 0);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (list->view),
		-1, title, renderer, "text", field == FABULOR_SERVER_ENTRY_PRIMARY ?
		SERVER_ENTRY_COLUMN_PRIMARY : SERVER_ENTRY_COLUMN_SECONDARY,
		"editable", SERVER_ENTRY_COLUMN_EDITABLE, NULL);
	gtk_tree_view_column_set_expand (gtk_tree_view_get_column (
		GTK_TREE_VIEW (list->view), field), TRUE);
}

#endif

static gboolean
server_entry_find (FabulorServerEntryList *list, gpointer identity,
	guint *position)
{
	guint count = fabulor_server_entry_list_get_n_rows (list);
	guint i;
	for (i = 0; i < count; i++)
		if (fabulor_server_entry_list_get_identity_at (list, i) == identity)
		{
			if (position)
				*position = i;
			return TRUE;
		}
	return FALSE;
}

FabulorServerEntryList *
fabulor_server_entry_list_new (gboolean has_secondary,
	FabulorServerEntrySelectionFunc selection_func,
	FabulorServerEntryEditFunc edit_func, gpointer user_data)
{
	FabulorServerEntryList *list = g_new0 (FabulorServerEntryList, 1);
	list->has_secondary = has_secondary;
	list->selection_func = selection_func;
	list->edit_func = edit_func;
	list->callback_data = user_data;
#if GTK_MAJOR_VERSION >= 4
	list->models = fabulor_gtk4_flat_model_stack_new (
		FABULOR_TYPE_SERVER_ENTRY_ROW, NULL, FABULOR_GTK4_SELECTION_SINGLE);
	list->primary_bindings = g_hash_table_new (g_direct_hash, g_direct_equal);
	list->selection_id = g_signal_connect (server_entry_selection (list),
		"notify::selected", G_CALLBACK (server_entry_selection_changed), list);
#else
	list->store = gtk_list_store_new (N_SERVER_ENTRY_COLUMNS, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_POINTER);
#endif
	return list;
}

void
fabulor_server_entry_list_free (FabulorServerEntryList *list)
{
	if (!list)
		return;
#if GTK_MAJOR_VERSION >= 4
	if (list->selection_id)
		g_signal_handler_disconnect (server_entry_selection (list),
			list->selection_id);
	g_hash_table_unref (list->primary_bindings);
	fabulor_gtk4_flat_model_stack_free (list->models);
#else
	g_clear_object (&list->store);
#endif
	g_free (list);
}

GtkWidget *
fabulor_server_entry_list_create_view (FabulorServerEntryList *list,
	GtkScrolledWindow *scroller, const gchar *primary_title,
	const gchar *secondary_title, gboolean headers_visible)
{
	g_return_val_if_fail (list && GTK_IS_SCROLLED_WINDOW (scroller) &&
		!list->view, NULL);
#if GTK_MAJOR_VERSION >= 4
	{
		GtkColumnViewColumn *column;
		(void) headers_visible;
		list->view = gtk_column_view_new (GTK_SELECTION_MODEL (g_object_ref (
			server_entry_selection (list))));
		column = server_entry_column_new (list, primary_title,
			FABULOR_SERVER_ENTRY_PRIMARY);
		gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view), column);
		g_object_unref (column);
		if (list->has_secondary)
		{
			column = server_entry_column_new (list, secondary_title,
				FABULOR_SERVER_ENTRY_SECONDARY);
			gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view), column);
			g_object_unref (column);
		}
		gtk_column_view_set_show_column_separators (GTK_COLUMN_VIEW (list->view),
			list->has_secondary);
	}
#else
	list->view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list->store));
	server_entry_append_column (list, primary_title,
		FABULOR_SERVER_ENTRY_PRIMARY);
	if (list->has_secondary)
		server_entry_append_column (list, secondary_title,
			FABULOR_SERVER_ENTRY_SECONDARY);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (list->view),
		headers_visible);
	g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (list->view)),
		"changed", G_CALLBACK (server_entry_selection_changed), list);
#endif
	fabulor_gtk_scrolled_window_set_child (scroller, list->view);
#if GTK_MAJOR_VERSION < 4
	gtk_widget_show (list->view);
#endif
	return list->view;
}

gboolean
fabulor_server_entry_list_append (FabulorServerEntryList *list,
	gpointer identity, const gchar *primary, const gchar *secondary)
{
	g_return_val_if_fail (list && identity, FALSE);
	if (server_entry_find (list, identity, NULL))
		return FALSE;
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorServerEntryRow *row = server_entry_row_new (identity, primary,
			secondary);
		g_list_store_append (server_entry_store (list), row);
		g_object_unref (row);
	}
#else
	{
		GtkTreeIter iter;
		gtk_list_store_append (list->store, &iter);
		gtk_list_store_set (list->store, &iter,
			SERVER_ENTRY_COLUMN_PRIMARY, primary ? primary : "",
			SERVER_ENTRY_COLUMN_SECONDARY, secondary ? secondary : "",
			SERVER_ENTRY_COLUMN_EDITABLE, TRUE,
			SERVER_ENTRY_COLUMN_IDENTITY, identity, -1);
	}
#endif
	return TRUE;
}

void
fabulor_server_entry_list_clear (FabulorServerEntryList *list)
{
	g_return_if_fail (list != NULL);
#if GTK_MAJOR_VERSION >= 4
	g_list_store_remove_all (server_entry_store (list));
#else
	gtk_list_store_clear (list->store);
#endif
}

guint
fabulor_server_entry_list_get_n_rows (FabulorServerEntryList *list)
{
	g_return_val_if_fail (list != NULL, 0);
#if GTK_MAJOR_VERSION >= 4
	return g_list_model_get_n_items (G_LIST_MODEL (server_entry_store (list)));
#else
	return (guint) gtk_tree_model_iter_n_children (
		GTK_TREE_MODEL (list->store), NULL);
#endif
}

gpointer
fabulor_server_entry_list_get_identity_at (FabulorServerEntryList *list,
	guint position)
{
	g_return_val_if_fail (list != NULL, NULL);
	if (position >= fabulor_server_entry_list_get_n_rows (list))
		return NULL;
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorServerEntryRow *row = server_entry_row_at (list, position);
		gpointer identity = row->identity;
		g_object_unref (row);
		return identity;
	}
#else
	{
		GtkTreeIter iter;
		gpointer identity = NULL;
		gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store), &iter,
			NULL, (gint) position);
		gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
			SERVER_ENTRY_COLUMN_IDENTITY, &identity, -1);
		return identity;
	}
#endif
}

gchar *
fabulor_server_entry_list_dup_text (FabulorServerEntryList *list,
	gpointer identity, FabulorServerEntryField field)
{
	guint position;
	gchar *text = NULL;
	g_return_val_if_fail (list != NULL, NULL);
	if (!server_entry_find (list, identity, &position) ||
		(field == FABULOR_SERVER_ENTRY_SECONDARY && !list->has_secondary))
		return NULL;
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorServerEntryRow *row = server_entry_row_at (list, position);
		text = g_strdup (server_entry_row_text (row, field));
		g_object_unref (row);
	}
#else
	{
		GtkTreeIter iter;
		gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store), &iter,
			NULL, (gint) position);
		gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
			field == FABULOR_SERVER_ENTRY_PRIMARY ? SERVER_ENTRY_COLUMN_PRIMARY :
			SERVER_ENTRY_COLUMN_SECONDARY, &text, -1);
	}
#endif
	return text;
}

gpointer
fabulor_server_entry_list_get_selected (FabulorServerEntryList *list)
{
	g_return_val_if_fail (list != NULL, NULL);
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorServerEntryRow *row = FABULOR_SERVER_ENTRY_ROW (
			gtk_single_selection_get_selected_item (server_entry_selection (list)));
		return row ? row->identity : NULL;
	}
#else
	if (list->view)
	{
		GtkTreeIter iter;
		gpointer identity = NULL;
		if (gtk_tree_selection_get_selected (gtk_tree_view_get_selection (
			GTK_TREE_VIEW (list->view)), NULL, &iter))
			gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
				SERVER_ENTRY_COLUMN_IDENTITY, &identity, -1);
		return identity;
	}
	return NULL;
#endif
}

gboolean
fabulor_server_entry_list_select (FabulorServerEntryList *list,
	gpointer identity)
{
	guint position;
	g_return_val_if_fail (list != NULL, FALSE);
	if (!server_entry_find (list, identity, &position))
		return FALSE;
#if GTK_MAJOR_VERSION >= 4
	gtk_single_selection_set_selected (server_entry_selection (list), position);
	if (list->view)
		gtk_column_view_scroll_to (GTK_COLUMN_VIEW (list->view), position, NULL,
			GTK_LIST_SCROLL_FOCUS, NULL);
#else
	if (list->view)
	{
		GtkTreePath *path = gtk_tree_path_new_from_indices ((gint) position, -1);
		gtk_tree_view_set_cursor (GTK_TREE_VIEW (list->view), path, NULL, FALSE);
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (list->view), path, NULL,
			TRUE, 0.5, 0.5);
		gtk_tree_path_free (path);
	}
#endif
	return TRUE;
}

gboolean
fabulor_server_entry_list_remove (FabulorServerEntryList *list,
	gpointer identity)
{
	guint position;
	g_return_val_if_fail (list != NULL, FALSE);
	if (!server_entry_find (list, identity, &position))
		return FALSE;
#if GTK_MAJOR_VERSION >= 4
	g_list_store_remove (server_entry_store (list), position);
#else
	{
		GtkTreeIter iter;
		gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store), &iter,
			NULL, (gint) position);
		gtk_list_store_remove (list->store, &iter);
	}
#endif
	return TRUE;
}

gboolean
fabulor_server_entry_list_move (FabulorServerEntryList *list,
	gpointer identity, gint delta)
{
	guint position;
	gint target;
	g_return_val_if_fail (list != NULL, FALSE);
	if (!server_entry_find (list, identity, &position) ||
		(delta != -1 && delta != 1))
		return FALSE;
	target = (gint) position + delta;
	if (target < 0 || target >= (gint) fabulor_server_entry_list_get_n_rows (
		list))
		return FALSE;
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorServerEntryRow *row = server_entry_row_at (list, position);
		g_list_store_remove (server_entry_store (list), position);
		g_list_store_insert (server_entry_store (list), (guint) target, row);
		g_object_unref (row);
	}
#else
	{
		GtkTreeIter iter;
		GtkTreeIter other;
		gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store), &iter,
			NULL, (gint) position);
		gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store), &other,
			NULL, target);
		gtk_list_store_swap (list->store, &iter, &other);
	}
#endif
	fabulor_server_entry_list_select (list, identity);
	return TRUE;
}

gboolean
fabulor_server_entry_list_update (FabulorServerEntryList *list,
	gpointer identity, FabulorServerEntryField field, const gchar *text)
{
	guint position;
	g_return_val_if_fail (list != NULL, FALSE);
	if (!server_entry_find (list, identity, &position) ||
		(field == FABULOR_SERVER_ENTRY_SECONDARY && !list->has_secondary))
		return FALSE;
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorServerEntryRow *row = server_entry_row_at (list, position);
		gchar **value = field == FABULOR_SERVER_ENTRY_PRIMARY ? &row->primary :
			&row->secondary;
		guint property_id = field == FABULOR_SERVER_ENTRY_PRIMARY ?
			PROP_SERVER_ENTRY_ROW_PRIMARY : PROP_SERVER_ENTRY_ROW_SECONDARY;
		if (g_strcmp0 (*value, text ? text : "") != 0)
		{
			g_free (*value);
			*value = g_strdup (text ? text : "");
			g_object_notify_by_pspec (G_OBJECT (row),
				server_entry_row_properties[property_id]);
		}
		g_object_unref (row);
	}
#else
	{
		GtkTreeIter iter;
		gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store), &iter,
			NULL, (gint) position);
		gtk_list_store_set (list->store, &iter,
			field == FABULOR_SERVER_ENTRY_PRIMARY ? SERVER_ENTRY_COLUMN_PRIMARY :
			SERVER_ENTRY_COLUMN_SECONDARY, text ? text : "", -1);
	}
#endif
	return TRUE;
}

void
fabulor_server_entry_list_start_editing_selected (FabulorServerEntryList *list)
{
	gpointer identity;
	g_return_if_fail (list != NULL);
	identity = fabulor_server_entry_list_get_selected (list);
	if (!identity || !list->view)
		return;
	list->pending_edit_identity = identity;
#if GTK_MAJOR_VERSION >= 4
	{
		ServerEntryBinding *binding = g_hash_table_lookup (
			list->primary_bindings, identity);
		if (binding)
		{
			list->pending_edit_identity = NULL;
			gtk_editable_label_start_editing (binding->label);
		}
	}
#else
	{
		guint position;
		GtkTreePath *path;
		server_entry_find (list, identity, &position);
		path = gtk_tree_path_new_from_indices ((gint) position, -1);
		gtk_tree_view_set_cursor (GTK_TREE_VIEW (list->view), path,
			gtk_tree_view_get_column (GTK_TREE_VIEW (list->view), 0), TRUE);
		gtk_tree_path_free (path);
		list->pending_edit_identity = NULL;
	}
#endif
}
