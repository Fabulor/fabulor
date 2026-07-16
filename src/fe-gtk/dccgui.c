/* X-Chat
 * Copyright (C) 1998-2006 Peter Zelezny.
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
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define WANTSOCKET
#define WANTARPA
#include "../common/inet.h"
#include "fe-gtk.h"

#include "../common/zoitechat.h"
#include "../common/zoitechatc.h"
#include "../common/fe.h"
#include "../common/util.h"
#include "../common/network.h"
#include "gtkutil.h"
#include "gtk-compat.h"
#include "theme/theme-gtk.h"
#include "maingui.h"
#include "dcc-chat-list.h"
#include "dcc-transfer-list.h"
#include "theme/theme-access.h"

#define ICON_DCC_CANCEL "dialog-cancel"
#define ICON_DCC_ACCEPT "dialog-apply"
#define ICON_DCC_RESUME "view-refresh"
#define ICON_DCC_CLEAR "edit-clear"


struct dccwindow
{
	GtkWidget *window;

	GtkWidget *list;
	FabulorDccChatList *chats;
	FabulorDccTransferList *transfers;

	GtkWidget *abort_button;
	GtkWidget *accept_button;
	GtkWidget *resume_button;
	GtkWidget *open_button;
	GtkWidget *clear_button; /* clears aborted and completed requests */	

	GtkWidget *file_label;
	GtkWidget *address_label;
};

struct my_dcc_send
{
	struct session *sess;
	char *nick;
	gint64 maxcps;
	int passive;
};

static struct dccwindow dccfwin = {NULL, };	/* file */
static struct dccwindow dcccwin = {NULL, };	/* chat */
static int win_width = 600;
static int win_height = 256;
static short view_mode;	/* 1=download 2=upload 3=both */
#define VIEW_DOWNLOAD 1
#define VIEW_UPLOAD 2
#define VIEW_BOTH 3

static void
proper_unit (guint64 size, char *buf, size_t buf_len)
{
	gchar *formatted_str;
	GFormatSizeFlags format_flags = G_FORMAT_SIZE_DEFAULT;

	if (prefs.hex_gui_filesize_iec)
		format_flags = G_FORMAT_SIZE_IEC_UNITS;

	formatted_str = g_format_size_full (size, format_flags);
	g_strlcpy (buf, formatted_str, buf_len);

	g_free (formatted_str);
}

static void
dcc_send_filereq_file (struct my_dcc_send *mdc, char *file)
{
	if (file)
		dcc_send (mdc->sess, mdc->nick, file, mdc->maxcps, mdc->passive);
	else
	{
		g_free (mdc->nick);
		g_free (mdc);
	}
}

void
fe_dcc_send_filereq (struct session *sess, char *nick, int maxcps, int passive)
{
	char* tbuf = g_strdup_printf (_("Send file to %s"), nick);

	struct my_dcc_send *mdc = g_new (struct my_dcc_send, 1);
	mdc->sess = sess;
	mdc->nick = g_strdup (nick);
	mdc->maxcps = maxcps;
	mdc->passive = passive;

	gtkutil_file_req (NULL, tbuf, dcc_send_filereq_file, mdc, prefs.hex_dcc_dir, NULL, FRF_MULTIPLE|FRF_FILTERISINITIAL);

	g_free (tbuf);
}

static void
dcc_chat_snapshot (struct DCC *dcc, FabulorDccChatSnapshot *snapshot)
{
	static char pos[32], size[32], start_time[32];
	char *date;
	GdkRGBA color;

	date = ctime (&dcc->starttime);
	if (date)
	{
		g_strlcpy (start_time, date, sizeof (start_time));
		start_time[strcspn (start_time, "\r\n")] = 0;
	}
	else
		start_time[0] = 0;

	proper_unit (dcc->pos, pos, sizeof (pos));
	proper_unit (dcc->size, size, sizeof (size));

	memset (snapshot, 0, sizeof (*snapshot));
	snapshot->identity = dcc;
	snapshot->status = _(dccstat[dcc->dccstat].name);
	snapshot->nick = dcc->nick;
	snapshot->received = pos;
	snapshot->sent = size;
	snapshot->start_time = start_time;
	snapshot->has_color = dccstat[dcc->dccstat].color != 1 &&
		theme_get_mirc_color ((unsigned int) dccstat[dcc->dccstat].color, &color);
	if (snapshot->has_color)
		snapshot->color = color;
}

