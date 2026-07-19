/* X-Chat
 * Copyright (C) 1998 Peter Zelezny.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>

#include "fe-gtk.h"
#include "theme/theme-manager.h"

#include "../common/zoitechat.h"
#include "../common/ignore.h"
#include "../common/fe.h"
#include "gtkutil.h"
#include "gtk-compat.h"
#include "ignore-list.h"
#include "maingui.h"

#define ICON_IGNORE_NEW "document-new"
#define ICON_IGNORE_DELETE "edit-delete"
#define ICON_IGNORE_CLEAR "edit-clear"

static GtkWidget *ignorewin;
static FabulorIgnoreList *ignore_view;
static GtkWidget *num_ctcp;
static GtkWidget *num_priv;
static GtkWidget *num_chan;
static GtkWidget *num_noti;
static GtkWidget *num_invi;

static gboolean
ignore_mask_renamed (const gchar *old_mask, const gchar *new_mask, guint flags,
	gpointer user_data)
{
	(void) user_data;
	if (ignore_exists ((char *) new_mask))
	{
		fe_message (_("That mask already exists."), FE_MSG_ERROR);
		return FALSE;
	}
	ignore_del ((char *) old_mask, NULL);
	ignore_add ((char *) new_mask, (int) flags, TRUE);
	return TRUE;
}

static void
ignore_flags_changed (const gchar *mask, guint flags, gpointer user_data)
{
	(void) user_data;
	if (ignore_add ((char *) mask, (int) flags, TRUE) != 2)
		g_warning ("ignore list view is out of sync!\n");
}

static void
ignore_delete_entry_clicked (GtkWidget *widget, gpointer user_data)
{
	gchar *mask;

	(void) widget;
	(void) user_data;
	mask = ignore_view ? fabulor_ignore_list_remove_selected (ignore_view) : NULL;
	if (mask)
	{
		ignore_del (mask, NULL);
		g_free (mask);
	}
}

static void
ignore_store_new (int cancel, char *mask, gpointer data)
{
	int flags = IG_CHAN | IG_PRIV | IG_NOTI | IG_CTCP | IG_DCC | IG_INVI;

	(void) data;
	if (cancel)
		return;
	if (ignore_exists (mask))
	{
		fe_message (_("That mask already exists."), FE_MSG_ERROR);
		return;
	}
	ignore_add (mask, flags, TRUE);
	fabulor_ignore_list_append (ignore_view, mask, (guint) flags, TRUE);
}

static void
ignore_clear_cb (GtkDialog *dialog, gint response, gpointer user_data)
{
	FabulorIgnoreList *list = user_data;

	fabulor_gtk_window_destroy (GTK_WINDOW (dialog));
	if (response == GTK_RESPONSE_OK)
	{
		GPtrArray *masks = fabulor_ignore_list_dup_masks (list);
		guint i;

		for (i = 0; i < masks->len; i++)
			ignore_del (g_ptr_array_index (masks, i), NULL);
		g_ptr_array_unref (masks);
		fabulor_ignore_list_clear (list);
	}
}

static void
ignore_clear_entry_clicked (GtkWidget *widget, gpointer user_data)
{
	GtkWidget *dialog;

	(void) widget;
	dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_OK_CANCEL, _("Are you sure you want to remove all ignores?"));
	theme_manager_attach_window (dialog);
	g_signal_connect (dialog, "response", G_CALLBACK (ignore_clear_cb),
		user_data);
	fabulor_gtk_window_position_at_pointer (GTK_WINDOW (dialog));
	gtk_widget_show (dialog);
}

static void
ignore_new_entry_clicked (GtkWidget *widget, gpointer user_data)
{
	(void) widget;
	(void) user_data;
	fe_get_str (_("Enter mask to ignore:"), "nick!userid@host.com",
		ignore_store_new, NULL);
}

static void
close_ignore_gui_callback (void)
{
	ignore_save ();
	ignorewin = NULL;
	ignore_view = NULL;
}

static GtkWidget *
ignore_stats_entry (GtkWidget *box, char *label, int value)
{
	GtkWidget *entry;
	char buf[16];

	g_snprintf (buf, sizeof (buf), "%d", value);
	gtkutil_label_new (label, box);
	entry = gtkutil_entry_new (16, box, 0, 0);
	gtk_widget_set_size_request (entry, 30, -1);
	gtk_editable_set_editable (GTK_EDITABLE (entry), FALSE);
	gtk_widget_set_sensitive (entry, FALSE);
	gtk_entry_set_text (GTK_ENTRY (entry), buf);
	return entry;
}

void
ignore_gui_open (void)
{
	GtkWidget *vbox, *box, *stat_box, *frame;
	GSList *item;
	char buf[128];

	if (ignorewin)
	{
		mg_bring_tofront (ignorewin);
		return;
	}

	g_snprintf (buf, sizeof (buf), _("Ignore list - %s"), _(DISPLAY_NAME));
	ignorewin = mg_create_generic_tab ("IgnoreList", buf, FALSE, TRUE,
		close_ignore_gui_callback, NULL, 700, 300, &vbox, 0);
	gtkutil_destroy_on_esc (ignorewin);
	ignore_view = fabulor_ignore_list_new (ignore_mask_renamed,
		ignore_flags_changed, NULL);
	if (!ignore_view || !fabulor_ignore_list_create_view (ignore_view,
		GTK_BOX (vbox), _("Mask"), _("Channel"), _("Private"), _("Notice"),
		_("CTCP"), _("DCC"), _("Invite"), _("Unignore")))
	{
		fabulor_ignore_list_free (ignore_view);
		ignore_view = NULL;
		fabulor_gtk_window_destroy (GTK_WINDOW (ignorewin));
		return;
	}
	g_object_set_data_full (G_OBJECT (ignorewin), "ignore-list", ignore_view,
		(GDestroyNotify) fabulor_ignore_list_free);

	frame = gtk_frame_new (_("Ignore Stats:"));
	gtk_widget_show (frame);
	stat_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_container_set_border_width (GTK_CONTAINER (stat_box), 6);
	fabulor_gtk_frame_set_child (GTK_FRAME (frame), stat_box);
	gtk_widget_show (stat_box);
	num_chan = ignore_stats_entry (stat_box, _("Channel:"), ignored_chan);
	num_priv = ignore_stats_entry (stat_box, _("Private:"), ignored_priv);
	num_noti = ignore_stats_entry (stat_box, _("Notice:"), ignored_noti);
	num_ctcp = ignore_stats_entry (stat_box, _("CTCP:"), ignored_ctcp);
	num_invi = ignore_stats_entry (stat_box, _("Invite:"), ignored_invi);
	fabulor_gtk_box_append (GTK_BOX (vbox), frame, FALSE, FALSE, 5);

	box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (box), GTK_BUTTONBOX_SPREAD);
	fabulor_gtk_box_append (GTK_BOX (vbox), box, FALSE, FALSE, 2);
	gtk_container_set_border_width (GTK_CONTAINER (box), 5);
	gtk_widget_show (box);
	gtkutil_button (box, ICON_IGNORE_NEW, 0, ignore_new_entry_clicked, NULL,
		_("Add..."));
	gtkutil_button (box, ICON_IGNORE_DELETE, 0, ignore_delete_entry_clicked,
		NULL, _("Delete"));
	gtkutil_button (box, ICON_IGNORE_CLEAR, 0, ignore_clear_entry_clicked,
		ignore_view, _("Clear"));

	for (item = ignore_list; item; item = item->next)
	{
		struct ignore *ignore = item->data;
		fabulor_ignore_list_append (ignore_view, ignore->mask,
			(guint) ignore->type, FALSE);
	}
	gtk_widget_show (ignorewin);
}

void
fe_ignore_update (int level)
{
	char buf[16];

	if (level != 2 || !ignorewin)
		return;
#define UPDATE_IGNORE_STAT(widget, value) \
	do { g_snprintf (buf, sizeof (buf), "%d", (value)); \
	gtk_entry_set_text (GTK_ENTRY (widget), buf); } while (0)
	UPDATE_IGNORE_STAT (num_ctcp, ignored_ctcp);
	UPDATE_IGNORE_STAT (num_noti, ignored_noti);
	UPDATE_IGNORE_STAT (num_chan, ignored_chan);
	UPDATE_IGNORE_STAT (num_invi, ignored_invi);
	UPDATE_IGNORE_STAT (num_priv, ignored_priv);
#undef UPDATE_IGNORE_STAT
}
