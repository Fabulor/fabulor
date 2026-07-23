/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ignore-list.h"

#include "gtk-compat.h"

#include "gtk4-list-models.h"

enum
{
	IGNORE_FLAG_PRIVATE = 1,
	IGNORE_FLAG_NOTICE = 2,
	IGNORE_FLAG_CHANNEL = 4,
	IGNORE_FLAG_CTCP = 8,
	IGNORE_FLAG_INVITE = 16,
	IGNORE_FLAG_UNIGNORE = 32,
	IGNORE_FLAG_DCC = 128
};

static gboolean
ignore_flag_is_valid (guint flag)
{
	return flag == IGNORE_FLAG_CHANNEL || flag == IGNORE_FLAG_PRIVATE ||
		flag == IGNORE_FLAG_NOTICE || flag == IGNORE_FLAG_CTCP ||
		flag == IGNORE_FLAG_DCC || flag == IGNORE_FLAG_INVITE ||
		flag == IGNORE_FLAG_UNIGNORE;
}

struct _FabulorIgnoreList
{
	GtkWidget *view;
	FabulorIgnoreListRenameFunc rename_func;
	FabulorIgnoreListFlagsFunc flags_func;
	gpointer callback_data;
	FabulorGtk4FlatModelStack *models;
};


typedef struct _FabulorIgnoreRow FabulorIgnoreRow;
typedef struct _FabulorIgnoreRowClass FabulorIgnoreRowClass;

struct _FabulorIgnoreRow
{
	GObject parent_instance;
	gchar *mask;
	guint flags;
};

struct _FabulorIgnoreRowClass
{
	GObjectClass parent_class;
};

enum
{
	PROP_IGNORE_ROW_0,
	PROP_IGNORE_ROW_MASK,
	PROP_IGNORE_ROW_FLAGS,
	N_IGNORE_ROW_PROPERTIES
};

static GParamSpec *ignore_row_properties[N_IGNORE_ROW_PROPERTIES];

#define FABULOR_TYPE_IGNORE_ROW (fabulor_ignore_row_get_type ())
#define FABULOR_IGNORE_ROW(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), FABULOR_TYPE_IGNORE_ROW, FabulorIgnoreRow))

G_DEFINE_TYPE (FabulorIgnoreRow, fabulor_ignore_row, G_TYPE_OBJECT)

static void
fabulor_ignore_row_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	FabulorIgnoreRow *row = FABULOR_IGNORE_ROW (object);

	if (property_id == PROP_IGNORE_ROW_MASK)
		g_value_set_string (value, row->mask);
	else if (property_id == PROP_IGNORE_ROW_FLAGS)
		g_value_set_uint (value, row->flags);
	else
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
fabulor_ignore_row_finalize (GObject *object)
{
	g_free (FABULOR_IGNORE_ROW (object)->mask);
	G_OBJECT_CLASS (fabulor_ignore_row_parent_class)->finalize (object);
}

static void
fabulor_ignore_row_class_init (FabulorIgnoreRowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = fabulor_ignore_row_get_property;
	object_class->finalize = fabulor_ignore_row_finalize;
	ignore_row_properties[PROP_IGNORE_ROW_MASK] = g_param_spec_string (
		"mask", "Mask", "Ignore mask", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	ignore_row_properties[PROP_IGNORE_ROW_FLAGS] = g_param_spec_uint (
		"flags", "Flags", "Complete ignore flags", 0, G_MAXUINT, 0,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties (object_class, N_IGNORE_ROW_PROPERTIES,
		ignore_row_properties);
}

static void
fabulor_ignore_row_init (FabulorIgnoreRow *row)
{
	(void) row;
}

static FabulorIgnoreRow *
ignore_row_new (const gchar *mask, guint flags)
{
	FabulorIgnoreRow *row = g_object_new (FABULOR_TYPE_IGNORE_ROW, NULL);

	row->mask = g_strdup (mask);
	row->flags = flags;
	return row;
}

static FabulorIgnoreRow *
ignore_row_at (FabulorIgnoreList *list, guint position)
{
	return g_list_model_get_item (G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_sorted (list->models)), position);
}

static gboolean
ignore_list_select_row (FabulorIgnoreList *list, FabulorIgnoreRow *row,
	gboolean scroll)
{
	GListModel *sorted = G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_sorted (list->models));
	guint i;

	for (i = 0; i < g_list_model_get_n_items (sorted); i++)
	{
		gpointer item = g_list_model_get_item (sorted, i);
		gboolean match = item == row;
		g_object_unref (item);
		if (match)
		{
			gtk_selection_model_select_item (
				fabulor_gtk4_flat_model_stack_get_selection (list->models), i,
				TRUE);
			if (scroll && list->view)
				gtk_column_view_scroll_to (GTK_COLUMN_VIEW (list->view), i, NULL,
					GTK_LIST_SCROLL_FOCUS, NULL);
			return TRUE;
		}
	}
	return FALSE;
}

