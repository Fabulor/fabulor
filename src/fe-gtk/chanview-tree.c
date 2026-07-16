/* ZoiteChat
 * Copyright (C) 1998-2010 Peter Zelezny.
 * Copyright (C) 2009-2013 Berke Viktor.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/* file included in chanview.c */

typedef struct
{
	GtkTreeView *tree;
	GtkWidget *scrollw;	/* scrolledWindow */
} treeview;

#include <gdk/gdk.h>

#include "theme/theme-access.h"

static void 	/* row-activated, when a row is double clicked */
cv_tree_activated_cb (GtkTreeView *view, GtkTreePath *path,
							 GtkTreeViewColumn *column, gpointer data)
{
	if (gtk_tree_view_row_expanded (view, path))
		gtk_tree_view_collapse_row (view, path);
	else
		gtk_tree_view_expand_row (view, path, FALSE);
}

static void		/* row selected callback */
cv_tree_sel_cb (GtkTreeSelection *sel, chanview *cv)
{
	GtkTreeModel *model;
	GtkTreeIter prev_iter;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeView *view;
	chan *ch;
	chan *prev_ch;
	gboolean has_prev;

	has_prev = cv->focused && fabulor_channel_model_get_iter (
		cv->model, cv->focused, &prev_iter);
	if (has_prev)
		prev_ch = cv->focused;

	if (gtk_tree_selection_get_selected (sel, &model, &iter))
	{
		gtk_tree_model_get (model, &iter,
			FABULOR_CHANNEL_COLUMN_IDENTITY, &ch, -1);

		if (has_prev)
		{
			if (prev_ch != ch && fabulor_channel_model_get_parent (
				cv->model, prev_ch) == ch)
			{
				view = gtk_tree_selection_get_tree_view (sel);
				path = gtk_tree_model_get_path (model, &iter);
				if (path)
				{
					if (!gtk_tree_view_row_expanded (view, path))
					{
						gtk_tree_path_free (path);
						return;
					}
					gtk_tree_path_free (path);
				}
			}
			if (prev_ch != ch)
			{
				prev_ch->underline = PANGO_UNDERLINE_NONE;
				chanview_update_model_row (prev_ch);
			}
		}
		ch->underline = PANGO_UNDERLINE_SINGLE;
		chanview_update_model_row (ch);

		cv->focused = ch;
		cv->cb_focus (cv, ch, ch->tag, ch->userdata);
	}
}

static gboolean
cv_tree_click_cb (GtkTreeView *tree, GdkEventButton *event, chanview *cv)
{
	chan *ch;
	GtkTreePath *path;
	GtkTreeIter iter;
	int ret = FALSE;

	if (gtk_tree_view_get_path_at_pos (tree, event->x, event->y, &path, 0, 0, 0))
	{
		if (gtk_tree_model_get_iter (
			fabulor_channel_model_get_tree_model (cv->model), &iter, path))
		{
			gtk_tree_model_get (
				fabulor_channel_model_get_tree_model (cv->model), &iter,
				FABULOR_CHANNEL_COLUMN_IDENTITY, &ch, -1);
			ret = cv->cb_contextmenu (cv, ch, ch->tag, ch->userdata, event);
		}
		gtk_tree_path_free (path);
	}
	return ret;
}

static gboolean
cv_tree_scroll_cb (GtkWidget *widget, gdouble dx, gdouble dy,
				   gpointer user_data)
{
	(void) widget;
	(void) user_data;
	if (prefs.hex_gui_tab_scrollchans)
	{
		int direction = cv_scroll_direction (dx, dy);
		int i;

		if (direction != 0)
			for (i = 0; i < cv_scroll_step_count (); i++)
				mg_switch_page (1, direction);

		return direction != 0;
	}

	return FALSE;
}

