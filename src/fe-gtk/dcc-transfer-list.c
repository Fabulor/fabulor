/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "dcc-transfer-list.h"

#include "gtk-compat.h"

#include "gtk4-list-models.h"

struct _FabulorDccTransferList
{
	GtkWidget *view;
	FabulorDccTransferSelectionFunc selection_func;
	FabulorDccTransferActivateFunc activate_func;
	gpointer callback_data;
	FabulorGtk4FlatModelStack *models;
	GHashTable *rows;
};

static void
dcc_transfer_selection_changed (FabulorDccTransferList *list)
{
	if (list->selection_func)
		list->selection_func (list->callback_data);
}


typedef struct _FabulorDccTransferRow FabulorDccTransferRow;
typedef struct _FabulorDccTransferRowClass FabulorDccTransferRowClass;

struct _FabulorDccTransferRow
{
	GObject parent_instance;
	gpointer identity;
	gboolean upload;
	gchar *status;
	gchar *file;
	gchar *size;
	gchar *position;
	gchar *percentage;
	gchar *speed;
	gchar *eta;
	gchar *nick;
	gboolean has_color;
	GdkRGBA color;
};

struct _FabulorDccTransferRowClass
{
	GObjectClass parent_class;
};

enum
{
	PROP_TRANSFER_0,
	PROP_TRANSFER_STATUS,
	PROP_TRANSFER_FILE,
	PROP_TRANSFER_SIZE,
	PROP_TRANSFER_POSITION,
	PROP_TRANSFER_PERCENTAGE,
	PROP_TRANSFER_SPEED,
	PROP_TRANSFER_ETA,
	PROP_TRANSFER_NICK,
	N_TRANSFER_PROPERTIES
};

static GParamSpec *transfer_properties[N_TRANSFER_PROPERTIES];

#define FABULOR_TYPE_DCC_TRANSFER_ROW (fabulor_dcc_transfer_row_get_type ())
#define FABULOR_DCC_TRANSFER_ROW(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), FABULOR_TYPE_DCC_TRANSFER_ROW, \
	 FabulorDccTransferRow))

G_DEFINE_TYPE (FabulorDccTransferRow, fabulor_dcc_transfer_row, G_TYPE_OBJECT)

static void
fabulor_dcc_transfer_row_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	FabulorDccTransferRow *row = FABULOR_DCC_TRANSFER_ROW (object);
	const gchar *text = NULL;

	switch (property_id)
	{
	case PROP_TRANSFER_STATUS: text = row->status; break;
	case PROP_TRANSFER_FILE: text = row->file; break;
	case PROP_TRANSFER_SIZE: text = row->size; break;
	case PROP_TRANSFER_POSITION: text = row->position; break;
	case PROP_TRANSFER_PERCENTAGE: text = row->percentage; break;
	case PROP_TRANSFER_SPEED: text = row->speed; break;
	case PROP_TRANSFER_ETA: text = row->eta; break;
	case PROP_TRANSFER_NICK: text = row->nick; break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		return;
	}
	g_value_set_string (value, text);
}

static void
fabulor_dcc_transfer_row_finalize (GObject *object)
{
	FabulorDccTransferRow *row = FABULOR_DCC_TRANSFER_ROW (object);

	g_free (row->status);
	g_free (row->file);
	g_free (row->size);
	g_free (row->position);
	g_free (row->percentage);
	g_free (row->speed);
	g_free (row->eta);
	g_free (row->nick);
	G_OBJECT_CLASS (fabulor_dcc_transfer_row_parent_class)->finalize (object);
}

