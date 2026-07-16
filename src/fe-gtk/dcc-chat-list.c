/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "dcc-chat-list.h"

#include "gtk-compat.h"

#if GTK_MAJOR_VERSION >= 4
#include "gtk4-list-models.h"
#else
#include "gtkutil.h"
#include "theme/theme-gtk.h"
#endif

struct _FabulorDccChatList
{
	GtkWidget *view;
	FabulorDccChatSelectionFunc selection_func;
	FabulorDccChatActivateFunc activate_func;
	gpointer callback_data;
#if GTK_MAJOR_VERSION >= 4
	FabulorGtk4FlatModelStack *models;
	GHashTable *rows;
#else
	GtkListStore *store;
#endif
};

static void
dcc_chat_selection_changed (FabulorDccChatList *list)
{
	if (list->selection_func)
		list->selection_func (fabulor_dcc_chat_list_get_n_selected (list),
			list->callback_data);
}

#if GTK_MAJOR_VERSION >= 4

typedef struct _FabulorDccChatRow FabulorDccChatRow;
typedef struct _FabulorDccChatRowClass FabulorDccChatRowClass;

struct _FabulorDccChatRow
{
	GObject parent_instance;
	gpointer identity;
	gchar *status;
	gchar *nick;
	gchar *received;
	gchar *sent;
	gchar *start_time;
	gboolean has_color;
	GdkRGBA color;
};

struct _FabulorDccChatRowClass
{
	GObjectClass parent_class;
};

enum
{
	PROP_CHAT_0,
	PROP_CHAT_STATUS,
	PROP_CHAT_NICK,
	PROP_CHAT_RECEIVED,
	PROP_CHAT_SENT,
	PROP_CHAT_START_TIME,
	N_CHAT_PROPERTIES
};

static GParamSpec *chat_properties[N_CHAT_PROPERTIES];

#define FABULOR_TYPE_DCC_CHAT_ROW (fabulor_dcc_chat_row_get_type ())
#define FABULOR_DCC_CHAT_ROW(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), FABULOR_TYPE_DCC_CHAT_ROW, \
	 FabulorDccChatRow))

G_DEFINE_TYPE (FabulorDccChatRow, fabulor_dcc_chat_row, G_TYPE_OBJECT)

static void
fabulor_dcc_chat_row_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	FabulorDccChatRow *row = FABULOR_DCC_CHAT_ROW (object);
	const gchar *text = NULL;

	switch (property_id)
	{
	case PROP_CHAT_STATUS: text = row->status; break;
	case PROP_CHAT_NICK: text = row->nick; break;
	case PROP_CHAT_RECEIVED: text = row->received; break;
	case PROP_CHAT_SENT: text = row->sent; break;
	case PROP_CHAT_START_TIME: text = row->start_time; break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		return;
	}
	g_value_set_string (value, text);
}

static void
fabulor_dcc_chat_row_finalize (GObject *object)
{
	FabulorDccChatRow *row = FABULOR_DCC_CHAT_ROW (object);

	g_free (row->status);
	g_free (row->nick);
	g_free (row->received);
	g_free (row->sent);
	g_free (row->start_time);
	G_OBJECT_CLASS (fabulor_dcc_chat_row_parent_class)->finalize (object);
}