static void
cv_tree_init (chanview *cv)
{
	GtkWidget *view, *win;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;
	int wid1, wid2;

	win = gtk_scrolled_window_new (0, 0);
	gtk_widget_set_hexpand (win, TRUE);
	gtk_widget_set_vexpand (win, TRUE);

	/*gtk_container_set_border_width (GTK_CONTAINER (win), 1);*/
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (win),
									 GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (win),
											  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_min_content_width (GTK_SCROLLED_WINDOW (win), 1);
	gtk_container_add (GTK_CONTAINER (cv->box), win);
	gtk_widget_show (win);

	view = gtk_tree_view_new_with_model (
		fabulor_channel_model_get_tree_model (cv->model));
	gtk_widget_set_hexpand (view, TRUE);
	gtk_widget_set_vexpand (view, TRUE);
	gtk_widget_set_name (view, "zoitechat-tree");
	{
		ThemeWidgetStyleValues style_values;

		theme_get_widget_style_values_for_widget (view, &style_values);
		gtkutil_apply_palette (view, &style_values.background, &style_values.foreground,
		                       cv->font_desc);
	}
	/*gtk_widget_modify_base (view, GTK_STATE_NORMAL, &colors[THEME_LEGACY_TEXT_BACKGROUND]);*/
	gtk_widget_set_can_focus (view, FALSE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (view), FALSE);

	if (prefs.hex_gui_tab_dots)
	{
		gtk_tree_view_set_enable_tree_lines (GTK_TREE_VIEW (view), TRUE);
	}
	
	/* Indented channels with no server looks silly, but we still want expanders */
	if (!prefs.hex_gui_tab_server)
	{
		gtk_widget_style_get (view, "expander-size", &wid1, "horizontal-separator", &wid2, NULL);
		gtk_tree_view_set_level_indentation (GTK_TREE_VIEW (view), -wid1 - wid2);
	}


	fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (win), view);
	col = gtk_tree_view_column_new();

	if (cv->use_icons)
	{
		renderer = gtk_cell_renderer_pixbuf_new ();
		if (prefs.hex_gui_compact)
			g_object_set (G_OBJECT (renderer), "ypad", 0, NULL);

		gtk_tree_view_column_pack_start (col, renderer, FALSE);
		gtk_tree_view_column_set_attributes (col, renderer, "pixbuf",
			FABULOR_CHANNEL_COLUMN_ICON, NULL);
	}

	renderer = gtk_cell_renderer_text_new ();
	if (prefs.hex_gui_compact)
		g_object_set (G_OBJECT (renderer), "ypad", 0, NULL);
	g_object_set (G_OBJECT (renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_cell_renderer_text_set_fixed_height_from_font (GTK_CELL_RENDERER_TEXT (renderer), 1);
	gtk_tree_view_column_pack_start (col, renderer, TRUE);
	gtk_tree_view_column_set_attributes (col, renderer,
								  "text", FABULOR_CHANNEL_COLUMN_NAME,
								  "attributes", FABULOR_CHANNEL_COLUMN_ATTRIBUTES,
								  "underline", FABULOR_CHANNEL_COLUMN_UNDERLINE,
								  NULL);
	gtk_tree_view_column_set_expand (col, TRUE);
	gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_column_set_min_width (col, 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW (view), col);
	gtk_tree_view_set_expander_column (GTK_TREE_VIEW (view), col);

	g_signal_connect (G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (view))),
							"changed", G_CALLBACK (cv_tree_sel_cb), cv);
	g_signal_connect (G_OBJECT (view), "button-press-event",
							G_CALLBACK (cv_tree_click_cb), cv);
	g_signal_connect (G_OBJECT (view), "row-activated",
							G_CALLBACK (cv_tree_activated_cb), NULL);
	fabulor_gtk_widget_on_scroll (view, cv_tree_scroll_cb, NULL);

	fabulor_gtk_widget_enable_internal_drag_source (view,
		FABULOR_GTK_INTERNAL_DRAG_CHANNEL_VIEW, mg_internal_drag_icon, NULL);
	fabulor_gtk_widget_enable_internal_drop_target (view,
		FABULOR_GTK_INTERNAL_DRAG_ACCEPT (FABULOR_GTK_INTERNAL_DRAG_USER_LIST),
		mg_internal_drag_motion, NULL, mg_internal_drag_drop, NULL);

	((treeview *)cv)->tree = GTK_TREE_VIEW (view);
	((treeview *)cv)->scrollw = win;
	chanview_apply_theme (cv);
	gtk_widget_show (view);
}

static void
cv_tree_postinit (chanview *cv)
{
	gtk_tree_view_expand_all (((treeview *)cv)->tree);
}

static void *
cv_tree_add (chanview *cv, chan *ch, char *name, chan *parent)
{
	GtkTreePath *path;
	GtkTreeIter parent_iter;

	if (parent && fabulor_channel_model_get_iter (cv->model, parent,
		&parent_iter))
	{
		/* expand the parent node */
		path = gtk_tree_model_get_path (
			fabulor_channel_model_get_tree_model (cv->model), &parent_iter);
		if (path)
		{
			gtk_tree_view_expand_row (((treeview *)cv)->tree, path, FALSE);
			gtk_tree_path_free (path);
		}
	}

	return NULL;
}

