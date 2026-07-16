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

#if GTK_MAJOR_VERSION >= 4
#include "gtk4-list-models.h"
#else
#include "gtkutil.h"
#include "theme/theme-gtk.h"
#endif

struct _FabulorDccTransferList
{
	GtkWidget *view;
	FabulorDccTransferSelectionFunc selection_func;
	FabulorDccTransferActivateFunc activate_func;
	gpointer callback_data;
#if GTK_MAJOR_VERSION >= 4
	FabulorGtk4FlatModelStack *models;
	GHashTable *rows;
#else
	GtkListStore *store;
#endif
};

static void
dcc_transfer_selection_changed (FabulorDccTransferList *list)
{
	if (list->selection_func)
		list->selection_func (list->callback_data);
}

#if GTK_MAJOR_VERSION >= 4

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

#else

enum
{
	TRANSFER_COLUMN_DIRECTION,
	TRANSFER_COLUMN_STATUS,
	TRANSFER_COLUMN_FILE,
	TRANSFER_COLUMN_SIZE,
	TRANSFER_COLUMN_POSITION,
	TRANSFER_COLUMN_PERCENTAGE,
	TRANSFER_COLUMN_SPEED,
	TRANSFER_COLUMN_ETA,
	TRANSFER_COLUMN_NICK,
	TRANSFER_COLUMN_IDENTITY,
	TRANSFER_COLUMN_COLOR,
	N_TRANSFER_COLUMNS
};

static gboolean
dcc_transfer_find_iter (FabulorDccTransferList *list, gpointer identity,
	GtkTreeIter *iter)
{
	gpointer row_identity;
	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list->store), iter))
		return FALSE;
	do
	{
		gtk_tree_model_get (GTK_TREE_MODEL (list->store), iter,
			TRANSFER_COLUMN_IDENTITY, &row_identity, -1);
		if (row_identity == identity)
			return TRUE;
	} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list->store), iter));
	return FALSE;
}

static void
dcc_transfer_gtk3_set (FabulorDccTransferList *list, GtkTreeIter *iter,
	const FabulorDccTransferSnapshot *snapshot)
{
	const GdkRGBA *color = snapshot->has_color ? &snapshot->color : NULL;
	gtk_list_store_set (list->store, iter,
		TRANSFER_COLUMN_DIRECTION, snapshot->upload ? "go-up" : "go-down",
		TRANSFER_COLUMN_STATUS, snapshot->status,
		TRANSFER_COLUMN_FILE, snapshot->file,
		TRANSFER_COLUMN_SIZE, snapshot->size,
		TRANSFER_COLUMN_POSITION, snapshot->position,
		TRANSFER_COLUMN_PERCENTAGE, snapshot->percentage,
		TRANSFER_COLUMN_SPEED, snapshot->speed,
		TRANSFER_COLUMN_ETA, snapshot->eta,
		TRANSFER_COLUMN_NICK, snapshot->nick,
		TRANSFER_COLUMN_IDENTITY, snapshot->identity,
		TRANSFER_COLUMN_COLOR, color, -1);
}

static void
dcc_transfer_gtk3_add_column (GtkWidget *view, gint text_column,
	const gchar *title, gboolean right_justified)
{
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
	if (right_justified)
		g_object_set (renderer, "xalign", 1.0f, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view), -1,
		title, renderer, "text", text_column, THEME_GTK_FOREGROUND_PROPERTY,
		TRANSFER_COLUMN_COLOR, NULL);
	gtk_cell_renderer_text_set_fixed_height_from_font (
		GTK_CELL_RENDERER_TEXT (renderer), 1);
}

static void
dcc_transfer_selection_changed_cb (GtkTreeSelection *selection,
	gpointer user_data)
{
	(void) selection;
	dcc_transfer_selection_changed (user_data);
}

static void
dcc_transfer_activate_cb (GtkTreeView *view, GtkTreePath *path,
	GtkTreeViewColumn *column, gpointer user_data)
{
	FabulorDccTransferList *list = user_data;
	GtkTreeIter iter;
	gpointer identity = NULL;
	(void) view;
	(void) column;
	if (gtk_tree_model_get_iter (GTK_TREE_MODEL (list->store), &iter, path))
		gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
			TRANSFER_COLUMN_IDENTITY, &identity, -1);
	if (identity && list->activate_func)
		list->activate_func (identity, list->callback_data);
}

#endif

FabulorDccTransferList *
fabulor_dcc_transfer_list_new (
	FabulorDccTransferSelectionFunc selection_func,
	FabulorDccTransferActivateFunc activate_func, gpointer user_data)
{
	FabulorDccTransferList *list = g_new0 (FabulorDccTransferList, 1);
	list->selection_func = selection_func;
	list->activate_func = activate_func;
	list->callback_data = user_data;
#if GTK_MAJOR_VERSION >= 4
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
#else
	list->store = gtk_list_store_new (N_TRANSFER_COLUMNS, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_POINTER, THEME_GTK_COLOR_TYPE);
#endif
	return list;
}

