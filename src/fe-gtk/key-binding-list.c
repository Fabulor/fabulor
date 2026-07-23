/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "key-binding-list.h"

#include <gdk/gdkkeysyms.h>

#include "gtk-compat.h"

struct _FabulorKeyBindingList
{
	GtkWidget *view;
	FabulorKeyBindingSelectionFunc selection_func;
	FabulorKeyBindingNormalizeFunc normalize_func;
	gpointer callback_data;
	GListStore *store;
	GtkSingleSelection *selection;
	GtkStringList *actions;
};

void
fabulor_key_binding_record_free (FabulorKeyBindingRecord *record)
{
	if (!record)
		return;
	g_free (record->key_label);
	g_free (record->accelerator);
	g_free (record->action);
	g_free (record->data1);
	g_free (record->data2);
	g_free (record);
}


typedef struct _FabulorKeyBindingRow FabulorKeyBindingRow;
typedef struct _FabulorKeyBindingRowClass FabulorKeyBindingRowClass;

struct _FabulorKeyBindingRow
{
	GObject parent_instance;
	gchar *key_label;
	gchar *accelerator;
	gchar *action;
	gchar *data1;
	gchar *data2;
	gboolean custom;
};

struct _FabulorKeyBindingRowClass { GObjectClass parent_class; };

#define FABULOR_TYPE_KEY_BINDING_ROW (fabulor_key_binding_row_get_type ())
#define FABULOR_KEY_BINDING_ROW(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), \
	FABULOR_TYPE_KEY_BINDING_ROW, FabulorKeyBindingRow))

G_DEFINE_TYPE (FabulorKeyBindingRow, fabulor_key_binding_row, G_TYPE_OBJECT)

static void
fabulor_key_binding_row_finalize (GObject *object)
{
	FabulorKeyBindingRow *row = FABULOR_KEY_BINDING_ROW (object);
	g_free (row->key_label);
	g_free (row->accelerator);
	g_free (row->action);
	g_free (row->data1);
	g_free (row->data2);
	G_OBJECT_CLASS (fabulor_key_binding_row_parent_class)->finalize (object);
}

static void
fabulor_key_binding_row_class_init (FabulorKeyBindingRowClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = fabulor_key_binding_row_finalize;
}

static void fabulor_key_binding_row_init (FabulorKeyBindingRow *row)
{
	(void) row;
}

static FabulorKeyBindingRow *
key_binding_row_new (const FabulorKeyBindingRecord *record)
{
	FabulorKeyBindingRow *row = g_object_new (FABULOR_TYPE_KEY_BINDING_ROW, NULL);
	row->key_label = g_strdup (record->key_label ? record->key_label : "");
	row->accelerator = g_strdup (record->accelerator ? record->accelerator : "");
	row->action = g_strdup (record->action ? record->action : "");
	row->data1 = g_strdup (record->data1 ? record->data1 : "");
	row->data2 = g_strdup (record->data2 ? record->data2 : "");
	row->custom = record->custom;
	return row;
}

static FabulorKeyBindingRow *
key_binding_row_at (FabulorKeyBindingList *list, guint position)
{
	return g_list_model_get_item (G_LIST_MODEL (list->store), position);
}

static guint
key_binding_action_position (FabulorKeyBindingList *list, const gchar *action)
{
	guint i;
	for (i = 0; i < g_list_model_get_n_items (G_LIST_MODEL (list->actions)); i++)
	{
		const gchar *candidate = gtk_string_list_get_string (list->actions, i);
		if (g_strcmp0 (candidate, action) == 0)
			return i;
	}
	return GTK_INVALID_LIST_POSITION;
}

typedef struct
{
	FabulorKeyBindingList *owner;
	FabulorKeyBindingRow *row;
	gboolean blocked;
} KeyBindingBinding;

