/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "channel-tree-view.h"

#define FABULOR_CHANNEL_TREE_VIEW_DATA "fabulor-channel-tree-view-data"

typedef struct
{
	GtkWidget *view;
	FabulorChannelModel *model;
	FabulorChannelTreeSelectionFunc selection_callback;
	gpointer selection_data;
	gulong selection_id;
	gboolean use_icons;
	gboolean compact;
} FabulorChannelTreeView;

static FabulorChannelTreeView *
channel_tree_view_data (GtkWidget *view)
{
	g_return_val_if_fail (GTK_IS_WIDGET (view), NULL);
	return g_object_get_data (G_OBJECT (view),
		FABULOR_CHANNEL_TREE_VIEW_DATA);
}

static void
channel_tree_view_free (gpointer data)
{
	FabulorChannelTreeView *owner = data;

#if GTK_MAJOR_VERSION >= 4
	if (owner->selection_id)
		g_signal_handler_disconnect (
			fabulor_channel_model_get_selection (owner->model),
			owner->selection_id);
#endif
	g_free (owner);
}

#if GTK_MAJOR_VERSION >= 4

typedef struct
{
	GtkWidget *expander;
	GtkWidget *box;
	GtkWidget *icon;
	GtkWidget *label;
	gpointer item;
	GdkPixbuf *icon_source;
	gulong notify_id;
} FabulorChannelTreeItemBinding;

static GQuark
channel_tree_item_quark (void)
{
	return g_quark_from_static_string ("fabulor-channel-tree-item");
}

static void
channel_tree_item_set_icon (FabulorChannelTreeItemBinding *binding,
	GdkPixbuf *pixbuf)
{
	GdkTexture *texture = NULL;

	if (binding->icon_source == pixbuf)
		return;
	binding->icon_source = pixbuf;
	if (pixbuf)
	{
		GBytes *bytes = g_bytes_new_with_free_func (
			gdk_pixbuf_get_pixels (pixbuf), gdk_pixbuf_get_byte_length (pixbuf),
			(GDestroyNotify) g_object_unref, g_object_ref (pixbuf));
		GdkMemoryFormat format = gdk_pixbuf_get_has_alpha (pixbuf) ?
			GDK_MEMORY_R8G8B8A8 : GDK_MEMORY_R8G8B8;

		texture = gdk_memory_texture_new (gdk_pixbuf_get_width (pixbuf),
			gdk_pixbuf_get_height (pixbuf), format, bytes,
			(gsize) gdk_pixbuf_get_rowstride (pixbuf));
		g_bytes_unref (bytes);
	}
	gtk_image_set_from_paintable (GTK_IMAGE (binding->icon),
		texture ? GDK_PAINTABLE (texture) : NULL);
	g_clear_object (&texture);
}

static void
channel_tree_item_update (FabulorChannelTreeItemBinding *binding)
{
	PangoAttrList *source = fabulor_channel_model_get_item_attributes (
		binding->item);
	PangoUnderline underline = fabulor_channel_model_get_item_underline (
		binding->item);
	PangoAttrList *attributes = source ? pango_attr_list_copy (source) : NULL;

	channel_tree_item_set_icon (binding,
		fabulor_channel_model_get_item_icon (binding->item));
	gtk_label_set_text (GTK_LABEL (binding->label),
		fabulor_channel_model_get_item_name (binding->item));
	if (underline != PANGO_UNDERLINE_NONE)
	{
		if (!attributes)
			attributes = pango_attr_list_new ();
		pango_attr_list_insert (attributes,
			pango_attr_underline_new (underline));
	}
	gtk_label_set_attributes (GTK_LABEL (binding->label), attributes);
	if (attributes)
		pango_attr_list_unref (attributes);
}

static void
channel_tree_item_notify (GObject *item, GParamSpec *pspec,
	gpointer user_data)
{
	(void) item;
	(void) pspec;
	channel_tree_item_update (user_data);
}

