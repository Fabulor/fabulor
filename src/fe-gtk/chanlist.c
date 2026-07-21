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
#include <fcntl.h>
#include <time.h>

#ifdef WIN32
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
#include "../common/util.h"
#include "../common/fe.h"
#include "../common/server.h"
#include "../common/servlist.h"
#include "gtkutil.h"
#include "gtk-compat.h"
#include "maingui.h"
#include "menu.h"
#include "servlistgui.h"

#include "channel-list.h"

#if GTK_MAJOR_VERSION >= 4
#include "channel-list-context-menu-model.h"
#include "context-menu-presenter-gtk4.h"
#include "menu-action-namespaces.h"
#endif

#define ICON_CHANLIST_JOIN "zc-menu-join"
#define ICON_CHANLIST_COPY "zc-menu-copy"
#define ICON_CHANLIST_FIND "zc-menu-find"
#define ICON_CHANLIST_REFRESH "zc-menu-refresh"
#define ICON_CHANLIST_SAVE "zc-menu-save"

#ifndef CUSTOM_LIST
typedef struct	/* this is now in custom-list.h */
{
	char *topic;
	char *collation_key;
	guint32	pos;
	guint32 users;
	/* channel string lives beyond "users" */
#define GET_CHAN(row) (((char *)row)+sizeof(chanlistrow))
}
chanlistrow;
#endif

static gboolean
chanlist_append_row (server *serv, chanlistrow *row)
{
	FabulorChannelListSnapshot snapshot = {
		row,
		GET_CHAN (row),
		row->users,
		row->topic,
		row->collation_key
	};

	return fabulor_channel_list_append (serv->gui->chanlist_model, &snapshot);
}

static int
chanlist_normalize_width (int width)
{
	if (width < 1)
		return 1;

	return width;
}

static void
chanlist_set_label_alignment (GtkWidget *widget)
{
	gtk_widget_set_halign (widget, GTK_ALIGN_START);
	gtk_widget_set_valign (widget, GTK_ALIGN_CENTER);
}

static void
chanlist_grid_attach (GtkWidget *grid, GtkWidget *child,
					  gint column, gint row,
					  gint width, gint height,
					  gboolean hexpand, gboolean vexpand,
					  GtkAlign halign, GtkAlign valign)
{
	gtk_widget_set_hexpand (child, hexpand);
	gtk_widget_set_vexpand (child, vexpand);
	gtk_widget_set_halign (child, halign);
	gtk_widget_set_valign (child, valign);
	gtk_grid_attach (GTK_GRID (grid), child, column, row, width, height);
}

static GtkWidget *
chanlist_box_new (void)
{
	GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

	gtk_box_set_homogeneous (GTK_BOX (box), FALSE);
	return box;
}

static GtkWidget *
chanlist_icon_button (const char *label, const char *icon_name,
							 GCallback callback, gpointer userdata)
{
	GtkWidget *button;
	GtkWidget *image;

	button = gtk_button_new_with_mnemonic (label);
	image = gtkutil_image_new_from_stock (icon_name, FABULOR_GTK_ICON_SIZE_MENU);
	gtk_button_set_image (GTK_BUTTON (button), image);
	gtk_button_set_use_underline (GTK_BUTTON (button), TRUE);
	g_signal_connect (G_OBJECT (button), "clicked", callback, userdata);
	gtk_widget_show (button);

	return button;
}

#if GTK_MAJOR_VERSION < 4
static GtkWidget *
chanlist_icon_menu_item (const char *label, const char *icon_name,
								 GCallback callback, gpointer userdata)
{
	GtkWidget *item;
	GtkWidget *box;
	GtkWidget *image = NULL;
	GtkWidget *label_widget;

	item = gtk_menu_item_new ();
	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
	image = icon_name ? gtkutil_image_new_from_stock (icon_name, FABULOR_GTK_ICON_SIZE_MENU) : NULL;
	label_widget = gtk_label_new_with_mnemonic (label);
	if (image)
		gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), label_widget, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (item), box);
	g_signal_connect (G_OBJECT (item), "activate", callback, userdata);
	gtk_widget_show_all (item);

	return item;
}
#endif


static gboolean
chanlist_match (server *serv, const char *str)
{
	switch (serv->gui->chanlist_search_type)
	{
	case 1:
		return match (fabulor_gtk_entry_get_text (
			GTK_ENTRY (serv->gui->chanlist_wild)), str);
	case 2:
		if (!serv->gui->have_regex)
			return 0;

		return g_regex_match (serv->gui->chanlist_match_regex, str, 0, NULL);
	default:	/* case 0: */
		return nocasestrstr (str, fabulor_gtk_entry_get_text (
			GTK_ENTRY (serv->gui->chanlist_wild))) ? 1 : 0;
	}
}

/**
 * Updates the caption to reflect the number of users and channels
 */
static void
chanlist_update_caption (server *serv)
{
	gchar tbuf[256];

	g_snprintf (tbuf, sizeof tbuf,
				 _("Displaying %d/%d users on %d/%d channels."),
				 serv->gui->chanlist_users_shown_count,
				 serv->gui->chanlist_users_found_count,
				 serv->gui->chanlist_channels_shown_count,
				 serv->gui->chanlist_channels_found_count);

	gtk_label_set_text (GTK_LABEL (serv->gui->chanlist_label), tbuf);
	serv->gui->chanlist_caption_is_stale = FALSE;
}

