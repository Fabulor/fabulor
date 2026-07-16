/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "channel-list.h"

#include "gtk-compat.h"

#if GTK_MAJOR_VERSION >= 4
#include "gtk4-list-models.h"
#else
#include "custom-list.h"
#include "gtkutil.h"
#endif

struct _FabulorChannelList
{
	GtkWidget *view;
	FabulorChannelListActivateFunc activate_func;
	gpointer callback_data;
#if GTK_MAJOR_VERSION >= 4
	FabulorGtk4FlatModelStack *models;
	GHashTable *rows;
	GtkColumnViewColumn *columns[3];
#else
	CustomList *store;
#endif
};

void
fabulor_channel_list_record_free (FabulorChannelListRecord *record)
{
	if (!record)
		return;
	g_free (record->channel);
	g_free (record->topic);
	g_free (record);
}

#if GTK_MAJOR_VERSION >= 4

typedef struct _FabulorChannelRow FabulorChannelRow;
typedef struct _FabulorChannelRowClass FabulorChannelRowClass;

struct _FabulorChannelRow
{
	GObject parent_instance;
	gpointer identity;
	gchar *channel;
	guint users;
	gchar *topic;
	gchar *collation_key;
};

struct _FabulorChannelRowClass
{
	GObjectClass parent_class;
};

enum
{
	PROP_CHANNEL_0,
	PROP_CHANNEL_NAME,
	PROP_CHANNEL_USERS,
	PROP_CHANNEL_TOPIC,
	PROP_CHANNEL_COLLATION_KEY,
	N_CHANNEL_PROPERTIES
};

static GParamSpec *channel_properties[N_CHANNEL_PROPERTIES];

#define FABULOR_TYPE_CHANNEL_ROW (fabulor_channel_row_get_type ())
#define FABULOR_CHANNEL_ROW(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), FABULOR_TYPE_CHANNEL_ROW, \
	 FabulorChannelRow))

G_DEFINE_TYPE (FabulorChannelRow, fabulor_channel_row, G_TYPE_OBJECT)

