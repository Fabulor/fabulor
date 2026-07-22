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

#ifdef WIN32
#include <windows.h>
#include <dwmapi.h>
#include <glib/gstdio.h>
#include <glib/gwin32.h>
#else
#include <unistd.h>
#endif

#include "../common/zoitechat.h"
#include "../common/fe.h"
#include "../common/util.h"
#include "../common/text.h"
#include "../common/cfgfiles.h"
#include "../common/zoitechatc.h"
#include "../common/plugin.h"
#include "../common/server.h"
#include "../common/url.h"
#include "gtkutil.h"
#include "gtk-compat.h"
#include "maingui.h"
#include "pixmaps.h"
#include "chanlist.h"
#include "joind.h"
#include "xtext.h"
#include "theme/theme-gtk.h"
#include "menu.h"
#include "notifygui.h"
#include "textgui.h"
#include "fkeys.h"
#include "plugin-tray.h"
#include "urlgrab.h"
#include "setup.h"
#include "plugin-notification.h"
#include "theme/theme-manager.h"
#include "theme/theme-application.h"
#include "preferences-persistence.h"
#include "window-state.h"
#if GTK_MAJOR_VERSION >= 4
#include "application-main-loop.h"
#endif

#ifdef USE_LIBCANBERRA
#include <canberra.h>
#endif

cairo_surface_t *channelwin_pix;

#if GTK_MAJOR_VERSION >= 4
static FabulorApplicationMainLoop *application_main_loop;
#endif

#ifdef USE_LIBCANBERRA
static ca_context *ca_con;
#endif

/* === command-line parameter parsing : requires glib 2.6 === */

static char *arg_cfgdir = NULL;
static gint arg_show_autoload = 0;
static gint arg_show_config = 0;
static gint arg_show_version = 0;

#ifdef WIN32
static void
win32_seed_user_icon_theme (void)
{
	char *user_icons_dir;
	char *user_index_theme;
	char *install_root;
	char *source_index_theme = NULL;
	char *data = NULL;
	gsize len = 0;
	GError *error = NULL;

	user_icons_dir = g_build_filename (get_xdir (), "icons", "hicolor", NULL);
	user_index_theme = g_build_filename (user_icons_dir, "index.theme", NULL);

	if (g_file_test (user_index_theme, G_FILE_TEST_IS_REGULAR))
		goto cleanup;

	install_root = g_win32_get_package_installation_directory_of_module (NULL);
	if (!install_root)
		goto cleanup;

	source_index_theme = g_build_filename (install_root, "share", "icons", "hicolor", "index.theme", NULL);
	if (!g_file_test (source_index_theme, G_FILE_TEST_IS_REGULAR))
	{
		g_free (source_index_theme);
		source_index_theme = g_build_filename (install_root, "icons", "hicolor", "index.theme", NULL);
	}

	if (!g_file_test (source_index_theme, G_FILE_TEST_IS_REGULAR))
	{
		g_free (source_index_theme);
		source_index_theme = g_build_filename (install_root, "Runtime", "GTK4", "share", "icons", "hicolor", "index.theme", NULL);
	}

	if (!g_file_test (source_index_theme, G_FILE_TEST_IS_REGULAR))
		goto cleanup;

	if (g_mkdir_with_parents (user_icons_dir, 0700) != 0)
		goto cleanup;

	if (!g_file_get_contents (source_index_theme, &data, &len, &error))
	{
		g_warning ("win32_seed_user_icon_theme: failed to read %s: %s",
			   source_index_theme, error ? error->message : "unknown error");
		g_clear_error (&error);
		goto cleanup;
	}

	if (!g_file_set_contents (user_index_theme, data, len, &error))
	{
		g_warning ("win32_seed_user_icon_theme: failed to write %s: %s",
			   user_index_theme, error ? error->message : "unknown error");
		g_clear_error (&error);
		goto cleanup;
	}

cleanup:
	g_clear_error (&error);
	g_free (data);
	g_free (source_index_theme);
	g_free (install_root);
	g_free (user_index_theme);
	g_free (user_icons_dir);
}
#endif
static gint arg_minimize = 0;

static const GOptionEntry gopt_entries[] = 
{
 {"no-auto",	'a', 0, G_OPTION_ARG_NONE,	&arg_dont_autoconnect, N_("Don't auto connect to servers"), NULL},
 {"cfgdir",	'd', 0, G_OPTION_ARG_STRING,	&arg_cfgdir, N_("Use a different config directory"), "PATH"},
 {"no-plugins",	'n', 0, G_OPTION_ARG_NONE,	&arg_skip_plugins, N_("Don't auto load any plugins"), NULL},
 {"plugindir",	'p', 0, G_OPTION_ARG_NONE,	&arg_show_autoload, N_("Show plugin/script auto-load directory"), NULL},
 {"configdir",	'u', 0, G_OPTION_ARG_NONE,	&arg_show_config, N_("Show user config directory"), NULL},
 {"url",	 0,  G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_STRING, &arg_url, N_("Open an irc://server:port/channel?key URL"), "URL"},
 {"command",	'c', 0, G_OPTION_ARG_STRING,	&arg_command, N_("Execute command:"), "COMMAND"},
#ifdef USE_DBUS
 {"existing",	'e', 0, G_OPTION_ARG_NONE,	&arg_existing, N_("Open URL or execute command in an existing Fabulor"), NULL},
#endif
 {"minimize",	 0,  0, G_OPTION_ARG_INT,	&arg_minimize, N_("Begin minimized. Level 0=Normal 1=Iconified 2=Tray"), N_("level")},
 {"version",	'v', 0, G_OPTION_ARG_NONE,	&arg_show_version, N_("Show version information"), NULL},
 {G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_STRING_ARRAY, &arg_urls, N_("Open an irc://server:port/channel?key URL"), "URL"},
 {NULL}
};