static void
fabulor_dcc_transfer_row_class_init (FabulorDccTransferRowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	const gchar *names[] = { "status", "file", "size", "position",
		"percentage", "speed", "eta", "nick" };
	guint i;

	object_class->get_property = fabulor_dcc_transfer_row_get_property;
	object_class->finalize = fabulor_dcc_transfer_row_finalize;
	for (i = 0; i < G_N_ELEMENTS (names); i++)
		transfer_properties[i + 1] = g_param_spec_string (names[i], names[i],
			names[i], NULL, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties (object_class, N_TRANSFER_PROPERTIES,
		transfer_properties);
}

static void
fabulor_dcc_transfer_row_init (FabulorDccTransferRow *row)
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
dcc_transfer_row_assign (FabulorDccTransferRow *row,
	const FabulorDccTransferSnapshot *snapshot, gboolean notify)
{
	guint i;

	row->identity = snapshot->identity;
	row->upload = snapshot->upload;
	replace_string (&row->status, snapshot->status);
	replace_string (&row->file, snapshot->file);
	replace_string (&row->size, snapshot->size);
	replace_string (&row->position, snapshot->position);
	replace_string (&row->percentage, snapshot->percentage);
	replace_string (&row->speed, snapshot->speed);
	replace_string (&row->eta, snapshot->eta);
	replace_string (&row->nick, snapshot->nick);
	row->has_color = snapshot->has_color;
	row->color = snapshot->color;
	if (notify)
		for (i = 1; i < N_TRANSFER_PROPERTIES; i++)
			g_object_notify_by_pspec (G_OBJECT (row), transfer_properties[i]);
}

static FabulorDccTransferRow *
dcc_transfer_row_new (const FabulorDccTransferSnapshot *snapshot)
{
	FabulorDccTransferRow *row = g_object_new (
		FABULOR_TYPE_DCC_TRANSFER_ROW, NULL);
	dcc_transfer_row_assign (row, snapshot, FALSE);
	return row;
}

typedef struct
{
	FabulorDccTransferRow *row;
	GtkLabel *label;
	GBinding *text_binding;
	gulong notify_handler;
	const gchar *property;
} DccTransferCellBinding;

static void
dcc_transfer_cell_apply_color (DccTransferCellBinding *binding)
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
dcc_transfer_cell_changed (GObject *object, GParamSpec *pspec,
	gpointer user_data)
{
	(void) object;
	(void) pspec;
	dcc_transfer_cell_apply_color (user_data);
}

static void
dcc_transfer_cell_binding_clear (DccTransferCellBinding *binding)
{
	if (binding->row && binding->notify_handler)
		g_signal_handler_disconnect (binding->row, binding->notify_handler);
	binding->notify_handler = 0;
	g_clear_object (&binding->text_binding);
	g_clear_object (&binding->row);
}

static void
dcc_transfer_cell_binding_free (gpointer data)
{
	DccTransferCellBinding *binding = data;
	dcc_transfer_cell_binding_clear (binding);
	g_free (binding);
}

static void
dcc_transfer_text_setup (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	DccTransferCellBinding *binding = g_new0 (DccTransferCellBinding, 1);
	GtkWidget *label = gtk_label_new (NULL);

	(void) factory;
	binding->property = user_data;
	binding->label = GTK_LABEL (label);
	gtk_label_set_xalign (binding->label,
		g_str_equal (binding->property, "size") ||
		g_str_equal (binding->property, "position") ||
		g_str_equal (binding->property, "percentage") ||
		g_str_equal (binding->property, "speed") ? 1.0f : 0.0f);
	gtk_label_set_ellipsize (binding->label, PANGO_ELLIPSIZE_END);
	gtk_widget_set_hexpand (label, TRUE);
	gtk_list_item_set_child (item, label);
	g_object_set_data_full (G_OBJECT (item), "fabulor-dcc-transfer-cell",
		binding, dcc_transfer_cell_binding_free);
}

static void
dcc_transfer_text_bind (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	DccTransferCellBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-dcc-transfer-cell");

	(void) factory;
	(void) user_data;
	dcc_transfer_cell_binding_clear (binding);
	binding->row = g_object_ref (FABULOR_DCC_TRANSFER_ROW (
		gtk_list_item_get_item (item)));
	binding->text_binding = g_object_bind_property (binding->row,
		binding->property, binding->label, "label", G_BINDING_SYNC_CREATE);
	binding->notify_handler = g_signal_connect (binding->row, "notify",
		G_CALLBACK (dcc_transfer_cell_changed), binding);
	dcc_transfer_cell_apply_color (binding);
}

static void
dcc_transfer_text_unbind (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	DccTransferCellBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-dcc-transfer-cell");
	(void) factory;
	(void) user_data;
	dcc_transfer_cell_binding_clear (binding);
}

typedef struct
{
	FabulorDccTransferRow *row;
	GtkImage *image;
	gulong notify_handler;
} DccTransferIconBinding;

static void
dcc_transfer_icon_refresh (DccTransferIconBinding *binding)
{
	gtk_image_set_from_icon_name (binding->image,
		binding->row->upload ? "go-up" : "go-down");
}

static void
dcc_transfer_icon_changed (GObject *object, GParamSpec *pspec,
	gpointer user_data)
{
	(void) object;
	(void) pspec;
	dcc_transfer_icon_refresh (user_data);
}

static void
dcc_transfer_icon_binding_clear (DccTransferIconBinding *binding)
{
	if (binding->row && binding->notify_handler)
		g_signal_handler_disconnect (binding->row, binding->notify_handler);
	binding->notify_handler = 0;
	g_clear_object (&binding->row);
}

static void
dcc_transfer_icon_binding_free (gpointer data)
{
	DccTransferIconBinding *binding = data;
	dcc_transfer_icon_binding_clear (binding);
	g_free (binding);
}

static void
dcc_transfer_icon_setup (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	DccTransferIconBinding *binding = g_new0 (DccTransferIconBinding, 1);
	GtkWidget *image = gtk_image_new ();
	(void) factory;
	(void) user_data;
	binding->image = GTK_IMAGE (image);
	gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
	gtk_list_item_set_child (item, image);
	g_object_set_data_full (G_OBJECT (item), "fabulor-dcc-transfer-icon",
		binding, dcc_transfer_icon_binding_free);
}

static void
dcc_transfer_icon_bind (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	DccTransferIconBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-dcc-transfer-icon");
	(void) factory;
	(void) user_data;
	dcc_transfer_icon_binding_clear (binding);
	binding->row = g_object_ref (FABULOR_DCC_TRANSFER_ROW (
		gtk_list_item_get_item (item)));
	binding->notify_handler = g_signal_connect (binding->row, "notify",
		G_CALLBACK (dcc_transfer_icon_changed), binding);
	dcc_transfer_icon_refresh (binding);
}

static void
dcc_transfer_icon_unbind (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	DccTransferIconBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-dcc-transfer-icon");
	(void) factory;
	(void) user_data;
	dcc_transfer_icon_binding_clear (binding);
}

static GtkColumnViewColumn *
dcc_transfer_text_column_new (const gchar *title, const gchar *property,
	gboolean expand)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	GtkColumnViewColumn *column;

	g_signal_connect (factory, "setup", G_CALLBACK (dcc_transfer_text_setup),
		(gpointer) property);
	g_signal_connect (factory, "bind", G_CALLBACK (dcc_transfer_text_bind), NULL);
	g_signal_connect (factory, "unbind", G_CALLBACK (dcc_transfer_text_unbind),
		NULL);
	column = gtk_column_view_column_new (title, factory);
	gtk_column_view_column_set_expand (column, expand);
	gtk_column_view_column_set_resizable (column, TRUE);
	return column;
}