static void
chanlist_update_buttons (server *serv)
{
	if (serv->gui->chanlist_channels_shown_count)
	{
		gtk_widget_set_sensitive (serv->gui->chanlist_join, TRUE);
		gtk_widget_set_sensitive (serv->gui->chanlist_savelist, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (serv->gui->chanlist_join, FALSE);
		gtk_widget_set_sensitive (serv->gui->chanlist_savelist, FALSE);
	}
}

static void
chanlist_reset_counters (server *serv)
{
	serv->gui->chanlist_users_found_count = 0;
	serv->gui->chanlist_users_shown_count = 0;
	serv->gui->chanlist_channels_found_count = 0;
	serv->gui->chanlist_channels_shown_count = 0;

	chanlist_update_caption (serv);
	chanlist_update_buttons (serv);
}

/* free up our entire linked list and all the nodes */

static void
chanlist_data_free (server *serv)
{
	GSList *rows;
	chanlistrow *data;

	if (serv->gui->chanlist_data_stored_rows)
	{
		for (rows = serv->gui->chanlist_data_stored_rows; rows != NULL;
			  rows = rows->next)
		{
			data = rows->data;
			g_free (data->topic);
			g_free (data->collation_key);
			g_free (data);
		}

		g_slist_free (serv->gui->chanlist_data_stored_rows);
		serv->gui->chanlist_data_stored_rows = NULL;
	}

	g_slist_free (serv->gui->chanlist_pending_rows);
	serv->gui->chanlist_pending_rows = NULL;
}

/* add any rows we received from the server in the last 0.25s to the GUI */

static void
chanlist_flush_pending (server *serv)
{
	GSList *list = serv->gui->chanlist_pending_rows;
	chanlistrow *row;

	if (!list)
	{
		if (serv->gui->chanlist_caption_is_stale)
			chanlist_update_caption (serv);
		return;
	}
	while (list)
	{
		row = list->data;
		chanlist_append_row (serv, row);
		list = list->next;
	}

	g_slist_free (serv->gui->chanlist_pending_rows);
	serv->gui->chanlist_pending_rows = NULL;
	chanlist_update_caption (serv);
}

static gboolean
chanlist_timeout (server *serv)
{
	chanlist_flush_pending (serv);
	return TRUE;
}

/**
 * Places a data row into the GUI list if and only if the row matches
 * the user and regex/search requirements.
 */
static void
chanlist_place_row_in_gui (server *serv, chanlistrow *next_row, gboolean force)
{
	/* First, update the 'found' counter values */
	serv->gui->chanlist_users_found_count += next_row->users;
	serv->gui->chanlist_channels_found_count++;

	if (serv->gui->chanlist_channels_shown_count == 1)
		/* join & save buttons become live */
		chanlist_update_buttons (serv);

	if (next_row->users < serv->gui->chanlist_minusers)
	{
		serv->gui->chanlist_caption_is_stale = TRUE;
		return;
	}

	if (next_row->users > serv->gui->chanlist_maxusers
		 && serv->gui->chanlist_maxusers > 0)
	{
		serv->gui->chanlist_caption_is_stale = TRUE;
		return;
	}

	if (fabulor_gtk_entry_get_text (
		GTK_ENTRY (serv->gui->chanlist_wild))[0])
	{
		/* Check what the user wants to match. If both buttons or _neither_
		 * button is checked, look for match in both by default. 
		 */
		if (serv->gui->chanlist_match_wants_channel ==
			 serv->gui->chanlist_match_wants_topic)
		{
			if (!chanlist_match (serv, GET_CHAN (next_row))
				 && !chanlist_match (serv, next_row->topic))
			{
				serv->gui->chanlist_caption_is_stale = TRUE;
				return;
			}
		}

		else if (serv->gui->chanlist_match_wants_channel)
		{
			if (!chanlist_match (serv, GET_CHAN (next_row)))
			{
				serv->gui->chanlist_caption_is_stale = TRUE;
				return;
			}
		}

		else if (serv->gui->chanlist_match_wants_topic)
		{
			if (!chanlist_match (serv, next_row->topic))
			{
				serv->gui->chanlist_caption_is_stale = TRUE;
				return;
			}
		}
	}

	if (force || serv->gui->chanlist_channels_shown_count < 20)
	{
		/* makes it appear fast :) */
		chanlist_append_row (serv, next_row);
		chanlist_update_caption (serv);
	}
	else
		/* add it to GUI at the next update interval */
		serv->gui->chanlist_pending_rows = g_slist_prepend (serv->gui->chanlist_pending_rows, next_row);

	/* Update the 'shown' counter values */
	serv->gui->chanlist_users_shown_count += next_row->users;
	serv->gui->chanlist_channels_shown_count++;
}

/* Performs the LIST download from the IRC server. */

static void
chanlist_do_refresh (server *serv)
{
	if (serv->gui->chanlist_flash_tag)
	{
		g_source_remove (serv->gui->chanlist_flash_tag);
		serv->gui->chanlist_flash_tag = 0;
	}

	if (!serv->connected)
	{
		fe_message (_("Not connected."), FE_MSG_ERROR);
		return;
	}

	fabulor_channel_list_clear (serv->gui->chanlist_model);
	gtk_widget_set_sensitive (serv->gui->chanlist_refresh, FALSE);

	chanlist_data_free (serv);
	chanlist_reset_counters (serv);

	/* can we request a list with minusers arg? */
	if (serv->use_listargs)
	{
		/* yes - it will download faster */
		serv->p_list_channels (serv, "", serv->gui->chanlist_minusers);
		/* don't allow the spin button below this value from now on */
		serv->gui->chanlist_minusers_downloaded = serv->gui->chanlist_minusers;
	}
	else
	{
		/* download all, filter minusers locally only */
		serv->p_list_channels (serv, "", 1);
		serv->gui->chanlist_minusers_downloaded = 1;
	}

/*	gtk_spin_button_set_range ((GtkSpinButton *)serv->gui->chanlist_min_spin,
										serv->gui->chanlist_minusers_downloaded, 999999);*/
}

static void
chanlist_refresh (GtkWidget * wid, server *serv)
{
	chanlist_do_refresh (serv);
}

/**
 * Fills the GUI list with stored items from the GSList.
 */
static void
chanlist_build_gui_list (server *serv)
{
	GSList *rows;

	/* first check if the list is present */
	if (serv->gui->chanlist_data_stored_rows == NULL)
	{
		/* start a download */
		chanlist_do_refresh (serv);
		return;
	}

	fabulor_channel_list_clear (serv->gui->chanlist_model);

	/* discard pending rows FIXME: free the structs? */
	g_slist_free (serv->gui->chanlist_pending_rows);
	serv->gui->chanlist_pending_rows = NULL;

	/* Reset the counters */
	chanlist_reset_counters (serv);

	/* Refill the list */
	for (rows = serv->gui->chanlist_data_stored_rows; rows != NULL;
		  rows = rows->next)
	{
		chanlist_place_row_in_gui (serv, rows->data, TRUE);
	}

	fabulor_channel_list_resort (serv->gui->chanlist_model);
}

/**
 * Accepts incoming channel data from inbound.c, allocates new space for a
 * chanlistrow, adds it to our linked list and calls chanlist_place_row_in_gui.
 */
void
fe_add_chan_list (server *serv, char *chan, char *users, char *topic)
{
	chanlistrow *next_row;
	size_t len = strlen (chan) + 1;

	/* we allocate the struct and channel string in one go */
	next_row = g_malloc (sizeof (chanlistrow) + len);
	memcpy (((char *)next_row) + sizeof (chanlistrow), chan, len);
	next_row->topic = strip_color (topic, -1, STRIP_ALL);
	next_row->collation_key = g_utf8_collate_key (chan, len-1);
	if (!(next_row->collation_key))
		next_row->collation_key = g_strdup (chan);
	next_row->users = atoi (users);

	/* add this row to the data */
	serv->gui->chanlist_data_stored_rows =
		g_slist_prepend (serv->gui->chanlist_data_stored_rows, next_row);

	/* _possibly_ add the row to the gui */
	chanlist_place_row_in_gui (serv, next_row, FALSE);
}

void
fe_chan_list_end (server *serv)
{
	/* download complete */
	chanlist_flush_pending (serv);
	gtk_widget_set_sensitive (serv->gui->chanlist_refresh, TRUE);
	fabulor_channel_list_resort (serv->gui->chanlist_model);
}

static void
chanlist_search_pressed (GtkButton * button, server *serv)
{
	chanlist_build_gui_list (serv);
}

static void
chanlist_find_cb (GtkWidget * wid, server *serv)
{
	const char *pattern = fabulor_gtk_entry_get_text (GTK_ENTRY (wid));

	/* recompile the regular expression. */
	if (serv->gui->have_regex)
	{
		serv->gui->have_regex = 0;
		g_regex_unref (serv->gui->chanlist_match_regex);
	}

	serv->gui->chanlist_match_regex = g_regex_new (pattern, G_REGEX_CASELESS | G_REGEX_EXTENDED,
												G_REGEX_MATCH_NOTBOL, NULL);

	if (serv->gui->chanlist_match_regex)
		serv->gui->have_regex = 1;
}

static void
chanlist_match_channel_button_toggled (GtkWidget * wid, server *serv)
{
	serv->gui->chanlist_match_wants_channel = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wid));
}

