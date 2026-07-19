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

#include "fe-gtk.h"
#include "theme/theme-manager.h"

#include "../common/zoitechat.h"
#include "../common/fe.h"
#include "../common/modes.h"
#include "../common/outbound.h"
#include "../common/zoitechatc.h"
#include "gtkutil.h"
#include "gtk-compat.h"
#include "maingui.h"
#include "ban-list.h"
#include "banlist.h"

#define ICON_BANLIST_REMOVE "list-remove"
#define ICON_BANLIST_CLEAR "edit-clear"
#define ICON_BANLIST_REFRESH "view-refresh"

/*
 * These supports_* routines set capable, readable, writable bits */
static void supports_bans (banlist_info *, int);
static void supports_exempt (banlist_info *, int);
static void supports_invite (banlist_info *, int);
static void supports_quiet (banlist_info *, int);

static mode_info modes[MODE_CT] = {
	{
		N_("Bans"),
		N_("Ban"),
		'b',
		RPL_BANLIST,
		RPL_ENDOFBANLIST,
		1<<MODE_BAN,
		supports_bans
	}
	,{
		N_("Exempts"),
		N_("Exempt"),
		'e',
		RPL_EXCEPTLIST,
		RPL_ENDOFEXCEPTLIST,
		1<<MODE_EXEMPT,
		supports_exempt
	}
	,{
		N_("Invites"),
		N_("Invite"),
		'I',
		RPL_INVITELIST,
		RPL_ENDOFINVITELIST,
		1<<MODE_INVITE,
		supports_invite
	}
	,{
		N_("Quiets"),
		N_("Quiet"),
		'q',
		RPL_QUIETLIST,
		RPL_ENDOFQUIETLIST,
		1<<MODE_QUIET,
		supports_quiet
	}
};

static void
supports_bans (banlist_info *banl, int i)
{
	int bit = 1<<i;

	banl->capable |= bit;
	banl->readable |= bit;
	banl->writeable |= bit;
	return;
}

static void
supports_exempt (banlist_info *banl, int i)
{
	server *serv = banl->sess->server;
	char *cm = serv->chanmodes;
	int bit = 1<<i;

	if (serv->have_except)
		goto yes;

	if (!cm)
		return;

	while (*cm)
	{
		if (*cm == ',')
			break;
		if (*cm == 'e')
			goto yes;
		cm++;
	}
	return;

yes:
	banl->capable |= bit;
	banl->writeable |= bit;
}

static void
supports_invite (banlist_info *banl, int i)
{
	server *serv = banl->sess->server;
	char *cm = serv->chanmodes;
	int bit = 1<<i;

	if (serv->have_invite)
		goto yes;

	if (!cm)
		return;

	while (*cm)
	{
		if (*cm == ',')
			break;
		if (*cm == 'I')
			goto yes;
		cm++;
	}
	return;

yes:
	banl->capable |= bit;
	banl->writeable |= bit;
}

static void
supports_quiet (banlist_info *banl, int i)
{
	server *serv = banl->sess->server;
	char *cm = serv->chanmodes;
	int bit = 1<<i;

	if (!cm)
		return;

	while (*cm)
	{
		if (*cm == ',')
			break;
		if (*cm == modes[i].letter)
			goto yes;
		cm++;
	}
	return;

yes:
	banl->capable |= bit;
	banl->readable |= bit;
	banl->writeable |= bit;
}

/* fe_add_ban_list() and fe_ban_list_end() return TRUE if consumed, FALSE otherwise */
gboolean
fe_add_ban_list (struct session *sess, char *mask, char *who, char *when, int rplcode)
{
	banlist_info *banl = sess->res->banlist;
	int i;

	if (!banl)
		return FALSE;

	for (i = 0; i < MODE_CT; i++)
		if (modes[i].code == rplcode)
			break;
	if (i == MODE_CT)
	{
		/* printf ("Unexpected value in fe_add_ban_list:  %d\n", rplcode); */
		return FALSE;
	}
	if (banl->pending & 1<<i)
	{
		fabulor_ban_list_append (banl->list, (guint) i, _(modes[i].type),
			mask, who, when);
		banl->line_ct = (int) fabulor_ban_list_get_n_rows (banl->list);
		return TRUE;
	}
	else return FALSE;
}