static void
dcc_transfer_selection_changed_cb (GtkSelectionModel *selection,
	guint position, guint n_items, gpointer user_data)
{
	(void) selection;
	(void) position;
	(void) n_items;
	dcc_transfer_selection_changed (user_data);
}

static void
dcc_transfer_activate_cb (GtkColumnView *view, guint position,
	gpointer user_data)
{
	FabulorDccTransferList *list = user_data;
	FabulorDccTransferRow *row = g_list_model_get_item (G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_sorted (list->models)), position);
	(void) view;
	if (row && list->activate_func)
		list->activate_func (row->identity, list->callback_data);
	g_clear_object (&row);
}


FabulorDccTransferList *
fabulor_dcc_transfer_list_new (
	FabulorDccTransferSelectionFunc selection_func,
	FabulorDccTransferActivateFunc activate_func, gpointer user_data)
{
	FabulorDccTransferList *list = g_new0 (FabulorDccTransferList, 1);
	list->selection_func = selection_func;
	list->activate_func = activate_func;
	list->callback_data = user_data;
	list->models = fabulor_gtk4_flat_model_stack_new (
		FABULOR_TYPE_DCC_TRANSFER_ROW, NULL, FABULOR_GTK4_SELECTION_MULTIPLE);
	if (!list->models)
	{
		g_free (list);
		return NULL;
	}
	list->rows = g_hash_table_new (g_direct_hash, g_direct_equal);
	g_signal_connect (fabulor_gtk4_flat_model_stack_get_selection (list->models),
		"selection-changed", G_CALLBACK (dcc_transfer_selection_changed_cb), list);
	return list;
}