static void
dcc_chat_apply (struct DCC *dcc, gboolean append, gboolean prepend)
{
	FabulorDccChatSnapshot snapshot;
	dcc_chat_snapshot (dcc, &snapshot);
	if (append)
		fabulor_dcc_chat_list_append (dcccwin.chats, &snapshot, prepend);
	else
		fabulor_dcc_chat_list_update (dcccwin.chats, &snapshot);
}

static void
dcc_transfer_snapshot (struct DCC *dcc,
	FabulorDccTransferSnapshot *snapshot)
{
	static char size[32], pos[32], kbs[32], perc[32], eta[32];
	guint64 progress = dcc->type == TYPE_SEND ? dcc->ack : dcc->pos;
	guint64 remaining = dcc->size > progress ? dcc->size - progress : 0;
	guint64 to_go = dcc->cps > 0 ? remaining / (guint64) dcc->cps : 0;
	double percentage = dcc->size > 0 ?
		((double) progress * 100.0) / (double) dcc->size : 0.0;
	GdkRGBA color;

	proper_unit (dcc->size, size, sizeof (size));
	if (dcc->type == TYPE_RECV && dcc->dccstat == STAT_QUEUED)
		proper_unit (dcc->resumable, pos, sizeof (pos));
	else
		proper_unit (dcc->pos, pos, sizeof (pos));
	g_snprintf (kbs, sizeof (kbs), "%.1f", ((double) dcc->cps) / 1024.0);
	g_snprintf (perc, sizeof (perc), "%.0f%%", percentage);
	if (dcc->cps > 0)
		g_snprintf (eta, sizeof (eta), "%.2llu:%.2llu:%.2llu",
			(unsigned long long) (to_go / 3600),
			(unsigned long long) ((to_go / 60) % 60),
			(unsigned long long) (to_go % 60));
	else
		g_strlcpy (eta, "--:--:--", sizeof (eta));

	memset (snapshot, 0, sizeof (*snapshot));
	snapshot->identity = dcc;
	snapshot->upload = dcc->type == TYPE_SEND;
	snapshot->status = _(dccstat[dcc->dccstat].name);
	snapshot->file = file_part (dcc->file);
	snapshot->size = size;
	snapshot->position = pos;
	snapshot->percentage = perc;
	snapshot->speed = kbs;
	snapshot->eta = eta;
	snapshot->nick = dcc->nick;
	snapshot->has_color = dccstat[dcc->dccstat].color != 1 &&
		theme_get_mirc_color ((unsigned int) dccstat[dcc->dccstat].color, &color);
	if (snapshot->has_color)
		snapshot->color = color;
}

static void
dcc_transfer_apply (struct DCC *dcc, gboolean append, gboolean prepend)
{
	FabulorDccTransferSnapshot snapshot;
	dcc_transfer_snapshot (dcc, &snapshot);
	if (append)
		fabulor_dcc_transfer_list_append (dccfwin.transfers, &snapshot, prepend);
	else
		fabulor_dcc_transfer_list_update (dccfwin.transfers, &snapshot);
}

static void
dcc_update_recv (struct DCC *dcc)
{
	if (!dccfwin.window)
		return;
	dcc_transfer_apply (dcc, FALSE, FALSE);
}

static void
dcc_update_chat (struct DCC *dcc)
{
	if (!dcccwin.window)
		return;
	dcc_chat_apply (dcc, FALSE, FALSE);
}

