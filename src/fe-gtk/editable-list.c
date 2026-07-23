/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "editable-list.h"

#include "gtk-compat.h"

#include "gtk4-list-models.h"

struct _FabulorEditableList
{
	GtkWidget *view;
	FabulorGtk4FlatModelStack *models;
	GObject *edit_row;
};

void
fabulor_editable_list_record_free (FabulorEditableListRecord *record)
{
	if (!record)
		return;
	g_free (record->name);
	g_free (record->command);
	g_free (record);
}


typedef struct _FabulorEditableRow FabulorEditableRow;
typedef struct _FabulorEditableRowClass FabulorEditableRowClass;

struct _FabulorEditableRow
{
	GObject parent_instance;
	gchar *name;
	gchar *command;
};

struct _FabulorEditableRowClass
{
	GObjectClass parent_class;
};

enum
{
	PROP_EDITABLE_ROW_0,
	PROP_EDITABLE_ROW_NAME,
	PROP_EDITABLE_ROW_COMMAND,
	N_EDITABLE_ROW_PROPERTIES
};

static GParamSpec *editable_row_properties[N_EDITABLE_ROW_PROPERTIES];

#define FABULOR_TYPE_EDITABLE_ROW (fabulor_editable_row_get_type ())
#define FABULOR_EDITABLE_ROW(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), FABULOR_TYPE_EDITABLE_ROW, \
	 FabulorEditableRow))

G_DEFINE_TYPE (FabulorEditableRow, fabulor_editable_row, G_TYPE_OBJECT)

static void
fabulor_editable_row_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	FabulorEditableRow *row = FABULOR_EDITABLE_ROW (object);

	if (property_id == PROP_EDITABLE_ROW_NAME)
		g_value_set_string (value, row->name);
	else if (property_id == PROP_EDITABLE_ROW_COMMAND)
		g_value_set_string (value, row->command);
	else
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
fabulor_editable_row_finalize (GObject *object)
{
	FabulorEditableRow *row = FABULOR_EDITABLE_ROW (object);

	g_free (row->name);
	g_free (row->command);
	G_OBJECT_CLASS (fabulor_editable_row_parent_class)->finalize (object);
}