static void
fabulor_channel_row_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	FabulorChannelRow *row = FABULOR_CHANNEL_ROW (object);

	switch (property_id)
	{
	case PROP_CHANNEL_NAME: g_value_set_string (value, row->channel); break;
	case PROP_CHANNEL_USERS: g_value_set_uint (value, row->users); break;
	case PROP_CHANNEL_TOPIC: g_value_set_string (value, row->topic); break;
	case PROP_CHANNEL_COLLATION_KEY:
		g_value_set_string (value, row->collation_key);
		break;
	default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
fabulor_channel_row_finalize (GObject *object)
{
	FabulorChannelRow *row = FABULOR_CHANNEL_ROW (object);
	g_free (row->channel);
	g_free (row->topic);
	g_free (row->collation_key);
	G_OBJECT_CLASS (fabulor_channel_row_parent_class)->finalize (object);
}

static void
fabulor_channel_row_class_init (FabulorChannelRowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = fabulor_channel_row_get_property;
	object_class->finalize = fabulor_channel_row_finalize;
	channel_properties[PROP_CHANNEL_NAME] = g_param_spec_string (
		"channel", "Channel", "Channel name", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	channel_properties[PROP_CHANNEL_USERS] = g_param_spec_uint (
		"users", "Users", "Channel user count", 0, G_MAXUINT, 0,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	channel_properties[PROP_CHANNEL_TOPIC] = g_param_spec_string (
		"topic", "Topic", "Channel topic", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	channel_properties[PROP_CHANNEL_COLLATION_KEY] = g_param_spec_string (
		"collation-key", "Collation key", "Channel collation key", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties (object_class, N_CHANNEL_PROPERTIES,
		channel_properties);
}

static void
fabulor_channel_row_init (FabulorChannelRow *row)
{
	(void) row;
}

static FabulorChannelRow *
channel_row_new (const FabulorChannelListSnapshot *snapshot)
{
	FabulorChannelRow *row = g_object_new (FABULOR_TYPE_CHANNEL_ROW, NULL);
	row->identity = snapshot->identity;
	row->channel = g_strdup (snapshot->channel);
	row->users = snapshot->users;
	row->topic = g_strdup (snapshot->topic ? snapshot->topic : "");
	row->collation_key = g_strdup (snapshot->collation_key ?
		snapshot->collation_key : snapshot->channel);
	return row;
}

typedef enum
{
	CHANNEL_FIELD_NAME,
	CHANNEL_FIELD_USERS,
	CHANNEL_FIELD_TOPIC
} ChannelField;

static GQuark
channel_list_item_quark (void)
{
	return g_quark_from_static_string ("fabulor-channel-list-item");
}

static void
channel_factory_setup (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	GtkWidget *label = gtk_label_new (NULL);
	ChannelField field = (ChannelField) GPOINTER_TO_UINT (user_data);
	(void) factory;
	gtk_label_set_xalign (GTK_LABEL (label),
		field == CHANNEL_FIELD_USERS ? 1.0f : 0.0f);
	gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
	gtk_widget_set_hexpand (label, TRUE);
	gtk_list_item_set_child (item, label);
}

static void
channel_factory_bind (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	FabulorChannelRow *row = FABULOR_CHANNEL_ROW (
		gtk_list_item_get_item (item));
	GtkWidget *label = gtk_list_item_get_child (item);
	ChannelField field = (ChannelField) GPOINTER_TO_UINT (user_data);
	(void) factory;
	if (field == CHANNEL_FIELD_USERS)
	{
		gchar users[32];
		g_snprintf (users, sizeof (users), "%u", row->users);
		gtk_label_set_text (GTK_LABEL (label), users);
	}
	else
		gtk_label_set_text (GTK_LABEL (label),
			field == CHANNEL_FIELD_NAME ? row->channel : row->topic);
	g_object_set_qdata (G_OBJECT (label), channel_list_item_quark (), item);
}

static void
channel_factory_unbind (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	(void) factory;
	(void) user_data;
	g_object_set_qdata (G_OBJECT (gtk_list_item_get_child (item)),
		channel_list_item_quark (), NULL);
}

static GtkColumnViewColumn *
channel_column_new (const gchar *title, ChannelField field,
	const gchar *property, gboolean numeric, gboolean expand,
	gboolean resizable, gint width)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	GtkColumnViewColumn *column;
	GtkExpression *expression;
	GtkSorter *sorter;

	g_signal_connect (factory, "setup", G_CALLBACK (channel_factory_setup),
		GUINT_TO_POINTER (field));
	g_signal_connect (factory, "bind", G_CALLBACK (channel_factory_bind),
		GUINT_TO_POINTER (field));
	g_signal_connect (factory, "unbind", G_CALLBACK (channel_factory_unbind),
		NULL);
	column = gtk_column_view_column_new (title, factory);
	gtk_column_view_column_set_expand (column, expand);
	gtk_column_view_column_set_resizable (column, resizable);
	if (width > 0)
		gtk_column_view_column_set_fixed_width (column, width);
	expression = gtk_property_expression_new (FABULOR_TYPE_CHANNEL_ROW, NULL,
		property);
	if (numeric)
		sorter = GTK_SORTER (gtk_numeric_sorter_new (expression));
	else
		sorter = GTK_SORTER (gtk_string_sorter_new (expression));
	gtk_column_view_column_set_sorter (column, sorter);
	g_object_unref (sorter);
	return column;
}

static gboolean
channel_list_position_at_point (FabulorChannelList *list, gdouble x,
	gdouble y, guint *position)
{
	GtkWidget *picked = gtk_widget_pick (list->view, x, y, GTK_PICK_DEFAULT);
	while (picked && picked != list->view)
	{
		GtkListItem *item = g_object_get_qdata (G_OBJECT (picked),
			channel_list_item_quark ());
		if (item)
		{
			*position = gtk_list_item_get_position (item);
			return TRUE;
		}
		picked = gtk_widget_get_parent (picked);
	}
	return FALSE;
}

static void
channel_activate_cb (GtkColumnView *view, guint position, gpointer user_data)
{
	FabulorChannelList *list = user_data;
	(void) view;
	(void) position;
	if (list->activate_func)
		list->activate_func (list->callback_data);
}

#else

static void
channel_activate_cb (GtkTreeView *view, GtkTreePath *path,
	GtkTreeViewColumn *column, gpointer user_data)
{
	FabulorChannelList *list = user_data;
	(void) view;
	(void) path;
	(void) column;
	if (list->activate_func)
		list->activate_func (list->callback_data);
}

static void
channel_gtk3_add_column (GtkWidget *view, gint text_column, gint size,
	const gchar *title, gboolean right_justified)
{
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
	GtkTreeViewColumn *column;
	if (right_justified)
		g_object_set (renderer, "xalign", 1.0f, NULL);
	g_object_set (renderer, "ypad", 0, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view), -1,
		title, renderer, "text", text_column, NULL);
	gtk_cell_renderer_text_set_fixed_height_from_font (
		GTK_CELL_RENDERER_TEXT (renderer), 1);
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (view), text_column);
	gtk_tree_view_column_set_sort_column_id (column, text_column);
	gtk_tree_view_column_set_resizable (column, TRUE);
	if (text_column == CUSTOM_LIST_COL_NAME)
	{
		gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_fixed_width (column, size);
	}
	else if (text_column == CUSTOM_LIST_COL_USERS)
	{
		gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
		gtk_tree_view_column_set_resizable (column, FALSE);
	}
}

#endif

FabulorChannelList *
fabulor_channel_list_new (FabulorChannelListActivateFunc activate_func,
	gpointer user_data)
{
	FabulorChannelList *list = g_new0 (FabulorChannelList, 1);
	list->activate_func = activate_func;
	list->callback_data = user_data;
#if GTK_MAJOR_VERSION >= 4
	{
		GtkExpression *expression = gtk_property_expression_new (
			FABULOR_TYPE_CHANNEL_ROW, NULL, "collation-key");
		GtkSorter *sorter = GTK_SORTER (gtk_string_sorter_new (expression));
		list->models = fabulor_gtk4_flat_model_stack_new (
			FABULOR_TYPE_CHANNEL_ROW, sorter, FABULOR_GTK4_SELECTION_MULTIPLE);
		g_object_unref (sorter);
	}
	if (!list->models)
	{
		g_free (list);
		return NULL;
	}
	list->rows = g_hash_table_new (g_direct_hash, g_direct_equal);
#else
	list->store = custom_list_new ();
#endif
	return list;
}

void
fabulor_channel_list_free (FabulorChannelList *list)
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
fabulor_channel_list_create_view (FabulorChannelList *list, GtkBox *parent,
	const gchar *channel_title, const gchar *users_title,
	const gchar *topic_title, gint channel_width, gint users_width,
	gint topic_width)
{
	g_return_val_if_fail (list && GTK_IS_BOX (parent) && !list->view, NULL);
#if GTK_MAJOR_VERSION >= 4
	{
		GtkWidget *scroller = gtk_scrolled_window_new ();
		GtkSelectionModel *selection =
			fabulor_gtk4_flat_model_stack_get_selection (list->models);
		list->view = gtk_column_view_new (GTK_SELECTION_MODEL (
			g_object_ref (selection)));
		list->columns[0] = channel_column_new (channel_title,
			CHANNEL_FIELD_NAME, "collation-key", FALSE, FALSE, TRUE,
			channel_width > 0 ? channel_width : 96);
		list->columns[1] = channel_column_new (users_title,
			CHANNEL_FIELD_USERS, "users", TRUE, FALSE, FALSE,
			users_width > 0 ? users_width : 50);
		list->columns[2] = channel_column_new (topic_title,
			CHANNEL_FIELD_TOPIC, "topic", FALSE, TRUE, TRUE, topic_width);
		for (guint i = 0; i < G_N_ELEMENTS (list->columns); i++)
			gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view),
				list->columns[i]);
		gtk_column_view_sort_by_column (GTK_COLUMN_VIEW (list->view),
			list->columns[0], GTK_SORT_ASCENDING);
		gtk_sort_list_model_set_sorter (
			fabulor_gtk4_flat_model_stack_get_sorted (list->models),
			gtk_column_view_get_sorter (GTK_COLUMN_VIEW (list->view)));
		gtk_sort_list_model_set_incremental (
			fabulor_gtk4_flat_model_stack_get_sorted (list->models), TRUE);
		gtk_column_view_set_show_row_separators (GTK_COLUMN_VIEW (list->view),
			TRUE);
		g_signal_connect (list->view, "activate",
			G_CALLBACK (channel_activate_cb), list);
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
		gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (
			gtk_widget_get_parent (list->view)), GTK_SHADOW_IN);
		channel_gtk3_add_column (list->view, CUSTOM_LIST_COL_NAME, 96,
			channel_title, FALSE);
		channel_gtk3_add_column (list->view, CUSTOM_LIST_COL_USERS, 50,
			users_title, TRUE);
		channel_gtk3_add_column (list->view, CUSTOM_LIST_COL_TOPIC, 50,
			topic_title, FALSE);
		if (channel_width > 0)
			gtk_tree_view_column_set_fixed_width (gtk_tree_view_get_column (
				GTK_TREE_VIEW (list->view), 0), channel_width);
		if (users_width > 0)
		{
			GtkTreeViewColumn *column = gtk_tree_view_get_column (
				GTK_TREE_VIEW (list->view), 1);
			gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
			gtk_tree_view_column_set_fixed_width (column, users_width);
			gtk_tree_view_column_set_resizable (column, FALSE);
		}
		if (topic_width > 0)
		{
			GtkTreeViewColumn *column = gtk_tree_view_get_column (
				GTK_TREE_VIEW (list->view), 2);
			gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
			gtk_tree_view_column_set_fixed_width (column, topic_width);
		}
		gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (list->view),
			GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (list->view));
		gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
		g_signal_connect (list->view, "row-activated",
			G_CALLBACK (channel_activate_cb), list);
		gtk_widget_show (list->view);
	}
