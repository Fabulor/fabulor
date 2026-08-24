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

#include "../common/fabulor.h"
#include "../common/util.h"
#include "../common/userlist.h"
#include "../common/modes.h"
#include "../common/text.h"
#include "../common/notify.h"
#include "../common/fabulorc.h"
#include "../common/fe.h"
#include "gtkutil.h"
#include "gtk-compat.h"
#include "theme/theme-gtk.h"
#include "maingui.h"
#include "menu.h"
#include "pixmaps.h"
#include "theme/theme-access.h"
#include "user-list-model.h"
#include "user-list-view.h"
#include "userlistgui.h"
#include "fkeys.h"

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
		if (sess->gui->userlist_empty)
			gtk_widget_set_visible (sess->gui->userlist_empty,
				sess->type == SESS_CHANNEL && sess->total == 0);

		if (sess->total)
		{
			g_snprintf (tbuf, sizeof (tbuf), _("%d ops, %d total"), sess->ops, sess->total);
			tbuf[sizeof (tbuf) - 1] = 0;
			gtk_label_set_text (GTK_LABEL (sess->gui->namelistinfo), tbuf);
			gtk_widget_set_tooltip_text (sess->gui->namelistinfo, tbuf);
			userlist_update_min_width (sess);
		} else
		{
			gtk_label_set_text (GTK_LABEL (sess->gui->namelistinfo), NULL);
			gtk_widget_set_tooltip_text (sess->gui->namelistinfo, NULL);
			userlist_update_min_width (sess);
		}

		if (sess->type == SESS_CHANNEL && prefs.hex_gui_win_ucount)
			fe_set_title (sess);
	}
}

/* select a row in the userlist by nick-name */

void
userlist_select (session *sess, char *name)
{
	struct User *user = userlist_find (sess, name);

	if (fabulor_user_list_view_get_model (sess->gui->user_tree) !=
		sess->res->user_model)
		return;
	if (user)
		fabulor_user_list_view_select_user (sess->gui->user_tree, user,
			TRUE, FALSE, TRUE);
}

char **
userlist_selection_list (GtkWidget *widget, int *num_ret)
{
	GPtrArray *users = fabulor_user_list_view_dup_selected_users (widget);
	int i;
	char **nicks;

	*num_ret = 0;
	if (users->len < 1)
	{
		g_ptr_array_unref (users);
		return NULL;
	}
	nicks = g_new (char *, users->len + 1);
	for (i = 0; i < (int) users->len; i++)
		nicks[i] = g_strdup (((struct User *) g_ptr_array_index (users, i))->nick);
	nicks[users->len] = NULL;
	*num_ret = (int) users->len;
	g_ptr_array_unref (users);
	return nicks;
}

void
fe_userlist_set_selected (struct session *sess)
{
	GPtrArray *selected;
	guint count;
	guint i;

	if (fabulor_user_list_view_get_model (sess->gui->user_tree) !=
		sess->res->user_model)
		return;
	count = fabulor_user_list_model_get_n_rows (sess->res->user_model);
	for (i = 0; i < count; i++)
		((struct User *) fabulor_user_list_model_get_user_at (
			sess->res->user_model, i))->selected = 0;
	selected = fabulor_user_list_view_dup_selected_users (sess->gui->user_tree);
	for (i = 0; i < selected->len; i++)
		((struct User *) g_ptr_array_index (selected, i))->selected = 1;
	g_ptr_array_unref (selected);
}

void
userlist_set_value (GtkWidget *treeview, gfloat val)
{
	fabulor_user_list_view_set_scroll_value (treeview, val);
}

gfloat
userlist_get_value (GtkWidget *treeview)
{
	return fabulor_user_list_view_get_scroll_value (treeview);
}

int
fe_userlist_remove (session *sess, struct User *user)
{
	int sel = FALSE;
	if (fabulor_user_list_view_get_model (sess->gui->user_tree) ==
		sess->res->user_model)
		sel = fabulor_user_list_view_is_user_selected (
			sess->gui->user_tree, user);

	fabulor_user_list_model_remove (sess->res->user_model, user);

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
	if (sel && fabulor_user_list_view_get_model (sess->gui->user_tree) ==
		sess->res->user_model)
		fabulor_user_list_view_select_user (sess->gui->user_tree, newuser,
			FALSE, FALSE, FALSE);
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
	(void) user_data;
	user = fabulor_user_list_view_get_user_at_position (widget, x, y);

	return user && mg_dnd_drop_file (current_sess, user->nick, uri_list);
}

static gboolean
userlist_file_drag_motion (GtkWidget *widget, gdouble x, gdouble y,
						   gpointer user_data)
{
	(void) user_data;
	fabulor_user_list_view_select_at_position (widget, x, y, TRUE);

	return TRUE;
}