static void
chanlist_match_topic_button_toggled (GtkWidget * wid, server *serv)
{
	serv->gui->chanlist_match_wants_topic = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wid));
}

static void
chanlist_join (GtkWidget * wid, server *serv)
{
	char tbuf[CHANLEN + 6];
	GPtrArray *selection;
	guint i;
	gboolean joined = FALSE;

	selection = fabulor_channel_list_dup_selected_text (
		serv->gui->chanlist_model, FABULOR_CHANNEL_LIST_CHANNEL);
	for (i = 0; i < selection->len; i++)
	{
		char *chan = g_ptr_array_index (selection, i);

		if (serv->connected && strcmp (chan, "*") != 0)
		{
			g_snprintf (tbuf, sizeof (tbuf), "join %s", chan);
			handle_command (serv->server_session, tbuf, FALSE);
			joined = TRUE;
		}
	}

	if (!joined && selection->len)
		gdk_display_beep (gdk_display_get_default ());

	g_ptr_array_unref (selection);
}

static void
chanlist_activate (gpointer data)
{
	chanlist_join (NULL, data);
}

static void
chanlist_filereq_done (server *serv, char *file)
{
	time_t t = time (0);
	int fh;
	char buf[1024];
	GPtrArray *rows;
	guint i;

	if (!file)
		return;

	fh = zoitechat_open_file (file, O_TRUNC | O_WRONLY | O_CREAT, 0600,
								 XOF_DOMODE | XOF_FULLPATH);
	if (fh == -1)
		return;

	g_snprintf (buf, sizeof buf, "Fabulor Channel List: %s - %s\n",
				 serv->servername, ctime (&t));
	write (fh, buf, strlen (buf));

	rows = fabulor_channel_list_dup_all (serv->gui->chanlist_model);
	for (i = 0; i < rows->len; i++)
	{
		FabulorChannelListRecord *row = g_ptr_array_index (rows, i);
		g_snprintf (buf, sizeof buf, "%-16s %-5u%s\n", row->channel,
			row->users, row->topic);
		write (fh, buf, strlen (buf));
	}
	g_ptr_array_unref (rows);

	close (fh);
}

