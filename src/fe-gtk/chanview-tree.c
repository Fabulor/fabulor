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
	GtkWidget *tree;
	GtkWidget *scrollw;
} treeview;

#include <gdk/gdk.h>

#include "channel-tree-view.h"
#include "theme/theme-access.h"

static void
cv_tree_selection_changed (GtkWidget *view, gpointer identity,
	gpointer user_data)
{
	chanview *cv = user_data;
	chan *ch = identity;
	chan *previous = cv->focused;

	if (previous && previous != ch &&
		fabulor_channel_model_get_parent (cv->model, previous) == ch &&
		!fabulor_channel_tree_view_is_expanded (view, ch))
		return;
	if (previous && previous != ch)
	{
		previous->underline = PANGO_UNDERLINE_NONE;
		chanview_update_model_row (previous);
	}
	ch->underline = PANGO_UNDERLINE_SINGLE;
	chanview_update_model_row (ch);
	cv->focused = ch;
	cv->cb_focus (cv, ch, ch->tag, ch->userdata);
}

static gboolean
cv_tree_click_cb (GtkWidget *tree, guint button, guint n_press, gdouble x,
	gdouble y, GdkModifierType state, gpointer user_data)
{
	chanview *cv = user_data;
	chan *ch = fabulor_channel_tree_view_get_identity_at_position (tree,
		x, y);

	(void) n_press;
	return ch ? cv->cb_contextmenu (cv, ch, ch->tag, ch->userdata, tree,
		button, x, y, state) : FALSE;
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
	GtkWidget *view;
	GtkWidget *win = gtk_scrolled_window_new (0, 0);

	gtk_widget_set_hexpand (win, TRUE);
	gtk_widget_set_vexpand (win, TRUE);
	fabulor_gtk_scrolled_window_set_framed (GTK_SCROLLED_WINDOW (win), TRUE);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (win),
		GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_min_content_width (GTK_SCROLLED_WINDOW (win), 1);
	gtk_container_add (GTK_CONTAINER (cv->box), win);
	gtk_widget_show (win);

	view = fabulor_channel_tree_view_new (cv->model, cv->use_icons,
		prefs.hex_gui_compact, prefs.hex_gui_tab_dots,
		!prefs.hex_gui_tab_server);
	{
		ThemeWidgetStyleValues style_values;
		theme_get_widget_style_values_for_widget (view, &style_values);
		gtkutil_apply_palette (view, &style_values.background,
			&style_values.foreground, cv->font_desc);
	}
	fabulor_channel_tree_view_set_selection_callback (view,
		cv_tree_selection_changed, cv);
	fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (win), view);
	fabulor_gtk_widget_on_multi_click (view, cv_tree_click_cb, cv);
	fabulor_gtk_widget_on_scroll (view, cv_tree_scroll_cb, NULL);
	fabulor_gtk_widget_enable_internal_drag_source (view,
		FABULOR_GTK_INTERNAL_DRAG_CHANNEL_VIEW, mg_internal_drag_icon, NULL);
	fabulor_gtk_widget_enable_internal_drop_target (view,
		FABULOR_GTK_INTERNAL_DRAG_ACCEPT (FABULOR_GTK_INTERNAL_DRAG_USER_LIST),
		mg_internal_drag_motion, NULL, mg_internal_drag_drop, NULL);

	((treeview *) cv)->tree = view;
	((treeview *) cv)->scrollw = win;
	chanview_apply_theme (cv);
	gtk_widget_show (view);
}

static void
cv_tree_postinit (chanview *cv)
{
	fabulor_channel_tree_view_expand_all (((treeview *) cv)->tree);
}

static void *
cv_tree_add (chanview *cv, chan *ch, char *name, chan *parent)
{
	(void) name;
	(void) parent;
	fabulor_channel_tree_view_expand_parent (((treeview *) cv)->tree, ch);
	return NULL;
}

static void
cv_tree_change_orientation (chanview *cv)
{
	(void) cv;
}

static void
cv_tree_focus (chan *ch)
{
	fabulor_channel_tree_view_focus_identity (
		((treeview *) ch->cv)->tree, ch);
}

static void
cv_tree_move_focus (chanview *cv, gboolean relative, int num)
{
	chan *ch;

	if (relative)
	{
		num += cv_find_number_of_chan (cv, cv->focused);
		num %= cv->size;
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
	(void) ch;
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
		gtk_widget_destroy (((treeview *) cv)->scrollw);
}

static void
cv_tree_set_color (chan *ch, PangoAttrList *list)
{
	(void) ch;
	(void) list;
}

static void
cv_tree_rename (chan *ch, char *name)
{
	(void) ch;
	(void) name;
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

	return parent && !fabulor_channel_tree_view_is_expanded (
		((treeview *) ch->cv)->tree, parent);
}