#endif
	return list->view;
}

GtkWidget *
fabulor_channel_list_get_view (FabulorChannelList *list)
{
	return list ? list->view : NULL;
}

gboolean
fabulor_channel_list_append (FabulorChannelList *list,
	const FabulorChannelListSnapshot *snapshot)
{
	g_return_val_if_fail (list && snapshot && snapshot->identity &&
		snapshot->channel, FALSE);
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorChannelRow *row;
		if (g_hash_table_contains (list->rows, snapshot->identity))
			return FALSE;
		row = channel_row_new (snapshot);
		fabulor_gtk4_flat_model_stack_append (list->models, row);
		g_hash_table_insert (list->rows, snapshot->identity, row);
		g_object_unref (row);
	}
#else
	custom_list_append (list->store, snapshot->identity);
#endif
	return TRUE;
}

void
fabulor_channel_list_clear (FabulorChannelList *list)
{
	g_return_if_fail (list != NULL);
#if GTK_MAJOR_VERSION >= 4
	g_hash_table_remove_all (list->rows);
	fabulor_gtk4_flat_model_stack_clear (list->models);
#else
	custom_list_clear (list->store);
#endif
}

void
fabulor_channel_list_resort (FabulorChannelList *list)
{
	g_return_if_fail (list != NULL);
#if GTK_MAJOR_VERSION >= 4
	if (list->view)
		gtk_sorter_changed (gtk_column_view_get_sorter (
			GTK_COLUMN_VIEW (list->view)), GTK_SORTER_CHANGE_DIFFERENT);
#else
	custom_list_resort (list->store);
#endif
}