static void
fabulor_editable_row_class_init (FabulorEditableRowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = fabulor_editable_row_get_property;
	object_class->finalize = fabulor_editable_row_finalize;
	editable_row_properties[PROP_EDITABLE_ROW_NAME] = g_param_spec_string (
		"name", "Name", "Editable list name", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	editable_row_properties[PROP_EDITABLE_ROW_COMMAND] = g_param_spec_string (
		"command", "Command", "Editable list command", NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties (object_class,
		N_EDITABLE_ROW_PROPERTIES, editable_row_properties);
}

static void
fabulor_editable_row_init (FabulorEditableRow *row)
{
	(void) row;
}

static FabulorEditableRow *
editable_row_new (const gchar *name, const gchar *command)
{
	FabulorEditableRow *row = g_object_new (FABULOR_TYPE_EDITABLE_ROW, NULL);

	row->name = g_strdup (name ? name : "");
	row->command = g_strdup (command ? command : "");
	return row;
}

static GListStore *
editable_store (FabulorEditableList *list)
{
	return fabulor_gtk4_flat_model_stack_get_store (list->models);
}

static GtkSelectionModel *
editable_selection (FabulorEditableList *list)
{
	return fabulor_gtk4_flat_model_stack_get_selection (list->models);
}

static FabulorEditableRow *
editable_row_at (FabulorEditableList *list, guint position)
{
	return g_list_model_get_item (G_LIST_MODEL (editable_store (list)), position);
}

static void
editable_row_set_text (FabulorEditableRow *row,
	FabulorEditableListField field, const gchar *text)
{
	gchar **value = field == FABULOR_EDITABLE_LIST_NAME ?
		&row->name : &row->command;
	guint property_id = field == FABULOR_EDITABLE_LIST_NAME ?
		PROP_EDITABLE_ROW_NAME : PROP_EDITABLE_ROW_COMMAND;

	if (g_strcmp0 (*value, text ? text : "") == 0)
		return;
	g_free (*value);
	*value = g_strdup (text ? text : "");
	g_object_notify_by_pspec (G_OBJECT (row),
		editable_row_properties[property_id]);
}

typedef struct
{
	FabulorEditableList *owner;
	FabulorEditableRow *row;
	GtkEditableLabel *label;
	FabulorEditableListField field;
	gulong notify_handler;
	gboolean editing;
	gboolean blocked;
} EditableBinding;

typedef struct
{
	FabulorEditableList *owner;
	FabulorEditableListField field;
} EditableFactoryData;

static GdkContentProvider *
editable_drag_prepare (GtkDragSource *source, gdouble x, gdouble y,
	gpointer user_data)
{
	EditableBinding *binding = user_data;

	(void) source;
	(void) x;
	(void) y;
	if (!binding->row || gtk_editable_label_get_editing (binding->label))
		return NULL;
	return gdk_content_provider_new_typed (FABULOR_TYPE_EDITABLE_ROW,
		binding->row);
}

static gboolean
editable_drop (GtkDropTarget *target, const GValue *value, gdouble x,
	gdouble y, gpointer user_data)
{
	EditableBinding *binding = user_data;
	FabulorEditableRow *source_row = g_value_get_object (value);
	GListStore *store = editable_store (binding->owner);
	guint source_position;
	guint target_position;
	guint insert_position;
	gboolean after;

	(void) target;
	(void) x;
	if (!source_row || !binding->row || source_row == binding->row ||
		!g_list_store_find (store, source_row, &source_position) ||
		!g_list_store_find (store, binding->row, &target_position))
		return FALSE;
	after = y >= gtk_widget_get_height (GTK_WIDGET (binding->label)) / 2.0;
	insert_position = target_position + (after ? 1u : 0u);
	if (source_position < insert_position)
		insert_position--;
	g_object_ref (source_row);
	g_list_store_remove (store, source_position);
	g_list_store_insert (store, insert_position, source_row);
	g_object_unref (source_row);
	fabulor_editable_list_set_selected (binding->owner, insert_position);
	if (binding->owner->view)
		gtk_column_view_scroll_to (GTK_COLUMN_VIEW (binding->owner->view),
			insert_position, NULL, GTK_LIST_SCROLL_FOCUS, NULL);
	return TRUE;
}

static void
editable_binding_refresh (EditableBinding *binding)
{
	const gchar *text = binding->field == FABULOR_EDITABLE_LIST_NAME ?
		binding->row->name : binding->row->command;

	binding->blocked = TRUE;
	gtk_editable_set_text (GTK_EDITABLE (binding->label), text);
	binding->blocked = FALSE;
}

static void
editable_row_changed (GObject *object, GParamSpec *pspec, gpointer user_data)
{
	EditableBinding *binding = user_data;
	GParamSpec *expected = binding->field == FABULOR_EDITABLE_LIST_NAME ?
		editable_row_properties[PROP_EDITABLE_ROW_NAME] :
		editable_row_properties[PROP_EDITABLE_ROW_COMMAND];

	(void) object;
	if (!binding->blocked && binding->row && pspec == expected)
		editable_binding_refresh (binding);
}

static void
editable_editing_changed (GtkEditableLabel *label, GParamSpec *pspec,
	gpointer user_data)
{
	EditableBinding *binding = user_data;
	gboolean editing = gtk_editable_label_get_editing (label);

	(void) pspec;
	if (!binding->blocked && binding->row && binding->editing && !editing)
		editable_row_set_text (binding->row, binding->field,
			gtk_editable_get_text (GTK_EDITABLE (label)));
	binding->editing = editing;
}

static void
editable_binding_free (gpointer data)
{
	EditableBinding *binding = data;

	if (binding->row && binding->notify_handler)
		g_signal_handler_disconnect (binding->row, binding->notify_handler);
	g_clear_object (&binding->row);
	g_free (binding);
}

static void
editable_factory_setup (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	EditableFactoryData *data = user_data;
	EditableBinding *binding = g_new0 (EditableBinding, 1);
	GtkWidget *label = gtk_editable_label_new (NULL);
	GtkDragSource *drag_source;
	GtkDropTarget *drop_target;

	(void) factory;
	binding->owner = data->owner;
	binding->field = data->field;
	binding->label = GTK_EDITABLE_LABEL (label);
	gtk_widget_set_hexpand (label, TRUE);
	gtk_widget_set_halign (label, GTK_ALIGN_FILL);
	g_signal_connect (label, "notify::editing",
		G_CALLBACK (editable_editing_changed), binding);
	drag_source = gtk_drag_source_new ();
	gtk_drag_source_set_actions (drag_source, GDK_ACTION_MOVE);
	g_signal_connect (drag_source, "prepare",
		G_CALLBACK (editable_drag_prepare), binding);
	gtk_widget_add_controller (label, GTK_EVENT_CONTROLLER (drag_source));
	drop_target = gtk_drop_target_new (FABULOR_TYPE_EDITABLE_ROW,
		GDK_ACTION_MOVE);
	g_signal_connect (drop_target, "drop", G_CALLBACK (editable_drop), binding);
	gtk_widget_add_controller (label, GTK_EVENT_CONTROLLER (drop_target));
	gtk_list_item_set_child (item, label);
	g_object_set_data_full (G_OBJECT (item), "fabulor-editable-binding",
		binding, editable_binding_free);
}

static void
editable_factory_bind (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	EditableBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-editable-binding");

	(void) factory;
	(void) user_data;
	if (binding->row && binding->notify_handler)
		g_signal_handler_disconnect (binding->row, binding->notify_handler);
	g_clear_object (&binding->row);
	binding->row = g_object_ref (FABULOR_EDITABLE_ROW (
		gtk_list_item_get_item (item)));
	binding->notify_handler = g_signal_connect (binding->row,
		binding->field == FABULOR_EDITABLE_LIST_NAME ? "notify::name" :
		"notify::command",
		G_CALLBACK (editable_row_changed), binding);
	editable_binding_refresh (binding);
	binding->editing = gtk_editable_label_get_editing (binding->label);
	if (binding->field == FABULOR_EDITABLE_LIST_NAME &&
		G_OBJECT (binding->row) == binding->owner->edit_row)
	{
		gtk_editable_label_start_editing (binding->label);
		g_clear_object (&binding->owner->edit_row);
	}
}

static void
editable_factory_unbind (GtkSignalListItemFactory *factory, GtkListItem *item,
	gpointer user_data)
{
	EditableBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-editable-binding");

	(void) factory;
	(void) user_data;
	if (binding->row && binding->notify_handler)
		g_signal_handler_disconnect (binding->row, binding->notify_handler);
	binding->notify_handler = 0;
	g_clear_object (&binding->row);
}

static GtkColumnViewColumn *
editable_column_new (FabulorEditableList *list, const gchar *title,
	FabulorEditableListField field, gboolean expand)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	GtkColumnViewColumn *column;
	EditableFactoryData *data = g_new (EditableFactoryData, 1);

	data->owner = list;
	data->field = field;
	g_object_set_data_full (G_OBJECT (factory), "fabulor-editable-factory-data",
		data, g_free);
	g_signal_connect (factory, "setup", G_CALLBACK (editable_factory_setup),
		data);
	g_signal_connect (factory, "bind", G_CALLBACK (editable_factory_bind), data);
	g_signal_connect (factory, "unbind", G_CALLBACK (editable_factory_unbind),
		data);
	column = gtk_column_view_column_new (title ? title : "", factory);
	gtk_column_view_column_set_expand (column, expand);
	gtk_column_view_column_set_resizable (column, TRUE);
	return column;
}


FabulorEditableList *
fabulor_editable_list_new (void)
{
	FabulorEditableList *list = g_new0 (FabulorEditableList, 1);

	list->models = fabulor_gtk4_flat_model_stack_new (FABULOR_TYPE_EDITABLE_ROW,
		NULL, FABULOR_GTK4_SELECTION_SINGLE);
	if (!list->models)
	{
		g_free (list);
		return NULL;
	}
	return list;
}

void
fabulor_editable_list_free (FabulorEditableList *list)
{
	if (!list)
		return;
	g_clear_object (&list->edit_row);
	fabulor_gtk4_flat_model_stack_free (list->models);
	g_free (list);
}

GtkWidget *
fabulor_editable_list_create_view (FabulorEditableList *list, GtkBox *parent,
	const gchar *name_title, const gchar *command_title)
{
	GtkWidget *scroller;

	g_return_val_if_fail (list && GTK_IS_BOX (parent) && !list->view, NULL);
	scroller = gtk_scrolled_window_new ();
	list->view = gtk_column_view_new (GTK_SELECTION_MODEL (g_object_ref (
		fabulor_gtk4_flat_model_stack_get_selection (list->models))));
	gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view),
		editable_column_new (list, name_title, FABULOR_EDITABLE_LIST_NAME,
		FALSE));
	gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view),
		editable_column_new (list, command_title, FABULOR_EDITABLE_LIST_COMMAND,
		TRUE));
	gtk_column_view_set_show_row_separators (GTK_COLUMN_VIEW (list->view), TRUE);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroller),
		list->view);
	gtk_widget_set_hexpand (scroller, TRUE);
	gtk_widget_set_vexpand (scroller, TRUE);
	fabulor_gtk_box_append (parent, scroller, TRUE, TRUE, 0);
	return list->view;
}

