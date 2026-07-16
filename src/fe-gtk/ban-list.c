/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ban-list.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gtk-compat.h"

#if GTK_MAJOR_VERSION >= 4
#include "gtk4-list-models.h"
#else
#include "gtkutil.h"
#endif

struct _FabulorBanList
{
	GtkWidget *view;
	FabulorBanListSelectionFunc selection_func;
	gpointer callback_data;
#if GTK_MAJOR_VERSION >= 4
	FabulorGtk4FlatModelStack *models;
#else
	GtkListStore *store;
#endif
};

static gint64
ban_list_parse_date (const gchar *text)
{
	static const gchar *months[] = { "Jan", "Feb", "Mar", "Apr", "May",
		"Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	struct tm value = { 0 };
	const gchar *cursor;
	gchar *end;
	guint month;

	if (!text || strlen (text) < 20)
		return 0;
	cursor = text + 4;
	for (month = 0; month < G_N_ELEMENTS (months); month++)
		if (strncmp (cursor, months[month], 3) == 0)
			break;
	if (month == G_N_ELEMENTS (months))
		return 0;
	cursor += 4;
	value.tm_mday = (gint) strtol (cursor, &end, 10);
	if (*end != ' ')
		return 0;
	value.tm_hour = (gint) strtol (end + 1, &end, 10);
	if (*end != ':')
		return 0;
	value.tm_min = (gint) strtol (end + 1, &end, 10);
	if (*end != ':')
		return 0;
	value.tm_sec = (gint) strtol (end + 1, &end, 10);
	if (*end != ' ')
		return 0;
	value.tm_year = (gint) strtol (end + 1, &end, 10) - 1900;
	value.tm_mon = (gint) month;
	value.tm_isdst = -1;
	return (gint64) mktime (&value);
}

static void
ban_list_selection_changed (FabulorBanList *list)
{
	if (list->selection_func)
		list->selection_func (fabulor_ban_list_get_n_selected (list),
			list->callback_data);
}

#if GTK_MAJOR_VERSION >= 4

typedef struct _FabulorBanRow FabulorBanRow;
typedef struct _FabulorBanRowClass FabulorBanRowClass;

struct _FabulorBanRow
{
	GObject parent_instance;
	guint mode;
	gchar *type;
	gchar *mask;
	gchar *from;
	gchar *date;
	gint64 timestamp;
};

struct _FabulorBanRowClass
{
	GObjectClass parent_class;
};

enum
{
	PROP_BAN_ROW_0,
	PROP_BAN_ROW_TYPE,
	PROP_BAN_ROW_MASK,
	PROP_BAN_ROW_FROM,
	PROP_BAN_ROW_DATE,
	PROP_BAN_ROW_TIMESTAMP,
	N_BAN_ROW_PROPERTIES
};

static GParamSpec *ban_row_properties[N_BAN_ROW_PROPERTIES];

#define FABULOR_TYPE_BAN_ROW (fabulor_ban_row_get_type ())
#define FABULOR_BAN_ROW(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), FABULOR_TYPE_BAN_ROW, FabulorBanRow))

G_DEFINE_TYPE (FabulorBanRow, fabulor_ban_row, G_TYPE_OBJECT)

static void
fabulor_ban_row_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	FabulorBanRow *row = FABULOR_BAN_ROW (object);

	switch (property_id)
	{
	case PROP_BAN_ROW_TYPE: g_value_set_string (value, row->type); break;
	case PROP_BAN_ROW_MASK: g_value_set_string (value, row->mask); break;
	case PROP_BAN_ROW_FROM: g_value_set_string (value, row->from); break;
	case PROP_BAN_ROW_DATE: g_value_set_string (value, row->date); break;
	case PROP_BAN_ROW_TIMESTAMP: g_value_set_int64 (value, row->timestamp); break;
	default: G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
fabulor_ban_row_finalize (GObject *object)
{
	FabulorBanRow *row = FABULOR_BAN_ROW (object);

	g_free (row->type);
	g_free (row->mask);
	g_free (row->from);
	g_free (row->date);
	G_OBJECT_CLASS (fabulor_ban_row_parent_class)->finalize (object);
}

static void
fabulor_ban_row_class_init (FabulorBanRowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = fabulor_ban_row_get_property;
	object_class->finalize = fabulor_ban_row_finalize;
	ban_row_properties[PROP_BAN_ROW_TYPE] = g_param_spec_string (
		"type", "Type", "Mode type", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	ban_row_properties[PROP_BAN_ROW_MASK] = g_param_spec_string (
		"mask", "Mask", "Mode mask", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	ban_row_properties[PROP_BAN_ROW_FROM] = g_param_spec_string (
		"from", "From", "Mode setter", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	ban_row_properties[PROP_BAN_ROW_DATE] = g_param_spec_string (
		"date", "Date", "Mode date", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	ban_row_properties[PROP_BAN_ROW_TIMESTAMP] = g_param_spec_int64 (
		"timestamp", "Timestamp", "Sortable mode date", G_MININT64,
		G_MAXINT64, 0, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties (object_class, N_BAN_ROW_PROPERTIES,
		ban_row_properties);
}

static void
fabulor_ban_row_init (FabulorBanRow *row)
{
	(void) row;
}

static FabulorBanRow *
ban_row_new (guint mode, const gchar *type, const gchar *mask,
	const gchar *from, const gchar *date)
{
	FabulorBanRow *row = g_object_new (FABULOR_TYPE_BAN_ROW, NULL);

	row->mode = mode;
	row->type = g_strdup (type);
	row->mask = g_strdup (mask);
	row->from = g_strdup (from ? from : "");
	row->date = g_strdup (date ? date : "");
	row->timestamp = ban_list_parse_date (date);
	return row;
}

static GQuark
ban_list_item_quark (void)
{
	return g_quark_from_static_string ("fabulor-ban-list-item");
}

typedef enum
{
	BAN_FIELD_TYPE,
	BAN_FIELD_MASK,
	BAN_FIELD_FROM,
	BAN_FIELD_DATE
} BanField;

static const gchar *
ban_row_field (FabulorBanRow *row, BanField field)
{
	switch (field)
	{
	case BAN_FIELD_TYPE: return row->type;
	case BAN_FIELD_MASK: return row->mask;
	case BAN_FIELD_FROM: return row->from;
	case BAN_FIELD_DATE: return row->date;
	default: return "";
	}
}

static void
ban_factory_setup (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	GtkWidget *label = gtk_label_new (NULL);

	(void) factory;
	(void) user_data;
	gtk_label_set_xalign (GTK_LABEL (label), 0.0f);
	gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
	gtk_widget_set_hexpand (label, TRUE);
	gtk_list_item_set_child (item, label);
}

static void
ban_factory_bind (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	FabulorBanRow *row = FABULOR_BAN_ROW (gtk_list_item_get_item (item));
	GtkWidget *label = gtk_list_item_get_child (item);

	(void) factory;
	gtk_label_set_text (GTK_LABEL (label), ban_row_field (row,
		(BanField) GPOINTER_TO_UINT (user_data)));
	g_object_set_qdata (G_OBJECT (label), ban_list_item_quark (), item);
}

static void
ban_factory_unbind (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	(void) factory;
	(void) user_data;
	g_object_set_qdata (G_OBJECT (gtk_list_item_get_child (item)),
		ban_list_item_quark (), NULL);
}

static GtkColumnViewColumn *
ban_column_new (const gchar *title, BanField field, const gchar *property,
	gboolean expand)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	GtkColumnViewColumn *column;
	GtkExpression *expression;
	GtkSorter *sorter;

	g_signal_connect (factory, "setup", G_CALLBACK (ban_factory_setup), NULL);
	g_signal_connect (factory, "bind", G_CALLBACK (ban_factory_bind),
		GUINT_TO_POINTER (field));
	g_signal_connect (factory, "unbind", G_CALLBACK (ban_factory_unbind), NULL);
	column = gtk_column_view_column_new (title, factory);
	gtk_column_view_column_set_expand (column, expand);
	gtk_column_view_column_set_resizable (column, TRUE);
	expression = gtk_property_expression_new (FABULOR_TYPE_BAN_ROW, NULL,
		property);
	if (field == BAN_FIELD_DATE)
		sorter = GTK_SORTER (gtk_numeric_sorter_new (expression));
	else
		sorter = GTK_SORTER (gtk_string_sorter_new (expression));
	gtk_column_view_column_set_sorter (column, sorter);
	g_object_unref (sorter);
	return column;
}

static gboolean
ban_list_position_at_point (FabulorBanList *list, gdouble x, gdouble y,
	guint *position)
{
	GtkWidget *picked = gtk_widget_pick (list->view, x, y, GTK_PICK_DEFAULT);

	while (picked && picked != list->view)
	{
		GtkListItem *item = g_object_get_qdata (G_OBJECT (picked),
			ban_list_item_quark ());
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
ban_selection_changed_cb (GtkSelectionModel *selection, guint position,
	guint n_items, gpointer user_data)
{
	(void) selection;
	(void) position;
	(void) n_items;
	ban_list_selection_changed (user_data);
}

#else

enum
{
	BAN_COLUMN_TYPE,
	BAN_COLUMN_MASK,
	BAN_COLUMN_FROM,
	BAN_COLUMN_DATE,
	BAN_COLUMN_MODE,
	BAN_COLUMN_TIMESTAMP,
	N_BAN_COLUMNS
};

static void
ban_selection_changed_cb (GtkTreeSelection *selection, gpointer user_data)
{
	(void) selection;
	ban_list_selection_changed (user_data);
}

#endif

FabulorBanList *
fabulor_ban_list_new (FabulorBanListSelectionFunc selection_func,
	gpointer user_data)
{
	FabulorBanList *list = g_new0 (FabulorBanList, 1);

	list->selection_func = selection_func;
	list->callback_data = user_data;
#if GTK_MAJOR_VERSION >= 4
	list->models = fabulor_gtk4_flat_model_stack_new (FABULOR_TYPE_BAN_ROW,
		NULL, FABULOR_GTK4_SELECTION_MULTIPLE);
	if (!list->models)
	{
		g_free (list);
		return NULL;
	}
	g_signal_connect (fabulor_gtk4_flat_model_stack_get_selection (list->models),
		"selection-changed", G_CALLBACK (ban_selection_changed_cb), list);
#else
	list->store = gtk_list_store_new (N_BAN_COLUMNS, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_INT64);
#endif
	return list;
}

void
fabulor_ban_list_free (FabulorBanList *list)
{
	if (!list)
		return;
#if GTK_MAJOR_VERSION >= 4
	fabulor_gtk4_flat_model_stack_free (list->models);
#else
	g_clear_object (&list->store);
#endif
	g_free (list);
}

GtkWidget *
fabulor_ban_list_create_view (FabulorBanList *list, GtkBox *parent,
	const gchar *type_title, const gchar *mask_title, const gchar *from_title,
	const gchar *date_title)
{
	g_return_val_if_fail (list != NULL, NULL);
	g_return_val_if_fail (GTK_IS_BOX (parent), NULL);
	g_return_val_if_fail (list->view == NULL, NULL);
#if GTK_MAJOR_VERSION >= 4
	{
		GtkWidget *scroller = gtk_scrolled_window_new ();
		GtkSelectionModel *selection =
			fabulor_gtk4_flat_model_stack_get_selection (list->models);
		GtkColumnViewColumn *column;

		list->view = gtk_column_view_new (GTK_SELECTION_MODEL (
			g_object_ref (selection)));
		column = ban_column_new (type_title, BAN_FIELD_TYPE, "type", FALSE);
		gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view), column);
		g_object_unref (column);
		column = ban_column_new (mask_title, BAN_FIELD_MASK, "mask", TRUE);
		gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view), column);
		g_object_unref (column);
		column = ban_column_new (from_title, BAN_FIELD_FROM, "from", TRUE);
		gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view), column);
		g_object_unref (column);
		column = ban_column_new (date_title, BAN_FIELD_DATE, "timestamp", TRUE);
		gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view), column);
		g_object_unref (column);
		gtk_sort_list_model_set_sorter (
			fabulor_gtk4_flat_model_stack_get_sorted (list->models),
			gtk_column_view_get_sorter (GTK_COLUMN_VIEW (list->view)));
		fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroller),
			list->view);
		gtk_widget_set_hexpand (scroller, TRUE);
		gtk_widget_set_vexpand (scroller, TRUE);
		fabulor_gtk_box_append (parent, scroller, TRUE, TRUE, 0);
	}