static void
chanlist_save (GtkWidget * wid, server *serv)
{
	if (fabulor_channel_list_get_n_rows (serv->gui->chanlist_model) > 0)
		gtkutil_file_req (NULL, _("Select an output filename"), chanlist_filereq_done,
								serv, NULL, NULL, FRF_WRITE);
}

static gboolean
chanlist_flash (server *serv)
{
	if (!(gtk_widget_get_state_flags (serv->gui->chanlist_refresh) & GTK_STATE_FLAG_ACTIVE))
	{
		gtk_widget_unset_state_flags (serv->gui->chanlist_refresh, GTK_STATE_FLAG_PRELIGHT);
		gtk_widget_set_state_flags (serv->gui->chanlist_refresh, GTK_STATE_FLAG_ACTIVE, FALSE);
	}
	else
	{
		gtk_widget_unset_state_flags (serv->gui->chanlist_refresh, GTK_STATE_FLAG_ACTIVE);
		gtk_widget_set_state_flags (serv->gui->chanlist_refresh, GTK_STATE_FLAG_PRELIGHT, FALSE);
	}

	return TRUE;
}

static void
chanlist_minusers (GtkSpinButton *wid, server *serv)
{
	serv->gui->chanlist_minusers = gtk_spin_button_get_value_as_int (wid);
	prefs.hex_gui_chanlist_minusers = serv->gui->chanlist_minusers;
	if (!save_config ())
		fe_message (_("Could not save fabulor.conf."), FE_MSG_WARN);

	if (serv->gui->chanlist_minusers < serv->gui->chanlist_minusers_downloaded)
	{
		if (serv->gui->chanlist_flash_tag == 0)
			serv->gui->chanlist_flash_tag = g_timeout_add (500, (GSourceFunc)chanlist_flash, serv);
	}
	else
	{
		if (serv->gui->chanlist_flash_tag)
		{
			g_source_remove (serv->gui->chanlist_flash_tag);
			serv->gui->chanlist_flash_tag = 0;
		}
	}
}

static void
chanlist_maxusers (GtkSpinButton *wid, server *serv)
{
	serv->gui->chanlist_maxusers = gtk_spin_button_get_value_as_int (wid);
	prefs.hex_gui_chanlist_maxusers = serv->gui->chanlist_maxusers;
	if (!save_config ())
		fe_message (_("Could not save fabulor.conf."), FE_MSG_WARN);
}

#if GTK_MAJOR_VERSION < 4
static void
chanlist_menu_destroy (GtkWidget *menu, gpointer userdata)
{
	gtk_widget_destroy (menu);
	g_object_unref (menu);
}

static void
chanlist_copychannel (GtkWidget *item, server *serv)
{
	GPtrArray *selection = fabulor_channel_list_dup_selected_text (
		serv->gui->chanlist_model, FABULOR_CHANNEL_LIST_CHANNEL);
	GString *text;
	guint i;

	if (!selection->len)
	{
		g_ptr_array_unref (selection);
		return;
	}

	text = g_string_new ("");
	for (i = 0; i < selection->len; i++)
	{
		if (text->len)
			g_string_append_c (text, '\n');
		g_string_append (text, g_ptr_array_index (selection, i));
	}

	gtkutil_copy_to_clipboard (item, text->str);
	g_string_free (text, TRUE);
	g_ptr_array_unref (selection);
}