static void
fabulor_dcc_chat_row_class_init (FabulorDccChatRowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	const gchar *names[] = { "status", "nick", "received", "sent",
		"start-time" };
	guint i;

	object_class->get_property = fabulor_dcc_chat_row_get_property;
	object_class->finalize = fabulor_dcc_chat_row_finalize;
	for (i = 0; i < G_N_ELEMENTS (names); i++)
		chat_properties[i + 1] = g_param_spec_string (names[i], names[i],
			names[i], NULL, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties (object_class, N_CHAT_PROPERTIES,
		chat_properties);
}

static void
fabulor_dcc_chat_row_init (FabulorDccChatRow *row)
{
	(void) row;
}

static void
replace_string (gchar **target, const gchar *value)
{
	g_free (*target);
	*target = g_strdup (value ? value : "");
}

static void
dcc_chat_row_assign (FabulorDccChatRow *row,
	const FabulorDccChatSnapshot *snapshot, gboolean notify)
{
	guint i;

	row->identity = snapshot->identity;
	replace_string (&row->status, snapshot->status);
	replace_string (&row->nick, snapshot->nick);
	replace_string (&row->received, snapshot->received);
	replace_string (&row->sent, snapshot->sent);
	replace_string (&row->start_time, snapshot->start_time);
	row->has_color = snapshot->has_color;
	row->color = snapshot->color;
	if (notify)
		for (i = 1; i < N_CHAT_PROPERTIES; i++)
			g_object_notify_by_pspec (G_OBJECT (row), chat_properties[i]);
}

static FabulorDccChatRow *
dcc_chat_row_new (const FabulorDccChatSnapshot *snapshot)
{
	FabulorDccChatRow *row = g_object_new (FABULOR_TYPE_DCC_CHAT_ROW, NULL);
	dcc_chat_row_assign (row, snapshot, FALSE);
	return row;
}

typedef struct
{
	FabulorDccChatRow *row;
	GtkLabel *label;
	GBinding *text_binding;
	gulong notify_handler;
	const gchar *property;
} DccChatCellBinding;

static void
dcc_chat_cell_apply_color (DccChatCellBinding *binding)
{
	PangoAttrList *attributes = NULL;

	if (binding->row && binding->row->has_color)
	{
		PangoAttribute *foreground;
		attributes = pango_attr_list_new ();
		foreground = pango_attr_foreground_new (
			(guint16) (binding->row->color.red * 65535.0),
			(guint16) (binding->row->color.green * 65535.0),
			(guint16) (binding->row->color.blue * 65535.0));
		pango_attr_list_insert (attributes, foreground);
	}
	gtk_label_set_attributes (binding->label, attributes);
	if (attributes)
		pango_attr_list_unref (attributes);
}

static void
dcc_chat_cell_changed (GObject *object, GParamSpec *pspec, gpointer user_data)
{
	(void) object;
	(void) pspec;
	dcc_chat_cell_apply_color (user_data);
}

static void
dcc_chat_cell_clear (DccChatCellBinding *binding)
{
	if (binding->row && binding->notify_handler)
		g_signal_handler_disconnect (binding->row, binding->notify_handler);
	binding->notify_handler = 0;
	g_clear_object (&binding->text_binding);
	g_clear_object (&binding->row);
}

static void
dcc_chat_cell_free (gpointer data)
{
	DccChatCellBinding *binding = data;
	dcc_chat_cell_clear (binding);
	g_free (binding);
}

static void
dcc_chat_factory_setup (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	DccChatCellBinding *binding = g_new0 (DccChatCellBinding, 1);
	GtkWidget *label = gtk_label_new (NULL);

	(void) factory;
	binding->property = user_data;
	binding->label = GTK_LABEL (label);
	gtk_label_set_xalign (binding->label,
		g_str_equal (binding->property, "received") ||
		g_str_equal (binding->property, "sent") ? 1.0f : 0.0f);
	gtk_label_set_ellipsize (binding->label, PANGO_ELLIPSIZE_END);
	gtk_widget_set_hexpand (label, TRUE);
	gtk_list_item_set_child (item, label);
	g_object_set_data_full (G_OBJECT (item), "fabulor-dcc-chat-cell",
		binding, dcc_chat_cell_free);
}

static void
dcc_chat_factory_bind (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	DccChatCellBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-dcc-chat-cell");

	(void) factory;
	(void) user_data;
	dcc_chat_cell_clear (binding);
	binding->row = g_object_ref (FABULOR_DCC_CHAT_ROW (
		gtk_list_item_get_item (item)));
	binding->text_binding = g_object_bind_property (binding->row,
		binding->property, binding->label, "label", G_BINDING_SYNC_CREATE);
	binding->notify_handler = g_signal_connect (binding->row, "notify",
		G_CALLBACK (dcc_chat_cell_changed), binding);
	dcc_chat_cell_apply_color (binding);
}

static void
dcc_chat_factory_unbind (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	DccChatCellBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-dcc-chat-cell");
	(void) factory;
	(void) user_data;
	dcc_chat_cell_clear (binding);
}

static GtkColumnViewColumn *
dcc_chat_column_new (const gchar *title, const gchar *property,
	gboolean expand)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	GtkColumnViewColumn *column;

	g_signal_connect (factory, "setup", G_CALLBACK (dcc_chat_factory_setup),
		(gpointer) property);
	g_signal_connect (factory, "bind", G_CALLBACK (dcc_chat_factory_bind), NULL);
	g_signal_connect (factory, "unbind", G_CALLBACK (dcc_chat_factory_unbind),
		NULL);
	column = gtk_column_view_column_new (title, factory);
	gtk_column_view_column_set_expand (column, expand);
	gtk_column_view_column_set_resizable (column, TRUE);
	return column;
}

