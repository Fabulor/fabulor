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
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#include "fe-gtk.h"

#include "../common/zoitechat.h"
#include "../common/notify.h"
#include "../common/cfgfiles.h"
#include "../common/fe.h"
#include "../common/server.h"
#include "../common/util.h"
#include "../common/userlist.h"
#include "../common/outbound.h"
#include "gtkutil.h"
#include "gtk-compat.h"
#include "maingui.h"
#include "notify-list.h"
#include "theme/theme-gtk.h"
#include "notifygui.h"
#include "theme/theme-access.h"
#include "theme/theme-manager.h"

#define ICON_NOTIFY_NEW "document-new"
#define ICON_NOTIFY_DELETE "edit-delete"
#define LABEL_NOTIFY_CANCEL _("_Cancel")
#define LABEL_NOTIFY_OK _("_OK")


static GtkWidget *notify_window = 0;
static GtkWidget *notify_button_opendialog;
static GtkWidget *notify_button_remove;
static FabulorNotifyList *notify_gui_list;


static void
notify_closegui (void)
{
	notify_window = 0;
	notify_gui_list = NULL;
	notify_button_opendialog = NULL;
	notify_button_remove = NULL;
	notify_save ();
}

static void
notify_selection_changed (gpointer user_data)
{
	struct notify_per_server *servnot;
	gboolean selected;

	(void) user_data;
	if (!notify_window || !notify_button_opendialog || !notify_button_remove)
		return;
	selected = notify_gui_list &&
		fabulor_notify_list_has_selection (notify_gui_list);
	servnot = notify_gui_list ?
		fabulor_notify_list_get_selected_server_data (notify_gui_list) : NULL;
	gtk_widget_set_sensitive (notify_button_opendialog,
		selected && servnot && servnot->ison);
	gtk_widget_set_sensitive (notify_button_remove, selected);
}

void
notify_gui_update (void)
{
	struct notify *notify;
	struct notify_per_server *servnot;
	GSList *list = notify_list;
	GSList *slist;
	gchar *name, *status, *server, *seen;
	int online, servcount, lastseenminutes;
	GdkRGBA color;
	time_t lastseen;
	char agobuf[128];
	FabulorNotifyListRow row;

	if (!notify_window)
		return;

	fabulor_notify_list_begin_update (notify_gui_list);

	while (list)
	{
		notify = (struct notify *) list->data;
		name = notify->name;
		status = _("Offline");
		server = "";

		online = FALSE;
		lastseen = 0;
		/* First see if they're online on any servers */
		slist = notify->server_list;
		while (slist)
		{
			servnot = (struct notify_per_server *) slist->data;
			if (servnot->ison)
				online = TRUE;
			if (servnot->lastseen > lastseen)
				lastseen = servnot->lastseen;
			slist = slist->next;
		}

		if (!online)				  /* Offline on all servers */
		{
			if (!lastseen)
				seen = _("Never");
			else
			{
				lastseenminutes = (int)(time (0) - lastseen) / 60;
				if (lastseenminutes < 60) 
					g_snprintf (agobuf, sizeof (agobuf), _("%d minutes ago"), lastseenminutes);
				else if (lastseenminutes < 120)
					g_snprintf (agobuf, sizeof (agobuf), _("An hour ago"));
				else
					g_snprintf (agobuf, sizeof (agobuf), _("%d hours ago"), lastseenminutes / 60);
				seen = agobuf;
			}
			row.owner = notify;
			row.server_data = NULL;
			row.owner_name = notify->name;
			row.display_name = name;
			row.status = status;
			row.network = server;
			row.last_seen = seen;
			row.foreground = theme_get_color (THEME_TOKEN_MIRC_4, &color) ?
				&color : NULL;
			g_warn_if_fail (fabulor_notify_list_append (notify_gui_list, &row));

		} else
		{
			/* Online - add one line per server */
			servcount = 0;
			slist = notify->server_list;
			status = _("Online");
			while (slist)
			{
				servnot = (struct notify_per_server *) slist->data;
				if (servnot->ison)
				{
					if (servcount > 0)
						name = "";
					server = server_get_network (servnot->server, TRUE);

					g_snprintf (agobuf, sizeof (agobuf), _("%d minutes ago"), (int)(time (0) - lastseen) / 60);
					seen = agobuf;

					row.owner = notify;
					row.server_data = servnot;
					row.owner_name = notify->name;
					row.display_name = name;
					row.status = status;
					row.network = server;
					row.last_seen = seen;
					row.foreground = theme_get_color (THEME_TOKEN_MIRC_3, &color) ?
						&color : NULL;
					g_warn_if_fail (fabulor_notify_list_append (notify_gui_list,
						&row));

					servcount++;
				}
				slist = slist->next;
			}
		}
		
		list = list->next;
	}

	fabulor_notify_list_end_update (notify_gui_list);
}

static void
notify_opendialog_clicked (GtkWidget * igad)
{
	struct notify_per_server *servnot;

	servnot = fabulor_notify_list_get_selected_server_data (notify_gui_list);
	if (servnot)
		open_query (servnot->server, servnot->notify->name, TRUE);
}

static void
notify_remove_clicked (GtkWidget * igad)
{
	char *name;

	name = fabulor_notify_list_dup_selected_name (notify_gui_list);
	if (name)
	{
		notify_deluser (name);
		g_free (name);
	}
}