#else
	{
		GtkTreeSelection *selection;
		GtkTreeViewColumn *column;
		GtkTreeSortable *sortable = GTK_TREE_SORTABLE (list->store);

		gtk_tree_sortable_set_sort_column_id (sortable, BAN_COLUMN_TIMESTAMP,
			GTK_SORT_DESCENDING);
		list->view = gtkutil_treeview_new (parent,
			GTK_TREE_MODEL (g_object_ref (list->store)), NULL,
			BAN_COLUMN_TYPE, (gchar *) type_title,
			BAN_COLUMN_MASK, (gchar *) mask_title,
			BAN_COLUMN_FROM, (gchar *) from_title,
			BAN_COLUMN_DATE, (gchar *) date_title, -1);
		for (gint i = 0; i < 4; i++)
		{
			column = gtk_tree_view_get_column (GTK_TREE_VIEW (list->view), i);
			gtk_tree_view_column_set_sort_column_id (column,
				i == BAN_COLUMN_DATE ? BAN_COLUMN_TIMESTAMP : i);
			gtk_tree_view_column_set_resizable (column, TRUE);
		}
		gtk_tree_view_column_set_min_width (gtk_tree_view_get_column (
			GTK_TREE_VIEW (list->view), BAN_COLUMN_MASK), 100);
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (list->view));
		gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
		g_signal_connect (selection, "changed",
			G_CALLBACK (ban_selection_changed_cb), list);
		gtk_widget_show (list->view);
	}
