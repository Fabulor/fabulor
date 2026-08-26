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

#include "gtk4-list-models.h"

struct _FabulorBanList
{
	GtkWidget *view;
	FabulorBanListSelectionFunc selection_func;
	gpointer callback_data;
	FabulorGtk4FlatModelStack *models;
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


FabulorBanList *
fabulor_ban_list_new (FabulorBanListSelectionFunc selection_func,
	gpointer user_data)
{
	FabulorBanList *list = g_new0 (FabulorBanList, 1);

	list->selection_func = selection_func;
	list->callback_data = user_data;
	list->models = fabulor_gtk4_flat_model_stack_new (FABULOR_TYPE_BAN_ROW,
		NULL, FABULOR_GTK4_SELECTION_MULTIPLE);
	if (!list->models)
	{
		g_free (list);
		return NULL;
	}
	g_signal_connect (fabulor_gtk4_flat_model_stack_get_selection (list->models),
		"selection-changed", G_CALLBACK (ban_selection_changed_cb), list);
	return list;
}

void
fabulor_ban_list_free (FabulorBanList *list)
{
	if (!list)
		return;
	fabulor_gtk4_flat_model_stack_free (list->models);
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
	{
		GtkWidget *scroller = gtk_scrolled_window_new ();
		GtkSelectionModel *selection =
			fabulor_gtk4_flat_model_stack_get_selection (list->models);
		GtkColumnViewColumn *column;

		gtk_scrolled_window_set_propagate_natural_width (
			GTK_SCROLLED_WINDOW (scroller), FALSE);
		gtk_scrolled_window_set_propagate_natural_height (
			GTK_SCROLLED_WINDOW (scroller), FALSE);
		gtk_widget_set_size_request (scroller, 1, 1);
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
	return list->view;
}

void
fabulor_ban_list_append (FabulorBanList *list, guint mode, const gchar *type,
	const gchar *mask, const gchar *from, const gchar *date)
{
	g_return_if_fail (list != NULL);
	g_return_if_fail (type != NULL && mask != NULL);
	{
		FabulorBanRow *row = ban_row_new (mode, type, mask, from, date);
		fabulor_gtk4_flat_model_stack_append (list->models, row);
		g_object_unref (row);
	}
}

void
fabulor_ban_list_clear (FabulorBanList *list)
{
	g_return_if_fail (list != NULL);
	fabulor_gtk4_flat_model_stack_clear (list->models);
	ban_list_selection_changed (list);
}

guint
fabulor_ban_list_get_n_rows (FabulorBanList *list)
{
	g_return_val_if_fail (list != NULL, 0);
	return g_list_model_get_n_items (G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_sorted (list->models)));
}

guint
fabulor_ban_list_get_n_selected (FabulorBanList *list)
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

void
fabulor_ban_list_select_all (FabulorBanList *list)
{
	g_return_if_fail (list != NULL);
	gtk_selection_model_select_all (
		fabulor_gtk4_flat_model_stack_get_selection (list->models));
}

gboolean
fabulor_ban_list_set_selected (FabulorBanList *list, guint position,
	gboolean selected)
{
	g_return_val_if_fail (list != NULL, FALSE);
	if (position >= fabulor_ban_list_get_n_rows (list))
		return FALSE;
	if (selected)
		return gtk_selection_model_select_item (
			fabulor_gtk4_flat_model_stack_get_selection (list->models), position,
			FALSE);
	return gtk_selection_model_unselect_item (
		fabulor_gtk4_flat_model_stack_get_selection (list->models), position);
}

void
fabulor_ban_list_invert_selection (FabulorBanList *list)
{
	guint count;
	guint i;

	g_return_if_fail (list != NULL);
	count = fabulor_ban_list_get_n_rows (list);
	{
		GtkSelectionModel *selection =
			fabulor_gtk4_flat_model_stack_get_selection (list->models);
		for (i = 0; i < count; i++)
			if (gtk_selection_model_is_selected (selection, i))
				gtk_selection_model_unselect_item (selection, i);
			else
				gtk_selection_model_select_item (selection, i, FALSE);
	}
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
	return masks;
}

gboolean
fabulor_ban_list_select_at_point (FabulorBanList *list, gdouble x, gdouble y)
{
	g_return_val_if_fail (list != NULL && list->view != NULL, FALSE);
	{
		guint position;
		if (!ban_list_position_at_point (list, x, y, &position))
			return FALSE;
		gtk_selection_model_select_item (
			fabulor_gtk4_flat_model_stack_get_selection (list->models), position,
			TRUE);
		return TRUE;
	}
}

static gboolean
ban_list_dup_selected_fields (FabulorBanList *list, gchar **mask,
	gchar **from, gchar **date)
{
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