void
fabulor_editable_list_append (FabulorEditableList *list, const gchar *name,
	const gchar *command)
{
	g_return_if_fail (list != NULL);
	{
		FabulorEditableRow *row = editable_row_new (name, command);
		g_list_store_append (editable_store (list), row);
		g_object_unref (row);
	}
}

guint
fabulor_editable_list_get_n_rows (FabulorEditableList *list)
{
	g_return_val_if_fail (list != NULL, 0);
	return g_list_model_get_n_items (G_LIST_MODEL (editable_store (list)));
}

gboolean
fabulor_editable_list_set_selected (FabulorEditableList *list, guint position)
{
	g_return_val_if_fail (list != NULL, FALSE);
	if (position >= fabulor_editable_list_get_n_rows (list))
		return FALSE;
	if (!gtk_selection_model_select_item (editable_selection (list), position,
		TRUE))
		return FALSE;
	if (list->view)
		gtk_column_view_scroll_to (GTK_COLUMN_VIEW (list->view), position, NULL,
			GTK_LIST_SCROLL_FOCUS, NULL);
	return TRUE;
}

void
fabulor_editable_list_add_empty (FabulorEditableList *list)
{
	guint position;

	g_return_if_fail (list != NULL);
	position = fabulor_editable_list_get_n_rows (list);
	fabulor_editable_list_append (list, "", "");
	{
		FabulorEditableRow *row = editable_row_at (list, position);
		g_set_object (&list->edit_row, G_OBJECT (row));
		g_object_unref (row);
		fabulor_editable_list_set_selected (list, position);
		if (list->view)
			gtk_column_view_scroll_to (GTK_COLUMN_VIEW (list->view), position,
				NULL, GTK_LIST_SCROLL_FOCUS, NULL);
	}
}