void
fabulor_dcc_transfer_list_free (FabulorDccTransferList *list)
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
fabulor_dcc_transfer_list_create_view (FabulorDccTransferList *list,
	GtkBox *parent, const gchar *status_title, const gchar *file_title,
	const gchar *size_title, const gchar *position_title,
	const gchar *percentage_title, const gchar *speed_title,
	const gchar *eta_title, const gchar *nick_title)
{
	g_return_val_if_fail (list != NULL && GTK_IS_BOX (parent), NULL);
	g_return_val_if_fail (list->view == NULL, NULL);
#if GTK_MAJOR_VERSION >= 4
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
#else
	{
		GtkTreeSelection *selection;
		list->view = gtkutil_treeview_new (parent,
			GTK_TREE_MODEL (g_object_ref (list->store)), NULL, -1);
		gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (list->view),
			GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);
		gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (list->view),
			-1, NULL, gtk_cell_renderer_pixbuf_new (), "icon-name",
			TRANSFER_COLUMN_DIRECTION, NULL);
		dcc_transfer_gtk3_add_column (list->view, TRANSFER_COLUMN_STATUS,
			status_title, FALSE);
		dcc_transfer_gtk3_add_column (list->view, TRANSFER_COLUMN_FILE,
			file_title, FALSE);
		dcc_transfer_gtk3_add_column (list->view, TRANSFER_COLUMN_SIZE,
			size_title, TRUE);
		dcc_transfer_gtk3_add_column (list->view, TRANSFER_COLUMN_POSITION,
			position_title, TRUE);
		dcc_transfer_gtk3_add_column (list->view, TRANSFER_COLUMN_PERCENTAGE,
			percentage_title, TRUE);
		dcc_transfer_gtk3_add_column (list->view, TRANSFER_COLUMN_SPEED,
			speed_title, TRUE);
		dcc_transfer_gtk3_add_column (list->view, TRANSFER_COLUMN_ETA,
			eta_title, FALSE);
		dcc_transfer_gtk3_add_column (list->view, TRANSFER_COLUMN_NICK,
			nick_title, FALSE);
		gtk_tree_view_column_set_expand (gtk_tree_view_get_column (
			GTK_TREE_VIEW (list->view), 2), TRUE);
		gtk_tree_view_column_set_expand (gtk_tree_view_get_column (
			GTK_TREE_VIEW (list->view), 8), TRUE);
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (list->view));
		gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
		g_signal_connect (selection, "changed",
			G_CALLBACK (dcc_transfer_selection_changed_cb), list);
		g_signal_connect (list->view, "row-activated",
			G_CALLBACK (dcc_transfer_activate_cb), list);
		gtk_widget_show (list->view);
	}
#endif
	return list->view;
}

gboolean
fabulor_dcc_transfer_list_append (FabulorDccTransferList *list,
	const FabulorDccTransferSnapshot *snapshot, gboolean prepend)
{
	g_return_val_if_fail (list != NULL && snapshot != NULL &&
		snapshot->identity != NULL, FALSE);
#if GTK_MAJOR_VERSION >= 4
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
#else
	{
		GtkTreeIter iter;
		if (dcc_transfer_find_iter (list, snapshot->identity, &iter))
			return FALSE;
		if (prepend)
			gtk_list_store_prepend (list->store, &iter);
		else
			gtk_list_store_append (list->store, &iter);
		dcc_transfer_gtk3_set (list, &iter, snapshot);
	}
#endif
	return TRUE;
}

gboolean
fabulor_dcc_transfer_list_update (FabulorDccTransferList *list,
	const FabulorDccTransferSnapshot *snapshot)
{
	g_return_val_if_fail (list != NULL && snapshot != NULL, FALSE);
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorDccTransferRow *row = g_hash_table_lookup (list->rows,
			snapshot->identity);
		if (!row)
			return FALSE;
		dcc_transfer_row_assign (row, snapshot, TRUE);
	}
#else
	{
		GtkTreeIter iter;
		if (!dcc_transfer_find_iter (list, snapshot->identity, &iter))
			return FALSE;
		dcc_transfer_gtk3_set (list, &iter, snapshot);
	}
#endif
	return TRUE;
}

gboolean
fabulor_dcc_transfer_list_remove (FabulorDccTransferList *list,
	gpointer identity)
{
	g_return_val_if_fail (list != NULL && identity != NULL, FALSE);
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorDccTransferRow *row = g_hash_table_lookup (list->rows, identity);
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
		if (!dcc_transfer_find_iter (list, identity, &iter))
			return FALSE;
		gtk_list_store_remove (list->store, &iter);
	}
#endif
	return TRUE;
}

void
fabulor_dcc_transfer_list_clear (FabulorDccTransferList *list)
{
	g_return_if_fail (list != NULL);
#if GTK_MAJOR_VERSION >= 4
	g_hash_table_remove_all (list->rows);
	fabulor_gtk4_flat_model_stack_clear (list->models);
#else
	gtk_list_store_clear (list->store);
#endif
	dcc_transfer_selection_changed (list);
}

guint
fabulor_dcc_transfer_list_get_n_rows (FabulorDccTransferList *list)
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
fabulor_dcc_transfer_list_get_n_selected (FabulorDccTransferList *list)
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
fabulor_dcc_transfer_list_set_selected (FabulorDccTransferList *list,
	guint position, gboolean selected)
{
	g_return_val_if_fail (list != NULL, FALSE);
	if (position >= fabulor_dcc_transfer_list_get_n_rows (list))
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
#if GTK_MAJOR_VERSION >= 4
	for (i = 0; i < count; i++)
	{
		FabulorDccTransferRow *row = g_list_model_get_item (G_LIST_MODEL (
			fabulor_gtk4_flat_model_stack_get_sorted (list->models)), i);
		g_ptr_array_add (identities, row->identity);
		g_object_unref (row);
	}
#else
	for (i = 0; i < count; i++)
	{
		GtkTreeIter iter;
		gpointer identity;
		if (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store), &iter,
			NULL, (gint) i))
		{
			gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
				TRANSFER_COLUMN_IDENTITY, &identity, -1);
			g_ptr_array_add (identities, identity);
		}
	}
#endif
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
#if GTK_MAJOR_VERSION >= 4
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
				TRANSFER_COLUMN_IDENTITY, &identity, -1);
			g_ptr_array_add (identities, identity);
		}
	}
#endif
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
