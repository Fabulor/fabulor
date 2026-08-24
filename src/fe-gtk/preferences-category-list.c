/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "preferences-category-list.h"

#include "gtk-compat.h"

#define PREFERENCES_CATEGORY_MIN_WIDTH 228

typedef struct _FabulorPreferencesCategoryRow FabulorPreferencesCategoryRow;
typedef struct _FabulorPreferencesCategoryRowClass FabulorPreferencesCategoryRowClass;

struct _FabulorPreferencesCategoryRow
{
	GObject parent_instance;
	gchar *title;
	gint page_index;
	GPtrArray *children;
	GListStore *child_store;
};

struct _FabulorPreferencesCategoryRowClass
{
	GObjectClass parent_class;
};

G_DEFINE_TYPE (FabulorPreferencesCategoryRow,
	fabulor_preferences_category_row, G_TYPE_OBJECT)

struct _FabulorPreferencesCategoryList
{
	GPtrArray *categories;
	guint page_count;
	gint selected_page;
	GtkWidget *view;
	FabulorPreferencesCategorySelectionFunc selection_func;
	gpointer callback_data;
	gboolean changing_selection;
	GListStore *roots;
	GtkTreeListModel *tree;
	GtkSingleSelection *selection;
	gulong selection_id;
};

static void
fabulor_preferences_category_row_finalize (GObject *object)
{
	FabulorPreferencesCategoryRow *row =
		(FabulorPreferencesCategoryRow *) object;

	g_free (row->title);
	g_ptr_array_unref (row->children);
	g_clear_object (&row->child_store);
	G_OBJECT_CLASS (fabulor_preferences_category_row_parent_class)->finalize (
		object);
}

static void
fabulor_preferences_category_row_class_init (
	FabulorPreferencesCategoryRowClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize =
		fabulor_preferences_category_row_finalize;
}

static void
fabulor_preferences_category_row_init (FabulorPreferencesCategoryRow *row)
{
	row->page_index = -1;
	row->children = g_ptr_array_new_with_free_func (g_object_unref);
	row->child_store = g_list_store_new (
		fabulor_preferences_category_row_get_type ());
}

static FabulorPreferencesCategoryRow *
preferences_category_row_new (const gchar *title, gint page_index)
{
	FabulorPreferencesCategoryRow *row = g_object_new (
		fabulor_preferences_category_row_get_type (), NULL);

	row->title = g_strdup (title ? title : "");
	row->page_index = page_index;
	return row;
}

static FabulorPreferencesCategoryRow *
preferences_category_find_page (FabulorPreferencesCategoryList *list,
	gint page_index)
{
	guint i;

	for (i = 0; i < list->categories->len; i++)
	{
		FabulorPreferencesCategoryRow *category =
			g_ptr_array_index (list->categories, i);
		guint j;

		for (j = 0; j < category->children->len; j++)
		{
			FabulorPreferencesCategoryRow *page =
				g_ptr_array_index (category->children, j);
			if (page->page_index == page_index)
				return page;
		}
	}
	return NULL;
}

static void
preferences_category_commit_selection (FabulorPreferencesCategoryList *list,
	gint page_index)
{
	if (list->selected_page == page_index)
		return;
	list->selected_page = page_index;
	if (list->selection_func)
		list->selection_func (page_index, list->callback_data);
}


static GListModel *
preferences_category_create_children (gpointer item, gpointer user_data)
{
	FabulorPreferencesCategoryRow *row = item;
	(void) user_data;

	return row->page_index < 0 ?
		G_LIST_MODEL (g_object_ref (row->child_store)) : NULL;
}

static FabulorPreferencesCategoryRow *
preferences_category_row_at (FabulorPreferencesCategoryList *list,
	guint position)
{
	GtkTreeListRow *tree_row = gtk_tree_list_model_get_row (list->tree,
		position);
	FabulorPreferencesCategoryRow *row = tree_row ?
		gtk_tree_list_row_get_item (tree_row) : NULL;

	if (row)
		g_object_ref (row);
	g_clear_object (&tree_row);
	return row;
}

static gboolean
preferences_category_find_position (FabulorPreferencesCategoryList *list,
	gint page_index, guint *position)
{
	guint count = g_list_model_get_n_items (G_LIST_MODEL (list->tree));
	guint i;

	for (i = 0; i < count; i++)
	{
		FabulorPreferencesCategoryRow *row =
			preferences_category_row_at (list, i);
		gboolean found = row && row->page_index == page_index;
		g_clear_object (&row);
		if (found)
		{
			if (position)
				*position = i;
			return TRUE;
		}
	}
	return FALSE;
}

