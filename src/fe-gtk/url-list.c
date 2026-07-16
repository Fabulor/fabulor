/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "url-list.h"

#include "gtk-compat.h"

#if GTK_MAJOR_VERSION >= 4
#include "gtk4-list-models.h"
#else
#include "gtkutil.h"
#endif

struct _FabulorUrlList
{
	GtkWidget *view;
#if GTK_MAJOR_VERSION >= 4
	FabulorGtk4FlatModelStack *models;
#else
	GtkListStore *store;
#endif
};

#if GTK_MAJOR_VERSION >= 4

typedef struct _FabulorUrlRow FabulorUrlRow;
typedef struct _FabulorUrlRowClass FabulorUrlRowClass;

struct _FabulorUrlRow
{
	GObject parent_instance;
	gchar *url;
};

struct _FabulorUrlRowClass
{
	GObjectClass parent_class;
};

#define FABULOR_TYPE_URL_ROW (fabulor_url_row_get_type ())
#define FABULOR_URL_ROW(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), FABULOR_TYPE_URL_ROW, FabulorUrlRow))

G_DEFINE_TYPE (FabulorUrlRow, fabulor_url_row, G_TYPE_OBJECT)

static GQuark
url_list_item_quark (void)
{
	return g_quark_from_static_string ("fabulor-url-list-item");
}

static void
fabulor_url_row_finalize (GObject *object)
{
	FabulorUrlRow *row = FABULOR_URL_ROW (object);

	g_free (row->url);
	G_OBJECT_CLASS (fabulor_url_row_parent_class)->finalize (object);
}

static void
fabulor_url_row_class_init (FabulorUrlRowClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = fabulor_url_row_finalize;
}

static void
fabulor_url_row_init (FabulorUrlRow *row)
{
	(void) row;
}

static FabulorUrlRow *
url_row_new (const gchar *url)
{
	FabulorUrlRow *row = g_object_new (FABULOR_TYPE_URL_ROW, NULL);

	row->url = g_strdup (url);
	return row;
}

static void
url_factory_setup (GtkSignalListItemFactory *factory, GtkListItem *item,
				   gpointer user_data)
{
	GtkWidget *label = gtk_label_new (NULL);

	(void) factory;
	(void) user_data;
	gtk_label_set_xalign (GTK_LABEL (label), 0.0f);
	gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
	gtk_list_item_set_child (item, label);
}

static void
url_factory_bind (GtkSignalListItemFactory *factory, GtkListItem *item,
				  gpointer user_data)
{
	FabulorUrlRow *row = FABULOR_URL_ROW (gtk_list_item_get_item (item));
	GtkWidget *label = gtk_list_item_get_child (item);

	(void) factory;
	(void) user_data;
	gtk_label_set_text (GTK_LABEL (label), row->url);
	g_object_set_qdata (G_OBJECT (label), url_list_item_quark (), item);
}

static void
url_factory_unbind (GtkSignalListItemFactory *factory, GtkListItem *item,
					gpointer user_data)
{
	GtkWidget *label = gtk_list_item_get_child (item);

	(void) factory;
	(void) user_data;
	g_object_set_qdata (G_OBJECT (label), url_list_item_quark (), NULL);
}

static gboolean
url_list_position_at_point (FabulorUrlList *list, gdouble x, gdouble y,
						guint *position)
{
	GtkWidget *picked = gtk_widget_pick (list->view, x, y, GTK_PICK_DEFAULT);

	while (picked && picked != list->view)
	{
		GtkListItem *item = g_object_get_qdata (G_OBJECT (picked),
			url_list_item_quark ());
		if (item)
		{
			*position = gtk_list_item_get_position (item);
			return TRUE;
		}
		picked = gtk_widget_get_parent (picked);
	}
	return FALSE;
}