static void
dcc_chat_selection_changed_cb (GtkSelectionModel *selection, guint position,
	guint n_items, gpointer user_data)
{
	(void) selection;
	(void) position;
	(void) n_items;
	dcc_chat_selection_changed (user_data);
}

static void
dcc_chat_activate_cb (GtkColumnView *view, guint position, gpointer user_data)
{
	FabulorDccChatList *list = user_data;
	FabulorDccChatRow *row = g_list_model_get_item (G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_sorted (list->models)), position);
	(void) view;
	if (row && list->activate_func)
		list->activate_func (row->identity, list->callback_data);
	g_clear_object (&row);
}

#else

enum
{
	CHAT_COLUMN_STATUS,
	CHAT_COLUMN_NICK,
	CHAT_COLUMN_RECEIVED,
	CHAT_COLUMN_SENT,
	CHAT_COLUMN_START_TIME,
	CHAT_COLUMN_IDENTITY,
	CHAT_COLUMN_COLOR,
	N_CHAT_COLUMNS
};

static gboolean
dcc_chat_find_iter (FabulorDccChatList *list, gpointer identity,
	GtkTreeIter *iter)
{
	gpointer row_identity;
	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list->store), iter))
		return FALSE;
	do
	{
		gtk_tree_model_get (GTK_TREE_MODEL (list->store), iter,
			CHAT_COLUMN_IDENTITY, &row_identity, -1);
		if (row_identity == identity)
			return TRUE;
	} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list->store), iter));
	return FALSE;
}

static void
dcc_chat_gtk3_set (FabulorDccChatList *list, GtkTreeIter *iter,
	const FabulorDccChatSnapshot *snapshot)
{
	const GdkRGBA *color = snapshot->has_color ? &snapshot->color : NULL;
	gtk_list_store_set (list->store, iter,
		CHAT_COLUMN_STATUS, snapshot->status,
		CHAT_COLUMN_NICK, snapshot->nick,
		CHAT_COLUMN_RECEIVED, snapshot->received,
		CHAT_COLUMN_SENT, snapshot->sent,
		CHAT_COLUMN_START_TIME, snapshot->start_time,
		CHAT_COLUMN_IDENTITY, snapshot->identity,
		CHAT_COLUMN_COLOR, color, -1);
}

static void
dcc_chat_gtk3_add_column (GtkWidget *view, gint text_column,
	const gchar *title, gboolean right_justified)
{
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
	if (right_justified)
		g_object_set (renderer, "xalign", 1.0f, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view), -1,
		title, renderer, "text", text_column, THEME_GTK_FOREGROUND_PROPERTY,
		CHAT_COLUMN_COLOR, NULL);
	gtk_cell_renderer_text_set_fixed_height_from_font (
		GTK_CELL_RENDERER_TEXT (renderer), 1);
}

static void
dcc_chat_selection_changed_cb (GtkTreeSelection *selection,
	gpointer user_data)
{
	(void) selection;
	dcc_chat_selection_changed (user_data);
}