static guint
key_binding_binding_position (KeyBindingBinding *binding)
{
	guint position = GTK_INVALID_LIST_POSITION;
	if (binding->row)
		g_list_store_find (binding->owner->store, binding->row, &position);
	return position;
}

static void
key_binding_binding_free (gpointer data)
{
	KeyBindingBinding *binding = data;
	g_clear_object (&binding->row);
	g_free (binding);
}

static void
key_binding_factory_unbind (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	KeyBindingBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-key-binding");
	(void) factory;
	(void) user_data;
	g_clear_object (&binding->row);
}

static void
key_binding_select_row (KeyBindingBinding *binding)
{
	guint position = key_binding_binding_position (binding);
	if (position != GTK_INVALID_LIST_POSITION)
		fabulor_key_binding_list_set_selected (binding->owner, position);
}

static gboolean
key_binding_key_pressed (GtkWidget *widget, guint keyval,
	GdkModifierType state, gpointer user_data)
{
	KeyBindingBinding *binding = user_data;
	guint position = key_binding_binding_position (binding);
	(void) widget;
	if (position == GTK_INVALID_LIST_POSITION)
		return FALSE;
	if (keyval == GDK_KEY_Tab && (state & GDK_SHIFT_MASK))
		keyval = GDK_KEY_ISO_Left_Tab;
	if (fabulor_key_binding_list_set_accelerator_at (binding->owner,
		position, keyval, state))
	{
		FabulorKeyBindingRow *row = key_binding_row_at (binding->owner, position);
		GtkWidget *label = gtk_button_get_child (GTK_BUTTON (widget));
		gtk_shortcut_label_set_accelerator (GTK_SHORTCUT_LABEL (label),
			row->accelerator);
		g_object_unref (row);
		return TRUE;
	}
	return FALSE;
}

static void
key_binding_key_clicked (GtkButton *button, gpointer user_data)
{
	KeyBindingBinding *binding = user_data;
	key_binding_select_row (binding);
	gtk_widget_grab_focus (GTK_WIDGET (button));
}

static void
key_binding_key_setup (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	KeyBindingBinding *binding = g_new0 (KeyBindingBinding, 1);
	GtkWidget *button = gtk_button_new ();
	GtkWidget *label = gtk_shortcut_label_new ("");
	(void) factory;
	(void) user_data;
	binding->owner = user_data;
	fabulor_gtk_button_set_child (GTK_BUTTON (button), label);
	g_signal_connect (button, "clicked", G_CALLBACK (key_binding_key_clicked), binding);
	fabulor_gtk_widget_on_key_pressed (button, key_binding_key_pressed, binding);
	gtk_list_item_set_child (item, button);
	g_object_set_data_full (G_OBJECT (item), "fabulor-key-binding", binding,
		key_binding_binding_free);
}

static void
key_binding_key_bind (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	KeyBindingBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-key-binding");
	GtkWidget *button = gtk_list_item_get_child (item);
	GtkWidget *label = gtk_button_get_child (GTK_BUTTON (button));
	(void) factory;
	(void) user_data;
	g_set_object (&binding->row, FABULOR_KEY_BINDING_ROW (gtk_list_item_get_item (item)));
	gtk_shortcut_label_set_accelerator (GTK_SHORTCUT_LABEL (label),
		binding->row->accelerator);
}

static void
key_binding_action_changed (GtkDropDown *dropdown, GParamSpec *pspec,
	gpointer user_data)
{
	KeyBindingBinding *binding = user_data;
	guint position = key_binding_binding_position (binding);
	guint selected = gtk_drop_down_get_selected (dropdown);
	const gchar *action;
	(void) pspec;
	if (binding->blocked || !binding->row || !binding->row->custom ||
		position == GTK_INVALID_LIST_POSITION ||
		selected == GTK_INVALID_LIST_POSITION)
		return;
	action = gtk_string_list_get_string (binding->owner->actions, selected);
	fabulor_key_binding_list_set_text_at (binding->owner, position,
		FABULOR_KEY_BINDING_ACTION, action);
}