static gboolean
ignore_list_rename_row (FabulorIgnoreList *list, FabulorIgnoreRow *row,
	const gchar *new_mask)
{
	gboolean was_selected;

	if (!new_mask || !*new_mask || g_strcmp0 (row->mask, new_mask) == 0)
		return new_mask && *new_mask;
	if (list->rename_func && !list->rename_func (row->mask, new_mask,
		row->flags, list->callback_data))
		return FALSE;
	g_free (row->mask);
	row->mask = g_strdup (new_mask);
	was_selected = gtk_single_selection_get_selected_item (GTK_SINGLE_SELECTION (
		fabulor_gtk4_flat_model_stack_get_selection (list->models))) ==
		G_OBJECT (row);
	g_object_notify_by_pspec (G_OBJECT (row),
		ignore_row_properties[PROP_IGNORE_ROW_MASK]);
	if (was_selected)
		ignore_list_select_row (list, row, FALSE);
	return TRUE;
}

static void
ignore_list_set_row_flag (FabulorIgnoreList *list, FabulorIgnoreRow *row,
	guint flag, gboolean active)
{
	guint flags = active ? row->flags | flag : row->flags & ~flag;

	if (flags == row->flags)
		return;
	row->flags = flags;
	g_object_notify_by_pspec (G_OBJECT (row),
		ignore_row_properties[PROP_IGNORE_ROW_FLAGS]);
	if (list->flags_func)
		list->flags_func (row->mask, row->flags, list->callback_data);
}

typedef struct
{
	FabulorIgnoreList *owner;
	FabulorIgnoreRow *row;
	GtkEditableLabel *label;
	gulong notify_handler;
	gboolean editing;
	gboolean blocked;
} IgnoreMaskBinding;

static void
ignore_mask_row_changed (GObject *object, GParamSpec *pspec, gpointer user_data)
{
	IgnoreMaskBinding *binding = user_data;

	(void) object;
	if (pspec != ignore_row_properties[PROP_IGNORE_ROW_MASK] ||
		binding->blocked || !binding->row)
		return;
	binding->blocked = TRUE;
	gtk_editable_set_text (GTK_EDITABLE (binding->label), binding->row->mask);
	binding->blocked = FALSE;
}

static void
ignore_mask_editing_changed (GtkEditableLabel *label, GParamSpec *pspec,
	gpointer user_data)
{
	IgnoreMaskBinding *binding = user_data;
	gboolean editing = gtk_editable_label_get_editing (label);

	(void) pspec;
	if (!binding->blocked && binding->row && binding->editing && !editing &&
		!ignore_list_rename_row (binding->owner, binding->row,
			gtk_editable_get_text (GTK_EDITABLE (label))))
	{
		binding->blocked = TRUE;
		gtk_editable_set_text (GTK_EDITABLE (label), binding->row->mask);
		binding->blocked = FALSE;
	}
	binding->editing = editing;
}

static void
ignore_mask_binding_free (gpointer data)
{
	IgnoreMaskBinding *binding = data;

	if (binding->row && binding->notify_handler)
		g_signal_handler_disconnect (binding->row, binding->notify_handler);
	g_clear_object (&binding->row);
	g_free (binding);
}

static void
ignore_mask_factory_setup (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	IgnoreMaskBinding *binding = g_new0 (IgnoreMaskBinding, 1);
	GtkWidget *label = gtk_editable_label_new (NULL);

	(void) factory;
	binding->owner = user_data;
	binding->label = GTK_EDITABLE_LABEL (label);
	gtk_widget_set_hexpand (label, TRUE);
	gtk_widget_set_size_request (label, 272, -1);
	g_signal_connect (label, "notify::editing",
		G_CALLBACK (ignore_mask_editing_changed), binding);
	gtk_list_item_set_child (item, label);
	g_object_set_data_full (G_OBJECT (item), "fabulor-ignore-mask-binding",
		binding, ignore_mask_binding_free);
}