static void
cv_tree_change_orientation (chanview *cv)
{
}

static void
cv_tree_focus (chan *ch)
{
	GtkTreeView *tree = ((treeview *)ch->cv)->tree;
	GtkTreeModel *model = gtk_tree_view_get_model (tree);
	GtkTreePath *path;
	GtkTreeIter parent;
	GtkTreeIter iter;
	GdkRectangle cell_rect;
	GdkRectangle vis_rect;
	gint dest_y;

	/* expand the parent node */
	if (!fabulor_channel_model_get_iter (ch->cv->model, ch, &iter))
		return;
	if (gtk_tree_model_iter_parent (model, &parent, &iter))
	{
		path = gtk_tree_model_get_path (model, &parent);
		if (path)
		{
			/*if (!gtk_tree_view_row_expanded (tree, path))
			{
				gtk_tree_path_free (path);
				return;
			}*/
			gtk_tree_view_expand_row (tree, path, FALSE);
			gtk_tree_path_free (path);
		}
	}

	path = gtk_tree_model_get_path (model, &iter);
	if (path)
	{
		/* This full section does what
		 * gtk_tree_view_scroll_to_cell (tree, path, NULL, TRUE, 0.5, 0.5);
		 * does, except it only scrolls the window if the provided cell is
		 * not visible. Basic algorithm taken from gtktreeview.c */

		/* obtain information to see if the cell is visible */
		gtk_tree_view_get_background_area (tree, path, NULL, &cell_rect);
		gtk_tree_view_get_visible_rect (tree, &vis_rect);

		/* The cordinates aren't offset correctly */
		gtk_tree_view_convert_widget_to_bin_window_coords ( tree, cell_rect.x, cell_rect.y, NULL, &cell_rect.y );

		/* only need to scroll if out of bounds */
		if (cell_rect.y < vis_rect.y ||
				cell_rect.y + cell_rect.height > vis_rect.y + vis_rect.height)
		{
			dest_y = cell_rect.y - ((vis_rect.height - cell_rect.height) * 0.5);
			if (dest_y < 0)
				dest_y = 0;
			gtk_tree_view_scroll_to_point (tree, -1, dest_y);
		}
		/* theft done, now make it focused like */
		gtk_tree_view_set_cursor (tree, path, NULL, FALSE);
		gtk_tree_path_free (path);
	}
}

static void
cv_tree_move_focus (chanview *cv, gboolean relative, int num)
{
	chan *ch;

	if (relative)
	{
		num += cv_find_number_of_chan (cv, cv->focused);
		num %= cv->size;
		/* make it wrap around at both ends */
		if (num < 0)
			num = cv->size - 1;
	}

	ch = cv_find_chan_by_number (cv, num);
	if (ch)
		cv_tree_focus (ch);
}

static void
cv_tree_remove (chan *ch)
{
}

static void
cv_tree_move (chan *ch, int delta)
{
	(void) ch;
	(void) delta;
}

static void
cv_tree_move_family (chan *ch, int delta)
{
	(void) ch;
	(void) delta;
}

static void
cv_tree_cleanup (chanview *cv)
{
	if (cv->box)
		/* kill the scrolled window */
		gtk_widget_destroy (((treeview *)cv)->scrollw);
}

static void
cv_tree_set_color (chan *ch, PangoAttrList *list)
{
	/* nothing to do, it's already set in the store */
}

static void
cv_tree_rename (chan *ch, char *name)
{
	/* nothing to do, it's already renamed in the store */
}

static chan *
cv_tree_get_parent (chan *ch)
{
	return fabulor_channel_model_get_parent (ch->cv->model, ch);
}

static gboolean
cv_tree_is_collapsed (chan *ch)
{
	chan *parent = cv_tree_get_parent (ch);
	GtkTreePath *path = NULL;
	GtkTreeIter parent_iter;
	gboolean ret;

	if (parent == NULL)
		return FALSE;

	if (!fabulor_channel_model_get_iter (parent->cv->model, parent,
		&parent_iter))
		return FALSE;
	path = gtk_tree_model_get_path (
		fabulor_channel_model_get_tree_model (parent->cv->model), &parent_iter);
	ret = !gtk_tree_view_row_expanded (((treeview *)parent->cv)->tree, path);
	gtk_tree_path_free (path);
	
	return ret;
}
