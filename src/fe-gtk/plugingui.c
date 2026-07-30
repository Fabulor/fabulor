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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib/gstdio.h>

#include "fe-gtk.h"

#include "../common/zoitechat.h"
#define PLUGIN_C
typedef struct session fabulor_context;
#include "../common/fabulor-plugin.h"
#include "../common/plugin.h"
#include "../common/util.h"
#include "../common/outbound.h"
#include "../common/fe.h"
#include "../common/zoitechatc.h"
#include "../common/cfgfiles.h"
#include "gtkutil.h"
#include "gtk-compat.h"
#include "addon-list.h"
#include "maingui.h"

static GtkWidget *plugin_window = NULL;
static FabulorAddonList *plugin_list_view = NULL;

static const char *
plugingui_safe_string (const char *value)
{
	return value ? value : "";
}

static session *
plugingui_get_target_session (void)
{
	if (is_session (current_sess))
		return current_sess;

	fe_message (_("No active session available for addon command."), FE_MSG_ERROR);
	return NULL;
}

static gboolean
plugingui_path_is_under_dir (const char *dir, const char *path)
{
	gsize dir_len;

	if (!dir || !path)
		return FALSE;

	dir_len = strlen (dir);
	if (strcmp (dir, path) == 0)
		return TRUE;

	return g_str_has_prefix (path, dir) && G_IS_DIR_SEPARATOR (path[dir_len]);
}

static gboolean
plugingui_command_path_is_safe (const char *path)
{
	const unsigned char *p;

	if (!path || !*path)
		return FALSE;

	for (p = (const unsigned char *) path; *p; p++)
	{
		if (*p < 0x20 || *p == 0x7f || *p == '"')
			return FALSE;
	}

	return TRUE;
}

static char *
plugingui_command_for_path (const char *command, const char *path)
{
	if (!plugingui_command_path_is_safe (path))
		return NULL;

	if (strchr (path, ' '))
		return g_strdup_printf ("%s \"%s\"", command, path);

	return g_strdup_printf ("%s %s", command, path);
}

static char *
plugingui_canonicalize_path (const char *path)
{
	if (!path || !*path)
		return NULL;

	return g_canonicalize_filename (path, NULL);
}

#define ICON_PLUGIN_LOAD "zc-menu-load-plugin"
#define ICON_PLUGIN_UNLOAD "zc-menu-delete"
#define ICON_PLUGIN_RELOAD "zc-menu-refresh"

static GtkWidget *
plugingui_icon_button (GtkWidget *box, const char *label,
							  const char *icon_name, GCallback callback,
							  gpointer userdata)
{
	GtkWidget *button;

	button = fabulor_gtk_button_new_with_icon_and_mnemonic (label, icon_name,
		FABULOR_GTK_ICON_SIZE_MENU);
	fabulor_gtk_box_append (GTK_BOX (box), button, FALSE, TRUE, 0);
	g_signal_connect (G_OBJECT (button), "clicked", callback, userdata);
	gtk_widget_show (button);

	return button;
}


static void
plugingui_close (gpointer userdata)
{
	(void) userdata;
	plugin_window = NULL;
	plugin_list_view = NULL;
}

extern GSList *plugin_list;

void
fe_pluginlist_update (void)
{
	fabulor_plugin *pl;
	GSList *list;

	if (!plugin_window || !plugin_list_view)
		return;

	fabulor_addon_list_clear (plugin_list_view);

	list = plugin_list;
	while (list)
	{
		pl = list->data;
		if (pl && pl->version && pl->version[0] != 0)
		{
			fabulor_addon_list_append (plugin_list_view,
				plugingui_safe_string (pl->name),
				plugingui_safe_string (pl->version),
				pl->filename ? file_part (pl->filename) : "",
				plugingui_safe_string (pl->desc),
				plugingui_safe_string (pl->filename));
		}
		list = list->next;
	}
}