#ifdef WIN32
static void
create_msg_dialog (gchar *title, gchar *message)
{
	gunichar2 *wide_title;
	gunichar2 *wide_message;

	wide_title = g_utf8_to_utf16 (title, -1, NULL, NULL, NULL);
	wide_message = g_utf8_to_utf16 (message, -1, NULL, NULL, NULL);
	MessageBoxW (NULL,
	             wide_message ? (LPCWSTR)wide_message : L"Unable to display message.",
	             wide_title ? (LPCWSTR)wide_title : L"Fabulor",
	             MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
	g_free (wide_message);
	g_free (wide_title);
}

static char *win32_argv0_dir;

static char *
win32_fontconfig_xml_path (const char *path)
{
	char *normalized;
	char *escaped;
	char *p;

	normalized = g_strdup (path);
	if (normalized == NULL)
		return NULL;

	for (p = normalized; *p != '\0'; p++)
	{
		if (*p == '\\')
			*p = '/';
	}

	escaped = g_markup_escape_text (normalized, -1);
	g_free (normalized);
	return escaped;
}

static void
win32_configure_font_rendering (void)
{
	char *base_path;
	char *system_fonts_conf;
	char *emoji_font;
	char *emoji_font_dir;
	char *user_fontconfig_dir;
	char *user_fonts_conf;
	char *system_fonts_conf_xml;
	char *emoji_font_dir_xml;
	char *config;
	GError *error = NULL;

	base_path = g_win32_get_package_installation_directory_of_module (NULL);
	if (base_path == NULL)
		return;

	system_fonts_conf = g_build_filename (base_path, "etc", "fonts", "fonts.conf", NULL);
	emoji_font = g_build_filename (base_path, "share", "fonts", "NotoColorEmoji.ttf", NULL);
	if (!g_file_test (system_fonts_conf, G_FILE_TEST_IS_REGULAR) ||
	    !g_file_test (emoji_font, G_FILE_TEST_IS_REGULAR))
		goto cleanup_base;

	emoji_font_dir = g_path_get_dirname (emoji_font);
	system_fonts_conf_xml = win32_fontconfig_xml_path (system_fonts_conf);
	emoji_font_dir_xml = win32_fontconfig_xml_path (emoji_font_dir);
	if (system_fonts_conf_xml == NULL || emoji_font_dir_xml == NULL)
		goto cleanup_paths;

	user_fontconfig_dir = g_build_filename (get_xdir (), "fontconfig", NULL);
	user_fonts_conf = g_build_filename (user_fontconfig_dir, "fabulor-fonts.conf", NULL);

	if (g_mkdir_with_parents (user_fontconfig_dir, 0700) != 0)
		goto cleanup_config_paths;

	config = g_strdup_printf (
		"<?xml version=\"1.0\"?>\n"
		"<!DOCTYPE fontconfig SYSTEM \"urn:fontconfig:fonts.dtd\">\n"
		"<fontconfig>\n"
		"  <include>%s</include>\n"
		"  <dir>%s</dir>\n"
		"  <match target=\"pattern\">\n"
		"    <test qual=\"any\" name=\"family\">\n"
		"      <string>Segoe UI Emoji</string>\n"
		"    </test>\n"
		"    <edit name=\"family\" mode=\"prepend\" binding=\"strong\">\n"
		"      <string>Noto Color Emoji</string>\n"
		"    </edit>\n"
		"  </match>\n"
		"  <match target=\"pattern\">\n"
		"    <test qual=\"any\" name=\"family\">\n"
		"      <string>emoji</string>\n"
		"    </test>\n"
		"    <edit name=\"family\" mode=\"prepend\" binding=\"strong\">\n"
		"      <string>Noto Color Emoji</string>\n"
		"    </edit>\n"
		"  </match>\n"
		"</fontconfig>\n",
		system_fonts_conf_xml,
		emoji_font_dir_xml);

	if (!g_file_set_contents (user_fonts_conf, config, -1, &error))
	{
		g_warning ("win32_configure_font_rendering: failed to write %s: %s",
		           user_fonts_conf, error ? error->message : "unknown error");
		g_clear_error (&error);
		goto cleanup_config;
	}

	g_setenv ("PANGOCAIRO_BACKEND", "fc", FALSE);
	g_setenv ("FONTCONFIG_FILE", user_fonts_conf, FALSE);

cleanup_config:
	g_free (config);
cleanup_config_paths:
	g_free (user_fonts_conf);
	g_free (user_fontconfig_dir);
cleanup_paths:
	g_free (emoji_font_dir_xml);
	g_free (system_fonts_conf_xml);
	g_free (emoji_font_dir);
cleanup_base:
	g_free (emoji_font);
	g_free (system_fonts_conf);
	g_free (base_path);
}

static void
win32_set_appusermodelid (void)
{
	HMODULE shell32;
	HRESULT (WINAPI *set_appid) (PCWSTR);

	shell32 = GetModuleHandleW (L"shell32.dll");
	if (!shell32)
		shell32 = LoadLibraryW (L"shell32.dll");
	if (!shell32)
		return;

	set_appid = (HRESULT (WINAPI *) (PCWSTR)) GetProcAddress (shell32, "SetCurrentProcessExplicitAppUserModelID");
	if (!set_appid)
		return;

	set_appid (L"Fabulor.Desktop.Notify");
}

static void
win32_set_gsettings_schema_dir (void)
{
	char *base_path;
	char *share_path;
	char *runtime_share_path;
	char *schema_path;
	char *runtime_schema_path;
	char *xdg_data_dirs;
	char **xdg_parts;
	gboolean have_share_path = FALSE;
	gboolean have_runtime_share_path = FALSE;
	gint i;

	base_path = g_win32_get_package_installation_directory_of_module (NULL);
	if (base_path == NULL)
		return;

	share_path = g_build_filename (base_path, "share", NULL);
	runtime_share_path = g_build_filename (base_path, "Runtime", "GTK4", "share", NULL);

	/* Ensure GTK can discover bundled icon themes and other shared data. */
	xdg_data_dirs = g_strdup (g_getenv ("XDG_DATA_DIRS"));
	if (xdg_data_dirs && *xdg_data_dirs)
	{
		xdg_parts = g_strsplit (xdg_data_dirs, G_SEARCHPATH_SEPARATOR_S, -1);
		for (i = 0; xdg_parts[i] != NULL; i++)
		{
			if (g_ascii_strcasecmp (xdg_parts[i], share_path) == 0)
			{
				have_share_path = TRUE;
			}
			else if (g_ascii_strcasecmp (xdg_parts[i], runtime_share_path) == 0)
			{
				have_runtime_share_path = TRUE;
			}
		}
		g_strfreev (xdg_parts);

		if (!have_runtime_share_path || !have_share_path)
		{
			GString *updated = g_string_new ("");
			if (!have_runtime_share_path && g_file_test (runtime_share_path, G_FILE_TEST_IS_DIR))
			{
				g_string_append (updated, runtime_share_path);
				g_string_append_c (updated, G_SEARCHPATH_SEPARATOR);
			}
			if (!have_share_path)
			{
				g_string_append (updated, share_path);
				g_string_append_c (updated, G_SEARCHPATH_SEPARATOR);
			}
			g_string_append (updated, xdg_data_dirs);
			g_setenv ("XDG_DATA_DIRS", updated->str, TRUE);
			g_string_free (updated, TRUE);
		}
	}
	else
	{
		if (g_file_test (runtime_share_path, G_FILE_TEST_IS_DIR))
		{
			char *updated = g_strdup_printf ("%s%c%s", runtime_share_path,
								 G_SEARCHPATH_SEPARATOR,
								 share_path);
			g_setenv ("XDG_DATA_DIRS", updated, TRUE);
			g_free (updated);
		}
		else
		{
			g_setenv ("XDG_DATA_DIRS", share_path, TRUE);
		}
	}

	schema_path = g_build_filename (base_path, "share", "glib-2.0", "schemas", NULL);
	runtime_schema_path = g_build_filename (base_path, "Runtime", "GTK4", "schemas", NULL);
	if (g_getenv ("GSETTINGS_SCHEMA_DIR") == NULL
		&& g_file_test (schema_path, G_FILE_TEST_IS_DIR))
	{
		g_setenv ("GSETTINGS_SCHEMA_DIR", schema_path, FALSE);
	}
	else if (g_getenv ("GSETTINGS_SCHEMA_DIR") == NULL
		 && g_file_test (runtime_schema_path, G_FILE_TEST_IS_DIR))
	{
		g_setenv ("GSETTINGS_SCHEMA_DIR", runtime_schema_path, FALSE);
	}

	g_free (xdg_data_dirs);
	g_free (runtime_share_path);
	g_free (runtime_schema_path);
	g_free (share_path);
	g_free (schema_path);
	g_free (base_path);
}


static void
win32_prepare_pixbuf_cache (const char *module_dir, const char *module_file)
{
	static const char old_prefix[] = "\"lib\\\\gdk-pixbuf-2.0\\\\2.10.0\\\\loaders\\\\";
	char *cache_target;
	char *contents = NULL;
	gsize len = 0;
	GError *error = NULL;

	if (module_dir == NULL || module_file == NULL)
		return;

	cache_target = g_build_filename (get_xdir (), "gdk-pixbuf-loaders.cache", NULL);

	if (g_file_get_contents (module_file, &contents, &len, &error))
	{
		char *pos = g_strstr_len (contents, len, old_prefix);
		if (pos != NULL)
		{
			GString *fixed = g_string_sized_new (len + strlen (module_dir) + 16);
			char *replacement = g_strdup_printf ("\"%s\\", module_dir);

			g_string_append_len (fixed, contents, pos - contents);
			g_string_append (fixed, replacement);
			g_string_append (fixed, pos + strlen (old_prefix));
			g_file_set_contents (cache_target, fixed->str, fixed->len, NULL);

			g_free (replacement);
			g_string_free (fixed, TRUE);
			g_setenv ("GDK_PIXBUF_MODULE_FILE", cache_target, TRUE);
		}
		else
		{
			g_setenv ("GDK_PIXBUF_MODULE_FILE", module_file, TRUE);
		}
	}
	else
	{
		char *svg_loader = g_build_filename (module_dir, "pixbufloader_svg.dll", NULL);

		if (g_file_test (svg_loader, G_FILE_TEST_EXISTS))
		{
			char *generated = g_strdup_printf (
				"# GdkPixbuf Image Loader Modules file\n"
				"\"%s\"\n"
				"\"svg\" 6 \"gdk-pixbuf\" \"Scalable Vector Graphics\" \"LGPL\"\n"
				"\"image/svg+xml\" \"image/svg\" \"image/svg-xml\" \"image/vnd.adobe.svg+xml\" \"text/xml-svg\" \"image/svg+xml-compressed\" \"\"\n"
				"\"svg\" \"svgz\" \"svg.gz\" \"\"\n"
				"\" <svg\" \"*    \" 100\n"
				"\" <!DOCTYPE svg\" \"*             \" 100\n",
				svg_loader);
			g_file_set_contents (cache_target, generated, -1, NULL);
			g_free (generated);
			g_setenv ("GDK_PIXBUF_MODULE_FILE", cache_target, TRUE);
		}

		g_free (svg_loader);
		g_clear_error (&error);
	}

	g_free (contents);
	g_free (cache_target);
}

static void
win32_configure_pixbuf_loaders (void)
{
	char *base_path;
	char *pixbuf_root;
	GDir *versions;
	const gchar *entry;
	gboolean configured = FALSE;

	base_path = g_win32_get_package_installation_directory_of_module (NULL);
	if (!base_path)
		return;

	pixbuf_root = g_build_filename (base_path, "lib", "gdk-pixbuf-2.0", NULL);

	versions = g_file_test (pixbuf_root, G_FILE_TEST_IS_DIR) ? g_dir_open (pixbuf_root, 0, NULL) : NULL;

	if (versions != NULL)
	{
		while ((entry = g_dir_read_name (versions)) != NULL)
		{
			char *module_dir = NULL;
			char *module_file = NULL;

			if (g_file_test (pixbuf_root, G_FILE_TEST_IS_DIR))
			{
				module_dir = g_build_filename (pixbuf_root, entry, "loaders", NULL);
				module_file = g_build_filename (pixbuf_root, entry, "loaders.cache", NULL);
			}

			if (g_file_test (module_dir, G_FILE_TEST_IS_DIR))
			{
				g_setenv ("GDK_PIXBUF_MODULEDIR", module_dir, TRUE);
				win32_prepare_pixbuf_cache (module_dir, module_file);
				configured = TRUE;
			}
			else if (g_file_test (module_file, G_FILE_TEST_EXISTS))
			{
				g_setenv ("GDK_PIXBUF_MODULE_FILE", module_file, TRUE);
			}

			g_free (module_file);
			g_free (module_dir);

			if (configured)
				break;
		}
		g_dir_close (versions);
	}

	g_free (pixbuf_root);
	g_free (base_path);
}

static gboolean
win32_icon_path_has_payload (const char *path)
{
	char *fixed_apps;
	char *scalable_apps;
	gboolean has_payload;

	if (!path || !g_file_test (path, G_FILE_TEST_IS_DIR))
		return FALSE;

	fixed_apps = g_build_filename (path, "hicolor", "48x48", "apps", NULL);
	scalable_apps = g_build_filename (path, "hicolor", "scalable", "apps", NULL);
	has_payload = g_file_test (fixed_apps, G_FILE_TEST_IS_DIR) ||
	              g_file_test (scalable_apps, G_FILE_TEST_IS_DIR);
	g_free (fixed_apps);
	g_free (scalable_apps);

	return has_payload;
}

static gboolean
win32_icon_path_is_gtk3_safe (const char *path)
{
	char *index_theme;
	gboolean has_index_theme;

	if (!path || !g_file_test (path, G_FILE_TEST_IS_DIR))
		return FALSE;

	index_theme = g_build_filename (path, "hicolor", "index.theme", NULL);
	has_index_theme = g_file_test (index_theme, G_FILE_TEST_EXISTS);
	g_free (index_theme);

	/* The Windows GTK3 runtime can fail-fast while scanning a hicolor index.theme. */
	return !has_index_theme;
}

static void
win32_configure_icon_theme (void)
{
	GtkIconTheme *theme;
	const char *env_icons_path;
	char *base_path;
	char *icons_path;
	char *adwaita_index_theme;
	char *cwd_dir;
	char *cwd_path;
	char *argv0_icons_path;
	const char *selected_source = NULL;
	char *selected_path = NULL;

	#define WIN32_SET_ICON_PATH(source_name, path_value) \
		G_STMT_START { \
			if (win32_icon_path_is_gtk3_safe (path_value)) \
			{ \
				gtk_icon_theme_append_search_path (theme, (path_value)); \
				if (selected_path == NULL) \
				{ \
					selected_source = (source_name); \
					selected_path = g_strdup (path_value); \
				} \
			} \
		} G_STMT_END

	theme = gtk_icon_theme_get_default ();
	if (!theme)
		return;

	win32_seed_user_icon_theme ();

	env_icons_path = g_getenv ("ZOITECHAT_ICON_PATH");
	if (env_icons_path && *env_icons_path)
		WIN32_SET_ICON_PATH ("ZOITECHAT_ICON_PATH", env_icons_path);

	icons_path = g_build_filename (get_xdir (), "icons", NULL);
	if (win32_icon_path_has_payload (icons_path))
		WIN32_SET_ICON_PATH ("user config/icons", icons_path);
	g_free (icons_path);

	base_path = g_win32_get_package_installation_directory_of_module (NULL);
	if (base_path)
	{
		icons_path = g_build_filename (base_path, "Runtime", "GTK4", "share", "icons", NULL);
		WIN32_SET_ICON_PATH ("module base/Runtime/GTK4/share/icons", icons_path);
		g_free (icons_path);

		icons_path = g_build_filename (base_path, "icons", NULL);
		WIN32_SET_ICON_PATH ("module base/icons", icons_path);
		g_free (icons_path);

		icons_path = g_build_filename (base_path, "share", "icons", NULL);
		WIN32_SET_ICON_PATH ("module base/share/icons", icons_path);
		g_free (icons_path);

		adwaita_index_theme = g_build_filename (base_path, "share", "icons", "Adwaita", "index.theme", NULL);
		if (g_file_test (adwaita_index_theme, G_FILE_TEST_IS_REGULAR))
		{
			gtk_icon_theme_set_custom_theme (theme, "Adwaita");
			gtk_icon_theme_rescan_if_needed (theme);
		}
		g_free (adwaita_index_theme);
	}

	cwd_dir = g_get_current_dir ();
	cwd_path = g_build_filename (cwd_dir, "icons", NULL);
	WIN32_SET_ICON_PATH ("current working directory/icons", cwd_path);
	g_free (cwd_path);

	cwd_path = g_build_filename (cwd_dir, "share", "icons", NULL);
	WIN32_SET_ICON_PATH ("current working directory/share/icons", cwd_path);
	g_free (cwd_path);
	g_free (cwd_dir);

	if (win32_argv0_dir)
	{
		argv0_icons_path = g_build_filename (win32_argv0_dir, "icons", NULL);
		WIN32_SET_ICON_PATH ("argv[0] directory/icons", argv0_icons_path);
		g_free (argv0_icons_path);

		argv0_icons_path = g_build_filename (win32_argv0_dir, "share", "icons", NULL);
		WIN32_SET_ICON_PATH ("argv[0] directory/share/icons", argv0_icons_path);
		g_free (argv0_icons_path);
	}

	if (selected_path)
		g_message ("win32_configure_icon_theme: selected icon path (%s): %s", selected_source, selected_path);
	else
		g_message ("win32_configure_icon_theme: no usable icon path found (checked ZOITECHAT_ICON_PATH, user config/icons, module base/icons, module base/share/icons, cwd/icons, cwd/share/icons, argv[0]/icons, argv[0]/share/icons)");

	g_free (selected_path);
	g_free (base_path);

	#undef WIN32_SET_ICON_PATH
}
#endif

int
fe_args (int argc, char *argv[])
{
	GError *error = NULL;
	GOptionContext *context;
	char *buffer;
	const char *desktop_id = "net.fabulor.Fabulor";
#ifdef WIN32
	char *base_path = NULL;
	char *locale_path = NULL;
#endif

#ifdef ENABLE_NLS
#ifdef WIN32
	base_path = g_win32_get_package_installation_directory_of_module (NULL);
	if (base_path)
	{
		locale_path = g_build_filename (base_path, "share", "locale", NULL);
		bindtextdomain (GETTEXT_PACKAGE, locale_path);
	}
	else
	{
		bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	}
	g_free (locale_path);
	g_free (base_path);
#else
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
#endif
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	context = g_option_context_new (NULL);
#ifdef WIN32
	g_option_context_set_help_enabled (context, FALSE);	/* disable stdout help as stdout is unavailable for subsystem:windows */
#endif
	g_option_context_add_main_entries (context, gopt_entries, GETTEXT_PACKAGE);
#if GTK_MAJOR_VERSION < 4
	g_option_context_add_group (context, gtk_get_option_group (FALSE));
#endif
	g_option_context_parse (context, &argc, &argv, &error);

#ifdef WIN32
	if (error)											/* workaround for argv not being available when using subsystem:windows */
	{
		if (error->message)								/* the error message contains argv so search for patterns in that */
		{
			if (strstr (error->message, "--help-all") != NULL)
			{
				buffer = g_option_context_get_help (context, FALSE, NULL);
				create_msg_dialog ("Long Help", buffer);
				g_free (buffer);
				return 0;
			}
			else if (strstr (error->message, "--help") != NULL || strstr (error->message, "-?") != NULL)
			{
				buffer = g_option_context_get_help (context, TRUE, NULL);
				create_msg_dialog ("Help", buffer);
				g_free (buffer);
				return 0;
			}
			else 
			{
				buffer = g_strdup_printf ("%s\n", error->message);
				create_msg_dialog ("Error", buffer);
				g_free (buffer);
				return 1;
			}
		}
	}
#else
	if (error)
	{
		if (error->message)
			printf ("%s\n", error->message);
		return 1;
	}
#endif

	g_option_context_free (context);

	if (arg_show_version)
	{
		buffer = g_strdup_printf ("%s %s", PACKAGE_NAME, PACKAGE_VERSION);
#ifdef WIN32
		create_msg_dialog ("Version Information", buffer);
#else
		puts (buffer);
#endif
		g_free (buffer);

		return 0;
	}

	if (arg_show_autoload)
	{
		buffer = g_strdup_printf ("%s%caddons%c", get_xdir(), G_DIR_SEPARATOR, G_DIR_SEPARATOR);
#ifdef WIN32
		create_msg_dialog ("Plugin/Script Auto-load Directory", buffer);
#else
		puts (buffer);
#endif
		g_free (buffer);

		return 0;
	}

	if (arg_show_config)
	{
		buffer = g_strdup_printf ("%s%c", get_xdir(), G_DIR_SEPARATOR);
#ifdef WIN32
		create_msg_dialog ("User Config Directory", buffer);
#else
		puts (buffer);
#endif
		g_free (buffer);

		return 0;
	}

#ifdef WIN32
	win32_configure_font_rendering ();
	win32_set_gsettings_schema_dir ();
	win32_set_appusermodelid ();
	win32_configure_pixbuf_loaders ();

	/* this is mainly for irc:// URL handling. When windows calls us from */
	/* I.E, it doesn't give an option of "Start in" directory, like short */
	/* cuts can. So we have to set the current dir manually, to the path  */
	/* of the exe. */
	{
		g_free (win32_argv0_dir);
		win32_argv0_dir = g_path_get_dirname (argv[0]);
		if (win32_argv0_dir)
			chdir (win32_argv0_dir);
	}
#endif

	g_set_prgname (desktop_id);
#ifndef WIN32
	gdk_set_program_class (desktop_id);
#endif
#if GTK_MAJOR_VERSION < 4
	gtk_init (&argc, &argv);
#else
	gtk_init ();
#endif

#ifdef WIN32
	win32_configure_icon_theme ();
#endif

	return -1;
}

#ifdef G_OS_WIN32
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

gboolean
fe_win32_high_contrast_is_enabled (void)
{
	HIGHCONTRASTW hc;

	ZeroMemory (&hc, sizeof (hc));
	hc.cbSize = sizeof (hc);
	if (!SystemParametersInfoW (SPI_GETHIGHCONTRAST, sizeof (hc), &hc, 0))
		return FALSE;

	return (hc.dwFlags & HCF_HIGHCONTRASTON) != 0;
}

gboolean
fe_win32_try_get_system_dark (gboolean *prefer_dark)
{
	DWORD value = 1;
	DWORD value_size = sizeof (value);
	LSTATUS status;

	status = RegGetValueW (HKEY_CURRENT_USER,
	                       L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
	                       L"AppsUseLightTheme",
	                       RRF_RT_REG_DWORD,
	                       NULL,
	                       &value,
	                       &value_size);
	if (status != ERROR_SUCCESS)
		status = RegGetValueW (HKEY_CURRENT_USER,
		                       L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
		                       L"SystemUsesLightTheme",
		                       RRF_RT_REG_DWORD,
		                       NULL,
		                       &value,
		                       &value_size);

	if (status != ERROR_SUCCESS)
		return FALSE;

	*prefer_dark = (value == 0);
	return TRUE;
}

void
fe_win32_apply_native_titlebar (GtkWidget *window, gboolean dark_mode)
{
	HWND hwnd;
	BOOL use_dark;

	if (!window || !gtk_widget_get_realized (window))
		return;

	hwnd = (HWND) fabulor_window_native_handle (GTK_WINDOW (window));
	if (!hwnd)
		return;

	if (fe_win32_high_contrast_is_enabled ())
		return;

	use_dark = dark_mode ? TRUE : FALSE;
	DwmSetWindowAttribute (hwnd,
	                       DWMWA_USE_IMMERSIVE_DARK_MODE,
	                       &use_dark,
	                       sizeof (use_dark));
}
#else
void
fe_win32_apply_native_titlebar (GtkWidget *window, gboolean dark_mode)
{
	(void) window;
	(void) dark_mode;
}
#endif

static gboolean auto_dark_mode_enabled = FALSE;

void
fe_set_auto_dark_mode_state (gboolean enabled)
{
	auto_dark_mode_enabled = enabled;
}

gboolean
fe_dark_mode_is_enabled_for (unsigned int mode)
{
	switch (mode)
	{
	case ZOITECHAT_DARK_MODE_DARK:
		return TRUE;
	case ZOITECHAT_DARK_MODE_LIGHT:
		return FALSE;
	case ZOITECHAT_DARK_MODE_AUTO:
	default:
		return auto_dark_mode_enabled;
	}
}

gboolean
fe_dark_mode_is_enabled (void)
{
	return fe_dark_mode_is_enabled_for (prefs.hex_gui_dark_mode);
}

void
fe_init (void)
{
#if GTK_MAJOR_VERSION >= 4
	application_main_loop = fabulor_application_main_loop_new ();
#endif
	theme_manager_init ();
	key_init ();
	pixmaps_init ();

	channelwin_pix = pixmap_load_from_file (prefs.hex_text_background);
	theme_application_reload_input_style ();

}

void
fe_main (void)
{
#if GTK_MAJOR_VERSION < 4
	gtk_main ();
#else
	if (!application_main_loop)
		application_main_loop = fabulor_application_main_loop_new ();
	fabulor_application_main_loop_run (application_main_loop);
	g_clear_pointer (&application_main_loop,
		fabulor_application_main_loop_free);
#endif

	/* sleep for 2 seconds so any QUIT messages are not lost. The  */
	/* GUI is closed at this point, so the user doesn't even know! */
	if (prefs.wait_on_exit)
		sleep (2);
}

void
fe_cleanup (void)
{
}

fe_preferences_save_result
fe_preferences_persistence_save_all (void)
{
	PreferencesPersistenceResult save_result;
	fe_preferences_save_result result;

	save_result = preferences_persistence_save_all ();
	result.success = save_result.success;
	result.partial_failure = save_result.partial_failure;
	result.config_failed = save_result.config_failed;
	result.theme_failed = save_result.theme_failed;

	return result;
}

void
fe_exit (void)
{
#if GTK_MAJOR_VERSION < 4
	gtk_main_quit ();
#else
	if (application_main_loop)
		fabulor_application_main_loop_request_quit (application_main_loop);
#endif
}

int
fe_timeout_add (int interval, void *callback, void *userdata)
{
	return g_timeout_add (interval, (GSourceFunc) callback, userdata);
}

int
fe_timeout_add_seconds (int interval, void *callback, void *userdata)
{
	return g_timeout_add_seconds (interval, (GSourceFunc) callback, userdata);
}

void
fe_timeout_remove (int tag)
{
	g_source_remove (tag);
}

#ifdef WIN32

static void
log_handler (const gchar   *log_domain,
		       GLogLevelFlags log_level,
		       const gchar   *message,
		       gpointer	      unused_data)
{
	session *sess;

	return;

	sess = find_dialog (serv_list->data, "(warnings)");
	if (!sess)
		sess = new_ircwindow (serv_list->data, "(warnings)", SESS_DIALOG, 0);

	PrintTextf (sess, "%s\t%s\n", log_domain, message);
	if (getenv ("ZOITECHAT_WARNING_ABORT"))
		abort ();
}

#endif

static int
fe_idle (gpointer data)
{
	session *sess = data;

	if (!is_session (sess))
	{
		if (!sess_list)
			return 0;
		sess = sess_list->data;
	}

	plugin_add (sess, NULL, NULL, notification_plugin_init, notification_plugin_deinit, NULL, FALSE);

	plugin_add (sess, NULL, NULL, tray_plugin_init, tray_plugin_deinit, NULL, FALSE);
	plugin_print_startup_report (sess);

	if (arg_minimize == 1)
		gtk_window_iconify (GTK_WINDOW (sess->gui->window));
	else if (arg_minimize == 2)
		tray_toggle_visibility (FALSE);

	return 0;
}

void
fe_new_window (session *sess, int focus)
{
	int tab = FALSE;

	if (sess->type == SESS_DIALOG)
	{
		if (prefs.hex_gui_tab_dialogs)
			tab = TRUE;
	} else
	{
		if (prefs.hex_gui_tab_chans)
			tab = TRUE;
	}

	mg_changui_new (sess, NULL, tab, focus);

#ifdef WIN32
	g_log_set_handler ("GLib", G_LOG_LEVEL_CRITICAL|G_LOG_LEVEL_WARNING, (GLogFunc)log_handler, 0);
	g_log_set_handler ("GLib-GObject", G_LOG_LEVEL_CRITICAL|G_LOG_LEVEL_WARNING, (GLogFunc)log_handler, 0);
	g_log_set_handler ("Gdk", G_LOG_LEVEL_CRITICAL|G_LOG_LEVEL_WARNING, (GLogFunc)log_handler, 0);
	g_log_set_handler ("Gtk", G_LOG_LEVEL_CRITICAL|G_LOG_LEVEL_WARNING, (GLogFunc)log_handler, 0);
#endif

	if (!sess_list->next)
		g_idle_add (fe_idle, sess);

	sess->scrollback_replay_marklast = gtk_xtext_set_marker_last;
}

void
fe_new_server (struct server *serv)
{
	serv->gui = g_new0 (struct server_gui, 1);
}

void
fe_message (char *msg, int flags)
{
	GtkWidget *dialog;
	int type = GTK_MESSAGE_WARNING;

	if (flags & FE_MSG_ERROR)
		type = GTK_MESSAGE_ERROR;
	if (flags & FE_MSG_INFO)
		type = GTK_MESSAGE_INFO;

	dialog = gtk_message_dialog_new (GTK_WINDOW (parent_window), 0, type,
												GTK_BUTTONS_OK, "%s", msg);
	theme_manager_attach_window (dialog);
	if (flags & FE_MSG_MARKUP)
		gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), msg);
	g_signal_connect (G_OBJECT (dialog), "response",
							G_CALLBACK (fabulor_gtk_dialog_destroy_on_response), NULL);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	fabulor_gtk_window_position_at_pointer (GTK_WINDOW (dialog));
	gtk_window_set_modal (GTK_WINDOW (dialog), (flags & FE_MSG_MODAL) != 0);
	gtk_widget_show (dialog);
}

