/* X-Chat
 * Copyright (C) 1998-2007 Peter Zelezny.
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
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include "fe-gtk.h"

#include <gdk/gdkkeysyms.h>

#include "../common/zoitechat.h"
#include "../common/zoitechatc.h"
#include "../common/cfgfiles.h"
#include "../common/outbound.h"
#include "../common/inbound.h"
#include "../common/ignore.h"
#include "../common/fe.h"
#include "../common/server.h"
#include "../common/servlist.h"
#include "../common/notify.h"
#include "../common/util.h"
#include "../common/text.h"
#include "xtext.h"
#include "ascii.h"
#include "banlist.h"
#include "chanlist.h"
#include "editlist.h"
#include "fkeys.h"
#include "gtkutil.h"
#include "gtk-compat.h"
#include "maingui.h"
#include "notifygui.h"
#include "pixmaps.h"
#include "rawlog.h"
#include "theme/theme-gtk.h"
#include "theme/theme-manager.h"
#include "plugingui.h"
#include "search.h"
#include "textgui.h"
#include "urlgrab.h"
#include "userlistgui.h"
#include "menu.h"
#include "servlistgui.h"
#if GTK_MAJOR_VERSION >= 4
#include "channel-context-menu-model.h"
#include "context-menu-presenter-gtk4.h"
#include "middle-context-menu-model.h"
#include "nick-context-menu-model.h"
#include "url-context-menu-model.h"
#endif

#define FABULOR_DOCS_URL "https://github.com/Fabulor/fabulor/tree/main/docs"
#define FABULOR_README_URL "https://github.com/Fabulor/fabulor/blob/main/README.md"

static GSList *submenu_list;

#define FABULOR_MENU_ACTION_GROUP "fabulor-menu-action-group"
#define FABULOR_MENU_ACTION_NAME "fabulor-menu-action-name"
#define FABULOR_MENU_ACTION_TARGET "fabulor-menu-action-target"
#define FABULOR_MENU_CHANNEL_SWITCHER_MODEL "fabulor-menu-channel-switcher-model"
#define FABULOR_MENU_NETWORK_METERS_MODEL "fabulor-menu-network-meters-model"
#define FABULOR_MENU_NEW_MODEL "fabulor-menu-new-model"
#define FABULOR_MENU_SERVER_MODEL "fabulor-menu-server-model"
#define FABULOR_MENU_SETTINGS_MODEL "fabulor-menu-settings-model"
#define FABULOR_MENU_SEARCH_MODEL "fabulor-menu-search-model"
#define FABULOR_MENU_HELP_MODEL "fabulor-menu-help-model"
#define FABULOR_MENU_WINDOW_MODEL "fabulor-menu-window-model"
#define FABULOR_MENU_VIEW_MODEL "fabulor-menu-view-model"
#define FABULOR_MENU_ROOT_MODEL "fabulor-menu-root-model"
#define FABULOR_MENU_USER_MODEL "fabulor-menu-user-model"
#define FABULOR_MENU_PLUGIN_MODEL "fabulor-menu-plugin-model"
#define FABULOR_MENU_COMPOSED_MODEL "fabulor-menu-composed-model"
#define FABULOR_MENU_ACTION_PROXIES "fabulor-menu-action-proxies"
#define FABULOR_MENU_CONTEXT_MODEL "fabulor-menu-context-model"
#define FABULOR_MENU_CONTEXT_ACTION_GROUP "fabulor-menu-context-action-group"

#define FABULOR_USER_COMMAND_ACTION "user-command"
#define FABULOR_USER_EDIT_ACTION "edit-user-menu"
#define FABULOR_USER_TOGGLE_ACTION_PREFIX "user-toggle-"
#define FABULOR_PLUGIN_ACTION_PREFIX "plugin-menu-"

static gboolean menu_action_set_item_state (GtkWidget *item, gboolean state);
static void menu_usermenu_model_refresh (GtkWidget *menu_bar);
#if GTK_MAJOR_VERSION >= 4
static void menu_main_composed_model_refresh (GtkWidget *menu_bar);
static void menu_middlemenu_gtk4 (session *sess, GtkWidget *origin,
	gdouble x, gdouble y);
#endif

#if GTK_MAJOR_VERSION < 4
static GtkWidget *
menu_icon_widget_new (const char *icon)
{
	GtkWidget *img = NULL;
	char *path;

	if (!icon)
		return NULL;

	if (access (icon, R_OK) == 0)
		return gtk_image_new_from_file (icon);

	path = g_build_filename (get_xdir (), icon, NULL);
	if (access (path, R_OK) == 0)
	{
		img = gtk_image_new_from_file (path);
	}
	else if (g_str_has_prefix (icon, "zc-menu-") || g_str_has_prefix (icon, "gtk-"))
	{
		img = gtkutil_image_new_from_stock (icon, GTK_ICON_SIZE_MENU);
	}
	else
	{
		img = gtkutil_image_new_from_stock (icon, GTK_ICON_SIZE_MENU);
	}

	g_free (path);

	return img;
}

static GtkWidget *
menu_new (void)
{
	GtkWidget *menu = gtk_menu_new ();

	gtk_menu_set_reserve_toggle_size (GTK_MENU (menu), FALSE);

	return menu;
}
#endif

enum
{
	M_MENUITEM,
	M_NEWMENU,
	M_END,
	M_SEP,
	M_MENUTOG,
	M_MENURADIO,
	M_MENUSTOCK,
	M_MENUPIX,
	M_MENUSUB
};

typedef enum
{
	MENU_ACTION_NONE,
	MENU_ACTION_NETWORK_LIST,
	MENU_ACTION_LOAD_PLUGIN_SCRIPT,
	MENU_ACTION_DETACH_ATTACH,
	MENU_ACTION_NEW_SERVER_TAB,
	MENU_ACTION_NEW_CHANNEL_TAB,
	MENU_ACTION_NEW_SERVER_WINDOW,
	MENU_ACTION_NEW_CHANNEL_WINDOW,
	MENU_ACTION_CLOSE,
	MENU_ACTION_QUIT,
	MENU_ACTION_MENU_TOGGLE,
	MENU_ACTION_TOPIC_BAR_TOGGLE,
	MENU_ACTION_USER_LIST_TOGGLE,
	MENU_ACTION_USER_LIST_BUTTONS_TOGGLE,
	MENU_ACTION_MODE_BUTTONS_TOGGLE,
	MENU_ACTION_FULLSCREEN_TOGGLE,
	MENU_ACTION_CHANNEL_SWITCHER,
	MENU_ACTION_NETWORK_METERS,
	MENU_ACTION_DISCONNECT,
	MENU_ACTION_RECONNECT,
	MENU_ACTION_JOIN_CHANNEL,
	MENU_ACTION_CHANNEL_LIST,
	MENU_ACTION_AWAY_TOGGLE,
	MENU_ACTION_RESET_MARKER,
	MENU_ACTION_MOVE_MARKER,
	MENU_ACTION_COPY_SELECTION,
	MENU_ACTION_SEARCH_TEXT,
	MENU_ACTION_SEARCH_NEXT,
	MENU_ACTION_SEARCH_PREVIOUS,
	MENU_ACTION_CONTENTS,
	MENU_ACTION_ABOUT,
	MENU_ACTION_PREFERENCES,
	MENU_ACTION_AUTO_REPLACE,
	MENU_ACTION_CTCP_REPLIES,
	MENU_ACTION_DIALOG_BUTTONS,
	MENU_ACTION_KEYBOARD_SHORTCUTS,
	MENU_ACTION_TEXT_EVENTS,
	MENU_ACTION_URL_HANDLERS,
	MENU_ACTION_USER_COMMANDS,
	MENU_ACTION_USER_LIST_BUTTONS,
	MENU_ACTION_USER_LIST_POPUP,
	MENU_ACTION_BAN_LIST,
	MENU_ACTION_CHARACTER_CHART,
	MENU_ACTION_DIRECT_CHAT,
	MENU_ACTION_FILE_TRANSFERS,
	MENU_ACTION_FRIENDS_LIST,
	MENU_ACTION_IGNORE_LIST,
	MENU_ACTION_PLUGINS_AND_SCRIPTS,
	MENU_ACTION_RAW_LOG,
	MENU_ACTION_URL_GRABBER,
	MENU_ACTION_CLEAR_TEXT,
	MENU_ACTION_SAVE_TEXT
} menu_action_id;

struct mymenu
{
	char *text;
	void *callback;
	char *image;
	unsigned char type;	/* M_XXX */
	unsigned char id;		/* MENU_ID_XXX (menu.h) */
	unsigned char state;	/* ticked or not? */
	unsigned char sensitive;	/* shaded out? */
	guint key;				/* GDK_KEY_x */
	const char *action_name;
	menu_action_id action_id;
	const char *action_target;
};

#define XCMENU_DOLIST 1
#define XCMENU_SHADED 1
#define XCMENU_MARKUP 2
#define XCMENU_MNEMONIC 4

/* execute a userlistbutton/popupmenu command */

static void
nick_command (session * sess, char *cmd)
{
	if (*cmd == '!')
		zoitechat_exec (cmd + 1);
	else
		handle_command (sess, cmd, TRUE);
}

/* fill in the %a %s %n etc and execute the command */

void
nick_command_parse (session *sess, char *cmd, char *nick, char *allnick)
{
	char *buf;
	char *host = _("Host unknown");
	char *account = _("Account unknown");
	struct User *user;
	int len;

/*	if (sess->type == SESS_DIALOG)
	{
		buf = (char *)(GTK_ENTRY (sess->gui->topic_entry)->text);
		buf = strrchr (buf, '@');
		if (buf)
			host = buf + 1;
	} else*/
	{
		user = userlist_find (sess, nick);
		if (user)
		{
			if (user->hostname)
				host = strchr (user->hostname, '@') + 1;
			if (user->account)
				account = user->account;
		}
	}

	/* this can't overflow, since popup->cmd is only 256 */
	len = strlen (cmd) + strlen (nick) + strlen (allnick) + 512;
	buf = g_malloc (len);

	auto_insert (buf, len, cmd, 0, 0, allnick, sess->channel, "",
					 server_get_network (sess->server, TRUE), host,
					 sess->server->nick, nick, account);

	nick_command (sess, buf);

	g_free (buf);
}

/* userlist button has been clicked */

void
userlist_button_cb (GtkWidget * button, char *cmd)
{
	int i, num_sel, using_allnicks = FALSE;
	char **nicks, *allnicks;
	char *nick = NULL;
	session *sess;

	sess = current_sess;

	if (strstr (cmd, "%a"))
		using_allnicks = TRUE;

	if (sess->type == SESS_DIALOG)
	{
		/* fake a selection */
		nicks = g_new (char *, 2);
		nicks[0] = g_strdup (sess->channel);
		nicks[1] = NULL;
		num_sel = 1;
	}
	else
	{
		/* find number of selected rows */
		nicks = userlist_selection_list (sess->gui->user_tree, &num_sel);
		if (num_sel < 1)
		{
			nick_command_parse (sess, cmd, "", "");

			g_free (nicks);
			return;
		}
	}

	/* create "allnicks" string */
	allnicks = g_malloc (((NICKLEN + 1) * num_sel) + 1);
	*allnicks = 0;

	i = 0;
	while (nicks[i])
	{
		if (i > 0)
			strcat (allnicks, " ");
		strcat (allnicks, nicks[i]);

		if (!nick)
			nick = nicks[0];

		/* if not using "%a", execute the command once for each nickname */
		if (!using_allnicks)
			nick_command_parse (sess, cmd, nicks[i], "");

		i++;
	}

	if (using_allnicks)
	{
		if (!nick)
			nick = "";
		nick_command_parse (sess, cmd, nick, allnicks);
	}

	while (num_sel)
	{
		num_sel--;
		g_free (nicks[num_sel]);
	}

	g_free (nicks);
	g_free (allnicks);
}

/* a popup-menu-item has been selected */

static void
popup_menu_cb (GtkWidget * item, char *cmd)
{
	char *nick;

	/* the userdata is set in menu_quick_item() */
	nick = g_object_get_data (G_OBJECT (item), "u");

	if (!nick)	/* userlist popup menu */
	{
		/* treat it just like a userlist button */
		userlist_button_cb (NULL, cmd);
		return;
	}

	if (!current_sess)	/* for url grabber window */
		nick_command_parse (sess_list->data, cmd, nick, nick);
	else
		nick_command_parse (current_sess, cmd, nick, nick);
}

GtkWidget *
menu_toggle_item (char *label, GtkWidget *menu, void *callback, void *userdata,
						int state)
{
	GtkWidget *item;

	item = gtk_check_menu_item_new_with_mnemonic (label);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*)item, state);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	g_signal_connect (G_OBJECT (item), "activate",
							G_CALLBACK (callback), userdata);
	gtk_widget_show (item);

	return item;
}

GtkWidget *
menu_quick_item (char *cmd, char *label, GtkWidget * menu, int flags,
					  gpointer userdata, char *icon)
{
	GtkWidget *img, *item;
	GtkWidget *box;
	GtkWidget *image = NULL;
	GtkWidget *label_widget;

	if (!label)
		item = gtk_menu_item_new ();
	else
	{
		if (icon)
		{
			/*if (flags & XCMENU_MARKUP)
				item = gtk_image_menu_item_new_with_markup (label);
			else*/
			img = menu_icon_widget_new (icon);

			item = gtk_menu_item_new ();
			box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
			if (img)
				image = img;
			label_widget = gtk_label_new_with_mnemonic (label);
			if (image)
				gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);
			gtk_box_pack_start (GTK_BOX (box), label_widget, FALSE, FALSE, 0);
			gtk_container_add (GTK_CONTAINER (item), box);
		}
		else
		{
			if (flags & XCMENU_MARKUP)
			{
				item = gtk_menu_item_new_with_label ("");
				if (flags & XCMENU_MNEMONIC)
				{
					gtk_label_set_markup_with_mnemonic (GTK_LABEL (gtk_bin_get_child (GTK_BIN (item))), label);
				} else
				{
					gtk_label_set_markup (GTK_LABEL (gtk_bin_get_child (GTK_BIN (item))), label);
				}
			} else
			{
				if (flags & XCMENU_MNEMONIC)
					item = gtk_menu_item_new_with_mnemonic (label);
				else
					item = gtk_menu_item_new_with_label (label);
			}
		}
	}
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	g_object_set_data (G_OBJECT (item), "u", userdata);
	if (cmd)
		g_signal_connect (G_OBJECT (item), "activate",
								G_CALLBACK (popup_menu_cb), cmd);
	if (flags & XCMENU_SHADED)
		gtk_widget_set_sensitive (GTK_WIDGET (item), FALSE);
	gtk_widget_show_all (item);

	return item;
}

static void
menu_quick_item_with_callback (void *callback, char *label, GtkWidget * menu,
										 void *arg)
{
	GtkWidget *item;

	item = gtk_menu_item_new_with_label (label);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	g_signal_connect (G_OBJECT (item), "activate",
							G_CALLBACK (callback), arg);
	gtk_widget_show (item);
}

GtkWidget *
menu_quick_sub (char *name, GtkWidget *menu, GtkWidget **sub_item_ret, int flags, int pos)
{
	GtkWidget *sub_menu;
	GtkWidget *sub_item;

	if (!name)
		return menu;

	/* Code to add a submenu */
	sub_menu = menu_new ();
	if (flags & XCMENU_MARKUP)
	{
		sub_item = gtk_menu_item_new_with_label ("");
		gtk_label_set_markup (GTK_LABEL (gtk_bin_get_child (GTK_BIN (sub_item))), name);
	}
	else
	{
		if (flags & XCMENU_MNEMONIC)
			sub_item = gtk_menu_item_new_with_mnemonic (name);
		else
			sub_item = gtk_menu_item_new_with_label (name);
	}
	gtk_menu_shell_insert (GTK_MENU_SHELL (menu), sub_item, pos);
	gtk_widget_show (sub_item);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (sub_item), sub_menu);

	if (sub_item_ret)
		*sub_item_ret = sub_item;

	if (flags & XCMENU_DOLIST)
		/* We create a new element in the list */
		submenu_list = g_slist_prepend (submenu_list, sub_menu);
	return sub_menu;
}

static GtkWidget *
menu_quick_endsub (void)
{
	/* Just delete the first element in the linked list pointed to by first */
	if (submenu_list)
		submenu_list = g_slist_remove (submenu_list, submenu_list->data);

	if (submenu_list)
		return (submenu_list->data);
	else
		return NULL;
}

static void
toggle_cb (GtkWidget *item, char *pref_name)
{
	char buf[256];

	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (item)))
		g_snprintf (buf, sizeof (buf), "set %s 1", pref_name);
	else
		g_snprintf (buf, sizeof (buf), "set %s 0", pref_name);

	handle_command (current_sess, buf, FALSE);
}

static int
is_in_path (char *cmd)
{
	char *orig = g_strdup (cmd + 1);	/* 1st char is "!" */
	char *prog = orig;
	char **argv;
	int argc;

	/* special-case these default entries. */
	/*                  123456789012345678 */
	if (strncmp (prog, "gnome-terminal -x ", 18) == 0)
	/* don't check for gnome-terminal, but the thing it's executing! */
		prog += 18;

	if (g_shell_parse_argv (prog, &argc, &argv, NULL))
	{
		char *path = g_find_program_in_path (argv[0]);
		g_strfreev (argv);
		if (path)
		{
			g_free (path);
			g_free (orig);
			return 1;
		}
	}

	g_free (orig);
	return 0;
}

/* syntax: "LABEL~ICON~STUFF~ADDED~LATER~" */

static void
menu_extract_icon (char *name, char **label, char **icon)
{
	char *p = name;
	char *start = NULL;
	char *end = NULL;

	while (*p)
	{
		if (*p == '~')
		{
			/* escape \~ */
			if (p == name || p[-1] != '\\')
			{
				if (!start)
					start = p + 1;
				else if (!end)
					end = p + 1;
			}
		}
		p++;
	}

	if (!end)
		end = p;

	if (start && start != end)
	{
		*label = g_strndup (name, (start - name) - 1);
		*icon = g_strndup (start, (end - start) - 1);
	}
	else
	{
		*label = g_strdup (name);
		*icon = NULL;
	}
}

/* append items to "menu" using the (struct popup*) list provided */

void
menu_create (GtkWidget *menu, GSList *list, char *target, int check_path)
{
	struct popup *pop;
	GtkWidget *tempmenu = menu, *subitem = NULL;
	int childcount = 0;

	submenu_list = g_slist_prepend (0, menu);
	while (list)
	{
		pop = (struct popup *) list->data;

		if (!g_ascii_strncasecmp (pop->name, "SUB", 3))
		{
			childcount = 0;
			tempmenu = menu_quick_sub (pop->cmd, tempmenu, &subitem, XCMENU_DOLIST|XCMENU_MNEMONIC, -1);

		} else if (!g_ascii_strncasecmp (pop->name, "TOGGLE", 6))
		{
			childcount++;
			menu_toggle_item (pop->name + 7, tempmenu, toggle_cb, pop->cmd,
									cfg_get_bool (pop->cmd));

		} else if (!g_ascii_strncasecmp (pop->name, "ENDSUB", 6))
		{
			/* empty sub menu due to no programs in PATH? */
			if (check_path && childcount < 1)
				gtk_widget_destroy (subitem);
			subitem = NULL;

			if (tempmenu != menu)
				tempmenu = menu_quick_endsub ();
			/* If we get here and tempmenu equals menu that means we havent got any submenus to exit from */

		} else if (!g_ascii_strncasecmp (pop->name, "SEP", 3))
		{
			menu_quick_item (0, 0, tempmenu, XCMENU_SHADED, 0, 0);

		} else
		{
			char *icon, *label;

			/* default command in zoitechat.c */
			if (pop->cmd[0] == 'n' && !strcmp (pop->cmd, "notify -n ASK %s"))
			{
				/* don't create this item if already in notify list */
				if (!target || notify_is_in_list (current_sess->server, target))
				{
					list = list->next;
					continue;
				}
			}

			menu_extract_icon (pop->name, &label, &icon);

			if (!check_path || pop->cmd[0] != '!')
			{
				menu_quick_item (pop->cmd, label, tempmenu, 0, target, icon);
			/* check if the program is in path, if not, leave it out! */
			} else if (is_in_path (pop->cmd))
			{
				childcount++;
				menu_quick_item (pop->cmd, label, tempmenu, 0, target, icon);
			}

			g_free (label);
			g_free (icon);
		}

		list = list->next;
	}

	/* Let's clean up the linked list from mem */
	while (submenu_list)
		submenu_list = g_slist_remove (submenu_list, submenu_list->data);
}

static char *str_copy = NULL;		/* for all pop-up menus */
#if GTK_MAJOR_VERSION < 4
static GtkWidget *nick_submenu = NULL;	/* user info submenu */
#endif

static void
menu_destroy (GtkWidget *menu, gpointer objtounref)
{
	gtk_widget_destroy (menu);
	g_object_unref (menu);
	if (objtounref)
		g_object_unref (G_OBJECT (objtounref));
#if GTK_MAJOR_VERSION < 4
	nick_submenu = NULL;
#endif
}

#if GTK_MAJOR_VERSION < 4
static void
menu_popup_at (GtkWidget *menu, GtkWidget *origin, gdouble x, gdouble y,
	GdkModifierType state, guint button, gpointer objtounref)
{
	GdkEventButton event;
	gint origin_x = 0;
	gint origin_y = 0;

	memset (&event, 0, sizeof (event));
	event.type = GDK_BUTTON_PRESS;
	event.window = origin ? gtk_widget_get_window (origin) : NULL;
	event.send_event = TRUE;
	event.time = GDK_CURRENT_TIME;
	event.x = x;
	event.y = y;
	event.state = state;
	event.button = button;
	if (event.window)
	{
		gdk_window_get_origin (event.window, &origin_x, &origin_y);
		gtk_menu_set_screen (GTK_MENU (menu), gdk_window_get_screen (event.window));
	}
	event.x_root = origin_x + x;
	event.y_root = origin_y + y;

	g_object_ref (menu);
	g_object_ref_sink (menu);
	g_object_unref (menu);
	g_signal_connect (G_OBJECT (menu), "selection-done",
							G_CALLBACK (menu_destroy), objtounref);
	gtk_menu_popup_at_pointer (GTK_MENU (menu), (GdkEvent *)&event);
}
#else
static void
menu_popup_at (GtkWidget *menu, GtkWidget *origin, gdouble x, gdouble y,
	GdkModifierType state, guint button, gpointer objtounref)
{
	(void)menu;
	(void)origin;
	(void)x;
	(void)y;
	(void)state;
	(void)button;
	(void)objtounref;
	g_warning ("GTK4 context menu presenter is not implemented");
}
#endif

#if GTK_MAJOR_VERSION < 4
static void
menu_nickinfo_cb (GtkWidget *menu, session *sess)
{
	char buf[512];

	if (!is_session (sess))
		return;

	/* issue a /WHOIS */
	g_snprintf (buf, sizeof (buf), "WHOIS %s %s", str_copy, str_copy);
	handle_command (sess, buf, FALSE);
	/* and hide the output */
	sess->server->skip_next_whois = 1;
}