static void
notifygui_add_cb (GtkDialog *dialog, gint response, gpointer entry)
{
	char *networks;
	char *text;

	text = (char *)gtk_entry_get_text (GTK_ENTRY (entry));
	if (text[0] && response == GTK_RESPONSE_ACCEPT)
	{
		networks = (char*)gtk_entry_get_text (GTK_ENTRY (g_object_get_data (G_OBJECT (entry), "net")));
		if (g_ascii_strcasecmp (networks, "ALL") == 0 || networks[0] == 0)
			notify_adduser (text, NULL);
		else
			notify_adduser (text, networks);
	}

	fabulor_gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
notifygui_add_enter (GtkWidget *entry, GtkWidget *dialog)
{
	gtk_dialog_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);
}

void
fe_notify_ask (char *nick, char *networks)
{
	GtkWidget *dialog;
	GtkWidget *entry;
	GtkWidget *label;
	GtkWidget *wid;
	GtkWidget *table;
	GtkWidget *content_area;
	char *msg = _("Enter nickname to add:");
	char buf[256];

	dialog = gtk_dialog_new_with_buttons (msg, NULL, 0,
										LABEL_NOTIFY_CANCEL, GTK_RESPONSE_REJECT,
										LABEL_NOTIFY_OK, GTK_RESPONSE_ACCEPT,
										NULL);
	theme_manager_attach_window (dialog);
	if (parent_window)
		gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent_window));
	fabulor_gtk_window_position_at_pointer (GTK_WINDOW (dialog));

	content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	table = gtkutil_grid_new (2, 3, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 12);
	gtk_grid_set_row_spacing (GTK_GRID (table), 3);
	gtk_grid_set_column_spacing (GTK_GRID (table), 8);
	fabulor_gtk_box_append (GTK_BOX (content_area), table, TRUE, TRUE, 0);

	label = gtk_label_new (msg);
	gtkutil_grid_attach_defaults (table, label, 0, 1, 0, 1);

	entry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (entry), nick);
	g_signal_connect (G_OBJECT (entry), "activate",
						 	G_CALLBACK (notifygui_add_enter), dialog);
	gtkutil_grid_attach_defaults (table, entry, 1, 2, 0, 1);

	g_signal_connect (G_OBJECT (dialog), "response",
						   G_CALLBACK (notifygui_add_cb), entry);

	label = gtk_label_new (_("Notify on these networks:"));
	gtkutil_grid_attach_defaults (table, label, 0, 1, 2, 3);

	wid = gtk_entry_new ();
	g_object_set_data (G_OBJECT (entry), "net", wid);
	g_signal_connect (G_OBJECT (wid), "activate",
						 	G_CALLBACK (notifygui_add_enter), dialog);
	gtk_entry_set_text (GTK_ENTRY (wid), networks ? networks : "ALL");
	gtkutil_grid_attach_defaults (table, wid, 1, 2, 2, 3);

	label = gtk_label_new (NULL);
	g_snprintf (buf, sizeof (buf), "<i><span size=\"smaller\">%s</span></i>", _("Comma separated list of networks is accepted."));
	gtk_label_set_markup (GTK_LABEL (label), buf);
	gtkutil_grid_attach_defaults (table, label, 1, 2, 3, 4);

	fabulor_gtk_widget_reveal_tree (dialog);
}

static void
notify_add_clicked (GtkWidget * igad)
{
	fe_notify_ask ("", NULL);
}

void
notify_opengui (void)
{
	GtkWidget *vbox, *bbox;
	char buf[128];

	if (notify_window)
	{
		mg_bring_tofront (notify_window);
		return;
	}

	g_snprintf(buf, sizeof(buf), _("Friends List - %s"), _(DISPLAY_NAME));
	notify_window =
		mg_create_generic_tab ("Notify", buf, FALSE, TRUE, notify_closegui, NULL, 400,
								250, &vbox, 0);
	gtkutil_destroy_on_esc (notify_window);

	notify_gui_list = fabulor_notify_list_new (notify_selection_changed, NULL);
	if (!notify_gui_list || !fabulor_notify_list_create_view (notify_gui_list,
		GTK_BOX (vbox), _("Name"), _("Status"), _("Network"), _("Last Seen")))
	{
		fabulor_notify_list_free (notify_gui_list);
		notify_gui_list = NULL;
		fabulor_gtk_window_destroy (GTK_WINDOW (notify_window));
		return;
	}
	g_object_set_data_full (G_OBJECT (notify_window), "notify-list",
		notify_gui_list, (GDestroyNotify) fabulor_notify_list_free);
  
	bbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_SPREAD);
	gtk_container_set_border_width (GTK_CONTAINER (bbox), 5);
	fabulor_gtk_box_append (GTK_BOX (vbox), bbox, FALSE, FALSE, 0);
	gtk_widget_show (bbox);

	gtkutil_button (bbox, ICON_NOTIFY_NEW, 0, notify_add_clicked, 0,
	                _("Add..."));

	notify_button_remove =
	gtkutil_button (bbox, ICON_NOTIFY_DELETE, 0, notify_remove_clicked, 0,
	                _("Remove"));

	notify_button_opendialog =
	gtkutil_button (bbox, NULL, 0, notify_opendialog_clicked, 0,
	                _("Open Dialog"));

	gtk_widget_set_sensitive (notify_button_opendialog, FALSE);
	gtk_widget_set_sensitive (notify_button_remove, FALSE);

	notify_gui_update ();

	gtk_widget_show (notify_window);
}