void
fe_idle_add (void *func, void *data)
{
	g_idle_add (func, data);
}

void
fe_input_remove (int tag)
{
	g_source_remove (tag);
}

int
fe_input_add (int sok, int flags, void *func, void *data)
{
	int tag, type = 0;
	GIOChannel *channel;

#ifdef WIN32
	if (flags & FIA_FD)
		channel = g_io_channel_win32_new_fd (sok);
	else
		channel = g_io_channel_win32_new_socket (sok);
#else
	channel = g_io_channel_unix_new (sok);
#endif

	if (flags & FIA_READ)
		type |= G_IO_IN | G_IO_HUP | G_IO_ERR;
	if (flags & FIA_WRITE)
		type |= G_IO_OUT | G_IO_ERR;
	if (flags & FIA_EX)
		type |= G_IO_PRI;

	tag = g_io_add_watch (channel, type, (GIOFunc) func, data);
	g_io_channel_unref (channel);

	return tag;
}

void
fe_set_topic (session *sess, char *topic, char *stripped_topic)
{
	if (!sess->gui->is_tab || sess == current_tab)
	{
		GtkTextBuffer *topic_buffer;
		GtkTextIter start;

		topic_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sess->gui->topic_entry));
		if (prefs.hex_text_stripcolor_topic)
			gtk_text_buffer_set_text (topic_buffer, stripped_topic, -1);
		else
			gtk_text_buffer_set_text (topic_buffer, topic, -1);

		gtk_text_buffer_get_start_iter (topic_buffer, &start);
		gtk_text_buffer_place_cursor (topic_buffer, &start);
		gtk_text_view_scroll_to_iter (GTK_TEXT_VIEW (sess->gui->topic_entry),
			&start, 0.0, FALSE, 0.0, 0.0);
		mg_set_topic_tip (sess);
	}
	else
	{
		g_free (sess->res->topic_text);

		if (prefs.hex_text_stripcolor_topic)
		{
			sess->res->topic_text = g_strdup (stripped_topic);
		}
		else
		{
			sess->res->topic_text = g_strdup (topic);
		}
	}
}

