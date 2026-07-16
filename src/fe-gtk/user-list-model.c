/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "user-list-model.h"

#if GTK_MAJOR_VERSION >= 4
#include "gtk4-list-models.h"
#else
#include "theme/theme-gtk.h"
#endif

struct _FabulorUserListModel
{
	FabulorUserListCompareFunc compare;
	gpointer compare_data;
	gboolean descending;
	GHashTable *rows;
#if GTK_MAJOR_VERSION >= 4
	FabulorGtk4FlatModelStack *models;
	GtkSorter *sorter;
#else
	GtkListStore *store;
#endif
};

#if GTK_MAJOR_VERSION >= 4

typedef struct _FabulorUserListRowObject FabulorUserListRowObject;
typedef struct _FabulorUserListRowObjectClass FabulorUserListRowObjectClass;

struct _FabulorUserListRowObject
{
	GObject parent_instance;
	gpointer user;
	GdkPixbuf *icon;
	gchar *prefix_markup;
	gchar *nick_markup;
	gchar *hostname;
	GdkRGBA foreground;
	gboolean has_foreground;
};

struct _FabulorUserListRowObjectClass
{
	GObjectClass parent_class;
};

enum
{
	PROP_USER_ROW_0,
	PROP_USER_ROW_ICON,
	PROP_USER_ROW_PREFIX,
	PROP_USER_ROW_NICK,
	PROP_USER_ROW_HOST,
	PROP_USER_ROW_FOREGROUND,
	N_USER_ROW_PROPERTIES
};

static GParamSpec *user_row_properties[N_USER_ROW_PROPERTIES];

#define FABULOR_TYPE_USER_LIST_ROW_OBJECT \
	(fabulor_user_list_row_object_get_type ())
#define FABULOR_USER_LIST_ROW_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), FABULOR_TYPE_USER_LIST_ROW_OBJECT, \
		FabulorUserListRowObject))

G_DEFINE_TYPE (FabulorUserListRowObject, fabulor_user_list_row_object,
	G_TYPE_OBJECT)