static void
chanlist_copytopic (GtkWidget *item, server *serv)
{
	GPtrArray *selection = fabulor_channel_list_dup_selected_text (
		serv->gui->chanlist_model, FABULOR_CHANNEL_LIST_TOPIC);
	GString *text;
	guint i;

	if (!selection->len)
	{
		g_ptr_array_unref (selection);
		return;
	}

	text = g_string_new ("");
	for (i = 0; i < selection->len; i++)
	{
		if (text->len)
			g_string_append_c (text, '\n');
		g_string_append (text, g_ptr_array_index (selection, i));
	}

	gtkutil_copy_to_clipboard (item, text->str);
	g_string_free (text, TRUE);
	g_ptr_array_unref (selection);
}
#else
#define FABULOR_CHANNEL_LIST_CONTEXT_POPUP \
	"fabulor-channel-list-context-popup"

typedef struct
{
	FabulorChannelListContextMenuModel *model;
	FabulorContextMenuPresenterGtk4 *presenter;
	server *serv;
	GtkWidget *origin;
} FabulorChannelListContextPopup;

static void
chanlist_context_popup_free (gpointer data)
{
	FabulorChannelListContextPopup *popup = data;

	if (!popup)
		return;
	fabulor_context_menu_presenter_gtk4_free (popup->presenter);
	fabulor_channel_list_context_menu_model_free (popup->model);
	g_free (popup);
}

static char *
chanlist_context_join_lines (const GPtrArray *values)
{
	GString *text = g_string_new ("");
	guint i;

	for (i = 0; values && i < values->len; i++)
	{
		if (text->len)
			g_string_append_c (text, '\n');
		g_string_append (text, g_ptr_array_index (values, i));
	}
	return g_string_free (text, FALSE);
}

static void
chanlist_context_dispatch (FabulorChannelListContextAction action,
	gboolean state, const GPtrArray *channels, const GPtrArray *topics,
	gpointer user_data)
{
	FabulorChannelListContextPopup *popup = user_data;
	guint i;

	if (action == FABULOR_CHANNEL_LIST_CONTEXT_JOIN)
	{
		gboolean joined = FALSE;

		for (i = 0; i < channels->len; i++)
		{
			const char *channel = g_ptr_array_index (channels, i);
			char command[CHANLEN + 6];

			if (popup->serv->connected && strcmp (channel, "*") != 0)
			{
				g_snprintf (command, sizeof command, "join %s", channel);
				handle_command (popup->serv->server_session, command, FALSE);
				joined = TRUE;
			}
		}
		if (!joined && channels->len)
			gdk_display_beep (gdk_display_get_default ());
		return;
	}
	if (action == FABULOR_CHANNEL_LIST_CONTEXT_COPY_CHANNELS ||
		action == FABULOR_CHANNEL_LIST_CONTEXT_COPY_TOPICS)
	{
		char *text = chanlist_context_join_lines (
			action == FABULOR_CHANNEL_LIST_CONTEXT_COPY_CHANNELS ?
			channels : topics);

		if (*text)
			gtkutil_copy_to_clipboard (popup->origin, text);
		g_free (text);
		return;
	}
	if (action == FABULOR_CHANNEL_LIST_CONTEXT_AUTOJOIN &&
		popup->serv->network && channels->len)
		servlist_autojoinedit (popup->serv->network,
			g_ptr_array_index (channels, 0), state);
}

static gboolean
chanlist_context_popup (server *serv, GtkWidget *origin, gdouble x, gdouble y)
{
	FabulorChannelListContextLabels labels = {
		_("_Join Channel"), _("_Copy Channel Name"),
		_("Copy _Topic Text"), _("Autojoin Channel"),
		ICON_CHANLIST_JOIN, ICON_CHANLIST_COPY
	};
	FabulorChannelListContextPopup *popup;
	GPtrArray *channels;
	GPtrArray *topics;
	gboolean has_network;
	gboolean autojoin;

	channels = fabulor_channel_list_dup_selected_text (
		serv->gui->chanlist_model, FABULOR_CHANNEL_LIST_CHANNEL);
	topics = fabulor_channel_list_dup_selected_text (
		serv->gui->chanlist_model, FABULOR_CHANNEL_LIST_TOPIC);
	if (!channels->len)
	{
		g_ptr_array_unref (topics);
		g_ptr_array_unref (channels);
		return FALSE;
	}
	has_network = serv->network != NULL;
	autojoin = has_network && joinlist_is_in_list (serv,
		g_ptr_array_index (channels, 0));
	popup = g_new0 (FabulorChannelListContextPopup, 1);
	popup->serv = serv;
	popup->origin = origin;
	popup->model = fabulor_channel_list_context_menu_model_new (channels,
		topics, has_network, autojoin, &labels, chanlist_context_dispatch,
		popup);
	g_ptr_array_unref (topics);
	g_ptr_array_unref (channels);
	if (!popup->model)
	{
		chanlist_context_popup_free (popup);
		return FALSE;
	}
	popup->presenter =
		fabulor_context_menu_presenter_gtk4_new_with_namespaces (
			fabulor_channel_list_context_menu_model_get_menu (popup->model),
			FABULOR_CONTEXT_ACTION_NAMESPACE,
			fabulor_channel_list_context_menu_model_get_actions (popup->model),
			NULL, NULL);
	if (!popup->presenter)
	{
		chanlist_context_popup_free (popup);
		return FALSE;
	}
	g_object_set_data_full (G_OBJECT (origin),
		FABULOR_CHANNEL_LIST_CONTEXT_POPUP, popup,
		chanlist_context_popup_free);
	return fabulor_context_menu_presenter_gtk4_popup_at (popup->presenter,
		origin, x, y);
}
#endif