static FabulorUrlRow *
url_list_selected_row (FabulorUrlList *list)
{
	GtkSelectionModel *selection =
		fabulor_gtk4_flat_model_stack_get_selection (list->models);

	if (!GTK_IS_SINGLE_SELECTION (selection))
		return NULL;
	return FABULOR_URL_ROW (gtk_single_selection_get_selected_item (
		GTK_SINGLE_SELECTION (selection)));
}

#else

enum
{
	URL_LIST_COLUMN,
	N_URL_LIST_COLUMNS
};

#endif

FabulorUrlList *
fabulor_url_list_new (void)
{
	FabulorUrlList *list = g_new0 (FabulorUrlList, 1);

#if GTK_MAJOR_VERSION >= 4
	list->models = fabulor_gtk4_flat_model_stack_new (FABULOR_TYPE_URL_ROW,
		NULL, FABULOR_GTK4_SELECTION_SINGLE);
	if (!list->models)
	{
		g_free (list);
		return NULL;
	}
#else
	list->store = gtk_list_store_new (N_URL_LIST_COLUMNS, G_TYPE_STRING);
#endif
	return list;
}

void
fabulor_url_list_free (FabulorUrlList *list)
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
fabulor_url_list_create_view (FabulorUrlList *list, GtkBox *parent,
							  const gchar *title)
{
	g_return_val_if_fail (list != NULL, NULL);
	g_return_val_if_fail (GTK_IS_BOX (parent), NULL);
	g_return_val_if_fail (list->view == NULL, NULL);

#if GTK_MAJOR_VERSION >= 4
	{
		GtkWidget *scroller = gtk_scrolled_window_new ();
		GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
		GtkSelectionModel *selection =
			fabulor_gtk4_flat_model_stack_get_selection (list->models);

		(void) title;
		g_signal_connect (factory, "setup", G_CALLBACK (url_factory_setup), NULL);
		g_signal_connect (factory, "bind", G_CALLBACK (url_factory_bind), NULL);
		g_signal_connect (factory, "unbind", G_CALLBACK (url_factory_unbind), NULL);
		list->view = gtk_list_view_new (GTK_SELECTION_MODEL (
			g_object_ref (selection)), factory);
		fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroller),
			list->view);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_widget_set_hexpand (scroller, TRUE);
		gtk_widget_set_vexpand (scroller, TRUE);
		fabulor_gtk_box_append (parent, scroller, TRUE, TRUE, 0);
	}
#else
	list->view = gtkutil_treeview_new (parent,
		GTK_TREE_MODEL (g_object_ref (list->store)), NULL,
		URL_LIST_COLUMN, (gchar *) title, -1);
	if (list->view)
	{
		GtkWidget *scroller = gtk_widget_get_parent (list->view);
		gtk_widget_set_hexpand (scroller, TRUE);
		gtk_widget_set_vexpand (scroller, TRUE);
		gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (list->view), FALSE);
		gtk_widget_show (list->view);
	}
#endif
	return list->view;
}

GtkWidget *
fabulor_url_list_get_view (FabulorUrlList *list)
{
	g_return_val_if_fail (list != NULL, NULL);
	return list->view;
}

void
fabulor_url_list_clear (FabulorUrlList *list)
{
	g_return_if_fail (list != NULL);
#if GTK_MAJOR_VERSION >= 4
	fabulor_gtk4_flat_model_stack_clear (list->models);
#else
	gtk_list_store_clear (list->store);
#endif
}

void
fabulor_url_list_prepend (FabulorUrlList *list, const gchar *url, guint limit)
{
	g_return_if_fail (list != NULL);
	g_return_if_fail (url != NULL);

#if GTK_MAJOR_VERSION >= 4
	{
		GListStore *store = fabulor_gtk4_flat_model_stack_get_store (list->models);
		FabulorUrlRow *row = url_row_new (url);
		guint count;

		g_list_store_insert (store, 0, row);
		g_object_unref (row);
		count = g_list_model_get_n_items (G_LIST_MODEL (store));
		if (limit > 0 && count > limit)
			g_list_store_splice (store, limit, count - limit, NULL, 0);
	}
#else
	{
		GtkTreeIter iter;
		gboolean valid;

		gtk_list_store_prepend (list->store, &iter);
		gtk_list_store_set (list->store, &iter, URL_LIST_COLUMN, url, -1);
		if (limit > 0)
		{
			valid = gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store),
				&iter, NULL, (gint) limit);
			while (valid)
				valid = gtk_list_store_remove (list->store, &iter);
		}
	}