static void
dcc_update_send (struct DCC *dcc)
{
	if (!dccfwin.window)
		return;
	dcc_transfer_apply (dcc, FALSE, FALSE);
}

static void
close_dcc_file_window (GtkWindow *win, gpointer data)
{
	fabulor_dcc_transfer_list_free (dccfwin.transfers);
	dccfwin.transfers = NULL;
	dccfwin.window = NULL;
}

static void
dcc_append (struct DCC *dcc, gboolean prepend)
{
	dcc_transfer_apply (dcc, TRUE, prepend);
}

/* Returns aborted and completed transfers. */
static GSList *
dcc_get_completed (void)
{
	GPtrArray *rows;
	GSList *completed = NULL;
	guint i;

	rows = fabulor_dcc_transfer_list_dup_all (dccfwin.transfers);
	for (i = 0; i < rows->len; i++)
	{
		struct DCC *dcc = g_ptr_array_index (rows, i);
		if (is_dcc_completed (dcc))
			completed = g_slist_prepend (completed, dcc);
	}
	g_ptr_array_unref (rows);
	return completed;
}

static gboolean
dcc_completed_transfer_exists (void)
{
	gboolean exist;
	GSList *comp_list;
	
	comp_list = dcc_get_completed (); 
	exist = comp_list != NULL;
	
	g_slist_free (comp_list);	
	return exist;
}

static void
update_clear_button_sensitivity (void)
{
	gboolean sensitive = dcc_completed_transfer_exists ();
	gtk_widget_set_sensitive (dccfwin.clear_button, sensitive);
}

static void
dcc_fill_window (int flags)
{
	struct DCC *dcc;
	GSList *list;
	int i = 0;

	fabulor_dcc_transfer_list_clear (dccfwin.transfers);

	if (flags & VIEW_UPLOAD)
	{
		list = dcc_list;
		while (list)
		{
			dcc = list->data;
			if (dcc->type == TYPE_SEND)
			{
				dcc_append (dcc, FALSE);
				i++;
			}
			list = list->next;
		}
	}

	if (flags & VIEW_DOWNLOAD)
	{
		list = dcc_list;
		while (list)
		{
			dcc = list->data;
			if (dcc->type == TYPE_RECV)
			{
				dcc_append (dcc, FALSE);
				i++;
			}
			list = list->next;
		}
	}

	/* if only one entry, select it (so Accept button can work) */
	if (i == 1)
		fabulor_dcc_transfer_list_select_first (dccfwin.transfers);
	
	update_clear_button_sensitivity ();
}

static void
resume_clicked (GtkWidget * wid, gpointer none)
{
	struct DCC *dcc;
	char buf[512];
	GPtrArray *list;

	list = fabulor_dcc_transfer_list_dup_selected (dccfwin.transfers);
	if (list->len == 0)
	{
		g_ptr_array_unref (list);
		return;
	}
	dcc = g_ptr_array_index (list, 0);
	g_ptr_array_unref (list);

	if (dcc->type == TYPE_RECV && !dcc_resume (dcc))
	{
		switch (dcc->resume_error)
		{
		case 0:	/* unknown error */
			fe_message (_("That file is not resumable."), FE_MSG_ERROR);
			break;
		case 1:
			g_snprintf (buf, sizeof (buf),
						_(	"Cannot access file: %s\n"
							"%s.\n"
							"Resuming not possible."), dcc->destfile,	
							errorstring (dcc->resume_errno));
			fe_message (buf, FE_MSG_ERROR);
			break;
		case 2:
			fe_message (_("File in download directory is larger "
							"than file offered. Resuming not possible."), FE_MSG_ERROR);
			break;
		case 3:
			fe_message (_("Cannot resume the same file from two people."), FE_MSG_ERROR);
		}
	}
}

