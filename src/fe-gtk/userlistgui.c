/* X-Chat
 * Copyright (C) 1998 Peter Zelezny.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "fe-gtk.h"

#include <gdk/gdkkeysyms.h>

#include "../common/zoitechat.h"
#include "../common/util.h"
#include "../common/userlist.h"
#include "../common/modes.h"
#include "../common/text.h"
#include "../common/notify.h"
#include "../common/zoitechatc.h"
#include "../common/fe.h"
#include "gtkutil.h"
#include "gtk-compat.h"
#include "theme/theme-gtk.h"
#include "maingui.h"
#include "menu.h"
#include "pixmaps.h"
#include "theme/theme-access.h"
#include "user-list-model.h"
#include "userlistgui.h"
#include "fkeys.h"

#define COL_PIX FABULOR_USER_LIST_COLUMN_ICON
#define COL_PREFIX FABULOR_USER_LIST_COLUMN_PREFIX
#define COL_NICK FABULOR_USER_LIST_COLUMN_NICK
#define COL_HOST FABULOR_USER_LIST_COLUMN_HOST
#define COL_USER FABULOR_USER_LIST_COLUMN_USER
#define COL_GDKCOLOR FABULOR_USER_LIST_COLUMN_FOREGROUND

static const char *
userlist_typing_suffix (session *sess, struct User *user)
{
	static const char *active[] = { " [✎]", " [✎.]", " [✎..]" };

	if (!user || !user->typing)
		return "";

	if (user->typing == 2)
		return " [✎…]";

	return active[sess->typing_animation_frame % G_N_ELEMENTS (active)];
}

static char *
userlist_nick_markup (session *sess, struct User *user)
{
	char *nick = g_markup_escape_text (user->nick, -1);
	const char *typing = userlist_typing_suffix (sess, user);

	if (*typing)
	{
		char *marked = g_strdup_printf ("%s%s", nick, typing);
		g_free (nick);
		return marked;
	}

	return nick;
}

static const char *
userlist_prefix_color (char prefix)
{
	switch (prefix)
	{
		case '~': return "#d46a6a";
		case '@': return "#5ea36a";
		case '%': return "#d39a5f";
		case '&': return "#79aecd";
		case '+': return "#d2bf6a";
		default: return NULL;
	}
}

static void
userlist_column_width_notify_cb (GtkTreeViewColumn *column, GParamSpec *pspec, gpointer userdata)
{
	(void)pspec;

	int width = gtk_tree_view_column_get_width (column);
	int *target = (int *)userdata;

	if (!target || width < 1 || *target == width)
		return;

	*target = width;
}

static void
userlist_apply_saved_column_width (GtkTreeViewColumn *column, int width)
{
	if (!column || width < 1)
		return;

	gtk_tree_view_column_set_fixed_width (column, width);
}

static void
userlist_update_min_width (session *sess)
{
	GtkWidget *scrolled_window;

	if (!sess || !sess->gui || !sess->gui->user_tree)
		return;

	scrolled_window = gtk_widget_get_parent (sess->gui->user_tree);
	if (GTK_IS_SCROLLED_WINDOW (scrolled_window))
		gtk_scrolled_window_set_min_content_width (GTK_SCROLLED_WINDOW (scrolled_window), 1);
}

GdkPixbuf *
get_user_icon (server *serv, struct User *user)
{
	char *pre;
	int level;

	if (!user)
		return NULL;

	/* these ones are hardcoded */
	switch (user->prefix[0])
	{
		case 0: return NULL;
		case '+': return pix_ulist_voice;
		case '%': return pix_ulist_halfop;
		case '@': return pix_ulist_op;
	}

	/* find out how many levels above Op this user is */
	pre = strchr (serv->nick_prefixes, '@');
	if (pre && pre != serv->nick_prefixes)
	{
		pre--;
		level = 0;
		while (1)
		{
			if (pre[0] == user->prefix[0])
			{
				switch (level)
				{
					case 0: return pix_ulist_owner;		/* 1 level above op */
					case 1: return pix_ulist_founder;	/* 2 levels above op */
					case 2: return pix_ulist_netop;		/* 3 levels above op */
				}
				break;	/* 4+, no icons */
			}
			level++;
			if (pre == serv->nick_prefixes)
				break;
			pre--;
		}
	}

	return NULL;
}