static gboolean
editable_list_selected_position (FabulorEditableList *list, guint *position)
{
	guint selected = gtk_single_selection_get_selected (GTK_SINGLE_SELECTION (
		editable_selection (list)));
	if (selected == GTK_INVALID_LIST_POSITION)
		return FALSE;
	*position = selected;
	return TRUE;
}

gboolean
fabulor_editable_list_remove_selected (FabulorEditableList *list)
{
	guint position;
	guint count;

	g_return_val_if_fail (list != NULL, FALSE);
	if (!editable_list_selected_position (list, &position))
		return FALSE;
	g_list_store_remove (editable_store (list), position);
	count = fabulor_editable_list_get_n_rows (list);
	if (position < count)
		fabulor_editable_list_set_selected (list, position);
	return TRUE;
}

gboolean
fabulor_editable_list_move_selected (FabulorEditableList *list, gint delta)
{
	guint position;
	gint target;

	g_return_val_if_fail (list != NULL, FALSE);
	if ((delta != -1 && delta != 1) ||
		!editable_list_selected_position (list, &position))
		return FALSE;
	target = (gint) position + delta;
	if (target < 0 || (guint) target >= fabulor_editable_list_get_n_rows (list))
		return FALSE;
	{
		FabulorEditableRow *row = editable_row_at (list, position);
		g_list_store_remove (editable_store (list), position);
		g_list_store_insert (editable_store (list), (guint) target, row);
		g_object_unref (row);
	}
	fabulor_editable_list_set_selected (list, (guint) target);
	return TRUE;
}

gboolean
fabulor_editable_list_set_text_at (FabulorEditableList *list, guint position,
	FabulorEditableListField field, const gchar *text)
{
	g_return_val_if_fail (list != NULL, FALSE);
	if (position >= fabulor_editable_list_get_n_rows (list) ||
		(field != FABULOR_EDITABLE_LIST_NAME &&
		 field != FABULOR_EDITABLE_LIST_COMMAND))
		return FALSE;
	{
		FabulorEditableRow *row = editable_row_at (list, position);
		editable_row_set_text (row, field, text);
		g_object_unref (row);
	}
	return TRUE;
}

GPtrArray *
fabulor_editable_list_dup_all (FabulorEditableList *list)
{
	GPtrArray *records = g_ptr_array_new_with_free_func (
		(GDestroyNotify) fabulor_editable_list_record_free);
	guint count;
	guint i;

	g_return_val_if_fail (list != NULL, records);
	count = fabulor_editable_list_get_n_rows (list);
	for (i = 0; i < count; i++)
	{
		FabulorEditableListRecord *record = g_new0 (
			FabulorEditableListRecord, 1);
		FabulorEditableRow *row = editable_row_at (list, i);
		record->name = g_strdup (row->name);
		record->command = g_strdup (row->command);
		g_object_unref (row);
		g_ptr_array_add (records, record);
	}
	return records;
}