static gboolean
chanlist_button_cb (GtkWidget *view, guint button, guint n_press,
	gdouble x, gdouble y, GdkModifierType state, gpointer data)
{
	server *serv = data;

#if GTK_MAJOR_VERSION < 4
	GtkWidget *menu;
	char *chan;
#endif

	(void) view;
	(void) n_press;
	(void) state;
	if (button != 3 || !fabulor_channel_list_select_at_point (
		serv->gui->chanlist_model, x, y))
		return FALSE;

#if GTK_MAJOR_VERSION >= 4
	return chanlist_context_popup (serv, view, x, y);
#else
	menu = gtk_menu_new ();
	g_object_ref (menu);
	g_object_ref_sink (menu);
	g_object_unref (menu);
	g_signal_connect (G_OBJECT (menu), "selection-done",
							G_CALLBACK (chanlist_menu_destroy), NULL);
	{
		GtkWidget *item;

		item = chanlist_icon_menu_item (_("_Join Channel"), ICON_CHANLIST_JOIN,
												G_CALLBACK (chanlist_join), serv);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

		item = chanlist_icon_menu_item (_("_Copy Channel Name"), ICON_CHANLIST_COPY,
												G_CALLBACK (chanlist_copychannel), serv);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

		item = chanlist_icon_menu_item (_("Copy _Topic Text"), ICON_CHANLIST_COPY,
												G_CALLBACK (chanlist_copytopic), serv);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	}

	chan = fabulor_channel_list_dup_first_selected_channel (
		serv->gui->chanlist_model);
	menu_addfavoritemenu (serv, menu, chan, FALSE);
	g_free (chan);

	gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);

	return TRUE;
#endif
}

static void
chanlist_destroy_widget (GtkWidget *wid, server *serv)
{
	int channel_width, users_width, topic_width;

	fabulor_channel_list_get_column_widths (serv->gui->chanlist_model,
		&channel_width, &users_width, &topic_width);
	prefs.hex_gui_chanlist_width_channel =
		chanlist_normalize_width (channel_width);
	prefs.hex_gui_chanlist_width_users = chanlist_normalize_width (users_width);
	prefs.hex_gui_chanlist_width_topic = chanlist_normalize_width (topic_width);

	if (!save_config ())
		fe_message (_("Could not save fabulor.conf."), FE_MSG_WARN);

	fabulor_channel_list_clear (serv->gui->chanlist_model);
	chanlist_data_free (serv);
	fabulor_channel_list_free (serv->gui->chanlist_model);
	serv->gui->chanlist_model = NULL;
	serv->gui->chanlist_list = NULL;

	if (serv->gui->chanlist_flash_tag)
	{
		g_source_remove (serv->gui->chanlist_flash_tag);
		serv->gui->chanlist_flash_tag = 0;
	}

	if (serv->gui->chanlist_tag)
	{
		g_source_remove (serv->gui->chanlist_tag);
		serv->gui->chanlist_tag = 0;
	}

	if (serv->gui->have_regex)
	{
		g_regex_unref (serv->gui->chanlist_match_regex);
		serv->gui->have_regex = 0;
	}
}

static void
chanlist_closegui (GtkWidget *wid, server *serv)
{
	if (is_server (serv))
		serv->gui->chanlist_window = NULL;
}

static void
chanlist_combo_cb (GtkWidget *combo, server *serv)
{
	serv->gui->chanlist_search_type = gtk_combo_box_get_active (GTK_COMBO_BOX (combo));
}