static void
copy_to_clipboard_cb (GtkWidget *item, char *url)
{
	gtkutil_copy_to_clipboard (item, url);
}

/* returns boolean: Some data is missing */

static gboolean
menu_create_nickinfo_menu (struct User *user, GtkWidget *submenu)
{
	char buf[512];
	char unknown[96];
	char *real, *fmt, *users_country;
	struct away_msg *away;
	gboolean missing = FALSE;
	GtkWidget *item;

	/* let the translators tweak this if need be */
	fmt = _("<tt><b>%-11s</b></tt> %s");
	g_snprintf (unknown, sizeof (unknown), "<i>%s</i>", _("Unknown"));

	if (user->realname)
	{
		real = strip_color (user->realname, -1, STRIP_ALL|STRIP_ESCMARKUP);
		g_snprintf (buf, sizeof (buf), fmt, _("Real Name:"), real);
		g_free (real);
	} else
	{
		g_snprintf (buf, sizeof (buf), fmt, _("Real Name:"), unknown);
	}
	item = menu_quick_item (0, buf, submenu, XCMENU_MARKUP, 0, 0);
	g_signal_connect (G_OBJECT (item), "activate",
							G_CALLBACK (copy_to_clipboard_cb), 
							user->realname ? user->realname : unknown);

	g_snprintf (buf, sizeof (buf), fmt, _("User:"),
				 user->hostname ? user->hostname : unknown);
	item = menu_quick_item (0, buf, submenu, XCMENU_MARKUP, 0, 0);
	g_signal_connect (G_OBJECT (item), "activate",
							G_CALLBACK (copy_to_clipboard_cb), 
							user->hostname ? user->hostname : unknown);
	
	g_snprintf (buf, sizeof (buf), fmt, _("Account:"),
				 user->account ? user->account : unknown);
	item = menu_quick_item (0, buf, submenu, XCMENU_MARKUP, 0, 0);
	g_signal_connect (G_OBJECT (item), "activate",
							G_CALLBACK (copy_to_clipboard_cb), 
							user->account ? user->account : unknown);

	users_country = country (user->hostname);
	if (users_country)
	{
		g_snprintf (buf, sizeof (buf), fmt, _ ("Country:"), users_country);
		item = menu_quick_item (0, buf, submenu, XCMENU_MARKUP, 0, 0);
		g_signal_connect (G_OBJECT (item), "activate",
			G_CALLBACK (copy_to_clipboard_cb), users_country);
	}

	g_snprintf (buf, sizeof (buf), fmt, _("Server:"),
				 user->servername ? user->servername : unknown);
	item = menu_quick_item (0, buf, submenu, XCMENU_MARKUP, 0, 0);
	g_signal_connect (G_OBJECT (item), "activate",
							G_CALLBACK (copy_to_clipboard_cb), 
							user->servername ? user->servername : unknown);

	if (user->lasttalk)
	{
		char min[96];

		g_snprintf (min, sizeof (min), _("%u minutes ago"),
					(unsigned int) ((time (0) - user->lasttalk) / 60));
		g_snprintf (buf, sizeof (buf), fmt, _("Last Msg:"), min);
	} else
	{
		g_snprintf (buf, sizeof (buf), fmt, _("Last Msg:"), unknown);
	}
	menu_quick_item (0, buf, submenu, XCMENU_MARKUP, 0, 0);

	if (user->away)
	{
		away = server_away_find_message (current_sess->server, user->nick);
		if (away)
		{
			char *msg = strip_color (away->message ? away->message : unknown, -1, STRIP_ALL|STRIP_ESCMARKUP);
			g_snprintf (buf, sizeof (buf), fmt, _("Away Msg:"), msg);
			g_free (msg);
			item = menu_quick_item (0, buf, submenu, XCMENU_MARKUP, 0, 0);
			g_signal_connect (G_OBJECT (item), "activate",
									G_CALLBACK (copy_to_clipboard_cb), 
									away->message ? away->message : unknown);
		}
		else
			missing = TRUE;
	}

	return missing;
}

void
fe_userlist_update (session *sess, struct User *user)
{
	GList *items, *next;
	GList *iter;
	gboolean needs_refresh;

	if (!nick_submenu || !str_copy)
		return;

	/* not the same nick as the menu? */
	if (sess->server->p_cmp (user->nick, str_copy))
		return;

	/* get rid of the "show" signal */
	g_signal_handlers_disconnect_by_func (nick_submenu, menu_nickinfo_cb, sess);

	/* destroy all the old items */
	items = gtk_container_get_children (GTK_CONTAINER (nick_submenu));
	iter = items;
	while (iter)
	{
		next = iter->next;
		gtk_widget_destroy (iter->data);
		iter = next;
	}
	g_list_free (items);

	/* and re-create them with new info */
	needs_refresh = menu_create_nickinfo_menu (user, nick_submenu) ||
		!user->hostname || !user->realname || !user->servername;

	if (needs_refresh)
	{
		g_signal_connect (G_OBJECT (nick_submenu), "show",
						 G_CALLBACK (menu_nickinfo_cb), sess);
	}
}


static void
menu_reply_to_latest_cb (GtkWidget *wid, gpointer data)
{
	reply_item *item;

	item = reply_cache_latest_from (current_sess, str_copy);
	if (!item)
	{
		PrintText (current_sess, _("No recent message to reply to.\n"));
		return;
	}

	reply_state_set (current_sess, item->msgid, current_sess->channel, item->nick, item->text);
	mg_reply_update (current_sess);
	if (current_sess->gui && current_sess->gui->input_box)
		gtk_widget_grab_focus (current_sess->gui->input_box);
}
#endif

#if GTK_MAJOR_VERSION >= 4
#define FABULOR_NICK_CONTEXT_POPUP "fabulor-nick-context-popup"

typedef struct
{
	FabulorNickContextMenuModel *model;
	FabulorContextMenuPresenterGtk4 *presenter;
	char *nick;
	char *network_name;
	int num_sel;
	GWeakRef origin;
} FabulorNickContextPopup;

static GWeakRef *
menu_nick_context_origin_ref (void)
{
	static GWeakRef origin_ref;
	static gsize initialized;

	if (g_once_init_enter (&initialized))
	{
		g_weak_ref_init (&origin_ref, NULL);
		g_once_init_leave (&initialized, 1);
	}
	return &origin_ref;
}

static void
menu_nick_context_deactivate (void)
{
	g_weak_ref_set (menu_nick_context_origin_ref (), NULL);
}

static void
menu_nick_context_popup_free (gpointer data)
{
	FabulorNickContextPopup *popup = data;
	if (!popup)
		return;
	fabulor_context_menu_presenter_gtk4_free (popup->presenter);
	fabulor_nick_context_menu_model_free (popup->model);
	g_free (popup->nick);
	g_free (popup->network_name);
	g_weak_ref_clear (&popup->origin);
	g_free (popup);
}

static void
menu_nick_context_dispatch (FabulorNickContextAction action,
	const char *nick, const char *command, gboolean selection_dispatch,
	gpointer user_data)
{
	FabulorNickContextPopup *popup = user_data;
	session *sess;

	if (action == FABULOR_NICK_CONTEXT_COPY_INFO)
	{
		if (command)
		{
			GObject *origin = g_weak_ref_get (&popup->origin);
			if (origin)
			{
				gtkutil_copy_to_clipboard (GTK_WIDGET (origin), (char *)command);
				g_object_unref (origin);
			}
		}
		return;
	}
	if (action == FABULOR_NICK_CONTEXT_REPLY)
	{
		reply_item *item;
		if (!current_sess)
			return;
		item = reply_cache_latest_from (current_sess, nick);
		if (!item)
		{
			PrintText (current_sess, _("No recent message to reply to.\n"));
			return;
		}
		reply_state_set (current_sess, item->msgid, current_sess->channel,
			item->nick, item->text);
		mg_reply_update (current_sess);
		if (current_sess->gui && current_sess->gui->input_box)
			gtk_widget_grab_focus (current_sess->gui->input_box);
		return;
	}
	if (action != FABULOR_NICK_CONTEXT_COMMAND || !command)
		return;
	if (selection_dispatch)
	{
		if (current_sess)
			userlist_button_cb (NULL, (char *)command);
		return;
	}
	sess = current_sess ? current_sess : (sess_list ? sess_list->data : NULL);
	if (sess)
		nick_command_parse (sess, (char *)command, (char *)nick, (char *)nick);
}

static void
menu_nick_handler_clear (gpointer data)
{
	FabulorNickHandler *handler = data;
	g_free ((char *)handler->label);
	g_free ((char *)handler->icon);
}

static GArray *
menu_nick_handlers_snapshot (session *sess, char *target)
{
	GArray *handlers = g_array_new (FALSE, FALSE, sizeof (FabulorNickHandler));
	GSList *list;

	g_array_set_clear_func (handlers, menu_nick_handler_clear);
	for (list = popup_list; list; list = list->next)
	{
		struct popup *pop = list->data;
		FabulorNickHandler handler = { 0 };

		handler.enabled = TRUE;
		if (!g_ascii_strncasecmp (pop->name, "SUB", 3))
		{
			handler.kind = FABULOR_NICK_HANDLER_SUBMENU_BEGIN;
			handler.label = g_strdup (pop->cmd);
		}
		else if (!g_ascii_strncasecmp (pop->name, "TOGGLE", 6))
		{
			handler.kind = FABULOR_NICK_HANDLER_TOGGLE;
			handler.label = g_strdup (pop->name + 7);
			handler.command = pop->cmd;
			handler.active = cfg_get_bool (pop->cmd);
		}
		else if (!g_ascii_strncasecmp (pop->name, "ENDSUB", 6))
			handler.kind = FABULOR_NICK_HANDLER_SUBMENU_END;
		else if (!g_ascii_strncasecmp (pop->name, "SEP", 3))
			handler.kind = FABULOR_NICK_HANDLER_SEPARATOR;
		else
		{
			char *icon;
			char *label;
			if (pop->cmd[0] == 'n' &&
				!strcmp (pop->cmd, "notify -n ASK %s") &&
				(!target || notify_is_in_list (sess->server, target)))
				continue;
			handler.kind = FABULOR_NICK_HANDLER_COMMAND;
			menu_extract_icon (pop->name, &label, &icon);
			handler.label = label;
			handler.icon = icon;
			handler.command = pop->cmd;
		}
		g_array_append_val (handlers, handler);
	}
	return handlers;
}

static void
menu_nick_info_clear (gpointer data)
{
	FabulorNickInfoItem *info = data;
	g_free ((char *)info->label);
	g_free ((char *)info->value);
}

static void
menu_nick_info_append (GArray *items, const char *name, const char *display,
	const char *value)
{
	FabulorNickInfoItem item;
	item.label = g_strdup_printf ("%s %s", name, display);
	item.value = g_strdup (value);
	g_array_append_val (items, item);
}

static GArray *
menu_nick_info_snapshot (session *sess, struct User *user,
	gboolean *needs_refresh)
{
	GArray *items = g_array_new (FALSE, FALSE, sizeof (FabulorNickInfoItem));
	const char *unknown = _("Unknown");
	char *real = NULL;
	char *users_country;
	char last_message[96];
	struct away_msg *away;

	g_array_set_clear_func (items, menu_nick_info_clear);
	*needs_refresh = FALSE;
	if (!user)
		return items;
	if (user->realname)
		real = strip_color (user->realname, -1, STRIP_ALL | STRIP_ESCMARKUP);
	menu_nick_info_append (items, _("Real Name:"),
		real ? real : unknown, user->realname ? user->realname : unknown);
	g_free (real);
	menu_nick_info_append (items, _("User:"),
		user->hostname ? user->hostname : unknown,
		user->hostname ? user->hostname : unknown);
	menu_nick_info_append (items, _("Account:"),
		user->account ? user->account : unknown,
		user->account ? user->account : unknown);
	users_country = country (user->hostname);
	if (users_country)
		menu_nick_info_append (items, _("Country:"), users_country,
			users_country);
	menu_nick_info_append (items, _("Server:"),
		user->servername ? user->servername : unknown,
		user->servername ? user->servername : unknown);
	if (user->lasttalk)
		g_snprintf (last_message, sizeof last_message, _("%u minutes ago"),
			(unsigned int)((time (NULL) - user->lasttalk) / 60));
	else
		g_strlcpy (last_message, unknown, sizeof last_message);
	menu_nick_info_append (items, _("Last Msg:"), last_message, NULL);
	if (user->away)
	{
		away = server_away_find_message (sess->server, user->nick);
		if (away)
		{
			char *message = strip_color (away->message ? away->message : unknown,
				-1, STRIP_ALL | STRIP_ESCMARKUP);
			menu_nick_info_append (items, _("Away Msg:"), message,
				away->message ? away->message : unknown);
			g_free (message);
		}
		else
			*needs_refresh = TRUE;
	}
	if (!user->hostname || !user->realname || !user->servername)
		*needs_refresh = TRUE;
	return items;
}

static FabulorNickContextMenuModel *
menu_nick_context_model_snapshot (FabulorNickContextPopup *popup,
	session *sess, GtkWidget *origin, struct User *user)
{
	GMenuModel *plugin_model = menu_plugin_context_model (G_OBJECT (origin));
	GArray *handlers;
	GArray *info_items;
	char heading[96];
	const char *heading_text = NULL;
	char *command_target = popup->num_sel > 1 ? NULL : popup->nick;
	gboolean needs_refresh = FALSE;
	FabulorNickContextMenuModel *model;

	if (popup->num_sel > 1)
	{
		g_snprintf (heading, sizeof heading, _("%d nicks selected."),
			popup->num_sel);
		heading_text = heading;
	}
	else if (user)
		heading_text = popup->nick;
	handlers = menu_nick_handlers_snapshot (sess, command_target);
	info_items = menu_nick_info_snapshot (sess, user, &needs_refresh);
	model = fabulor_nick_context_menu_model_new_with_details (popup->nick,
		heading_text, popup->num_sel <= 1, _("Reply"), popup->num_sel > 1,
		(FabulorNickHandler *)handlers->data, handlers->len,
		(FabulorNickInfoItem *)info_items->data, info_items->len,
		needs_refresh, plugin_model, menu_nick_context_dispatch, popup);
	g_array_unref (info_items);
	g_array_unref (handlers);
	return model;
}

static void
menu_nick_context_refresh (FabulorNickContextPopup *popup, session *sess,
	GtkWidget *origin, struct User *user)
{
	FabulorNickContextMenuModel *model;
	GActionGroup *plugin_actions;

	model = menu_nick_context_model_snapshot (popup, sess, origin, user);
	if (!model)
		return;
	plugin_actions = menu_plugin_context_actions (G_OBJECT (origin));
	if (!fabulor_context_menu_presenter_gtk4_set_projection (popup->presenter,
		fabulor_nick_context_menu_model_get_menu (model),
		fabulor_nick_context_menu_model_get_actions (model), plugin_actions))
	{
		fabulor_nick_context_menu_model_free (model);
		return;
	}
	fabulor_nick_context_menu_model_free (popup->model);
	popup->model = model;
}

void
fe_userlist_update (session *sess, struct User *user)
{
	GObject *origin;
	FabulorNickContextPopup *popup;
	const char *network_name;

	if (!is_session (sess) || !user)
		return;
	origin = g_weak_ref_get (menu_nick_context_origin_ref ());
	if (!origin)
		return;
	popup = g_object_get_data (origin, FABULOR_NICK_CONTEXT_POPUP);
	network_name = server_get_network (sess->server, TRUE);
	if (popup && popup->num_sel <= 1 &&
		!sess->server->p_cmp (user->nick, popup->nick) &&
		!g_strcmp0 (network_name, popup->network_name))
		menu_nick_context_refresh (popup, sess, GTK_WIDGET (origin), user);
	g_object_unref (origin);
}

static void
menu_nick_request_info (session *sess, const char *nick)
{
	char command[512];
	if (!is_session (sess))
		return;
	g_snprintf (command, sizeof command, "WHOIS %s %s", nick, nick);
	handle_command (sess, command, FALSE);
	sess->server->skip_next_whois = 1;
}

static void
menu_nickmenu_gtk4 (session *sess, GtkWidget *origin, gdouble x, gdouble y,
	char *nick, int num_sel)
{
	FabulorNickContextPopup *popup;
	GActionGroup *plugin_actions;
	struct User *user = NULL;

	if (num_sel <= 1)
	{
		user = userlist_find (sess, nick);
		if (!user)
			user = userlist_find_global (sess->server, nick);
	}
	menu_add_plugin_model (G_OBJECT (origin), "\x5$NICK",
		num_sel == 0 ? nick : NULL);
	plugin_actions = menu_plugin_context_actions (G_OBJECT (origin));
	popup = g_new0 (FabulorNickContextPopup, 1);
	popup->nick = g_strdup (nick);
	popup->network_name = g_strdup (server_get_network (sess->server, TRUE));
	popup->num_sel = num_sel;
	g_weak_ref_init (&popup->origin, G_OBJECT (origin));
	popup->model = menu_nick_context_model_snapshot (popup, sess, origin, user);
	if (!popup->model)
	{
		menu_nick_context_popup_free (popup);
		return;
	}
	popup->presenter = fabulor_context_menu_presenter_gtk4_new (
		fabulor_nick_context_menu_model_get_menu (popup->model),
		fabulor_nick_context_menu_model_get_actions (popup->model),
		plugin_actions);
	if (!popup->presenter)
	{
		menu_nick_context_popup_free (popup);
		return;
	}
	g_object_set_data_full (G_OBJECT (origin), FABULOR_NICK_CONTEXT_POPUP,
		popup, menu_nick_context_popup_free);
	g_weak_ref_set (menu_nick_context_origin_ref (), G_OBJECT (origin));
	if (fabulor_nick_context_menu_model_needs_info_refresh (popup->model))
		menu_nick_request_info (sess, nick);
	fabulor_context_menu_presenter_gtk4_popup_at (popup->presenter, origin, x, y);
}
#endif

void
menu_nickmenu_at (session *sess, GtkWidget *origin, gdouble x, gdouble y,
	GdkModifierType state, char *nick, int num_sel)
{
#if GTK_MAJOR_VERSION >= 4
	(void)state;
	menu_nickmenu_gtk4 (sess, origin, x, y, nick, num_sel);
#else
	char buf[512];
	struct User *user;
	GtkWidget *submenu, *menu = menu_new ();

	g_free (str_copy);
	str_copy = g_strdup (nick);

	submenu_list = 0;	/* first time through, might not be 0 */

	/* more than 1 nick selected? */
	if (num_sel > 1)
	{
		g_snprintf (buf, sizeof (buf), _("%d nicks selected."), num_sel);
		menu_quick_item (0, buf, menu, 0, 0, 0);
		menu_quick_item (0, 0, menu, XCMENU_SHADED, 0, 0);
	} else
	{
		user = userlist_find (sess, nick);	/* lasttalk is channel specific */
		if (!user)
			user = userlist_find_global (current_sess->server, nick);
		if (user)
		{
			nick_submenu = submenu = menu_quick_sub (nick, menu, NULL, XCMENU_DOLIST, -1);

			if (menu_create_nickinfo_menu (user, submenu) ||
				 !user->hostname || !user->realname || !user->servername)
			{
				g_signal_connect (G_OBJECT (submenu), "show", G_CALLBACK (menu_nickinfo_cb), sess);
			}

			menu_quick_endsub ();
			menu_quick_item (0, 0, menu, XCMENU_SHADED, 0, 0);
		}
	}

	if (num_sel > 1)
		menu_create (menu, popup_list, NULL, FALSE);
	else
		menu_create (menu, popup_list, str_copy, FALSE);

	if (num_sel <= 1)
	{
		menu_quick_item_with_callback (menu_reply_to_latest_cb, _("Reply"), menu, 0);
		menu_quick_item (0, 0, menu, XCMENU_SHADED, 0, 0);
	}

	if (num_sel == 0)	/* xtext click */
		menu_add_plugin_items (menu, "\x5$NICK", str_copy);
	else	/* userlist treeview click */
		menu_add_plugin_items (menu, "\x5$NICK", NULL);

	menu_popup_at (menu, origin, x, y, state, 3, NULL);
#endif
}

/* stuff for the View menu */

static void
menu_showhide_cb (session *sess)
{
	if (!sess->gui->menu || !GTK_IS_WIDGET (sess->gui->menu))
		return;

	if (prefs.hex_gui_hide_menu)
		gtk_widget_hide (sess->gui->menu);
	else
		gtk_widget_show (sess->gui->menu);
}

static void
menu_topic_showhide_cb (session *sess)
{
	if (prefs.hex_gui_topicbar)
		gtk_widget_show (sess->gui->topic_bar);
	else
		gtk_widget_hide (sess->gui->topic_bar);
}

static void
menu_userlist_showhide_cb (session *sess)
{
	mg_decide_userlist (sess, TRUE);
}

static void
menu_ulbuttons_showhide_cb (session *sess)
{
	if (prefs.hex_gui_ulist_buttons)
		gtk_widget_show (sess->gui->button_box);
	else
		gtk_widget_hide (sess->gui->button_box);
}

static void
menu_cmbuttons_showhide_cb (session *sess)
{
	switch (sess->type)
	{
	case SESS_CHANNEL:
		if (prefs.hex_gui_mode_buttons)
			gtk_widget_show (sess->gui->topicbutton_box);
		else
			gtk_widget_hide (sess->gui->topicbutton_box);
		break;
	default:
		gtk_widget_hide (sess->gui->topicbutton_box);
	}
}

static void
menu_setting_foreach (void (*callback) (session *), int id, guint state)
{
	session *sess;
	GSList *list;
	int maindone = FALSE;	/* do it only once for EVERY tab */

	list = sess_list;
	while (list)
	{
		sess = list->data;
		if (!sess || !sess->gui)
		{
			list = list->next;
			continue;
		}

		if (!sess->gui->is_tab || !maindone)
		{
			if (sess->gui->is_tab)
				maindone = TRUE;
			if (id != -1)
			{
				GtkWidget *menu_item = sess->gui->menu_item[id];

				if (menu_item != NULL &&
					!menu_action_set_item_state (menu_item, state) &&
					GTK_IS_CHECK_MENU_ITEM (menu_item))
				{
					guint toggled_signal = g_signal_lookup ("toggled", G_OBJECT_TYPE (menu_item));

					if (toggled_signal != 0)
					{
						g_signal_handlers_block_matched (menu_item, G_SIGNAL_MATCH_ID, toggled_signal, 0, NULL, NULL, NULL);
						gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), state);
						g_signal_handlers_unblock_matched (menu_item, G_SIGNAL_MATCH_ID, toggled_signal, 0, NULL, NULL, NULL);
					}
					else
					{
						gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), state);
					}
				}
			}
			if (callback)
				callback (sess);
		}

		list = list->next;
	}
}

void
menu_bar_toggle (void)
{
	prefs.hex_gui_hide_menu = !prefs.hex_gui_hide_menu;
	menu_setting_foreach (menu_showhide_cb, MENU_ID_MENUBAR,
						  !prefs.hex_gui_hide_menu);
}

static void
menu_bar_toggle_cb (void)
{
	menu_bar_toggle ();
	if (prefs.hex_gui_hide_menu)
		fe_message (_("The Menubar is now hidden. You can show it again"
						  " by pressing Control+F9 or right-clicking in a blank part of"
						  " the main text area."), FE_MSG_INFO);
}