#endif
}

guint
fabulor_url_list_get_n_rows (FabulorUrlList *list)
{
	g_return_val_if_fail (list != NULL, 0);
#if GTK_MAJOR_VERSION >= 4
	return g_list_model_get_n_items (G_LIST_MODEL (
		fabulor_gtk4_flat_model_stack_get_store (list->models)));
#else
	return (guint) gtk_tree_model_iter_n_children (
		GTK_TREE_MODEL (list->store), NULL);
#endif
}

gchar *
fabulor_url_list_dup_at (FabulorUrlList *list, guint position)
{
	g_return_val_if_fail (list != NULL, NULL);
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorUrlRow *row = g_list_model_get_item (G_LIST_MODEL (
			fabulor_gtk4_flat_model_stack_get_store (list->models)), position);
		gchar *url;
		if (!row)
			return NULL;
		url = g_strdup (row->url);
		g_object_unref (row);
		return url;
	}
#else
	{
		GtkTreeIter iter;
		gchar *url = NULL;
		if (gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list->store), &iter,
			NULL, (gint) position))
			gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
				URL_LIST_COLUMN, &url, -1);
		return url;
	}
#endif
}

gchar *
fabulor_url_list_dup_selected (FabulorUrlList *list)
{
	g_return_val_if_fail (list != NULL, NULL);
#if GTK_MAJOR_VERSION >= 4
	{
		FabulorUrlRow *row = url_list_selected_row (list);
		return row ? g_strdup (row->url) : NULL;
	}
#else
	{
		GtkTreeIter iter;
		GtkTreeSelection *selection;
		gchar *url = NULL;
		if (!list->view)
			return NULL;
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (list->view));
		if (gtk_tree_selection_get_selected (selection, NULL, &iter))
			gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
				URL_LIST_COLUMN, &url, -1);
		return url;
	}
#endif
}

gchar *
fabulor_url_list_select_and_dup_at_point (FabulorUrlList *list,
								  gdouble x, gdouble y)
{
	g_return_val_if_fail (list != NULL, NULL);
	g_return_val_if_fail (list->view != NULL, NULL);
#if GTK_MAJOR_VERSION >= 4
	{
		GtkSelectionModel *selection =
			fabulor_gtk4_flat_model_stack_get_selection (list->models);
		GListModel *model = G_LIST_MODEL (
			fabulor_gtk4_flat_model_stack_get_store (list->models));
		FabulorUrlRow *row;
		gchar *url;
		guint position;

		if (!url_list_position_at_point (list, x, y, &position))
			return NULL;
		gtk_selection_model_select_item (selection, position, TRUE);
		row = g_list_model_get_item (model, position);
		if (!row)
			return NULL;
		url = g_strdup (row->url);
		g_object_unref (row);
		return url;
	}
#else
	{
		GtkTreePath *path;
		GtkTreeIter iter;
		GtkTreeSelection *selection;
		gchar *url = NULL;

		if (!gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (list->view),
			(gint) x, (gint) y, &path, NULL, NULL, NULL))
			return NULL;
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (list->view));
		gtk_tree_selection_unselect_all (selection);
		gtk_tree_selection_select_path (selection, path);
		if (gtk_tree_model_get_iter (GTK_TREE_MODEL (list->store), &iter, path))
			gtk_tree_model_get (GTK_TREE_MODEL (list->store), &iter,
				URL_LIST_COLUMN, &url, -1);
		gtk_tree_path_free (path);
		return url;
	}
#endif
}