static void
channel_tree_factory_setup (GtkSignalListItemFactory *factory,
	GtkListItem *list_item, gpointer user_data)
{
	FabulorChannelTreeView *owner = user_data;
	FabulorChannelTreeItemBinding *binding = g_new0 (
		FabulorChannelTreeItemBinding, 1);
	gint spacing = owner->compact ? 1 : 3;

	(void) factory;
	binding->expander = gtk_tree_expander_new ();
	binding->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, spacing);
	binding->icon = gtk_image_new ();
	binding->label = gtk_label_new (NULL);
	gtk_label_set_xalign (GTK_LABEL (binding->label), 0.0f);
	gtk_label_set_ellipsize (GTK_LABEL (binding->label), PANGO_ELLIPSIZE_END);
	gtk_widget_set_hexpand (binding->label, TRUE);
	if (owner->use_icons)
		gtk_box_append (GTK_BOX (binding->box), binding->icon);
	gtk_box_append (GTK_BOX (binding->box), binding->label);
	gtk_tree_expander_set_child (GTK_TREE_EXPANDER (binding->expander),
		binding->box);
	gtk_list_item_set_child (list_item, binding->expander);
	g_object_set_data_full (G_OBJECT (list_item),
		"fabulor-channel-tree-item-binding", binding, g_free);
}

static void
channel_tree_factory_bind (GtkSignalListItemFactory *factory,
	GtkListItem *list_item, gpointer user_data)
{
	FabulorChannelTreeItemBinding *binding = g_object_get_data (
		G_OBJECT (list_item), "fabulor-channel-tree-item-binding");
	GtkTreeListRow *tree_row = gtk_list_item_get_item (list_item);

	(void) factory;
	(void) user_data;
	binding->item = gtk_tree_list_row_get_item (tree_row);
	binding->notify_id = g_signal_connect (binding->item, "notify",
		G_CALLBACK (channel_tree_item_notify), binding);
	gtk_tree_expander_set_list_row (GTK_TREE_EXPANDER (binding->expander),
		tree_row);
	g_object_set_qdata (G_OBJECT (binding->box), channel_tree_item_quark (),
		list_item);
	g_object_set_qdata (G_OBJECT (binding->expander),
		channel_tree_item_quark (), list_item);
	channel_tree_item_update (binding);
}

static void
channel_tree_factory_unbind (GtkSignalListItemFactory *factory,
	GtkListItem *list_item, gpointer user_data)
{
	FabulorChannelTreeItemBinding *binding = g_object_get_data (
		G_OBJECT (list_item), "fabulor-channel-tree-item-binding");

	(void) factory;
	(void) user_data;
	if (binding->item && binding->notify_id)
		g_signal_handler_disconnect (binding->item, binding->notify_id);
	binding->item = NULL;
	binding->icon_source = NULL;
	binding->notify_id = 0;
	gtk_tree_expander_set_list_row (GTK_TREE_EXPANDER (binding->expander),
		NULL);
	g_object_set_qdata (G_OBJECT (binding->box), channel_tree_item_quark (),
		NULL);
	g_object_set_qdata (G_OBJECT (binding->expander),
		channel_tree_item_quark (), NULL);
}

static gpointer
channel_tree_identity_at_position (FabulorChannelTreeView *owner,
	guint position)
{
	GtkTreeListRow *tree_row = gtk_tree_list_model_get_row (
		fabulor_channel_model_get_tree (owner->model), position);
	gpointer item = tree_row ? gtk_tree_list_row_get_item (tree_row) : NULL;
	gpointer identity = item ?
		fabulor_channel_model_get_item_identity (item) : NULL;

	g_clear_object (&tree_row);
	return identity;
}

static GtkTreeListRow *
channel_tree_find_row (FabulorChannelTreeView *owner, gpointer identity,
	guint *position)
{
	GtkTreeListModel *tree = fabulor_channel_model_get_tree (owner->model);
	guint count = g_list_model_get_n_items (G_LIST_MODEL (tree));
	guint i;

	for (i = 0; i < count; i++)
	{
		GtkTreeListRow *tree_row = gtk_tree_list_model_get_row (tree, i);
		gpointer item = tree_row ? gtk_tree_list_row_get_item (tree_row) : NULL;

		if (item && fabulor_channel_model_get_item_identity (item) == identity)
		{
			if (position)
				*position = i;
			return tree_row;
		}
		g_clear_object (&tree_row);
	}
	return NULL;
}

static void
channel_tree_selection_changed (GtkSingleSelection *selection,
	GParamSpec *pspec, gpointer user_data)
{
	FabulorChannelTreeView *owner = user_data;
	gpointer identity;

	(void) selection;
	(void) pspec;
	if (!owner->selection_callback)
		return;
	identity = fabulor_channel_model_get_selected_identity (owner->model);
	if (identity)
		owner->selection_callback (owner->view, identity,
			owner->selection_data);
}