static void
key_binding_action_setup (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	FabulorKeyBindingList *list = user_data;
	KeyBindingBinding *binding = g_new0 (KeyBindingBinding, 1);
	GtkWidget *dropdown = gtk_drop_down_new (G_LIST_MODEL (g_object_ref (
		list->actions)), NULL);
	(void) factory;
	binding->owner = list;
	g_signal_connect (dropdown, "notify::selected",
		G_CALLBACK (key_binding_action_changed), binding);
	gtk_list_item_set_child (item, dropdown);
	g_object_set_data_full (G_OBJECT (item), "fabulor-key-binding", binding,
		key_binding_binding_free);
}

static void
key_binding_action_bind (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	KeyBindingBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-key-binding");
	GtkDropDown *dropdown = GTK_DROP_DOWN (gtk_list_item_get_child (item));
	(void) factory;
	(void) user_data;
	g_set_object (&binding->row, FABULOR_KEY_BINDING_ROW (gtk_list_item_get_item (item)));
	binding->blocked = TRUE;
	gtk_drop_down_set_selected (dropdown, key_binding_action_position (
		binding->owner, binding->row->action));
	binding->blocked = FALSE;
	gtk_widget_set_sensitive (GTK_WIDGET (dropdown), binding->row->custom);
}

typedef struct
{
	FabulorKeyBindingList *owner;
	FabulorKeyBindingField field;
} KeyBindingTextFactoryData;

static void
key_binding_text_editing_changed (GtkEditableLabel *label, GParamSpec *pspec,
	gpointer user_data)
{
	KeyBindingBinding *binding = user_data;
	KeyBindingTextFactoryData *data = g_object_get_data (G_OBJECT (label),
		"fabulor-key-binding-text-data");
	guint position = key_binding_binding_position (binding);
	(void) pspec;
	if (binding->row && binding->row->custom && position != GTK_INVALID_LIST_POSITION &&
		!gtk_editable_label_get_editing (label))
		fabulor_key_binding_list_set_text_at (binding->owner, position,
			data->field, gtk_editable_get_text (GTK_EDITABLE (label)));
}

static void
key_binding_text_setup (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	KeyBindingTextFactoryData *data = user_data;
	KeyBindingBinding *binding = g_new0 (KeyBindingBinding, 1);
	GtkWidget *label = gtk_editable_label_new (NULL);
	(void) factory;
	binding->owner = data->owner;
	g_object_set_data (G_OBJECT (label), "fabulor-key-binding-text-data", data);
	g_signal_connect (label, "notify::editing",
		G_CALLBACK (key_binding_text_editing_changed), binding);
	gtk_widget_set_hexpand (label, TRUE);
	gtk_list_item_set_child (item, label);
	g_object_set_data_full (G_OBJECT (item), "fabulor-key-binding", binding,
		key_binding_binding_free);
}

static void
key_binding_text_bind (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	KeyBindingTextFactoryData *data = user_data;
	KeyBindingBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-key-binding");
	GtkWidget *label = gtk_list_item_get_child (item);
	const gchar *text;
	(void) factory;
	g_set_object (&binding->row, FABULOR_KEY_BINDING_ROW (gtk_list_item_get_item (item)));
	text = data->field == FABULOR_KEY_BINDING_DATA1 ?
		binding->row->data1 : binding->row->data2;
	gtk_editable_set_text (GTK_EDITABLE (label), text);
	gtk_widget_set_sensitive (label, binding->row->custom);
}