static void
plugingui_load_cb (session *sess, char *file)
{
	session *target_sess;

	if (file)
	{
		char *buf;
		char *load_target;
		char *addons_dir;
		char *addons_dir_canonical;
		char *file_canonical;
		char *basename;
		char *addons_target;
		gboolean file_in_addons;
		char *error;

		target_sess = is_session (sess) ? sess : current_sess;
		if (!is_session (target_sess))
		{
			fe_message (_("No active session available for loading addons."), FE_MSG_ERROR);
			return;
		}

		addons_dir = g_build_filename (get_xdir (), "addons", NULL);
		if (g_mkdir_with_parents (addons_dir, 0700) != 0)
		{
			fe_message (_("Could not create the addons directory."), FE_MSG_ERROR);
			g_free (addons_dir);
			return;
		}

		addons_dir_canonical = plugingui_canonicalize_path (addons_dir);
		file_canonical = plugingui_canonicalize_path (file);
		if (!addons_dir_canonical || !file_canonical)
		{
			fe_message (_("Could not resolve the selected addon path."), FE_MSG_ERROR);
			g_free (file_canonical);
			g_free (addons_dir_canonical);
			g_free (addons_dir);
			return;
		}

		load_target = g_strdup (file_canonical);
		file_in_addons = plugingui_path_is_under_dir (addons_dir_canonical, file_canonical);

		if (!file_in_addons)
		{
			char *contents = NULL;
			gsize length;

			basename = g_path_get_basename (file_canonical);
			addons_target = g_build_filename (addons_dir_canonical, basename, NULL);
			if (!plugingui_path_is_under_dir (addons_dir_canonical, addons_target))
			{
				g_free (addons_target);
				g_free (basename);
				g_free (load_target);
				g_free (file_canonical);
				g_free (addons_dir_canonical);
				g_free (addons_dir);
				fe_message (_("Selected addon path is outside the addons directory."), FE_MSG_ERROR);
				return;
			}

			if (!g_file_get_contents (file_canonical, &contents, &length, NULL)
				|| !g_file_set_contents (addons_target, contents, length, NULL))
			{
				g_free (contents);
				g_free (addons_target);
				g_free (basename);
				g_free (load_target);
				g_free (file_canonical);
				g_free (addons_dir_canonical);
				g_free (addons_dir);
				fe_message (_("Could not copy the selected addon into the addons directory."), FE_MSG_ERROR);
				return;
			}

			g_free (contents);
			g_free (load_target);
			load_target = g_strdup (addons_target);
			g_free (addons_target);
			g_free (basename);
		}

		g_free (file_canonical);
		g_free (addons_dir_canonical);
		g_free (addons_dir);

#ifdef WIN32
		/*
		 * The command parser is more reliable with forward slashes on Windows
		 * paths (especially when quoted), so normalize before issuing LOAD.
		 */
		g_strdelimit (load_target, "\\", '/');
#endif

		if (g_str_has_suffix (load_target, "."PLUGIN_SUFFIX))
		{
			error = plugin_load (target_sess, load_target, NULL);
			if (error)
				fe_message (error, FE_MSG_ERROR);
		}
		else
		{
			buf = plugingui_command_for_path ("LOAD", load_target);
			if (!buf)
				fe_message (_("Addon path contains characters that cannot be loaded safely."), FE_MSG_ERROR);
			else
			{
				handle_command (target_sess, buf, FALSE);
				g_free (buf);
			}
		}
		g_free (load_target);
	}
}

void
plugingui_load (void)
{
	const char *xdir = get_xdir ();
	char *sub_dir = NULL;

	if (xdir && xdir[0] != '\0')
		sub_dir = g_build_filename (xdir, "addons", NULL);

	gtkutil_file_req (NULL, _("Select a Plugin or Script to load"), plugingui_load_cb, NULL,
						sub_dir, "*."PLUGIN_SUFFIX";*.py;*.tcl", FRF_FILTERISINITIAL|FRF_EXTENSIONS);
		g_free (sub_dir);
}

static void
plugingui_loadbutton_cb (GtkWidget * wid, gpointer unused)
{
	plugingui_load ();
}