static void
channel_tree_activate (GtkListView *view, guint position,
	gpointer user_data)
{
	FabulorChannelTreeView *owner = user_data;
	GtkTreeListRow *tree_row = gtk_tree_list_model_get_row (
		fabulor_channel_model_get_tree (owner->model), position);

	(void) view;
	if (tree_row && gtk_tree_list_row_is_expandable (tree_row))
		gtk_tree_list_row_set_expanded (tree_row,
			!gtk_tree_list_row_get_expanded (tree_row));
	g_clear_object (&tree_row);
}

#else

static void
channel_tree_selection_changed (GtkTreeSelection *selection,
	gpointer user_data)
{
	FabulorChannelTreeView *owner = user_data;
	GtkTreeModel *tree_model;
	GtkTreeIter iter;
	gpointer identity = NULL;

	if (!owner->selection_callback ||
		!gtk_tree_selection_get_selected (selection, &tree_model, &iter))
		return;
	gtk_tree_model_get (tree_model, &iter,
		FABULOR_CHANNEL_COLUMN_IDENTITY, &identity, -1);
	if (identity)
		owner->selection_callback (owner->view, identity,
			owner->selection_data);
}

static void
channel_tree_row_activated (GtkTreeView *view, GtkTreePath *path,
	GtkTreeViewColumn *column, gpointer user_data)
{
	(void) column;
	(void) user_data;
	if (gtk_tree_view_row_expanded (view, path))
		gtk_tree_view_collapse_row (view, path);
	else
		gtk_tree_view_expand_row (view, path, FALSE);
}

static void
channel_tree_add_columns (FabulorChannelTreeView *owner)
{
	GtkTreeView *tree = GTK_TREE_VIEW (owner->view);
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column = gtk_tree_view_column_new ();

	if (owner->use_icons)
	{
		renderer = gtk_cell_renderer_pixbuf_new ();
		if (owner->compact)
			g_object_set (renderer, "ypad", 0, NULL);
		gtk_tree_view_column_pack_start (column, renderer, FALSE);
		gtk_tree_view_column_set_attributes (column, renderer, "pixbuf",
			FABULOR_CHANNEL_COLUMN_ICON, NULL);
	}
	renderer = gtk_cell_renderer_text_new ();
	if (owner->compact)
		g_object_set (renderer, "ypad", 0, NULL);
	g_object_set (renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_cell_renderer_text_set_fixed_height_from_font (
		GTK_CELL_RENDERER_TEXT (renderer), 1);
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_attributes (column, renderer,
		"text", FABULOR_CHANNEL_COLUMN_NAME,
		"attributes", FABULOR_CHANNEL_COLUMN_ATTRIBUTES,
		"underline", FABULOR_CHANNEL_COLUMN_UNDERLINE, NULL);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_column_set_min_width (column, 1);
	gtk_tree_view_append_column (tree, column);
	gtk_tree_view_set_expander_column (tree, column);
}

#endif

GtkWidget *
fabulor_channel_tree_view_new (FabulorChannelModel *model,
	gboolean use_icons, gboolean compact, gboolean show_tree_lines,
	gboolean unindent_children)
{
	FabulorChannelTreeView *owner;

	g_return_val_if_fail (model != NULL, NULL);
	owner = g_new0 (FabulorChannelTreeView, 1);
	owner->model = model;
	owner->use_icons = use_icons;
	owner->compact = compact;
#if GTK_MAJOR_VERSION >= 4
	{
		GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
		GtkSelectionModel *selection = GTK_SELECTION_MODEL (g_object_ref (
			fabulor_channel_model_get_selection (model)));
		owner->view = gtk_list_view_new (selection, factory);
		g_signal_connect (factory, "setup",
			G_CALLBACK (channel_tree_factory_setup), owner);
		g_signal_connect (factory, "bind",
			G_CALLBACK (channel_tree_factory_bind), owner);
		g_signal_connect (factory, "unbind",
			G_CALLBACK (channel_tree_factory_unbind), owner);
		g_signal_connect (owner->view, "activate",
			G_CALLBACK (channel_tree_activate), owner);
		owner->selection_id = g_signal_connect (
			fabulor_channel_model_get_selection (model),
			"notify::selected-item",
			G_CALLBACK (channel_tree_selection_changed), owner);
		gtk_list_view_set_single_click_activate (GTK_LIST_VIEW (owner->view),
			FALSE);
	}
	(void) show_tree_lines;
	(void) unindent_children;
#else
	owner->view = gtk_tree_view_new_with_model (
		fabulor_channel_model_get_tree_model (model));
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (owner->view), FALSE);
	gtk_tree_view_set_enable_tree_lines (GTK_TREE_VIEW (owner->view),
		show_tree_lines);
	if (unindent_children)
	{
		gint expander_size;
		gint separator;
		gtk_widget_style_get (owner->view, "expander-size", &expander_size,
			"horizontal-separator", &separator, NULL);
		gtk_tree_view_set_level_indentation (GTK_TREE_VIEW (owner->view),
			-expander_size - separator);
	}
	channel_tree_add_columns (owner);
	g_signal_connect (gtk_tree_view_get_selection (
		GTK_TREE_VIEW (owner->view)), "changed",
		G_CALLBACK (channel_tree_selection_changed), owner);
	g_signal_connect (owner->view, "row-activated",
		G_CALLBACK (channel_tree_row_activated), owner);
#endif
	gtk_widget_set_hexpand (owner->view, TRUE);
	gtk_widget_set_vexpand (owner->view, TRUE);
	gtk_widget_set_name (owner->view, "zoitechat-tree");
	gtk_widget_set_can_focus (owner->view, GTK_MAJOR_VERSION >= 4);
	g_object_set_data_full (G_OBJECT (owner->view),
		FABULOR_CHANNEL_TREE_VIEW_DATA, owner, channel_tree_view_free);
	return owner->view;
}