/* Sensitize checkboxes and buttons as appropriate for the moment  */
static void
banlist_sensitize (banlist_info *banl)
{
	int checkable, i;
	gboolean is_op = FALSE;

	if (banl->sess->me == NULL)
		return;

	/* FIXME: More access levels than these can unban */
	if (banl->sess->me->op || banl->sess->me->hop)
		is_op = TRUE;

	/* CHECKBOXES -- */
	checkable = is_op? banl->writeable: banl->readable;
	for (i = 0; i < MODE_CT; i++)
	{
		if (banl->checkboxes[i] == NULL)
			continue;
		if ((checkable & 1<<i) == 0)
		/* Checkbox is not checkable.  Grey it and uncheck it. */
		{
			gtk_widget_set_sensitive (banl->checkboxes[i], FALSE);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (banl->checkboxes[i]), FALSE);
		}
		else
		/* Checkbox is checkable.  Be sure it's sensitive. */
		{
			gtk_widget_set_sensitive (banl->checkboxes[i], TRUE);
		}
	}

	/* BUTTONS --- */
	if (!is_op || banl->line_ct == 0)
	{
		/* If user is not op or list is empty, buttons should be all greyed */
		gtk_widget_set_sensitive (banl->but_clear, FALSE);
		gtk_widget_set_sensitive (banl->but_crop, FALSE);
		gtk_widget_set_sensitive (banl->but_remove, FALSE);
	}
	else
	{
		/* If no lines are selected, only the CLEAR button should be sensitive */
		if (banl->select_ct == 0)
		{
			gtk_widget_set_sensitive (banl->but_clear, TRUE);
			gtk_widget_set_sensitive (banl->but_crop, FALSE);
			gtk_widget_set_sensitive (banl->but_remove, FALSE);
		}
		/* If any lines are selected, only the REMOVE and CROP buttons should be sensitive */
		else
		{
			gtk_widget_set_sensitive (banl->but_clear, FALSE);
			gtk_widget_set_sensitive (banl->but_crop, banl->line_ct == banl->select_ct? FALSE: TRUE);
			gtk_widget_set_sensitive (banl->but_remove, TRUE);
		}
	}

	/* Set "Refresh" sensitvity */
	gtk_widget_set_sensitive (banl->but_refresh, banl->pending? FALSE: banl->checked? TRUE: FALSE);
}
/* fe_ban_list_end() returns TRUE if consumed, FALSE otherwise */
gboolean
fe_ban_list_end (struct session *sess, int rplcode)
{
	banlist_info *banl = sess->res->banlist;
	int i;

	if (!banl)
		return FALSE;

	for (i = 0; i < MODE_CT; i++)
		if (modes[i].endcode == rplcode)
			break;
	if (i == MODE_CT)
	{
		/* printf ("Unexpected rplcode value in fe_ban_list_end:  %d\n", rplcode); */
		return FALSE;
	}
	if (banl->pending & modes[i].bit)
	{
		banl->pending &= ~modes[i].bit;
		if (!banl->pending)
		{
			gtk_widget_set_sensitive (banl->but_refresh, TRUE);
			banlist_sensitize (banl);
		}
		return TRUE;
	}
	else return FALSE;
}

static void
banlist_copyentry (GtkWidget *item, banlist_info *banl)
{
	char *str;

	if (GPOINTER_TO_INT (g_object_get_data (G_OBJECT (item),
		"fabulor-ban-copy-entry")))
		str = fabulor_ban_list_dup_selected_entry (banl->list,
			_("%s on %s by %s"));
	else
		str = fabulor_ban_list_dup_selected_mask (banl->list);
	if (str && *str)
		gtkutil_copy_to_clipboard (item, str);
	g_free (str);
#if GTK_MAJOR_VERSION >= 4
	{
		GtkWidget *popover = gtk_widget_get_ancestor (item, GTK_TYPE_POPOVER);
		if (popover)
			gtk_popover_popdown (GTK_POPOVER (popover));
	}
#endif
}