void
fabulor_dcc_transfer_list_free (FabulorDccTransferList *list)
{
	if (!list)
		return;
	g_hash_table_unref (list->rows);
	fabulor_gtk4_flat_model_stack_free (list->models);
	g_free (list);
}

GtkWidget *
fabulor_dcc_transfer_list_create_view (FabulorDccTransferList *list,
	GtkBox *parent, const gchar *status_title, const gchar *file_title,
	const gchar *size_title, const gchar *position_title,
	const gchar *percentage_title, const gchar *speed_title,
	const gchar *eta_title, const gchar *nick_title)
{
	g_return_val_if_fail (list != NULL && GTK_IS_BOX (parent), NULL);
	g_return_val_if_fail (list->view == NULL, NULL);
	{
		GtkWidget *scroller = gtk_scrolled_window_new ();
		GtkSelectionModel *selection =
			fabulor_gtk4_flat_model_stack_get_selection (list->models);
		GtkListItemFactory *icon_factory = gtk_signal_list_item_factory_new ();
		GtkColumnViewColumn *column;
		const gchar *titles[] = { status_title, file_title, size_title,
			position_title, percentage_title, speed_title, eta_title, nick_title };
		const gchar *properties[] = { "status", "file", "size", "position",
			"percentage", "speed", "eta", "nick" };
		guint i;

		list->view = gtk_column_view_new (GTK_SELECTION_MODEL (
			g_object_ref (selection)));
		gtk_column_view_set_show_row_separators (GTK_COLUMN_VIEW (list->view),
			TRUE);
		g_signal_connect (icon_factory, "setup",
			G_CALLBACK (dcc_transfer_icon_setup), NULL);
		g_signal_connect (icon_factory, "bind",
			G_CALLBACK (dcc_transfer_icon_bind), NULL);
		g_signal_connect (icon_factory, "unbind",
			G_CALLBACK (dcc_transfer_icon_unbind), NULL);
		column = gtk_column_view_column_new (NULL, icon_factory);
		gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view), column);
		g_object_unref (column);
		for (i = 0; i < G_N_ELEMENTS (properties); i++)
		{
			column = dcc_transfer_text_column_new (titles[i], properties[i],
				i == 1 || i == 7);
			gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view), column);
			g_object_unref (column);
		}
		g_signal_connect (list->view, "activate",
			G_CALLBACK (dcc_transfer_activate_cb), list);
		fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroller),
			list->view);
		gtk_widget_set_hexpand (scroller, TRUE);
		gtk_widget_set_vexpand (scroller, TRUE);
		fabulor_gtk_box_append (parent, scroller, TRUE, TRUE, 0);
	}
	return list->view;
}

gboolean
fabulor_dcc_transfer_list_append (FabulorDccTransferList *list,
	const FabulorDccTransferSnapshot *snapshot, gboolean prepend)
{
	g_return_val_if_fail (list != NULL && snapshot != NULL &&
		snapshot->identity != NULL, FALSE);
	{
		FabulorDccTransferRow *row;
		GListStore *store;
		if (g_hash_table_contains (list->rows, snapshot->identity))
			return FALSE;
		row = dcc_transfer_row_new (snapshot);
		store = fabulor_gtk4_flat_model_stack_get_store (list->models);
		if (prepend)
			g_list_store_insert (store, 0, row);
		else
			g_list_store_append (store, row);
		g_hash_table_insert (list->rows, snapshot->identity, row);
		g_object_unref (row);
	}
	return TRUE;
}

gboolean
fabulor_dcc_transfer_list_update (FabulorDccTransferList *list,
	const FabulorDccTransferSnapshot *snapshot)
{
	g_return_val_if_fail (list != NULL && snapshot != NULL, FALSE);
	{
		FabulorDccTransferRow *row = g_hash_table_lookup (list->rows,
			snapshot->identity);
		if (!row)
			return FALSE;
		dcc_transfer_row_assign (row, snapshot, TRUE);
	}
	return TRUE;
}