void
chanlist_opengui (server *serv, int do_refresh)
{
	GtkWidget *vbox, *hbox, *table, *wid, *view;
	char tbuf[256];

	if (serv->gui->chanlist_window)
	{
		mg_bring_tofront (serv->gui->chanlist_window);
		return;
	}

	g_snprintf (tbuf, sizeof tbuf, _("Channel List (%s) - %s"),
				 server_get_network (serv, TRUE), _(DISPLAY_NAME));

	serv->gui->chanlist_pending_rows = NULL;
	serv->gui->chanlist_tag = 0;
	serv->gui->chanlist_flash_tag = 0;
	serv->gui->chanlist_data_stored_rows = NULL;

	if (!serv->gui->chanlist_minusers)
	{
		if (prefs.hex_gui_chanlist_minusers < 1 || prefs.hex_gui_chanlist_minusers > 999999)
		{
			prefs.hex_gui_chanlist_minusers = 5;
			if (!save_config ())
				fe_message (_("Could not save fabulor.conf."), FE_MSG_WARN);
		}

		serv->gui->chanlist_minusers = prefs.hex_gui_chanlist_minusers;
	}

	if (!serv->gui->chanlist_maxusers)
	{
		if (prefs.hex_gui_chanlist_maxusers < 1 || prefs.hex_gui_chanlist_maxusers > 999999)
		{
			prefs.hex_gui_chanlist_maxusers = 9999;
			if (!save_config ())
				fe_message (_("Could not save fabulor.conf."), FE_MSG_WARN);
		}

		serv->gui->chanlist_maxusers = prefs.hex_gui_chanlist_maxusers;
	}

	serv->gui->chanlist_window =
		mg_create_generic_tab ("ChanList", tbuf, FALSE, TRUE, chanlist_closegui,
								serv, 640, 480, &vbox, serv);
	gtkutil_destroy_on_esc (serv->gui->chanlist_window);

	fabulor_gtk_container_set_uniform_inset (vbox, 6);
	gtk_box_set_spacing (GTK_BOX (vbox), 12);

	/* make a label to store the user/channel info */
	wid = gtk_label_new (NULL);
	fabulor_gtk_box_append (GTK_BOX (vbox), wid, FALSE, FALSE, 0);
	gtk_widget_show (wid);
	serv->gui->chanlist_label = wid;

	/* ============================================================= */

	serv->gui->chanlist_model = fabulor_channel_list_new (
		chanlist_activate, serv);
	if (!serv->gui->chanlist_model)
	{
		gtk_widget_destroy (serv->gui->chanlist_window);
		return;
	}
	view = fabulor_channel_list_create_view (serv->gui->chanlist_model,
		GTK_BOX (vbox), _("Channel"), _("Users"), _("Topic"),
		prefs.hex_gui_chanlist_width_channel > 0 ?
			chanlist_normalize_width (prefs.hex_gui_chanlist_width_channel) : 0,
		prefs.hex_gui_chanlist_width_users > 0 ?
			chanlist_normalize_width (prefs.hex_gui_chanlist_width_users) : 0,
		prefs.hex_gui_chanlist_width_topic > 0 ?
			chanlist_normalize_width (prefs.hex_gui_chanlist_width_topic) : 0);
	if (!view)
	{
		fabulor_channel_list_free (serv->gui->chanlist_model);
		serv->gui->chanlist_model = NULL;
		gtk_widget_destroy (serv->gui->chanlist_window);
		return;
	}
	serv->gui->chanlist_list = view;
	fabulor_gtk_widget_on_multi_click (view, chanlist_button_cb, serv);

	/* ============================================================= */

	table = gtk_grid_new ();
	gtk_grid_set_column_spacing (GTK_GRID (table), 12);
	gtk_grid_set_row_spacing (GTK_GRID (table), 3);
	fabulor_gtk_box_append (GTK_BOX (vbox), table, FALSE, TRUE, 0);
	gtk_widget_show (table);

	wid = chanlist_icon_button (_("_Search"), ICON_CHANLIST_FIND,
										 G_CALLBACK (chanlist_search_pressed), serv);
	serv->gui->chanlist_search = wid;
	chanlist_grid_attach (table, wid, 3, 3, 1, 1, FALSE, FALSE,
						  GTK_ALIGN_FILL, GTK_ALIGN_FILL);

	wid = chanlist_icon_button (_("_Download List"), ICON_CHANLIST_REFRESH,
										 G_CALLBACK (chanlist_refresh), serv);
	serv->gui->chanlist_refresh = wid;
	chanlist_grid_attach (table, wid, 3, 2, 1, 1, FALSE, FALSE,
						  GTK_ALIGN_FILL, GTK_ALIGN_FILL);

	wid = chanlist_icon_button (_("Save _List..."), ICON_CHANLIST_SAVE,
										 G_CALLBACK (chanlist_save), serv);
	serv->gui->chanlist_savelist = wid;
	chanlist_grid_attach (table, wid, 3, 1, 1, 1, FALSE, FALSE,
						  GTK_ALIGN_FILL, GTK_ALIGN_FILL);

	wid = chanlist_icon_button (_("_Join Channel"), ICON_CHANLIST_JOIN,
										 G_CALLBACK (chanlist_join), serv);
	serv->gui->chanlist_join = wid;
	chanlist_grid_attach (table, wid, 3, 0, 1, 1, FALSE, FALSE,
						  GTK_ALIGN_FILL, GTK_ALIGN_FILL);

	/* ============================================================= */

	wid = gtk_label_new (_("Show only:"));
	chanlist_set_label_alignment (wid);
	chanlist_grid_attach (table, wid, 0, 3, 1, 1, FALSE, FALSE,
						  GTK_ALIGN_START, GTK_ALIGN_CENTER);
	gtk_widget_show (wid);

	hbox = chanlist_box_new ();
	gtk_box_set_spacing (GTK_BOX (hbox), 9);
	chanlist_grid_attach (table, hbox, 1, 3, 1, 1, FALSE, FALSE,
						  GTK_ALIGN_FILL, GTK_ALIGN_FILL);
	gtk_widget_show (hbox);

	wid = gtk_label_new (_("channels with"));
	fabulor_gtk_box_append (GTK_BOX (hbox), wid, FALSE, FALSE, 0);
	gtk_widget_show (wid);

	wid = gtk_spin_button_new_with_range (1, 999999, 1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (wid),
										serv->gui->chanlist_minusers);
	g_signal_connect (G_OBJECT (wid), "value-changed",
							G_CALLBACK (chanlist_minusers), serv);
	fabulor_gtk_box_append (GTK_BOX (hbox), wid, FALSE, FALSE, 0);
	gtk_widget_show (wid);
	serv->gui->chanlist_min_spin = wid;

	wid = gtk_label_new (_("to"));
	fabulor_gtk_box_append (GTK_BOX (hbox), wid, FALSE, FALSE, 0);
	gtk_widget_show (wid);

	wid = gtk_spin_button_new_with_range (1, 999999, 1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (wid),
										serv->gui->chanlist_maxusers);
	g_signal_connect (G_OBJECT (wid), "value-changed",
							G_CALLBACK (chanlist_maxusers), serv);
	fabulor_gtk_box_append (GTK_BOX (hbox), wid, FALSE, FALSE, 0);
	gtk_widget_show (wid);

	wid = gtk_label_new (_("users."));
	fabulor_gtk_box_append (GTK_BOX (hbox), wid, FALSE, FALSE, 0);
	gtk_widget_show (wid);

	/* ============================================================= */

	wid = gtk_label_new (_("Look in:"));
	chanlist_set_label_alignment (wid);
	chanlist_grid_attach (table, wid, 0, 2, 1, 1, FALSE, FALSE,
						  GTK_ALIGN_START, GTK_ALIGN_CENTER);
	gtk_widget_show (wid);

	hbox = chanlist_box_new ();
	gtk_box_set_spacing (GTK_BOX (hbox), 12);
	chanlist_grid_attach (table, hbox, 1, 2, 1, 1, FALSE, FALSE,
						  GTK_ALIGN_FILL, GTK_ALIGN_FILL);
	gtk_widget_show (hbox);

	wid = gtk_check_button_new_with_label (_("Channel name"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wid), TRUE);
	g_signal_connect (G_OBJECT (wid), "toggled",
							  G_CALLBACK(chanlist_match_channel_button_toggled), serv);
	fabulor_gtk_box_append (GTK_BOX (hbox), wid, FALSE, FALSE, 0);
	gtk_widget_show (wid);

	wid = gtk_check_button_new_with_label (_("Topic"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wid), TRUE);
	g_signal_connect (G_OBJECT (wid), "toggled",
							  G_CALLBACK (chanlist_match_topic_button_toggled),
							  serv);
	fabulor_gtk_box_append (GTK_BOX (hbox), wid, FALSE, FALSE, 0);
	gtk_widget_show (wid);

	serv->gui->chanlist_match_wants_channel = 1;
	serv->gui->chanlist_match_wants_topic = 1;

	/* ============================================================= */

	wid = gtk_label_new (_("Search type:"));
	chanlist_set_label_alignment (wid);
	chanlist_grid_attach (table, wid, 0, 1, 1, 1, FALSE, FALSE,
						  GTK_ALIGN_START, GTK_ALIGN_CENTER);
	gtk_widget_show (wid);

	wid = gtk_combo_box_text_new ();
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (wid), _("Simple Search"));
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (wid), _("Pattern Match (Wildcards)"));
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (wid), _("Regular Expression"));
	gtk_combo_box_set_active (GTK_COMBO_BOX (wid), serv->gui->chanlist_search_type);
	chanlist_grid_attach (table, wid, 1, 1, 1, 1, FALSE, FALSE,
						  GTK_ALIGN_FILL, GTK_ALIGN_CENTER);
	g_signal_connect (G_OBJECT (wid), "changed",
							G_CALLBACK (chanlist_combo_cb), serv);
	gtk_widget_show (wid);

	/* ============================================================= */

	wid = gtk_label_new (_("Find:"));
	chanlist_set_label_alignment (wid);
	chanlist_grid_attach (table, wid, 0, 0, 1, 1, FALSE, FALSE,
						  GTK_ALIGN_START, GTK_ALIGN_CENTER);
	gtk_widget_show (wid);

	wid = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY(wid), 255);
	g_signal_connect (G_OBJECT (wid), "changed",
							  G_CALLBACK (chanlist_find_cb), serv);
	g_signal_connect (G_OBJECT (wid), "activate",
							  G_CALLBACK (chanlist_search_pressed),
							  (gpointer) serv);
	chanlist_grid_attach (table, wid, 1, 0, 1, 1, TRUE, FALSE,
						  GTK_ALIGN_FILL, GTK_ALIGN_CENTER);
	gtk_widget_show (wid);
	serv->gui->chanlist_wild = wid;

	chanlist_find_cb (wid, serv);

	/* ============================================================= */

	wid = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
	chanlist_grid_attach (table, wid, 2, 0, 1, 5, FALSE, FALSE,
						  GTK_ALIGN_FILL, GTK_ALIGN_FILL);
	gtk_widget_show (wid);

	g_signal_connect (G_OBJECT (serv->gui->chanlist_window), "destroy",
							G_CALLBACK (chanlist_destroy_widget), serv);

	/* reset the counters. */
	chanlist_reset_counters (serv);

	serv->gui->chanlist_tag = g_timeout_add (250, (GSourceFunc)chanlist_timeout, serv);

	if (do_refresh)
		chanlist_do_refresh (serv);

	chanlist_update_buttons (serv);
	gtk_widget_show (serv->gui->chanlist_window);
	gtk_widget_grab_focus (serv->gui->chanlist_refresh);
}