static void
menu_topicbar_toggle (GtkWidget *wid, gpointer ud)
{
	prefs.hex_gui_topicbar = !prefs.hex_gui_topicbar;
	menu_setting_foreach (menu_topic_showhide_cb, MENU_ID_TOPICBAR,
								 prefs.hex_gui_topicbar);
}

static void
menu_userlist_toggle (GtkWidget *wid, gpointer ud)
{
	prefs.hex_gui_ulist_hide = !prefs.hex_gui_ulist_hide;
	menu_setting_foreach (menu_userlist_showhide_cb, MENU_ID_USERLIST,
								 !prefs.hex_gui_ulist_hide);
}

static void
menu_ulbuttons_toggle (GtkWidget *wid, gpointer ud)
{
	prefs.hex_gui_ulist_buttons = !prefs.hex_gui_ulist_buttons;
	menu_setting_foreach (menu_ulbuttons_showhide_cb, MENU_ID_ULBUTTONS,
								 prefs.hex_gui_ulist_buttons);
}

static void
menu_cmbuttons_toggle (GtkWidget *wid, gpointer ud)
{
	prefs.hex_gui_mode_buttons = !prefs.hex_gui_mode_buttons;
	menu_setting_foreach (menu_cmbuttons_showhide_cb, MENU_ID_MODEBUTTONS,
								 prefs.hex_gui_mode_buttons);
}

static void
menu_fullscreen_toggle (GtkWidget *wid, gpointer ud)
{
	if (!prefs.hex_gui_win_fullscreen)
		gtk_window_fullscreen (GTK_WINDOW(parent_window));
	else
	{
		gtk_window_unfullscreen (GTK_WINDOW(parent_window));

#ifdef WIN32
		if (!prefs.hex_gui_win_state) /* not maximized */
		{
			/* other window managers seem to handle this */
			gtk_window_resize (GTK_WINDOW (parent_window),
				prefs.hex_gui_win_width, prefs.hex_gui_win_height);
			fabulor_gtk_window_move (GTK_WINDOW (parent_window),
				prefs.hex_gui_win_left, prefs.hex_gui_win_top);
		}
#endif
	}
}

void
menu_middlemenu_at (session *sess, GtkWidget *origin, gdouble x, gdouble y,
	GdkModifierType state)
{
#if GTK_MAJOR_VERSION >= 4
	(void) state;
	menu_middlemenu_gtk4 (sess, origin, x, y);
#else
	GtkWidget *menu;
	GtkAccelGroup *accel_group;

	accel_group = gtk_accel_group_new ();
	menu = menu_create_main (accel_group, FALSE, sess->server->is_away,
							 sess->server->connected,
							 sess->server->connected || sess->server->recondelay_tag,
							 sess->server->end_of_motd, !sess->gui->is_tab, NULL);
	menu_popup_at (menu, origin, x, y, state, 2, accel_group);
#endif
}

static void
open_url_cb (GtkWidget *item, char *url)
{
	char buf[512];

	/* pass this to /URL so it can handle irc:// */
	g_snprintf (buf, sizeof (buf), "URL %s", url);
	handle_command (current_sess, buf, FALSE);
}

#if GTK_MAJOR_VERSION >= 4
#define FABULOR_URL_CONTEXT_POPUP "fabulor-url-context-popup"

typedef struct
{
	FabulorUrlContextMenuModel *model;
	FabulorContextMenuPresenterGtk4 *presenter;
} FabulorUrlContextPopup;

static void
menu_url_context_popup_free (gpointer data)
{
	FabulorUrlContextPopup *popup = data;
	if (!popup)
		return;
	fabulor_context_menu_presenter_gtk4_free (popup->presenter);
	fabulor_url_context_menu_model_free (popup->model);
	g_free (popup);
}

static void
menu_url_context_dispatch (FabulorUrlContextAction action, const char *url,
	const char *command, gpointer user_data)
{
	if (action == FABULOR_URL_CONTEXT_COPY)
	{
		gtkutil_copy_to_clipboard (GTK_WIDGET (user_data), (char *)url);
		return;
	}
	if (action == FABULOR_URL_CONTEXT_OPEN)
	{
		char buf[512];
		g_snprintf (buf, sizeof (buf), "URL %s", url);
		if (current_sess)
			handle_command (current_sess, buf, FALSE);
		return;
	}
	if (!command)
		return;
	if (current_sess)
		nick_command_parse (current_sess, (char *)command, (char *)url,
			(char *)url);
	else if (sess_list)
		nick_command_parse (sess_list->data, (char *)command, (char *)url,
			(char *)url);
}

static void
menu_url_handler_clear (gpointer data)
{
	FabulorUrlHandler *handler = data;
	g_free ((char *)handler->label);
	g_free ((char *)handler->icon);
}

static GArray *
menu_url_handlers_snapshot (void)
{
	GArray *handlers = g_array_new (FALSE, FALSE, sizeof (FabulorUrlHandler));
	GSList *list;
	g_array_set_clear_func (handlers, menu_url_handler_clear);
	for (list = urlhandler_list; list; list = list->next)
	{
		struct popup *pop = list->data;
		FabulorUrlHandler handler = { 0 };

		handler.enabled = TRUE;
		if (!g_ascii_strncasecmp (pop->name, "SUB", 3))
		{
			handler.kind = FABULOR_URL_HANDLER_SUBMENU_BEGIN;
			handler.label = g_strdup (pop->cmd);
		}
		else if (!g_ascii_strncasecmp (pop->name, "TOGGLE", 6))
		{
			handler.kind = FABULOR_URL_HANDLER_TOGGLE;
			handler.label = g_strdup (pop->name + 7);
			handler.command = pop->cmd;
			handler.active = cfg_get_bool (pop->cmd);
		}
		else if (!g_ascii_strncasecmp (pop->name, "ENDSUB", 6))
			handler.kind = FABULOR_URL_HANDLER_SUBMENU_END;
		else if (!g_ascii_strncasecmp (pop->name, "SEP", 3))
			handler.kind = FABULOR_URL_HANDLER_SEPARATOR;
		else
		{
			char *icon;
			char *label;

			if (pop->cmd[0] == '!' && !is_in_path (pop->cmd))
				continue;
			handler.kind = FABULOR_URL_HANDLER_COMMAND;
			menu_extract_icon (pop->name, &label, &icon);
			handler.label = label;
			handler.icon = icon;
			handler.command = pop->cmd;
		}
		g_array_append_val (handlers, handler);
	}
	return handlers;
}

static void
menu_urlmenu_gtk4 (GtkWidget *origin, gdouble x, gdouble y, char *url)
{
	FabulorUrlContextPopup *popup;
	GActionGroup *plugin_actions;
	GMenuModel *plugin_model;
	GArray *handlers;

	menu_nick_context_deactivate ();
	menu_add_plugin_model (G_OBJECT (origin), "\x4$URL", url);
	plugin_model = menu_plugin_context_model (G_OBJECT (origin));
	plugin_actions = menu_plugin_context_actions (G_OBJECT (origin));
	handlers = menu_url_handlers_snapshot ();
	popup = g_new0 (FabulorUrlContextPopup, 1);
	popup->model = fabulor_url_context_menu_model_new_with_handlers (url,
		_("Open Link in Browser"), _("Connect"), _("Copy Selected Link"),
		(FabulorUrlHandler *)handlers->data, handlers->len, plugin_model,
		menu_url_context_dispatch, origin);
	g_array_unref (handlers);
	if (!popup->model)
	{
		g_free (popup);
		return;
	}
	popup->presenter = fabulor_context_menu_presenter_gtk4_new (
		fabulor_url_context_menu_model_get_menu (popup->model),
		fabulor_url_context_menu_model_get_actions (popup->model), plugin_actions);
	if (!popup->presenter)
	{
		menu_url_context_popup_free (popup);
		return;
	}
	g_object_set_data_full (G_OBJECT (origin), FABULOR_URL_CONTEXT_POPUP,
		popup, menu_url_context_popup_free);
	fabulor_context_menu_presenter_gtk4_popup_at (popup->presenter, origin, x, y);
}
#endif

void
menu_urlmenu_at (GtkWidget *origin, gdouble x, gdouble y,
	GdkModifierType state, char *url)
{
#if GTK_MAJOR_VERSION >= 4
	(void)state;
	menu_urlmenu_gtk4 (origin, x, y, url);
#else
	GtkWidget *menu;
	char *tmp, *chop;

	g_free (str_copy);
	str_copy = g_strdup (url);

	menu = menu_new ();
	/* more than 51 chars? Chop it */
	if (g_utf8_strlen (str_copy, -1) >= 52)
	{
		tmp = g_strdup (str_copy);
		chop = g_utf8_offset_to_pointer (tmp, 48);
		chop[0] = chop[1] = chop[2] = '.';
		chop[3] = 0;
		menu_quick_item (0, tmp, menu, XCMENU_SHADED, 0, 0);
		g_free (tmp);
	} else
	{
		menu_quick_item (0, str_copy, menu, XCMENU_SHADED, 0, 0);
	}
	menu_quick_item (0, 0, menu, XCMENU_SHADED, 0, 0);

	/* Two hardcoded entries */
	if (strncmp (str_copy, "irc://", 6) == 0 ||
	    strncmp (str_copy, "ircs://",7) == 0)
		menu_quick_item_with_callback (open_url_cb, _("Connect"), menu, str_copy);
	else
		menu_quick_item_with_callback (open_url_cb, _("Open Link in Browser"), menu, str_copy);
	menu_quick_item_with_callback (copy_to_clipboard_cb, _("Copy Selected Link"), menu, str_copy);
	/* custom ones from urlhandlers.conf */
	menu_create (menu, urlhandler_list, str_copy, TRUE);
	menu_add_plugin_items (menu, "\x4$URL", str_copy);
	menu_popup_at (menu, origin, x, y, state, 3, NULL);
#endif
}

static void
menu_chan_cycle (GtkWidget * menu, char *chan)
{
	char tbuf[256];

	if (current_sess)
	{
		g_snprintf (tbuf, sizeof tbuf, "CYCLE %s", chan);
		handle_command (current_sess, tbuf, FALSE);
	}
}

static void
menu_chan_part (GtkWidget * menu, char *chan)
{
	char tbuf[256];

	if (current_sess)
	{
		g_snprintf (tbuf, sizeof tbuf, "part %s", chan);
		handle_command (current_sess, tbuf, FALSE);
	}
}

static void
menu_chan_focus (GtkWidget * menu, char *chan)
{
	char tbuf[256];

	if (current_sess)
	{
		g_snprintf (tbuf, sizeof tbuf, "doat %s gui focus", chan);
		handle_command (current_sess, tbuf, FALSE);
	}
}

static void
menu_chan_join (GtkWidget * menu, char *chan)
{
	char tbuf[256];

	if (current_sess)
	{
		g_snprintf (tbuf, sizeof tbuf, "join %s", chan);
		handle_command (current_sess, tbuf, FALSE);
	}
}

#if GTK_MAJOR_VERSION >= 4
#define FABULOR_CHANNEL_CONTEXT_POPUP "fabulor-channel-context-popup"

typedef struct
{
	FabulorChannelContextMenuModel *model;
	FabulorContextMenuPresenterGtk4 *presenter;
	char *network_name;
} FabulorChannelContextPopup;

static void
menu_channel_context_popup_free (gpointer data)
{
	FabulorChannelContextPopup *popup = data;
	if (!popup)
		return;
	fabulor_context_menu_presenter_gtk4_free (popup->presenter);
	fabulor_channel_context_menu_model_free (popup->model);
	g_free (popup->network_name);
	g_free (popup);
}

static void
menu_channel_context_dispatch (FabulorChannelContextAction action,
	const char *channel, gboolean state, gpointer user_data)
{
	FabulorChannelContextPopup *popup = user_data;
	char command[256];
	ircnet *network;

	if (action == FABULOR_CHANNEL_CONTEXT_AUTOJOIN)
	{
		if (!popup->network_name)
			return;
		network = servlist_net_find (popup->network_name, NULL,
			g_ascii_strcasecmp);
		if (network)
			servlist_autojoinedit (network, (char *)channel, state);
		return;
	}
	if (!current_sess)
		return;
	if (action == FABULOR_CHANNEL_CONTEXT_JOIN)
		g_snprintf (command, sizeof command, "join %s", channel);
	else if (action == FABULOR_CHANNEL_CONTEXT_FOCUS)
		g_snprintf (command, sizeof command, "doat %s gui focus", channel);
	else if (action == FABULOR_CHANNEL_CONTEXT_PART)
		g_snprintf (command, sizeof command, "part %s", channel);
	else if (action == FABULOR_CHANNEL_CONTEXT_CYCLE)
		g_snprintf (command, sizeof command, "CYCLE %s", channel);
	else
		return;
	handle_command (current_sess, command, FALSE);
}

static void
menu_chanmenu_gtk4 (session *sess, GtkWidget *origin, gdouble x, gdouble y,
	char *channel)
{
	FabulorChannelContextPopup *popup;
	GActionGroup *plugin_actions;
	GMenuModel *plugin_model;
	ircnet *network = sess->server->network;
	session *channel_session = find_channel (sess->server, channel);

	menu_nick_context_deactivate ();
	menu_add_plugin_model (G_OBJECT (origin), "\x5$CHAN", channel);
	plugin_model = menu_plugin_context_model (G_OBJECT (origin));
	plugin_actions = menu_plugin_context_actions (G_OBJECT (origin));
	popup = g_new0 (FabulorChannelContextPopup, 1);
	popup->network_name = g_strdup (network ? network->name : NULL);
	popup->model = fabulor_channel_context_menu_model_new (channel,
		channel_session != NULL, channel_session == current_sess,
		network != NULL, network && joinlist_is_in_list (sess->server, channel),
		_("Join Channel"), _("Focus Channel"), _("Part Channel"),
		_("Cycle Channel"), _("Autojoin Channel"), plugin_model,
		menu_channel_context_dispatch, popup);
	if (!popup->model)
	{
		menu_channel_context_popup_free (popup);
		return;
	}
	popup->presenter = fabulor_context_menu_presenter_gtk4_new (
		fabulor_channel_context_menu_model_get_menu (popup->model),
		fabulor_channel_context_menu_model_get_actions (popup->model),
		plugin_actions);
	if (!popup->presenter)
	{
		menu_channel_context_popup_free (popup);
		return;
	}
	g_object_set_data_full (G_OBJECT (origin), FABULOR_CHANNEL_CONTEXT_POPUP,
		popup, menu_channel_context_popup_free);
	fabulor_context_menu_presenter_gtk4_popup_at (popup->presenter, origin, x, y);
}
#endif

void
menu_chanmenu_at (session *sess, GtkWidget *origin, gdouble x, gdouble y,
	GdkModifierType state, char *chan)
{
#if GTK_MAJOR_VERSION >= 4
	(void)state;
	menu_chanmenu_gtk4 (sess, origin, x, y, chan);
#else
	GtkWidget *menu;
	int is_joined = FALSE;
	session * chan_session;

	chan_session = find_channel (sess->server, chan);

	if (chan_session)
		is_joined = TRUE;

	g_free (str_copy);
	str_copy = g_strdup (chan);

	menu = menu_new ();

	menu_quick_item (0, chan, menu, XCMENU_SHADED, str_copy, 0);
	menu_quick_item (0, 0, menu, XCMENU_SHADED, str_copy, 0);

	if (!is_joined)
		menu_quick_item_with_callback (menu_chan_join, _("Join Channel"), menu,
												 str_copy);
	else
	{
		if (chan_session != current_sess)
			menu_quick_item_with_callback (menu_chan_focus, _("Focus Channel"), menu,
													 str_copy);
		menu_quick_item_with_callback (menu_chan_part, _("Part Channel"), menu,
												 str_copy);
		menu_quick_item_with_callback (menu_chan_cycle, _("Cycle Channel"), menu,
												 str_copy);
	}

	menu_addfavoritemenu (sess->server, menu, str_copy, FALSE);

	menu_add_plugin_items (menu, "\x5$CHAN", str_copy);
	menu_popup_at (menu, origin, x, y, state, 3, NULL);
#endif
}

static void
menu_delfav_cb (GtkWidget *item, server *serv)
{
	servlist_autojoinedit (serv->network, str_copy, FALSE);
}

static void
menu_addfav_cb (GtkWidget *item, server *serv)
{
	servlist_autojoinedit (serv->network, str_copy, TRUE);
}

void
menu_addfavoritemenu (server *serv, GtkWidget *menu, char *channel, gboolean istree)
{
	char *str;
	
	if (!serv->network)
		return;

	if (channel != str_copy)
	{
		g_free (str_copy);
		str_copy = g_strdup (channel);
	}
	
	if (istree)
		str = _("_Autojoin");
	else
		str = _("Autojoin Channel");

	if (joinlist_is_in_list (serv, channel))
	{
		menu_toggle_item (str, menu, menu_delfav_cb, serv, TRUE);
	}
	else
	{
		menu_toggle_item (str, menu, menu_addfav_cb, serv, FALSE);
	}
}

static void
menu_delautoconn_cb (GtkWidget *item, server *serv)
{
	((ircnet*)serv->network)->flags &= ~FLAG_AUTO_CONNECT;
	servlist_save ();
}

static void
menu_addautoconn_cb (GtkWidget *item, server *serv)
{
	((ircnet*)serv->network)->flags |= FLAG_AUTO_CONNECT;
	servlist_save ();
}

void
menu_addconnectmenu (server *serv, GtkWidget *menu)
{
	if (!serv->network)
		return;

	if (((ircnet*)serv->network)->flags & FLAG_AUTO_CONNECT)
	{
		menu_toggle_item (_("_Auto-Connect"), menu, menu_delautoconn_cb, serv, TRUE);
	}
	else
	{
		menu_toggle_item (_("_Auto-Connect"), menu, menu_addautoconn_cb, serv, FALSE);
	}
}

static void
menu_open_server_list (GtkWidget *wid, gpointer none)
{
	fe_serverlist_open (current_sess);
}

static void
menu_settings (GtkWidget * wid, gpointer none)
{
	extern void setup_open (void);
	setup_open ();
}

static void
menu_usermenu (void)
{
	char buf[128];
	g_snprintf(buf, sizeof(buf), _("User menu - %s"), _(DISPLAY_NAME));
	editlist_gui_open (NULL, NULL, usermenu_list, buf, "usermenu", "usermenu.conf", 0);
}

static void
usermenu_create (GtkWidget *menu)
{
	menu_create (menu, usermenu_list, "", FALSE);
	menu_quick_item (0, 0, menu, XCMENU_SHADED, 0, 0);	/* sep */
	menu_quick_item_with_callback (menu_usermenu, _("Edit This Menu" ELLIPSIS), menu, 0);
}

static void
usermenu_destroy (GtkWidget * menu)
{
	GList *items;
	GList *next;
	GList *iter;

	items = gtk_container_get_children (GTK_CONTAINER (menu));
	iter = items;
	while (iter)
	{
		next = iter->next;
		gtk_widget_destroy (iter->data);
		iter = next;
	}
	g_list_free (items);
}

void
usermenu_update (void)
{
	int done_main = FALSE;
	GSList *list = sess_list;
	session *sess;
#if GTK_MAJOR_VERSION < 4
	GtkWidget *menu;
#endif

	while (list)
	{
		sess = list->data;
#if GTK_MAJOR_VERSION < 4
		menu = sess->gui->menu_item[MENU_ID_USERMENU];
#endif
		if (sess->gui->is_tab)
		{
			if (!done_main)
			{
#if GTK_MAJOR_VERSION < 4
				if (menu)
				{
					usermenu_destroy (menu);
					usermenu_create (menu);
				}
#endif
				menu_usermenu_model_refresh (sess->gui->menu);
#if GTK_MAJOR_VERSION >= 4
				menu_main_composed_model_refresh (sess->gui->menu);
#endif
				done_main = TRUE;
			}
		} else
		{
#if GTK_MAJOR_VERSION < 4
			if (menu)
			{
				usermenu_destroy (menu);
				usermenu_create (menu);
			}
#endif
			menu_usermenu_model_refresh (sess->gui->menu);
#if GTK_MAJOR_VERSION >= 4
			menu_main_composed_model_refresh (sess->gui->menu);
#endif
		}
		list = list->next;
	}
}

static void
menu_newserver_window (GtkWidget * wid, gpointer none)
{
	int old = prefs.hex_gui_tab_chans;

	prefs.hex_gui_tab_chans = 0;
	new_ircwindow (NULL, NULL, SESS_SERVER, 0);
	prefs.hex_gui_tab_chans = old;
}

static void
menu_newchannel_window (GtkWidget * wid, gpointer none)
{
	int old = prefs.hex_gui_tab_chans;

	prefs.hex_gui_tab_chans = 0;
	new_ircwindow (current_sess->server, NULL, SESS_CHANNEL, 0);
	prefs.hex_gui_tab_chans = old;
}

static void
menu_newserver_tab (GtkWidget * wid, gpointer none)
{
	int old = prefs.hex_gui_tab_chans;
	int oldf = prefs.hex_gui_tab_newtofront;

	prefs.hex_gui_tab_chans = 1;
	/* force focus if setting is "only requested tabs" */
	if (prefs.hex_gui_tab_newtofront == 2)
		prefs.hex_gui_tab_newtofront = 1;
	new_ircwindow (NULL, NULL, SESS_SERVER, 0);
	prefs.hex_gui_tab_chans = old;
	prefs.hex_gui_tab_newtofront = oldf;
}

static void
menu_newchannel_tab (GtkWidget * wid, gpointer none)
{
	int old = prefs.hex_gui_tab_chans;

	prefs.hex_gui_tab_chans = 1;
	new_ircwindow (current_sess->server, NULL, SESS_CHANNEL, 0);
	prefs.hex_gui_tab_chans = old;
}

static void
menu_rawlog (GtkWidget * wid, gpointer none)
{
	open_rawlog (current_sess->server);
}

static void
menu_detach (GtkWidget * wid, gpointer none)
{
	mg_detach (current_sess, 0);
}

static void
menu_close (GtkWidget * wid, gpointer none)
{
	mg_close_sess (current_sess);
}

static void
menu_quit (GtkWidget * wid, gpointer none)
{
	mg_open_quit_dialog (FALSE);
}

static void
menu_search (void)
{
	mg_search_toggle (current_sess);
}

static void
menu_search_next (GtkWidget *wid)
{
	mg_search_handle_next(wid, current_sess);
}

static void
menu_search_prev (GtkWidget *wid)
{
	mg_search_handle_previous(wid, current_sess);
}

static void
menu_resetmarker (GtkWidget * wid, gpointer none)
{
	gtk_xtext_reset_marker_pos (GTK_XTEXT (current_sess->gui->xtext));
}