static void
fabulor_user_list_row_object_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	FabulorUserListRowObject *row = FABULOR_USER_LIST_ROW_OBJECT (object);

	switch (property_id)
	{
	case PROP_USER_ROW_ICON:
		g_value_set_object (value, row->icon);
		break;
	case PROP_USER_ROW_PREFIX:
		g_value_set_string (value, row->prefix_markup);
		break;
	case PROP_USER_ROW_NICK:
		g_value_set_string (value, row->nick_markup);
		break;
	case PROP_USER_ROW_HOST:
		g_value_set_string (value, row->hostname);
		break;
	case PROP_USER_ROW_FOREGROUND:
		g_value_set_boxed (value,
			row->has_foreground ? &row->foreground : NULL);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
fabulor_user_list_row_object_finalize (GObject *object)
{
	FabulorUserListRowObject *row = FABULOR_USER_LIST_ROW_OBJECT (object);

	g_clear_object (&row->icon);
	g_free (row->prefix_markup);
	g_free (row->nick_markup);
	g_free (row->hostname);
	G_OBJECT_CLASS (fabulor_user_list_row_object_parent_class)->finalize (object);
}

static void
fabulor_user_list_row_object_class_init (
	FabulorUserListRowObjectClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = fabulor_user_list_row_object_get_property;
	object_class->finalize = fabulor_user_list_row_object_finalize;
	user_row_properties[PROP_USER_ROW_ICON] = g_param_spec_object (
		"icon", "Icon", "User privilege icon", GDK_TYPE_PIXBUF,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	user_row_properties[PROP_USER_ROW_PREFIX] = g_param_spec_string (
		"prefix-markup", "Prefix markup", "User prefix markup", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	user_row_properties[PROP_USER_ROW_NICK] = g_param_spec_string (
		"nick-markup", "Nick markup", "User nickname markup", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	user_row_properties[PROP_USER_ROW_HOST] = g_param_spec_string (
		"hostname", "Hostname", "User hostname", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	user_row_properties[PROP_USER_ROW_FOREGROUND] = g_param_spec_boxed (
		"foreground", "Foreground", "Nickname foreground colour",
		GDK_TYPE_RGBA, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties (object_class, N_USER_ROW_PROPERTIES,
		user_row_properties);
}

static void
fabulor_user_list_row_object_init (FabulorUserListRowObject *row)
{
	(void) row;
}

static gboolean
rgba_changed (FabulorUserListRowObject *target,
	const FabulorUserListRow *source)
{
	gboolean source_has_color = source->foreground != NULL;

	return target->has_foreground != source_has_color ||
		(target->has_foreground && source_has_color &&
		 !gdk_rgba_equal (&target->foreground, source->foreground));
}

static void
user_row_object_update (FabulorUserListRowObject *target,
	const FabulorUserListRow *source)
{
	gboolean icon_changed = target->icon != source->icon;
	gboolean prefix_changed =
		g_strcmp0 (target->prefix_markup, source->prefix_markup) != 0;
	gboolean nick_changed =
		g_strcmp0 (target->nick_markup, source->nick_markup) != 0;
	gboolean host_changed = g_strcmp0 (target->hostname, source->hostname) != 0;
	gboolean foreground_changed = rgba_changed (target, source);

	if (icon_changed)
		g_set_object (&target->icon, source->icon);
	if (prefix_changed)
	{
		g_free (target->prefix_markup);
		target->prefix_markup = g_strdup (source->prefix_markup);
	}
	if (nick_changed)
	{
		g_free (target->nick_markup);
		target->nick_markup = g_strdup (source->nick_markup);
	}
	if (host_changed)
	{
		g_free (target->hostname);
		target->hostname = g_strdup (source->hostname);
	}
	if (foreground_changed)
	{
		target->has_foreground = source->foreground != NULL;
		if (source->foreground)
			target->foreground = *source->foreground;
	}

	if (icon_changed)
		g_object_notify_by_pspec (G_OBJECT (target),
			user_row_properties[PROP_USER_ROW_ICON]);
	if (prefix_changed)
		g_object_notify_by_pspec (G_OBJECT (target),
			user_row_properties[PROP_USER_ROW_PREFIX]);
	if (nick_changed)
		g_object_notify_by_pspec (G_OBJECT (target),
			user_row_properties[PROP_USER_ROW_NICK]);
	if (host_changed)
		g_object_notify_by_pspec (G_OBJECT (target),
			user_row_properties[PROP_USER_ROW_HOST]);
	if (foreground_changed)
		g_object_notify_by_pspec (G_OBJECT (target),
			user_row_properties[PROP_USER_ROW_FOREGROUND]);
}

static FabulorUserListRowObject *
user_row_object_new (const FabulorUserListRow *source)
{
	FabulorUserListRowObject *row = g_object_new (
		FABULOR_TYPE_USER_LIST_ROW_OBJECT, NULL);

	row->user = source->user;
	user_row_object_update (row, source);
	return row;
}

static gint
user_list_gtk4_compare (gconstpointer left, gconstpointer right,
	gpointer user_data)
{
	FabulorUserListModel *model = user_data;
	FabulorUserListRowObject *left_row = FABULOR_USER_LIST_ROW_OBJECT (
		(gpointer) left);
	FabulorUserListRowObject *right_row = FABULOR_USER_LIST_ROW_OBJECT (
		(gpointer) right);
	gint result = model->compare (left_row->user, right_row->user,
		model->compare_data);

	if (!model->descending)
		return result;
	return result < 0 ? 1 : result > 0 ? -1 : 0;
}

#else

static gint
user_list_gtk3_compare (GtkTreeModel *tree_model, GtkTreeIter *left,
	GtkTreeIter *right, gpointer user_data)
{
	FabulorUserListModel *model = user_data;
	gpointer left_user;
	gpointer right_user;
	gint result;

	gtk_tree_model_get (tree_model, left,
		FABULOR_USER_LIST_COLUMN_USER, &left_user, -1);
	gtk_tree_model_get (tree_model, right,
		FABULOR_USER_LIST_COLUMN_USER, &right_user, -1);
	result = model->compare (left_user, right_user, model->compare_data);
	if (!model->descending)
		return result;
	return result < 0 ? 1 : result > 0 ? -1 : 0;
}

static void
user_list_store_color (FabulorUserListModel *model, GtkTreeIter *iter,
	const GdkRGBA *foreground)
{
	gtk_list_store_set (model->store, iter,
		FABULOR_USER_LIST_COLUMN_FOREGROUND, foreground, -1);
}

#endif

FabulorUserListModel *
fabulor_user_list_model_new (FabulorUserListCompareFunc compare,
	gpointer compare_data, gboolean descending)
{
	FabulorUserListModel *model = g_new0 (FabulorUserListModel, 1);

	model->compare = compare;
	model->compare_data = compare_data;
	model->descending = descending;
#if GTK_MAJOR_VERSION >= 4
	if (compare)
		model->sorter = GTK_SORTER (gtk_custom_sorter_new (
			user_list_gtk4_compare, model, NULL));
	model->models = fabulor_gtk4_flat_model_stack_new (
		FABULOR_TYPE_USER_LIST_ROW_OBJECT, model->sorter,
		FABULOR_GTK4_SELECTION_MULTIPLE);
	if (!model->models)
	{
		fabulor_user_list_model_free (model);
		return NULL;
	}
	model->rows = g_hash_table_new (g_direct_hash, g_direct_equal);
#else
	model->store = gtk_list_store_new (FABULOR_USER_LIST_N_COLUMNS,
		GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_POINTER, THEME_GTK_COLOR_TYPE);
	model->rows = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL,
		(GDestroyNotify) gtk_tree_row_reference_free);
	if (compare)
	{
		gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (model->store),
			user_list_gtk3_compare, model, NULL);
		gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model->store),
			GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);
	}
#endif
	return model;
}

void
fabulor_user_list_model_free (FabulorUserListModel *model)
{
	if (!model)
		return;

	if (model->rows)
		g_hash_table_destroy (model->rows);
#if GTK_MAJOR_VERSION >= 4
	fabulor_gtk4_flat_model_stack_free (model->models);
	g_clear_object (&model->sorter);
#else
	g_clear_object (&model->store);
#endif
	g_free (model);
}

gboolean
fabulor_user_list_model_insert (FabulorUserListModel *model,
	const FabulorUserListRow *row)
{
	g_return_val_if_fail (model != NULL, FALSE);
	g_return_val_if_fail (row != NULL && row->user != NULL, FALSE);
	if (g_hash_table_contains (model->rows, row->user))
		return FALSE;

#if GTK_MAJOR_VERSION >= 4
	{
		FabulorUserListRowObject *item = user_row_object_new (row);
		GListStore *store = fabulor_gtk4_flat_model_stack_get_store (
			model->models);
		g_list_store_insert (store, 0, item);
		g_hash_table_insert (model->rows, row->user, item);
		g_object_unref (item);
	}
#else
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GtkTreeRowReference *reference;

		gtk_list_store_insert_with_values (model->store, &iter, 0,
			FABULOR_USER_LIST_COLUMN_ICON, row->icon,
			FABULOR_USER_LIST_COLUMN_PREFIX, row->prefix_markup,
			FABULOR_USER_LIST_COLUMN_NICK, row->nick_markup,
			FABULOR_USER_LIST_COLUMN_HOST, row->hostname,
			FABULOR_USER_LIST_COLUMN_USER, row->user, -1);
		user_list_store_color (model, &iter, row->foreground);
		path = gtk_tree_model_get_path (GTK_TREE_MODEL (model->store), &iter);
		reference = gtk_tree_row_reference_new (
			GTK_TREE_MODEL (model->store), path);
		gtk_tree_path_free (path);
		if (!reference)
		{
			gtk_list_store_remove (model->store, &iter);
			return FALSE;
		}
		g_hash_table_insert (model->rows, row->user, reference);
	}
#endif
	return TRUE;
}

gboolean
fabulor_user_list_model_update (FabulorUserListModel *model,
	const FabulorUserListRow *row, gboolean sort_changed)
{
	g_return_val_if_fail (model != NULL, FALSE);
	g_return_val_if_fail (row != NULL && row->user != NULL, FALSE);

#if GTK_MAJOR_VERSION >= 4
	{
		FabulorUserListRowObject *item = g_hash_table_lookup (
			model->rows, row->user);
		if (!item)
			return FALSE;
		user_row_object_update (item, row);
		if (model->sorter && sort_changed)
			gtk_sorter_changed (model->sorter, GTK_SORTER_CHANGE_DIFFERENT);
	}
#else
	{
		GtkTreeIter iter;
		if (!fabulor_user_list_model_get_iter (model, row->user, &iter))
			return FALSE;
		gtk_list_store_set (model->store, &iter,
			FABULOR_USER_LIST_COLUMN_ICON, row->icon,
			FABULOR_USER_LIST_COLUMN_PREFIX, row->prefix_markup,
			FABULOR_USER_LIST_COLUMN_NICK, row->nick_markup,
			FABULOR_USER_LIST_COLUMN_HOST, row->hostname, -1);
		user_list_store_color (model, &iter, row->foreground);
		if (sort_changed && model->compare)
			gtk_tree_sortable_sort_column_changed (
				GTK_TREE_SORTABLE (model->store));
	}
#endif
	return TRUE;
}

gboolean
fabulor_user_list_model_remove (FabulorUserListModel *model, gpointer user)
{
	g_return_val_if_fail (model != NULL, FALSE);
	g_return_val_if_fail (user != NULL, FALSE);

#if GTK_MAJOR_VERSION >= 4
	{
		FabulorUserListRowObject *item = g_hash_table_lookup (model->rows, user);
		gboolean removed;
		if (!item)
			return FALSE;
		removed = fabulor_gtk4_flat_model_stack_remove (model->models, item);
		if (removed)
			g_hash_table_remove (model->rows, user);
		return removed;
	}
#else
	{
		GtkTreeIter iter;
		if (!fabulor_user_list_model_get_iter (model, user, &iter))
			return FALSE;
		g_hash_table_remove (model->rows, user);
		gtk_list_store_remove (model->store, &iter);
		return TRUE;
	}
#endif
}

void
fabulor_user_list_model_clear (FabulorUserListModel *model)
{
	g_return_if_fail (model != NULL);
	g_hash_table_remove_all (model->rows);
#if GTK_MAJOR_VERSION >= 4
	fabulor_gtk4_flat_model_stack_clear (model->models);
#else
	gtk_list_store_clear (model->store);
#endif
}

guint
fabulor_user_list_model_get_n_rows (FabulorUserListModel *model)
{
	g_return_val_if_fail (model != NULL, 0);
#if GTK_MAJOR_VERSION >= 4
	return g_list_model_get_n_items (G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_sorted (model->models)));
#else
	return (guint) gtk_tree_model_iter_n_children (
		GTK_TREE_MODEL (model->store), NULL);
#endif
}