static GtkColumnViewColumn *
key_binding_column_new (const gchar *title, GCallback setup, GCallback bind,
	gpointer data, GDestroyNotify destroy, gboolean expand)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	GtkColumnViewColumn *column;
	if (destroy)
		g_object_set_data_full (G_OBJECT (factory), "fabulor-key-binding-data",
			data, destroy);
	g_signal_connect (factory, "setup", setup, data);
	g_signal_connect (factory, "bind", bind, data);
	g_signal_connect (factory, "unbind", G_CALLBACK (key_binding_factory_unbind), data);
	column = gtk_column_view_column_new (title ? title : "", factory);
	gtk_column_view_column_set_expand (column, expand);
	gtk_column_view_column_set_resizable (column, TRUE);
	return column;
}

static void
key_binding_selected_changed (GtkSingleSelection *selection, GParamSpec *pspec,
	gpointer user_data)
{
	FabulorKeyBindingList *list = user_data;
	guint position = gtk_single_selection_get_selected (selection);
	FabulorKeyBindingRow *row = NULL;
	(void) pspec;
	if (position != GTK_INVALID_LIST_POSITION)
		row = key_binding_row_at (list, position);
	if (list->selection_func)
		list->selection_func (row && row->action[0] ? row->action : NULL,
			row ? row->custom : FALSE, list->callback_data);
	g_clear_object (&row);
}

static gboolean
key_binding_view_key_pressed (GtkWidget *widget, guint keyval,
	GdkModifierType state, gpointer user_data)
{
	FabulorKeyBindingList *list = user_data;
	(void) widget;
	if (!(state & GDK_SHIFT_MASK))
		return FALSE;
	if (keyval == GDK_KEY_Up)
		return fabulor_key_binding_list_move_selected (list, -1);
	if (keyval == GDK_KEY_Down)
		return fabulor_key_binding_list_move_selected (list, 1);
	return FALSE;
}


FabulorKeyBindingList *
fabulor_key_binding_list_new (FabulorKeyBindingSelectionFunc selection_func,
	FabulorKeyBindingNormalizeFunc normalize_func, gpointer user_data)
{
	FabulorKeyBindingList *list = g_new0 (FabulorKeyBindingList, 1);
	list->selection_func = selection_func;
	list->normalize_func = normalize_func;
	list->callback_data = user_data;
	list->store = g_list_store_new (FABULOR_TYPE_KEY_BINDING_ROW);
	list->selection = gtk_single_selection_new (G_LIST_MODEL (g_object_ref (list->store)));
	list->actions = gtk_string_list_new (NULL);
	g_signal_connect (list->selection, "notify::selected",
		G_CALLBACK (key_binding_selected_changed), list);
	return list;
}

void
fabulor_key_binding_list_free (FabulorKeyBindingList *list)
{
	if (!list)
		return;
	g_clear_object (&list->selection);
	g_clear_object (&list->store);
	g_clear_object (&list->actions);
	g_free (list);
}

GtkWidget *
fabulor_key_binding_list_create_view (FabulorKeyBindingList *list,
	GtkBox *parent, const gchar *key_title, const gchar *action_title,
	const gchar *data1_title, const gchar *data2_title,
	const gchar *const *actions, guint n_actions)
{
	GtkWidget *scroller;
	guint i;
	g_return_val_if_fail (list && GTK_IS_BOX (parent) && !list->view, NULL);
	scroller = gtk_scrolled_window_new ();
	for (i = 0; i < n_actions; i++)
		gtk_string_list_append (list->actions, actions[i]);
	list->view = gtk_column_view_new (GTK_SELECTION_MODEL (g_object_ref (list->selection)));
	gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view),
		key_binding_column_new (key_title, G_CALLBACK (key_binding_key_setup),
			G_CALLBACK (key_binding_key_bind), list, NULL, FALSE));
	gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view),
		key_binding_column_new (action_title, G_CALLBACK (key_binding_action_setup),
			G_CALLBACK (key_binding_action_bind), list, NULL, FALSE));
	for (i = 0; i < 2; i++)
	{
		KeyBindingTextFactoryData *data = g_new (KeyBindingTextFactoryData, 1);
		data->owner = list;
		data->field = i == 0 ? FABULOR_KEY_BINDING_DATA1 : FABULOR_KEY_BINDING_DATA2;
		gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view),
			key_binding_column_new (i == 0 ? data1_title : data2_title,
				G_CALLBACK (key_binding_text_setup), G_CALLBACK (key_binding_text_bind),
				data, g_free, TRUE));
	}
	gtk_column_view_set_show_row_separators (GTK_COLUMN_VIEW (list->view), TRUE);
	fabulor_gtk_widget_on_key_pressed (list->view,
		key_binding_view_key_pressed, list);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroller), list->view);
	fabulor_gtk_box_append (parent, scroller, TRUE, TRUE, 0);
	return list->view;
}