static void
dcc_chat_activate_cb (GtkTreeView *view, GtkTreePath *path,
	GtkTreeViewColumn *column, gpointer user_data)
{
	FabulorDccChatList *list = user_data;
	GtkTreeIter iter;
	gpointer identity = NULL;
	(void) view;
	(void) column;
	if (gtk_tree_model_get_iter (GTK_TREE_MODEL (list->store), &iter, path))
		gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
			CHAT_COLUMN_IDENTITY, &identity, -1);
	if (identity && list->activate_func)
		list->activate_func (identity, list->callback_data);
}

#endif

FabulorDccChatList *
fabulor_dcc_chat_list_new (FabulorDccChatSelectionFunc selection_func,
	FabulorDccChatActivateFunc activate_func, gpointer user_data)
{
	FabulorDccChatList *list = g_new0 (FabulorDccChatList, 1);
	list->selection_func = selection_func;
	list->activate_func = activate_func;
	list->callback_data = user_data;
#if GTK_MAJOR_VERSION >= 4
	list->models = fabulor_gtk4_flat_model_stack_new (FABULOR_TYPE_DCC_CHAT_ROW,
		NULL, FABULOR_GTK4_SELECTION_MULTIPLE);
	if (!list->models)
	{
		g_free (list);
		return NULL;
	}
	list->rows = g_hash_table_new (g_direct_hash, g_direct_equal);
	g_signal_connect (fabulor_gtk4_flat_model_stack_get_selection (list->models),
		"selection-changed", G_CALLBACK (dcc_chat_selection_changed_cb), list);
#else
	list->store = gtk_list_store_new (N_CHAT_COLUMNS, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_POINTER, THEME_GTK_COLOR_TYPE);
#endif
	return list;
}

void
fabulor_dcc_chat_list_free (FabulorDccChatList *list)
{
	if (!list)
		return;
#if GTK_MAJOR_VERSION >= 4
	g_hash_table_unref (list->rows);
	fabulor_gtk4_flat_model_stack_free (list->models);
#else
	g_clear_object (&list->store);
#endif
	g_free (list);
}

GtkWidget *
fabulor_dcc_chat_list_create_view (FabulorDccChatList *list, GtkBox *parent,
	const gchar *status_title, const gchar *nick_title,
	const gchar *received_title, const gchar *sent_title,
	const gchar *start_time_title)
{
	g_return_val_if_fail (list != NULL && GTK_IS_BOX (parent), NULL);
	g_return_val_if_fail (list->view == NULL, NULL);
#if GTK_MAJOR_VERSION >= 4
	{
		GtkWidget *scroller = gtk_scrolled_window_new ();
		GtkSelectionModel *selection =
			fabulor_gtk4_flat_model_stack_get_selection (list->models);
		const gchar *titles[] = { status_title, nick_title, received_title,
			sent_title, start_time_title };
		const gchar *properties[] = { "status", "nick", "received", "sent",
			"start-time" };
		guint i;

		list->view = gtk_column_view_new (GTK_SELECTION_MODEL (
			g_object_ref (selection)));
		gtk_column_view_set_show_row_separators (GTK_COLUMN_VIEW (list->view),
			TRUE);
		for (i = 0; i < G_N_ELEMENTS (properties); i++)
		{
			GtkColumnViewColumn *column = dcc_chat_column_new (titles[i],
				properties[i], i == 1);
			gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view), column);
			g_object_unref (column);
		}
		g_signal_connect (list->view, "activate",
			G_CALLBACK (dcc_chat_activate_cb), list);
		fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroller),
			list->view);
		gtk_widget_set_hexpand (scroller, TRUE);
		gtk_widget_set_vexpand (scroller, TRUE);
		fabulor_gtk_box_append (parent, scroller, TRUE, TRUE, 0);
	}
