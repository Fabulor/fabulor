/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "sound-event-list.h"

#include "gtk-compat.h"

struct _FabulorSoundEventList
{
	GtkWidget *view;
	FabulorSoundEventSelectionFunc selection_func;
	gpointer callback_data;
#if GTK_MAJOR_VERSION >= 4
	GListStore *store;
	GtkSingleSelection *selection;
#else
	GtkListStore *store;
#endif
};

#if GTK_MAJOR_VERSION >= 4

typedef struct _FabulorSoundEventRow FabulorSoundEventRow;
typedef struct _FabulorSoundEventRowClass FabulorSoundEventRowClass;

struct _FabulorSoundEventRow
{
	GObject parent_instance;
	gchar *event_name;
	gchar *sound_file;
	gint event_index;
};

struct _FabulorSoundEventRowClass { GObjectClass parent_class; };

enum
{
	PROP_SOUND_EVENT_ROW_0,
	PROP_SOUND_EVENT_ROW_EVENT_NAME,
	PROP_SOUND_EVENT_ROW_SOUND_FILE,
	PROP_SOUND_EVENT_ROW_EVENT_INDEX,
	N_SOUND_EVENT_ROW_PROPERTIES
};

static GParamSpec *sound_event_row_properties[N_SOUND_EVENT_ROW_PROPERTIES];

#define FABULOR_TYPE_SOUND_EVENT_ROW (fabulor_sound_event_row_get_type ())
#define FABULOR_SOUND_EVENT_ROW(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), \
	FABULOR_TYPE_SOUND_EVENT_ROW, FabulorSoundEventRow))

G_DEFINE_TYPE (FabulorSoundEventRow, fabulor_sound_event_row, G_TYPE_OBJECT)

static void
fabulor_sound_event_row_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	FabulorSoundEventRow *row = FABULOR_SOUND_EVENT_ROW (object);
	if (property_id == PROP_SOUND_EVENT_ROW_EVENT_NAME)
		g_value_set_string (value, row->event_name);
	else if (property_id == PROP_SOUND_EVENT_ROW_SOUND_FILE)
		g_value_set_string (value, row->sound_file);
	else if (property_id == PROP_SOUND_EVENT_ROW_EVENT_INDEX)
		g_value_set_int (value, row->event_index);
	else
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
fabulor_sound_event_row_finalize (GObject *object)
{
	FabulorSoundEventRow *row = FABULOR_SOUND_EVENT_ROW (object);
	g_free (row->event_name);
	g_free (row->sound_file);
	G_OBJECT_CLASS (fabulor_sound_event_row_parent_class)->finalize (object);
}