typedef struct
{
	FabulorUserListRow row;
	gchar *prefix_markup;
	gchar *nick_markup;
	GdkRGBA foreground;
} FabulorBuiltUserRow;

static void
userlist_build_row (session *sess, struct User *user,
	FabulorBuiltUserRow *built)
{
	char prefix_text[2];
	const char *prefix_color;
	ThemeSemanticToken nick_token = THEME_TOKEN_TEXT_FOREGROUND;
	gboolean have_nick_token = FALSE;

	memset (built, 0, sizeof (*built));
	built->row.user = user;
	built->row.icon = get_user_icon (sess->server, user);
	built->nick_markup = userlist_nick_markup (sess, user);
	built->row.nick_markup = built->nick_markup;
	built->row.hostname = user->hostname;

	if (!prefs.hex_gui_ulist_icons)
	{
		if (user->prefix[0] != '\0' && user->prefix[0] != ' ')
		{
			gchar *escaped;

			prefix_text[0] = user->prefix[0];
			prefix_text[1] = '\0';
			escaped = g_markup_escape_text (prefix_text, -1);
			prefix_color = userlist_prefix_color (user->prefix[0]);
			built->prefix_markup = prefix_color ?
				g_strdup_printf ("<span foreground=\"%s\">%s</span>",
					prefix_color, escaped) : g_strdup (escaped);
			g_free (escaped);
		}
		built->row.icon = NULL;
	}
	built->row.prefix_markup = built->prefix_markup;

	if (prefs.hex_away_track && user->away)
	{
		nick_token = THEME_TOKEN_TAB_AWAY;
		have_nick_token = TRUE;
	}
	else if (prefs.hex_gui_ulist_color || prefs.hex_text_color_nicks)
	{
		int mirc_index = text_color_of (user->nick);

		if (mirc_index >= 0 && mirc_index < 32)
		{
			nick_token = (ThemeSemanticToken) (THEME_TOKEN_MIRC_0 + mirc_index);
			have_nick_token = TRUE;
		}
	}
	if (have_nick_token && theme_get_color (nick_token, &built->foreground))
		built->row.foreground = &built->foreground;
}

static void
userlist_built_row_clear (FabulorBuiltUserRow *built)
{
	g_free (built->prefix_markup);
	g_free (built->nick_markup);
}

void
fe_userlist_numbers (session *sess)
{
	char tbuf[256];

	if (sess == current_tab || !sess->gui->is_tab)
	{
		if (sess->total)
		{
			g_snprintf (tbuf, sizeof (tbuf), _("%d ops, %d total"), sess->ops, sess->total);
			tbuf[sizeof (tbuf) - 1] = 0;
			gtk_label_set_text (GTK_LABEL (sess->gui->namelistinfo), tbuf);
			userlist_update_min_width (sess);
		} else
		{
			gtk_label_set_text (GTK_LABEL (sess->gui->namelistinfo), NULL);
			userlist_update_min_width (sess);
		}

		if (sess->type == SESS_CHANNEL && prefs.hex_gui_win_ucount)
			fe_set_title (sess);
	}
}

static void
scroll_to_iter (GtkTreeIter *iter, GtkTreeView *treeview, GtkTreeModel *model)
{
	GtkTreePath *path = gtk_tree_model_get_path (model, iter);
	if (path)
	{
		gtk_tree_view_scroll_to_cell (treeview, path, NULL, TRUE, 0.5, 0.5);
		gtk_tree_path_free (path);
	}
}

/* select a row in the userlist by nick-name */