#endif
	return list->view;
}

void
fabulor_ban_list_append (FabulorBanList *list, guint mode, const gchar *type,
	const gchar *mask, const gchar *from, const gchar *date)
{
	g_return_if_fail (list != NULL);
	g_return_if_fail (type != NULL && mask != NULL);
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorBanRow *row = ban_row_new (mode, type, mask, from, date);
		fabulor_gtk4_flat_model_stack_append (list->models, row);
		g_object_unref (row);
	}
#else
	{
		GtkTreeIter iter;
		gtk_list_store_append (list->store, &iter);
		gtk_list_store_set (list->store, &iter,
			BAN_COLUMN_TYPE, type, BAN_COLUMN_MASK, mask,
			BAN_COLUMN_FROM, from, BAN_COLUMN_DATE, date,
			BAN_COLUMN_MODE, mode,
			BAN_COLUMN_TIMESTAMP, ban_list_parse_date (date), -1);
	}
#endif
}

void
fabulor_ban_list_clear (FabulorBanList *list)
{
	g_return_if_fail (list != NULL);
#if GTK_MAJOR_VERSION >= 4
	fabulor_gtk4_flat_model_stack_clear (list->models);
#else
	gtk_list_store_clear (list->store);
#endif
	ban_list_selection_changed (list);
}