static void
fabulor_sound_event_row_class_init (FabulorSoundEventRowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->get_property = fabulor_sound_event_row_get_property;
	object_class->finalize = fabulor_sound_event_row_finalize;
	sound_event_row_properties[PROP_SOUND_EVENT_ROW_EVENT_NAME] =
		g_param_spec_string ("event-name", "Event name", "Sound event name",
			NULL, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	sound_event_row_properties[PROP_SOUND_EVENT_ROW_SOUND_FILE] =
		g_param_spec_string ("sound-file", "Sound file", "Sound event file",
			NULL, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	sound_event_row_properties[PROP_SOUND_EVENT_ROW_EVENT_INDEX] =
		g_param_spec_int ("event-index", "Event index", "Stable event index",
			G_MININT, G_MAXINT, -1,
			G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties (object_class,
		N_SOUND_EVENT_ROW_PROPERTIES, sound_event_row_properties);
}

static void fabulor_sound_event_row_init (FabulorSoundEventRow *row)
{
	row->event_index = -1;
}

static FabulorSoundEventRow *
sound_event_row_new (const gchar *event_name, const gchar *sound_file,
	gint event_index)
{
	FabulorSoundEventRow *row = g_object_new (FABULOR_TYPE_SOUND_EVENT_ROW, NULL);
	row->event_name = g_strdup (event_name ? event_name : "");
	row->sound_file = g_strdup (sound_file ? sound_file : "");
	row->event_index = event_index;
	return row;
}

static FabulorSoundEventRow *
sound_event_row_at (FabulorSoundEventList *list, guint position)
{
	return g_list_model_get_item (G_LIST_MODEL (list->store), position);
}

typedef struct
{
	const gchar *property;
	GParamSpec *pspec;
} SoundEventColumnData;

typedef struct
{
	GtkLabel *label;
	FabulorSoundEventRow *row;
	SoundEventColumnData *column;
	gulong notify_handler;
} SoundEventBinding;

static void
sound_event_binding_refresh (SoundEventBinding *binding)
{
	const gchar *text = binding->column->pspec ==
		sound_event_row_properties[PROP_SOUND_EVENT_ROW_EVENT_NAME] ?
		binding->row->event_name : binding->row->sound_file;
	gtk_label_set_text (binding->label, text);
}

static void
sound_event_row_changed (GObject *object, GParamSpec *pspec, gpointer user_data)
{
	SoundEventBinding *binding = user_data;
	(void) object;
	if (binding->row && pspec == binding->column->pspec)
		sound_event_binding_refresh (binding);
}

static void
sound_event_binding_free (gpointer data)
{
	SoundEventBinding *binding = data;
	if (binding->row && binding->notify_handler)
		g_signal_handler_disconnect (binding->row, binding->notify_handler);
	g_clear_object (&binding->row);
	g_free (binding);
}

static void
sound_event_factory_setup (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	SoundEventColumnData *column = user_data;
	SoundEventBinding *binding = g_new0 (SoundEventBinding, 1);
	GtkWidget *label = gtk_label_new (NULL);
	(void) factory;
	binding->label = GTK_LABEL (label);
	binding->column = column;
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_hexpand (label, TRUE);
	gtk_list_item_set_child (item, label);
	g_object_set_data_full (G_OBJECT (item), "fabulor-sound-event-binding",
		binding, sound_event_binding_free);
}

static void
sound_event_factory_bind (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	SoundEventBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-sound-event-binding");
	(void) factory;
	(void) user_data;
	if (binding->row && binding->notify_handler)
		g_signal_handler_disconnect (binding->row, binding->notify_handler);
	g_clear_object (&binding->row);
	binding->row = g_object_ref (FABULOR_SOUND_EVENT_ROW (
		gtk_list_item_get_item (item)));
	binding->notify_handler = g_signal_connect (binding->row,
		binding->column->property, G_CALLBACK (sound_event_row_changed), binding);
	sound_event_binding_refresh (binding);
}

static void
sound_event_factory_unbind (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	SoundEventBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-sound-event-binding");
	(void) factory;
	(void) user_data;
	if (binding->row && binding->notify_handler)
		g_signal_handler_disconnect (binding->row, binding->notify_handler);
	binding->notify_handler = 0;
	g_clear_object (&binding->row);
}

static GtkColumnViewColumn *
sound_event_column_new (const gchar *title, const gchar *property,
	GParamSpec *pspec, gboolean expand)
{
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	SoundEventColumnData *data = g_new (SoundEventColumnData, 1);
	GtkColumnViewColumn *column;
	data->property = property;
	data->pspec = pspec;
	g_object_set_data_full (G_OBJECT (factory), "fabulor-sound-event-column",
		data, g_free);
	g_signal_connect (factory, "setup", G_CALLBACK (sound_event_factory_setup), data);
	g_signal_connect (factory, "bind", G_CALLBACK (sound_event_factory_bind), data);
	g_signal_connect (factory, "unbind", G_CALLBACK (sound_event_factory_unbind), data);
	column = gtk_column_view_column_new (title ? title : "", factory);
	gtk_column_view_column_set_expand (column, expand);
	gtk_column_view_column_set_resizable (column, TRUE);
	return column;
}

static void
sound_event_selection_changed (GtkSingleSelection *selection,
	GParamSpec *pspec, gpointer user_data)
{
	FabulorSoundEventList *list = user_data;
	(void) selection;
	(void) pspec;
	if (list->selection_func)
		list->selection_func (fabulor_sound_event_list_get_selected_event (list),
			list->callback_data);
}

#else

enum
{
	SOUND_EVENT_NAME_COLUMN,
	SOUND_EVENT_FILE_COLUMN,
	SOUND_EVENT_INDEX_COLUMN,
	N_SOUND_EVENT_COLUMNS
};

static void
sound_event_selection_changed (GtkTreeSelection *selection, gpointer user_data)
{
	FabulorSoundEventList *list = user_data;
	(void) selection;
	if (list->selection_func)
		list->selection_func (fabulor_sound_event_list_get_selected_event (list),
			list->callback_data);
}

#endif

FabulorSoundEventList *
fabulor_sound_event_list_new (FabulorSoundEventSelectionFunc selection_func,
	gpointer user_data)
{
	FabulorSoundEventList *list = g_new0 (FabulorSoundEventList, 1);
	list->selection_func = selection_func;
	list->callback_data = user_data;
#if GTK_MAJOR_VERSION >= 4
	list->store = g_list_store_new (FABULOR_TYPE_SOUND_EVENT_ROW);
	list->selection = gtk_single_selection_new (G_LIST_MODEL (g_object_ref (list->store)));
	gtk_single_selection_set_autoselect (list->selection, FALSE);
	g_signal_connect (list->selection, "notify::selected",
		G_CALLBACK (sound_event_selection_changed), list);
#else
	list->store = gtk_list_store_new (N_SOUND_EVENT_COLUMNS, G_TYPE_STRING,
		G_TYPE_STRING, G_TYPE_INT);
#endif
	return list;
}

void
fabulor_sound_event_list_free (FabulorSoundEventList *list)
{
	if (!list)
		return;
#if GTK_MAJOR_VERSION >= 4
	g_clear_object (&list->selection);
#endif
	g_clear_object (&list->store);
	g_free (list);
}

GtkWidget *
fabulor_sound_event_list_create_view (FabulorSoundEventList *list,
	GtkBox *parent, const gchar *event_title, const gchar *file_title)
{
	GtkWidget *scroller;
	g_return_val_if_fail (list && GTK_IS_BOX (parent) && !list->view, NULL);
#if GTK_MAJOR_VERSION >= 4
	scroller = gtk_scrolled_window_new ();
	list->view = gtk_column_view_new (GTK_SELECTION_MODEL (g_object_ref (list->selection)));
	gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view),
		sound_event_column_new (event_title, "notify::event-name",
			sound_event_row_properties[PROP_SOUND_EVENT_ROW_EVENT_NAME], FALSE));
	gtk_column_view_append_column (GTK_COLUMN_VIEW (list->view),
		sound_event_column_new (file_title, "notify::sound-file",
			sound_event_row_properties[PROP_SOUND_EVENT_ROW_SOUND_FILE], TRUE));
	gtk_column_view_set_show_row_separators (GTK_COLUMN_VIEW (list->view), TRUE);
#else
	{
		GtkCellRenderer *renderer;
		scroller = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroller), GTK_SHADOW_IN);
		list->view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (g_object_ref (list->store)));
		gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (list->view)),
			GTK_SELECTION_SINGLE);
		g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (list->view)),
			"changed", G_CALLBACK (sound_event_selection_changed), list);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (list->view), -1,
			event_title, renderer, "text", SOUND_EVENT_NAME_COLUMN, NULL);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (list->view), -1,
			file_title, renderer, "text", SOUND_EVENT_FILE_COLUMN, NULL);
		gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (list->view),
			GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);
	}