static void
menu_movetomarker (GtkWidget *wid, gpointer none)
{
	marker_reset_reason reason;
	char *str;

	if (!prefs.hex_text_show_marker)
		PrintText (current_sess, _("Marker line disabled."));
	else
	{
		reason = gtk_xtext_moveto_marker_pos (GTK_XTEXT (current_sess->gui->xtext));
		switch (reason) {
		case MARKER_WAS_NEVER_SET:
			str = _("Marker line never set."); break;
		case MARKER_IS_SET:
			str = ""; break;
		case MARKER_RESET_MANUALLY:
			str = _("Marker line reset manually."); break;
		case MARKER_RESET_BY_KILL:
			str = _("Marker line reset because exceeded scrollback limit."); break;
		case MARKER_RESET_BY_CLEAR:
			str = _("Marker line reset by CLEAR command."); break;
		default:
			str = _("Marker line state unknown."); break;
		}
		if (str[0])
			PrintText (current_sess, str);
	}
}

static void
menu_copy_selection (GtkWidget * wid, gpointer none)
{
	gtk_xtext_copy_selection (GTK_XTEXT (current_sess->gui->xtext));
}

static void
menu_flushbuffer (GtkWidget * wid, gpointer none)
{
	fe_text_clear (current_sess, 0);
}

static void
savebuffer_req_done (session *sess, char *file)
{
	int fh;

	if (!file)
		return;

	fh = g_open (file, O_TRUNC | O_WRONLY | O_CREAT, 0600);
	if (fh != -1)
	{
		gtk_xtext_save (GTK_XTEXT (sess->gui->xtext), fh);
		close (fh);
	}
}

static void
menu_savebuffer (GtkWidget * wid, gpointer none)
{
	gtkutil_file_req (NULL, _("Select an output filename"), savebuffer_req_done,
							current_sess, NULL, NULL, FRF_WRITE);
}

static void
menu_disconnect (GtkWidget * wid, gpointer none)
{
	handle_command (current_sess, "DISCON", FALSE);
}

static void
menu_reconnect (GtkWidget * wid, gpointer none)
{
	if (current_sess->server->hostname[0])
		handle_command (current_sess, "RECONNECT", FALSE);
	else
		fe_serverlist_open (current_sess);
}

static void
menu_join_cb (GtkDialog *dialog, gint response, GtkEntry *entry)
{
	switch (response)
	{
	case GTK_RESPONSE_ACCEPT:
		menu_chan_join (NULL, (char *)gtk_entry_get_text (GTK_ENTRY (entry)));
		break;

	case GTK_RESPONSE_HELP:
		chanlist_opengui (current_sess->server, TRUE);
		break;
	}

	fabulor_gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
menu_join_entry_cb (GtkWidget *entry, GtkDialog *dialog)
{
	gtk_dialog_response (dialog, GTK_RESPONSE_ACCEPT);
}

static void
menu_join (GtkWidget * wid, gpointer none)
{
	GtkWidget *hbox, *dialog, *entry, *label;
	GtkWidget *content_area;

	dialog = gtk_dialog_new_with_buttons (_("Join Channel"),
									GTK_WINDOW (parent_window), 0,
									_("Retrieve channel list"), GTK_RESPONSE_HELP,
									_("_Cancel"), GTK_RESPONSE_REJECT,
									_("_OK"), GTK_RESPONSE_ACCEPT,
									NULL);
	theme_manager_attach_window (dialog);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	gtk_box_set_homogeneous (GTK_BOX (content_area), TRUE);
	fabulor_gtk_window_position_at_pointer (GTK_WINDOW (dialog));
	hbox = gtkutil_box_new (GTK_ORIENTATION_HORIZONTAL, TRUE, 0);

	entry = gtk_entry_new ();
	gtk_editable_set_editable (GTK_EDITABLE (entry), FALSE);	/* avoid auto-selection */
	gtk_entry_set_text (GTK_ENTRY (entry), "#");
	g_signal_connect (G_OBJECT (entry), "activate",
						 	G_CALLBACK (menu_join_entry_cb), dialog);
	label = gtk_label_new (_("Enter Channel to Join:"));
	fabulor_gtk_box_append_trailing_pair (GTK_BOX (hbox), label, entry);

	g_signal_connect (G_OBJECT (dialog), "response",
						   G_CALLBACK (menu_join_cb), entry);

	fabulor_gtk_box_append (GTK_BOX (content_area), hbox, TRUE, TRUE, 0);

	fabulor_gtk_widget_reveal_tree (dialog);

	gtk_editable_set_editable (GTK_EDITABLE (entry), TRUE);
	gtk_editable_set_position (GTK_EDITABLE (entry), 1);
}

static void
menu_away (GtkCheckMenuItem *item, gpointer none)
{
	handle_command (current_sess, gtk_check_menu_item_get_active (item) ? "away" : "back", FALSE);
}

static void
menu_away_toggle (GtkWidget *item, gpointer none)
{
	handle_command (current_sess, current_sess->server->is_away ? "back" : "away", FALSE);
}

static void
menu_chanlist (GtkWidget * wid, gpointer none)
{
	chanlist_opengui (current_sess->server, FALSE);
}

static void
menu_banlist (GtkWidget * wid, gpointer none)
{
	banlist_opengui (current_sess);
}

#ifdef USE_PLUGIN

static void
menu_loadplugin (void)
{
	plugingui_load ();
}

static void
menu_pluginlist (void)
{
	plugingui_open ();
}

#else

static void
menu_noplugin_info (void)
{
	fe_message (_(DISPLAY_NAME " has been build without plugin support."), FE_MSG_INFO);
}

#define menu_loadplugin menu_noplugin_info
#define menu_pluginlist menu_noplugin_info

#endif

#define usercommands_help  _("User Commands - Special codes:\n\n"\
                           "%c  =  current channel\n"\
									"%e  =  current network name\n"\
									"%m  =  machine info\n"\
                           "%n  =  your nick\n"\
									"%t  =  time/date\n"\
                           "%v  =  " DISPLAY_NAME " version\n"\
                           "%2  =  word 2\n"\
                           "%3  =  word 3\n"\
                           "&2  =  word 2 to the end of line\n"\
                           "&3  =  word 3 to the end of line\n\n"\
                           "eg:\n"\
                           "/cmd john hello\n\n"\
                           "%2 would be \042john\042\n"\
                           "&2 would be \042john hello\042.")

#define ulbutton_help       _("Userlist Buttons - Special codes:\n\n"\
							"%a  =  all selected nicks\n"\
							"%c  =  current channel\n"\
							"%e  =  current network name\n"\
							"%h  =  selected nick's hostname\n"\
							"%m  =  machine info\n"\
							"%n  =  your nick\n"\
							"%s  =  selected nick\n"\
							"%t  =  time/date\n"\
							"%u  =  selected users account")

#define dlgbutton_help      _("Dialog Buttons - Special codes:\n\n"\
							"%a  =  all selected nicks\n"\
							"%c  =  current channel\n"\
							"%e  =  current network name\n"\
							"%h  =  selected nick's hostname\n"\
							"%m  =  machine info\n"\
							"%n  =  your nick\n"\
							"%s  =  selected nick\n"\
							"%t  =  time/date\n"\
							"%u  =  selected users account")

#define ctcp_help          _("CTCP Replies - Special codes:\n\n"\
                           "%d  =  data (the whole ctcp)\n"\
									"%e  =  current network name\n"\
									"%m  =  machine info\n"\
                           "%s  =  nick who sent the ctcp\n"\
                           "%t  =  time/date\n"\
                           "%2  =  word 2\n"\
                           "%3  =  word 3\n"\
                           "&2  =  word 2 to the end of line\n"\
                           "&3  =  word 3 to the end of line\n\n")

#define url_help           _("URL Handlers - Special codes:\n\n"\
                           "%s  =  the URL string\n\n"\
                           "Putting a ! in front of the command\n"\
                           "indicates it should be sent to a\n"\
                           "shell instead of " DISPLAY_NAME)

static void
menu_usercommands (void)
{
	char buf[128];
	g_snprintf(buf, sizeof(buf), _("User Defined Commands - %s"), _(DISPLAY_NAME));
	editlist_gui_open (NULL, NULL, command_list, buf, "commands", "commands.conf",
							usercommands_help);
}

static void
menu_ulpopup (void)
{
	char buf[128];
	g_snprintf(buf, sizeof(buf), _("Userlist Popup menu -  %s"), _(DISPLAY_NAME));
	editlist_gui_open (NULL, NULL, popup_list, buf, "popup", "popup.conf", ulbutton_help);
}

static void
menu_rpopup (void)
{
	char buf[128];
	g_snprintf(buf, sizeof(buf), _("Replace - %s"), _(DISPLAY_NAME));
	editlist_gui_open (_("Text"), _("Replace with"), replace_list, buf, "replace", "replace.conf", 0);
}

static void
menu_urlhandlers (void)
{
	char buf[128];
	g_snprintf(buf, sizeof(buf), _("URL Handlers - %s"), _(DISPLAY_NAME));
	editlist_gui_open (NULL, NULL, urlhandler_list, buf, "urlhandlers", "urlhandlers.conf", url_help);
}

static void
menu_evtpopup (void)
{
	pevent_dialog_show ();
}

static void
menu_keypopup (void)
{
	key_dialog_show ();
}

static void
menu_ulbuttons (void)
{
	char buf[128];
	g_snprintf(buf, sizeof(buf), _("Userlist buttons - %s"), _(DISPLAY_NAME));
	editlist_gui_open (NULL, NULL, button_list, buf, "buttons", "buttons.conf", ulbutton_help);
}

static void
menu_dlgbuttons (void)
{
	char buf[128];
	g_snprintf(buf, sizeof(buf), _("Dialog buttons - %s"), _(DISPLAY_NAME));
	editlist_gui_open (NULL, NULL, dlgbutton_list, buf, "dlgbuttons", "dlgbuttons.conf",
							 dlgbutton_help);
}

static void
menu_ctcpguiopen (void)
{
	char buf[128];
	g_snprintf(buf, sizeof(buf), _("CTCP Replies - %s"), _(DISPLAY_NAME));
	editlist_gui_open (NULL, NULL, ctcp_list, buf, "ctcpreply", "ctcpreply.conf", ctcp_help);
}

static void
menu_docs (GtkWidget *wid, gpointer none)
{
	fe_open_url (FABULOR_DOCS_URL);
}

static void
menu_dcc_win (GtkWidget *wid, gpointer none)
{
	fe_dcc_open_recv_win (FALSE);
	fe_dcc_open_send_win (FALSE);
}

static void
menu_dcc_chat_win (GtkWidget *wid, gpointer none)
{
	fe_dcc_open_chat_win (FALSE);
}

void
menu_change_layout (void)
{
	if (prefs.hex_gui_tab_layout == 0)
	{
		menu_setting_foreach (NULL, MENU_ID_LAYOUT_TABS, 1);
		menu_setting_foreach (NULL, MENU_ID_LAYOUT_TREE, 0);
		mg_change_layout (0);
	} else
	{
		menu_setting_foreach (NULL, MENU_ID_LAYOUT_TABS, 0);
		menu_setting_foreach (NULL, MENU_ID_LAYOUT_TREE, 1);
		mg_change_layout (2);
	}
}

static void
menu_layout_cb (GtkWidget *item, gpointer none)
{
	prefs.hex_gui_tab_layout = 2;
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (item)))
		prefs.hex_gui_tab_layout = 0;

	menu_change_layout ();
}

static void
menu_apply_metres_cb (session *sess)
{
	mg_update_meters (sess->gui);
}

static void
menu_action_sync_selection (const char *name, const char *target)
{
	GHashTable *seen;
	GSList *list;

	seen = g_hash_table_new (g_direct_hash, g_direct_equal);
	for (list = sess_list; list; list = g_slist_next (list))
	{
		GAction *action;
		GActionGroup *group;
		GtkWidget *menu_root;
		session *sess = list->data;

		if (!sess || !sess->gui || !sess->gui->menu)
			continue;

		menu_root = sess->gui->menu;
		if (g_hash_table_contains (seen, menu_root))
			continue;
		g_hash_table_add (seen, menu_root);

		group = g_object_get_data (G_OBJECT (menu_root), FABULOR_MENU_ACTION_GROUP);
		if (!group)
			continue;
		action = g_action_map_lookup_action (G_ACTION_MAP (group), name);
		if (G_IS_SIMPLE_ACTION (action) &&
			g_variant_type_equal (g_action_get_state_type (action),
								 G_VARIANT_TYPE_STRING))
			g_simple_action_set_state (G_SIMPLE_ACTION (action),
								   g_variant_new_string (target));
	}
	g_hash_table_unref (seen);
}

static const char *
menu_metres_target (int mode)
{
	switch (mode)
	{
	case 0:
		return "off";
	case 1:
		return "graph";
	case 2:
		return "text";
	default:
		return "both";
	}
}

static void
menu_set_metres (int mode)
{
	prefs.hex_gui_lagometer = mode;
	prefs.hex_gui_throttlemeter = mode;
	menu_action_sync_selection ("network-meters", menu_metres_target (mode));
	zoitechat_reinit_timers ();
	menu_setting_foreach (menu_apply_metres_cb, -1, 0);
}

static void
menu_metres_off (GtkWidget *item, gpointer none)
{
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (item)))
		menu_set_metres (0);
}

static void
menu_metres_text (GtkWidget *item, gpointer none)
{
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (item)))
		menu_set_metres (2);
}

static void
menu_metres_graph (GtkWidget *item, gpointer none)
{
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (item)))
		menu_set_metres (1);
}

static void
menu_metres_both (GtkWidget *item, gpointer none)
{
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (item)))
		menu_set_metres (3);
}

static void
about_dialog_response (GtkDialog *dialog, int response, gpointer data)
{
	if (response == GTK_RESPONSE_HELP)
	{
		fe_open_url (FABULOR_README_URL);
		return;
	}
	if (response == GTK_RESPONSE_APPLY)
	{
		fe_open_url ("https://www.gnu.org/licenses/old-licenses/gpl-2.0.html");
		return;
	}
	gtk_widget_destroy (GTK_WIDGET(dialog));
}

static gboolean
about_dialog_openurl (GtkAboutDialog *dialog, char *uri, gpointer data)
{
	fe_open_url (uri);
	return TRUE;
}

static void
menu_about (GtkWidget *wid, gpointer sess)
{
	GtkAboutDialog *dialog = GTK_ABOUT_DIALOG (g_object_new (GTK_TYPE_ABOUT_DIALOG, "use-header-bar", FALSE, NULL));
	GtkWidget *website;
	GtkWidget *license;
	GtkWidget *close;
	GtkWidget *actions;
	GList *children;
	GList *child;
	static const gchar *empty_people[] = { NULL };
	theme_manager_attach_window (GTK_WIDGET (dialog));
	char comment[512];
	g_snprintf  (comment, sizeof(comment), ""
#ifdef WIN32
				"Portable Mode: %s\n"
				"Build Type: x%d\n"
#endif
				"OS: %s",
#ifdef WIN32
				(portable_mode () ? "Yes" : "No"),
				get_cpu_arch (),
#endif
				get_sys_str (0));

	gtk_about_dialog_set_program_name (dialog, _(DISPLAY_NAME));
	gtk_about_dialog_set_version (dialog, PACKAGE_VERSION);
	gtk_about_dialog_set_authors (dialog, empty_people);
	gtk_about_dialog_set_documenters (dialog, empty_people);
	gtk_about_dialog_set_artists (dialog, empty_people);
	gtk_about_dialog_set_translator_credits (dialog, "");
	gtk_about_dialog_set_website (dialog, NULL);
	gtk_about_dialog_set_website_label (dialog, NULL);
	gtk_about_dialog_set_license (dialog, NULL);
	gtk_about_dialog_set_wrap_license (dialog, FALSE);
	gtk_about_dialog_set_logo (dialog, pix_zoitechat);
	gtk_about_dialog_set_copyright (dialog, "\302\251 1998-2010 Peter \305\275elezn\303\275\n\302\251 2009-2014 Berke Viktor\n\302\251 2015-2025 Patrick Griffis\n\302\251 2026 deepend");
	gtk_about_dialog_set_comments (dialog, comment);
	actions = gtk_dialog_get_action_area (GTK_DIALOG (dialog));
	children = gtk_container_get_children (GTK_CONTAINER (actions));
	for (child = children; child; child = child->next)
		gtk_widget_destroy (GTK_WIDGET (child->data));
	g_list_free (children);
	website = gtk_dialog_add_button (GTK_DIALOG (dialog), "Website", GTK_RESPONSE_HELP);
	license = gtk_dialog_add_button (GTK_DIALOG (dialog), "License", GTK_RESPONSE_APPLY);
	close = gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Close"), GTK_RESPONSE_CLOSE);
	gtk_button_box_set_child_secondary (GTK_BUTTON_BOX (actions), website, TRUE);
	gtk_button_box_set_child_secondary (GTK_BUTTON_BOX (actions), license, TRUE);
	gtk_button_box_set_child_secondary (GTK_BUTTON_BOX (actions), close, FALSE);

	gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(parent_window));
	g_signal_connect (G_OBJECT(dialog), "response", G_CALLBACK(about_dialog_response), NULL);
	g_signal_connect (G_OBJECT(dialog), "activate-link", G_CALLBACK(about_dialog_openurl), NULL);
	
	gtk_widget_show_all (GTK_WIDGET(dialog));
}

static struct mymenu mymenu[] = {
	{N_("_Fabulor"), 0, 0, M_NEWMENU, MENU_ID_ZOITECHAT, 0, 1},
#define NETWORK_LIST_OFFSET (1)
	{N_("Network Li_st"), menu_open_server_list, 0, M_MENUITEM, 0, 0, 1, 0,
		"network-list", MENU_ACTION_NETWORK_LIST},
	{0, 0, 0, M_SEP, 0, 0, 0},

#define NEW_OFFSET (3)
#define NEW_ACTION_COUNT (4)
	{N_("_New"), 0, 0, M_MENUSUB, 0, 0, 1},
		{N_("Server Tab"), menu_newserver_tab, 0, M_MENUITEM, 0, 0, 1, 0,
			"new-server-tab", MENU_ACTION_NEW_SERVER_TAB},
		{N_("Channel Tab"), menu_newchannel_tab, 0, M_MENUITEM, 0, 0, 1, 0,
			"new-channel-tab", MENU_ACTION_NEW_CHANNEL_TAB},
		{N_("Server Window"), menu_newserver_window, 0, M_MENUITEM, 0, 0, 1, 0,
			"new-server-window", MENU_ACTION_NEW_SERVER_WINDOW},
		{N_("Channel Window"), menu_newchannel_window, 0, M_MENUITEM, 0, 0, 1, 0,
			"new-channel-window", MENU_ACTION_NEW_CHANNEL_WINDOW},
		{0, 0, 0, M_END, 0, 0, 0},
	{0, 0, 0, M_SEP, 0, 0, 0},

#define LOAD_PLUGIN_OFFSET (10)
	{N_("_Load Plugin or Script" ELLIPSIS), menu_loadplugin, 0, M_MENUITEM, 0, 0, 1, 0,
		"load-plugin-script", MENU_ACTION_LOAD_PLUGIN_SCRIPT},
	{0, 0, 0, M_SEP, 0, 0, 0},	/* 11 */
#define DETACH_OFFSET (12)
	{0, menu_detach, 0, M_MENUITEM, 0, 0, 1, 0,
		"detach-attach", MENU_ACTION_DETACH_ATTACH},	/* 12 */
#define CLOSE_OFFSET (13)
	{0, menu_close, 0, M_MENUITEM, 0, 0, 1, 0, "close", MENU_ACTION_CLOSE},
	{0, 0, 0, M_SEP, 0, 0, 0},
	{N_("_Quit"), menu_quit, 0, M_MENUITEM, MENU_ID_QUIT, 0, 1, 0,
		"quit", MENU_ACTION_QUIT},	/* 15 */

	{N_("_View"), 0, 0, M_NEWMENU, 0, 0, 1},
#define MENUBAR_OFFSET (17)
#define VIEW_TOGGLE_ACTION_COUNT (5)
	{N_("_Menu Bar"), menu_bar_toggle_cb, 0, M_MENUTOG, MENU_ID_MENUBAR, 0, 1, 0,
		"menu-toggle", MENU_ACTION_MENU_TOGGLE},
	{N_("_Topic Bar"), menu_topicbar_toggle, 0, M_MENUTOG, MENU_ID_TOPICBAR, 0, 1, 0,
		"topic-bar-toggle", MENU_ACTION_TOPIC_BAR_TOGGLE},
	{N_("_User List"), menu_userlist_toggle, 0, M_MENUTOG, MENU_ID_USERLIST, 0, 1, 0,
		"user-list-toggle", MENU_ACTION_USER_LIST_TOGGLE},
	{N_("U_ser List Buttons"), menu_ulbuttons_toggle, 0, M_MENUTOG, MENU_ID_ULBUTTONS, 0, 1, 0,
		"user-list-buttons-toggle", MENU_ACTION_USER_LIST_BUTTONS_TOGGLE},
	{N_("M_ode Buttons"), menu_cmbuttons_toggle, 0, M_MENUTOG, MENU_ID_MODEBUTTONS, 0, 1, 0,
		"mode-buttons-toggle", MENU_ACTION_MODE_BUTTONS_TOGGLE},
	{0, 0, 0, M_SEP, 0, 0, 0},
#define CHANNEL_SWITCHER_OFFSET (23)
#define CHANNEL_SWITCHER_ACTION_COUNT (2)
	{N_("_Channel Switcher"), 0, 0, M_MENUSUB, 0, 0, 1},	/* 23 */
#define TABS_OFFSET (24)
		{N_("_Tabs"), menu_layout_cb, 0, M_MENURADIO, MENU_ID_LAYOUT_TABS, 0, 1, 0,
			"channel-switcher", MENU_ACTION_CHANNEL_SWITCHER, "tabs"},
		{N_("T_ree"), 0, 0, M_MENURADIO, MENU_ID_LAYOUT_TREE, 0, 1, 0,
			"channel-switcher", MENU_ACTION_CHANNEL_SWITCHER, "tree"},
		{0, 0, 0, M_END, 0, 0, 0},
#define NETWORK_METERS_OFFSET (27)
#define NETWORK_METERS_ACTION_COUNT (4)
	{N_("_Network Meters"), 0, 0, M_MENUSUB, 0, 0, 1},	/* 27 */
#define METRE_OFFSET (28)
		{N_("Off"), menu_metres_off, 0, M_MENURADIO, 0, 0, 1, 0,
			"network-meters", MENU_ACTION_NETWORK_METERS, "off"},
		{N_("Graph"), menu_metres_graph, 0, M_MENURADIO, 0, 0, 1, 0,
			"network-meters", MENU_ACTION_NETWORK_METERS, "graph"},
		{N_("Text"), menu_metres_text, 0, M_MENURADIO, 0, 0, 1, 0,
			"network-meters", MENU_ACTION_NETWORK_METERS, "text"},
		{N_("Both"), menu_metres_both, 0, M_MENURADIO, 0, 0, 1, 0,
			"network-meters", MENU_ACTION_NETWORK_METERS, "both"},
		{0, 0, 0, M_END, 0, 0, 0},	/* 32 */
	{ 0, 0, 0, M_SEP, 0, 0, 0 },
#define FULLSCREEN_OFFSET (34)
	{N_ ("_Fullscreen"), menu_fullscreen_toggle, 0, M_MENUTOG, MENU_ID_FULLSCREEN, 0, 1, 0,
		"fullscreen-toggle", MENU_ACTION_FULLSCREEN_TOGGLE},

#define SERVER_OFFSET (35)
#define SERVER_ACTION_COUNT (4)
	{N_("_Server"), 0, 0, M_NEWMENU, 0, 0, 1},
	{N_("_Disconnect"), menu_disconnect, 0, M_MENUITEM, MENU_ID_DISCONNECT, 0, 1, 0,
		"disconnect", MENU_ACTION_DISCONNECT},
	{N_("_Reconnect"), menu_reconnect, 0, M_MENUITEM, MENU_ID_RECONNECT, 0, 1, 0,
		"reconnect", MENU_ACTION_RECONNECT},
	{N_("_Join a Channel" ELLIPSIS), menu_join, 0, M_MENUITEM, MENU_ID_JOIN, 0, 1, 0,
		"join-channel", MENU_ACTION_JOIN_CHANNEL},
	{N_("Channel _List"), menu_chanlist, 0, M_MENUITEM, 0, 0, 1, 0,
		"channel-list", MENU_ACTION_CHANNEL_LIST},
	{0, 0, 0, M_SEP, 0, 0, 0},
#define AWAY_OFFSET (41)
	{N_("Marked _Away"), menu_away_toggle, 0, M_MENUITEM, MENU_ID_AWAY, 0, 1, 0,
		"away-toggle", MENU_ACTION_AWAY_TOGGLE},