gpointer
fabulor_user_list_model_get_user_at (FabulorUserListModel *model,
	guint position)
{
	g_return_val_if_fail (model != NULL, NULL);
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorUserListRowObject *row = g_list_model_get_item (G_LIST_MODEL (
			fabulor_gtk4_flat_model_stack_get_sorted (model->models)), position);
		gpointer user;
		if (!row)
			return NULL;
		user = row->user;
		g_object_unref (row);
		return user;
	}
#else
	{
		GtkTreeIter iter;
		gpointer user = NULL;
		if (!gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (model->store), &iter,
			NULL, (gint) position))
			return NULL;
		gtk_tree_model_get (GTK_TREE_MODEL (model->store), &iter,
			FABULOR_USER_LIST_COLUMN_USER, &user, -1);
		return user;
	}
#endif
}

#if GTK_MAJOR_VERSION >= 4

GListModel *
fabulor_user_list_model_get_list_model (FabulorUserListModel *model)
{
	g_return_val_if_fail (model != NULL, NULL);
	return G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_sorted (model->models));
}

GtkSelectionModel *
fabulor_user_list_model_get_selection (FabulorUserListModel *model)
{
	g_return_val_if_fail (model != NULL, NULL);
	return fabulor_gtk4_flat_model_stack_get_selection (model->models);
}