static void
abort_clicked (GtkWidget * wid, gpointer none)
{
	GPtrArray *list = fabulor_dcc_transfer_list_dup_selected (dccfwin.transfers);
	guint i;

	for (i = 0; i < list->len; i++)
	{
		struct DCC *dcc = g_ptr_array_index (list, i);
		dcc_abort (dcc->serv->front_session, dcc);
	}
	g_ptr_array_unref (list);
	
	/* Enable the clear button if it wasn't already enabled */
	update_clear_button_sensitivity ();
}

static void
accept_clicked (GtkWidget * wid, gpointer none)
{
	GPtrArray *list = fabulor_dcc_transfer_list_dup_selected (dccfwin.transfers);
	guint i;

	for (i = 0; i < list->len; i++)
	{
		struct DCC *dcc = g_ptr_array_index (list, i);
		if (dcc->type != TYPE_SEND)
			dcc_get (dcc);
	}
	g_ptr_array_unref (list);
}

static void
clear_completed (GtkWidget * wid, gpointer none)
{
	struct DCC *dcc;
	GSList *completed, *current;

	/* Make a new list of only the completed items and abort each item.
	 * A new list is made because calling dcc_abort removes items from the original list,
	 * making it impossible to iterate over that list directly.
	*/
	completed = dcc_get_completed ();
	for (current = completed; current; current = current->next)
	{
		dcc = current->data;
		dcc_abort (dcc->serv->front_session, dcc);
	}

	/* The data was freed by dcc_close */
	g_slist_free (completed);
	update_clear_button_sensitivity ();
}

static void
browse_folder (char *dir)
{
	fe_open_url (dir);
}

static void
browse_dcc_folder (void)
{
	if (prefs.hex_dcc_completed_dir[0])
		browse_folder (prefs.hex_dcc_completed_dir);
	else
		browse_folder (prefs.hex_dcc_dir);
}

static void
dcc_details_populate (struct DCC *dcc)
{
	char buf[128];

	if (!dcc)
	{
		gtk_label_set_text (GTK_LABEL (dccfwin.file_label), NULL);
		gtk_label_set_text (GTK_LABEL (dccfwin.address_label), NULL);
		return;
	}

	/* full path */
	if (dcc->type == TYPE_RECV)
		gtk_label_set_text (GTK_LABEL (dccfwin.file_label), dcc->destfile);
	else
		gtk_label_set_text (GTK_LABEL (dccfwin.file_label), dcc->file);

	/* address and port */
	g_snprintf (buf, sizeof (buf), "%s : %d", net_ip (dcc->addr), dcc->port);
	gtk_label_set_text (GTK_LABEL (dccfwin.address_label), buf);
}

static void
dcc_row_cb (gpointer user_data)
{
	struct DCC *dcc;
	GPtrArray *list;

	list = fabulor_dcc_transfer_list_dup_selected (dccfwin.transfers);
	if (list->len == 0)
	{
		gtk_widget_set_sensitive (dccfwin.accept_button, FALSE);
		gtk_widget_set_sensitive (dccfwin.resume_button, FALSE);
		gtk_widget_set_sensitive (dccfwin.abort_button, FALSE);
		dcc_details_populate (NULL);
		g_ptr_array_unref (list);
		return;
	}

	gtk_widget_set_sensitive (dccfwin.abort_button, TRUE);

	if (list->len > 1)	/* multi selection */
	{
		gtk_widget_set_sensitive (dccfwin.accept_button, TRUE);
		gtk_widget_set_sensitive (dccfwin.resume_button, TRUE);
		dcc_details_populate (g_ptr_array_index (list, 0));
	}
	else
	{
		/* turn OFF/ON appropriate buttons */
		dcc = g_ptr_array_index (list, 0);
		if (dcc->dccstat == STAT_QUEUED && dcc->type == TYPE_RECV)
		{
			gtk_widget_set_sensitive (dccfwin.accept_button, TRUE);
			gtk_widget_set_sensitive (dccfwin.resume_button, TRUE);
		}
		else
		{
			gtk_widget_set_sensitive (dccfwin.accept_button, FALSE);
			gtk_widget_set_sensitive (dccfwin.resume_button, FALSE);
		}

		dcc_details_populate (dcc);
	}

	g_ptr_array_unref (list);
}