	{N_("_Usermenu"), 0, 0, M_NEWMENU, MENU_ID_USERMENU, 0, 1},	/* 42 */

#define SETTINGS_OFFSET (43)
#define SETTINGS_PREFERENCES_ACTION_COUNT (1)
	{N_("S_ettings"), 0, 0, M_NEWMENU, 0, 0, 1},
	{N_("_Preferences"), menu_settings, 0, M_MENUITEM, 0, 0, 1, 0,
		"preferences", MENU_ACTION_PREFERENCES},
	{0, 0, 0, M_SEP, 0, 0, 0},
#define SETTINGS_EDITOR_OFFSET (46)
#define SETTINGS_EDITOR_ACTION_COUNT (9)
	{N_("Auto Replace"), menu_rpopup, 0, M_MENUITEM, 0, 0, 1, 0,
		"auto-replace", MENU_ACTION_AUTO_REPLACE},
	{N_("CTCP Replies"), menu_ctcpguiopen, 0, M_MENUITEM, 0, 0, 1, 0,
		"ctcp-replies", MENU_ACTION_CTCP_REPLIES},
	{N_("Dialog Buttons"), menu_dlgbuttons, 0, M_MENUITEM, 0, 0, 1, 0,
		"dialog-buttons", MENU_ACTION_DIALOG_BUTTONS},
	{N_("Keyboard Shortcuts"), menu_keypopup, 0, M_MENUITEM, 0, 0, 1, 0,
		"keyboard-shortcuts", MENU_ACTION_KEYBOARD_SHORTCUTS},
	{N_("Text Events"), menu_evtpopup, 0, M_MENUITEM, 0, 0, 1, 0,
		"text-events", MENU_ACTION_TEXT_EVENTS},
	{N_("URL Handlers"), menu_urlhandlers, 0, M_MENUITEM, 0, 0, 1, 0,
		"url-handlers", MENU_ACTION_URL_HANDLERS},
	{N_("User Commands"), menu_usercommands, 0, M_MENUITEM, 0, 0, 1, 0,
		"user-commands", MENU_ACTION_USER_COMMANDS},
	{N_("User List Buttons"), menu_ulbuttons, 0, M_MENUITEM, 0, 0, 1, 0,
		"user-list-buttons", MENU_ACTION_USER_LIST_BUTTONS},
	{N_("User List Popup"), menu_ulpopup, 0, M_MENUITEM, 0, 0, 1, 0,
		"user-list-popup", MENU_ACTION_USER_LIST_POPUP},	/* 54 */

#define WINDOW_OFFSET (SETTINGS_EDITOR_OFFSET + SETTINGS_EDITOR_ACTION_COUNT)
#define WINDOW_SURFACE_OFFSET (WINDOW_OFFSET + 1)
#define WINDOW_SURFACE_ACTION_COUNT (9)
	{N_("_Window"), 0, 0, M_NEWMENU, 0, 0, 1},
	{N_("_Ban List"), menu_banlist, 0, M_MENUITEM, 0, 0, 1, 0,
		"ban-list", MENU_ACTION_BAN_LIST},
	{N_("Character Chart"), ascii_open, 0, M_MENUITEM, 0, 0, 1, 0,
		"character-chart", MENU_ACTION_CHARACTER_CHART},
	{N_("Direct Chat"), menu_dcc_chat_win, 0, M_MENUITEM, 0, 0, 1, 0,
		"direct-chat", MENU_ACTION_DIRECT_CHAT},
	{N_("File _Transfers"), menu_dcc_win, 0, M_MENUITEM, 0, 0, 1, 0,
		"file-transfers", MENU_ACTION_FILE_TRANSFERS},
	{N_("Friends List"), notify_opengui, 0, M_MENUITEM, 0, 0, 1, 0,
		"friends-list", MENU_ACTION_FRIENDS_LIST},
	{N_("Ignore List"), ignore_gui_open, 0, M_MENUITEM, 0, 0, 1, 0,
		"ignore-list", MENU_ACTION_IGNORE_LIST},
	{N_("_Plugins and Scripts"), menu_pluginlist, 0, M_MENUITEM, 0, 0, 1, 0,
		"plugins-and-scripts", MENU_ACTION_PLUGINS_AND_SCRIPTS},
	{N_("_Raw Log"), menu_rawlog, 0, M_MENUITEM, 0, 0, 1, 0,
		"raw-log", MENU_ACTION_RAW_LOG},
	{N_("_URL Grabber"), url_opengui, 0, M_MENUITEM, 0, 0, 1, 0,
		"url-grabber", MENU_ACTION_URL_GRABBER},
	{0, 0, 0, M_SEP, 0, 0, 0},
#define WINDOW_TRANSCRIPT_OFFSET (WINDOW_SURFACE_OFFSET + WINDOW_SURFACE_ACTION_COUNT + 1)
#define WINDOW_TRANSCRIPT_ACTION_COUNT (5)
	{N_("Reset Marker Line"), menu_resetmarker, 0, M_MENUITEM, 0, 0, 1, 0,
		"reset-marker", MENU_ACTION_RESET_MARKER},
	{N_("Move to Marker Line"), menu_movetomarker, 0, M_MENUITEM, 0, 0, 1, 0,
		"move-marker", MENU_ACTION_MOVE_MARKER},
	{N_("_Copy Selection"), menu_copy_selection, 0, M_MENUITEM, 0, 0, 1, 0,
		"copy-selection", MENU_ACTION_COPY_SELECTION},
	{N_("C_lear Text"), menu_flushbuffer, 0, M_MENUITEM, 0, 0, 1, 0,
		"clear-text", MENU_ACTION_CLEAR_TEXT},
	{N_("Save Text" ELLIPSIS), menu_savebuffer, 0, M_MENUITEM, 0, 0, 1, 0,
		"save-text", MENU_ACTION_SAVE_TEXT},
#define SEARCH_OFFSET (WINDOW_TRANSCRIPT_OFFSET + WINDOW_TRANSCRIPT_ACTION_COUNT)
#define SEARCH_ACTION_COUNT (3)
	{N_("Search"), 0, 0, M_MENUSUB, 0, 0, 1},
		{N_("Search Text" ELLIPSIS), menu_search, 0, M_MENUITEM, 0, 0, 1, 0,
			"search-text", MENU_ACTION_SEARCH_TEXT},
		{N_("Search Next"   ), menu_search_next, 0, M_MENUITEM, 0, 0, 1, 0,
			"search-next", MENU_ACTION_SEARCH_NEXT},
		{N_("Search Previous"   ), menu_search_prev, 0, M_MENUITEM, 0, 0, 1, 0,
			"search-previous", MENU_ACTION_SEARCH_PREVIOUS},
		{0, 0, 0, M_END, 0, 0, 0},

#define HELP_OFFSET (SEARCH_OFFSET + SEARCH_ACTION_COUNT + 2)
#define HELP_ACTION_COUNT (2)
	{N_("_Help"), 0, 0, M_NEWMENU, 0, 0, 1},
	{N_("_Contents"), menu_docs, 0, M_MENUITEM, 0, 0, 1, 0,
		"contents", MENU_ACTION_CONTENTS},
	{N_("_About"), menu_about, 0, M_MENUITEM, 0, 0, 1, 0,
		"about", MENU_ACTION_ABOUT},

	{0, 0, 0, M_END, 0, 0, 0},
};

static const struct mymenu *
menu_action_find (const char *name)
{
	guint i;

	if (!name)
		return NULL;

	for (i = 0; i < G_N_ELEMENTS (mymenu); i++)
	{
		if (mymenu[i].action_name && strcmp (name, mymenu[i].action_name) == 0)
			return &mymenu[i];
	}

	return NULL;
}

static const char *
menu_get_key_action_name (int index)
{
	if (index < 0 || index >= (int) G_N_ELEMENTS (mymenu))
		return NULL;

	return mymenu[index].action_name;
}

#if GTK_MAJOR_VERSION < 4
static void
menu_add_keybinding_accel (GtkWidget *item, GtkAccelGroup *accel_group, const char *name)
{
	guint keyval;
	GdkModifierType mod;

	if (!accel_group || !key_get_menu_accel (name, &keyval, &mod))
		return;

	if (!strcmp (name, "quit") && !prefs.hex_gui_ctrlq_quit && keyval == GDK_KEY_q && mod == STATE_CTRL)
		return;

	gtk_widget_add_accelerator (item, "activate", accel_group, keyval, mod, GTK_ACCEL_VISIBLE);
	g_object_set_data (G_OBJECT (item), "zc-key-accel-key", GUINT_TO_POINTER (keyval));
	g_object_set_data (G_OBJECT (item), "zc-key-accel-mod", GUINT_TO_POINTER (mod));
}

static void
menu_refresh_keybinding_accels (GtkWidget *widget, gpointer data)
{
	GtkAccelGroup *accel_group = data;
	const char *name;
	guint keyval;
	GdkModifierType mod;
	GtkWidget *submenu;
	GList *children, *list;

	if (GTK_IS_MENU_ITEM (widget))
	{
		keyval = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (widget), "zc-key-accel-key"));
		mod = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (widget), "zc-key-accel-mod"));
		if (keyval != 0)
		{
			gtk_widget_remove_accelerator (widget, accel_group, keyval, mod);
			g_object_set_data (G_OBJECT (widget), "zc-key-accel-key", NULL);
			g_object_set_data (G_OBJECT (widget), "zc-key-accel-mod", NULL);
		}

		name = g_object_get_data (G_OBJECT (widget), "zc-key-action");
		menu_add_keybinding_accel (widget, accel_group, name);

		submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (widget));
		if (submenu)
			menu_refresh_keybinding_accels (submenu, data);
	}

	if (GTK_IS_CONTAINER (widget))
	{
		children = gtk_container_get_children (GTK_CONTAINER (widget));
		for (list = children; list; list = g_list_next (list))
			menu_refresh_keybinding_accels (list->data, data);
		g_list_free (children);
	}
}
#endif

void
menu_update_quit_accel (void)
{
#if GTK_MAJOR_VERSION < 4
	session *sess;
	GSList *list;
	GtkAccelGroup *accel_group;

	list = sess_list;
	while (list)
	{
		sess = list->data;
		if (sess && sess->gui && sess->gui->menu)
		{
			accel_group = g_object_get_data (G_OBJECT (sess->gui->menu), "accel");
			if (accel_group)
				menu_refresh_keybinding_accels (sess->gui->menu, accel_group);
		}
		list = g_slist_next (list);
	}
#else
	/* GTK4 configurable shortcuts dispatch directly through menu_key_action(). */
#endif
}