gpointer
fabulor_user_list_model_get_item_user (gpointer item)
{
	g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE ((GTypeInstance *) item,
		FABULOR_TYPE_USER_LIST_ROW_OBJECT), NULL);
	return FABULOR_USER_LIST_ROW_OBJECT (item)->user;
}

#else

GtkTreeModel *
fabulor_user_list_model_get_tree_model (FabulorUserListModel *model)
{
	g_return_val_if_fail (model != NULL, NULL);
	return GTK_TREE_MODEL (model->store);
}

gboolean
fabulor_user_list_model_get_iter (FabulorUserListModel *model, gpointer user,
	GtkTreeIter *iter)
{
	GtkTreeRowReference *reference;
	GtkTreePath *path;
	gpointer row_user;

	g_return_val_if_fail (model != NULL, FALSE);
	g_return_val_if_fail (user != NULL, FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);
	reference = g_hash_table_lookup (model->rows, user);
	if (!reference)
		return FALSE;
	path = gtk_tree_row_reference_get_path (reference);
	if (!path)
	{
		g_hash_table_remove (model->rows, user);
		return FALSE;
	}
	if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (model->store), iter, path))
	{
		gtk_tree_path_free (path);
		g_hash_table_remove (model->rows, user);
		return FALSE;
	}
	gtk_tree_path_free (path);
	gtk_tree_model_get (GTK_TREE_MODEL (model->store), iter,
		FABULOR_USER_LIST_COLUMN_USER, &row_user, -1);
	if (row_user != user)
	{
		g_hash_table_remove (model->rows, user);
		return FALSE;
	}
	return TRUE;
}

#endif