static void
dcc_dclick_cb (gpointer identity, gpointer user_data)
{
	struct DCC *dcc = identity;
	(void) user_data;

	if (dcc->type == TYPE_RECV)
	{
		accept_clicked (0, 0);
		return;
	}

	switch (dcc->dccstat)
	{
	case STAT_FAILED:
	case STAT_ABORTED:
	case STAT_DONE:
		dcc_abort (dcc->serv->front_session, dcc);
		break;
	case STAT_QUEUED:
	case STAT_ACTIVE:
	case STAT_CONNECTING:
		break;
	}
}

static GtkWidget *
dcc_detail_label (char *text, GtkWidget *box, int num)
{
	GtkWidget *label;
	char buf[64];

	label = gtk_label_new (NULL);
	g_snprintf (buf, sizeof (buf), "<b>%s</b>", text);
	gtk_label_set_markup (GTK_LABEL (label), buf);
	gtk_widget_set_hexpand (label, FALSE);
	gtk_widget_set_vexpand (label, FALSE);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_valign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (box), label, 0, 0 + num, 1, 1);

	label = gtk_label_new (NULL);
	gtk_label_set_selectable (GTK_LABEL (label), TRUE);
	gtk_widget_set_hexpand (label, FALSE);
	gtk_widget_set_vexpand (label, FALSE);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_valign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (box), label, 1, 0 + num, 1, 1);

	return label;
}

static void
dcc_exp_cb (GtkWidget *exp, GtkWidget *box)
{
	if (gtk_widget_get_visible (box))
	{
		gtk_widget_hide (box);
	}
	else
	{
		gtk_widget_show (box);
	}
}

static void
dcc_toggle (GtkWidget *item, gpointer data)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (item)))
	{
		view_mode = GPOINTER_TO_INT (data);
		dcc_fill_window (GPOINTER_TO_INT (data));
	}
}

static gboolean
dcc_configure_cb (GtkWindow *win, GdkEventConfigure *event, gpointer data)
{
	/* remember the window size */
	gtk_window_get_size (win, &win_width, &win_height);
	return FALSE;
}