void
fabulor_channel_tree_view_set_selection_callback (GtkWidget *view,
	FabulorChannelTreeSelectionFunc callback, gpointer user_data)
{
	FabulorChannelTreeView *owner = channel_tree_view_data (view);

	if (!owner)
		return;
	owner->selection_callback = callback;
	owner->selection_data = user_data;
}

gpointer
fabulor_channel_tree_view_get_identity_at_position (GtkWidget *view,
	gdouble x, gdouble y)
{
	FabulorChannelTreeView *owner = channel_tree_view_data (view);

	if (!owner)
		return NULL;
#if GTK_MAJOR_VERSION >= 4
	{
		GtkWidget *picked = gtk_widget_pick (view, x, y, GTK_PICK_DEFAULT);
		while (picked && picked != view)
		{
			GtkListItem *item = g_object_get_qdata (G_OBJECT (picked),
				channel_tree_item_quark ());
			if (item)
				return channel_tree_identity_at_position (owner,
					gtk_list_item_get_position (item));
			picked = gtk_widget_get_parent (picked);
		}
		return NULL;
	}
#else
	{
		GtkTreePath *path;
		GtkTreeIter iter;
		gpointer identity = NULL;
		if (!gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (view), (gint) x,
			(gint) y, &path, NULL, NULL, NULL))
			return NULL;
		if (gtk_tree_model_get_iter (
			fabulor_channel_model_get_tree_model (owner->model), &iter, path))
			gtk_tree_model_get (fabulor_channel_model_get_tree_model (
				owner->model), &iter, FABULOR_CHANNEL_COLUMN_IDENTITY,
				&identity, -1);
		gtk_tree_path_free (path);
		return identity;
	}
#endif
}