static void
fe_update_mode_entry (session *sess, GtkWidget *entry, char **text, char *new_text)
{
	if (!sess->gui->is_tab || sess == current_tab)
	{
		if (sess->gui->flag_wid[0])	/* channel mode buttons enabled? */
			fabulor_gtk_entry_set_text (GTK_ENTRY (entry), new_text);
	} else
	{
		if (sess->gui->is_tab)
		{
			g_free (*text);
			*text = g_strdup (new_text);
		}
	}
}

void
fe_update_channel_key (struct session *sess)
{
	fe_update_mode_entry (sess, sess->gui->key_entry,
								 &sess->res->key_text, sess->channelkey);
	fe_set_title (sess);
}

void
fe_update_channel_limit (struct session *sess)
{
	char tmp[16];

	sprintf (tmp, "%d", sess->limit);
	fe_update_mode_entry (sess, sess->gui->limit_entry,
								 &sess->res->limit_text, tmp);
	fe_set_title (sess);
}

int
fe_is_chanwindow (struct server *serv)
{
	if (!serv->gui->chanlist_window)
		return 0;
	return 1;
}

void
fe_notify_update (char *name)
{
	if (!name)
		notify_gui_update ();
}

void
fe_text_clear (struct session *sess, int lines)
{
	gtk_xtext_clear (sess->res->buffer, lines);
}