int
fe_dcc_open_recv_win (int passive)
{
	GtkWidget *radio, *table, *vbox, *bbox, *view, *exp, *detailbox;
	GSList *group;
	char buf[128];

	if (dccfwin.window)
	{
		if (!passive)
			mg_bring_tofront (dccfwin.window);
		return TRUE;
	}
	g_snprintf(buf, sizeof(buf), _("Uploads and Downloads - %s"), _(DISPLAY_NAME));
	dccfwin.window = mg_create_generic_tab ("Transfers", buf, FALSE, TRUE, close_dcc_file_window,
														 NULL, win_width, win_height, &vbox, 0);
	gtkutil_destroy_on_esc (dccfwin.window);
	gtk_container_set_border_width (GTK_CONTAINER (dccfwin.window), 3);
	gtk_box_set_spacing (GTK_BOX (vbox), 3);

	dccfwin.transfers = fabulor_dcc_transfer_list_new (dcc_row_cb,
		dcc_dclick_cb, NULL);
	if (!dccfwin.transfers)
	{
		fe_message (_("Failed to create the DCC transfer list."), FE_MSG_ERROR);
		fabulor_gtk_window_destroy (GTK_WINDOW (dccfwin.window));
		return FALSE;
	}
	view = fabulor_dcc_transfer_list_create_view (dccfwin.transfers,
		GTK_BOX (vbox), _("Status"), _("File"), _("Size"), _("Position"),
		"%", "KB/s", _("ETA"), _("Nick"));
	dccfwin.list = view;
	view_mode = VIEW_BOTH;

	if (!prefs.hex_gui_tab_utils)
		g_signal_connect (G_OBJECT (dccfwin.window), "configure-event",
								G_CALLBACK (dcc_configure_cb), 0);

	table = gtk_grid_new ();
	gtk_grid_set_column_spacing (GTK_GRID (table), 16);
	fabulor_gtk_box_append (GTK_BOX (vbox), table, FALSE, FALSE, 0);

	radio = gtk_radio_button_new_with_mnemonic (NULL, _("Both"));
	g_signal_connect (G_OBJECT (radio), "toggled",
							G_CALLBACK (dcc_toggle), GINT_TO_POINTER (VIEW_BOTH));
	gtk_widget_set_hexpand (radio, FALSE);
	gtk_widget_set_vexpand (radio, FALSE);
	gtk_widget_set_halign (radio, GTK_ALIGN_FILL);
	gtk_widget_set_valign (radio, GTK_ALIGN_FILL);
	gtk_grid_attach (GTK_GRID (table), radio, 3, 0, 1, 1);
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio));

	radio = gtk_radio_button_new_with_mnemonic (group, _("Uploads"));
	g_signal_connect (G_OBJECT (radio), "toggled",
							G_CALLBACK (dcc_toggle), GINT_TO_POINTER (VIEW_UPLOAD));
	gtk_widget_set_hexpand (radio, FALSE);
	gtk_widget_set_vexpand (radio, FALSE);
	gtk_widget_set_halign (radio, GTK_ALIGN_FILL);
	gtk_widget_set_valign (radio, GTK_ALIGN_FILL);
	gtk_grid_attach (GTK_GRID (table), radio, 1, 0, 1, 1);
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio));

	radio = gtk_radio_button_new_with_mnemonic (group, _("Downloads"));
	g_signal_connect (G_OBJECT (radio), "toggled",
							G_CALLBACK (dcc_toggle), GINT_TO_POINTER (VIEW_DOWNLOAD));
	gtk_widget_set_hexpand (radio, FALSE);
	gtk_widget_set_vexpand (radio, FALSE);
	gtk_widget_set_halign (radio, GTK_ALIGN_FILL);
	gtk_widget_set_valign (radio, GTK_ALIGN_FILL);
	gtk_grid_attach (GTK_GRID (table), radio, 2, 0, 1, 1);

	exp = gtk_expander_new (_("Details"));
	gtk_widget_set_hexpand (exp, TRUE);
	gtk_widget_set_vexpand (exp, FALSE);
	gtk_widget_set_halign (exp, GTK_ALIGN_FILL);
	gtk_widget_set_valign (exp, GTK_ALIGN_FILL);
	gtk_grid_attach (GTK_GRID (table), exp, 0, 0, 1, 1);

	detailbox = gtk_grid_new ();
	gtk_grid_set_column_spacing (GTK_GRID (detailbox), 6);
	gtk_grid_set_row_spacing (GTK_GRID (detailbox), 2);
	gtk_container_set_border_width (GTK_CONTAINER (detailbox), 6);
	g_signal_connect (G_OBJECT (exp), "activate",
							G_CALLBACK (dcc_exp_cb), detailbox);
	gtk_widget_set_hexpand (detailbox, TRUE);
	gtk_widget_set_vexpand (detailbox, FALSE);
	gtk_widget_set_halign (detailbox, GTK_ALIGN_FILL);
	gtk_widget_set_valign (detailbox, GTK_ALIGN_FILL);
	gtk_grid_attach (GTK_GRID (table), detailbox, 0, 1, 4, 1);

	dccfwin.file_label = dcc_detail_label (_("File:"), detailbox, 0);
	dccfwin.address_label = dcc_detail_label (_("Address:"), detailbox, 1);

	bbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_SPREAD);
	fabulor_gtk_box_append (GTK_BOX (vbox), bbox, FALSE, FALSE, 2);

	dccfwin.abort_button = gtkutil_button (bbox, ICON_DCC_CANCEL, 0, abort_clicked, 0, _("Abort"));
	dccfwin.accept_button = gtkutil_button (bbox, ICON_DCC_ACCEPT, 0, accept_clicked, 0, _("Accept"));
	dccfwin.resume_button = gtkutil_button (bbox, ICON_DCC_RESUME, 0, resume_clicked, 0, _("Resume"));
	dccfwin.clear_button = gtkutil_button (bbox, ICON_DCC_CLEAR, 0, clear_completed, 0, _("Clear"));
	dccfwin.open_button = gtkutil_button (bbox, 0, 0, browse_dcc_folder, 0, _("Open Folder..."));
	gtk_widget_set_sensitive (dccfwin.accept_button, FALSE);
	gtk_widget_set_sensitive (dccfwin.resume_button, FALSE);
	gtk_widget_set_sensitive (dccfwin.abort_button, FALSE);

	dcc_fill_window (3);
	fabulor_gtk_widget_reveal_tree (dccfwin.window);
	gtk_widget_hide (detailbox);

	return FALSE;
}

