/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "channel-model.h"

#if GTK_MAJOR_VERSION >= 4
#include "gtk4-list-models.h"
#endif

typedef struct _FabulorChannelRecord FabulorChannelRecord;
typedef struct _FabulorChannelRecordClass FabulorChannelRecordClass;

struct _FabulorChannelRecord
{
	GObject parent_instance;
	gpointer identity;
	FabulorChannelRecord *parent;
	GPtrArray *children;
	gchar *name;
	PangoAttrList *attributes;
	GdkPixbuf *icon;
	PangoUnderline underline;
#if GTK_MAJOR_VERSION >= 4
	GListStore *child_store;
#else
	GtkTreeRowReference *reference;
#endif
};

struct _FabulorChannelRecordClass
{
	GObjectClass parent_class;
};

struct _FabulorChannelModel
{
	GHashTable *records;
	GPtrArray *roots;
#if GTK_MAJOR_VERSION >= 4
	FabulorGtk4TreeModelStack *models;
#else
	GtkTreeStore *store;
#endif
};

enum
{
	PROP_CHANNEL_RECORD_0,
	PROP_CHANNEL_RECORD_NAME,
	PROP_CHANNEL_RECORD_ATTRIBUTES,
	PROP_CHANNEL_RECORD_ICON,
	PROP_CHANNEL_RECORD_UNDERLINE,
	N_CHANNEL_RECORD_PROPERTIES
};

static GParamSpec *channel_record_properties[N_CHANNEL_RECORD_PROPERTIES];

#define FABULOR_TYPE_CHANNEL_RECORD (fabulor_channel_record_get_type ())
#define FABULOR_CHANNEL_RECORD(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), \
	FABULOR_TYPE_CHANNEL_RECORD, FabulorChannelRecord))

G_DEFINE_TYPE (FabulorChannelRecord, fabulor_channel_record, G_TYPE_OBJECT)

