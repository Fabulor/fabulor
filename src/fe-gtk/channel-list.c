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

#include "gtk4-list-models.h"

struct _FabulorChannelList
{
	GtkWidget *view;
	FabulorChannelListActivateFunc activate_func;
	gpointer callback_data;
	FabulorGtk4FlatModelStack *models;
	GHashTable *rows;
	GtkColumnViewColumn *columns[3];
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

static gint
channel_row_compare (gconstpointer item1, gconstpointer item2,
	gpointer user_data)
{
	const FabulorChannelRow *row1 = item1;
	const FabulorChannelRow *row2 = item2;
	ChannelField field = (ChannelField) GPOINTER_TO_UINT (user_data);
	gint result;

	if (field == CHANNEL_FIELD_USERS)
		result = row1->users < row2->users ? -1 :
			(row1->users > row2->users ? 1 : 0);
	else if (field == CHANNEL_FIELD_TOPIC)
		result = g_strcmp0 (row1->topic, row2->topic);
	else
		result = g_strcmp0 (row1->collation_key, row2->collation_key);

	return result;
}

static GtkColumnViewColumn *
channel_column_new (const gchar *title, ChannelField field,
	gboolean expand, gboolean resizable, gint width)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	GtkColumnViewColumn *column;
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
	sorter = GTK_SORTER (gtk_custom_sorter_new (channel_row_compare,
		GUINT_TO_POINTER (field), NULL));
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


FabulorChannelList *
fabulor_channel_list_new (FabulorChannelListActivateFunc activate_func,
	gpointer user_data)
{
	FabulorChannelList *list = g_new0 (FabulorChannelList, 1);
	list->activate_func = activate_func;
	list->callback_data = user_data;
	{
		GtkSorter *sorter = GTK_SORTER (gtk_custom_sorter_new (
			channel_row_compare, GUINT_TO_POINTER (CHANNEL_FIELD_NAME), NULL));
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
	return list;
}

void
fabulor_channel_list_free (FabulorChannelList *list)
{
	if (!list)
		return;
	g_hash_table_unref (list->rows);
	fabulor_gtk4_flat_model_stack_free (list->models);
	g_free (list);
}

GtkWidget *
fabulor_channel_list_create_view (FabulorChannelList *list, GtkBox *parent,
	const gchar *channel_title, const gchar *users_title,
	const gchar *topic_title, gint channel_width, gint users_width,
	gint topic_width)
{
	g_return_val_if_fail (list && GTK_IS_BOX (parent) && !list->view, NULL);
	{
		GtkWidget *scroller = gtk_scrolled_window_new ();
		GtkSelectionModel *selection =
			fabulor_gtk4_flat_model_stack_get_selection (list->models);
		list->view = gtk_column_view_new (GTK_SELECTION_MODEL (
			g_object_ref (selection)));
		list->columns[0] = channel_column_new (channel_title,
			CHANNEL_FIELD_NAME, FALSE, TRUE,
			channel_width > 0 ? channel_width : 96);
		list->columns[1] = channel_column_new (users_title,
			CHANNEL_FIELD_USERS, FALSE, FALSE,
			users_width > 0 ? users_width : 50);
		list->columns[2] = channel_column_new (topic_title,
			CHANNEL_FIELD_TOPIC, TRUE, TRUE, topic_width);
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
	{
		FabulorChannelRow *row;
		if (g_hash_table_contains (list->rows, snapshot->identity))
			return FALSE;
		row = channel_row_new (snapshot);
		fabulor_gtk4_flat_model_stack_append (list->models, row);
		g_hash_table_insert (list->rows, snapshot->identity, row);
		g_object_unref (row);
	}
	return TRUE;
}

void
fabulor_channel_list_clear (FabulorChannelList *list)
{
	g_return_if_fail (list != NULL);
	g_hash_table_remove_all (list->rows);
	fabulor_gtk4_flat_model_stack_clear (list->models);
}

void
fabulor_channel_list_resort (FabulorChannelList *list)
{
	g_return_if_fail (list != NULL);
	if (list->view)
		gtk_sorter_changed (gtk_column_view_get_sorter (
			GTK_COLUMN_VIEW (list->view)), GTK_SORTER_CHANGE_DIFFERENT);
}

guint
fabulor_channel_list_get_n_rows (FabulorChannelList *list)
{
	g_return_val_if_fail (list != NULL, 0);
	return g_list_model_get_n_items (G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_sorted (list->models)));
}

gboolean
fabulor_channel_list_set_selected (FabulorChannelList *list, guint position,
	gboolean selected)
{
	g_return_val_if_fail (list != NULL, FALSE);
	if (position >= fabulor_channel_list_get_n_rows (list))
		return FALSE;
	if (selected)
		return gtk_selection_model_select_item (
			fabulor_gtk4_flat_model_stack_get_selection (list->models), position,
			FALSE);
	return gtk_selection_model_unselect_item (
		fabulor_gtk4_flat_model_stack_get_selection (list->models), position);
}

gboolean
fabulor_channel_list_select_at_point (FabulorChannelList *list, gdouble x,
	gdouble y)
{
	g_return_val_if_fail (list && list->view, FALSE);
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
	return records;
}

void
fabulor_channel_list_get_column_widths (FabulorChannelList *list,
	gint *channel_width, gint *users_width, gint *topic_width)
{
	g_return_if_fail (list && list->view);
	if (channel_width)
		*channel_width = gtk_column_view_column_get_fixed_width (list->columns[0]);
	if (users_width)
		*users_width = gtk_column_view_column_get_fixed_width (list->columns[1]);
	if (topic_width)
		*topic_width = gtk_column_view_column_get_fixed_width (list->columns[2]);
}