gboolean
fabulor_channel_tree_view_focus_identity (GtkWidget *view, gpointer identity)
{
	FabulorChannelTreeView *owner = channel_tree_view_data (view);

	if (!owner || !identity)
		return FALSE;
#if GTK_MAJOR_VERSION >= 4
	{
		guint position;
		if (!fabulor_channel_model_select_identity (owner->model, identity))
			return FALSE;
		position = gtk_single_selection_get_selected (
			fabulor_channel_model_get_selection (owner->model));
		if (position == GTK_INVALID_LIST_POSITION)
			return FALSE;
		gtk_list_view_scroll_to (GTK_LIST_VIEW (view), position,
			GTK_LIST_SCROLL_FOCUS, NULL);
		return TRUE;
	}
#else
	{
		GtkTreeIter iter;
		GtkTreeIter parent;
		GtkTreePath *path;
		GtkTreeModel *tree_model = fabulor_channel_model_get_tree_model (
			owner->model);
		GdkRectangle cell_rect;
		GdkRectangle visible_rect;
		if (!fabulor_channel_model_get_iter (owner->model, identity, &iter))
			return FALSE;
		if (gtk_tree_model_iter_parent (tree_model, &parent, &iter))
		{
			path = gtk_tree_model_get_path (tree_model, &parent);
			gtk_tree_view_expand_row (GTK_TREE_VIEW (view), path, FALSE);
			gtk_tree_path_free (path);
		}
		path = gtk_tree_model_get_path (tree_model, &iter);
		gtk_tree_view_get_background_area (GTK_TREE_VIEW (view), path, NULL,
			&cell_rect);
		gtk_tree_view_get_visible_rect (GTK_TREE_VIEW (view), &visible_rect);
		gtk_tree_view_convert_widget_to_bin_window_coords (
			GTK_TREE_VIEW (view), cell_rect.x, cell_rect.y, NULL, &cell_rect.y);
		if (cell_rect.y < visible_rect.y ||
			cell_rect.y + cell_rect.height >
				visible_rect.y + visible_rect.height)
		{
			gint destination = cell_rect.y -
				((visible_rect.height - cell_rect.height) / 2);
			gtk_tree_view_scroll_to_point (GTK_TREE_VIEW (view), -1,
				MAX (0, destination));
		}
		gtk_tree_view_set_cursor (GTK_TREE_VIEW (view), path, NULL, FALSE);
		gtk_tree_path_free (path);
		return TRUE;
	}
#endif
}

gboolean
fabulor_channel_tree_view_expand_parent (GtkWidget *view, gpointer identity)
{
	FabulorChannelTreeView *owner = channel_tree_view_data (view);
	gpointer parent_identity;

	if (!owner || !identity)
		return FALSE;
	parent_identity = fabulor_channel_model_get_parent (owner->model, identity);
	if (!parent_identity)
		return TRUE;
#if GTK_MAJOR_VERSION >= 4
	{
		GtkTreeListRow *row = channel_tree_find_row (owner, parent_identity,
			NULL);
		if (!row)
			return FALSE;
		gtk_tree_list_row_set_expanded (row, TRUE);
		g_object_unref (row);
		return TRUE;
	}
#else
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		if (!fabulor_channel_model_get_iter (owner->model, parent_identity,
			&iter))
			return FALSE;
		path = gtk_tree_model_get_path (
			fabulor_channel_model_get_tree_model (owner->model), &iter);
		gtk_tree_view_expand_row (GTK_TREE_VIEW (view), path, FALSE);
		gtk_tree_path_free (path);
		return TRUE;
	}
#endif
}

void
fabulor_channel_tree_view_expand_all (GtkWidget *view)
{
	FabulorChannelTreeView *owner = channel_tree_view_data (view);

	if (!owner)
		return;
#if GTK_MAJOR_VERSION >= 4
	{
		guint count = fabulor_channel_model_get_root_count (owner->model);
		guint i;
		for (i = 0; i < count; i++)
		{
			GtkTreeListRow *row = channel_tree_find_row (owner,
				fabulor_channel_model_get_root_at (owner->model, i), NULL);
			if (row)
			{
				gtk_tree_list_row_set_expanded (row, TRUE);
				g_object_unref (row);
			}
		}
	}
#else
	gtk_tree_view_expand_all (GTK_TREE_VIEW (view));
#endif
}

gboolean
fabulor_channel_tree_view_is_expanded (GtkWidget *view, gpointer identity)
{
	FabulorChannelTreeView *owner = channel_tree_view_data (view);

	if (!owner || !identity)
		return FALSE;
#if GTK_MAJOR_VERSION >= 4
	{
		GtkTreeListRow *row = channel_tree_find_row (owner, identity, NULL);
		gboolean expanded = row && gtk_tree_list_row_get_expanded (row);
		g_clear_object (&row);
		return expanded;
	}
#else
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		gboolean expanded;
		if (!fabulor_channel_model_get_iter (owner->model, identity, &iter))
			return FALSE;
		path = gtk_tree_model_get_path (
			fabulor_channel_model_get_tree_model (owner->model), &iter);
		expanded = gtk_tree_view_row_expanded (GTK_TREE_VIEW (view), path);
		gtk_tree_path_free (path);
		return expanded;
	}
#endif
}