guint
fabulor_channel_list_get_n_rows (FabulorChannelList *list)
{
	g_return_val_if_fail (list != NULL, 0);
#if GTK_MAJOR_VERSION >= 4
	return g_list_model_get_n_items (G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_sorted (list->models)));
#else
	return list->store->num_rows;
#endif
}

gboolean
fabulor_channel_list_set_selected (FabulorChannelList *list, guint position,
	gboolean selected)
{
	g_return_val_if_fail (list != NULL, FALSE);
	if (position >= fabulor_channel_list_get_n_rows (list))
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
fabulor_channel_list_select_at_point (FabulorChannelList *list, gdouble x,
	gdouble y)
{
	g_return_val_if_fail (list && list->view, FALSE);
#if GTK_MAJOR_VERSION >= 4
	{
		guint position;
		GtkSelectionModel *selection =
			fabulor_gtk4_flat_model_stack_get_selection (list->models);
		if (!channel_list_position_at_point (list, x, y, &position))
			return FALSE;
		if (!gtk_selection_model_is_selected (selection, position))
			gtk_selection_model_select_item (selection, position, TRUE);
		return TRUE;
	}
#else
	{
		GtkTreePath *path;
		GtkTreeSelection *selection;
		if (!gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (list->view),
			(gint) x, (gint) y, &path, NULL, NULL, NULL))
			return FALSE;
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (list->view));
		if (!gtk_tree_selection_path_is_selected (selection, path))
		{
			gtk_tree_selection_unselect_all (selection);
			gtk_tree_selection_select_path (selection, path);
		}
		gtk_tree_path_free (path);
		return TRUE;
	}
#endif
}

