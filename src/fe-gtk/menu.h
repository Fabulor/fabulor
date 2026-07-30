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

#ifndef FABULOR_MENU_H
#define FABULOR_MENU_H

#include "menu-action-namespaces.h"

GtkWidget *menu_create_main (
								 int bar, int away,
								 int away_sensitive, int disconnect_sensitive,
								 int join_sensitive, int toplevel,
								 GtkWidget **menu_widgets);
void menu_urlmenu_at (GtkWidget *origin, gdouble x, gdouble y,
	GdkModifierType state, char *url);
void menu_chanmenu_at (session *sess, GtkWidget *origin, gdouble x,
	gdouble y, GdkModifierType state, char *chan);
void menu_nickmenu_at (session *sess, GtkWidget *origin, gdouble x,
	gdouble y, GdkModifierType state, char *nick, int num_sel);
void menu_middlemenu_at (session *sess, GtkWidget *origin, gdouble x,
	gdouble y, GdkModifierType state);
void userlist_button_cb (GtkWidget * button, char *cmd);
void nick_command_parse (session *sess, char *cmd, char *nick, char *allnick);
void usermenu_update (void);
void menu_bar_toggle (void);
void menu_add_plugin_model (GObject *owner, const char *root, const char *target);
GMenuModel *menu_plugin_context_model (GObject *owner);
GActionGroup *menu_plugin_context_actions (GObject *owner);
void menu_change_layout (void);
void menu_update_quit_accel (void);
gboolean menu_key_action (const char *name, guint keyval, GdkModifierType state);
void menu_parse_icon_label (const char *name, char **label, char **icon);

void menu_set_away (session_gui *gui, int away);
void menu_set_away_sensitive (session_gui *gui, int sensitive);
void menu_set_disconnect_sensitive (session_gui *gui, int sensitive);
void menu_set_join_sensitive (session_gui *gui, int sensitive);
void menu_set_fullscreen (session_gui *gui, int fullscreen);

/* for menu_quick functions */
#define XCMENU_DOLIST 1
#define XCMENU_SHADED 1
#define XCMENU_MARKUP 2
#define XCMENU_MNEMONIC 4

/* menu items we keep a GtkWidget* for (to change their state) */
#define MENU_ID_AWAY 1
#define MENU_ID_MENUBAR 2
#define MENU_ID_TOPICBAR 3
#define MENU_ID_USERLIST 4
#define MENU_ID_ULBUTTONS 5
#define MENU_ID_MODEBUTTONS 6
#define MENU_ID_LAYOUT_TABS 7
#define MENU_ID_LAYOUT_TREE 8
#define MENU_ID_DISCONNECT 9
#define MENU_ID_RECONNECT 10
#define MENU_ID_JOIN 11
#define MENU_ID_USERMENU 12
#define MENU_ID_FULLSCREEN 13
#define MENU_ID_FABULOR 14
#define MENU_ID_QUIT 15

#if (MENU_ID_NUM < MENU_ID_QUIT)
#error MENU_ID_NUM is set wrong
#endif

#endif