guint
fabulor_ban_list_get_n_rows (FabulorBanList *list)
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
fabulor_ban_list_get_n_selected (FabulorBanList *list)
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
	{
		GList *rows;
		guint count;
		if (!list->view)
			return 0;
		rows = gtk_tree_selection_get_selected_rows (gtk_tree_view_get_selection (
			GTK_TREE_VIEW (list->view)), NULL);
		count = g_list_length (rows);
		g_list_free_full (rows, (GDestroyNotify) gtk_tree_path_free);
		return count;
	}
#endif
}

void
fabulor_ban_list_select_all (FabulorBanList *list)
{
	g_return_if_fail (list != NULL);
#if GTK_MAJOR_VERSION >= 4
	gtk_selection_model_select_all (
		fabulor_gtk4_flat_model_stack_get_selection (list->models));
#else
	if (list->view)
		gtk_tree_selection_select_all (gtk_tree_view_get_selection (
			GTK_TREE_VIEW (list->view)));
#endif
}

gboolean
fabulor_ban_list_set_selected (FabulorBanList *list, guint position,
	gboolean selected)
{
	g_return_val_if_fail (list != NULL, FALSE);
	if (position >= fabulor_ban_list_get_n_rows (list))
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

void
fabulor_ban_list_invert_selection (FabulorBanList *list)
{
	guint count;
	guint i;

	g_return_if_fail (list != NULL);
	count = fabulor_ban_list_get_n_rows (list);
#if GTK_MAJOR_VERSION >= 4
	{
		GtkSelectionModel *selection =
			fabulor_gtk4_flat_model_stack_get_selection (list->models);
		for (i = 0; i < count; i++)
			if (gtk_selection_model_is_selected (selection, i))
				gtk_selection_model_unselect_item (selection, i);
			else
				gtk_selection_model_select_item (selection, i, FALSE);
	}
#else
	if (list->view)
	{
		GtkTreeSelection *selection = gtk_tree_view_get_selection (
			GTK_TREE_VIEW (list->view));
		GtkTreeIter iter;
		for (i = 0; gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store),
			&iter, NULL, (gint) i); i++)
			if (gtk_tree_selection_iter_is_selected (selection, &iter))
				gtk_tree_selection_unselect_iter (selection, &iter);
			else
				gtk_tree_selection_select_iter (selection, &iter);
	}
#endif
}