static void
ignore_mask_factory_bind (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	IgnoreMaskBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-ignore-mask-binding");

	(void) factory;
	(void) user_data;
	if (binding->row && binding->notify_handler)
		g_signal_handler_disconnect (binding->row, binding->notify_handler);
	g_clear_object (&binding->row);
	binding->row = g_object_ref (FABULOR_IGNORE_ROW (
		gtk_list_item_get_item (item)));
	binding->notify_handler = g_signal_connect (binding->row, "notify::mask",
		G_CALLBACK (ignore_mask_row_changed), binding);
	binding->blocked = TRUE;
	gtk_editable_set_text (GTK_EDITABLE (binding->label), binding->row->mask);
	binding->blocked = FALSE;
	binding->editing = gtk_editable_label_get_editing (binding->label);
}

static void
ignore_mask_factory_unbind (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	IgnoreMaskBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-ignore-mask-binding");

	(void) factory;
	(void) user_data;
	if (binding->row && binding->notify_handler)
		g_signal_handler_disconnect (binding->row, binding->notify_handler);
	binding->notify_handler = 0;
	g_clear_object (&binding->row);
}

typedef struct
{
	FabulorIgnoreList *owner;
	FabulorIgnoreRow *row;
	GtkCheckButton *button;
	guint flag;
	gulong notify_handler;
	gboolean blocked;
} IgnoreToggleBinding;

static void
ignore_toggle_refresh (IgnoreToggleBinding *binding)
{
	binding->blocked = TRUE;
	gtk_check_button_set_active (binding->button,
		(binding->row->flags & binding->flag) != 0);
	binding->blocked = FALSE;
}

static void
ignore_toggle_row_changed (GObject *object, GParamSpec *pspec,
	gpointer user_data)
{
	IgnoreToggleBinding *binding = user_data;

	(void) object;
	if (pspec == ignore_row_properties[PROP_IGNORE_ROW_FLAGS] && binding->row)
		ignore_toggle_refresh (binding);
}

static void
ignore_toggle_changed (GtkCheckButton *button, gpointer user_data)
{
	IgnoreToggleBinding *binding = user_data;

	if (!binding->blocked && binding->row)
		ignore_list_set_row_flag (binding->owner, binding->row, binding->flag,
			gtk_check_button_get_active (button));
}

static void
ignore_toggle_binding_free (gpointer data)
{
	IgnoreToggleBinding *binding = data;

	if (binding->row && binding->notify_handler)
		g_signal_handler_disconnect (binding->row, binding->notify_handler);
	g_clear_object (&binding->row);
	g_free (binding);
}

static void
ignore_toggle_factory_setup (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	IgnoreToggleBinding *binding = g_new0 (IgnoreToggleBinding, 1);
	GtkWidget *button = gtk_check_button_new ();

	(void) factory;
	binding->flag = GPOINTER_TO_UINT (user_data);
	binding->button = GTK_CHECK_BUTTON (button);
	gtk_widget_set_halign (button, GTK_ALIGN_CENTER);
	g_signal_connect (button, "toggled", G_CALLBACK (ignore_toggle_changed),
		binding);
	gtk_list_item_set_child (item, button);
	g_object_set_data_full (G_OBJECT (item), "fabulor-ignore-toggle-binding",
		binding, ignore_toggle_binding_free);
}

static void
ignore_toggle_factory_bind (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	IgnoreToggleBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-ignore-toggle-binding");

	(void) factory;
	(void) user_data;
	binding->owner = user_data;
	if (binding->row && binding->notify_handler)
		g_signal_handler_disconnect (binding->row, binding->notify_handler);
	g_clear_object (&binding->row);
	binding->row = g_object_ref (FABULOR_IGNORE_ROW (
		gtk_list_item_get_item (item)));
	binding->notify_handler = g_signal_connect (binding->row, "notify::flags",
		G_CALLBACK (ignore_toggle_row_changed), binding);
	ignore_toggle_refresh (binding);
}