void
userlist_select (session *sess, char *name)
{
	GtkTreeIter iter;
	GtkTreeView *treeview = GTK_TREE_VIEW (sess->gui->user_tree);
	GtkTreeModel *model = fabulor_user_list_model_get_tree_model (
		sess->res->user_model);
	GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview);
	struct User *user = userlist_find (sess, name);

	if (gtk_tree_view_get_model (treeview) != model)
		return;

	if (user && fabulor_user_list_model_get_iter (
		sess->res->user_model, user, &iter))
	{
		if (gtk_tree_selection_iter_is_selected (selection, &iter))
			gtk_tree_selection_unselect_iter (selection, &iter);
		else
			gtk_tree_selection_select_iter (selection, &iter);

		scroll_to_iter (&iter, treeview, model);
		return;
	}
}

char **
userlist_selection_list (GtkWidget *widget, int *num_ret)
{
	GtkTreeIter iter;
	GtkTreeView *treeview = (GtkTreeView *) widget;
	GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview);
	GtkTreeModel *model = gtk_tree_view_get_model (treeview);
	struct User *user;
	int i, num_sel;
	char **nicks;

	*num_ret = 0;
	/* first, count the number of selections */
	num_sel = 0;
	if (gtk_tree_model_get_iter_first (model, &iter))
	{
		do
		{
			if (gtk_tree_selection_iter_is_selected (selection, &iter))
				num_sel++;
		}
		while (gtk_tree_model_iter_next (model, &iter));
	}

	if (num_sel < 1)
		return NULL;

	nicks = g_new (char *, num_sel + 1);

	i = 0;
	gtk_tree_model_get_iter_first (model, &iter);
	do
	{
		if (gtk_tree_selection_iter_is_selected (selection, &iter))
		{
			gtk_tree_model_get (model, &iter, COL_USER, &user, -1);
			nicks[i] = g_strdup (user->nick);
			i++;
			nicks[i] = NULL;
		}
	}
	while (gtk_tree_model_iter_next (model, &iter));

	*num_ret = i;
	return nicks;
}

void
fe_userlist_set_selected (struct session *sess)
{
	GtkTreeModel *store = fabulor_user_list_model_get_tree_model (
		sess->res->user_model);
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (sess->gui->user_tree));
	GtkTreeIter iter;
	struct User *user;

	/* if it's not front-most tab it doesn't own the GtkTreeView! */
	if (store != gtk_tree_view_get_model (GTK_TREE_VIEW (sess->gui->user_tree)))
		return;

	if (gtk_tree_model_get_iter_first (store, &iter))
	{
		do
		{
			gtk_tree_model_get (store, &iter, COL_USER, &user, -1);

			if (gtk_tree_selection_iter_is_selected (selection, &iter))
				user->selected = 1;
			else
				user->selected = 0;
				
		} while (gtk_tree_model_iter_next (store, &iter));
	}
}

void
userlist_set_value (GtkWidget *treeview, gfloat val)
{
	gtk_adjustment_set_value (
			gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (treeview)), val);
}

gfloat
userlist_get_value (GtkWidget *treeview)
{
	return gtk_adjustment_get_value (gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (treeview)));
}

int
fe_userlist_remove (session *sess, struct User *user)
{
	GtkTreeIter iter;
/*	GtkAdjustment *adj;
	gfloat val, end;*/
	int sel = FALSE;
	GtkTreeView *treeview = GTK_TREE_VIEW (sess->gui->user_tree);
	GtkTreeModel *model = fabulor_user_list_model_get_tree_model (
		sess->res->user_model);

	if (!fabulor_user_list_model_get_iter (
		sess->res->user_model, user, &iter))
		return 0;
	if (gtk_tree_view_get_model (treeview) == model &&
		gtk_tree_selection_iter_is_selected (
			gtk_tree_view_get_selection (treeview), &iter))
		sel = TRUE;
/*	adj = gtk_tree_view_get_vadjustment (GTK_TREE_VIEW (sess->gui->user_tree));
	val = adj->value;*/

	fabulor_user_list_model_remove (sess->res->user_model, user);

	/* is it the front-most tab? */
/*	if (gtk_tree_view_get_model (GTK_TREE_VIEW (sess->gui->user_tree))
		 == sess->res->user_model)
	{
		end = adj->upper - adj->lower - adj->page_size;
		if (val > end)
			val = end;
		gtk_adjustment_set_value (adj, val);
	}*/

	return sel;
}