#if GTK_MAJOR_VERSION >= 4
static void
banlist_popover_closed (GtkPopover *popover, gpointer user_data)
{
	(void) user_data;
	gtk_widget_unparent (GTK_WIDGET (popover));
}
#endif

static gboolean
banlist_button_pressed (GtkWidget *wid, guint button, guint n_press,
	gdouble x, gdouble y, GdkModifierType state, gpointer userdata)
{
	banlist_info *banl = userdata;
	GtkWidget *menu, *maskitem, *allitem;

	(void) n_press;
	(void) state;

	if (button == 3 && fabulor_ban_list_select_at_point (banl->list, x, y))
	{
#if GTK_MAJOR_VERSION >= 4
		GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
		GdkRectangle point = { (gint) x, (gint) y, 1, 1 };
		menu = gtk_popover_new ();
		maskitem = gtk_button_new_with_label (_("Copy mask"));
		allitem = gtk_button_new_with_label (_("Copy entry"));
		g_object_set_data (G_OBJECT (allitem), "fabulor-ban-copy-entry",
			GINT_TO_POINTER (TRUE));
		g_signal_connect (maskitem, "clicked", G_CALLBACK (banlist_copyentry), banl);
		g_signal_connect (allitem, "clicked", G_CALLBACK (banlist_copyentry), banl);
		fabulor_gtk_box_append (GTK_BOX (box), maskitem, FALSE, FALSE, 0);
		fabulor_gtk_box_append (GTK_BOX (box), allitem, FALSE, FALSE, 0);
		fabulor_gtk_popover_set_child (GTK_POPOVER (menu), box);
		gtk_widget_set_parent (menu, wid);
		gtk_popover_set_pointing_to (GTK_POPOVER (menu), &point);
		g_signal_connect (menu, "closed", G_CALLBACK (banlist_popover_closed), NULL);
		gtk_popover_popup (GTK_POPOVER (menu));
#else
			menu = gtk_menu_new ();
			maskitem = gtk_menu_item_new_with_label (_("Copy mask"));
			allitem = gtk_menu_item_new_with_label (_("Copy entry"));
			g_object_set_data (G_OBJECT (allitem), "fabulor-ban-copy-entry",
				GINT_TO_POINTER (TRUE));
			g_signal_connect (maskitem, "activate", G_CALLBACK (banlist_copyentry), banl);
			g_signal_connect (allitem, "activate", G_CALLBACK (banlist_copyentry), banl);
			gtk_menu_shell_append (GTK_MENU_SHELL(menu), maskitem);
			gtk_menu_shell_append (GTK_MENU_SHELL(menu), allitem);
			gtk_widget_show_all (menu);
			gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);
#endif
		return TRUE;
	}
	return FALSE;
}

static void
banlist_select_changed (guint selected, gpointer user_data)
{
	banlist_info *banl = user_data;
	banl->select_ct = (int) selected;
	banlist_sensitize (banl);
}

/**
 *  * Performs the actual refresh operations.
 *  */
static void
banlist_do_refresh (banlist_info *banl)
{
	session *sess = banl->sess;
	char tbuf[256];
	int i;

	banlist_sensitize (banl);

	if (sess->server->connected)
	{
		g_snprintf (tbuf, sizeof tbuf, "Ban List (%s, %s) - %s",
						sess->channel, sess->server->servername, _(DISPLAY_NAME));
		mg_set_title (banl->window, tbuf);

		banl->line_ct = 0;
		fabulor_ban_list_clear (banl->list);
		banl->pending = banl->checked;
		if (banl->pending)
		{
			for (i = 0; i < MODE_CT; i++)
				if (banl->pending & 1<<i)
				{
					g_snprintf (tbuf, sizeof tbuf, "quote mode %s +%c", sess->channel, modes[i].letter);
					handle_command (sess, tbuf, FALSE);
				}
		}
	}
	else
	{
		fe_message (_("Not connected."), FE_MSG_ERROR);
	}
}