#else
	{
		GtkTreeSelection *selection;
		list->view = gtkutil_treeview_new (parent,
			GTK_TREE_MODEL (g_object_ref (list->store)), NULL, -1);
		dcc_chat_gtk3_add_column (list->view, CHAT_COLUMN_STATUS,
			status_title, FALSE);
		dcc_chat_gtk3_add_column (list->view, CHAT_COLUMN_NICK,
			nick_title, FALSE);
		dcc_chat_gtk3_add_column (list->view, CHAT_COLUMN_RECEIVED,
			received_title, TRUE);
		dcc_chat_gtk3_add_column (list->view, CHAT_COLUMN_SENT,
			sent_title, TRUE);
		dcc_chat_gtk3_add_column (list->view, CHAT_COLUMN_START_TIME,
			start_time_title, FALSE);
		gtk_tree_view_column_set_expand (gtk_tree_view_get_column (
			GTK_TREE_VIEW (list->view), 1), TRUE);
		gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (list->view),
			GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (list->view));
		gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
		g_signal_connect (selection, "changed",
			G_CALLBACK (dcc_chat_selection_changed_cb), list);
		g_signal_connect (list->view, "row-activated",
			G_CALLBACK (dcc_chat_activate_cb), list);
		gtk_widget_show (list->view);
	}
#endif
	return list->view;
}

gboolean
fabulor_dcc_chat_list_append (FabulorDccChatList *list,
	const FabulorDccChatSnapshot *snapshot, gboolean prepend)
{
	g_return_val_if_fail (list && snapshot && snapshot->identity, FALSE);
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorDccChatRow *row;
		GListStore *store;
		if (g_hash_table_contains (list->rows, snapshot->identity))
			return FALSE;
		row = dcc_chat_row_new (snapshot);
		store = fabulor_gtk4_flat_model_stack_get_store (list->models);
		if (prepend)
			g_list_store_insert (store, 0, row);
		else
			g_list_store_append (store, row);
		g_hash_table_insert (list->rows, snapshot->identity, row);
		g_object_unref (row);
	}
#else
	{
		GtkTreeIter iter;
		if (dcc_chat_find_iter (list, snapshot->identity, &iter))
			return FALSE;
		if (prepend)
			gtk_list_store_prepend (list->store, &iter);
		else
			gtk_list_store_append (list->store, &iter);
		dcc_chat_gtk3_set (list, &iter, snapshot);
	}
#endif
	return TRUE;
}

gboolean
fabulor_dcc_chat_list_update (FabulorDccChatList *list,
	const FabulorDccChatSnapshot *snapshot)
{
	g_return_val_if_fail (list && snapshot && snapshot->identity, FALSE);
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorDccChatRow *row = g_hash_table_lookup (list->rows,
			snapshot->identity);
		if (!row)
			return FALSE;
		dcc_chat_row_assign (row, snapshot, TRUE);
	}
#else
	{
		GtkTreeIter iter;
		if (!dcc_chat_find_iter (list, snapshot->identity, &iter))
			return FALSE;
		dcc_chat_gtk3_set (list, &iter, snapshot);
	}
#endif
	return TRUE;
}

gboolean
fabulor_dcc_chat_list_remove (FabulorDccChatList *list, gpointer identity)
{
	g_return_val_if_fail (list && identity, FALSE);
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorDccChatRow *row = g_hash_table_lookup (list->rows, identity);
		if (!row)
			return FALSE;
		g_object_ref (row);
		g_hash_table_remove (list->rows, identity);
		fabulor_gtk4_flat_model_stack_remove (list->models, row);
		g_object_unref (row);
	}
#else
	{
		GtkTreeIter iter;
		if (!dcc_chat_find_iter (list, identity, &iter))
			return FALSE;
		gtk_list_store_remove (list->store, &iter);
	}
#endif
	return TRUE;
}

void
fabulor_dcc_chat_list_clear (FabulorDccChatList *list)
{
	g_return_if_fail (list != NULL);
#if GTK_MAJOR_VERSION >= 4
	g_hash_table_remove_all (list->rows);
	fabulor_gtk4_flat_model_stack_clear (list->models);
#else
	gtk_list_store_clear (list->store);
#endif
	dcc_chat_selection_changed (list);
}