gboolean
menu_key_action (const char *name, guint keyval, GdkModifierType state)
{
	const struct mymenu *action = menu_action_find (name);

	if (!action)
		return FALSE;

	switch (action->action_id)
	{
	case MENU_ACTION_NETWORK_LIST:
		menu_open_server_list (NULL, NULL);
		break;
	case MENU_ACTION_LOAD_PLUGIN_SCRIPT:
		menu_loadplugin ();
		break;
	case MENU_ACTION_DETACH_ATTACH:
		menu_detach (NULL, NULL);
		break;
	case MENU_ACTION_NEW_SERVER_TAB:
		menu_newserver_tab (NULL, NULL);
		break;
	case MENU_ACTION_NEW_CHANNEL_TAB:
		menu_newchannel_tab (NULL, NULL);
		break;
	case MENU_ACTION_NEW_SERVER_WINDOW:
		menu_newserver_window (NULL, NULL);
		break;
	case MENU_ACTION_NEW_CHANNEL_WINDOW:
		menu_newchannel_window (NULL, NULL);
		break;
	case MENU_ACTION_CLOSE:
		menu_close (NULL, NULL);
		break;
	case MENU_ACTION_QUIT:
		if (!prefs.hex_gui_ctrlq_quit && keyval == GDK_KEY_q && (state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK)) == STATE_CTRL)
			return FALSE;
		menu_quit (NULL, NULL);
		break;
	case MENU_ACTION_MENU_TOGGLE:
		menu_bar_toggle_cb ();
		break;
	case MENU_ACTION_TOPIC_BAR_TOGGLE:
		menu_topicbar_toggle (NULL, NULL);
		break;
	case MENU_ACTION_USER_LIST_TOGGLE:
		menu_userlist_toggle (NULL, NULL);
		break;
	case MENU_ACTION_USER_LIST_BUTTONS_TOGGLE:
		menu_ulbuttons_toggle (NULL, NULL);
		break;
	case MENU_ACTION_MODE_BUTTONS_TOGGLE:
		menu_cmbuttons_toggle (NULL, NULL);
		break;
	case MENU_ACTION_FULLSCREEN_TOGGLE:
		menu_fullscreen_toggle (NULL, NULL);
		break;
	case MENU_ACTION_DISCONNECT:
		menu_disconnect (NULL, NULL);
		break;
	case MENU_ACTION_RECONNECT:
		menu_reconnect (NULL, NULL);
		break;
	case MENU_ACTION_JOIN_CHANNEL:
		menu_join (NULL, NULL);
		break;
	case MENU_ACTION_CHANNEL_LIST:
		menu_chanlist (NULL, NULL);
		break;
	case MENU_ACTION_AWAY_TOGGLE:
		menu_away_toggle (NULL, NULL);
		break;
	case MENU_ACTION_RESET_MARKER:
		menu_resetmarker (NULL, NULL);
		break;
	case MENU_ACTION_MOVE_MARKER:
		menu_movetomarker (NULL, NULL);
		break;
	case MENU_ACTION_COPY_SELECTION:
		menu_copy_selection (NULL, NULL);
		break;
	case MENU_ACTION_SEARCH_TEXT:
		menu_search ();
		break;
	case MENU_ACTION_SEARCH_NEXT:
		menu_search_next (NULL);
		break;
	case MENU_ACTION_SEARCH_PREVIOUS:
		menu_search_prev (NULL);
		break;
	case MENU_ACTION_CONTENTS:
		menu_docs (NULL, NULL);
		break;
	case MENU_ACTION_ABOUT:
		menu_about (NULL, NULL);
		break;
	case MENU_ACTION_PREFERENCES:
		menu_settings (NULL, NULL);
		break;
	case MENU_ACTION_AUTO_REPLACE:
		menu_rpopup ();
		break;
	case MENU_ACTION_CTCP_REPLIES:
		menu_ctcpguiopen ();
		break;
	case MENU_ACTION_DIALOG_BUTTONS:
		menu_dlgbuttons ();
		break;
	case MENU_ACTION_KEYBOARD_SHORTCUTS:
		menu_keypopup ();
		break;
	case MENU_ACTION_TEXT_EVENTS:
		menu_evtpopup ();
		break;
	case MENU_ACTION_URL_HANDLERS:
		menu_urlhandlers ();
		break;
	case MENU_ACTION_USER_COMMANDS:
		menu_usercommands ();
		break;
	case MENU_ACTION_USER_LIST_BUTTONS:
		menu_ulbuttons ();
		break;
	case MENU_ACTION_USER_LIST_POPUP:
		menu_ulpopup ();
		break;
	case MENU_ACTION_BAN_LIST:
		menu_banlist (NULL, NULL);
		break;
	case MENU_ACTION_CHARACTER_CHART:
		ascii_open ();
		break;
	case MENU_ACTION_DIRECT_CHAT:
		menu_dcc_chat_win (NULL, NULL);
		break;
	case MENU_ACTION_FILE_TRANSFERS:
		menu_dcc_win (NULL, NULL);
		break;
	case MENU_ACTION_FRIENDS_LIST:
		notify_opengui ();
		break;
	case MENU_ACTION_IGNORE_LIST:
		ignore_gui_open ();
		break;
	case MENU_ACTION_PLUGINS_AND_SCRIPTS:
		menu_pluginlist ();
		break;
	case MENU_ACTION_RAW_LOG:
		menu_rawlog (NULL, NULL);
		break;
	case MENU_ACTION_URL_GRABBER:
		url_opengui ();
		break;
	case MENU_ACTION_CLEAR_TEXT:
		menu_flushbuffer (NULL, NULL);
		break;
	case MENU_ACTION_SAVE_TEXT:
		menu_savebuffer (NULL, NULL);
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

static gboolean
menu_action_is_stateless (menu_action_id id)
{
	switch (id)
	{
	case MENU_ACTION_NETWORK_LIST:
	case MENU_ACTION_LOAD_PLUGIN_SCRIPT:
	case MENU_ACTION_DETACH_ATTACH:
	case MENU_ACTION_NEW_SERVER_TAB:
	case MENU_ACTION_NEW_CHANNEL_TAB:
	case MENU_ACTION_NEW_SERVER_WINDOW:
	case MENU_ACTION_NEW_CHANNEL_WINDOW:
	case MENU_ACTION_CLOSE:
	case MENU_ACTION_QUIT:
	case MENU_ACTION_DISCONNECT:
	case MENU_ACTION_RECONNECT:
	case MENU_ACTION_JOIN_CHANNEL:
	case MENU_ACTION_CHANNEL_LIST:
	case MENU_ACTION_RESET_MARKER:
	case MENU_ACTION_MOVE_MARKER:
	case MENU_ACTION_COPY_SELECTION:
	case MENU_ACTION_SEARCH_TEXT:
	case MENU_ACTION_SEARCH_NEXT:
	case MENU_ACTION_SEARCH_PREVIOUS:
	case MENU_ACTION_CONTENTS:
	case MENU_ACTION_ABOUT:
	case MENU_ACTION_PREFERENCES:
	case MENU_ACTION_AUTO_REPLACE:
	case MENU_ACTION_CTCP_REPLIES:
	case MENU_ACTION_DIALOG_BUTTONS:
	case MENU_ACTION_KEYBOARD_SHORTCUTS:
	case MENU_ACTION_TEXT_EVENTS:
	case MENU_ACTION_URL_HANDLERS:
	case MENU_ACTION_USER_COMMANDS:
	case MENU_ACTION_USER_LIST_BUTTONS:
	case MENU_ACTION_USER_LIST_POPUP:
	case MENU_ACTION_BAN_LIST:
	case MENU_ACTION_CHARACTER_CHART:
	case MENU_ACTION_DIRECT_CHAT:
	case MENU_ACTION_FILE_TRANSFERS:
	case MENU_ACTION_FRIENDS_LIST:
	case MENU_ACTION_IGNORE_LIST:
	case MENU_ACTION_PLUGINS_AND_SCRIPTS:
	case MENU_ACTION_RAW_LOG:
	case MENU_ACTION_URL_GRABBER:
	case MENU_ACTION_CLEAR_TEXT:
	case MENU_ACTION_SAVE_TEXT:
		return TRUE;
	default:
		return FALSE;
	}
}

static gboolean
menu_action_is_view_stateful (menu_action_id id)
{
	return id == MENU_ACTION_MENU_TOGGLE ||
		   id == MENU_ACTION_TOPIC_BAR_TOGGLE ||
		   id == MENU_ACTION_USER_LIST_TOGGLE ||
		   id == MENU_ACTION_USER_LIST_BUTTONS_TOGGLE ||
		   id == MENU_ACTION_MODE_BUTTONS_TOGGLE ||
		   id == MENU_ACTION_FULLSCREEN_TOGGLE;
}

static gboolean
menu_action_is_session_stateful (menu_action_id id)
{
	return id == MENU_ACTION_AWAY_TOGGLE;
}

static gboolean
menu_action_is_selection_stateful (menu_action_id id)
{
	return id == MENU_ACTION_CHANNEL_SWITCHER ||
		   id == MENU_ACTION_NETWORK_METERS;
}

static const char *
menu_action_selection_target (menu_action_id id)
{
	if (id == MENU_ACTION_CHANNEL_SWITCHER)
		return mymenu[TABS_OFFSET].state ? "tabs" : "tree";
	if (id == MENU_ACTION_NETWORK_METERS)
		return menu_metres_target (prefs.hex_gui_lagometer);

	return NULL;
}

static void
menu_action_activate (GSimpleAction *action, GVariant *parameter,
					  gpointer user_data)
{
	const struct mymenu *definition = user_data;
	const char *target;
	GVariant *state;

	if (menu_action_is_selection_stateful (definition->action_id))
	{
		if (!parameter || !g_variant_is_of_type (parameter, G_VARIANT_TYPE_STRING))
			return;

		target = g_variant_get_string (parameter, NULL);
		if (definition->action_id == MENU_ACTION_CHANNEL_SWITCHER)
		{
			if (g_strcmp0 (target, "tabs") != 0 &&
				g_strcmp0 (target, "tree") != 0)
				return;

			g_simple_action_set_state (action, parameter);
			prefs.hex_gui_tab_layout = g_strcmp0 (target, "tabs") == 0 ? 0 : 2;
			menu_change_layout ();
		}
		else if (definition->action_id == MENU_ACTION_NETWORK_METERS)
		{
			int mode;

			if (g_strcmp0 (target, "off") == 0)
				mode = 0;
			else if (g_strcmp0 (target, "graph") == 0)
				mode = 1;
			else if (g_strcmp0 (target, "text") == 0)
				mode = 2;
			else if (g_strcmp0 (target, "both") == 0)
				mode = 3;
			else
				return;

			g_simple_action_set_state (action, parameter);
			menu_set_metres (mode);
		}
		return;
	}

	(void) parameter;

	if (menu_action_is_view_stateful (definition->action_id))
	{
		state = g_action_get_state (G_ACTION (action));
		if (state)
		{
			g_simple_action_set_state (action,
								   g_variant_new_boolean (!g_variant_get_boolean (state)));
			g_variant_unref (state);
		}
	}

	menu_key_action (definition->action_name, 0, 0);
}

static GSimpleActionGroup *
menu_action_group_new (void)
{
	GSimpleActionGroup *group;
	GSimpleAction *action;
	guint i;

	group = g_simple_action_group_new ();
	for (i = 0; i < G_N_ELEMENTS (mymenu); i++)
	{
		if (!mymenu[i].action_name)
			continue;
		if (g_action_map_lookup_action (G_ACTION_MAP (group), mymenu[i].action_name))
			continue;

		if (menu_action_is_stateless (mymenu[i].action_id))
			action = g_simple_action_new (mymenu[i].action_name, NULL);
		else if (menu_action_is_selection_stateful (mymenu[i].action_id))
			action = g_simple_action_new_stateful (mymenu[i].action_name,
											 G_VARIANT_TYPE_STRING,
											 g_variant_new_string (
												 menu_action_selection_target (mymenu[i].action_id)));
		else if (menu_action_is_view_stateful (mymenu[i].action_id) ||
				 menu_action_is_session_stateful (mymenu[i].action_id))
			action = g_simple_action_new_stateful (mymenu[i].action_name, NULL,
										   g_variant_new_boolean (mymenu[i].state));
		else
			continue;

		g_simple_action_set_enabled (action, mymenu[i].sensitive);
		g_signal_connect (action, "activate",
						  G_CALLBACK (menu_action_activate), &mymenu[i]);
		g_action_map_add_action (G_ACTION_MAP (group), G_ACTION (action));
		g_object_unref (action);
	}

	return group;
}

static char *
menu_action_detailed_name (const struct mymenu *definition)
{
	if (definition->action_target)
		return g_strdup_printf ("fabulor.%s::%s", definition->action_name,
								 definition->action_target);

	return g_strconcat ("fabulor.", definition->action_name, NULL);
}

static void
menu_action_model_append_range (GMenu *model, guint first, guint action_count)
{
	guint i;

	for (i = first; i < first + action_count; i++)
	{
		char *detailed_name = menu_action_detailed_name (&mymenu[i]);

		g_menu_append (model, _(mymenu[i].text), detailed_name);
		g_free (detailed_name);
	}
}

static GMenuModel *
menu_action_model_range_new (guint first, guint action_count)
{
	GMenu *model;

	model = g_menu_new ();
	menu_action_model_append_range (model, first, action_count);
	g_menu_freeze (model);

	return G_MENU_MODEL (model);
}

static GMenuModel *
menu_action_model_new (guint offset, guint action_count)
{
	return menu_action_model_range_new (offset + 1, action_count);
}

static GMenuModel *
menu_server_action_model_new (void)
{
	GMenu *model;
	GMenuModel *commands;
	GMenuModel *away;

	model = g_menu_new ();
	commands = menu_action_model_range_new (SERVER_OFFSET + 1,
										 SERVER_ACTION_COUNT);
	away = menu_action_model_range_new (AWAY_OFFSET, 1);
	g_menu_append_section (model, NULL, commands);
	g_menu_append_section (model, NULL, away);
	g_object_unref (commands);
	g_object_unref (away);
	g_menu_freeze (model);

	return G_MENU_MODEL (model);
}

static GMenuModel *
menu_settings_action_model_new (void)
{
	GMenu *model;
	GMenuModel *editors;
	GMenuModel *preferences;

	model = g_menu_new ();
	preferences = menu_action_model_range_new (SETTINGS_OFFSET + 1,
										SETTINGS_PREFERENCES_ACTION_COUNT);
	editors = menu_action_model_range_new (SETTINGS_EDITOR_OFFSET,
									SETTINGS_EDITOR_ACTION_COUNT);
	g_menu_append_section (model, NULL, preferences);
	g_menu_append_section (model, NULL, editors);
	g_object_unref (preferences);
	g_object_unref (editors);
	g_menu_freeze (model);

	return G_MENU_MODEL (model);
}

static GMenuModel *
menu_window_action_model_new (GMenuModel *search)
{
	GMenu *model;
	GMenu *surfaces;
	GMenu *transcript;

	model = g_menu_new ();
	surfaces = g_menu_new ();
	transcript = g_menu_new ();
	menu_action_model_append_range (surfaces, WINDOW_SURFACE_OFFSET,
									WINDOW_SURFACE_ACTION_COUNT);
	menu_action_model_append_range (transcript, WINDOW_TRANSCRIPT_OFFSET,
									WINDOW_TRANSCRIPT_ACTION_COUNT);
	g_menu_append_submenu (transcript, _(mymenu[SEARCH_OFFSET].text), search);
	g_menu_freeze (surfaces);
	g_menu_freeze (transcript);
	g_menu_append_section (model, NULL, G_MENU_MODEL (surfaces));
	g_menu_append_section (model, NULL, G_MENU_MODEL (transcript));
	g_object_unref (surfaces);
	g_object_unref (transcript);
	g_menu_freeze (model);

	return G_MENU_MODEL (model);
}

static GMenuModel *
menu_view_action_model_new (GMenuModel *channel_switcher,
							GMenuModel *network_meters)
{
	GMenu *fullscreen;
	GMenu *model;
	GMenu *selection;
	GMenu *toggles;

	model = g_menu_new ();
	toggles = g_menu_new ();
	selection = g_menu_new ();
	fullscreen = g_menu_new ();
	menu_action_model_append_range (toggles, MENUBAR_OFFSET,
									VIEW_TOGGLE_ACTION_COUNT);
	g_menu_append_submenu (selection, _(mymenu[CHANNEL_SWITCHER_OFFSET].text),
						 channel_switcher);
	g_menu_append_submenu (selection, _(mymenu[NETWORK_METERS_OFFSET].text),
						 network_meters);
	menu_action_model_append_range (fullscreen, FULLSCREEN_OFFSET, 1);
	g_menu_freeze (toggles);
	g_menu_freeze (selection);
	g_menu_freeze (fullscreen);
	g_menu_append_section (model, NULL, G_MENU_MODEL (toggles));
	g_menu_append_section (model, NULL, G_MENU_MODEL (selection));
	g_menu_append_section (model, NULL, G_MENU_MODEL (fullscreen));
	g_object_unref (toggles);
	g_object_unref (selection);
	g_object_unref (fullscreen);
	g_menu_freeze (model);

	return G_MENU_MODEL (model);
}

static GMenuModel *
menu_fabulor_action_model_new (GMenuModel *new_model)
{
	GMenu *detach;
	GMenu *load;
	GMenu *model;
	GMenu *network_list;
	GMenu *new_section;
	GMenu *quit;

	model = g_menu_new ();
	network_list = g_menu_new ();
	new_section = g_menu_new ();
	load = g_menu_new ();
	detach = g_menu_new ();
	quit = g_menu_new ();
	menu_action_model_append_range (network_list, NETWORK_LIST_OFFSET, 1);
	g_menu_append_submenu (new_section, _(mymenu[NEW_OFFSET].text), new_model);
	menu_action_model_append_range (load, LOAD_PLUGIN_OFFSET, 1);
	menu_action_model_append_range (detach, DETACH_OFFSET, 2);
	menu_action_model_append_range (quit, CLOSE_OFFSET + 2, 1);
	g_menu_freeze (network_list);
	g_menu_freeze (new_section);
	g_menu_freeze (load);
	g_menu_freeze (detach);
	g_menu_freeze (quit);
	g_menu_append_section (model, NULL, G_MENU_MODEL (network_list));
	g_menu_append_section (model, NULL, G_MENU_MODEL (new_section));
	g_menu_append_section (model, NULL, G_MENU_MODEL (load));
	g_menu_append_section (model, NULL, G_MENU_MODEL (detach));
	g_menu_append_section (model, NULL, G_MENU_MODEL (quit));
	g_object_unref (network_list);
	g_object_unref (new_section);
	g_object_unref (load);
	g_object_unref (detach);
	g_object_unref (quit);
	g_menu_freeze (model);

	return G_MENU_MODEL (model);
}

static session *
menu_usermenu_session (void)
{
	if (current_sess)
		return current_sess;
	if (sess_list)
		return sess_list->data;

	return NULL;
}

static void
menu_usermenu_command_activate (GSimpleAction *action, GVariant *parameter,
							 gpointer user_data)
{
	const char *cmd;
	session *sess;

	(void) action;
	(void) user_data;
	if (!parameter || !g_variant_is_of_type (parameter, G_VARIANT_TYPE_STRING))
		return;

	sess = menu_usermenu_session ();
	if (!sess)
		return;

	cmd = g_variant_get_string (parameter, NULL);
	nick_command_parse (sess, (char *) cmd, "", "");
}

static void
menu_usermenu_edit_activate (GSimpleAction *action, GVariant *parameter,
						  gpointer user_data)
{
	(void) action;
	(void) parameter;
	(void) user_data;
	menu_usermenu ();
}

static void
menu_usermenu_toggle_activate (GSimpleAction *action, GVariant *parameter,
							gpointer user_data)
{
	GVariant *state;
	gboolean enabled;
	char command[256];
	session *sess;

	(void) parameter;
	state = g_action_get_state (G_ACTION (action));
	if (!state)
		return;

	enabled = !g_variant_get_boolean (state);
	g_variant_unref (state);
	g_simple_action_set_state (action, g_variant_new_boolean (enabled));
	g_snprintf (command, sizeof (command), "set %s %d",
				(char *) user_data, enabled ? 1 : 0);
	sess = menu_usermenu_session ();
	if (sess)
		handle_command (sess, command, FALSE);
}

static void
menu_usermenu_toggle_data_free (gpointer data, GClosure *closure)
{
	(void) closure;
	g_free (data);
}

static void
menu_usermenu_remove_actions (GActionMap *action_map)
{
	char **actions;
	guint i;

	actions = g_action_group_list_actions (G_ACTION_GROUP (action_map));
	for (i = 0; actions[i]; i++)
	{
		if (!strcmp (actions[i], FABULOR_USER_COMMAND_ACTION) ||
			!strcmp (actions[i], FABULOR_USER_EDIT_ACTION) ||
			g_str_has_prefix (actions[i], FABULOR_USER_TOGGLE_ACTION_PREFIX))
			g_action_map_remove_action (action_map, actions[i]);
	}
	g_strfreev (actions);
}

static void
menu_usermenu_append_section (GMenu *model, GMenu **section)
{
	if (g_menu_model_get_n_items (G_MENU_MODEL (*section)) > 0)
		g_menu_append_section (model, NULL, G_MENU_MODEL (*section));
	g_object_unref (*section);
	*section = g_menu_new ();
}

static GMenuModel *
menu_usermenu_model_build (GSList **cursor, GActionMap *action_map,
						 guint *toggle_index)
{
	GMenu *model;
	GMenu *section;

	model = g_menu_new ();
	section = g_menu_new ();
	while (*cursor)
	{
		struct popup *pop = (*cursor)->data;

		if (!g_ascii_strncasecmp (pop->name, "ENDSUB", 6))
		{
			*cursor = (*cursor)->next;
			break;
		}
		if (!g_ascii_strncasecmp (pop->name, "SUB", 3))
		{
			GMenuModel *submenu;

			*cursor = (*cursor)->next;
			submenu = menu_usermenu_model_build (cursor, action_map, toggle_index);
			g_menu_append_submenu (section, pop->cmd, submenu);
			g_object_unref (submenu);
			continue;
		}
		if (!g_ascii_strncasecmp (pop->name, "SEP", 3))
		{
			menu_usermenu_append_section (model, &section);
			*cursor = (*cursor)->next;
			continue;
		}
		if (!g_ascii_strncasecmp (pop->name, "TOGGLE", 6))
		{
			GSimpleAction *action;
			char *action_name;
			char *detailed_name;

			action_name = g_strdup_printf (FABULOR_USER_TOGGLE_ACTION_PREFIX "%u",
									   (*toggle_index)++);
			action = g_simple_action_new_stateful (action_name, NULL,
										 g_variant_new_boolean (cfg_get_bool (pop->cmd)));
			g_signal_connect_data (action, "activate",
							   G_CALLBACK (menu_usermenu_toggle_activate),
							   g_strdup (pop->cmd),
							   menu_usermenu_toggle_data_free, 0);
			g_action_map_add_action (action_map, G_ACTION (action));
			g_object_unref (action);
			detailed_name = g_strconcat ("fabulor.", action_name, NULL);
			g_menu_append (section, pop->name + 7, detailed_name);
			g_free (detailed_name);
			g_free (action_name);
		}
		else
		{
			GMenuItem *item;
			char *icon;
			char *label;

			menu_extract_icon (pop->name, &label, &icon);
			item = g_menu_item_new (label, NULL);
			if (pop->cmd && pop->cmd[0])
				g_menu_item_set_action_and_target (item,
										   "fabulor." FABULOR_USER_COMMAND_ACTION,
										   "s", pop->cmd);
			if (icon)
				g_menu_item_set_attribute (item, "fabulor-icon", "s", icon);
			g_menu_append_item (section, item);
			g_object_unref (item);
			g_free (label);
			g_free (icon);
		}
		*cursor = (*cursor)->next;
	}

	menu_usermenu_append_section (model, &section);
	g_object_unref (section);
	g_menu_freeze (model);

	return G_MENU_MODEL (model);
}

static void
menu_usermenu_model_refresh (GtkWidget *menu_bar)
{
	GActionMap *action_map;
	GMenuModel *content;
	GMenu *edit;
	GMenu *model;
	GSimpleAction *action;
	GSList *cursor;
	guint toggle_index = 0;
	gint i;

	if (!menu_bar)
		return;
	action_map = g_object_get_data (G_OBJECT (menu_bar), FABULOR_MENU_ACTION_GROUP);
	if (!action_map)
		return;

	menu_usermenu_remove_actions (action_map);
	action = g_simple_action_new (FABULOR_USER_COMMAND_ACTION,
								  G_VARIANT_TYPE_STRING);
	g_signal_connect (action, "activate",
					  G_CALLBACK (menu_usermenu_command_activate), NULL);
	g_action_map_add_action (action_map, G_ACTION (action));
	g_object_unref (action);
	action = g_simple_action_new (FABULOR_USER_EDIT_ACTION, NULL);
	g_signal_connect (action, "activate",
					  G_CALLBACK (menu_usermenu_edit_activate), NULL);
	g_action_map_add_action (action_map, G_ACTION (action));
	g_object_unref (action);

	cursor = usermenu_list;
	content = menu_usermenu_model_build (&cursor, action_map, &toggle_index);
	edit = g_menu_new ();
	g_menu_append (edit, _("Edit This Menu" ELLIPSIS),
				   "fabulor." FABULOR_USER_EDIT_ACTION);
	g_menu_freeze (edit);
	model = g_menu_new ();
	for (i = 0; i < g_menu_model_get_n_items (content); i++)
	{
		GMenuItem *item = g_menu_item_new_from_model (content, i);

		g_menu_append_item (model, item);
		g_object_unref (item);
	}
	g_menu_append_section (model, NULL, G_MENU_MODEL (edit));
	g_object_unref (content);
	g_object_unref (edit);
	g_menu_freeze (model);
	g_object_set_data_full (G_OBJECT (menu_bar), FABULOR_MENU_USER_MODEL,
						 G_MENU_MODEL (model), g_object_unref);
}

static gboolean
menu_action_bind_item (GtkWidget *item, GSimpleActionGroup *group,
					   const struct mymenu *definition)
{
	char *detailed_name;

	if (!definition->action_name ||
		(!menu_action_is_stateless (definition->action_id) &&
		 !menu_action_is_view_stateful (definition->action_id) &&
		 !menu_action_is_session_stateful (definition->action_id) &&
		 !menu_action_is_selection_stateful (definition->action_id)))
		return FALSE;

	detailed_name = menu_action_detailed_name (definition);
	gtk_widget_insert_action_group (item, "fabulor", G_ACTION_GROUP (group));
	g_object_set_data (G_OBJECT (item), FABULOR_MENU_ACTION_GROUP, group);
	g_object_set_data (G_OBJECT (item), FABULOR_MENU_ACTION_NAME,
					   (gpointer) definition->action_name);
	g_object_set_data (G_OBJECT (item), FABULOR_MENU_ACTION_TARGET,
					   (gpointer) definition->action_target);
	if (menu_action_is_selection_stateful (definition->action_id))
	{
		/* GTK3 radio items keep their callback; their GtkActionable target is unreliable. */
		g_free (detailed_name);
		return FALSE;
	}

	gtk_actionable_set_action_name (GTK_ACTIONABLE (item), detailed_name);
	g_free (detailed_name);

	return TRUE;
}

static GSimpleAction *
menu_action_get_item_action (GtkWidget *item)
{
	GAction *action;
	GActionGroup *group;
	const char *name;

	if (!GTK_IS_WIDGET (item))
		return NULL;

	group = g_object_get_data (G_OBJECT (item), FABULOR_MENU_ACTION_GROUP);
	name = g_object_get_data (G_OBJECT (item), FABULOR_MENU_ACTION_NAME);
	if (!group || !name)
		return NULL;

	action = g_action_map_lookup_action (G_ACTION_MAP (group), name);
	return G_IS_SIMPLE_ACTION (action) ? G_SIMPLE_ACTION (action) : NULL;
}

static gboolean
menu_action_set_item_state (GtkWidget *item, gboolean state)
{
	GSimpleAction *action;
	const char *target;
	const GVariantType *state_type;

	action = menu_action_get_item_action (item);
	if (!action)
		return FALSE;

	state_type = g_action_get_state_type (G_ACTION (action));
	if (!state_type)
		return FALSE;
	if (g_variant_type_equal (state_type, G_VARIANT_TYPE_STRING))
	{
		target = g_object_get_data (G_OBJECT (item), FABULOR_MENU_ACTION_TARGET);
		if (state && target)
			g_simple_action_set_state (action, g_variant_new_string (target));
		return FALSE;
	}
	if (!g_variant_type_equal (state_type, G_VARIANT_TYPE_BOOLEAN))
		return FALSE;

	g_simple_action_set_state (action, g_variant_new_boolean (state));
	return TRUE;
}

static gboolean
menu_action_set_item_enabled (GtkWidget *item, gboolean enabled)
{
	GSimpleAction *action = menu_action_get_item_action (item);

	if (!action)
		return FALSE;

	g_simple_action_set_enabled (action, enabled);
	return TRUE;
}

void
menu_set_away (session_gui *gui, int away)
{
	if (menu_action_set_item_state (gui->menu_item[MENU_ID_AWAY], away))
		return;

#if GTK_MAJOR_VERSION < 4
	if (GTK_IS_CHECK_MENU_ITEM (gui->menu_item[MENU_ID_AWAY]))
	{
		GtkCheckMenuItem *item = GTK_CHECK_MENU_ITEM (gui->menu_item[MENU_ID_AWAY]);

		g_signal_handlers_block_by_func (G_OBJECT (item), menu_away, NULL);
		gtk_check_menu_item_set_active (item, away);
		g_signal_handlers_unblock_by_func (G_OBJECT (item), menu_away, NULL);
	}
#endif
}

void
menu_set_away_sensitive (session_gui *gui, int sensitive)
{
	GtkWidget *item = gui->menu_item[MENU_ID_AWAY];

	if (!menu_action_set_item_enabled (item, sensitive) && GTK_IS_WIDGET (item))
		gtk_widget_set_sensitive (item, sensitive);
}

static void
menu_set_item_sensitive (session_gui *gui, int id, int sensitive)
{
	GtkWidget *item = gui->menu_item[id];

	if (!menu_action_set_item_enabled (item, sensitive) && GTK_IS_WIDGET (item))
		gtk_widget_set_sensitive (item, sensitive);
}

void
menu_set_disconnect_sensitive (session_gui *gui, int sensitive)
{
	menu_set_item_sensitive (gui, MENU_ID_DISCONNECT, sensitive);
}

void
menu_set_join_sensitive (session_gui *gui, int sensitive)
{
	menu_set_item_sensitive (gui, MENU_ID_JOIN, sensitive);
}

void
menu_set_fullscreen (session_gui *gui, int full)
{
#if GTK_MAJOR_VERSION >= 4
	menu_action_set_item_state (gui->menu_item[MENU_ID_FULLSCREEN], full);
#else
	GtkCheckMenuItem *item = GTK_CHECK_MENU_ITEM (gui->menu_item[MENU_ID_FULLSCREEN]);

	if (menu_action_set_item_state (GTK_WIDGET (item), full))
		return;

	g_signal_handlers_block_by_func (G_OBJECT (item), menu_fullscreen_toggle, NULL);
	gtk_check_menu_item_set_active (item, full);
	g_signal_handlers_unblock_by_func (G_OBJECT (item), menu_fullscreen_toggle, NULL);
#endif
}

GtkWidget *
create_icon_menu (char *labeltext, void *stock_name, int is_stock)
{
	GtkWidget *item;
	GtkWidget *img = NULL;
	GtkWidget *box;
	GtkWidget *label_widget;

	if (is_stock)
	{
		img = gtkutil_image_new_from_stock (stock_name, GTK_ICON_SIZE_MENU);
	}
	else
	{
		img = gtk_image_new_from_pixbuf (*((GdkPixbuf **)stock_name));
	}
	item = gtk_menu_item_new ();
	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
	label_widget = gtk_label_new_with_mnemonic (labeltext);
	if (img)
		gtk_box_pack_start (GTK_BOX (box), img, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), label_widget, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (item), box);
	if (img)
		gtk_widget_show (img);
	gtk_widget_show (label_widget);
	gtk_widget_show (box);

	return item;
}

/* Override the default handler, which would make menu
   bindings not work when the menu-bar is hidden. */
static gboolean
menu_canacaccel (GtkWidget *widget, guint signal_id, gpointer user_data)
{
	return gtk_widget_is_sensitive (widget);
}

/* === STUFF FOR /MENU === */

static GtkMenuItem *
menu_find_item (GtkWidget *menu, char *name)
{
	GList *items;
	GList *items_head;
	GtkMenuItem *item;
	GtkWidget *child;
	const char *labeltext;
	GtkMenuItem *found = NULL;

	items_head = gtk_container_get_children (GTK_CONTAINER (menu));
	items = items_head;
	while (items)
	{
		item = items->data;
		child = gtk_bin_get_child (GTK_BIN (item));
		if (child)	/* separators arn't labels, skip them */
		{
			labeltext = g_object_get_data (G_OBJECT (item), "name");
			if (!labeltext)
			{
				if (GTK_IS_LABEL (child))
					labeltext = gtk_label_get_text (GTK_LABEL (child));
				else if (GTK_IS_CONTAINER (child))
				{
					GList *kids, *l;
					kids = gtk_container_get_children (GTK_CONTAINER (child));
					for (l = kids; l; l = l->next)
					{
						if (GTK_IS_LABEL (l->data))
						{
							labeltext = gtk_label_get_text (GTK_LABEL (l->data));
							break;
						}
					}
					g_list_free (kids);
				}
			}

			if (!menu_streq (labeltext, name, 1))
			{
				found = item;
				break;
			}
		} else if (name == NULL)
		{
			found = item;
			break;
		}
		items = items->next;
	}
	g_list_free (items_head);

	return found;
}

static GtkWidget *
menu_find_path (GtkWidget *menu, char *path)
{
	GtkMenuItem *item;
	char *s;
	char name[128];
	int len;

	/* grab the next part of the path */
	s = strchr (path, '/');
	len = s - path;
	if (!s)
		len = strlen (path);
	len = MIN (len, sizeof (name) - 1);
	memcpy (name, path, len);
	name[len] = 0;

	item = menu_find_item (menu, name);
	if (!item)
		return NULL;

	menu = gtk_menu_item_get_submenu (item);
	if (!menu)
		return NULL;

	path += len;
	if (*path == 0)
		return menu;

	return menu_find_path (menu, path + 1);
}

