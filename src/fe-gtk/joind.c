/* X-Chat
 * Copyright (C) 2005 Peter Zelezny.
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

/* joind.c - The Join Dialog.

   Popups up when you connect without any autojoin channels and helps you
   to find or join a channel.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

#ifndef WIN32
#include <unistd.h>
#endif

#include "../common/zoitechat.h"
#include "../common/zoitechatc.h"
#include "../common/server.h"
#include "../common/servlist.h"
#include "../common/fe.h"
#include "fe-gtk.h"
#include "chanlist.h"
#include "gtkutil.h"
#include "gtk-compat.h"
#include "theme/theme-manager.h"

#define ICON_JOIND_NETWORK "network-workgroup"


static void
joind_radio2_cb (GtkWidget *radio, server *serv)
{
	if (fabulor_gtk_check_button_get_active (radio))
	{
		gtk_widget_grab_focus (serv->gui->joind_entry);
		gtk_editable_set_position (GTK_EDITABLE (serv->gui->joind_entry), 999);
	}
}

static void
joind_entryenter_cb (GtkWidget *entry, GtkWidget *ok)
{
	gtk_widget_grab_focus (ok);
}

static void
joind_entryfocus_cb (GtkWidget *entry, gpointer user_data)
{
	server *serv = user_data;

	(void) entry;
	fabulor_gtk_check_button_set_active (serv->gui->joind_radio2, TRUE);
}

static void
joind_finalized_cb (gpointer user_data, GObject *window)
{
	server *serv = user_data;

	(void) window;
	if (is_server (serv))
		serv->gui->joind_win = NULL;
}

static void
joind_ok_cb (GtkWidget *ok, server *serv)
{
	if (!is_server (serv))
	{
		GtkWindow *window = fabulor_gtk_widget_get_root_window (ok);
		if (window)
			fabulor_gtk_window_destroy (window);
		return;
	}

	if (fabulor_gtk_check_button_get_active (serv->gui->joind_radio1))
		goto xit;

	if (fabulor_gtk_check_button_get_active (serv->gui->joind_radio2))
	{
		char *text = (char *)fabulor_gtk_entry_get_text (GTK_ENTRY (serv->gui->joind_entry));
		if (strlen (text) < 1)
		{
			fe_message (_("Channel name too short, try again."), FE_MSG_ERROR);
			return;
		}
		serv->p_join (serv, text, "");
		goto xit;
	}

	chanlist_opengui (serv, TRUE);

xit:
	prefs.hex_gui_join_dialog = 0;
	if (fabulor_gtk_check_button_get_active (serv->gui->joind_check))
		prefs.hex_gui_join_dialog = 1;

	fabulor_gtk_window_destroy (GTK_WINDOW (serv->gui->joind_win));
	serv->gui->joind_win = NULL;
}

static void
joind_show_dialog (server *serv)
{
	GtkWidget *dialog1;
	GtkWidget *dialog_vbox1;
	GtkWidget *vbox1;
	GtkWidget *hbox1;
	GtkWidget *image1;
	GtkWidget *vbox2;
	GtkWidget *label;
	GtkWidget *radiobutton1;
	GtkWidget *radiobutton2;
	GtkWidget *radiobutton3;
	GtkWidget *hbox2;
	GtkWidget *entry1;
	GtkWidget *checkbutton1;
	GtkWidget *okbutton1;
	char buf[256];
	char buf2[256];

	serv->gui->joind_win = dialog1 = gtk_dialog_new ();
	theme_manager_attach_window (dialog1);
	g_snprintf(buf, sizeof(buf), _("Connection Complete - %s"), _(DISPLAY_NAME));
	gtk_window_set_title (GTK_WINDOW (dialog1), buf);
	fabulor_gtk_window_set_dialog_hint (GTK_WINDOW (dialog1));
	fabulor_gtk_window_position_center_on_parent (GTK_WINDOW (dialog1));
	gtk_window_set_transient_for (GTK_WINDOW(dialog1), GTK_WINDOW(serv->front_session->gui->window));
	gtk_window_set_modal (GTK_WINDOW (dialog1), TRUE);
	gtk_window_set_resizable (GTK_WINDOW (dialog1), FALSE);

	dialog_vbox1 = gtk_dialog_get_content_area (GTK_DIALOG (dialog1));
	gtk_widget_show (dialog_vbox1);

	vbox1 = gtkutil_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 0);
	gtk_widget_show (vbox1);
	fabulor_gtk_box_append (GTK_BOX (dialog_vbox1), vbox1, TRUE, TRUE, 0);

	hbox1 = gtkutil_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
	gtk_widget_show (hbox1);
	fabulor_gtk_box_append (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

	image1 = gtkutil_image_new_from_stock (ICON_JOIND_NETWORK, FABULOR_GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_widget_show (image1);
	fabulor_gtk_box_append (GTK_BOX (hbox1), image1, FALSE, TRUE, 24);
	gtk_widget_set_halign (image1, GTK_ALIGN_CENTER);
	gtk_widget_set_valign (image1, GTK_ALIGN_START);
	gtk_widget_set_margin_top (image1, 2);

	vbox2 = gtkutil_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 10);
	fabulor_gtk_container_set_uniform_inset (vbox2, 6);
	gtk_widget_show (vbox2);
	fabulor_gtk_box_append (GTK_BOX (hbox1), vbox2, TRUE, TRUE, 0);

	g_snprintf (buf2, sizeof (buf2), _("Connection to %s complete."),
				 server_get_network (serv, TRUE));
	g_snprintf (buf, sizeof (buf), "\n<b>%s</b>", buf2);
	label = gtk_label_new (buf);
	gtk_widget_show (label);
	fabulor_gtk_box_append (GTK_BOX (vbox2), label, FALSE, FALSE, 0);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_valign (label, GTK_ALIGN_CENTER);

	label = gtk_label_new (_("In the server list window, no channel (chat room) has been entered to be automatically joined for this network."));
	gtk_widget_show (label);
	fabulor_gtk_box_append (GTK_BOX (vbox2), label, FALSE, FALSE, 0);
	fabulor_gtk_label_set_wrap (GTK_LABEL (label), TRUE);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_valign (label, GTK_ALIGN_CENTER);

	label = gtk_label_new (_("What would you like to do next?"));
	gtk_widget_show (label);
	fabulor_gtk_box_append (GTK_BOX (vbox2), label, FALSE, FALSE, 0);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_valign (label, GTK_ALIGN_CENTER);

	serv->gui->joind_radio1 = radiobutton1 =
		fabulor_gtk_radio_button_new_with_mnemonic (NULL,
			_("_Nothing, I'll join a channel later."));
	gtk_widget_show (radiobutton1);
	fabulor_gtk_box_append (GTK_BOX (vbox2), radiobutton1, FALSE, FALSE, 0);

	hbox2 = gtkutil_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
	gtk_widget_show (hbox2);
	fabulor_gtk_box_append (GTK_BOX (vbox2), hbox2, FALSE, FALSE, 0);

	serv->gui->joind_radio2 = radiobutton2 =
		fabulor_gtk_radio_button_new_with_mnemonic (radiobutton1,
			_("_Join this channel:"));
	gtk_widget_show (radiobutton2);
	fabulor_gtk_box_append (GTK_BOX (hbox2), radiobutton2, FALSE, FALSE, 0);

	serv->gui->joind_entry = entry1 = gtk_entry_new ();
	fabulor_gtk_entry_set_text (GTK_ENTRY (entry1), "#");
	gtk_widget_show (entry1);
	fabulor_gtk_box_append (GTK_BOX (hbox2), entry1, TRUE, TRUE, 8);

	g_snprintf (buf, sizeof (buf), "<small>     %s</small>",
				 _("If you know the name of the channel you want to join, enter it here."));
	label = gtk_label_new (buf);
	gtk_widget_show (label);
	fabulor_gtk_box_append (GTK_BOX (vbox2), label, FALSE, FALSE, 0);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_valign (label, GTK_ALIGN_CENTER);

	radiobutton3 = fabulor_gtk_radio_button_new_with_mnemonic (radiobutton1,
		_("O_pen the channel list."));
	gtk_widget_show (radiobutton3);
	fabulor_gtk_box_append (GTK_BOX (vbox2), radiobutton3, FALSE, FALSE, 0);

	g_snprintf (buf, sizeof (buf), "<small>     %s</small>",
				 _("Retrieving the channel list may take a minute or two."));
	label = gtk_label_new (buf);
	gtk_widget_show (label);
	fabulor_gtk_box_append (GTK_BOX (vbox2), label, FALSE, FALSE, 0);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_valign (label, GTK_ALIGN_CENTER);

	serv->gui->joind_check = checkbutton1 = gtk_check_button_new_with_mnemonic (_("_Always show this dialog after connecting."));
	if (prefs.hex_gui_join_dialog)
		fabulor_gtk_check_button_set_active (checkbutton1, TRUE);
	gtk_widget_show (checkbutton1);
	fabulor_gtk_box_append (GTK_BOX (vbox1), checkbutton1, FALSE, FALSE, 0);

	okbutton1 = gtkutil_button_new_from_stock ("gtk-ok", _("_OK"));
	gtk_widget_show (okbutton1);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), okbutton1, GTK_RESPONSE_OK);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog1), GTK_RESPONSE_OK);

	g_object_weak_ref (G_OBJECT (dialog1), joind_finalized_cb, serv);
	fabulor_gtk_widget_on_focus_enter (entry1, joind_entryfocus_cb, serv);
	g_signal_connect (G_OBJECT (entry1), "activate",
							G_CALLBACK (joind_entryenter_cb), okbutton1);
	g_signal_connect (G_OBJECT (radiobutton2), "toggled",
							G_CALLBACK (joind_radio2_cb), serv);
	g_signal_connect (G_OBJECT (okbutton1), "clicked",
							G_CALLBACK (joind_ok_cb), serv);
							
	if (serv->network)
		if (g_ascii_strcasecmp(((ircnet*)serv->network)->name, "Zoite") == 0)
		{
			fabulor_gtk_entry_set_text (GTK_ENTRY (entry1), "#zoitechat");
		}

	gtk_widget_grab_focus (okbutton1);
	fabulor_gtk_widget_reveal_tree (dialog1);
}

void
joind_open (server *serv)
{
	if (prefs.hex_gui_join_dialog)
		joind_show_dialog (serv);
}

void
joind_close (server *serv)
{
	if (serv->gui->joind_win)
	{
		fabulor_gtk_window_destroy (GTK_WINDOW (serv->gui->joind_win));
		serv->gui->joind_win = NULL;
	}
}