static void
banlist_refresh (GtkWidget * wid, banlist_info *banl)
{
	/* JG NOTE: Didn't see actual use of wid here, so just forwarding
	   *          * this to chanlist_do_refresh because I use it without any widget
	   *          * param in chanlist_build_gui_list when the user presses enter
	   *          * or apply for the first time if the list has not yet been
	   *          * received.
	   *          */
	banlist_do_refresh (banl);
}

static int
banlist_unban_inner (banlist_info *banl, int mode_num, gboolean selected)
{
	session *sess = banl->sess;
	char tbuf[2048];
	GPtrArray *masks = fabulor_ban_list_dup_masks (banl->list,
		(guint) mode_num, selected);
	int count = (int) masks->len;

	if (count)
		send_channel_modes (sess, tbuf, (char **) masks->pdata, 0, count, '-',
			modes[mode_num].letter, 0);
	g_ptr_array_unref (masks);
	return count;
}

static void
banlist_remove_rows (banlist_info *banl, gboolean selected)
{
	int i, num = 0;

	for (i = 0; i < MODE_CT; i++)
		num += banlist_unban_inner (banl, i, selected);

	if (num < 1)
	{
		fe_message (_("You must select some bans."), FE_MSG_ERROR);
		return;
	}

	banlist_do_refresh (banl);
}

static void
banlist_unban (GtkWidget *wid, banlist_info *banl)
{
	(void) wid;
	banlist_remove_rows (banl, TRUE);
}

static void
banlist_clear_cb (GtkDialog *dialog, gint response, gpointer data)
{
	banlist_info *banl = data;

	fabulor_gtk_window_destroy (GTK_WINDOW (dialog));

	if (response == GTK_RESPONSE_OK)
	{
		fabulor_ban_list_select_all (banl->list);
		banlist_unban (NULL, banl);
	}
}

static void
banlist_clear (GtkWidget * wid, banlist_info *banl)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new (NULL, 0,
								GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL,
					_("Are you sure you want to remove all listed items in %s?"), banl->sess->channel);
	theme_manager_attach_window (dialog);

	g_signal_connect (G_OBJECT (dialog), "response",
							G_CALLBACK (banlist_clear_cb), banl);
	fabulor_gtk_window_position_at_pointer (GTK_WINDOW (dialog));
	gtk_widget_show (dialog);
}

static void
banlist_crop (GtkWidget * wid, banlist_info *banl)
{
	(void) wid;
	if (fabulor_ban_list_get_n_selected (banl->list) > 0)
		banlist_remove_rows (banl, FALSE);
	else
		fe_message (_("You must select some bans."), FE_MSG_ERROR);
}

static void
banlist_toggle (GtkWidget *item, gpointer data)
{
	banlist_info *banl = data;
	int i, bit = 0;

	for (i = 0; i < MODE_CT; i++)
		if (banl->checkboxes[i] == item)
		{
			bit = 1<<i;
			break;
		}

	if (bit)
	{
		banl->checked &= ~bit;
		banl->checked |= (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (item)))? bit: 0;
		banlist_do_refresh (banl);
	}
}

static void
banlist_closegui (GtkWidget *wid, banlist_info *banl)
{
	session *sess = banl->sess;

	if (sess->res->banlist == banl)
	{
		fabulor_ban_list_free (banl->list);
		g_free (banl);
		sess->res->banlist = NULL;
	}
}

static GtkWidget *
banlist_table_new (void)
{
	GtkWidget *table = gtkutil_grid_new (1, MODE_CT, FALSE);

	gtk_grid_set_column_spacing (GTK_GRID (table), 16);
	return table;
}