static GtkWidget *
menu_find (GtkWidget *menu, char *path, char *label)
{
	GtkWidget *item = NULL;

	if (path[0] != 0)
		menu = menu_find_path (menu, path);
	if (menu)
		item = (GtkWidget *)menu_find_item (menu, label);
	return item;
}

static void
menu_foreach_gui (menu_entry *me, void (*callback) (GtkWidget *, menu_entry *, char *))
{
#if GTK_MAJOR_VERSION >= 4
	(void) me;
	(void) callback;
#else
	GSList *list = sess_list;
	int tabdone = FALSE;
	session *sess;

	if (!me->is_main)
		return;	/* not main menu */

	while (list)
	{
		sess = list->data;
		if (!sess || !sess->gui)
		{
			list = list->next;
			continue;
		}
		/* do it only once for tab sessions, since they share a GUI */
		if (!sess->gui->is_tab || !tabdone)
		{
			callback (sess->gui->menu, me, NULL);
			if (sess->gui->is_tab)
				tabdone = TRUE;
		}
		list = list->next;
	}
#endif
}

static void
menu_update_cb (GtkWidget *menu, menu_entry *me, char *target)
{
	GtkWidget *item;

	item = menu_find (menu, me->path, me->label);
	if (item)
	{
		gtk_widget_set_sensitive (item, me->enable);
		/* must do it without triggering the callback */
		if (GTK_IS_CHECK_MENU_ITEM (item))
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), me->state);
	}
}

/* radio state changed via mouse click */
static void
menu_radio_cb (GtkCheckMenuItem *item, menu_entry *me)
{
	me->state = 0;
	if (gtk_check_menu_item_get_active (item))
		me->state = 1;

	/* update the state, incase this was changed via right-click. */
	/* This will update all other windows and menu bars */
	menu_foreach_gui (me, menu_update_cb);

	if (me->state && me->cmd)
		handle_command (current_sess, me->cmd, FALSE);
}

/* toggle state changed via mouse click */
static void
menu_toggle_cb (GtkCheckMenuItem *item, menu_entry *me)
{
	me->state = 0;
	if (gtk_check_menu_item_get_active (item))
		me->state = 1;

	/* update the state, incase this was changed via right-click. */
	/* This will update all other windows and menu bars */
	menu_foreach_gui (me, menu_update_cb);

	if (me->state)
		handle_command (current_sess, me->cmd, FALSE);
	else
		handle_command (current_sess, me->ucmd, FALSE);
}

static GtkWidget *
menu_radio_item (char *label, GtkWidget *menu, void *callback, void *userdata,
						int state, char *groupname)
{
	GtkWidget *item;
	GtkMenuItem *parent;
	GSList *grouplist = NULL;

	parent = menu_find_item (menu, groupname);
	if (parent)
		grouplist = gtk_radio_menu_item_get_group ((GtkRadioMenuItem *)parent);

	item = gtk_radio_menu_item_new_with_label (grouplist, label);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*)item, state);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	g_signal_connect (G_OBJECT (item), "activate",
							G_CALLBACK (callback), userdata);
	gtk_widget_show (item);

	return item;
}

static void
menu_reorder (GtkMenu *menu, GtkWidget *item, int pos)
{
	if (pos == 0xffff)	/* outbound.c uses this default */
		return;

	if (pos < 0)	/* position offset from end/bottom */
	{
		GList *children = gtk_container_get_children (GTK_CONTAINER (menu));
		gint length = (gint) g_list_length (children);

		g_list_free (children);
		gtk_menu_reorder_child (menu, item, (length + pos) - 1);
	}
	else
		gtk_menu_reorder_child (menu, item, pos);
}

static GtkWidget *
menu_add_radio (GtkWidget *menu, menu_entry *me)
{
	GtkWidget *item = NULL;
	char *path = me->path + me->root_offset;

	if (path[0] != 0)
		menu = menu_find_path (menu, path);
	if (menu)
	{
		item = menu_radio_item (me->label, menu, menu_radio_cb, me, me->state, me->group);
		menu_reorder (GTK_MENU (menu), item, me->pos);
	}
	return item;
}

static GtkWidget *
menu_add_toggle (GtkWidget *menu, menu_entry *me)
{
	GtkWidget *item = NULL;
	char *path = me->path + me->root_offset;

	if (path[0] != 0)
		menu = menu_find_path (menu, path);
	if (menu)
	{
		item = menu_toggle_item (me->label, menu, menu_toggle_cb, me, me->state);
		menu_reorder (GTK_MENU (menu), item, me->pos);
	}
	return item;
}

static GtkWidget *
menu_add_item (GtkWidget *menu, menu_entry *me, char *target)
{
	GtkWidget *item = NULL;
	char *path = me->path + me->root_offset;

	if (path[0] != 0)
		menu = menu_find_path (menu, path);
	if (menu)
	{
		item = menu_quick_item (me->cmd, me->label, menu, me->markup ? XCMENU_MARKUP|XCMENU_MNEMONIC : XCMENU_MNEMONIC, target, me->icon);
		menu_reorder (GTK_MENU (menu), item, me->pos);
	}
	return item;
}

static GtkWidget *
menu_add_sub (GtkWidget *menu, menu_entry *me)
{
	GtkWidget *item = NULL;
	char *path = me->path + me->root_offset;
	int pos;

	if (path[0] != 0)
		menu = menu_find_path (menu, path);
	if (menu)
	{
		pos = me->pos;
		if (pos < 0)	/* position offset from end/bottom */
		{
			GList *children = gtk_container_get_children (GTK_CONTAINER (menu));
			gint length = (gint) g_list_length (children);

			g_list_free (children);
			pos = length + pos;
		}
		menu_quick_sub (me->label, menu, &item, me->markup ? XCMENU_MARKUP|XCMENU_MNEMONIC : XCMENU_MNEMONIC, pos);
	}
	return item;
}

static void
menu_del_cb (GtkWidget *menu, menu_entry *me, char *target)
{
	GtkWidget *item = menu_find (menu, me->path + me->root_offset, me->label);
	if (item)
		gtk_widget_destroy (item);
}

static void
menu_add_cb (GtkWidget *menu, menu_entry *me, char *target)
{
	GtkWidget *item;
	GtkAccelGroup *accel_group;

	if (me->group)	/* have a group name? Must be a radio item */
		item = menu_add_radio (menu, me);
	else if (me->ucmd)	/* have unselect-cmd? Must be a toggle item */
		item = menu_add_toggle (menu, me);
	else if (me->cmd || !me->label)	/* label=NULL for separators */
		item = menu_add_item (menu, me, target);
	else
		item = menu_add_sub (menu, me);

	if (item)
	{
		gtk_widget_set_sensitive (item, me->enable);
		if (me->key)
		{
			accel_group = g_object_get_data (G_OBJECT (menu), "accel");
			if (accel_group)	/* popup menus don't have them */
				gtk_widget_add_accelerator (item, "activate", accel_group, me->key,
													 me->modifier, GTK_ACCEL_VISIBLE);
		}
	}
}

typedef struct menu_plugin_node
{
	char *label;
	menu_entry *entry;
	GPtrArray *children;
	gboolean submenu;
	gboolean separator;
} menu_plugin_node;

typedef struct
{
	char *path;
	char *label;
	char *root;
	char *target;
	gboolean is_main;
	gboolean has_target;
	GWeakRef owner;
} menu_plugin_action_data;

typedef struct
{
	GObject *owner;
	const char *action_namespace;
	const char *root;
	const char *target;
	gboolean is_main;
} menu_plugin_projection;

static void menu_plugin_models_refresh (void);
void menu_add_plugin_model (GObject *owner, const char *root, const char *target);

static void
menu_plugin_node_free (gpointer data)
{
	menu_plugin_node *node = data;

	if (!node)
		return;
	g_free (node->label);
	if (node->children)
		g_ptr_array_free (node->children, TRUE);
	g_free (node);
}

static menu_plugin_node *
menu_plugin_node_new (const char *label, gboolean submenu,
					  gboolean separator, menu_entry *entry)
{
	menu_plugin_node *node = g_new0 (menu_plugin_node, 1);

	node->label = g_strdup (label);
	node->entry = entry;
	node->submenu = submenu;
	node->separator = separator;
	if (submenu)
		node->children = g_ptr_array_new_with_free_func (menu_plugin_node_free);

	return node;
}

static menu_plugin_node *
menu_plugin_node_find_submenu (menu_plugin_node *parent, const char *label)
{
	guint i;

	for (i = 0; i < parent->children->len; i++)
	{
		menu_plugin_node *child = g_ptr_array_index (parent->children, i);

		if (child->submenu && !menu_streq (child->label, label, 1))
			return child;
	}

	return NULL;
}

static guint
menu_plugin_node_position (menu_plugin_node *parent, const menu_entry *me)
{
	gint position;
	guint length = parent->children->len;

	if (me->pos == 0xffff)
		return length;
	position = me->pos < 0 ? (gint) length + me->pos : me->pos;
	if (position < 0)
		return 0;
	if ((guint) position > length)
		return length;

	return (guint) position;
}

static menu_plugin_node *
menu_plugin_tree_path (menu_plugin_node *root, const char *path)
{
	menu_plugin_node *parent = root;
	char **parts;
	guint i;

	parts = g_strsplit (path ? path : "", "/", -1);
	for (i = 0; parts[i]; i++)
	{
		menu_plugin_node *child;

		if (!parts[i][0])
			continue;
		child = menu_plugin_node_find_submenu (parent, parts[i]);
		if (!child)
		{
			parent = NULL;
			break;
		}
		parent = child;
	}
	g_strfreev (parts);

	return parent;
}

static menu_plugin_node *
menu_plugin_tree_seed_path (menu_plugin_node *root, const char *path)
{
	menu_plugin_node *parent = root;
	char **parts;
	guint i;

	parts = g_strsplit (path, "/", -1);
	for (i = 0; parts[i]; i++)
	{
		menu_plugin_node *child;

		if (!parts[i][0])
			continue;
		child = menu_plugin_node_find_submenu (parent, parts[i]);
		if (!child)
		{
			child = menu_plugin_node_new (parts[i], TRUE, FALSE, NULL);
			g_ptr_array_add (parent->children, child);
		}
		parent = child;
	}
	g_strfreev (parts);

	return parent;
}

static void
menu_plugin_tree_add (menu_plugin_node *root, menu_entry *me, const char *path)
{
	menu_plugin_node *node;
	menu_plugin_node *parent;
	gboolean submenu;

	parent = menu_plugin_tree_path (root, path);
	if (!parent)
		return;
	submenu = me->label && !me->cmd && !me->ucmd && !me->group;
	node = menu_plugin_node_new (me->label, submenu, !me->label, me);
	g_ptr_array_insert (parent->children, menu_plugin_node_position (parent, me), node);
}

static menu_plugin_node *
menu_plugin_tree_new (void)
{
	menu_plugin_node *root;
	GSList *list;

	root = menu_plugin_node_new (NULL, TRUE, FALSE, NULL);
	menu_plugin_tree_seed_path (root, mymenu[0].text);
	menu_plugin_tree_seed_path (root, "Fabulor/New");
	menu_plugin_tree_seed_path (root, mymenu[MENUBAR_OFFSET - 1].text);
	menu_plugin_tree_seed_path (root, "View/Channel Switcher");
	menu_plugin_tree_seed_path (root, "View/Network Meters");
	menu_plugin_tree_seed_path (root, mymenu[SERVER_OFFSET].text);
	if (prefs.hex_gui_usermenu)
		menu_plugin_tree_seed_path (root, mymenu[AWAY_OFFSET + 1].text);
	menu_plugin_tree_seed_path (root, mymenu[SETTINGS_OFFSET].text);
	menu_plugin_tree_seed_path (root, mymenu[WINDOW_OFFSET].text);
	menu_plugin_tree_seed_path (root, "Window/Search");
	menu_plugin_tree_seed_path (root, mymenu[HELP_OFFSET].text);
	for (list = menu_list; list; list = list->next)
	{
		menu_entry *me = list->data;

		if (me->is_main)
			menu_plugin_tree_add (root, me, me->path);
	}

	return root;
}

static menu_plugin_node *
menu_plugin_context_tree_new (const char *root_name)
{
	menu_plugin_node *root;
	GSList *list;

	root = menu_plugin_node_new (NULL, TRUE, FALSE, NULL);
	for (list = menu_list; list; list = list->next)
	{
		menu_entry *me = list->data;

		if (!me->is_main &&
			!strncmp (me->path, root_name + 1, (guchar) root_name[0]))
			menu_plugin_tree_add (root, me, me->path + me->root_offset);
	}

	return root;
}

static gboolean
menu_plugin_node_has_content (const menu_plugin_node *node)
{
	guint i;

	if (node->entry)
		return TRUE;
	for (i = 0; node->children && i < node->children->len; i++)
	{
		if (menu_plugin_node_has_content (g_ptr_array_index (node->children, i)))
			return TRUE;
	}

	return FALSE;
}

static menu_entry *
menu_plugin_entry_find (const menu_plugin_action_data *data)
{
	GSList *list;

	for (list = menu_list; list; list = list->next)
	{
		menu_entry *me = list->data;

		if ((me->is_main != 0) == data->is_main &&
			!strcmp (me->path, data->path) &&
			!menu_streq (me->label, data->label, 1))
			return me;
	}

	return NULL;
}

static void
menu_plugin_action_data_free (gpointer data, GClosure *closure)
{
	menu_plugin_action_data *action_data = data;

	(void) closure;
	g_free (action_data->path);
	g_free (action_data->label);
	g_free (action_data->root);
	g_free (action_data->target);
	g_weak_ref_clear (&action_data->owner);
	g_free (action_data);
}

static void
menu_plugin_action_activate (GSimpleAction *action, GVariant *parameter,
						  gpointer user_data)
{
	menu_plugin_action_data *data = user_data;
	menu_entry *me;
	const char *command = NULL;
	GObject *owner = NULL;
	char *root = NULL;
	char *target = NULL;
	gboolean has_target;
	gboolean is_main;

	(void) action;
	(void) parameter;
	me = menu_plugin_entry_find (data);
	if (!me)
		return;

	has_target = data->has_target;
	is_main = data->is_main;
	if (!is_main)
	{
		owner = g_weak_ref_get (&data->owner);
		root = g_strdup (data->root);
		target = g_strdup (data->target);
	}

	if (me->group)
	{
		GSList *list;

		for (list = menu_list; list; list = list->next)
		{
			menu_entry *peer = list->data;

			if ((peer->is_main != 0) == is_main && peer->group &&
				!strcmp (peer->path, me->path) &&
				!strcmp (peer->group, me->group))
			{
				peer->state = peer == me;
				menu_foreach_gui (peer, menu_update_cb);
			}
		}
		command = me->cmd;
		if (is_main)
			menu_plugin_models_refresh ();
	}
	else if (me->ucmd)
	{
		me->state = !me->state;
		menu_foreach_gui (me, menu_update_cb);
		command = me->state ? me->cmd : me->ucmd;
		if (is_main)
			menu_plugin_models_refresh ();
	}
	else
		command = me->cmd;

	if (!is_main && owner)
		menu_add_plugin_model (owner, root, has_target ? target : NULL);

	if (command && current_sess)
	{
		if (me->group || me->ucmd)
			handle_command (current_sess, (char *) command, FALSE);
		else if (!is_main && has_target)
			nick_command_parse (current_sess, (char *) command, target, target);
		else
			userlist_button_cb (NULL, (char *) command);
	}
	else if (command && !is_main && has_target && sess_list)
	{
		nick_command_parse (sess_list->data, (char *) command, target, target);
	}

	g_clear_object (&owner);
	g_free (root);
	g_free (target);
}

static void
menu_plugin_remove_actions (GActionMap *action_map)
{
	char **actions;
	guint i;

	actions = g_action_group_list_actions (G_ACTION_GROUP (action_map));
	for (i = 0; actions[i]; i++)
	{
		if (g_str_has_prefix (actions[i], FABULOR_PLUGIN_ACTION_PREFIX))
			g_action_map_remove_action (action_map, actions[i]);
	}
	g_strfreev (actions);
}

static void
menu_plugin_item_set_metadata (GMenuItem *item, const menu_plugin_node *node,
							   const char *kind)
{
	menu_entry *me = node->entry;

	g_menu_item_set_attribute (item, "fabulor-menu-kind", "s", kind);
	if (!me)
		return;
	g_menu_item_set_attribute (item, "fabulor-menu-path", "s", me->path);
	g_menu_item_set_attribute (item, "fabulor-menu-position", "i", me->pos);
	g_menu_item_set_attribute (item, "fabulor-menu-markup", "b", me->markup != 0);
	g_menu_item_set_attribute (item, "fabulor-menu-enabled", "b", me->enable != 0);
	g_menu_item_set_attribute (item, "fabulor-menu-key", "i", me->key);
	g_menu_item_set_attribute (item, "fabulor-menu-modifier", "i", me->modifier);
	if (me->group)
		g_menu_item_set_attribute (item, "fabulor-menu-group", "s", me->group);
	if (me->icon)
		g_menu_item_set_attribute (item, "fabulor-icon", "s", me->icon);
}

static void
menu_plugin_model_append_section (GMenu *model, GMenu **section)
{
	if (g_menu_model_get_n_items (G_MENU_MODEL (*section)) > 0)
		g_menu_append_section (model, NULL, G_MENU_MODEL (*section));
	g_object_unref (*section);
	*section = g_menu_new ();
}

static GMenuModel *
menu_plugin_node_model_new (menu_plugin_node *parent, GActionMap *action_map,
							guint *action_index,
							const menu_plugin_projection *projection)
{
	GMenu *model;
	GMenu *section;
	guint i;

	model = g_menu_new ();
	section = g_menu_new ();
	for (i = 0; i < parent->children->len; i++)
	{
		menu_plugin_node *node = g_ptr_array_index (parent->children, i);

		if (!menu_plugin_node_has_content (node))
			continue;
		if (node->separator)
		{
			menu_plugin_model_append_section (model, &section);
			continue;
		}
		if (node->submenu)
		{
			GMenuItem *item;
			GMenuModel *submenu;

			submenu = menu_plugin_node_model_new (node, action_map, action_index,
											 projection);
			item = g_menu_item_new_submenu (node->label, submenu);
			menu_plugin_item_set_metadata (item, node,
									  node->entry ? "submenu" : "path");
			g_menu_append_item (section, item);
			g_object_unref (item);
			g_object_unref (submenu);
		}
		else
		{
			GMenuItem *item;
			GSimpleAction *action;
			menu_plugin_action_data *data;
			char *action_name;
			char *detailed_name;

			action_name = g_strdup_printf (FABULOR_PLUGIN_ACTION_PREFIX "%u",
									   (*action_index)++);
			if (node->entry->ucmd || node->entry->group)
				action = g_simple_action_new_stateful (action_name, NULL,
										 g_variant_new_boolean (node->entry->state != 0));
			else
				action = g_simple_action_new (action_name, NULL);
			g_simple_action_set_enabled (action, node->entry->enable != 0);
			data = g_new0 (menu_plugin_action_data, 1);
			data->path = g_strdup (node->entry->path);
			data->label = g_strdup (node->entry->label);
			data->root = g_strdup (projection->root);
			data->target = g_strdup (projection->target);
			data->is_main = projection->is_main;
			data->has_target = projection->target != NULL;
			g_weak_ref_init (&data->owner, projection->owner);
			g_signal_connect_data (action, "activate",
							   G_CALLBACK (menu_plugin_action_activate), data,
							   menu_plugin_action_data_free, 0);
			g_action_map_add_action (action_map, G_ACTION (action));
			g_object_unref (action);
			detailed_name = g_strconcat (projection->action_namespace, ".",
									  action_name, NULL);
			item = g_menu_item_new (node->label, detailed_name);
			menu_plugin_item_set_metadata (item, node,
									  node->entry->group ? "radio" :
									  node->entry->ucmd ? "toggle" : "command");
			g_menu_append_item (section, item);
			g_object_unref (item);
			g_free (detailed_name);
			g_free (action_name);
		}
	}
	menu_plugin_model_append_section (model, &section);
	g_object_unref (section);
	g_menu_freeze (model);

	return G_MENU_MODEL (model);
}

static void
menu_plugin_model_refresh (GtkWidget *menu_bar)
{
	GActionMap *action_map;
	GMenuModel *model;
	menu_plugin_node *root;
	menu_plugin_projection projection;
	guint action_index = 0;

	if (!menu_bar)
		return;
	action_map = g_object_get_data (G_OBJECT (menu_bar), FABULOR_MENU_ACTION_GROUP);
	if (!action_map)
		return;

	menu_plugin_remove_actions (action_map);
	root = menu_plugin_tree_new ();
	projection.owner = NULL;
	projection.action_namespace = "fabulor";
	projection.root = NULL;
	projection.target = NULL;
	projection.is_main = TRUE;
	model = menu_plugin_node_model_new (root, action_map, &action_index,
									 &projection);
	menu_plugin_node_free (root);
	g_object_set_data_full (G_OBJECT (menu_bar), FABULOR_MENU_PLUGIN_MODEL,
						 model, g_object_unref);
}

static gboolean
menu_plugin_context_root_valid (const char *root_name)
{
	static const char *roots[] = {
		"\x5$NICK", "\x4$URL", "\x5$CHAN", "\x4$TAB", "\x5$TRAY"
	};
	guint i;

	if (!root_name)
		return FALSE;
	for (i = 0; i < G_N_ELEMENTS (roots); i++)
	{
		if (!strcmp (root_name, roots[i]))
			return TRUE;
	}

	return FALSE;
}

void
menu_add_plugin_model (GObject *owner, const char *root_name, const char *target)
{
	GSimpleActionGroup *action_group;
	GMenuModel *model;
	menu_plugin_node *root;
	menu_plugin_projection projection;
	guint action_index = 0;

	if (!owner || !menu_plugin_context_root_valid (root_name))
		return;

	action_group = g_simple_action_group_new ();
	root = menu_plugin_context_tree_new (root_name);
	projection.owner = owner;
	projection.action_namespace = FABULOR_PLUGIN_CONTEXT_ACTION_NAMESPACE;
	projection.root = root_name;
	projection.target = target;
	projection.is_main = FALSE;
	model = menu_plugin_node_model_new (root, G_ACTION_MAP (action_group),
									 &action_index, &projection);
	menu_plugin_node_free (root);
	if (GTK_IS_WIDGET (owner))
		gtk_widget_insert_action_group (GTK_WIDGET (owner),
									FABULOR_PLUGIN_CONTEXT_ACTION_NAMESPACE,
									G_ACTION_GROUP (action_group));
	g_object_set_data_full (owner, FABULOR_MENU_CONTEXT_ACTION_GROUP,
						 action_group, g_object_unref);
	g_object_set_data_full (owner, FABULOR_MENU_CONTEXT_MODEL,
						 model, g_object_unref);
}

GMenuModel *
menu_plugin_context_model (GObject *owner)
{
	g_return_val_if_fail (G_IS_OBJECT (owner), NULL);
	return g_object_get_data (owner, FABULOR_MENU_CONTEXT_MODEL);
}