static void
ignore_toggle_factory_unbind (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	IgnoreToggleBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-ignore-toggle-binding");

	(void) factory;
	(void) user_data;
	if (binding->row && binding->notify_handler)
		g_signal_handler_disconnect (binding->row, binding->notify_handler);
	binding->notify_handler = 0;
	g_clear_object (&binding->row);
}

static GtkColumnViewColumn *
ignore_mask_column_new (FabulorIgnoreList *list, const gchar *title)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	GtkColumnViewColumn *column;
	GtkExpression *expression;
	GtkSorter *sorter;

	g_signal_connect (factory, "setup", G_CALLBACK (ignore_mask_factory_setup),
		list);
	g_signal_connect (factory, "bind", G_CALLBACK (ignore_mask_factory_bind),
		list);
	g_signal_connect (factory, "unbind", G_CALLBACK (ignore_mask_factory_unbind),
		list);
	column = gtk_column_view_column_new (title, factory);
	gtk_column_view_column_set_expand (column, TRUE);
	gtk_column_view_column_set_resizable (column, TRUE);
	expression = gtk_property_expression_new (FABULOR_TYPE_IGNORE_ROW, NULL,
		"mask");
	sorter = GTK_SORTER (gtk_string_sorter_new (expression));
	gtk_column_view_column_set_sorter (column, sorter);
	g_object_unref (sorter);
	return column;
}

static GtkColumnViewColumn *
ignore_toggle_column_new (FabulorIgnoreList *list, const gchar *title,
	guint flag)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	GtkColumnViewColumn *column;

	g_signal_connect (factory, "setup", G_CALLBACK (ignore_toggle_factory_setup),
		GUINT_TO_POINTER (flag));
	g_signal_connect (factory, "bind", G_CALLBACK (ignore_toggle_factory_bind),
		list);
	g_signal_connect (factory, "unbind", G_CALLBACK (ignore_toggle_factory_unbind),
		list);
	column = gtk_column_view_column_new (title, factory);
	gtk_column_view_column_set_resizable (column, TRUE);
	return column;
}


FabulorIgnoreList *
fabulor_ignore_list_new (FabulorIgnoreListRenameFunc rename_func,
	FabulorIgnoreListFlagsFunc flags_func, gpointer user_data)
{
	FabulorIgnoreList *list = g_new0 (FabulorIgnoreList, 1);

	list->rename_func = rename_func;
	list->flags_func = flags_func;
	list->callback_data = user_data;
	list->models = fabulor_gtk4_flat_model_stack_new (FABULOR_TYPE_IGNORE_ROW,
		NULL, FABULOR_GTK4_SELECTION_SINGLE);
	if (!list->models)
	{
		g_free (list);
		return NULL;
	}
	return list;
}

void
fabulor_ignore_list_free (FabulorIgnoreList *list)
{
	if (!list)
		return;
	fabulor_gtk4_flat_model_stack_free (list->models);
	g_free (list);
}

GtkWidget *
fabulor_ignore_list_create_view (FabulorIgnoreList *list, GtkBox *parent,
	const gchar *mask_title, const gchar *channel_title,
	const gchar *private_title, const gchar *notice_title,
	const gchar *ctcp_title, const gchar *dcc_title,
	const gchar *invite_title, const gchar *unignore_title)
{
	g_return_val_if_fail (list != NULL, NULL);
	g_return_val_if_fail (GTK_IS_BOX (parent), NULL);
	g_return_val_if_fail (list->view == NULL, NULL);
	{
		const gchar *titles[] = { channel_title, private_title, notice_title,
			ctcp_title, dcc_title, invite_title, unignore_title };
		const guint flags[] = { IGNORE_FLAG_CHANNEL, IGNORE_FLAG_PRIVATE,
			IGNORE_FLAG_NOTICE, IGNORE_FLAG_CTCP, IGNORE_FLAG_DCC,
			IGNORE_FLAG_INVITE, IGNORE_FLAG_UNIGNORE };
		GtkWidget *scroller = gtk_scrolled_window_new ();
		GtkSelectionModel *selection =
			fabulor_gtk4_flat_model_stack_get_selection (list->models);
		GtkColumnViewColumn *column;
		guint i;

		list->view = gtk_column_view_new (GTK_SELECTION_MODEL (
			g_object_ref (selection)));
		column = ignore_mask_column_new (list, mask_title);
		gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view), column);
		g_object_unref (column);
		for (i = 0; i < G_N_ELEMENTS (flags); i++)
		{
			column = ignore_toggle_column_new (list, titles[i], flags[i]);
			gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view), column);
			g_object_unref (column);
		}
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
fabulor_ignore_list_append (FabulorIgnoreList *list, const gchar *mask,
	guint flags, gboolean select)
{
	g_return_if_fail (list != NULL);
	g_return_if_fail (mask != NULL);
	{
		FabulorIgnoreRow *row = ignore_row_new (mask, flags);
		fabulor_gtk4_flat_model_stack_append (list->models, row);
		if (select)
			ignore_list_select_row (list, row, TRUE);
		g_object_unref (row);
	}
}