static void
plugingui_unload (GtkWidget * wid, gpointer unused)
{
	char *modname, *file;
	session *target_sess;

	if (!plugin_list_view || !fabulor_addon_list_dup_selected (
		plugin_list_view, &modname, &file))
		return;
	if (!modname || !*modname)
	{
		g_free (modname);
		g_free (file);
		return;
	}
	if (!file || !*file)
	{
		g_free (modname);
		g_free (file);
		return;
	}

	if (g_str_has_suffix (file, "."PLUGIN_SUFFIX))
	{
		if (plugin_kill (modname, FALSE) == 2)
			fe_message (_("That plugin is refusing to unload.\n"), FE_MSG_ERROR);
	}
	else
	{
		char *buf;
		/* let the language runtime handle script unloads */
		target_sess = plugingui_get_target_session ();
		if (!target_sess)
		{
			g_free (modname);
			g_free (file);
			return;
		}

		buf = plugingui_command_for_path ("UNLOAD", file);
		if (!buf)
			fe_message (_("Addon path contains characters that cannot be unloaded safely."), FE_MSG_ERROR);
		else
		{
			handle_command (target_sess, buf, FALSE);
			g_free (buf);
		}
	}

	g_free (modname);
	g_free (file);
}

static void
plugingui_reloadbutton_cb (GtkWidget *wid, gpointer unused)
{
	char *file = plugin_list_view ?
		fabulor_addon_list_dup_selected_path (plugin_list_view) : NULL;
	session *target_sess;

	if (file)
	{
		char *buf;

		target_sess = plugingui_get_target_session ();
		if (!target_sess)
		{
			g_free (file);
			return;
		}

		buf = plugingui_command_for_path ("RELOAD", file);
		if (!buf)
			fe_message (_("Addon path contains characters that cannot be reloaded safely."), FE_MSG_ERROR);
		else
		{
			handle_command (target_sess, buf, FALSE);
			g_free (buf);
		}
		g_free (file);
	}
}

void
plugingui_open (void)
{
	GtkWidget *view;
	GtkWidget *vbox, *hbox;
	char buf[128];

	if (plugin_window)
	{
		mg_bring_tofront (plugin_window);
		return;
	}

	g_snprintf(buf, sizeof(buf), _("Plugins and Scripts - %s"), _(DISPLAY_NAME));
	plugin_window = mg_create_generic_tab ("Addons", buf, FALSE, TRUE, plugingui_close, NULL,
														 700, 300, &vbox, 0);
	gtkutil_destroy_on_esc (plugin_window);

	plugin_list_view = fabulor_addon_list_new ();
	view = plugin_list_view ? fabulor_addon_list_create_view (plugin_list_view,
		GTK_BOX (vbox), _("Name"), _("Version"), _("File"), _("Description")) : NULL;
	if (!view)
	{
		fabulor_addon_list_free (plugin_list_view);
		plugin_list_view = NULL;
		fabulor_gtk_window_destroy (GTK_WINDOW (plugin_window));
		return;
	}
	g_object_set_data_full (G_OBJECT (plugin_window), "addon-list",
		plugin_list_view, (GDestroyNotify) fabulor_addon_list_free);


	hbox = fabulor_gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL,
		FABULOR_GTK_BUTTON_BOX_SPREAD, 0);
	fabulor_gtk_container_set_uniform_inset (hbox, 5);
	fabulor_gtk_box_append (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	{
		plugingui_icon_button (hbox, _("_Load..."), ICON_PLUGIN_LOAD,
									  G_CALLBACK (plugingui_loadbutton_cb), NULL);
		plugingui_icon_button (hbox, _("_Unload"), ICON_PLUGIN_UNLOAD,
									  G_CALLBACK (plugingui_unload), NULL);
		plugingui_icon_button (hbox, _("_Reload"), ICON_PLUGIN_RELOAD,
									  G_CALLBACK (plugingui_reloadbutton_cb), NULL);
	}

	fe_pluginlist_update ();

	fabulor_gtk_widget_reveal_tree (plugin_window);
}