static gboolean
userlist_typing_tick (session *sess)
{
	GPtrArray *users;
	guint count;
	guint i;
	gboolean keep = FALSE;
	time_t now = time (NULL);

	if (!sess || !sess->res || !sess->res->user_model)
		return FALSE;

	sess->typing_animation_frame++;
	count = fabulor_user_list_model_get_n_rows (sess->res->user_model);
	users = g_ptr_array_sized_new (count);
	for (i = 0; i < count; i++)
		g_ptr_array_add (users, fabulor_user_list_model_get_user_at (
			sess->res->user_model, i));

	for (i = 0; i < users->len; i++)
	{
		struct User *user = g_ptr_array_index (users, i);

		if (user && user->typing)
		{
			FabulorBuiltUserRow built;

			if ((user->typing == 1 && now - user->typing_time >= 6) || (user->typing == 2 && now - user->typing_time >= 30))
				user->typing = 0;

			userlist_build_row (sess, user, &built);
			g_warn_if_fail (fabulor_user_list_model_update (
				sess->res->user_model, &built.row, FALSE));
			userlist_built_row_clear (&built);
			if (user->typing)
				keep = TRUE;
		}
	}
	g_ptr_array_unref (users);

	if (!keep)
	{
		sess->typing_animation_tag = 0;
		return FALSE;
	}

	return TRUE;
}

void
fe_userlist_set_typing (session *sess, const char *nick, const char *state)
{
	struct User *user;
	FabulorBuiltUserRow built;

	if (!sess || !nick || !sess->res || !sess->res->user_model)
		return;

	user = userlist_find (sess, nick);
	if (!user)
		return;

	if (!strcmp (state, "active"))
		user->typing = 1;
	else if (!strcmp (state, "paused"))
		user->typing = 2;
	else
		user->typing = 0;
	user->typing_time = time (NULL);

	userlist_build_row (sess, user, &built);
	g_warn_if_fail (fabulor_user_list_model_update (
		sess->res->user_model, &built.row, FALSE));
	userlist_built_row_clear (&built);

	if (user->typing && !sess->typing_animation_tag)
		sess->typing_animation_tag = fe_timeout_add (350, userlist_typing_tick, sess);
}

void
fe_userlist_rehash (session *sess, struct User *user)
{
	FabulorBuiltUserRow built;

	userlist_build_row (sess, user, &built);
	g_warn_if_fail (fabulor_user_list_model_update (
		sess->res->user_model, &built.row, TRUE));
	userlist_built_row_clear (&built);
}

void
fe_userlist_insert (session *sess, struct User *newuser, gboolean sel)
{
	GtkTreeModel *model = fabulor_user_list_model_get_tree_model (
		sess->res->user_model);
	GtkTreeIter iter;
	FabulorBuiltUserRow built;

	userlist_build_row (sess, newuser, &built);
	g_warn_if_fail (fabulor_user_list_model_insert (
		sess->res->user_model, &built.row));

	/* is it me? */
	if (newuser->me && sess->gui->nick_box)
	{
		if (!sess->gui->is_tab || sess == current_tab)
			mg_set_access_icon (sess->gui, built.row.icon,
				sess->server->is_away);
	}

	/* is it the front-most tab? */
	if (gtk_tree_view_get_model (GTK_TREE_VIEW (sess->gui->user_tree))
		 == model)
	{
		if (sel && fabulor_user_list_model_get_iter (
			sess->res->user_model, newuser, &iter))
			gtk_tree_selection_select_iter (gtk_tree_view_get_selection
										(GTK_TREE_VIEW (sess->gui->user_tree)), &iter);
	}
	userlist_built_row_clear (&built);
}

void
fe_userlist_clear (session *sess)
{
	fabulor_user_list_model_clear (sess->res->user_model);
}