static void
preferences_category_restore_selection (FabulorPreferencesCategoryList *list)
{
	guint position;

	list->changing_selection = TRUE;
	if (list->selected_page >= 0 && preferences_category_find_position (list,
		list->selected_page, &position))
		gtk_single_selection_set_selected (list->selection, position);
	else
		gtk_single_selection_set_selected (list->selection,
			GTK_INVALID_LIST_POSITION);
	list->changing_selection = FALSE;
}

static void
preferences_category_selection_changed (GtkSingleSelection *selection,
	GParamSpec *pspec, gpointer user_data)
{
	FabulorPreferencesCategoryList *list = user_data;
	guint position;
	FabulorPreferencesCategoryRow *row;

	(void) pspec;
	if (list->changing_selection)
		return;
	position = gtk_single_selection_get_selected (selection);
	row = position == GTK_INVALID_LIST_POSITION ? NULL :
		preferences_category_row_at (list, position);
	if (!row || row->page_index < 0)
		preferences_category_restore_selection (list);
	else
		preferences_category_commit_selection (list, row->page_index);
	g_clear_object (&row);
}

static void
preferences_category_factory_setup (GtkSignalListItemFactory *factory,
	GtkListItem *list_item, gpointer user_data)
{
	GtkWidget *expander = gtk_tree_expander_new ();
	GtkWidget *label = gtk_label_new (NULL);

	(void) factory;
	(void) user_data;
	gtk_label_set_xalign (GTK_LABEL (label), 0.0f);
	gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
	gtk_widget_set_hexpand (label, TRUE);
	gtk_widget_set_margin_top (expander, 2);
	gtk_widget_set_margin_bottom (expander, 2);
	gtk_widget_set_margin_start (expander, 4);
	gtk_widget_set_margin_end (expander, 4);
	gtk_tree_expander_set_child (GTK_TREE_EXPANDER (expander), label);
	gtk_list_item_set_child (list_item, expander);
}

static void
preferences_category_factory_bind (GtkSignalListItemFactory *factory,
	GtkListItem *list_item, gpointer user_data)
{
	GtkTreeListRow *tree_row = gtk_list_item_get_item (list_item);
	FabulorPreferencesCategoryRow *row =
		gtk_tree_list_row_get_item (tree_row);
	GtkWidget *expander = gtk_list_item_get_child (list_item);
	GtkWidget *label = gtk_tree_expander_get_child (
		GTK_TREE_EXPANDER (expander));
	PangoAttrList *attributes = NULL;

	(void) factory;
	(void) user_data;
	gtk_tree_expander_set_list_row (GTK_TREE_EXPANDER (expander), tree_row);
	gtk_label_set_text (GTK_LABEL (label), row->title);
	gtk_widget_set_tooltip_text (label, row->title);
	if (row->page_index < 0)
	{
		attributes = pango_attr_list_new ();
		pango_attr_list_insert (attributes,
			pango_attr_weight_new (PANGO_WEIGHT_SEMIBOLD));
	}
	gtk_label_set_attributes (GTK_LABEL (label), attributes);
	if (attributes)
		pango_attr_list_unref (attributes);
}

static void
preferences_category_factory_unbind (GtkSignalListItemFactory *factory,
	GtkListItem *list_item, gpointer user_data)
{
	GtkWidget *expander = gtk_list_item_get_child (list_item);

	(void) factory;
	(void) user_data;
	gtk_tree_expander_set_list_row (GTK_TREE_EXPANDER (expander), NULL);
	gtk_widget_set_tooltip_text (gtk_tree_expander_get_child (
		GTK_TREE_EXPANDER (expander)), NULL);
}


FabulorPreferencesCategoryList *
fabulor_preferences_category_list_new (
	FabulorPreferencesCategorySelectionFunc selection_func, gpointer user_data)
{
	FabulorPreferencesCategoryList *list = g_new0 (
		FabulorPreferencesCategoryList, 1);

	list->categories = g_ptr_array_new_with_free_func (g_object_unref);
	list->selected_page = -1;
	list->selection_func = selection_func;
	list->callback_data = user_data;
	list->roots = g_list_store_new (
		fabulor_preferences_category_row_get_type ());
	list->tree = gtk_tree_list_model_new (
		G_LIST_MODEL (g_object_ref (list->roots)), FALSE, TRUE,
		preferences_category_create_children, NULL, NULL);
	list->selection = gtk_single_selection_new (
		G_LIST_MODEL (g_object_ref (list->tree)));
	gtk_single_selection_set_autoselect (list->selection, FALSE);
	gtk_single_selection_set_can_unselect (list->selection, TRUE);
	list->selection_id = g_signal_connect (list->selection, "notify::selected",
		G_CALLBACK (preferences_category_selection_changed), list);
	return list;
}

