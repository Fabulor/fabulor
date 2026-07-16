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
#include <sys/types.h>
#include <sys/stat.h>

#include <gdk/gdkkeysyms.h>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "fe-gtk.h"

#include "../common/zoitechat.h"
#include "../common/cfgfiles.h"
#include "../common/zoitechatc.h"
#include "../common/fe.h"
#include "menu.h"
#include "gtkutil.h"
#include "gtk-compat.h"
#include "maingui.h"
#include "editlist.h"
#include "editable-list.h"

#define ICON_EDITLIST_NEW "document-new"
#define ICON_EDITLIST_DELETE "edit-delete"
#define ICON_EDITLIST_CANCEL "dialog-cancel"
#define ICON_EDITLIST_SAVE "document-save"

static GtkWidget *editlist_win = NULL;
static GSList *editlist_list = NULL;
static FabulorEditableList *editlist_model = NULL;

static void
editlist_save (GtkWidget *igad, gchar *file)
{
	GPtrArray *rows;
	guint i;
	char buf[512];
	int fh;

	rows = fabulor_editable_list_dup_all (editlist_model);
	fh = zoitechat_open_file (file, O_TRUNC | O_WRONLY | O_CREAT, 0600, XOF_DOMODE);
	if (fh != -1)
	{
		for (i = 0; i < rows->len; i++)
		{
			FabulorEditableListRecord *row = g_ptr_array_index (rows, i);
			g_snprintf (buf, sizeof (buf), "NAME %s\nCMD %s\n\n",
				row->name, row->command);
			write (fh, buf, strlen (buf));
		}

		close (fh);
		fabulor_gtk_window_destroy (GTK_WINDOW (editlist_win));
		if (editlist_list == replace_list)
		{
			list_free (&replace_list);
			list_loadconf (file, &replace_list, 0);
		} else if (editlist_list == popup_list)
		{
			list_free (&popup_list);
			list_loadconf (file, &popup_list, 0);
		} else if (editlist_list == button_list)
		{
			GSList *list = sess_list;
			struct session *sess;
			list_free (&button_list);
			list_loadconf (file, &button_list, 0);
			while (list)
			{
				sess = (struct session *) list->data;
				fe_buttons_update (sess);
				list = list->next;
			}
		} else if (editlist_list == dlgbutton_list)
		{
			GSList *list = sess_list;
			struct session *sess;
			list_free (&dlgbutton_list);
			list_loadconf (file, &dlgbutton_list, 0);
			while (list)
			{
				sess = (struct session *) list->data;
				fe_dlgbuttons_update (sess);
				list = list->next;
			}
		} else if (editlist_list == ctcp_list)
		{
			list_free (&ctcp_list);
			list_loadconf (file, &ctcp_list, 0);
		} else if (editlist_list == command_list)
		{
			list_free (&command_list);
			list_loadconf (file, &command_list, 0);
		} else if (editlist_list == usermenu_list)
		{
			list_free (&usermenu_list);
			list_loadconf (file, &usermenu_list, 0);
			usermenu_update ();
		} else
		{
			list_free (&urlhandler_list);
			list_loadconf (file, &urlhandler_list, 0);
		}
	}
	g_ptr_array_unref (rows);
}

static void
editlist_load (FabulorEditableList *model, GSList *list)
{
	struct popup *pop;

	while (list)
	{
		pop = (struct popup *) list->data;
		fabulor_editable_list_append (model, pop->name, pop->cmd);
		list = list->next;
	}
}

static void
editlist_delete (GtkWidget *wid, gpointer userdata)
{
	fabulor_editable_list_remove_selected (editlist_model);
}

static void
editlist_add (GtkWidget *wid, gpointer userdata)
{
	fabulor_editable_list_add_empty (editlist_model);
}

static void
editlist_close (GtkWidget *wid, gpointer userdata)
{
	fabulor_gtk_window_destroy (GTK_WINDOW (editlist_win));
	editlist_win = NULL;
	fabulor_editable_list_free (editlist_model);
	editlist_model = NULL;
}

static gboolean
editlist_keypress (GtkWidget *wid, guint keyval, GdkModifierType state,
	gpointer userdata)
{
	(void) wid;
	(void) userdata;
	if (!(state & GDK_SHIFT_MASK))
		return FALSE;
	if (keyval == GDK_KEY_Up)
	{
		fabulor_editable_list_move_selected (editlist_model, -1);
		return TRUE;
	}
	if (keyval == GDK_KEY_Down)
	{
		fabulor_editable_list_move_selected (editlist_model, 1);
		return TRUE;
	}
	return FALSE;
}


void
editlist_gui_open (char *title1, char *title2, GSList *list, char *title, char *wmclass,
					char *file, char *help)
{
	GtkWidget *vbox, *box;
	GtkWidget *view;

	if (editlist_win)
	{
		mg_bring_tofront (editlist_win);
		return;
	}

	editlist_win = mg_create_generic_tab (wmclass, title, TRUE, FALSE,
												editlist_close, NULL, 450, 250, &vbox, 0);

	editlist_list = list;
	editlist_model = fabulor_editable_list_new ();
	view = editlist_model ? fabulor_editable_list_create_view (editlist_model,
		GTK_BOX (vbox), title1, title2) : NULL;
	if (!view)
	{
		fabulor_editable_list_free (editlist_model);
		editlist_model = NULL;
		fabulor_gtk_window_destroy (GTK_WINDOW (editlist_win));
		editlist_win = NULL;
		return;
	}
	fabulor_gtk_widget_on_key_pressed (view, editlist_keypress, NULL);

	if (help)
		gtk_widget_set_tooltip_text (view, help);

	box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (box), GTK_BUTTONBOX_SPREAD);
	fabulor_gtk_box_append (GTK_BOX (vbox), box, FALSE, FALSE, 2);
	gtk_container_set_border_width (GTK_CONTAINER (box), 5);
	gtk_widget_show (box);

	gtkutil_button (box, ICON_EDITLIST_NEW, 0, editlist_add,
					NULL, _("Add"));
	gtkutil_button (box, ICON_EDITLIST_DELETE, 0, editlist_delete,
					NULL, _("Delete"));
	gtkutil_button (box, ICON_EDITLIST_CANCEL, 0, editlist_close,
					NULL, _("Cancel"));
	gtkutil_button (box, ICON_EDITLIST_SAVE, 0, editlist_save,
					file, _("Save"));

	editlist_load (editlist_model, list);

	gtk_widget_show (editlist_win);
}