void
fe_close_window (struct session *sess)
{
	if (sess->gui->is_tab)
		mg_tab_close (sess);
	else
		fabulor_gtk_window_destroy (GTK_WINDOW (sess->gui->window));
}

void
fe_progressbar_start (session *sess)
{
	if (!sess->gui->is_tab || current_tab == sess)
	/* if it's the focused tab, create it for real! */
		mg_progressbar_create (sess->gui);
	else
	/* otherwise just remember to create on when it gets focused */
		sess->res->c_graph = TRUE;
}

void
fe_progressbar_end (server *serv)
{
	GSList *list = sess_list;
	session *sess;

	while (list)				  /* check all windows that use this server and  *
									   * remove the connecting graph, if it has one. */
	{
		sess = list->data;
		if (sess->server == serv)
		{
			if (sess->gui->bar)
				mg_progressbar_destroy (sess->gui);
			sess->res->c_graph = FALSE;
		}
		list = list->next;
	}
}

void
fe_print_text (struct session *sess, char *text, time_t stamp,
			   gboolean no_activity)
{
	PrintTextRaw (sess->res->buffer, (unsigned char *)text, prefs.hex_text_indent, stamp);

	if (no_activity || !sess->gui->is_tab)
		return;

	if (sess == current_tab)
		fe_set_tab_color (sess, FE_COLOR_NONE);
	else if (sess->tab_state & TAB_STATE_NEW_HILIGHT)
		fe_set_tab_color (sess, FE_COLOR_NEW_HILIGHT);
	else if (sess->tab_state & TAB_STATE_NEW_MSG)
		fe_set_tab_color (sess, FE_COLOR_NEW_MSG);
	else
		fe_set_tab_color (sess, FE_COLOR_NEW_DATA);
}