GPtrArray *
fabulor_ban_list_dup_masks (FabulorBanList *list, guint mode,
	gboolean selected)
{
	GPtrArray *masks = g_ptr_array_new_with_free_func (g_free);
	guint count;
	guint i;

	g_return_val_if_fail (list != NULL, masks);
	count = fabulor_ban_list_get_n_rows (list);
#if GTK_MAJOR_VERSION >= 4
	for (i = 0; i < count; i++)
	{
		GtkSelectionModel *selection =
			fabulor_gtk4_flat_model_stack_get_selection (list->models);
		FabulorBanRow *row;
		if (gtk_selection_model_is_selected (selection, i) != selected)
			continue;
		row = g_list_model_get_item (G_LIST_MODEL (
			fabulor_gtk4_flat_model_stack_get_sorted (list->models)), i);
		if (row->mode == mode)
			g_ptr_array_add (masks, g_strdup (row->mask));
		g_object_unref (row);
	}
#else
	if (list->view)
	{
		GtkTreeSelection *selection = gtk_tree_view_get_selection (
			GTK_TREE_VIEW (list->view));
		GtkTreeIter iter;
		for (i = 0; gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store),
			&iter, NULL, (gint) i); i++)
		{
			guint row_mode;
			gchar *mask;
			if (gtk_tree_selection_iter_is_selected (selection, &iter) != selected)
				continue;
			gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
				BAN_COLUMN_MODE, &row_mode, BAN_COLUMN_MASK, &mask, -1);
			if (row_mode == mode)
				g_ptr_array_add (masks, mask);
			else
				g_free (mask);
		}
	}
#endif
	return masks;
}

gboolean
fabulor_ban_list_select_at_point (FabulorBanList *list, gdouble x, gdouble y)
{
	g_return_val_if_fail (list != NULL && list->view != NULL, FALSE);
#if GTK_MAJOR_VERSION >= 4
	{
		guint position;
		if (!ban_list_position_at_point (list, x, y, &position))
			return FALSE;
		gtk_selection_model_select_item (
			fabulor_gtk4_flat_model_stack_get_selection (list->models), position,
			TRUE);
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
		gtk_tree_selection_unselect_all (selection);
		gtk_tree_selection_select_path (selection, path);
		gtk_tree_path_free (path);
		return TRUE;
	}
#endif
}

static gboolean
ban_list_dup_selected_fields (FabulorBanList *list, gchar **mask,
	gchar **from, gchar **date)
{
#if GTK_MAJOR_VERSION >= 4
	GtkSelectionModel *selection =
		fabulor_gtk4_flat_model_stack_get_selection (list->models);
	GListModel *model = G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_sorted (list->models));
	guint count = g_list_model_get_n_items (model);
	guint i;
	for (i = 0; i < count; i++)
		if (gtk_selection_model_is_selected (selection, i))
		{
			FabulorBanRow *row = g_list_model_get_item (model, i);
			*mask = g_strdup (row->mask);
			*from = g_strdup (row->from);
			*date = g_strdup (row->date);
			g_object_unref (row);
			return TRUE;
		}
#else
	if (list->view)
	{
		GtkTreeSelection *selection = gtk_tree_view_get_selection (
			GTK_TREE_VIEW (list->view));
		GList *rows = gtk_tree_selection_get_selected_rows (selection, NULL);
		if (rows)
		{
			GtkTreeIter iter;
			gboolean found = gtk_tree_model_get_iter (GTK_TREE_MODEL (list->store),
				&iter, rows->data);
			g_list_free_full (rows, (GDestroyNotify) gtk_tree_path_free);
			if (found)
			{
				gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
					BAN_COLUMN_MASK, mask, BAN_COLUMN_FROM, from,
					BAN_COLUMN_DATE, date, -1);
				return TRUE;
			}
		}
	}
#endif
	return FALSE;
}

gchar *
fabulor_ban_list_dup_selected_mask (FabulorBanList *list)
{
	gchar *mask = NULL;
	gchar *from = NULL;
	gchar *date = NULL;

	g_return_val_if_fail (list != NULL, NULL);
	ban_list_dup_selected_fields (list, &mask, &from, &date);
	g_free (from);
	g_free (date);
	return mask;
}

gchar *
fabulor_ban_list_dup_selected_entry (FabulorBanList *list,
	const gchar *format)
{
	gchar *mask = NULL;
	gchar *from = NULL;
	gchar *date = NULL;
	gchar *entry = NULL;

	g_return_val_if_fail (list != NULL && format != NULL, NULL);
	if (ban_list_dup_selected_fields (list, &mask, &from, &date))
		entry = g_strdup_printf (format, mask, date, from);
	g_free (mask);
	g_free (from);
	g_free (date);
	return entry;
}
