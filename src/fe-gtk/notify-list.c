/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "notify-list.h"

#include "gtk-compat.h"

#include "gtk4-list-models.h"

typedef struct
{
	gpointer owner;
	gpointer server_data;
	gchar *owner_name;
	gchar *display_name;
	gchar *status;
	gchar *network;
	gchar *last_seen;
	GdkRGBA foreground;
	gboolean has_foreground;
} FabulorNotifyPendingRow;

struct _FabulorNotifyList
{
	GPtrArray *pending;
	gboolean updating;
	FabulorNotifyListSelectionChangedFunc selection_changed;
	gpointer selection_data;
	gulong selection_handler;
	GObject *selection_source;
	GtkWidget *view;
	FabulorGtk4FlatModelStack *models;
};

static FabulorNotifyPendingRow *
pending_row_copy (const FabulorNotifyListRow *source)
{
	FabulorNotifyPendingRow *row = g_new0 (FabulorNotifyPendingRow, 1);

	row->owner = source->owner;
	row->server_data = source->server_data;
	row->owner_name = g_strdup (source->owner_name);
	row->display_name = g_strdup (source->display_name);
	row->status = g_strdup (source->status);
	row->network = g_strdup (source->network);
	row->last_seen = g_strdup (source->last_seen);
	if (source->foreground)
	{
		row->foreground = *source->foreground;
		row->has_foreground = TRUE;
	}
	return row;
}

static FabulorNotifyPendingRow *
pending_row_clone (const FabulorNotifyPendingRow *source)
{
	FabulorNotifyListRow row = {
		source->owner,
		source->server_data,
		source->owner_name,
		source->display_name,
		source->status,
		source->network,
		source->last_seen,
		source->has_foreground ? &source->foreground : NULL
	};
	return pending_row_copy (&row);
}

static void
pending_row_free (gpointer data)
{
	FabulorNotifyPendingRow *row = data;

	if (!row)
		return;
	g_free (row->owner_name);
	g_free (row->display_name);
	g_free (row->status);
	g_free (row->network);
	g_free (row->last_seen);
	g_free (row);
}

static gboolean
pending_row_has_identity (const FabulorNotifyPendingRow *row,
							  gpointer owner, gpointer server_data)
{
	return row->owner == owner && row->server_data == server_data;
}

static void
notify_selection_changed (FabulorNotifyList *list)
{
	if (list->selection_changed)
		list->selection_changed (list->selection_data);
}


typedef struct _FabulorNotifyRow FabulorNotifyRow;
typedef struct _FabulorNotifyRowClass FabulorNotifyRowClass;

struct _FabulorNotifyRow
{
	GObject parent_instance;
	FabulorNotifyPendingRow *data;
};

struct _FabulorNotifyRowClass
{
	GObjectClass parent_class;
};

enum
{
	PROP_ROW_0,
	PROP_ROW_DISPLAY_NAME,
	PROP_ROW_STATUS,
	PROP_ROW_NETWORK,
	PROP_ROW_LAST_SEEN,
	PROP_ROW_FOREGROUND,
	N_ROW_PROPERTIES
};

static GParamSpec *row_properties[N_ROW_PROPERTIES];

#define FABULOR_TYPE_NOTIFY_ROW (fabulor_notify_row_get_type ())
#define FABULOR_NOTIFY_ROW(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), FABULOR_TYPE_NOTIFY_ROW, FabulorNotifyRow))

G_DEFINE_TYPE (FabulorNotifyRow, fabulor_notify_row, G_TYPE_OBJECT)