void
fe_beep (session *sess)
{
#ifdef WIN32
	/* Play the "Instant Message Notification" system sound
	 */
	if (!PlaySoundW (L"Notification.IM", NULL, SND_ALIAS | SND_ASYNC))
	{
		/* The user does not have the "Instant Message Notification" sound set. Fall back to system beep.
		 */
		Beep (1000, 50);
	}
#else
#ifdef USE_LIBCANBERRA
	if (ca_con == NULL)
	{
		ca_context_create (&ca_con);
		ca_context_change_props (ca_con,
										CA_PROP_APPLICATION_ID, "fabulor",
										CA_PROP_APPLICATION_NAME, DISPLAY_NAME,
										CA_PROP_APPLICATION_ICON_NAME, "fabulor", NULL);
	}

	if (ca_context_play (ca_con, 0, CA_PROP_EVENT_ID, "message-new-instant", NULL) != 0)
#endif
	gdk_display_beep (gdk_display_get_default ());
#endif
}

void
fe_lastlog (session *sess, session *lastlog_sess, char *sstr, gtk_xtext_search_flags flags)
{
	GError *err = NULL;
	xtext_buffer *buf, *lbuf;

	buf = sess->res->buffer;

	if (gtk_xtext_is_empty (buf))
	{
		PrintText (lastlog_sess, _("Search buffer is empty.\n"));
		return;
	}

	lbuf = lastlog_sess->res->buffer;
	if (flags & regexp)
	{
		GRegexCompileFlags gcf = (flags & case_match)? 0: G_REGEX_CASELESS;

		lbuf->search_re = g_regex_new (sstr, gcf, 0, &err);
		if (err)
		{
			PrintText (lastlog_sess, _(err->message));
			g_error_free (err);
			return;
		}
	}
	else
	{
		if (flags & case_match)
		{
			lbuf->search_nee = g_strdup (sstr);
		}
		else
		{
			lbuf->search_nee = g_utf8_casefold (sstr, strlen (sstr));
		}
		lbuf->search_lnee = strlen (lbuf->search_nee);
	}
	lbuf->search_flags = flags;
	lbuf->search_text = g_strdup (sstr);
	gtk_xtext_lastlog (lbuf, buf);
}