static void
userlist_drag_leave (GtkWidget *widget, gpointer user_data)
{
	(void) user_data;
	fabulor_user_list_view_unselect_all (widget);
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

static gboolean
userlist_click_cb (GtkWidget *widget, guint button, guint n_press,
	gdouble x, gdouble y, GdkModifierType state, gpointer user_data)
{
	char **nicks;
	int i;

	(void) user_data;
	if (!(state & STATE_CTRL) && n_press == 2 &&
		prefs.hex_gui_ulist_doubleclick[0])
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

	if (button == 3)
	{
		/* do we have a multi-selection? */
		nicks = userlist_selection_list (widget, &i);
		if (nicks && i > 1)
		{
			menu_nickmenu_at (current_sess, widget, x, y, state, nicks[0], i);
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

		if (fabulor_user_list_view_select_at_position (widget, x, y, TRUE))
		{
			nicks = userlist_selection_list (widget, &i);
			if (nicks)
			{
				menu_nickmenu_at (current_sess, widget, x, y, state, nicks[0], i);
				while (i)
				{
					i--;
					g_free (nicks[i]);
				}
				g_free (nicks);
			}
		} else
		{
			fabulor_user_list_view_unselect_all (widget);
		}

		return TRUE;
	}

	return FALSE;
}

static gboolean
userlist_key_cb (GtkWidget *widget, guint keyval, GdkModifierType state,
	gpointer user_data)
{
	gunichar character;
	gchar text[7];
	gint length;
	gint position;
	gint selection_start;
	gint selection_end;

	(void) widget;
	(void) user_data;
	if ((state & (STATE_CTRL | STATE_ALT)) == 0 &&
		keyval >= GDK_KEY_asterisk && keyval <= GDK_KEY_z &&
		(character = gdk_keyval_to_unicode (keyval)) != 0)
	{
		SPELL_ENTRY_SET_EDITABLE (current_sess->gui->input_box, FALSE);
		gtk_widget_grab_focus (current_sess->gui->input_box);
		SPELL_ENTRY_SET_EDITABLE (current_sess->gui->input_box, TRUE);
		length = g_unichar_to_utf8 (character, text);
		text[length] = '\0';
		if (gtk_editable_get_selection_bounds (GTK_EDITABLE (
			current_sess->gui->input_box), &selection_start, &selection_end))
		{
			gtk_editable_delete_text (GTK_EDITABLE (
				current_sess->gui->input_box), selection_start, selection_end);
			position = selection_start;
		}
		else
		{
			position = SPELL_ENTRY_GET_POS (current_sess->gui->input_box);
		}
		SPELL_ENTRY_INSERT (current_sess->gui->input_box, text, length,
			&position);
		SPELL_ENTRY_SET_POS (current_sess->gui->input_box, position);
		return TRUE;
	}

	return FALSE;
}

GtkWidget *
userlist_create (GtkBox *box)
{
	GtkWidget *sw, *treeview;

	sw = fabulor_gtk_scrolled_window_new ();
	gtk_widget_set_hexpand (sw, TRUE);
	gtk_widget_set_vexpand (sw, TRUE);
	fabulor_gtk_scrolled_window_set_framed (GTK_SCROLLED_WINDOW (sw), TRUE);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
										  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_min_content_width (GTK_SCROLLED_WINDOW (sw), 1);
	fabulor_gtk_box_append (box, sw, TRUE, TRUE, 0);
	gtk_widget_show (sw);

	treeview = fabulor_user_list_view_new (prefs.hex_gui_compact,
		prefs.hex_gui_ulist_show_hosts, &prefs.hex_gui_ulist_nick_width,
		&prefs.hex_gui_ulist_host_width);

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

	fabulor_gtk_widget_on_multi_click (treeview, userlist_click_cb, NULL);
	fabulor_gtk_widget_on_key_pressed (treeview, userlist_key_cb, NULL);

	fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (sw), treeview);
	gtk_widget_show (treeview);

	return treeview;
}

void
userlist_show (session *sess)
{
	fabulor_user_list_view_set_model (sess->gui->user_tree,
		sess->res->user_model);
}

void
fe_uselect (session *sess, char *word[], int do_clear, int scroll_to)
{
	char *name;
	int thisname;
	struct User *user;

	if (fabulor_user_list_view_get_model (sess->gui->user_tree) !=
		sess->res->user_model)
		return;

	if (do_clear)
		fabulor_user_list_view_unselect_all (sess->gui->user_tree);

	thisname = 0;
	while (*(name = word[thisname++]))
	{
		user = userlist_find (sess, name);
		if (!user)
			continue;

		fabulor_user_list_view_select_user (sess->gui->user_tree, user,
			FALSE, FALSE, scroll_to);
	}
}