static void
fabulor_notify_row_get_property (GObject *object, guint property_id,
								 GValue *value, GParamSpec *pspec)
{
	FabulorNotifyPendingRow *row = FABULOR_NOTIFY_ROW (object)->data;

	switch (property_id)
	{
	case PROP_ROW_DISPLAY_NAME:
		g_value_set_string (value, row->display_name);
		break;
	case PROP_ROW_STATUS:
		g_value_set_string (value, row->status);
		break;
	case PROP_ROW_NETWORK:
		g_value_set_string (value, row->network);
		break;
	case PROP_ROW_LAST_SEEN:
		g_value_set_string (value, row->last_seen);
		break;
	case PROP_ROW_FOREGROUND:
		g_value_set_boxed (value, row->has_foreground ? &row->foreground : NULL);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
fabulor_notify_row_finalize (GObject *object)
{
	pending_row_free (FABULOR_NOTIFY_ROW (object)->data);
	G_OBJECT_CLASS (fabulor_notify_row_parent_class)->finalize (object);
}

static void
fabulor_notify_row_class_init (FabulorNotifyRowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = fabulor_notify_row_get_property;
	object_class->finalize = fabulor_notify_row_finalize;
	row_properties[PROP_ROW_DISPLAY_NAME] = g_param_spec_string (
		"display-name", "Display name", "Visible notify name", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	row_properties[PROP_ROW_STATUS] = g_param_spec_string (
		"status", "Status", "Notify status", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	row_properties[PROP_ROW_NETWORK] = g_param_spec_string (
		"network", "Network", "Notify network", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	row_properties[PROP_ROW_LAST_SEEN] = g_param_spec_string (
		"last-seen", "Last seen", "Last seen text", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	row_properties[PROP_ROW_FOREGROUND] = g_param_spec_boxed (
		"foreground", "Foreground", "Row foreground colour", GDK_TYPE_RGBA,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties (object_class, N_ROW_PROPERTIES,
		row_properties);
}

static void
fabulor_notify_row_init (FabulorNotifyRow *row)
{
	(void) row;
}

static FabulorNotifyRow *
notify_row_new (const FabulorNotifyPendingRow *source)
{
	FabulorNotifyRow *row = g_object_new (FABULOR_TYPE_NOTIFY_ROW, NULL);

	row->data = pending_row_clone (source);
	return row;
}

static void
notify_row_update (FabulorNotifyRow *row,
				   const FabulorNotifyPendingRow *source)
{
	gboolean display_name_changed =
		g_strcmp0 (row->data->display_name, source->display_name) != 0;
	gboolean status_changed = g_strcmp0 (row->data->status, source->status) != 0;
	gboolean network_changed =
		g_strcmp0 (row->data->network, source->network) != 0;
	gboolean last_seen_changed =
		g_strcmp0 (row->data->last_seen, source->last_seen) != 0;
	gboolean foreground_changed =
		row->data->has_foreground != source->has_foreground ||
		(row->data->has_foreground && source->has_foreground &&
		 !gdk_rgba_equal (&row->data->foreground, &source->foreground));

	pending_row_free (row->data);
	row->data = pending_row_clone (source);
	if (display_name_changed)
		g_object_notify_by_pspec (G_OBJECT (row),
			row_properties[PROP_ROW_DISPLAY_NAME]);
	if (status_changed)
		g_object_notify_by_pspec (G_OBJECT (row), row_properties[PROP_ROW_STATUS]);
	if (network_changed)
		g_object_notify_by_pspec (G_OBJECT (row), row_properties[PROP_ROW_NETWORK]);
	if (last_seen_changed)
		g_object_notify_by_pspec (G_OBJECT (row), row_properties[PROP_ROW_LAST_SEEN]);
	if (foreground_changed)
		g_object_notify_by_pspec (G_OBJECT (row),
			row_properties[PROP_ROW_FOREGROUND]);
}

typedef enum
{
	NOTIFY_FIELD_NAME,
	NOTIFY_FIELD_STATUS,
	NOTIFY_FIELD_NETWORK,
	NOTIFY_FIELD_LAST_SEEN
} FabulorNotifyField;

typedef struct
{
	FabulorNotifyRow *row;
	GtkLabel *label;
	FabulorNotifyField field;
	gulong notify_handler;
} FabulorNotifyCellBinding;

static const gchar *
notify_row_field_text (FabulorNotifyRow *row, FabulorNotifyField field)
{
	switch (field)
	{
	case NOTIFY_FIELD_NAME:
		return row->data->display_name;
	case NOTIFY_FIELD_STATUS:
		return row->data->status;
	case NOTIFY_FIELD_NETWORK:
		return row->data->network;
	case NOTIFY_FIELD_LAST_SEEN:
		return row->data->last_seen;
	default:
		return "";
	}
}

static guint16
notify_color_component (gdouble component)
{
	return (guint16) (CLAMP (component, 0.0, 1.0) * 65535.0 + 0.5);
}

static void
notify_cell_binding_refresh (FabulorNotifyCellBinding *binding)
{
	FabulorNotifyPendingRow *data = binding->row->data;
	PangoAttrList *attributes = NULL;

	gtk_label_set_text (binding->label,
		notify_row_field_text (binding->row, binding->field));
	if (data->has_foreground)
	{
		PangoAttribute *foreground;

		attributes = pango_attr_list_new ();
		foreground = pango_attr_foreground_new (
			notify_color_component (data->foreground.red),
			notify_color_component (data->foreground.green),
			notify_color_component (data->foreground.blue));
		pango_attr_list_insert (attributes, foreground);
		pango_attr_list_insert (attributes, pango_attr_foreground_alpha_new (
			notify_color_component (data->foreground.alpha)));
	}
	gtk_label_set_attributes (binding->label, attributes);
	if (attributes)
		pango_attr_list_unref (attributes);
}

static void
notify_cell_row_changed (GObject *object, GParamSpec *pspec, gpointer user_data)
{
	FabulorNotifyCellBinding *binding = user_data;
	guint field_property = PROP_ROW_DISPLAY_NAME + (guint) binding->field;

	(void) object;
	if (pspec == row_properties[field_property] ||
		pspec == row_properties[PROP_ROW_FOREGROUND])
		notify_cell_binding_refresh (binding);
}

static void
notify_cell_binding_free (gpointer data)
{
	FabulorNotifyCellBinding *binding = data;

	if (!binding)
		return;
	if (binding->notify_handler)
		g_signal_handler_disconnect (binding->row, binding->notify_handler);
	g_object_unref (binding->row);
	g_free (binding);
}

static void
notify_factory_setup (GtkSignalListItemFactory *factory, GtkListItem *list_item,
					  gpointer user_data)
{
	GtkWidget *label = gtk_label_new (NULL);

	(void) factory;
	(void) user_data;
	gtk_label_set_xalign (GTK_LABEL (label), 0.0f);
	gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
	gtk_list_item_set_child (list_item, label);
}

static void
notify_factory_bind (GtkSignalListItemFactory *factory, GtkListItem *list_item,
					 gpointer user_data)
{
	FabulorNotifyCellBinding *binding = g_new0 (FabulorNotifyCellBinding, 1);

	(void) factory;
	binding->row = g_object_ref (FABULOR_NOTIFY_ROW (
		gtk_list_item_get_item (list_item)));
	binding->label = GTK_LABEL (gtk_list_item_get_child (list_item));
	binding->field = (FabulorNotifyField) GPOINTER_TO_UINT (user_data);
	binding->notify_handler = g_signal_connect (binding->row, "notify",
		G_CALLBACK (notify_cell_row_changed), binding);
	notify_cell_binding_refresh (binding);
	g_object_set_data_full (G_OBJECT (list_item), "fabulor-notify-cell-binding",
		binding, notify_cell_binding_free);
}

static void
notify_factory_unbind (GtkSignalListItemFactory *factory,
					   GtkListItem *list_item, gpointer user_data)
{
	(void) factory;
	(void) user_data;
	g_object_set_data (G_OBJECT (list_item),
		"fabulor-notify-cell-binding", NULL);
}

static void
notify_column_append (GtkColumnView *view, const gchar *title,
					  FabulorNotifyField field, gboolean expand)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	GtkColumnViewColumn *column;

	g_signal_connect (factory, "setup", G_CALLBACK (notify_factory_setup), NULL);
	g_signal_connect (factory, "bind", G_CALLBACK (notify_factory_bind),
		GUINT_TO_POINTER ((guint) field));
	g_signal_connect (factory, "unbind", G_CALLBACK (notify_factory_unbind), NULL);
	column = gtk_column_view_column_new (title, factory);
	gtk_column_view_column_set_expand (column, expand);
	gtk_column_view_column_set_resizable (column, TRUE);
	gtk_column_view_append_column (view, column);
	g_object_unref (column);
}

static void
notify_gtk4_selection_changed (GtkSelectionModel *selection, guint position,
							   guint n_items, gpointer user_data)
{
	(void) selection;
	(void) position;
	(void) n_items;
	notify_selection_changed (user_data);
}

static FabulorNotifyRow *
notify_gtk4_selected_row (FabulorNotifyList *list)
{
	GtkSelectionModel *selection =
		fabulor_gtk4_flat_model_stack_get_selection (list->models);

	if (!GTK_IS_SINGLE_SELECTION (selection))
		return NULL;
	return FABULOR_NOTIFY_ROW (
		gtk_single_selection_get_selected_item (GTK_SINGLE_SELECTION (selection)));
}

static void
notify_gtk4_apply_pending (FabulorNotifyList *list)
{
	GListStore *store = fabulor_gtk4_flat_model_stack_get_store (list->models);
	GPtrArray *items = g_ptr_array_new_with_free_func (g_object_unref);
	guint old_count = g_list_model_get_n_items (G_LIST_MODEL (store));
	gboolean same_order;
	guint i;

	for (i = 0; i < list->pending->len; i++)
	{
		FabulorNotifyPendingRow *pending = g_ptr_array_index (list->pending, i);
		FabulorNotifyRow *match = NULL;
		guint old_index;

		for (old_index = 0; old_index < old_count; old_index++)
		{
			FabulorNotifyRow *candidate = g_list_model_get_item (
				G_LIST_MODEL (store), old_index);
			if (pending_row_has_identity (candidate->data, pending->owner,
										  pending->server_data))
			{
				match = candidate;
				break;
			}
			g_object_unref (candidate);
		}

		if (match)
			notify_row_update (match, pending);
		else
			match = notify_row_new (pending);
		g_ptr_array_add (items, match);
	}

	same_order = old_count == items->len;
	for (i = 0; same_order && i < old_count; i++)
	{
		gpointer old_item = g_list_model_get_item (G_LIST_MODEL (store), i);
		same_order = old_item == g_ptr_array_index (items, i);
		g_object_unref (old_item);
	}
	if (!same_order)
		g_list_store_splice (store, 0, old_count, items->pdata, items->len);
	g_ptr_array_unref (items);
}


FabulorNotifyList *
fabulor_notify_list_new (
	FabulorNotifyListSelectionChangedFunc selection_changed,
	gpointer user_data)
{
	FabulorNotifyList *list = g_new0 (FabulorNotifyList, 1);

	list->pending = g_ptr_array_new_with_free_func (pending_row_free);
	list->selection_changed = selection_changed;
	list->selection_data = user_data;
	list->models = fabulor_gtk4_flat_model_stack_new (FABULOR_TYPE_NOTIFY_ROW,
		NULL, FABULOR_GTK4_SELECTION_SINGLE);
	if (!list->models)
	{
		fabulor_notify_list_free (list);
		return NULL;
	}
	list->selection_source = G_OBJECT (g_object_ref (
		fabulor_gtk4_flat_model_stack_get_selection (list->models)));
	list->selection_handler = g_signal_connect (list->selection_source,
		"selection-changed", G_CALLBACK (notify_gtk4_selection_changed), list);
	return list;
}

void
fabulor_notify_list_free (FabulorNotifyList *list)
{
	if (!list)
		return;

	if (list->selection_handler && list->selection_source)
		g_signal_handler_disconnect (list->selection_source,
			list->selection_handler);
	g_clear_object (&list->selection_source);
	fabulor_gtk4_flat_model_stack_free (list->models);
	g_ptr_array_unref (list->pending);
	g_free (list);
}

GtkWidget *
fabulor_notify_list_create_view (FabulorNotifyList *list, GtkBox *parent,
							 const gchar *name_title,
							 const gchar *status_title,
							 const gchar *network_title,
							 const gchar *last_seen_title)
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
		notify_column_append (GTK_COLUMN_VIEW (list->view), name_title,
			NOTIFY_FIELD_NAME, TRUE);
		notify_column_append (GTK_COLUMN_VIEW (list->view), status_title,
			NOTIFY_FIELD_STATUS, FALSE);
		notify_column_append (GTK_COLUMN_VIEW (list->view), network_title,
			NOTIFY_FIELD_NETWORK, FALSE);
		notify_column_append (GTK_COLUMN_VIEW (list->view), last_seen_title,
			NOTIFY_FIELD_LAST_SEEN, FALSE);
		gtk_column_view_set_single_click_activate (
			GTK_COLUMN_VIEW (list->view), FALSE);
		fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroller),
			list->view);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_widget_set_vexpand (scroller, TRUE);
		gtk_widget_set_hexpand (scroller, TRUE);
		fabulor_gtk_box_append (parent, scroller, TRUE, TRUE, 0);
	}
	return list->view;
}

void
fabulor_notify_list_begin_update (FabulorNotifyList *list)
{
	g_return_if_fail (list != NULL);
	g_return_if_fail (!list->updating);
	g_ptr_array_set_size (list->pending, 0);
	list->updating = TRUE;
}

gboolean
fabulor_notify_list_append (FabulorNotifyList *list,
							const FabulorNotifyListRow *row)
{
	guint i;

	g_return_val_if_fail (list != NULL, FALSE);
	g_return_val_if_fail (list->updating, FALSE);
	g_return_val_if_fail (row != NULL, FALSE);
	g_return_val_if_fail (row->owner != NULL, FALSE);
	g_return_val_if_fail (row->owner_name != NULL, FALSE);
	g_return_val_if_fail (row->display_name != NULL, FALSE);
	g_return_val_if_fail (row->status != NULL, FALSE);
	g_return_val_if_fail (row->network != NULL, FALSE);
	g_return_val_if_fail (row->last_seen != NULL, FALSE);

	for (i = 0; i < list->pending->len; i++)
	{
		FabulorNotifyPendingRow *existing = g_ptr_array_index (list->pending, i);
		if (pending_row_has_identity (existing, row->owner, row->server_data))
			return FALSE;
	}

	g_ptr_array_add (list->pending, pending_row_copy (row));
	return TRUE;
}

static gboolean
notify_list_get_selected_identity (FabulorNotifyList *list, gpointer *owner,
								   gpointer *server_data)
{
	FabulorNotifyRow *row = notify_gtk4_selected_row (list);

	if (!row)
		return FALSE;
	*owner = row->data->owner;
	*server_data = row->data->server_data;
	return TRUE;
}

void
fabulor_notify_list_end_update (FabulorNotifyList *list)
{
	gpointer selected_owner = NULL;
	gpointer selected_server = NULL;
	gboolean had_selection;

	g_return_if_fail (list != NULL);
	g_return_if_fail (list->updating);
	had_selection = notify_list_get_selected_identity (list, &selected_owner,
		&selected_server);
	if (list->selection_handler)
	{
		g_signal_handler_block (list->selection_source, list->selection_handler);
	}

	notify_gtk4_apply_pending (list);
	if (had_selection)
		fabulor_notify_list_select_identity (list, selected_owner, selected_server);

	if (list->selection_handler)
	{
		g_signal_handler_unblock (list->selection_source, list->selection_handler);
	}
	list->updating = FALSE;
	g_ptr_array_set_size (list->pending, 0);
	notify_selection_changed (list);
}

guint
fabulor_notify_list_get_n_rows (FabulorNotifyList *list)
{
	g_return_val_if_fail (list != NULL, 0);
	return g_list_model_get_n_items (G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_store (list->models)));
}

gboolean
fabulor_notify_list_has_selection (FabulorNotifyList *list)
{
	gpointer owner;
	gpointer server_data;

	g_return_val_if_fail (list != NULL, FALSE);
	return notify_list_get_selected_identity (list, &owner, &server_data);
}

gchar *
fabulor_notify_list_dup_selected_name (FabulorNotifyList *list)
{
	g_return_val_if_fail (list != NULL, NULL);
	{
		FabulorNotifyRow *row = notify_gtk4_selected_row (list);
		return row ? g_strdup (row->data->owner_name) : NULL;
	}
}

gpointer
fabulor_notify_list_get_selected_server_data (FabulorNotifyList *list)
{
	gpointer owner;
	gpointer server_data;

	g_return_val_if_fail (list != NULL, NULL);
	if (!notify_list_get_selected_identity (list, &owner, &server_data))
		return NULL;
	return server_data;
}

gboolean
fabulor_notify_list_select_identity (FabulorNotifyList *list, gpointer owner,
								 gpointer server_data)
{
	gint owner_fallback = -1;
	guint position = 0;

	g_return_val_if_fail (list != NULL, FALSE);
	g_return_val_if_fail (owner != NULL, FALSE);

	{
		GListModel *model = G_LIST_MODEL (
			fabulor_gtk4_flat_model_stack_get_store (list->models));
		GtkSelectionModel *selection =
			fabulor_gtk4_flat_model_stack_get_selection (list->models);
		guint count = g_list_model_get_n_items (model);

		for (position = 0; position < count; position++)
		{
			FabulorNotifyRow *row = g_list_model_get_item (model, position);
			gboolean exact = pending_row_has_identity (row->data, owner,
				server_data);
			if (row->data->owner == owner && owner_fallback < 0)
				owner_fallback = (gint) position;
			g_object_unref (row);
			if (exact)
				break;
		}
		if (position >= count && owner_fallback >= 0)
			position = (guint) owner_fallback;
		if (position < count)
			return gtk_selection_model_select_item (selection, position, TRUE);
		gtk_selection_model_unselect_all (selection);
		return FALSE;
	}
}