gboolean
fabulor_dcc_transfer_list_remove (FabulorDccTransferList *list,
	gpointer identity)
{
	g_return_val_if_fail (list != NULL && identity != NULL, FALSE);
	{
		FabulorDccTransferRow *row = g_hash_table_lookup (list->rows, identity);
		if (!row)
			return FALSE;
		g_object_ref (row);
		g_hash_table_remove (list->rows, identity);
		fabulor_gtk4_flat_model_stack_remove (list->models, row);
		g_object_unref (row);
	}
	return TRUE;
}

void
fabulor_dcc_transfer_list_clear (FabulorDccTransferList *list)
{
	g_return_if_fail (list != NULL);
	g_hash_table_remove_all (list->rows);
	fabulor_gtk4_flat_model_stack_clear (list->models);
	dcc_transfer_selection_changed (list);
}

guint
fabulor_dcc_transfer_list_get_n_rows (FabulorDccTransferList *list)
{
	g_return_val_if_fail (list != NULL, 0);
	return g_list_model_get_n_items (G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_sorted (list->models)));
}

guint
fabulor_dcc_transfer_list_get_n_selected (FabulorDccTransferList *list)
{
	g_return_val_if_fail (list != NULL, 0);
	{
		GtkBitset *selected = gtk_selection_model_get_selection (
			fabulor_gtk4_flat_model_stack_get_selection (list->models));
		guint count = (guint) gtk_bitset_get_size (selected);
		gtk_bitset_unref (selected);
		return count;
	}
}

gboolean
fabulor_dcc_transfer_list_set_selected (FabulorDccTransferList *list,
	guint position, gboolean selected)
{
	g_return_val_if_fail (list != NULL, FALSE);
	if (position >= fabulor_dcc_transfer_list_get_n_rows (list))
		return FALSE;
	if (selected)
		return gtk_selection_model_select_item (
			fabulor_gtk4_flat_model_stack_get_selection (list->models), position,
			FALSE);
	return gtk_selection_model_unselect_item (
		fabulor_gtk4_flat_model_stack_get_selection (list->models), position);
}

gboolean
fabulor_dcc_transfer_list_select_first (FabulorDccTransferList *list)
{
	return fabulor_dcc_transfer_list_set_selected (list, 0, TRUE);
}

GPtrArray *
fabulor_dcc_transfer_list_dup_all (FabulorDccTransferList *list)
{
	GPtrArray *identities = g_ptr_array_new ();
	guint count;
	guint i;
	g_return_val_if_fail (list != NULL, identities);
	count = fabulor_dcc_transfer_list_get_n_rows (list);
	for (i = 0; i < count; i++)
	{
		FabulorDccTransferRow *row = g_list_model_get_item (G_LIST_MODEL (
			fabulor_gtk4_flat_model_stack_get_sorted (list->models)), i);
		g_ptr_array_add (identities, row->identity);
		g_object_unref (row);
	}
	return identities;
}

GPtrArray *
fabulor_dcc_transfer_list_dup_selected (FabulorDccTransferList *list)
{
	GPtrArray *identities = g_ptr_array_new ();
	guint count;
	guint i;
	g_return_val_if_fail (list != NULL, identities);
	count = fabulor_dcc_transfer_list_get_n_rows (list);
	for (i = 0; i < count; i++)
	{
		FabulorDccTransferRow *row;
		if (!gtk_selection_model_is_selected (
			fabulor_gtk4_flat_model_stack_get_selection (list->models), i))
			continue;
		row = g_list_model_get_item (G_LIST_MODEL (
			fabulor_gtk4_flat_model_stack_get_sorted (list->models)), i);
		g_ptr_array_add (identities, row->identity);
		g_object_unref (row);
	}
	return identities;
}

gpointer
fabulor_dcc_transfer_list_get_first_selected (FabulorDccTransferList *list)
{
	GPtrArray *selected;
	gpointer identity = NULL;
	g_return_val_if_fail (list != NULL, NULL);
	selected = fabulor_dcc_transfer_list_dup_selected (list);
	if (selected->len)
		identity = g_ptr_array_index (selected, 0);
	g_ptr_array_unref (selected);
	return identity;
}