#endif
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroller), list->view);
	fabulor_gtk_box_append (parent, scroller, TRUE, TRUE, 0);
#if GTK_MAJOR_VERSION < 4
	gtk_widget_show_all (scroller);
#endif
	return list->view;
}

void
fabulor_sound_event_list_append (FabulorSoundEventList *list,
	const gchar *event_name, const gchar *sound_file, gint event_index)
{
	g_return_if_fail (list != NULL);
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorSoundEventRow *row = sound_event_row_new (event_name, sound_file,
			event_index);
		g_list_store_append (list->store, row);
		g_object_unref (row);
	}
#else
	{
		GtkTreeIter iter;
		gtk_list_store_append (list->store, &iter);
		gtk_list_store_set (list->store, &iter,
			SOUND_EVENT_NAME_COLUMN, event_name ? event_name : "",
			SOUND_EVENT_FILE_COLUMN, sound_file ? sound_file : "",
			SOUND_EVENT_INDEX_COLUMN, event_index, -1);
	}
#endif
}

void
fabulor_sound_event_list_clear (FabulorSoundEventList *list)
{
	g_return_if_fail (list != NULL);
#if GTK_MAJOR_VERSION >= 4
	g_list_store_remove_all (list->store);
#else
	gtk_list_store_clear (list->store);
#endif
}

guint
fabulor_sound_event_list_get_n_rows (FabulorSoundEventList *list)
{
	g_return_val_if_fail (list != NULL, 0);
#if GTK_MAJOR_VERSION >= 4
	return g_list_model_get_n_items (G_LIST_MODEL (list->store));
#else
	return (guint) gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list->store), NULL);