int
fe_dcc_open_send_win (int passive)
{
	/* combined send/recv GUI */
	return fe_dcc_open_recv_win (passive);
}


/* DCC CHAT GUIs BELOW */

static void
accept_chat_clicked (GtkWidget * wid, gpointer none)
{
	GPtrArray *list = fabulor_dcc_chat_list_dup_selected (dcccwin.chats);
	guint i;

	for (i = 0; i < list->len; i++)
	{
		struct DCC *dcc = g_ptr_array_index (list, i);
		dcc_get (dcc);
	}
	g_ptr_array_unref (list);
}

static void
abort_chat_clicked (GtkWidget * wid, gpointer none)
{
	GPtrArray *list = fabulor_dcc_chat_list_dup_selected (dcccwin.chats);
	guint i;

	for (i = 0; i < list->len; i++)
	{
		struct DCC *dcc = g_ptr_array_index (list, i);
		dcc_abort (dcc->serv->front_session, dcc);
	}
	g_ptr_array_unref (list);
}

static void
dcc_chat_close_cb (void)
{
	fabulor_dcc_chat_list_free (dcccwin.chats);
	dcccwin.chats = NULL;
	dcccwin.window = NULL;
}

static void
dcc_chat_append (struct DCC *dcc, gboolean prepend)
{
	dcc_chat_apply (dcc, TRUE, prepend);
}

static void
dcc_chat_fill_win (void)
{
	struct DCC *dcc;
	GSList *list;
	int i = 0;

	fabulor_dcc_chat_list_clear (dcccwin.chats);

	list = dcc_list;
	while (list)
	{
		dcc = list->data;
		if (dcc->type == TYPE_CHATSEND || dcc->type == TYPE_CHATRECV)
		{
			dcc_chat_append (dcc, FALSE);
			i++;
		}
		list = list->next;
	}

	/* if only one entry, select it (so Accept button can work) */
	if (i == 1)
		fabulor_dcc_chat_list_select_first (dcccwin.chats);
}

static void
dcc_chat_row_cb (guint selected, gpointer user_data)
{
	struct DCC *dcc;
	GPtrArray *list;

	if (selected == 0)
	{
		gtk_widget_set_sensitive (dcccwin.accept_button, FALSE);
		gtk_widget_set_sensitive (dcccwin.abort_button, FALSE);
		return;
	}

	list = fabulor_dcc_chat_list_dup_selected (dcccwin.chats);
	gtk_widget_set_sensitive (dcccwin.abort_button, TRUE);

	if (selected > 1)	/* multi selection */
		gtk_widget_set_sensitive (dcccwin.accept_button, TRUE);
	else
	{
		/* turn OFF/ON appropriate buttons */
		dcc = g_ptr_array_index (list, 0);
		if (dcc->dccstat == STAT_QUEUED && dcc->type == TYPE_CHATRECV)
			gtk_widget_set_sensitive (dcccwin.accept_button, TRUE);
		else
			gtk_widget_set_sensitive (dcccwin.accept_button, FALSE);
	}

	g_ptr_array_unref (list);
}