static void
fabulor_channel_record_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	FabulorChannelRecord *record = FABULOR_CHANNEL_RECORD (object);

	switch (property_id)
	{
	case PROP_CHANNEL_RECORD_NAME:
		g_value_set_string (value, record->name);
		break;
	case PROP_CHANNEL_RECORD_ATTRIBUTES:
		g_value_set_boxed (value, record->attributes);
		break;
	case PROP_CHANNEL_RECORD_ICON:
		g_value_set_object (value, record->icon);
		break;
	case PROP_CHANNEL_RECORD_UNDERLINE:
		g_value_set_enum (value, record->underline);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
fabulor_channel_record_finalize (GObject *object)
{
	FabulorChannelRecord *record = FABULOR_CHANNEL_RECORD (object);

#if GTK_MAJOR_VERSION >= 4
	g_clear_object (&record->child_store);
#else
	if (record->reference)
		gtk_tree_row_reference_free (record->reference);
#endif
	g_clear_pointer (&record->children, g_ptr_array_unref);
	g_clear_pointer (&record->attributes, pango_attr_list_unref);
	g_clear_object (&record->icon);
	g_free (record->name);
	G_OBJECT_CLASS (fabulor_channel_record_parent_class)->finalize (object);
}

static void
fabulor_channel_record_class_init (FabulorChannelRecordClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = fabulor_channel_record_get_property;
	object_class->finalize = fabulor_channel_record_finalize;
	channel_record_properties[PROP_CHANNEL_RECORD_NAME] = g_param_spec_string (
		"name", "Name", "Displayed channel name", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	channel_record_properties[PROP_CHANNEL_RECORD_ATTRIBUTES] =
		g_param_spec_boxed ("attributes", "Attributes", "Text attributes",
			PANGO_TYPE_ATTR_LIST, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	channel_record_properties[PROP_CHANNEL_RECORD_ICON] = g_param_spec_object (
		"icon", "Icon", "Channel icon", GDK_TYPE_PIXBUF,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	channel_record_properties[PROP_CHANNEL_RECORD_UNDERLINE] = g_param_spec_enum (
		"underline", "Underline", "Focused-row underline", PANGO_TYPE_UNDERLINE,
		PANGO_UNDERLINE_NONE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties (object_class,
		N_CHANNEL_RECORD_PROPERTIES, channel_record_properties);
}

static void
fabulor_channel_record_init (FabulorChannelRecord *record)
{
	record->children = g_ptr_array_new ();
#if GTK_MAJOR_VERSION >= 4
	record->child_store = g_list_store_new (FABULOR_TYPE_CHANNEL_RECORD);
#endif
}

static FabulorChannelRecord *
channel_record_lookup (FabulorChannelModel *model, gpointer identity)
{
	return model && identity ? g_hash_table_lookup (model->records, identity) :
		NULL;
}

static FabulorChannelRecord *
channel_record_new (const FabulorChannelModelRow *row)
{
	FabulorChannelRecord *record = g_object_new (
		FABULOR_TYPE_CHANNEL_RECORD, NULL);

	record->identity = row->identity;
	record->name = g_strdup (row->name);
	if (row->attributes)
		record->attributes = pango_attr_list_ref (row->attributes);
	if (row->icon)
		record->icon = g_object_ref (row->icon);
	record->underline = row->underline;
	return record;
}

static void
channel_record_update (FabulorChannelRecord *record,
	const FabulorChannelModelRow *row)
{
	gboolean name_changed = g_strcmp0 (record->name, row->name) != 0;
	gboolean attributes_changed = record->attributes != row->attributes;
	gboolean icon_changed = record->icon != row->icon;
	gboolean underline_changed = record->underline != row->underline;

	if (name_changed)
	{
		g_free (record->name);
		record->name = g_strdup (row->name);
	}
	if (attributes_changed)
	{
		g_clear_pointer (&record->attributes, pango_attr_list_unref);
		if (row->attributes)
			record->attributes = pango_attr_list_ref (row->attributes);
	}
	if (icon_changed)
		g_set_object (&record->icon, row->icon);
	if (underline_changed)
		record->underline = row->underline;

	if (name_changed)
		g_object_notify_by_pspec (G_OBJECT (record),
			channel_record_properties[PROP_CHANNEL_RECORD_NAME]);
	if (attributes_changed)
		g_object_notify_by_pspec (G_OBJECT (record),
			channel_record_properties[PROP_CHANNEL_RECORD_ATTRIBUTES]);
	if (icon_changed)
		g_object_notify_by_pspec (G_OBJECT (record),
			channel_record_properties[PROP_CHANNEL_RECORD_ICON]);
	if (underline_changed)
		g_object_notify_by_pspec (G_OBJECT (record),
			channel_record_properties[PROP_CHANNEL_RECORD_UNDERLINE]);
}

static guint
channel_array_index (GPtrArray *array, FabulorChannelRecord *record)
{
	guint i;

	for (i = 0; i < array->len; i++)
		if (g_ptr_array_index (array, i) == record)
			return i;
	return G_MAXUINT;
}

static GPtrArray *
channel_record_siblings (FabulorChannelModel *model,
	FabulorChannelRecord *record)
{
	return record->parent ? record->parent->children : model->roots;
}

#if GTK_MAJOR_VERSION >= 4

static GListStore *
channel_record_store (FabulorChannelModel *model, FabulorChannelRecord *parent)
{
	return parent ? parent->child_store :
		fabulor_gtk4_tree_model_stack_get_roots (model->models);
}

static GListModel *
channel_record_create_children (gpointer item, gpointer user_data)
{
	FabulorChannelRecord *record = FABULOR_CHANNEL_RECORD (item);

	(void) user_data;
	return G_LIST_MODEL (g_object_ref (record->child_store));
}

gpointer
fabulor_channel_model_get_selected_identity (FabulorChannelModel *model)
{
	GtkTreeListRow *tree_row;
	FabulorChannelRecord *record;

	if (!model)
		return NULL;
	tree_row = gtk_single_selection_get_selected_item (
		fabulor_gtk4_tree_model_stack_get_selection (model->models));
	if (!tree_row)
		return NULL;
	record = gtk_tree_list_row_get_item (tree_row);
	return record ? record->identity : NULL;
}

gboolean
fabulor_channel_model_select_identity (FabulorChannelModel *model,
	gpointer identity)
{
	GtkTreeListModel *tree;
	guint count;
	guint i;

	if (!model)
		return FALSE;
	if (!identity)
	{
		gtk_single_selection_set_selected (
			fabulor_gtk4_tree_model_stack_get_selection (model->models),
			GTK_INVALID_LIST_POSITION);
		return TRUE;
	}
	if (!channel_record_lookup (model, identity))
		return FALSE;
	tree = fabulor_gtk4_tree_model_stack_get_tree (model->models);
	count = g_list_model_get_n_items (G_LIST_MODEL (tree));
	for (i = 0; i < count; i++)
	{
		GtkTreeListRow *tree_row = gtk_tree_list_model_get_row (tree, i);
		FabulorChannelRecord *record = tree_row ?
			gtk_tree_list_row_get_item (tree_row) : NULL;

		if (record && record->identity == identity)
		{
			gtk_single_selection_set_selected (
				fabulor_gtk4_tree_model_stack_get_selection (model->models), i);
			g_object_unref (tree_row);
			return TRUE;
		}
		if (record && record->parent == NULL && record->children->len > 0)
			gtk_tree_list_row_set_expanded (tree_row, TRUE);
		g_clear_object (&tree_row);
		count = g_list_model_get_n_items (G_LIST_MODEL (tree));
	}
	return FALSE;
}

#else

static gboolean
channel_record_get_iter (FabulorChannelRecord *record, GtkTreeIter *iter)
{
	GtkTreePath *path;
	GtkTreeModel *model;

	if (!record || !record->reference)
		return FALSE;
	path = gtk_tree_row_reference_get_path (record->reference);
	if (!path)
		return FALSE;
	model = gtk_tree_row_reference_get_model (record->reference);
	if (!gtk_tree_model_get_iter (model, iter, path))
	{
		gtk_tree_path_free (path);
		return FALSE;
	}
	gtk_tree_path_free (path);
	return TRUE;
}

static gboolean
channel_record_insert_gtk3 (FabulorChannelModel *model,
	FabulorChannelRecord *record, FabulorChannelRecord *parent, guint position)
{
	GtkTreeIter iter;
	GtkTreeIter parent_iter;
	GtkTreeIter *parent_ptr = NULL;
	GtkTreePath *path;

	if (parent)
	{
		if (!channel_record_get_iter (parent, &parent_iter))
			return FALSE;
		parent_ptr = &parent_iter;
	}
	gtk_tree_store_insert (model->store, &iter, parent_ptr, (gint) position);
	gtk_tree_store_set (model->store, &iter,
		FABULOR_CHANNEL_COLUMN_NAME, record->name,
		FABULOR_CHANNEL_COLUMN_IDENTITY, record->identity,
		FABULOR_CHANNEL_COLUMN_ATTRIBUTES, record->attributes,
		FABULOR_CHANNEL_COLUMN_ICON, record->icon,
		FABULOR_CHANNEL_COLUMN_UNDERLINE, record->underline, -1);
	path = gtk_tree_model_get_path (GTK_TREE_MODEL (model->store), &iter);
	record->reference = gtk_tree_row_reference_new (
		GTK_TREE_MODEL (model->store), path);
	gtk_tree_path_free (path);
	return record->reference != NULL;
}

#endif

FabulorChannelModel *
fabulor_channel_model_new (void)
{
	FabulorChannelModel *model = g_new0 (FabulorChannelModel, 1);

	model->records = g_hash_table_new_full (g_direct_hash, g_direct_equal,
		NULL, g_object_unref);
	model->roots = g_ptr_array_new ();
#if GTK_MAJOR_VERSION >= 4
	model->models = fabulor_gtk4_tree_model_stack_new (
		FABULOR_TYPE_CHANNEL_RECORD, FALSE, channel_record_create_children,
		NULL, NULL);
	if (!model->models)
	{
		fabulor_channel_model_free (model);
		return NULL;
	}
#else
	model->store = gtk_tree_store_new (FABULOR_CHANNEL_N_COLUMNS,
		G_TYPE_STRING, G_TYPE_POINTER, PANGO_TYPE_ATTR_LIST, GDK_TYPE_PIXBUF,
		G_TYPE_INT);
#endif
	return model;
}

void
fabulor_channel_model_free (FabulorChannelModel *model)
{
	if (!model)
		return;
	g_clear_pointer (&model->records, g_hash_table_destroy);
#if GTK_MAJOR_VERSION >= 4
	fabulor_gtk4_tree_model_stack_free (model->models);
#else
	g_clear_object (&model->store);
#endif
	g_clear_pointer (&model->roots, g_ptr_array_unref);
	g_free (model);
}

gboolean
fabulor_channel_model_insert (FabulorChannelModel *model,
	const FabulorChannelModelRow *row, gpointer parent_identity, guint position)
{
	FabulorChannelRecord *parent = channel_record_lookup (model,
		parent_identity);
	FabulorChannelRecord *record;
	GPtrArray *siblings;
#if GTK_MAJOR_VERSION >= 4
	gpointer selected_identity = fabulor_channel_model_get_selected_identity (
		model);
#endif

	g_return_val_if_fail (model != NULL, FALSE);
	g_return_val_if_fail (row != NULL && row->identity != NULL, FALSE);
	if (g_hash_table_contains (model->records, row->identity) ||
		(parent_identity && (!parent || parent->parent)))
		return FALSE;
	record = channel_record_new (row);
	record->parent = parent;
	siblings = parent ? parent->children : model->roots;
	position = MIN (position, siblings->len);
	g_ptr_array_insert (siblings, position, record);
	g_hash_table_insert (model->records, row->identity, record);
#if GTK_MAJOR_VERSION >= 4
	g_list_store_insert (channel_record_store (model, parent), position, record);
	fabulor_channel_model_select_identity (model, selected_identity);
#else
	if (!channel_record_insert_gtk3 (model, record, parent, position))
	{
		g_ptr_array_remove_index (siblings, position);
		g_hash_table_remove (model->records, row->identity);
		return FALSE;
	}
#endif
	return TRUE;
}

gboolean
fabulor_channel_model_update (FabulorChannelModel *model,
	const FabulorChannelModelRow *row)
{
	FabulorChannelRecord *record;

	g_return_val_if_fail (model != NULL, FALSE);
	g_return_val_if_fail (row != NULL && row->identity != NULL, FALSE);
	record = channel_record_lookup (model, row->identity);
	if (!record)
		return FALSE;
	channel_record_update (record, row);
#if GTK_MAJOR_VERSION < 4
	{
		GtkTreeIter iter;
		if (!channel_record_get_iter (record, &iter))
			return FALSE;
		gtk_tree_store_set (model->store, &iter,
			FABULOR_CHANNEL_COLUMN_NAME, record->name,
			FABULOR_CHANNEL_COLUMN_ATTRIBUTES, record->attributes,
			FABULOR_CHANNEL_COLUMN_ICON, record->icon,
			FABULOR_CHANNEL_COLUMN_UNDERLINE, record->underline, -1);
	}
#endif
	return TRUE;
}

gboolean
fabulor_channel_model_remove (FabulorChannelModel *model, gpointer identity)
{
	FabulorChannelRecord *record = channel_record_lookup (model, identity);
	GPtrArray *siblings;
	guint position;
#if GTK_MAJOR_VERSION >= 4
	gpointer selected_identity = fabulor_channel_model_get_selected_identity (
		model);
#endif

	if (!record || record->children->len != 0)
		return FALSE;
	siblings = channel_record_siblings (model, record);
	position = channel_array_index (siblings, record);
	if (position == G_MAXUINT)
		return FALSE;
#if GTK_MAJOR_VERSION >= 4
	g_list_store_remove (channel_record_store (model, record->parent), position);
#else
	{
		GtkTreeIter iter;
		if (!channel_record_get_iter (record, &iter))
			return FALSE;
		gtk_tree_store_remove (model->store, &iter);
	}
#endif
	g_ptr_array_remove_index (siblings, position);
	g_hash_table_remove (model->records, identity);
#if GTK_MAJOR_VERSION >= 4
	fabulor_channel_model_select_identity (model,
		selected_identity == identity ? NULL : selected_identity);
#endif
	return TRUE;
}

gboolean
fabulor_channel_model_reparent (FabulorChannelModel *model, gpointer identity,
	gpointer parent_identity, guint position)
{
	FabulorChannelRecord *record = channel_record_lookup (model, identity);
	FabulorChannelRecord *parent = channel_record_lookup (model,
		parent_identity);
	FabulorChannelRecord *old_parent;
	GPtrArray *old_siblings;
	GPtrArray *new_siblings;
	guint old_position;
#if GTK_MAJOR_VERSION >= 4
	gpointer selected_identity = fabulor_channel_model_get_selected_identity (
		model);
#endif

	if (!record || record->children->len != 0 || record == parent ||
		(parent_identity && (!parent || parent->parent)))
		return FALSE;
	old_parent = record->parent;
	old_siblings = channel_record_siblings (model, record);
	old_position = channel_array_index (old_siblings, record);
	if (old_position == G_MAXUINT)
		return FALSE;
	new_siblings = parent ? parent->children : model->roots;
	if (old_siblings == new_siblings && position > old_position)
		position--;
	position = MIN (position, new_siblings->len -
		(old_siblings == new_siblings ? 1 : 0));
#if GTK_MAJOR_VERSION >= 4
	g_list_store_remove (channel_record_store (model, old_parent), old_position);
#else
	{
		GtkTreeIter iter;
		if (!channel_record_get_iter (record, &iter))
			return FALSE;
		gtk_tree_store_remove (model->store, &iter);
		gtk_tree_row_reference_free (record->reference);
		record->reference = NULL;
	}
#endif
	g_ptr_array_remove_index (old_siblings, old_position);
	record->parent = parent;
	position = MIN (position, new_siblings->len);
	g_ptr_array_insert (new_siblings, position, record);
#if GTK_MAJOR_VERSION >= 4
	g_list_store_insert (channel_record_store (model, parent), position, record);
	fabulor_channel_model_select_identity (model, selected_identity);
#else
	if (!channel_record_insert_gtk3 (model, record, parent, position))
		return FALSE;
#endif
	return TRUE;
}

gboolean
fabulor_channel_model_move_cyclic (FabulorChannelModel *model,
	gpointer identity, gint delta)
{
	FabulorChannelRecord *record = channel_record_lookup (model, identity);
	GPtrArray *siblings;
	guint old_position;
	guint new_position;
	gint normalized;
#if GTK_MAJOR_VERSION >= 4
	gpointer selected_identity = fabulor_channel_model_get_selected_identity (
		model);
#endif

	if (!record)
		return FALSE;
	siblings = channel_record_siblings (model, record);
	if (siblings->len < 2)
		return TRUE;
	old_position = channel_array_index (siblings, record);
	if (old_position == G_MAXUINT)
		return FALSE;
	normalized = ((gint) old_position - delta) % (gint) siblings->len;
	if (normalized < 0)
		normalized += (gint) siblings->len;
	new_position = (guint) normalized;
	if (new_position == old_position)
		return TRUE;
	g_ptr_array_remove_index (siblings, old_position);
	g_ptr_array_insert (siblings, new_position, record);
#if GTK_MAJOR_VERSION >= 4
	{
		GListStore *store = channel_record_store (model, record->parent);
		g_list_store_remove (store, old_position);
		g_list_store_insert (store, new_position, record);
		fabulor_channel_model_select_identity (model, selected_identity);
	}
#else
	{
		GtkTreeIter iter;
		GtkTreeIter successor;
		FabulorChannelRecord *next = new_position + 1 < siblings->len ?
			g_ptr_array_index (siblings, new_position + 1) : NULL;
		if (!channel_record_get_iter (record, &iter))
			return FALSE;
		if (next)
		{
			if (!channel_record_get_iter (next, &successor))
				return FALSE;
			gtk_tree_store_move_before (model->store, &iter, &successor);
		}
		else
		{
			gtk_tree_store_move_before (model->store, &iter, NULL);
		}
	}
#endif
	return TRUE;
}

gboolean
fabulor_channel_model_contains (FabulorChannelModel *model, gpointer identity)
{
	return channel_record_lookup (model, identity) != NULL;
}

gpointer
fabulor_channel_model_get_parent (FabulorChannelModel *model,
	gpointer identity)
{
	FabulorChannelRecord *record = channel_record_lookup (model, identity);
	return record && record->parent ? record->parent->identity : NULL;
}

guint
fabulor_channel_model_get_root_count (FabulorChannelModel *model)
{
	return model ? model->roots->len : 0;
}

gpointer
fabulor_channel_model_get_root_at (FabulorChannelModel *model, guint position)
{
	if (!model || position >= model->roots->len)
		return NULL;
	return ((FabulorChannelRecord *) g_ptr_array_index (
		model->roots, position))->identity;
}

guint
fabulor_channel_model_get_child_count (FabulorChannelModel *model,
	gpointer parent_identity)
{
	FabulorChannelRecord *parent = channel_record_lookup (model,
		parent_identity);
	return parent ? parent->children->len : 0;
}

gpointer
fabulor_channel_model_get_child_at (FabulorChannelModel *model,
	gpointer parent_identity, guint position)
{
	FabulorChannelRecord *parent = channel_record_lookup (model,
		parent_identity);
	if (!parent || position >= parent->children->len)
		return NULL;
	return ((FabulorChannelRecord *) g_ptr_array_index (
		parent->children, position))->identity;
}

guint
fabulor_channel_model_get_flat_count (FabulorChannelModel *model)
{
	guint count = 0;
	guint i;

	if (!model)
		return 0;
	for (i = 0; i < model->roots->len; i++)
		count += 1 + ((FabulorChannelRecord *) g_ptr_array_index (
			model->roots, i))->children->len;
	return count;
}

gpointer
fabulor_channel_model_get_flat_at (FabulorChannelModel *model, guint position)
{
	guint i;

	if (!model)
		return NULL;
	for (i = 0; i < model->roots->len; i++)
	{
		FabulorChannelRecord *root = g_ptr_array_index (model->roots, i);
		if (position == 0)
			return root->identity;
		position--;
		if (position < root->children->len)
			return ((FabulorChannelRecord *) g_ptr_array_index (
				root->children, position))->identity;
		position -= root->children->len;
	}
	return NULL;
}

gint
fabulor_channel_model_get_flat_position (FabulorChannelModel *model,
	gpointer identity)
{
	guint count = fabulor_channel_model_get_flat_count (model);
	guint i;

	for (i = 0; i < count; i++)
		if (fabulor_channel_model_get_flat_at (model, i) == identity)
			return (gint) i;
	return -1;
}

const gchar *
fabulor_channel_model_get_name (FabulorChannelModel *model, gpointer identity)
{
	FabulorChannelRecord *record = channel_record_lookup (model, identity);
	return record ? record->name : NULL;
}

PangoAttrList *
fabulor_channel_model_ref_attributes (FabulorChannelModel *model,
	gpointer identity)
{
	FabulorChannelRecord *record = channel_record_lookup (model, identity);
	return record && record->attributes ?
		pango_attr_list_ref (record->attributes) : NULL;
}

#if GTK_MAJOR_VERSION >= 4

GtkTreeListModel *
fabulor_channel_model_get_tree (FabulorChannelModel *model)
{
	return model ? fabulor_gtk4_tree_model_stack_get_tree (model->models) : NULL;
}

GtkSingleSelection *
fabulor_channel_model_get_selection (FabulorChannelModel *model)
{
	return model ? fabulor_gtk4_tree_model_stack_get_selection (model->models) :
		NULL;
}

gpointer
fabulor_channel_model_get_item_identity (gpointer item)
{
	g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE ((GTypeInstance *) item,
		FABULOR_TYPE_CHANNEL_RECORD), NULL);
	return FABULOR_CHANNEL_RECORD (item)->identity;
}

const gchar *
fabulor_channel_model_get_item_name (gpointer item)
{
	g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE ((GTypeInstance *) item,
		FABULOR_TYPE_CHANNEL_RECORD), NULL);
	return FABULOR_CHANNEL_RECORD (item)->name;
}

PangoAttrList *
fabulor_channel_model_get_item_attributes (gpointer item)
{
	g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE ((GTypeInstance *) item,
		FABULOR_TYPE_CHANNEL_RECORD), NULL);
	return FABULOR_CHANNEL_RECORD (item)->attributes;
}

GdkPixbuf *
fabulor_channel_model_get_item_icon (gpointer item)
{
	g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE ((GTypeInstance *) item,
		FABULOR_TYPE_CHANNEL_RECORD), NULL);
	return FABULOR_CHANNEL_RECORD (item)->icon;
}

PangoUnderline
fabulor_channel_model_get_item_underline (gpointer item)
{
	g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE ((GTypeInstance *) item,
		FABULOR_TYPE_CHANNEL_RECORD), PANGO_UNDERLINE_NONE);
	return FABULOR_CHANNEL_RECORD (item)->underline;
}

#else

GtkTreeModel *
fabulor_channel_model_get_tree_model (FabulorChannelModel *model)
{
	return model ? GTK_TREE_MODEL (model->store) : NULL;
}

gboolean
fabulor_channel_model_get_iter (FabulorChannelModel *model, gpointer identity,
	GtkTreeIter *iter)
{
	return channel_record_get_iter (channel_record_lookup (model, identity),
		iter);
}

#endif