#endif
}

static gint
sound_event_index_at (FabulorSoundEventList *list, guint position)
{
	gint event_index = -1;
#if GTK_MAJOR_VERSION >= 4
	FabulorSoundEventRow *row = sound_event_row_at (list, position);
	event_index = row->event_index;
	g_object_unref (row);
#else
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store), &iter, NULL,
		(gint) position);
	gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
		SOUND_EVENT_INDEX_COLUMN, &event_index, -1);
#endif
	return event_index;
}

static gboolean
sound_event_find (FabulorSoundEventList *list, gint event_index,
	guint *position)
{
	guint i;
	for (i = 0; i < fabulor_sound_event_list_get_n_rows (list); i++)
		if (sound_event_index_at (list, i) == event_index)
		{
			if (position)
				*position = i;
			return TRUE;
		}
	return FALSE;
}

gboolean
fabulor_sound_event_list_update_file (FabulorSoundEventList *list,
	gint event_index, const gchar *sound_file)
{
	guint position;
	g_return_val_if_fail (list != NULL, FALSE);
	if (!sound_event_find (list, event_index, &position))
		return FALSE;
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorSoundEventRow *row = sound_event_row_at (list, position);
		if (g_strcmp0 (row->sound_file, sound_file ? sound_file : "") != 0)
		{
			g_free (row->sound_file);
			row->sound_file = g_strdup (sound_file ? sound_file : "");
			g_object_notify_by_pspec (G_OBJECT (row),
				sound_event_row_properties[PROP_SOUND_EVENT_ROW_SOUND_FILE]);
		}
		g_object_unref (row);
	}
#else
	{
		GtkTreeIter iter;
		gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store), &iter, NULL,
			(gint) position);
		gtk_list_store_set (list->store, &iter, SOUND_EVENT_FILE_COLUMN,
			sound_file ? sound_file : "", -1);
	}
#endif
	return TRUE;
}

gboolean
fabulor_sound_event_list_select_event (FabulorSoundEventList *list,
	gint event_index)
{
	guint position;
	g_return_val_if_fail (list != NULL, FALSE);
	if (!sound_event_find (list, event_index, &position))
		return FALSE;
#if GTK_MAJOR_VERSION >= 4
	if (!gtk_selection_model_select_item (GTK_SELECTION_MODEL (list->selection),
		position, TRUE))
		return FALSE;
	if (list->view)
		gtk_column_view_scroll_to (GTK_COLUMN_VIEW (list->view), position, NULL,
			GTK_LIST_SCROLL_FOCUS, NULL);
#else
	if (list->view)
	{
		GtkTreePath *path = gtk_tree_path_new_from_indices ((gint) position, -1);
		gtk_tree_view_set_cursor (GTK_TREE_VIEW (list->view), path, NULL, FALSE);
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (list->view), path, NULL,
			TRUE, 0.5, 0.5);
		gtk_tree_path_free (path);
	}
#endif
	return TRUE;
}

gint
fabulor_sound_event_list_get_selected_event (FabulorSoundEventList *list)
{
	g_return_val_if_fail (list != NULL, -1);
#if GTK_MAJOR_VERSION >= 4
	{
		guint position = gtk_single_selection_get_selected (list->selection);
		return position == GTK_INVALID_LIST_POSITION ? -1 :
			sound_event_index_at (list, position);
	}
#else
	if (list->view)
	{
		GtkTreeModel *model;
		GtkTreeIter iter;
		gint event_index;
		if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection (
			GTK_TREE_VIEW (list->view)), &model, &iter))
			return -1;
		gtk_tree_model_get (model, &iter, SOUND_EVENT_INDEX_COLUMN,
			&event_index, -1);
		return event_index;
	}
	return -1;
#endif
}

gchar *
fabulor_sound_event_list_dup_file (FabulorSoundEventList *list,
	gint event_index)
{
	guint position;
	gchar *sound_file = NULL;
	g_return_val_if_fail (list != NULL, NULL);
	if (!sound_event_find (list, event_index, &position))
		return NULL;
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorSoundEventRow *row = sound_event_row_at (list, position);
		sound_file = g_strdup (row->sound_file);
		g_object_unref (row);
	}
#else
	{
		GtkTreeIter iter;
		gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store), &iter, NULL,
			(gint) position);
		gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
			SOUND_EVENT_FILE_COLUMN, &sound_file, -1);
	}
#endif
	return sound_file;
}