void
fabulor_ignore_list_clear (FabulorIgnoreList *list)
{
	g_return_if_fail (list != NULL);
	fabulor_gtk4_flat_model_stack_clear (list->models);
}

guint
fabulor_ignore_list_get_n_rows (FabulorIgnoreList *list)
{
	g_return_val_if_fail (list != NULL, 0);
	return g_list_model_get_n_items (G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_sorted (list->models)));
}

gchar *
fabulor_ignore_list_dup_mask_at (FabulorIgnoreList *list, guint position)
{
	g_return_val_if_fail (list != NULL, NULL);
	{
		FabulorIgnoreRow *row = ignore_row_at (list, position);
		gchar *mask;
		if (!row)
			return NULL;
		mask = g_strdup (row->mask);
		g_object_unref (row);
		return mask;
	}
}

guint
fabulor_ignore_list_get_flags_at (FabulorIgnoreList *list, guint position)
{
	g_return_val_if_fail (list != NULL, 0);
	{
		FabulorIgnoreRow *row = ignore_row_at (list, position);
		guint flags = row ? row->flags : 0;
		g_clear_object (&row);
		return flags;
	}
}

gboolean
fabulor_ignore_list_rename_at (FabulorIgnoreList *list, guint position,
	const gchar *new_mask)
{
	g_return_val_if_fail (list != NULL, FALSE);
	{
		FabulorIgnoreRow *row = ignore_row_at (list, position);
		gboolean result;
		if (!row)
			return FALSE;
		result = ignore_list_rename_row (list, row, new_mask);
		g_object_unref (row);
		return result;
	}
}

gboolean
fabulor_ignore_list_set_flag_at (FabulorIgnoreList *list, guint position,
	guint flag, gboolean active)
{
	g_return_val_if_fail (list != NULL, FALSE);
	if (!ignore_flag_is_valid (flag))
		return FALSE;
	{
		FabulorIgnoreRow *row = ignore_row_at (list, position);
		if (!row)
			return FALSE;
		ignore_list_set_row_flag (list, row, flag, active);
		g_object_unref (row);
		return TRUE;
	}
}

gchar *
fabulor_ignore_list_remove_selected (FabulorIgnoreList *list)
{
	g_return_val_if_fail (list != NULL, NULL);
	{
		GtkSingleSelection *selection = GTK_SINGLE_SELECTION (
			fabulor_gtk4_flat_model_stack_get_selection (list->models));
		FabulorIgnoreRow *row = FABULOR_IGNORE_ROW (
			gtk_single_selection_get_selected_item (selection));
		GListStore *store;
		guint selected;
		guint store_position;
		gchar *mask;
		if (!row)
			return NULL;
		selected = gtk_single_selection_get_selected (selection);
		store = fabulor_gtk4_flat_model_stack_get_store (list->models);
		if (!g_list_store_find (store, row, &store_position))
			return NULL;
		mask = g_strdup (row->mask);
		g_list_store_remove (store, store_position);
		if (selected < g_list_model_get_n_items (G_LIST_MODEL (
			fabulor_gtk4_flat_model_stack_get_sorted (list->models))))
			gtk_selection_model_select_item (GTK_SELECTION_MODEL (selection),
				selected, TRUE);
		return mask;
	}
}

GPtrArray *
fabulor_ignore_list_dup_masks (FabulorIgnoreList *list)
{
	GPtrArray *masks = g_ptr_array_new_with_free_func (g_free);
	guint count;
	guint i;

	g_return_val_if_fail (list != NULL, masks);
	count = fabulor_ignore_list_get_n_rows (list);
	for (i = 0; i < count; i++)
		g_ptr_array_add (masks, fabulor_ignore_list_dup_mask_at (list, i));
	return masks;
}