GActionGroup *
menu_plugin_context_actions (GObject *owner)
{
	g_return_val_if_fail (G_IS_OBJECT (owner), NULL);
	return g_object_get_data (owner, FABULOR_MENU_CONTEXT_ACTION_GROUP);
}

static void
menu_plugin_models_refresh (void)
{
	GHashTable *seen;
	GSList *list;

	seen = g_hash_table_new (g_direct_hash, g_direct_equal);
	for (list = sess_list; list; list = list->next)
	{
		session *sess = list->data;

		if (!sess || !sess->gui || !sess->gui->menu ||
			g_hash_table_contains (seen, sess->gui->menu))
			continue;
		g_hash_table_add (seen, sess->gui->menu);
		menu_plugin_model_refresh (sess->gui->menu);
#if GTK_MAJOR_VERSION >= 4
		menu_main_composed_model_refresh (sess->gui->menu);
#endif
	}
	g_hash_table_destroy (seen);
}

char *
fe_menu_add (menu_entry *me)
{
	char *text;

	menu_foreach_gui (me, menu_add_cb);

	if (!me->markup)
		return NULL;

	if (!pango_parse_markup (me->label, -1, 0, NULL, &text, NULL, NULL))
		return NULL;

	/* return the label with markup stripped */
	return text;
}

void
fe_menu_del (menu_entry *me)
{
	menu_foreach_gui (me, menu_del_cb);
}

void
fe_menu_update (menu_entry *me)
{
	menu_foreach_gui (me, menu_update_cb);
}

void
fe_menu_sync (void)
{
	menu_plugin_models_refresh ();
}

/* used to add custom menus to the right-click menu */

static void
menu_add_plugin_mainmenu_items (GtkWidget *menu)
{
	GSList *list;
	menu_entry *me;

	list = menu_list;	/* outbound.c */
	while (list)
	{
		me = list->data;
		if (me->is_main)
			menu_add_cb (menu, me, NULL);
		list = list->next;
	}
}

void
menu_add_plugin_items (GtkWidget *menu, char *root, char *target)
{
	GSList *list;
	menu_entry *me;

	list = menu_list;	/* outbound.c */
	while (list)
	{
		me = list->data;
		if (!me->is_main && !strncmp (me->path, root + 1, root[0]))
			menu_add_cb (menu, me, target);
		list = list->next;
	}
	menu_add_plugin_model (G_OBJECT (menu), root, target);
}

/* === END STUFF FOR /MENU === */

static GSimpleActionGroup *
menu_main_projection_prepare (GtkWidget *owner)
{
	GSimpleActionGroup *action_group;
	GMenuModel *channel_switcher_model;
	GMenuModel *fabulor_model;
	GMenuModel *help_model;
	GMenuModel *network_meters_model;
	GMenuModel *new_model;
	GMenuModel *search_model;
	GMenuModel *server_model;
	GMenuModel *settings_model;
	GMenuModel *view_model;
	GMenuModel *window_model;

	action_group = menu_action_group_new ();
	gtk_widget_insert_action_group (owner, "fabulor",
		G_ACTION_GROUP (action_group));
	g_object_set_data_full (G_OBJECT (owner), FABULOR_MENU_ACTION_GROUP,
		action_group, g_object_unref);
	menu_usermenu_model_refresh (owner);
	menu_plugin_model_refresh (owner);
	channel_switcher_model = menu_action_model_new (CHANNEL_SWITCHER_OFFSET,
		CHANNEL_SWITCHER_ACTION_COUNT);
	g_object_set_data_full (G_OBJECT (owner),
		FABULOR_MENU_CHANNEL_SWITCHER_MODEL, channel_switcher_model,
		g_object_unref);
	network_meters_model = menu_action_model_new (NETWORK_METERS_OFFSET,
		NETWORK_METERS_ACTION_COUNT);
	g_object_set_data_full (G_OBJECT (owner), FABULOR_MENU_NETWORK_METERS_MODEL,
		network_meters_model, g_object_unref);
	view_model = menu_view_action_model_new (channel_switcher_model,
		network_meters_model);
	g_object_set_data_full (G_OBJECT (owner), FABULOR_MENU_VIEW_MODEL,
		view_model, g_object_unref);
	new_model = menu_action_model_new (NEW_OFFSET, NEW_ACTION_COUNT);
	g_object_set_data_full (G_OBJECT (owner), FABULOR_MENU_NEW_MODEL,
		new_model, g_object_unref);
	fabulor_model = menu_fabulor_action_model_new (new_model);
	g_object_set_data_full (G_OBJECT (owner), FABULOR_MENU_ROOT_MODEL,
		fabulor_model, g_object_unref);
	server_model = menu_server_action_model_new ();
	g_object_set_data_full (G_OBJECT (owner), FABULOR_MENU_SERVER_MODEL,
		server_model, g_object_unref);
	settings_model = menu_settings_action_model_new ();
	g_object_set_data_full (G_OBJECT (owner), FABULOR_MENU_SETTINGS_MODEL,
		settings_model, g_object_unref);
	search_model = menu_action_model_new (SEARCH_OFFSET, SEARCH_ACTION_COUNT);
	g_object_set_data_full (G_OBJECT (owner), FABULOR_MENU_SEARCH_MODEL,
		search_model, g_object_unref);
	window_model = menu_window_action_model_new (search_model);
	g_object_set_data_full (G_OBJECT (owner), FABULOR_MENU_WINDOW_MODEL,
		window_model, g_object_unref);
	help_model = menu_action_model_new (HELP_OFFSET, HELP_ACTION_COUNT);
	g_object_set_data_full (G_OBJECT (owner), FABULOR_MENU_HELP_MODEL,
		help_model, g_object_unref);

	return action_group;
}

#if GTK_MAJOR_VERSION >= 4
static FabulorMiddleContextMenuModel *
menu_main_composed_model_new (GtkWidget *owner)
{
	FabulorMiddleContextSection sections[7];
	GMenuModel *plugin_model;
	GMenuModel *user_model;
	gsize section_count = 0;

	user_model = g_object_get_data (G_OBJECT (owner), FABULOR_MENU_USER_MODEL);
	plugin_model = g_object_get_data (G_OBJECT (owner),
		FABULOR_MENU_PLUGIN_MODEL);
	sections[section_count++] = (FabulorMiddleContextSection) {
		_(mymenu[0].text), mymenu[0].text,
		g_object_get_data (G_OBJECT (owner), FABULOR_MENU_ROOT_MODEL)
	};
	sections[section_count++] = (FabulorMiddleContextSection) {
		_(mymenu[MENUBAR_OFFSET - 1].text),
		mymenu[MENUBAR_OFFSET - 1].text,
		g_object_get_data (G_OBJECT (owner), FABULOR_MENU_VIEW_MODEL)
	};
	sections[section_count++] = (FabulorMiddleContextSection) {
		_(mymenu[SERVER_OFFSET].text), mymenu[SERVER_OFFSET].text,
		g_object_get_data (G_OBJECT (owner), FABULOR_MENU_SERVER_MODEL)
	};
	if (prefs.hex_gui_usermenu && user_model)
	{
		sections[section_count++] = (FabulorMiddleContextSection) {
			_(mymenu[AWAY_OFFSET + 1].text),
			mymenu[AWAY_OFFSET + 1].text, user_model
		};
	}
	sections[section_count++] = (FabulorMiddleContextSection) {
		_(mymenu[SETTINGS_OFFSET].text), mymenu[SETTINGS_OFFSET].text,
		g_object_get_data (G_OBJECT (owner), FABULOR_MENU_SETTINGS_MODEL)
	};
	sections[section_count++] = (FabulorMiddleContextSection) {
		_(mymenu[WINDOW_OFFSET].text), mymenu[WINDOW_OFFSET].text,
		g_object_get_data (G_OBJECT (owner), FABULOR_MENU_WINDOW_MODEL)
	};
	sections[section_count++] = (FabulorMiddleContextSection) {
		_(mymenu[HELP_OFFSET].text), mymenu[HELP_OFFSET].text,
		g_object_get_data (G_OBJECT (owner), FABULOR_MENU_HELP_MODEL)
	};

	return fabulor_middle_context_menu_model_new (sections, section_count,
		plugin_model);
}

static void
menu_main_composed_model_refresh (GtkWidget *menu_bar)
{
	FabulorMiddleContextMenuModel *composition;
	GMenu *add_ons;
	GMenu *menu;
	GMenuModel *source;
	gint i;

	if (!GTK_IS_POPOVER_MENU_BAR (menu_bar))
		return;
	composition = menu_main_composed_model_new (menu_bar);
	if (!composition)
		return;
	source = fabulor_middle_context_menu_model_get_menu (composition);
	menu = g_menu_new ();
	add_ons = g_menu_new ();
	for (i = 0; i < g_menu_model_get_n_items (source); i++)
	{
		GMenuItem *item;
		GMenuModel *submenu;

		item = g_menu_item_new_from_model (source, i);
		submenu = g_menu_model_get_item_link (source, i, G_MENU_LINK_SUBMENU);
		if (submenu)
			g_menu_append_item (menu, item);
		else
			g_menu_append_item (add_ons, item);
		g_clear_object (&submenu);
		g_object_unref (item);
	}
	if (g_menu_model_get_n_items (G_MENU_MODEL (add_ons)) > 0)
		g_menu_append_submenu (menu, _("Add-ons"), G_MENU_MODEL (add_ons));
	g_object_unref (add_ons);
	g_menu_freeze (menu);
	fabulor_middle_context_menu_model_free (composition);
	gtk_popover_menu_bar_set_menu_model (GTK_POPOVER_MENU_BAR (menu_bar),
		G_MENU_MODEL (menu));
	g_object_set_data_full (G_OBJECT (menu_bar), FABULOR_MENU_COMPOSED_MODEL,
		G_MENU_MODEL (menu), g_object_unref);
}

static void
menu_main_action_proxies_create (GtkWidget *menu_bar,
	GSimpleActionGroup *action_group, GtkWidget **menu_widgets)
{
	GPtrArray *proxies;
	guint i;

	if (!menu_widgets)
		return;
	proxies = g_ptr_array_new_with_free_func (g_object_unref);
	for (i = 0; i < G_N_ELEMENTS (mymenu); i++)
	{
		GtkWidget *proxy;

		if (!mymenu[i].id || !mymenu[i].action_name)
			continue;
		proxy = g_object_ref_sink (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
		g_object_set_data (G_OBJECT (proxy), FABULOR_MENU_ACTION_GROUP,
			action_group);
		g_object_set_data (G_OBJECT (proxy), FABULOR_MENU_ACTION_NAME,
			(gpointer) mymenu[i].action_name);
		g_object_set_data (G_OBJECT (proxy), FABULOR_MENU_ACTION_TARGET,
			(gpointer) mymenu[i].action_target);
		menu_widgets[mymenu[i].id] = proxy;
		g_ptr_array_add (proxies, proxy);
	}
	g_object_set_data_full (G_OBJECT (menu_bar), FABULOR_MENU_ACTION_PROXIES,
		proxies, (GDestroyNotify) g_ptr_array_unref);
}
#endif

static void
menu_main_model_state_prepare (int away, int away_sensitive,
	int disconnect_sensitive, int join_sensitive, int toplevel)
{
	mymenu[MENUBAR_OFFSET].state = !prefs.hex_gui_hide_menu;
	mymenu[MENUBAR_OFFSET+1].state = prefs.hex_gui_topicbar;
	mymenu[MENUBAR_OFFSET+2].state = !prefs.hex_gui_ulist_hide;
	mymenu[MENUBAR_OFFSET+3].state = prefs.hex_gui_ulist_buttons;
	mymenu[MENUBAR_OFFSET+4].state = prefs.hex_gui_mode_buttons;
	mymenu[FULLSCREEN_OFFSET].state = prefs.hex_gui_win_fullscreen;

	mymenu[AWAY_OFFSET].state = away;
	mymenu[AWAY_OFFSET].sensitive = away_sensitive;
	mymenu[SERVER_OFFSET+1].sensitive = disconnect_sensitive;
	mymenu[SERVER_OFFSET+3].sensitive = join_sensitive;

	if (prefs.hex_gui_tab_layout == 0)
	{
		mymenu[TABS_OFFSET].state = 1;
		mymenu[TABS_OFFSET+1].state = 0;
	}
	else
	{
		mymenu[TABS_OFFSET].state = 0;
		mymenu[TABS_OFFSET+1].state = 1;
	}

	mymenu[METRE_OFFSET].state = 0;
	mymenu[METRE_OFFSET+1].state = 0;
	mymenu[METRE_OFFSET+2].state = 0;
	mymenu[METRE_OFFSET+3].state = 0;
	switch (prefs.hex_gui_lagometer)
	{
	case 0:
		mymenu[METRE_OFFSET].state = 1;
		break;
	case 1:
		mymenu[METRE_OFFSET+1].state = 1;
		break;
	case 2:
		mymenu[METRE_OFFSET+2].state = 1;
		break;
	default:
		mymenu[METRE_OFFSET+3].state = 1;
	}

	mymenu[DETACH_OFFSET].text = toplevel ? N_("_Attach") : N_("_Detach");
	mymenu[CLOSE_OFFSET].text = N_("_Close");
}

GtkWidget *
menu_create_main (
#if GTK_MAJOR_VERSION < 4
				  GtkAccelGroup *accel_group,
#endif
				  int bar, int away, int away_sensitive,
					 int disconnect_sensitive, int join_sensitive, int toplevel,
					 GtkWidget **menu_widgets)
{
	GSimpleActionGroup *action_group;
	GtkWidget *menu_bar;
#if GTK_MAJOR_VERSION < 4
	int i = 0;
	gboolean action_bound;
	GtkWidget *item;
	GtkWidget *menu = 0;
	GtkWidget *menu_item = 0;
	GtkWidget *usermenu = 0;
	GtkWidget *submenu = 0;
	int close_mask = STATE_CTRL;
	int away_mask = STATE_ALT;
	char *key_theme = NULL;
	GtkSettings *settings;
	GSList *group = NULL;
#endif

	menu_main_model_state_prepare (away, away_sensitive,
		disconnect_sensitive, join_sensitive, toplevel);

#if GTK_MAJOR_VERSION >= 4
	(void) bar;
	menu_bar = gtk_popover_menu_bar_new_from_model (NULL);
#else
	if (bar)
	{
		menu_bar = gtk_menu_bar_new ();
		gtk_style_context_add_class (gtk_widget_get_style_context (menu_bar), GTK_STYLE_CLASS_MENUBAR);
	}
	else
		menu_bar = menu_new ();

	/* /MENU needs to know this later */
	g_object_set_data (G_OBJECT (menu_bar), "accel", accel_group);

	g_signal_connect (G_OBJECT (menu_bar), "can-activate-accel",
							G_CALLBACK (menu_canacaccel), 0);

	/* change Close binding to ctrl-shift-w when using emacs keys */
	settings = gtk_widget_get_settings (menu_bar);
	if (settings)
	{
		g_object_get (settings, "gtk-key-theme-name", &key_theme, NULL);
		if (key_theme)
		{
			if (!g_ascii_strcasecmp (key_theme, "Emacs"))
			{
				close_mask = STATE_SHIFT | STATE_CTRL;
				mymenu[SEARCH_OFFSET].key = 0;
			}
			g_free (key_theme);
		}
	}

	/* Away binding to ctrl-alt-a if the _Help menu conflicts (FR/PT/IT) */
	{
		char *help = _("_Help");
		char *under = strchr (help, '_');
		if (under && (under[1] == 'a' || under[1] == 'A'))
			away_mask = STATE_ALT | STATE_CTRL;
	}
#endif

	action_group = menu_main_projection_prepare (menu_bar);
#if GTK_MAJOR_VERSION >= 4
	menu_main_composed_model_refresh (menu_bar);
	menu_main_action_proxies_create (menu_bar, action_group, menu_widgets);
	return menu_bar;
#else

	while (1)
	{
		item = NULL;
		if (mymenu[i].id == MENU_ID_USERMENU && !prefs.hex_gui_usermenu)
		{
			i++;
			continue;
		}

		switch (mymenu[i].type)
		{
		case M_NEWMENU:
			if (menu)
				gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu);
			item = menu = menu_new ();
			if (mymenu[i].id == MENU_ID_USERMENU)
				usermenu = menu;
			menu_item = gtk_menu_item_new_with_mnemonic (_(mymenu[i].text));
			/* record the English name for /menu */
			g_object_set_data (G_OBJECT (menu_item), "name", mymenu[i].text);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), menu_item);
			gtk_widget_show (menu_item);
			break;

		case M_MENUPIX:
			item = create_icon_menu (_(mymenu[i].text), mymenu[i].image, FALSE);
			goto normalitem;

		case M_MENUSTOCK:
			item = create_icon_menu (_(mymenu[i].text), mymenu[i].image, TRUE);
			goto normalitem;

		case M_MENUITEM:
			item = gtk_menu_item_new_with_mnemonic (_(mymenu[i].text));
normalitem:
			action_bound = menu_action_bind_item (item, action_group, &mymenu[i]);
			g_object_set_data (G_OBJECT (item), "zc-key-action", (gpointer) menu_get_key_action_name (i));
			menu_add_keybinding_accel (item, accel_group, menu_get_key_action_name (i));
			if (mymenu[i].key != 0 && !(mymenu[i].id == MENU_ID_QUIT && !prefs.hex_gui_ctrlq_quit))
				gtk_widget_add_accelerator (item, "activate", accel_group,
									mymenu[i].key,
									mymenu[i].key == GDK_KEY_F1 ? 0 :
									mymenu[i].key == GDK_KEY_w ? close_mask :
									mymenu[i].id == MENU_ID_AWAY ? away_mask :
									(g_ascii_isupper (mymenu[i].key)) ?
										STATE_SHIFT | STATE_CTRL :
										STATE_CTRL,
									GTK_ACCEL_VISIBLE);
			if (mymenu[i].id == MENU_ID_QUIT)
			{
				g_object_set_data (G_OBJECT (item), "zc-quit-accel-group", accel_group);
				g_object_set_data (G_OBJECT (item), "zc-ctrlq-enabled", GINT_TO_POINTER (prefs.hex_gui_ctrlq_quit));
			}
			if (mymenu[i].callback && !action_bound)
				g_signal_connect (G_OBJECT (item), "activate",
										G_CALLBACK (mymenu[i].callback), 0);
			if (submenu)
				gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
			else
				gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
			gtk_widget_show (item);
			break;

		case M_MENUTOG:
			item = gtk_check_menu_item_new_with_mnemonic (_(mymenu[i].text));
togitem:
			action_bound = menu_action_bind_item (item, action_group, &mymenu[i]);
			g_object_set_data (G_OBJECT (item), "zc-key-action", (gpointer) menu_get_key_action_name (i));
			menu_add_keybinding_accel (item, accel_group, menu_get_key_action_name (i));
			/* must avoid callback for Radio buttons */
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), mymenu[i].state);
			/*gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
													 mymenu[i].state);*/
			if (mymenu[i].key != 0)
				gtk_widget_add_accelerator (item, "activate", accel_group,
											mymenu[i].key,
											mymenu[i].id == MENU_ID_FULLSCREEN ? 0 :
											mymenu[i].id == MENU_ID_AWAY ? away_mask :
											STATE_CTRL, GTK_ACCEL_VISIBLE);
			if (mymenu[i].callback && !action_bound)
				g_signal_connect (G_OBJECT (item), "toggled",
									G_CALLBACK (mymenu[i].callback), NULL);

			if (submenu)
				gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
			else
				gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
			gtk_widget_show (item);
			gtk_widget_set_sensitive (item, mymenu[i].sensitive);
			break;

		case M_MENURADIO:
			item = gtk_radio_menu_item_new_with_mnemonic (group, _(mymenu[i].text));
			group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (item));
			goto togitem;

		case M_SEP:
			item = gtk_separator_menu_item_new ();
			if (submenu)
				gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
			else
				gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
			gtk_widget_show (item);
			break;

		case M_MENUSUB:
			group = NULL;
			submenu = menu_new ();
			item = create_icon_menu (_(mymenu[i].text), mymenu[i].image, TRUE);
			/* record the English name for /menu */
			g_object_set_data (G_OBJECT (item), "name", mymenu[i].text);
			gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), submenu);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
			gtk_widget_show (item);
			break;

		/*case M_END:*/ default:
			if (!submenu)
			{
				if (menu)
				{
					gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu);
					menu_add_plugin_mainmenu_items (menu_bar);
				}
				if (usermenu)
					usermenu_create (usermenu);
				return (menu_bar);
			}
			submenu = NULL;
		}

		/* record this GtkWidget * so it's state might be changed later */
		if (mymenu[i].id != 0 && menu_widgets)
			/* this ends up in sess->gui->menu_item[MENU_ID_XXX] */
			menu_widgets[mymenu[i].id] = item;

		i++;
	}
#endif
}

#if GTK_MAJOR_VERSION >= 4
#define FABULOR_MIDDLE_CONTEXT_POPUP "fabulor-middle-context-popup"

typedef struct
{
	FabulorMiddleContextMenuModel *model;
	FabulorContextMenuPresenterGtk4 *presenter;
	GtkWidget *projection_owner;
} FabulorMiddleContextPopup;

static void
menu_middle_context_popup_free (gpointer data)
{
	FabulorMiddleContextPopup *popup = data;

	if (!popup)
		return;
	fabulor_context_menu_presenter_gtk4_free (popup->presenter);
	fabulor_middle_context_menu_model_free (popup->model);
	g_clear_object (&popup->projection_owner);
	g_free (popup);
}

static void
menu_middlemenu_gtk4 (session *sess, GtkWidget *origin, gdouble x, gdouble y)
{
	FabulorMiddleContextPopup *popup;
	GSimpleActionGroup *action_group;

	g_return_if_fail (sess != NULL);
	g_return_if_fail (sess->server != NULL);
	g_return_if_fail (sess->gui != NULL);
	g_return_if_fail (GTK_IS_WIDGET (origin));

	menu_nick_context_deactivate ();
	menu_main_model_state_prepare (sess->server->is_away,
		sess->server->connected,
		sess->server->connected || sess->server->recondelay_tag,
		sess->server->end_of_motd, !sess->gui->is_tab);

	popup = g_new0 (FabulorMiddleContextPopup, 1);
	popup->projection_owner = g_object_ref_sink (
		gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
	action_group = menu_main_projection_prepare (popup->projection_owner);
	popup->model = menu_main_composed_model_new (popup->projection_owner);
	if (!popup->model)
	{
		menu_middle_context_popup_free (popup);
		return;
	}

	popup->presenter =
		fabulor_context_menu_presenter_gtk4_new_with_namespaces (
			fabulor_middle_context_menu_model_get_menu (popup->model),
			"fabulor", G_ACTION_GROUP (action_group), NULL, NULL);
	if (!popup->presenter)
	{
		menu_middle_context_popup_free (popup);
		return;
	}

	g_object_set_data_full (G_OBJECT (origin), FABULOR_MIDDLE_CONTEXT_POPUP,
		popup, menu_middle_context_popup_free);
	fabulor_context_menu_presenter_gtk4_popup_at (popup->presenter, origin,
		x, y);
}
#endif