guint
fabulor_dcc_chat_list_get_n_rows (FabulorDccChatList *list)
{
	g_return_val_if_fail (list != NULL, 0);
#if GTK_MAJOR_VERSION >= 4
	return g_list_model_get_n_items (G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_sorted (list->models)));
#else
	return (guint) gtk_tree_model_iter_n_children (
		GTK_TREE_MODEL (list->store), NULL);
#endif
}

guint
fabulor_dcc_chat_list_get_n_selected (FabulorDccChatList *list)
{
	g_return_val_if_fail (list != NULL, 0);
#if GTK_MAJOR_VERSION >= 4
	{
		GtkBitset *selected = gtk_selection_model_get_selection (
			fabulor_gtk4_flat_model_stack_get_selection (list->models));
		guint count = (guint) gtk_bitset_get_size (selected);
		gtk_bitset_unref (selected);
		return count;
	}
#else
	if (list->view)
	{
		GList *rows = gtk_tree_selection_get_selected_rows (
			gtk_tree_view_get_selection (GTK_TREE_VIEW (list->view)), NULL);
		guint count = g_list_length (rows);
		g_list_free_full (rows, (GDestroyNotify) gtk_tree_path_free);
		return count;
	}
	return 0;
#endif
}

gboolean
fabulor_dcc_chat_list_set_selected (FabulorDccChatList *list, guint position,
	gboolean selected)
{
	g_return_val_if_fail (list != NULL, FALSE);
	if (position >= fabulor_dcc_chat_list_get_n_rows (list))
		return FALSE;
#if GTK_MAJOR_VERSION >= 4
	if (selected)
		return gtk_selection_model_select_item (
			fabulor_gtk4_flat_model_stack_get_selection (list->models), position,
			FALSE);
	return gtk_selection_model_unselect_item (
		fabulor_gtk4_flat_model_stack_get_selection (list->models), position);
#else
	if (list->view)
	{
		GtkTreeIter iter;
		GtkTreeSelection *selection = gtk_tree_view_get_selection (
			GTK_TREE_VIEW (list->view));
		if (!gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store), &iter,
			NULL, (gint) position))
			return FALSE;
		if (selected)
			gtk_tree_selection_select_iter (selection, &iter);
		else
			gtk_tree_selection_unselect_iter (selection, &iter);
		return TRUE;
	}
	return FALSE;
#endif
}

gboolean
fabulor_dcc_chat_list_select_first (FabulorDccChatList *list)
{
	return fabulor_dcc_chat_list_set_selected (list, 0, TRUE);
}

GPtrArray *
fabulor_dcc_chat_list_dup_selected (FabulorDccChatList *list)
{
	GPtrArray *identities = g_ptr_array_new ();
	guint count;
	guint i;
	g_return_val_if_fail (list != NULL, identities);
	count = fabulor_dcc_chat_list_get_n_rows (list);
#if GTK_MAJOR_VERSION >= 4
	for (i = 0; i < count; i++)
	{
		FabulorDccChatRow *row;
		if (!gtk_selection_model_is_selected (
			fabulor_gtk4_flat_model_stack_get_selection (list->models), i))
			continue;
		row = g_list_model_get_item (G_LIST_MODEL (
			fabulor_gtk4_flat_model_stack_get_sorted (list->models)), i);
		g_ptr_array_add (identities, row->identity);
		g_object_unref (row);
	}
#else
	if (list->view)
	{
		GtkTreeSelection *selection = gtk_tree_view_get_selection (
			GTK_TREE_VIEW (list->view));
		for (i = 0; i < count; i++)
		{
			GtkTreeIter iter;
			gpointer identity;
			if (!gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store), &iter,
				NULL, (gint) i) ||
				!gtk_tree_selection_iter_is_selected (selection, &iter))
				continue;
			gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
				CHAT_COLUMN_IDENTITY, &identity, -1);
			g_ptr_array_add (identities, identity);
		}
	}
#endif
	return identities;
}
