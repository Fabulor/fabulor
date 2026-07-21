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

#include "fe-gtk.h"

#include "../common/zoitechat.h"
#include "../common/zoitechatc.h"
#include "../common/cfgfiles.h"
#include "../common/fe.h"
#include "../common/url.h"
#include "../common/tree.h"
#include "gtkutil.h"
#include "gtk-compat.h"
#include "menu.h"
#include "maingui.h"
#include "url-list.h"
#include "urlgrab.h"

#define ICON_URLGRAB_CLEAR "zc-menu-clear"
#define ICON_URLGRAB_COPY "zc-menu-copy"
#define ICON_URLGRAB_SAVE_AS "zc-menu-save-as"

static GtkWidget *urlgrabberwindow = 0;
static FabulorUrlList *urlgrabberlist = NULL;


static gboolean
url_list_clicked_cb (GtkWidget *view, guint button, guint n_press, gdouble x,
					gdouble y, GdkModifierType state, gpointer data)
{
	FabulorUrlList *list = data;
	gchar *url = fabulor_url_list_select_and_dup_at_point (list, x, y);

	(void) view;
	(void) state;
	if (!url)
		return FALSE;

	switch (button)
	{
		case 1:
			if (n_press == 2)
				fe_open_url (url);
			break;
		case 3:
			menu_urlmenu_at (view, x, y, state, url);
			break;
		default:
			break;
	}
	g_free (url);

	return FALSE;
}

static void
url_closegui (GtkWidget *wid, gpointer userdata)
{
	urlgrabberwindow = 0;
	urlgrabberlist = NULL;
}

static void
url_button_clear (void)
{
	url_clear ();
	if (urlgrabberlist)
		fabulor_url_list_clear (urlgrabberlist);
}

static void
url_button_copy (GtkWidget *widget, gpointer data)
{
	FabulorUrlList *list = data;
	gchar *url = fabulor_url_list_dup_selected (list);

	if (url)
	{
		fabulor_gtk_copy_text_to_clipboards (
			fabulor_url_list_get_view (list), url);
		g_free (url);
	}
}

static void
url_save_callback (void *arg1, char *file)
{
	if (file)
	{
		url_save_tree (file, "w", TRUE);
	}
}

static void
url_button_save (void)
{
	gtkutil_file_req (NULL, _("Select an output filename"),
							url_save_callback, NULL, NULL, NULL, FRF_WRITE);
}

void
fe_url_add (const char *urltext)
{
	if (urlgrabberwindow && urlgrabberlist)
		fabulor_url_list_prepend (urlgrabberlist, urltext,
			prefs.hex_url_grabber_limit > 0 ?
			(guint) prefs.hex_url_grabber_limit : 0);
}

static int
populate_cb (char *urltext, gpointer userdata)
{
	fe_url_add (urltext);
	return TRUE;
}

void
url_opengui ()
{
	GtkWidget *vbox, *hbox, *view;
	char buf[128];

	if (urlgrabberwindow)
	{
		mg_bring_tofront (urlgrabberwindow);
		return;
	}

	g_snprintf(buf, sizeof(buf), _("URL Grabber - %s"), _(DISPLAY_NAME));
	urlgrabberwindow =
		mg_create_generic_tab ("UrlGrabber", buf, FALSE, TRUE, url_closegui, NULL,
							 400, 256, &vbox, 0);
	gtkutil_destroy_on_esc (urlgrabberwindow);
	urlgrabberlist = fabulor_url_list_new ();
	view = urlgrabberlist ? fabulor_url_list_create_view (urlgrabberlist,
		GTK_BOX (vbox), _("URL")) : NULL;
	if (!view)
	{
		fabulor_url_list_free (urlgrabberlist);
		urlgrabberlist = NULL;
		fabulor_gtk_window_destroy (GTK_WINDOW (urlgrabberwindow));
		return;
	}
	g_object_set_data_full (G_OBJECT (urlgrabberwindow), "url-list",
		urlgrabberlist, (GDestroyNotify) fabulor_url_list_free);
	fabulor_gtk_widget_on_multi_click (view, url_list_clicked_cb,
		urlgrabberlist);

	hbox = fabulor_gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL,
		FABULOR_GTK_BUTTON_BOX_SPREAD, 0);
	fabulor_gtk_container_set_uniform_inset (hbox, 5);
	fabulor_gtk_box_append (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show (hbox);

	gtkutil_button (hbox, ICON_URLGRAB_CLEAR,
						 _("Clear list"), url_button_clear, 0, _("Clear"));
	gtkutil_button (hbox, ICON_URLGRAB_COPY,
						 _("Copy selected URL"), url_button_copy, urlgrabberlist, _("Copy"));
	gtkutil_button (hbox, ICON_URLGRAB_SAVE_AS,
						 _("Save list to a file"), url_button_save, 0, _("Save As..."));

	gtk_widget_show (urlgrabberwindow);

	if (prefs.hex_url_grabber)
		tree_foreach (url_tree, (tree_traverse_func *)populate_cb, NULL);
	else
	{
		fabulor_url_list_clear (urlgrabberlist);
		fe_url_add ("URL Grabber is disabled.");
	}
}
