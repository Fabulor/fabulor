/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "user-list-model.h"

#include "gtk4-list-models.h"

struct _FabulorUserListModel
{
	FabulorUserListCompareFunc compare;
	gpointer compare_data;
	gboolean descending;
	GHashTable *rows;
	FabulorGtk4FlatModelStack *models;
	GtkSorter *sorter;
};


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


FabulorUserListModel *
fabulor_user_list_model_new (FabulorUserListCompareFunc compare,
	gpointer compare_data, gboolean descending)
{
	FabulorUserListModel *model = g_new0 (FabulorUserListModel, 1);

	model->compare = compare;
	model->compare_data = compare_data;
	model->descending = descending;
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
	return model;
}

void
fabulor_user_list_model_free (FabulorUserListModel *model)
{
	if (!model)
		return;

	if (model->rows)
		g_hash_table_destroy (model->rows);
	fabulor_gtk4_flat_model_stack_free (model->models);
	g_clear_object (&model->sorter);
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

	{
		FabulorUserListRowObject *item = user_row_object_new (row);
		GListStore *store = fabulor_gtk4_flat_model_stack_get_store (
			model->models);
		g_list_store_insert (store, 0, item);
		g_hash_table_insert (model->rows, row->user, item);
		g_object_unref (item);
	}
	return TRUE;
}

gboolean
fabulor_user_list_model_update (FabulorUserListModel *model,
	const FabulorUserListRow *row, gboolean sort_changed)
{
	g_return_val_if_fail (model != NULL, FALSE);
	g_return_val_if_fail (row != NULL && row->user != NULL, FALSE);

	{
		FabulorUserListRowObject *item = g_hash_table_lookup (
			model->rows, row->user);
		if (!item)
			return FALSE;
		user_row_object_update (item, row);
		if (model->sorter && sort_changed)
			gtk_sorter_changed (model->sorter, GTK_SORTER_CHANGE_DIFFERENT);
	}
	return TRUE;
}

gboolean
fabulor_user_list_model_remove (FabulorUserListModel *model, gpointer user)
{
	g_return_val_if_fail (model != NULL, FALSE);
	g_return_val_if_fail (user != NULL, FALSE);

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
}

void
fabulor_user_list_model_clear (FabulorUserListModel *model)
{
	g_return_if_fail (model != NULL);
	g_hash_table_remove_all (model->rows);
	fabulor_gtk4_flat_model_stack_clear (model->models);
}

guint
fabulor_user_list_model_get_n_rows (FabulorUserListModel *model)
{
	g_return_val_if_fail (model != NULL, 0);
	return g_list_model_get_n_items (G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_sorted (model->models)));
}

gpointer
fabulor_user_list_model_get_user_at (FabulorUserListModel *model,
	guint position)
{
	g_return_val_if_fail (model != NULL, NULL);
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
}


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

GdkPixbuf *
fabulor_user_list_model_get_item_icon (gpointer item)
{
	g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE ((GTypeInstance *) item,
		FABULOR_TYPE_USER_LIST_ROW_OBJECT), NULL);
	return FABULOR_USER_LIST_ROW_OBJECT (item)->icon;
}

const gchar *
fabulor_user_list_model_get_item_prefix (gpointer item)
{
	g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE ((GTypeInstance *) item,
		FABULOR_TYPE_USER_LIST_ROW_OBJECT), NULL);
	return FABULOR_USER_LIST_ROW_OBJECT (item)->prefix_markup;
}

const gchar *
fabulor_user_list_model_get_item_nick (gpointer item)
{
	g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE ((GTypeInstance *) item,
		FABULOR_TYPE_USER_LIST_ROW_OBJECT), NULL);
	return FABULOR_USER_LIST_ROW_OBJECT (item)->nick_markup;
}

const gchar *
fabulor_user_list_model_get_item_host (gpointer item)
{
	g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE ((GTypeInstance *) item,
		FABULOR_TYPE_USER_LIST_ROW_OBJECT), NULL);
	return FABULOR_USER_LIST_ROW_OBJECT (item)->hostname;
}

const GdkRGBA *
fabulor_user_list_model_get_item_foreground (gpointer item)
{
	FabulorUserListRowObject *row;

	g_return_val_if_fail (G_TYPE_CHECK_INSTANCE_TYPE ((GTypeInstance *) item,
		FABULOR_TYPE_USER_LIST_ROW_OBJECT), NULL);
	row = FABULOR_USER_LIST_ROW_OBJECT (item);
	return row->has_foreground ? &row->foreground : NULL;
}