void
banlist_opengui (struct session *sess)
{
	banlist_info *banl;
	int i;
	GtkWidget *table, *vbox, *bbox;
	char tbuf[256];

	if (sess->type != SESS_CHANNEL || sess->channel[0] == 0)
	{
		fe_message (_("You can only open the Ban List window while in a channel tab."), FE_MSG_ERROR);
		return;
	}

	if (sess->res->banlist == NULL)
	{
		sess->res->banlist = g_new0 (banlist_info, 1);
	}
	banl = sess->res->banlist;
	if (banl->window)
	{
		mg_bring_tofront (banl->window);
		return;
	}

	/* New banlist for this session -- Initialize it */
	banl->sess = sess;
	/* For each mode set its bit in capable/readable/writeable */
	for (i = 0; i < MODE_CT; i++)
		modes[i].tester (banl, i);
	/* Force on the checkmark in the "Bans" box */
	banl->checked = 1<<MODE_BAN;

	g_snprintf (tbuf, sizeof tbuf, _("Ban List (%s) - %s"),
					sess->server->servername, _(DISPLAY_NAME));

	banl->window = mg_create_generic_tab ("BanList", tbuf, FALSE,
					TRUE, banlist_closegui, banl, 700, 300, &vbox, sess->server);
	gtkutil_destroy_on_esc (banl->window);

	gtk_container_set_border_width (GTK_CONTAINER (banl->window), 3);
	gtk_box_set_spacing (GTK_BOX (vbox), 3);

	banl->list = fabulor_ban_list_new (banlist_select_changed, banl);
	if (!banl->list)
	{
		fe_message (_("Failed to create the Ban List."), FE_MSG_ERROR);
		fabulor_gtk_window_destroy (GTK_WINDOW (banl->window));
		return;
	}
	{
		GtkWidget *view = fabulor_ban_list_create_view (banl->list,
			GTK_BOX (vbox), _("Type"), _("Mask"), _("From"), _("Date"));
		fabulor_gtk_widget_on_multi_click (view, banlist_button_pressed, banl);
	}

	table = banlist_table_new ();
	fabulor_gtk_box_append (GTK_BOX (vbox), table, FALSE, FALSE, 0);

	for (i = 0; i < MODE_CT; i++)
	{
		if (!(banl->capable & 1<<i))
			continue;
		banl->checkboxes[i] = gtk_check_button_new_with_label (_(modes[i].name));
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (banl->checkboxes[i]), (banl->checked & 1<<i? TRUE: FALSE));
		g_signal_connect (G_OBJECT (banl->checkboxes[i]), "toggled",
								G_CALLBACK (banlist_toggle), banl);
		gtkutil_grid_attach (table, banl->checkboxes[i], i + 1, i + 2, 0, 1,
				     GTKUTIL_ATTACH_FILL, GTKUTIL_ATTACH_FILL, 0, 0);
	}

	bbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_SPREAD);
	gtk_container_set_border_width (GTK_CONTAINER (bbox), 5);
	fabulor_gtk_box_append (GTK_BOX (vbox), bbox, FALSE, FALSE, 0);
	gtk_widget_show (bbox);

	banl->but_remove = gtkutil_button (bbox, ICON_BANLIST_REMOVE, 0, banlist_unban, banl,
	                _("Remove"));
	banl->but_crop = gtkutil_button (bbox, ICON_BANLIST_REMOVE, 0, banlist_crop, banl,
	                _("Crop"));
	banl->but_clear = gtkutil_button (bbox, ICON_BANLIST_CLEAR, 0, banlist_clear, banl,
	                _("Clear"));

	banl->but_refresh = gtkutil_button (bbox, ICON_BANLIST_REFRESH, 0, banlist_refresh, banl, _("Refresh"));

	banlist_do_refresh (banl);

	fabulor_gtk_widget_reveal_tree (banl->window);
}