void
fe_set_lag (server *serv, long lag)
{
	GSList *list = sess_list;
	session *sess;
	gdouble per;
	char lagtext[64];
	char lagtip[128];
	unsigned long nowtim;

	if (lag == -1)
	{
		if (!serv->lag_sent)
			return;
		nowtim = make_ping_time ();
		lag = nowtim - serv->lag_sent;
	}

	/* if there is no pong for >30s report the lag as +30s */
	if (lag > 30000 && serv->lag_sent)
		lag=30000;

	per = ((double)lag) / 1000.0;
	if (per > 1.0)
		per = 1.0;

	g_snprintf (lagtext, sizeof (lagtext) - 1, "%s%ld.%lds",
			  serv->lag_sent ? "+" : "", lag / 1000, (lag/100) % 10);
	g_snprintf (lagtip, sizeof (lagtip) - 1, "Lag: %s%ld.%ld seconds",
				 serv->lag_sent ? "+" : "", lag / 1000, (lag/100) % 10);

	while (list)
	{
		sess = list->data;
		if (sess->server == serv)
		{
			g_free (sess->res->lag_tip);
			sess->res->lag_tip = g_strdup (lagtip);

			if (!sess->gui->is_tab || current_tab == sess)
			{
				if (sess->gui->lagometer)
				{
					gtk_progress_bar_set_fraction ((GtkProgressBar *) sess->gui->lagometer, per);
					gtk_widget_set_tooltip_text (gtk_widget_get_parent (sess->gui->lagometer), lagtip);
				}
				if (sess->gui->laginfo)
					gtk_label_set_text ((GtkLabel *) sess->gui->laginfo, lagtext);
			} else
			{
				sess->res->lag_value = per;
				g_free (sess->res->lag_text);
				sess->res->lag_text = g_strdup (lagtext);
			}
		}
		list = list->next;
	}
}

void
fe_set_throttle (server *serv)
{
	GSList *list = sess_list;
	struct session *sess;
	float per;
	char tbuf[96];
	char tip[160];

	per = (float) serv->sendq_len / 1024.0;
	if (per > 1.0)
		per = 1.0;

	while (list)
	{
		sess = list->data;
		if (sess->server == serv)
		{
			g_snprintf (tbuf, sizeof (tbuf) - 1, _("%d bytes"), serv->sendq_len);
			g_snprintf (tip, sizeof (tip) - 1, _("Network send queue: %d bytes"), serv->sendq_len);

			g_free (sess->res->queue_tip);
			sess->res->queue_tip = g_strdup (tip);

			if (!sess->gui->is_tab || current_tab == sess)
			{
				if (sess->gui->throttlemeter)
				{
					gtk_progress_bar_set_fraction ((GtkProgressBar *) sess->gui->throttlemeter, per);
					gtk_widget_set_tooltip_text (gtk_widget_get_parent (sess->gui->throttlemeter), tip);
				}
				if (sess->gui->throttleinfo)
					gtk_label_set_text ((GtkLabel *) sess->gui->throttleinfo, tbuf);
			} else
			{
				sess->res->queue_value = per;
				g_free (sess->res->queue_text);
				sess->res->queue_text = g_strdup (tbuf);
			}
		}
		list = list->next;
	}
}

void
fe_ctrl_gui (session *sess, fe_gui_action action, int arg)
{
	switch (action)
	{
	case FE_GUI_HIDE:
		gtk_widget_hide (sess->gui->window); break;
	case FE_GUI_SHOW:
		gtk_widget_show (sess->gui->window);
		gtk_window_present (GTK_WINDOW (sess->gui->window));
		break;
	case FE_GUI_FOCUS:
		mg_bring_tofront_sess (sess); break;
	case FE_GUI_FLASH:
		fe_flash_window (sess); break;
	case FE_GUI_COLOR:
		fe_set_tab_color (sess, arg); break;
	case FE_GUI_ICONIFY:
		gtk_window_iconify (GTK_WINDOW (sess->gui->window)); break;
	case FE_GUI_MENU:
		menu_bar_toggle ();	/* toggle menubar on/off */
		break;
	case FE_GUI_ATTACH:
		mg_detach (sess, arg);	/* arg: 0=toggle 1=detach 2=attach */
		break;
	case FE_GUI_APPLY:
		theme_manager_dispatch_changed (THEME_CHANGED_REASON_PIXMAP | THEME_CHANGED_REASON_USERLIST | THEME_CHANGED_REASON_LAYOUT | THEME_CHANGED_REASON_WIDGET_STYLE);
	}
}

static void
dcc_saveas_cb (struct DCC *dcc, char *file)
{
	if (is_dcc (dcc))
	{
		if (dcc->dccstat == STAT_QUEUED)
		{
			if (file)
				dcc_get_with_destfile (dcc, file);
			else if (dcc->resume_sent == 0)
				dcc_abort (dcc->serv->front_session, dcc);
		}
	}
}

void
fe_confirm (const char *message, void (*yesproc)(void *), void (*noproc)(void *), void *ud)
{
	/* warning, assuming fe_confirm is used by DCC only! */
	struct DCC *dcc = ud;

	if (dcc->file)
	{
		char *filepath = g_build_filename (prefs.hex_dcc_dir, dcc->file, NULL);
		gtkutil_file_req (NULL, message, dcc_saveas_cb, ud, filepath, NULL,
								FRF_WRITE|FRF_NOASKOVERWRITE|FRF_FILTERISINITIAL);
		g_free (filepath);
	}
}

int
fe_gui_info (session *sess, int info_type)
{
	FabulorWindowState state;

	switch (info_type)
	{
	case 0:	/* window status */
		if (!gtk_widget_get_visible (GTK_WIDGET (sess->gui->window)))
		{
			return 2;	/* hidden (iconified or systray) */
		}

		fabulor_window_state_get (GTK_WINDOW (sess->gui->window), &state);
		if (state.minimized)
			return 2;

		if (gtk_window_is_active (GTK_WINDOW (sess->gui->window)))
		{
			return 1;	/* active/focused */
		}

		return 0;		/* normal (no keyboard focus or behind a window) */
	}

	return -1;
}

void *
fe_gui_info_ptr (session *sess, int info_type)
{
	switch (info_type)
	{
	case 0:	/* native window pointer (for plugins) */
#ifdef GDK_WINDOWING_WIN32
		return fabulor_window_native_handle (GTK_WINDOW (sess->gui->window));
#else
		return sess->gui->window;
#endif
		break;

	case 1:	/* GtkWindow * (for plugins) */
		return sess->gui->window;
	}
	return NULL;
}

char *
fe_get_inputbox_contents (session *sess)
{
	/* not the current tab */
	if (sess->res->input_text)
		return sess->res->input_text;

	/* current focused tab */
	return SPELL_ENTRY_GET_TEXT (sess->gui->input_box);
}

int
fe_get_inputbox_cursor (session *sess)
{
	/* not the current tab (we don't remember the cursor pos) */
	if (sess->res->input_text)
		return 0;

	/* current focused tab */
	return SPELL_ENTRY_GET_POS (sess->gui->input_box);
}

void
fe_set_inputbox_cursor (session *sess, int delta, int pos)
{
	if (!sess->gui->is_tab || sess == current_tab)
	{
		if (delta)
			pos += SPELL_ENTRY_GET_POS (sess->gui->input_box);
		SPELL_ENTRY_SET_POS (sess->gui->input_box, pos);
	} else
	{
		/* we don't support changing non-front tabs yet */
	}
}

void
fe_set_inputbox_contents (session *sess, char *text)
{
	if (!sess->gui->is_tab || sess == current_tab)
	{
		SPELL_ENTRY_SET_TEXT (sess->gui->input_box, text);
	} else
	{
		g_free (sess->res->input_text);
		sess->res->input_text = g_strdup (text);
	}
}

static inline char *
escape_uri (const char *uri)
{
	return g_uri_escape_string(uri, G_URI_RESERVED_CHARS_GENERIC_DELIMITERS G_URI_RESERVED_CHARS_SUBCOMPONENT_DELIMITERS, FALSE);
}

static inline gboolean
uri_contains_forbidden_characters (const char *uri)
{
	while (*uri)
	{
		if (!g_ascii_isalnum (*uri) && !strchr ("-._~%:/?#[]@!$&'()*+,;=", *uri))
			return TRUE;
		uri++;
	}

	return FALSE;
}