GPtrArray *
fabulor_channel_list_dup_selected_text (FabulorChannelList *list,
	FabulorChannelListTextField field)
{
	GPtrArray *values = g_ptr_array_new_with_free_func (g_free);
	guint count;
	guint i;
	g_return_val_if_fail (list != NULL, values);
	count = fabulor_channel_list_get_n_rows (list);
#if GTK_MAJOR_VERSION >= 4
	for (i = 0; i < count; i++)
	{
		FabulorChannelRow *row;
		if (!gtk_selection_model_is_selected (
			fabulor_gtk4_flat_model_stack_get_selection (list->models), i))
			continue;
		row = g_list_model_get_item (G_LIST_MODEL (
			fabulor_gtk4_flat_model_stack_get_sorted (list->models)), i);
		g_ptr_array_add (values, g_strdup (
			field == FABULOR_CHANNEL_LIST_CHANNEL ? row->channel : row->topic));
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
			gchar *value;
			if (!gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store), &iter,
				NULL, (gint) i) ||
				!gtk_tree_selection_iter_is_selected (selection, &iter))
				continue;
			gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
				field == FABULOR_CHANNEL_LIST_CHANNEL ? CUSTOM_LIST_COL_NAME :
				CUSTOM_LIST_COL_TOPIC, &value, -1);
			g_ptr_array_add (values, value);
		}
	}
#endif
	return values;
}

gchar *
fabulor_channel_list_dup_first_selected_channel (FabulorChannelList *list)
{
	GPtrArray *values = fabulor_channel_list_dup_selected_text (list,
		FABULOR_CHANNEL_LIST_CHANNEL);
	gchar *value = NULL;
	if (values->len)
	{
		value = g_ptr_array_index (values, 0);
		g_ptr_array_index (values, 0) = NULL;
	}
	g_ptr_array_unref (values);
	return value;
}

GPtrArray *
fabulor_channel_list_dup_all (FabulorChannelList *list)
{
	GPtrArray *records = g_ptr_array_new_with_free_func (
		(GDestroyNotify) fabulor_channel_list_record_free);
	guint count;
	guint i;
	g_return_val_if_fail (list != NULL, records);
	count = fabulor_channel_list_get_n_rows (list);
#if GTK_MAJOR_VERSION >= 4
	for (i = 0; i < count; i++)
	{
		FabulorChannelRow *row = g_list_model_get_item (G_LIST_MODEL (
			fabulor_gtk4_flat_model_stack_get_sorted (list->models)), i);
		FabulorChannelListRecord *record = g_new0 (
			FabulorChannelListRecord, 1);
		record->channel = g_strdup (row->channel);
		record->users = row->users;
		record->topic = g_strdup (row->topic);
		g_ptr_array_add (records, record);
		g_object_unref (row);
	}
#else
	for (i = 0; i < count; i++)
	{
		GtkTreeIter iter;
		FabulorChannelListRecord *record;
		if (!gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store), &iter,
			NULL, (gint) i))
			continue;
		record = g_new0 (FabulorChannelListRecord, 1);
		gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
			CUSTOM_LIST_COL_NAME, &record->channel,
			CUSTOM_LIST_COL_USERS, &record->users,
			CUSTOM_LIST_COL_TOPIC, &record->topic, -1);
		g_ptr_array_add (records, record);
	}
#endif
	return records;
}

void
fabulor_channel_list_get_column_widths (FabulorChannelList *list,
	gint *channel_width, gint *users_width, gint *topic_width)
{
	g_return_if_fail (list && list->view);
#if GTK_MAJOR_VERSION >= 4
	if (channel_width)
		*channel_width = gtk_column_view_column_get_fixed_width (list->columns[0]);
	if (users_width)
		*users_width = gtk_column_view_column_get_fixed_width (list->columns[1]);
	if (topic_width)
		*topic_width = gtk_column_view_column_get_fixed_width (list->columns[2]);
#else
	if (channel_width)
		*channel_width = gtk_tree_view_column_get_width (gtk_tree_view_get_column (
			GTK_TREE_VIEW (list->view), 0));
	if (users_width)
		*users_width = gtk_tree_view_column_get_width (gtk_tree_view_get_column (
			GTK_TREE_VIEW (list->view), 1));
	if (topic_width)
		*topic_width = gtk_tree_view_column_get_width (gtk_tree_view_get_column (
			GTK_TREE_VIEW (list->view), 2));
#endif
}