static gboolean
userlist_file_drop (GtkWidget *widget, gdouble x, gdouble y,
					const gchar *uri_list, gpointer user_data)
{
	struct User *user;
	GtkTreePath *path;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeView *tree = GTK_TREE_VIEW (widget);

	(void) user_data;

	if (!gtk_tree_view_get_path_at_pos (tree, (gint) x, (gint) y,
		&path, NULL, NULL, NULL))
		return FALSE;

	model = gtk_tree_view_get_model (tree);
	if (!gtk_tree_model_get_iter (model, &iter, path))
	{
		gtk_tree_path_free (path);
		return FALSE;
	}
	gtk_tree_path_free (path);
	gtk_tree_model_get (model, &iter, COL_USER, &user, -1);

	return user && mg_dnd_drop_file (current_sess, user->nick, uri_list);
}

static gboolean
userlist_file_drag_motion (GtkWidget *widget, gdouble x, gdouble y,
						   gpointer user_data)
{
	GtkTreePath *path;
	GtkTreeSelection *sel;
	GtkTreeView *tree = GTK_TREE_VIEW (widget);

	(void) user_data;

	if (gtk_tree_view_get_path_at_pos (tree, (gint) x, (gint) y,
		&path, NULL, NULL, NULL))
	{
		sel = gtk_tree_view_get_selection (tree);
		gtk_tree_selection_unselect_all (sel);
		gtk_tree_selection_select_path (sel, path);
		gtk_tree_path_free (path);
	}

	return TRUE;
}

static void
userlist_drag_leave (GtkWidget *widget, gpointer user_data)
{
	(void) user_data;
	gtk_tree_selection_unselect_all (
		gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)));
}

static gboolean
userlist_internal_drag_motion (GtkWidget *widget,
							   FabulorGtkInternalDragKind kind,
							   gdouble x, gdouble y, gpointer user_data)
{
	userlist_file_drag_motion (widget, x, y, user_data);
	return mg_internal_drag_motion (widget, kind, x, y, NULL);
}

static void
userlist_internal_drag_leave (GtkWidget *widget,
							  FabulorGtkInternalDragKind kind,
							  gpointer user_data)
{
	(void) kind;
	userlist_drag_leave (widget, user_data);
}

static gint
userlist_alpha_cmp (gconstpointer left_user, gconstpointer right_user,
					gpointer user_data)
{
	return nick_cmp_alpha ((struct User *) left_user,
		(struct User *) right_user, ((session *) user_data)->server);
}

static gint
userlist_ops_cmp (gconstpointer left_user, gconstpointer right_user,
				  gpointer user_data)
{
	return nick_cmp_az_ops (((session *) user_data)->server,
		(struct User *) left_user, (struct User *) right_user);
}

FabulorUserListModel *
userlist_create_model (session *sess)
{
	FabulorUserListCompareFunc compare = NULL;
	gboolean descending = FALSE;

	switch (prefs.hex_gui_ulist_sort)
	{
	case 0:
		compare = userlist_ops_cmp;
		break;
	case 1:
		compare = userlist_alpha_cmp;
		break;
	case 2:
		compare = userlist_ops_cmp;
		descending = TRUE;
		break;
	case 3:
		compare = userlist_alpha_cmp;
		descending = TRUE;
		break;
	default:
		break;
	}

	return fabulor_user_list_model_new (compare, sess, descending);
}