static void
dcc_chat_dclick_cb (gpointer identity, gpointer user_data)
{
	(void) identity;
	(void) user_data;
	accept_chat_clicked (0, 0);
}

int
fe_dcc_open_chat_win (int passive)
{
	GtkWidget *view, *vbox, *bbox;
	char buf[128];

	if (dcccwin.window)
	{
		if (!passive)
			mg_bring_tofront (dcccwin.window);
		return TRUE;
	}

	g_snprintf(buf, sizeof(buf), _("DCC Chat List - %s"), _(DISPLAY_NAME));
	dcccwin.window =
			  mg_create_generic_tab ("DCCChat", buf, FALSE, TRUE, dcc_chat_close_cb,
						NULL, 550, 180, &vbox, 0);
	gtkutil_destroy_on_esc (dcccwin.window);
	gtk_container_set_border_width (GTK_CONTAINER (dcccwin.window), 3);
	gtk_box_set_spacing (GTK_BOX (vbox), 3);

	dcccwin.chats = fabulor_dcc_chat_list_new (dcc_chat_row_cb,
		dcc_chat_dclick_cb, NULL);
	if (!dcccwin.chats)
	{
		fe_message (_("Failed to create the DCC Chat list."), FE_MSG_ERROR);
		fabulor_gtk_window_destroy (GTK_WINDOW (dcccwin.window));
		return FALSE;
	}
	view = fabulor_dcc_chat_list_create_view (dcccwin.chats, GTK_BOX (vbox),
		_("Status"), _("Nick"), _("Recv"), _("Sent"), _("Start Time"));
	dcccwin.list = view;

	bbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_SPREAD);
	fabulor_gtk_box_append (GTK_BOX (vbox), bbox, FALSE, FALSE, 2);

	dcccwin.abort_button = gtkutil_button (bbox, ICON_DCC_CANCEL, 0, abort_chat_clicked, 0, _("Abort"));
	dcccwin.accept_button = gtkutil_button (bbox, ICON_DCC_ACCEPT, 0, accept_chat_clicked, 0, _("Accept"));
	gtk_widget_set_sensitive (dcccwin.accept_button, FALSE);
	gtk_widget_set_sensitive (dcccwin.abort_button, FALSE);

	dcc_chat_fill_win ();
	fabulor_gtk_widget_reveal_tree (dcccwin.window);

	return FALSE;
}

void
fe_dcc_add (struct DCC *dcc)
{
	switch (dcc->type)
	{
	case TYPE_RECV:
		if (dccfwin.window && (view_mode & VIEW_DOWNLOAD))
			dcc_append (dcc, TRUE);
		break;

	case TYPE_SEND:
		if (dccfwin.window && (view_mode & VIEW_UPLOAD))
			dcc_append (dcc, TRUE);
		break;

	default: /* chat */
		if (dcccwin.window)
			dcc_chat_append (dcc, TRUE);
	}
}

void
fe_dcc_update (struct DCC *dcc)
{
	switch (dcc->type)
	{
	case TYPE_SEND:
		dcc_update_send (dcc);
		break;

	case TYPE_RECV:
		dcc_update_recv (dcc);
		break;

	default:
		dcc_update_chat (dcc);
	}

	if (dccfwin.window)
		update_clear_button_sensitivity();
}

void
fe_dcc_remove (struct DCC *dcc)
{
	switch (dcc->type)
	{
	case TYPE_SEND:
	case TYPE_RECV:
		if (dccfwin.window)
			fabulor_dcc_transfer_list_remove (dccfwin.transfers, dcc);
		break;

	default:	/* chat */
		if (dcccwin.window)
			fabulor_dcc_chat_list_remove (dcccwin.chats, dcc);
		break;
	}
}