void
fabulor_preferences_category_list_free (
	FabulorPreferencesCategoryList *list)
{
	if (!list)
		return;
	if (list->selection_id)
		g_signal_handler_disconnect (list->selection, list->selection_id);
	g_clear_object (&list->selection);
	g_clear_object (&list->tree);
	g_clear_object (&list->roots);
	g_ptr_array_unref (list->categories);
	g_free (list);
}

guint
fabulor_preferences_category_list_append_category (
	FabulorPreferencesCategoryList *list, const gchar *title)
{
	FabulorPreferencesCategoryRow *category;
	guint position;

	g_return_val_if_fail (list != NULL, G_MAXUINT);
	category = preferences_category_row_new (title, -1);
	position = list->categories->len;
	g_ptr_array_add (list->categories, category);
	g_list_store_append (list->roots, category);
	return position;
}

void
fabulor_preferences_category_list_append_page (
	FabulorPreferencesCategoryList *list, guint category_position,
	const gchar *title, gint page_index)
{
	FabulorPreferencesCategoryRow *category;
	FabulorPreferencesCategoryRow *page;

	g_return_if_fail (list != NULL);
	g_return_if_fail (category_position < list->categories->len);
	g_return_if_fail (page_index >= 0);
	g_return_if_fail (preferences_category_find_page (list, page_index) == NULL);
	category = g_ptr_array_index (list->categories, category_position);
	page = preferences_category_row_new (title, page_index);
	g_ptr_array_add (category->children, page);
	g_list_store_append (category->child_store, page);
	list->page_count++;
}

GtkWidget *
fabulor_preferences_category_list_create_view (
	FabulorPreferencesCategoryList *list, GtkBox *parent,
	const gchar *column_title)
{
	GtkWidget *frame;

	g_return_val_if_fail (list && GTK_IS_BOX (parent) && !list->view, NULL);
	{
		GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
		g_signal_connect (factory, "setup",
			G_CALLBACK (preferences_category_factory_setup), list);
		g_signal_connect (factory, "bind",
			G_CALLBACK (preferences_category_factory_bind), list);
		g_signal_connect (factory, "unbind",
			G_CALLBACK (preferences_category_factory_unbind), list);
		list->view = gtk_list_view_new (
			GTK_SELECTION_MODEL (g_object_ref (list->selection)), factory);
		frame = gtk_frame_new (column_title);
	}
	gtk_widget_set_size_request (frame, PREFERENCES_CATEGORY_MIN_WIDTH, -1);
	fabulor_gtk_frame_set_child (GTK_FRAME (frame), list->view);
	fabulor_gtk_box_append (parent, frame, FALSE, FALSE, 0);
	return list->view;
}

guint
fabulor_preferences_category_list_get_n_categories (
	FabulorPreferencesCategoryList *list)
{
	g_return_val_if_fail (list != NULL, 0);
	return list->categories->len;
}

guint
fabulor_preferences_category_list_get_n_pages (
	FabulorPreferencesCategoryList *list)
{
	g_return_val_if_fail (list != NULL, 0);
	return list->page_count;
}

gboolean
fabulor_preferences_category_list_select_page (
	FabulorPreferencesCategoryList *list, gint page_index)
{
	g_return_val_if_fail (list != NULL, FALSE);
	if (!preferences_category_find_page (list, page_index))
		return FALSE;
	{
		guint position;
		if (!preferences_category_find_position (list, page_index, &position))
			return FALSE;
		if (gtk_single_selection_get_selected (list->selection) != position)
			gtk_single_selection_set_selected (list->selection, position);
		preferences_category_commit_selection (list, page_index);
	}
	return TRUE;
}

gint
fabulor_preferences_category_list_get_selected_page (
	FabulorPreferencesCategoryList *list)
{
	g_return_val_if_fail (list != NULL, -1);
	return list->selected_page;
}