static void
userlist_add_columns (GtkTreeView * treeview)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	/* icon column */
	renderer = gtk_cell_renderer_pixbuf_new ();
	if (prefs.hex_gui_compact)
		g_object_set (G_OBJECT (renderer), "ypad", 0, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
																-1, NULL, renderer,
																"pixbuf", COL_PIX, NULL);
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), 0);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);

	/* nick column */
	column = gtk_tree_view_column_new ();
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

	renderer = gtk_cell_renderer_text_new ();
	if (prefs.hex_gui_compact)
		g_object_set (G_OBJECT (renderer), "ypad", 0, NULL);
	gtk_cell_renderer_text_set_fixed_height_from_font (GTK_CELL_RENDERER_TEXT (renderer), 1);
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute (column, renderer, "markup", COL_PREFIX);

	renderer = gtk_cell_renderer_text_new ();
	if (prefs.hex_gui_compact)
		g_object_set (G_OBJECT (renderer), "ypad", 0, NULL);
	g_object_set (G_OBJECT (renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_cell_renderer_text_set_fixed_height_from_font (GTK_CELL_RENDERER_TEXT (renderer), 1);
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute (column, renderer, "markup", COL_NICK);
	gtk_tree_view_column_add_attribute (column, renderer, THEME_GTK_FOREGROUND_PROPERTY, COL_GDKCOLOR);

	column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), 1);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 1);
	gtk_tree_view_column_set_resizable (column, TRUE);
	userlist_apply_saved_column_width (column, prefs.hex_gui_ulist_nick_width);
	g_signal_connect (G_OBJECT (column), "notify::width",
							G_CALLBACK (userlist_column_width_notify_cb),
							&prefs.hex_gui_ulist_nick_width);

	if (prefs.hex_gui_ulist_show_hosts)
	{
		/* hostname column */
		renderer = gtk_cell_renderer_text_new ();
		if (prefs.hex_gui_compact)
			g_object_set (G_OBJECT (renderer), "ypad", 0, NULL);
		g_object_set (G_OBJECT (renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
		gtk_cell_renderer_text_set_fixed_height_from_font (GTK_CELL_RENDERER_TEXT (renderer), 1);
		gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
																	-1, NULL, renderer,
																	"text", COL_HOST, NULL);
		column = gtk_tree_view_get_column (GTK_TREE_VIEW (treeview), 2);
		gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_expand (column, TRUE);
		gtk_tree_view_column_set_min_width (column, 1);
		gtk_tree_view_column_set_resizable (column, TRUE);
		userlist_apply_saved_column_width (column, prefs.hex_gui_ulist_host_width);
		g_signal_connect (G_OBJECT (column), "notify::width",
								G_CALLBACK (userlist_column_width_notify_cb),
								&prefs.hex_gui_ulist_host_width);
	}
}

static gint
userlist_click_cb (GtkWidget *widget, GdkEventButton *event, gpointer userdata)
{
	char **nicks;
	int i;
	GtkTreeSelection *sel;
	GtkTreePath *path;

	if (!event)
		return FALSE;

	if (!(event->state & STATE_CTRL) &&
		event->type == GDK_2BUTTON_PRESS && prefs.hex_gui_ulist_doubleclick[0])
	{
		nicks = userlist_selection_list (widget, &i);
		if (nicks)
		{
			nick_command_parse (current_sess, prefs.hex_gui_ulist_doubleclick, nicks[0],
									  nicks[0]);
			while (i)
			{
				i--;
				g_free (nicks[i]);
			}
			g_free (nicks);
		}
		return TRUE;
	}

	if (event->button == 3)
	{
		/* do we have a multi-selection? */
		nicks = userlist_selection_list (widget, &i);
		if (nicks && i > 1)
		{
			menu_nickmenu (current_sess, event, nicks[0], i);
			while (i)
			{
				i--;
				g_free (nicks[i]);
			}
			g_free (nicks);
			return TRUE;
		}
		if (nicks)
		{
			g_free (nicks[0]);
			g_free (nicks);
		}

		sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (widget));
		if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget),
			 event->x, event->y, &path, 0, 0, 0))
		{
			gtk_tree_selection_unselect_all (sel);
			gtk_tree_selection_select_path (sel, path);
			gtk_tree_path_free (path);
			nicks = userlist_selection_list (widget, &i);
			if (nicks)
			{
				menu_nickmenu (current_sess, event, nicks[0], i);
				while (i)
				{
					i--;
					g_free (nicks[i]);
				}
				g_free (nicks);
			}
		} else
		{
			gtk_tree_selection_unselect_all (sel);
		}

		return TRUE;
	}

	return FALSE;
}