void
fabulor_key_binding_list_append (FabulorKeyBindingList *list,
	const FabulorKeyBindingRecord *record)
{
	g_return_if_fail (list && record);
	{
		FabulorKeyBindingRow *row = key_binding_row_new (record);
		g_list_store_append (list->store, row);
		g_object_unref (row);
	}
}

void
fabulor_key_binding_list_clear (FabulorKeyBindingList *list)
{
	g_return_if_fail (list != NULL);
	g_list_store_remove_all (list->store);
}

guint
fabulor_key_binding_list_get_n_rows (FabulorKeyBindingList *list)
{
	g_return_val_if_fail (list != NULL, 0);
	return g_list_model_get_n_items (G_LIST_MODEL (list->store));
}

guint
fabulor_key_binding_list_add_custom (FabulorKeyBindingList *list)
{
	FabulorKeyBindingRecord record = { "", "", "", "", "", TRUE };
	guint position;
	g_return_val_if_fail (list != NULL, G_MAXUINT);
	position = fabulor_key_binding_list_get_n_rows (list);
	fabulor_key_binding_list_append (list, &record);
	fabulor_key_binding_list_set_selected (list, position);
	if (list->view)
		gtk_column_view_scroll_to (GTK_COLUMN_VIEW (list->view), position, NULL,
			GTK_LIST_SCROLL_FOCUS, NULL);
	return position;
}

static gboolean
key_binding_selected_position (FabulorKeyBindingList *list, guint *position)
{
	*position = gtk_single_selection_get_selected (list->selection);
	return *position != GTK_INVALID_LIST_POSITION;
}

gboolean
fabulor_key_binding_list_set_selected (FabulorKeyBindingList *list,
	guint position)
{
	g_return_val_if_fail (list != NULL, FALSE);
	if (position >= fabulor_key_binding_list_get_n_rows (list))
		return FALSE;
	return gtk_selection_model_select_item (GTK_SELECTION_MODEL (list->selection), position, TRUE);
}

gboolean
fabulor_key_binding_list_set_accelerator_at (FabulorKeyBindingList *list,
	guint position, guint keyval, GdkModifierType modifiers)
{
	gchar *label;
	gchar *accelerator;
	g_return_val_if_fail (list != NULL, FALSE);
	if (position >= fabulor_key_binding_list_get_n_rows (list))
		return FALSE;
	if (keyval == GDK_KEY_Tab && (modifiers & GDK_SHIFT_MASK))
		keyval = GDK_KEY_ISO_Left_Tab;
	if (list->normalize_func)
		modifiers = list->normalize_func (modifiers, list->callback_data);
	label = gtk_accelerator_get_label (keyval, modifiers);
	accelerator = gtk_accelerator_name (keyval, modifiers);
	{
		FabulorKeyBindingRow *row = key_binding_row_at (list, position);
		g_free (row->key_label);
		g_free (row->accelerator);
		row->key_label = label;
		row->accelerator = accelerator;
		g_object_unref (row);
	}
	return TRUE;
}