static char *
maybe_escape_uri (const char *uri)
{
	/* The only way to know if a string has already been escaped or not
	 * is by fulling parsing each segement but we can try some more simple heuristics. */

	/* If we find characters that should clearly be escaped. */
	if (uri_contains_forbidden_characters (uri))
		return escape_uri (uri);

	/* If it fails to be unescaped then it was not escaped. */
	char *unescaped = g_uri_unescape_string (uri, NULL);
	if (!unescaped)
		return escape_uri (uri);
	g_free (unescaped);

	/* At this point it is probably safe to pass through as-is. */
	return g_strdup (uri);
}

#ifndef WIN32
static gchar **
fe_open_url_env_sanitized (void)
{
	gchar **env = g_get_environ ();
	const char *vars[] = {"LD_LIBRARY_PATH", "LD_PRELOAD", "APPDIR", "APPIMAGE", "ARGV0", NULL};
	int i;

	for (i = 0; vars[i]; i++)
	{
		gchar **tmp_env = env;
		env = g_environ_unsetenv (tmp_env, vars[i]);
		if (env != tmp_env)
			g_strfreev (tmp_env);
	}

	return env;
}
#endif

static void
fe_open_url_inner (const char *url)
{
	GError *error = NULL;
	char *escaped_url = maybe_escape_uri (url);
	gboolean opened = FALSE;

#ifdef WIN32
	opened = g_app_info_launch_default_for_uri (escaped_url, NULL, &error);
	if (!opened)
	{
		g_clear_error (&error);
		gunichar2 *url_utf16 = g_utf8_to_utf16 (escaped_url, -1, NULL, NULL, NULL);

		if (url_utf16 != NULL)
		{
			opened = ((INT_PTR) ShellExecuteW (0, L"open", url_utf16, NULL, NULL, SW_SHOWNORMAL)) > 32;
			g_free (url_utf16);
		}
	}
#else
	{
		gchar *xdg_open_argv[] = {(gchar *) "xdg-open", escaped_url, NULL};
		gchar **spawn_env = fe_open_url_env_sanitized ();

		if (g_spawn_async (NULL, xdg_open_argv, spawn_env,
							 G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
							 NULL, NULL, NULL, &error))
		{
			opened = TRUE;
		}
		else
		{
			g_clear_error (&error);
		}

		g_strfreev (spawn_env);
	}

	if (!opened)
	{
		opened = g_app_info_launch_default_for_uri (escaped_url, NULL, &error);
		if (!opened)
			g_clear_error (&error);
	}

	if (!opened && gtk_show_uri_on_window (NULL, escaped_url, GDK_CURRENT_TIME, &error))
	{
		opened = TRUE;
	}
	else if (!opened)
	{
		g_clear_error (&error);
	}
#endif

	if (!opened)
	{
		g_warning ("Unable to open URL '%s' using system default application", escaped_url);
	}

	g_free (escaped_url);
}

static gboolean
fe_open_url_is_local_path (const char *url)
{
	if (g_path_is_absolute (url) || g_file_test (url, G_FILE_TEST_EXISTS))
		return TRUE;

#ifdef WIN32
	if (g_ascii_isalpha (url[0]) && url[1] == ':' &&
		(url[2] == '\\' || url[2] == '/'))
		return TRUE;

	if (url[0] == '\\' && url[1] == '\\')
		return TRUE;
#endif

	return FALSE;
}

static char *
fe_open_url_canonicalize_path (const char *path)
{
	char *absolute_path;
	char *cwd;

	if (!path || path[0] == '\0')
		return NULL;

	if (g_path_is_absolute (path))
		return g_strdup (path);

	cwd = g_get_current_dir ();
	absolute_path = g_build_filename (cwd, path, NULL);
	g_free (cwd);
	return absolute_path;
}

void
fe_open_url (const char *url)
{
	int url_type = url_check_word (url);
	char *uri;
	char *path;
	char *path_uri;

	if (fe_open_url_is_local_path (url))
	{
		path = fe_open_url_canonicalize_path (url);
		path_uri = g_filename_to_uri (path, NULL, NULL);
		g_free (path);

		if (path_uri)
		{
			fe_open_url_inner (path_uri);
			g_free (path_uri);
			return;
		}
	}

	/* gvfs likes file:// */
	if (url_type == WORD_PATH)
	{
		path = fe_open_url_canonicalize_path (url);
		path_uri = g_filename_to_uri (path, NULL, NULL);
		g_free (path);
		if (path_uri)
		{
			fe_open_url_inner (path_uri);
			g_free (path_uri);
		}
		else
		{
			fe_open_url_inner (url);
		}
	}
	/* IPv6 addr. Add http:// */
	else if (url_type == WORD_HOST6)
	{
		/* IPv6 addrs in urls should be enclosed in [ ] */
		if (*url != '[')
			uri = g_strdup_printf ("http://[%s]", url);
		else
			uri = g_strdup_printf ("http://%s", url);

		fe_open_url_inner (uri);
		g_free (uri);
	}
	/* the http:// part's missing, prepend it, otherwise it won't always work */
	else if (strchr (url, ':') == NULL)
	{
		uri = g_strdup_printf ("http://%s", url);
		fe_open_url_inner (uri);
		g_free (uri);
	}
	/* we have a sane URL, send it to the browser untouched */
	else
	{
		fe_open_url_inner (url);
	}
}

void
fe_server_event (server *serv, int type, int arg)
{
	GSList *list = sess_list;
	session *sess;

	while (list)
	{
		sess = list->data;
		if (sess->server == serv && (current_tab == sess || !sess->gui->is_tab))
		{
			session_gui *gui = sess->gui;

			switch (type)
			{
			case FE_SE_CONNECTING:	/* connecting in progress */
			case FE_SE_RECONDELAY:	/* reconnect delay begun */
				/* enable Disconnect item */
				menu_set_disconnect_sensitive (gui, 1);
				break;

			case FE_SE_CONNECT:
				/* enable Disconnect and Away menu items */
				menu_set_away_sensitive (gui, 1);
				menu_set_disconnect_sensitive (gui, 1);
				break;

			case FE_SE_LOGGEDIN:	/* end of MOTD */
				menu_set_join_sensitive (gui, 1);
				/* if number of auto-join channels is zero, open joind */
				if (arg == 0)
					joind_open (serv);
				break;

			case FE_SE_DISCONNECT:
				/* disable Disconnect and Away menu items */
				menu_set_away_sensitive (gui, 0);
				menu_set_disconnect_sensitive (gui, 0);
				menu_set_join_sensitive (gui, 0);
				/* close the join-dialog, if one exists */
				joind_close (serv);
			}
		}
		list = list->next;
	}
}

void
fe_get_file (const char *title, char *initial,
				 void (*callback) (void *userdata, char *file), void *userdata,
				 int flags)
				
{
	/* OK: Call callback once per file, then once more with file=NULL. */
	/* CANCEL: Call callback once with file=NULL. */
	gtkutil_file_req (NULL, title, callback, userdata, initial, NULL, flags | FRF_FILTERISINITIAL);
}

void
fe_open_chan_list (server *serv, char *filter, int do_refresh)
{
	chanlist_opengui (serv, do_refresh);
}

const char *
fe_get_default_font (void)
{
#ifdef WIN32
	if (gtkutil_find_font ("Consolas"))
		return "Consolas 10";
#endif
	return NULL;
}