static gboolean
userlist_key_cb (GtkWidget *wid, GdkEventKey *evt, gpointer userdata)
{
	if (evt->keyval >= GDK_KEY_asterisk && evt->keyval <= GDK_KEY_z)
	{
		/* dirty trick to avoid auto-selection */
		SPELL_ENTRY_SET_EDITABLE (current_sess->gui->input_box, FALSE);
		gtk_widget_grab_focus (current_sess->gui->input_box);
		SPELL_ENTRY_SET_EDITABLE (current_sess->gui->input_box, TRUE);
		gtk_widget_event (current_sess->gui->input_box, (GdkEvent *)evt);
		return TRUE;
	}

	return FALSE;
}

GtkWidget *
userlist_create (GtkBox *box)
{
	GtkWidget *sw, *treeview;

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_hexpand (sw, TRUE);
	gtk_widget_set_vexpand (sw, TRUE);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
													 GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
										  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_min_content_width (GTK_SCROLLED_WINDOW (sw), 1);
	fabulor_gtk_box_append (box, sw, TRUE, TRUE, 0);
	gtk_widget_show (sw);

	treeview = gtk_tree_view_new ();
	gtk_widget_set_hexpand (treeview, TRUE);
	gtk_widget_set_vexpand (treeview, TRUE);
	gtk_widget_set_name (treeview, "zoitechat-userlist");
	gtk_widget_set_can_focus (treeview, TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
	gtk_tree_selection_set_mode (gtk_tree_view_get_selection
										  (GTK_TREE_VIEW (treeview)),
										  GTK_SELECTION_MULTIPLE);

	/* set up drops */
	fabulor_gtk_widget_enable_internal_drag_source (treeview,
		FABULOR_GTK_INTERNAL_DRAG_USER_LIST, mg_internal_drag_icon, NULL);
	fabulor_gtk_widget_enable_internal_drop_target (treeview,
		FABULOR_GTK_INTERNAL_DRAG_ACCEPT (FABULOR_GTK_INTERNAL_DRAG_CHANNEL_VIEW),
		userlist_internal_drag_motion, userlist_internal_drag_leave,
		mg_internal_drag_drop, NULL);
	fabulor_gtk_widget_on_file_drop_full (treeview,
		GDK_ACTION_MOVE | GDK_ACTION_COPY | GDK_ACTION_LINK,
		userlist_file_drop, userlist_file_drag_motion, userlist_drag_leave, NULL);

	g_signal_connect (G_OBJECT (treeview), "button-press-event",
							G_CALLBACK (userlist_click_cb), 0);
	g_signal_connect (G_OBJECT (treeview), "key-press-event",
							G_CALLBACK (userlist_key_cb), 0);

	userlist_add_columns (GTK_TREE_VIEW (treeview));

	fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (sw), treeview);
	gtk_widget_show (treeview);

	return treeview;
}

void
userlist_show (session *sess)
{
	gtk_tree_view_set_model (GTK_TREE_VIEW (sess->gui->user_tree),
		fabulor_user_list_model_get_tree_model (sess->res->user_model));
}

void
fe_uselect (session *sess, char *word[], int do_clear, int scroll_to)
{
	char *name;
	int thisname;
	GtkTreeIter iter;
	GtkTreeView *treeview = GTK_TREE_VIEW (sess->gui->user_tree);
	GtkTreeModel *model = fabulor_user_list_model_get_tree_model (
		sess->res->user_model);
	GtkTreeSelection *selection = gtk_tree_view_get_selection (treeview);
	struct User *user;

	if (gtk_tree_view_get_model (treeview) != model)
		return;

	if (do_clear)
		gtk_tree_selection_unselect_all (selection);

	thisname = 0;
	while (*(name = word[thisname++]))
	{
		user = userlist_find (sess, name);
		if (!user)
			continue;

		if (fabulor_user_list_model_get_iter (
			sess->res->user_model, user, &iter))
		{
			gtk_tree_selection_select_iter (selection, &iter);
			if (scroll_to)
				scroll_to_iter (&iter, treeview, model);
			continue;
		}
	}
}