gboolean
fabulor_key_binding_list_set_text_at (FabulorKeyBindingList *list,
	guint position, FabulorKeyBindingField field, const gchar *text)
{
	gboolean custom;
	g_return_val_if_fail (list != NULL, FALSE);
	if (position >= fabulor_key_binding_list_get_n_rows (list) ||
		field < FABULOR_KEY_BINDING_ACTION || field > FABULOR_KEY_BINDING_DATA2)
		return FALSE;
	{
		FabulorKeyBindingRow *row = key_binding_row_at (list, position);
		gchar **value;
		custom = row->custom;
		if (!custom)
		{
			g_object_unref (row);
			return FALSE;
		}
		value = field == FABULOR_KEY_BINDING_ACTION ? &row->action :
			field == FABULOR_KEY_BINDING_DATA1 ? &row->data1 : &row->data2;
		g_free (*value);
		*value = g_strdup (text ? text : "");
		g_object_unref (row);
	}
	if (field == FABULOR_KEY_BINDING_ACTION && list->selection_func)
		list->selection_func (text && text[0] ? text : NULL, TRUE,
			list->callback_data);
	return TRUE;
}

GPtrArray *
fabulor_key_binding_list_dup_all (FabulorKeyBindingList *list)
{
	GPtrArray *rows = g_ptr_array_new_with_free_func (
		(GDestroyNotify) fabulor_key_binding_record_free);
	guint i;
	g_return_val_if_fail (list != NULL, rows);
	for (i = 0; i < fabulor_key_binding_list_get_n_rows (list); i++)
	{
		FabulorKeyBindingRecord *record = g_new0 (FabulorKeyBindingRecord, 1);
		FabulorKeyBindingRow *row = key_binding_row_at (list, i);
		record->key_label = g_strdup (row->key_label);
		record->accelerator = g_strdup (row->accelerator);
		record->action = g_strdup (row->action);
		record->data1 = g_strdup (row->data1);
		record->data2 = g_strdup (row->data2);
		record->custom = row->custom;
		g_object_unref (row);
		g_ptr_array_add (rows, record);
	}
	return rows;
}

gboolean
fabulor_key_binding_list_remove_selected (FabulorKeyBindingList *list)
{
	guint position;
	GPtrArray *rows;
	FabulorKeyBindingRecord *record;
	guint count;
	g_return_val_if_fail (list != NULL, FALSE);
	if (!key_binding_selected_position (list, &position))
		return FALSE;
	rows = fabulor_key_binding_list_dup_all (list);
	record = g_ptr_array_index (rows, position);
	if (!record->custom)
	{
		g_ptr_array_unref (rows);
		return FALSE;
	}
	g_ptr_array_unref (rows);
	g_list_store_remove (list->store, position);
	count = fabulor_key_binding_list_get_n_rows (list);
	if (position < count)
		fabulor_key_binding_list_set_selected (list, position);
	return TRUE;
}

gboolean
fabulor_key_binding_list_move_selected (FabulorKeyBindingList *list, gint delta)
{
	guint position;
	gint target;
	GPtrArray *rows;
	FabulorKeyBindingRecord *first;
	FabulorKeyBindingRecord *second;
	g_return_val_if_fail (list != NULL, FALSE);
	if ((delta != -1 && delta != 1) || !key_binding_selected_position (list, &position))
		return FALSE;
	target = (gint) position + delta;
	if (target < 0 || (guint) target >= fabulor_key_binding_list_get_n_rows (list))
		return FALSE;
	rows = fabulor_key_binding_list_dup_all (list);
	first = g_ptr_array_index (rows, position);
	second = g_ptr_array_index (rows, (guint) target);
	if (!first->custom || !second->custom)
	{
		g_ptr_array_unref (rows);
		return FALSE;
	}
	g_ptr_array_unref (rows);
	{
		FabulorKeyBindingRow *row = key_binding_row_at (list, position);
		g_list_store_remove (list->store, position);
		g_list_store_insert (list->store, (guint) target, row);
		g_object_unref (row);
	}
	fabulor_key_binding_list_set_selected (list, (guint) target);
	return TRUE;
}
