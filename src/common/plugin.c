/* X-Chat
 * Copyright (C) 2002 Peter Zelezny.
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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "fabulor.h"
#ifdef WIN32
#include <glib/gwin32.h>
#endif
#include "fe.h"
#include "util.h"
#include "outbound.h"
#include "cfgfiles.h"
#include "ignore.h"
#include "server.h"
#include "servlist.h"
#include "modes.h"
#include "notify.h"
#include "text.h"
#define PLUGIN_C
typedef struct session fabulor_context;
#include "fabulor-plugin.h"
#include "plugin.h"
#include "fabulor-plugin-host.h"
#include "fabulor-plugin-enable-policy.h"
#include "typedef.h"


#include "fabulorc.h"

/* the USE_PLUGIN define only removes libdl stuff */

#ifdef USE_PLUGIN
#include <gmodule.h>
#endif

#define DEBUG(x) {x;}

/* crafted to be an even 32 bytes */
struct _fabulor_hook
{
	fabulor_plugin *pl;	/* the plugin to which it belongs */
	char *name;			/* "xdcc" */
	void *callback;	/* pointer to xdcc_callback */
	char *help_text;	/* help_text for commands only */
	void *userdata;	/* passed to the callback */
	int tag;				/* for timers & FDs only */
	int type;			/* HOOK_* */
	int pri;	/* fd */	/* priority / fd for HOOK_FD only */
};

struct _fabulor_list
{
	int type;			/* LIST_* */
	GSList *pos;		/* current pos */
	GSList *next;		/* next pos */
	GSList *head;		/* for LIST_USERS only */
	struct notify_per_server *notifyps;	/* notify_per_server * */
};

typedef int (fabulor_cmd_cb) (char *word[], char *word_eol[], void *user_data);
typedef int (fabulor_serv_cb) (char *word[], char *word_eol[], void *user_data);
typedef int (fabulor_print_cb) (char *word[], void *user_data);
typedef int (fabulor_serv_attrs_cb) (char *word[], char *word_eol[], fabulor_event_attrs *attrs, void *user_data);
typedef int (fabulor_print_attrs_cb) (char *word[], fabulor_event_attrs *attrs, void *user_data);
typedef int (fabulor_fd_cb) (int fd, int flags, void *user_data);
typedef int (fabulor_timer_cb) (void *user_data);
typedef int (fabulor_init_func) (fabulor_plugin *, char **, char **, char **, char *);
typedef int (fabulor_deinit_func) (fabulor_plugin *);

enum
{
	LIST_CHANNELS,
	LIST_DCC,
	LIST_IGNORE,
	LIST_NOTIFY,
	LIST_USERS
};

/* We use binary flags here because it makes it possible for plugin_hook_find()
 * to match several types of hooks.  This is used so that plugin_hook_run()
 * match both HOOK_SERVER and HOOK_SERVER_ATTRS hooks when plugin_emit_server()
 * is called.
 */
enum
{
	HOOK_COMMAND      = 1 << 0, /* /command */
	HOOK_SERVER       = 1 << 1, /* PRIVMSG, NOTICE, numerics */
	HOOK_SERVER_ATTRS = 1 << 2, /* same as above, with attributes */
	HOOK_PRINT        = 1 << 3, /* All print events */
	HOOK_PRINT_ATTRS  = 1 << 4, /* same as above, with attributes */
	HOOK_TIMER        = 1 << 5, /* timeouts */
	HOOK_FD           = 1 << 6, /* sockets & fds */
	HOOK_DELETED      = 1 << 7  /* marked for deletion */
};

enum
{
	CHANNEL_FLAG_CONNECTED             = 1 << 0,
	CHANNEL_FLAG_CONNECING             = 1 << 1,
	CHANNEL_FLAG_AWAY                  = 1 << 2,
	CHANNEL_FLAG_END_OF_MOTD           = 1 << 3,
	CHANNEL_FLAG_HAS_WHOX              = 1 << 4,
	CHANNEL_FLAG_HAS_IDMSG             = 1 << 5,
	CHANNEL_FLAG_HIDE_JOIN_PARTS       = 1 << 6,
	CHANNEL_FLAG_HIDE_JOIN_PARTS_UNSET = 1 << 7,
	CHANNEL_FLAG_BEEP                  = 1 << 8,
	CHANNEL_FLAG_BEEP_UNSET            = 1 << 9,
	CHANNEL_FLAG_UNUSED                = 1 << 10,
	CHANNEL_FLAG_LOGGING               = 1 << 11,
	CHANNEL_FLAG_LOGGING_UNSET         = 1 << 12,
	CHANNEL_FLAG_SCROLLBACK            = 1 << 13,
	CHANNEL_FLAG_SCROLLBACK_UNSET      = 1 << 14,
	CHANNEL_FLAG_STRIP_COLORS          = 1 << 15,
	CHANNEL_FLAG_STRIP_COLORS_UNSET    = 1 << 16,
	CHANNEL_FLAG_TRAY                  = 1 << 17,
	CHANNEL_FLAG_TRAY_UNSET            = 1 << 18,
	CHANNEL_FLAG_TASKBAR               = 1 << 19,
	CHANNEL_FLAG_TASKBAR_UNSET         = 1 << 20,
	CHANNEL_FLAG_BALLOON               = 1 << 21,
	CHANNEL_FLAG_BALLOON_UNSET         = 1 << 22,
	CHANNEL_FLAG_COUNT                 = 23
};

GSList *plugin_list = NULL;	/* export for plugingui.c */
static GSList *hook_list = NULL;
static FabulorPluginCatalog *fabulor_plugin_catalog = NULL;
static FabulorCallbackRegistry *fabulor_callback_registry = NULL;
static GHashTable *fabulor_loaded_manifest_ids = NULL;
static GPtrArray *fabulor_loaded_manifest_reports = NULL;
static FabulorAPI fabulor_plugin_api;

extern const struct prefs vars[];	/* cfgfiles.c */
static const char *plugin_get_libdir (void);
static char *fabulor_manifest_bundled_plugins_root (void);

static gboolean
fabulor_manifest_plugins_enabled (void)
{
	const char *enabled = g_getenv ("FABULOR_ENABLE_MANIFEST_PLUGINS");

	return fabulor_plugin_enable_policy_should_autoload (prefs.hex_gui_manifest_plugins,
												 enabled,
												 arg_skip_plugins);
}

static gboolean
fabulor_api_send_message (void *user_data, const char *target, const char *text, GError **error)
{
	session *sess = user_data;
	char *command;
	const unsigned char *cursor;
	gsize target_length;
	gsize text_length;

	if (!sess || !target || !*target || !text)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "FabulorAPI send_message requires a valid session, target, and text.");
		return FALSE;
	}

	target_length = strlen (target);
	text_length = strlen (text);
	if (target_length > FABULOR_PLUGIN_MESSAGE_TARGET_MAX
		|| text_length > FABULOR_PLUGIN_MESSAGE_TEXT_MAX
		|| !g_utf8_validate (target, -1, NULL)
		|| !g_utf8_validate (text, -1, NULL))
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "FabulorAPI send_message received invalid UTF-8 or exceeded its size limit.");
		return FALSE;
	}

	for (cursor = (const unsigned char *) target; *cursor; cursor++)
	{
		if (*cursor <= 0x20 || *cursor == 0x7f)
		{
			g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "FabulorAPI message targets cannot contain whitespace or control characters.");
			return FALSE;
		}
	}
	if (strchr (text, '\r') || strchr (text, '\n'))
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "FabulorAPI message text cannot contain CR or LF characters.");
		return FALSE;
	}

	command = g_strdup_printf ("MSG %s %s", target, text);
	handle_command (sess, command, FALSE);
	g_free (command);
	return TRUE;
}

static void
fabulor_api_log (void *user_data, const char *text)
{
	session *sess = user_data;

	if (sess && text)
	{
		if (!g_utf8_validate (text, -1, NULL)
			|| strlen (text) > FABULOR_PLUGIN_LOG_TEXT_MAX)
		{
			PrintText (sess, "Fabulor plugin host\tRejected invalid or oversized plugin log text.\n");
			return;
		}
		PrintTextf (sess, "Fabulor plugin host\t%s\n", text);
	}
}

static void
fabulor_api_logf (session *sess, const char *format, ...)
{
	va_list args;
	char *message;

	if (!sess || !format)
	{
		return;
	}

	va_start (args, format);
	message = g_strdup_vprintf (format, args);
	va_end (args);

	fabulor_api_log (sess, message);
	g_free (message);
}

static guint
fabulor_api_get_user_count (void *user_data)
{
	session *sess = user_data;

	if (!sess || sess->total < 0)
	{
		return 0;
	}

	return (guint) sess->total;
}

static gboolean
fabulor_api_get_user_info (void *user_data, FabulorUserInfo *user_info)
{
	session *sess = user_data;

	if (!user_info)
	{
		return FALSE;
	}

	memset (user_info, 0, sizeof (*user_info));

	if (!sess)
	{
		return FALSE;
	}

	if (sess->server)
	{
		user_info->nickname = sess->server->nick;
		user_info->server_name = sess->server->connected ? sess->server->servername : NULL;
		user_info->network_name = server_get_network (sess->server, TRUE);
	}

	user_info->channel = sess->channel;
	return user_info->nickname != NULL
		|| user_info->channel != NULL
		|| user_info->server_name != NULL
		|| user_info->network_name != NULL;
}

static void
fabulor_plugin_api_set_session (session *sess)
{
	fabulor_plugin_api.api_version = FABULOR_PLUGIN_API_VERSION;
	fabulor_plugin_api.user_data = sess;
	fabulor_plugin_api.send_message = fabulor_api_send_message;
	fabulor_plugin_api.log = fabulor_api_log;
	fabulor_plugin_api.get_user_count = fabulor_api_get_user_count;
	fabulor_plugin_api.get_user_info = fabulor_api_get_user_info;
}

static void
fabulor_plugin_host_free (void)
{
	if (fabulor_loaded_manifest_reports)
	{
		g_ptr_array_unref (fabulor_loaded_manifest_reports);
		fabulor_loaded_manifest_reports = NULL;
	}
	if (fabulor_loaded_manifest_ids)
	{
		g_hash_table_unref (fabulor_loaded_manifest_ids);
		fabulor_loaded_manifest_ids = NULL;
	}

	if (fabulor_callback_registry)
	{
		fabulor_callback_registry_shutdown (fabulor_callback_registry);
	}

	fabulor_plugin_host_shutdown ();

	if (fabulor_callback_registry)
	{
		fabulor_callback_registry_free (fabulor_callback_registry);
		fabulor_callback_registry = NULL;
	}

	if (fabulor_plugin_catalog)
	{
		fabulor_plugin_catalog_free (fabulor_plugin_catalog);
		fabulor_plugin_catalog = NULL;
	}
}

static void
fabulor_plugin_host_blacklist_load (void)
{
	char *blacklist_path;
	char *contents;
	gchar **lines;
	guint i;

	if (!fabulor_plugin_catalog)
	{
		return;
	}

	blacklist_path = g_build_filename (get_xdir (), "plugin-blacklist.txt", NULL);
	if (!g_file_get_contents (blacklist_path, &contents, NULL, NULL))
	{
		g_free (blacklist_path);
		return;
	}

	lines = g_strsplit (contents, "\n", -1);
	for (i = 0; lines[i] != NULL; i++)
	{
		char *line = g_strstrip (lines[i]);
		if (*line == '\0' || *line == '#')
		{
			continue;
		}

		fabulor_plugin_catalog_blacklist_plugin (fabulor_plugin_catalog, line);
	}

	g_strfreev (lines);
	g_free (contents);
	g_free (blacklist_path);
}

static void
append_json_string (GString *json, const char *value)
{
	const unsigned char *p;

	g_string_append_c (json, '"');
	if (value)
	{
		for (p = (const unsigned char *) value; *p; p++)
		{
			switch (*p)
			{
			case '\\':
				g_string_append (json, "\\\\");
				break;
			case '"':
				g_string_append (json, "\\\"");
				break;
			case '\b':
				g_string_append (json, "\\b");
				break;
			case '\f':
				g_string_append (json, "\\f");
				break;
			case '\n':
				g_string_append (json, "\\n");
				break;
			case '\r':
				g_string_append (json, "\\r");
				break;
			case '\t':
				g_string_append (json, "\\t");
				break;
			default:
				if (*p < 0x20)
				{
					g_string_append_printf (json, "\\u%04x", (unsigned int)*p);
				}
				else
				{
					g_string_append_c (json, (char)*p);
				}
			}
		}
	}
	g_string_append_c (json, '"');
}

static void
fabulor_plugin_host_dispatch_event (session *sess,
									const char *event_name,
									const char *source_name,
									char *word[],
									char *word_eol[],
									time_t server_time)
{
	GError *error = NULL;
	GString *json;
	const char *network = NULL;
	const char *nick = NULL;
	const char *server_name = NULL;
	const char *channel = NULL;

	if (!fabulor_callback_registry
		|| !fabulor_callback_registry_has_event (fabulor_callback_registry, event_name))
	{
		return;
	}

	if (sess)
	{
		channel = sess->channel;
		if (sess->server)
		{
			network = server_get_network (sess->server, FALSE);
			nick = sess->server->nick;
			server_name = sess->server->servername;
		}
	}

	json = g_string_new ("{");
	g_string_append (json, "\"event\":");
	append_json_string (json, event_name);
	g_string_append (json, ",\"source\":");
	append_json_string (json, source_name);
	g_string_append (json, ",\"channel\":");
	append_json_string (json, channel);
	g_string_append (json, ",\"network\":");
	append_json_string (json, network);
	g_string_append (json, ",\"nick\":");
	append_json_string (json, nick);
	g_string_append (json, ",\"server\":");
	append_json_string (json, server_name);
	g_string_append_printf (json, ",\"time\":%lld", (long long) server_time);
	g_string_append (json, ",\"word1\":");
	append_json_string (json, word && word[1] ? word[1] : "");
	g_string_append (json, ",\"word2\":");
	append_json_string (json, word && word[2] ? word[2] : "");
	g_string_append (json, ",\"word3\":");
	append_json_string (json, word && word[3] ? word[3] : "");
	g_string_append (json, ",\"word4\":");
	append_json_string (json, word && word[4] ? word[4] : "");
	g_string_append (json, ",\"word_eol1\":");
	append_json_string (json, word_eol && word_eol[1] ? word_eol[1] : "");
	g_string_append (json, ",\"word_eol2\":");
	append_json_string (json, word_eol && word_eol[2] ? word_eol[2] : "");
	g_string_append_c (json, '}');

	fabulor_callback_registry_fire_event (fabulor_callback_registry,
										  event_name,
										  json->str,
										  NULL,
										  &error);
	if (error)
	{
		if (current_sess)
		{
			PrintTextf (current_sess, "Fabulor plugin host\tCallback dispatch error: %s\n", error->message);
		}
		g_clear_error (&error);
	}

	g_string_free (json, TRUE);
}

static void
fabulor_plugin_host_fire_event (session *sess,
								const char *event_name,
								const char *source_name,
								char *word[],
								char *word_eol[],
								time_t server_time)
{
	if (!event_name)
	{
		return;
	}

	fabulor_plugin_host_dispatch_event (sess, event_name, source_name, word, word_eol, server_time);

	if (source_name && *source_name)
	{
		char *specific_event = g_strdup_printf ("%s:%s", event_name, source_name);
		fabulor_plugin_host_dispatch_event (sess, specific_event, source_name, word, word_eol, server_time);
		g_free (specific_event);
	}
}

static const char *
fabulor_manifest_language_display_name (FabulorPluginLanguage language)
{
	switch (language)
	{
	case FABULOR_PLUGIN_LANGUAGE_CSHARP:
		return "C#";
	case FABULOR_PLUGIN_LANGUAGE_PYTHON:
		return "Python";
	case FABULOR_PLUGIN_LANGUAGE_TCL:
		return "Tcl";
	default:
		return "Unknown";
	}
}

static void
fabulor_plugin_host_autoload (session *sess)
{
	GError *error = NULL;
	GPtrArray *ordered_manifests;
	char *bundled_plugins_dir;
	char *user_plugins_dir;
	guint i;

	fabulor_plugin_api_set_session (sess);

	fabulor_plugin_catalog = fabulor_plugin_catalog_new (&fabulor_plugin_api);
	fabulor_plugin_catalog_set_safe_mode (fabulor_plugin_catalog, arg_skip_plugins);
	fabulor_plugin_host_blacklist_load ();

	bundled_plugins_dir = fabulor_manifest_bundled_plugins_root ();
	user_plugins_dir = g_build_filename (get_xdir (), "plugins", NULL);

	if (bundled_plugins_dir
		&& g_file_test (bundled_plugins_dir, G_FILE_TEST_IS_DIR)
		&& !fabulor_plugin_catalog_discover_root (fabulor_plugin_catalog, bundled_plugins_dir, &error))
	{
		fabulor_api_log (sess, error->message);
		g_clear_error (&error);
	}

	if (g_file_test (user_plugins_dir, G_FILE_TEST_IS_DIR)
		&& !fabulor_plugin_catalog_discover_root (fabulor_plugin_catalog, user_plugins_dir, &error))
	{
		fabulor_api_log (sess, error->message);
		g_clear_error (&error);
	}

	fabulor_callback_registry = fabulor_callback_registry_new (&fabulor_plugin_api, fabulor_plugin_catalog, NULL);

	ordered_manifests = fabulor_plugin_catalog_resolve_load_order (fabulor_plugin_catalog,
															 FABULOR_PLUGIN_API_VERSION,
															 &error);
	if (!ordered_manifests)
	{
		if (error)
		{
			fabulor_api_log (sess, error->message);
			g_clear_error (&error);
		}
		g_free (bundled_plugins_dir);
		g_free (user_plugins_dir);
		return;
	}

	for (i = 0; i < ordered_manifests->len; i++)
	{
		const FabulorPluginManifest *manifest = g_ptr_array_index (ordered_manifests, i);
		const FabulorPluginLoader *loader = fabulor_plugin_loader_for_language (manifest->language);

		if (!loader)
		{
			fabulor_api_logf (sess, "Skipping %s because no loader is registered for %s.",
							  manifest->id,
							  fabulor_plugin_language_to_string (manifest->language));
			continue;
		}

		if (!loader->load (manifest, &fabulor_plugin_api, sess, &error))
		{
			fabulor_callback_registry_remove_plugin (fabulor_callback_registry, manifest->id);
			fabulor_api_logf (sess, "Skipping %s: %s", manifest->id, error ? error->message : "unknown loader failure");
			g_clear_error (&error);
			continue;
		}

		if (!fabulor_loaded_manifest_ids)
			fabulor_loaded_manifest_ids = g_hash_table_new_full (g_str_hash,
														  g_str_equal, g_free, NULL);
		if (!fabulor_loaded_manifest_reports)
			fabulor_loaded_manifest_reports = g_ptr_array_new_with_free_func (g_free);
		g_hash_table_add (fabulor_loaded_manifest_ids, g_strdup (manifest->id));
		g_ptr_array_add (fabulor_loaded_manifest_reports,
			g_strdup_printf ("Manifest %s plugin: %s %s [%s]",
				fabulor_manifest_language_display_name (manifest->language),
				manifest->name,
				manifest->version,
				manifest->entrypoint));
		fabulor_api_logf (sess, "Loaded manifest plugin %s (%s).", manifest->id, loader->language_name);
	}

	g_ptr_array_free (ordered_manifests, TRUE);
	g_free (bundled_plugins_dir);
	g_free (user_plugins_dir);
}


/* unload a plugin and remove it from our linked list */

static int
plugin_free (fabulor_plugin *pl, int do_deinit, int allow_refuse)
{
	GSList *list, *next;
	fabulor_hook *hook;
	fabulor_deinit_func *deinit_func;

	/* fake plugin added by fabulor_plugingui_add() */
	if (pl->fake)
		goto xit;

	/* run the plugin's deinit routine, if any */
	if (do_deinit && pl->deinit_callback != NULL)
	{
		deinit_func = pl->deinit_callback;
		if (!deinit_func (pl) && allow_refuse)
			return FALSE;
	}

	/* remove all of this plugin's hooks */
	list = hook_list;
	while (list)
	{
		hook = list->data;
		next = list->next;
		if (hook->pl == pl)
			fabulor_unhook (NULL, hook);
		list = next;
	}

#ifdef USE_PLUGIN
	if (pl->handle)
		g_module_close (pl->handle);
#endif

xit:
	if (pl->free_strings)
	{
		g_free (pl->name);
		g_free (pl->desc);
		g_free (pl->version);
	}
	g_free ((char *)pl->filename);
	g_free (pl);

	plugin_list = g_slist_remove (plugin_list, pl);

#ifdef USE_PLUGIN
	fe_pluginlist_update ();
#endif

	return TRUE;
}

static fabulor_plugin *
plugin_list_add (fabulor_context *ctx, char *filename, const char *name,
					  const char *desc, const char *version, void *handle,
					  void *deinit_func, int fake, int free_strings)
{
	fabulor_plugin *pl;

	pl = g_new (fabulor_plugin, 1);
	pl->handle = handle;
	pl->filename = filename;
	pl->context = ctx;
	pl->name = (char *)name;
	pl->desc = (char *)desc;
	pl->version = (char *)version;
	pl->deinit_callback = deinit_func;
	pl->fake = fake;
	pl->free_strings = free_strings;	/* free() name,desc,version? */

	plugin_list = g_slist_prepend (plugin_list, pl);

	return pl;
}

#ifndef WIN32
static void *
fabulor_dummy (fabulor_plugin *ph)
{
	return NULL;
}

#else

static int
fabulor_read_fd (fabulor_plugin *ph, GIOChannel *source, char *buf, int *len)
{
	GError *error = NULL;

	g_io_channel_set_buffered (source, FALSE);
	g_io_channel_set_encoding (source, NULL, &error);

	if (g_io_channel_read_chars (source, buf, *len, (gsize*)len, &error) == G_IO_STATUS_NORMAL)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}
#endif

/* Load a static plugin */

void
plugin_add (session *sess, char *filename, void *handle, void *init_func,
				void *deinit_func, char *arg, int fake)
{
	fabulor_plugin *pl;
	char *file;

	file = g_strdup (filename);

	pl = plugin_list_add (sess, file, file, NULL, NULL, handle, deinit_func,
								 fake, FALSE);

	if (!fake)
	{
		/* win32 uses these because it doesn't have --export-dynamic! */
		pl->fabulor_hook_command = fabulor_hook_command;
		pl->fabulor_hook_server = fabulor_hook_server;
		pl->fabulor_hook_print = fabulor_hook_print;
		pl->fabulor_hook_timer = fabulor_hook_timer;
		pl->fabulor_hook_fd = fabulor_hook_fd;
		pl->fabulor_unhook = fabulor_unhook;
		pl->fabulor_print = fabulor_print;
		pl->fabulor_printf = fabulor_printf;
		pl->fabulor_command = fabulor_command;
		pl->fabulor_commandf = fabulor_commandf;
		pl->fabulor_nickcmp = fabulor_nickcmp;
		pl->fabulor_set_context = fabulor_set_context;
		pl->fabulor_find_context = fabulor_find_context;
		pl->fabulor_get_context = fabulor_get_context;
		pl->fabulor_get_info = fabulor_get_info;
		pl->fabulor_get_prefs = fabulor_get_prefs;
		pl->fabulor_list_get = fabulor_list_get;
		pl->fabulor_list_free = fabulor_list_free;
		pl->fabulor_list_fields = fabulor_list_fields;
		pl->fabulor_list_str = fabulor_list_str;
		pl->fabulor_list_next = fabulor_list_next;
		pl->fabulor_list_int = fabulor_list_int;
		pl->fabulor_plugingui_add = fabulor_plugingui_add;
		pl->fabulor_plugingui_remove = fabulor_plugingui_remove;
		pl->fabulor_emit_print = fabulor_emit_print;
#ifdef WIN32
		pl->fabulor_read_fd = (void *) fabulor_read_fd;
#else
		pl->fabulor_read_fd = fabulor_dummy;
#endif
		pl->fabulor_list_time = fabulor_list_time;
		pl->fabulor_gettext = fabulor_gettext;
		pl->fabulor_send_modes = fabulor_send_modes;
		pl->fabulor_strip = fabulor_strip;
		pl->fabulor_free = fabulor_free;
		pl->fabulor_pluginpref_set_str = fabulor_pluginpref_set_str;
		pl->fabulor_pluginpref_get_str = fabulor_pluginpref_get_str;
		pl->fabulor_pluginpref_set_int = fabulor_pluginpref_set_int;
		pl->fabulor_pluginpref_get_int = fabulor_pluginpref_get_int;
		pl->fabulor_pluginpref_delete = fabulor_pluginpref_delete;
		pl->fabulor_pluginpref_list = fabulor_pluginpref_list;
		pl->fabulor_hook_server_attrs = fabulor_hook_server_attrs;
		pl->fabulor_hook_print_attrs = fabulor_hook_print_attrs;
		pl->fabulor_emit_print_attrs = fabulor_emit_print_attrs;
		pl->fabulor_event_attrs_create = fabulor_event_attrs_create;
		pl->fabulor_event_attrs_free = fabulor_event_attrs_free;

		/* run fabulor_plugin_init, if it returns 0, close the plugin */
		if (((fabulor_init_func *)init_func) (pl, &pl->name, &pl->desc, &pl->version, arg) == 0)
		{
			plugin_free (pl, FALSE, FALSE);
			return;
		}
	}

#ifdef USE_PLUGIN
	fe_pluginlist_update ();
#endif
}

/* kill any plugin by the given (file) name (used by /unload) */

int
plugin_kill (char *name, int by_filename)
{
	GSList *list;
	fabulor_plugin *pl;

	list = plugin_list;
	while (list)
	{
		pl = list->data;
		/* static-plugins (plugin-timer.c) have a NULL filename */
		if ((by_filename && pl->filename && g_ascii_strcasecmp (name, pl->filename) == 0) ||
			 (by_filename && pl->filename && g_ascii_strcasecmp (name, file_part (pl->filename)) == 0) ||
			(!by_filename && g_ascii_strcasecmp (name, pl->name) == 0))
		{
			/* statically linked plugins have a NULL filename */
			if (pl->filename != NULL && !pl->fake)
			{
				if (plugin_free (pl, TRUE, TRUE))
					return 1;
				return 2;
			}
		}
		list = list->next;
	}

	return 0;
}

/* kill all running plugins (at shutdown) */

void
plugin_kill_all (void)
{
	GSList *list, *next;
	fabulor_plugin *pl;

	list = plugin_list;
	while (list)
	{
		pl = list->data;
		next = list->next;
		if (!pl->fake)
			plugin_free (list->data, TRUE, FALSE);
		list = next;
	}

	fabulor_plugin_host_free ();
}

#if defined(USE_PLUGIN) || defined(WIN32)
/* used for loading plugins, and in fe-gtk/notifications/notification-windows.c */

GModule *
module_load (char *filename)
{
	void *handle;
	char *filepart;
	char *pluginpath;

	/* get the filename without path */
	filepart = file_part (filename);

	/* load the plugin */
	if (!g_ascii_strcasecmp (filepart, filename))
	{
		/* no path specified, it's just the filename, try to load from config dir */
		pluginpath = g_build_filename (get_xdir (), "addons", filename, NULL);
		handle = g_module_open (pluginpath, 0);
		g_free (pluginpath);
	}
	else
	{
		/* try to load with absolute path */
		handle = g_module_open (filename, 0);
	}

	return handle;
}

#endif

#ifdef USE_PLUGIN

/* load a plugin from a filename. Returns: NULL-success or an error string */

char *
plugin_load (session *sess, char *filename, char *arg)
{
	GModule *handle = module_load (filename);
	fabulor_init_func *init_func;
	fabulor_deinit_func *deinit_func;

	if (handle == NULL)
		return (char *)g_module_error ();

	/* find the init routine fabulor_plugin_init */
	if (!g_module_symbol (handle, "fabulor_plugin_init", (gpointer *)&init_func))
	{
		g_module_close (handle);
		return _("No fabulor_plugin_init symbol; is this really a Fabulor plugin?");
	}

	/* find the plugin's deinit routine, if any */
	if (!g_module_symbol (handle, "fabulor_plugin_deinit", (gpointer *)&deinit_func))
		deinit_func = NULL;

	/* add it to our linked list */
	plugin_add (sess, filename, handle, init_func, deinit_func, arg, FALSE);

	return NULL;
}

static session *ps;

static void
plugin_auto_load_cb (char *filename)
{
	char *pMsg;

	pMsg = plugin_load (ps, filename, NULL);
	if (pMsg)
	{
		PrintTextf (ps, "AutoLoad failed for: %s\n", filename);
		PrintText (ps, pMsg);
	}
}

static const char *
plugin_get_libdir (void)
{
	const char *libdir;

	libdir = g_getenv ("FABULOR_LIBDIR");
#ifdef WIN32
	if (libdir && *libdir)
	{
		const char *development_roots_enabled;

		development_roots_enabled = g_getenv ("FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS");
		if (development_roots_enabled &&
			g_ascii_strcasecmp (development_roots_enabled, "1") == 0)
			return libdir;
	}

	{
		static char *windows_libdir;
		char *install_root;

		if (windows_libdir)
			return windows_libdir;
		install_root = g_win32_get_package_installation_directory_of_module (NULL);
		if (install_root)
		{
			windows_libdir = g_build_filename (install_root, "plugins", NULL);
			g_free (install_root);
			return windows_libdir;
		}
	}
#else
	if (libdir && *libdir)
		return libdir;
#endif
	return FABULORLIBDIR;
}

static char *
fabulor_manifest_bundled_plugins_root (void)
{
#ifdef WIN32
	char *install_root;
	char *plugins_root;
	const char *development_roots_enabled;

	install_root = g_win32_get_package_installation_directory_of_module (NULL);
	if (install_root)
	{
		plugins_root = g_build_filename (install_root, "Plugins", NULL);
		g_free (install_root);
		return plugins_root;
	}

	development_roots_enabled = g_getenv ("FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS");
	if (!development_roots_enabled || g_ascii_strcasecmp (development_roots_enabled, "1") != 0)
	{
		return NULL;
	}
#endif

	return g_build_filename (plugin_get_libdir (), "Plugins", NULL);
}

#ifdef WIN32
static HMODULE python_runtime_module;

static gboolean
plugin_prepare_python_runtime (session *sess)
{
	char *install_root;
	char *runtime_path;
	wchar_t *runtime_path_utf16;
	char *load_error;
	const char *development_roots_enabled;

	if (python_runtime_module)
		return TRUE;

	install_root = g_win32_get_package_installation_directory_of_module (NULL);
	if (!install_root)
		return TRUE;

	runtime_path = g_build_filename (install_root, "Runtime", "Python314", "python314.dll", NULL);
	g_free (install_root);

	if (!g_file_test (runtime_path, G_FILE_TEST_IS_REGULAR))
	{
		development_roots_enabled = g_getenv ("FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS");
		if (!development_roots_enabled || g_ascii_strcasecmp (development_roots_enabled, "1") != 0)
		{
			PrintTextf (sess, "Python runtime is missing from the trusted path: %s\n", runtime_path);
			g_free (runtime_path);
			return FALSE;
		}

		/* Explicitly gated development builds may retain their configured DLL search path. */
		g_free (runtime_path);
		return TRUE;
	}

	runtime_path_utf16 = g_utf8_to_utf16 (runtime_path, -1, NULL, NULL, NULL);
	if (runtime_path_utf16)
	{
		python_runtime_module = LoadLibraryExW (
			runtime_path_utf16,
			NULL,
			LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
		g_free (runtime_path_utf16);
	}

	if (!python_runtime_module)
	{
		load_error = g_win32_error_message (GetLastError ());
		PrintTextf (sess, "Python runtime load failed for %s: %s\n", runtime_path, load_error);
		g_free (load_error);
		g_free (runtime_path);
		return FALSE;
	}

	g_free (runtime_path);
	return TRUE;
}
#endif

static gint
plugin_startup_report_compare (gconstpointer left, gconstpointer right)
{
	const char *left_entry = *(const char * const *) left;
	const char *right_entry = *(const char * const *) right;
	return g_ascii_strcasecmp (left_entry, right_entry);
}

void
plugin_print_startup_report (session *sess)
{
	GPtrArray *entries;
	GSList *list;
	guint i;

	if (!sess)
		return;

	entries = g_ptr_array_new_with_free_func (g_free);
	for (list = plugin_list; list; list = list->next)
	{
		fabulor_plugin *plugin = list->data;
		const char *name;
		const char *version;
		const char *filename;
		const char *kind;

		if (!plugin)
			continue;

		name = plugin->name && *plugin->name ? plugin->name : "unnamed";
		version = plugin->version && *plugin->version ? plugin->version : NULL;
		filename = plugin->filename && *plugin->filename ? file_part (plugin->filename) : NULL;
		kind = plugin->fake && filename && g_str_has_suffix (filename, ".py")
			? "Python add-on"
			: (plugin->fake ? "Script add-on" : "Plugin");
		if (plugin->fake && fabulor_loaded_manifest_ids && name &&
			g_hash_table_contains (fabulor_loaded_manifest_ids, name))
			continue;

		if (version && filename)
			g_ptr_array_add (entries, g_strdup_printf ("%s: %s %s [%s]", kind, name, version, filename));
		else if (version)
			g_ptr_array_add (entries, g_strdup_printf ("%s: %s %s", kind, name, version));
		else if (filename)
			g_ptr_array_add (entries, g_strdup_printf ("%s: %s [%s]", kind, name, filename));
		else
			g_ptr_array_add (entries, g_strdup_printf ("%s: %s", kind, name));
	}

	fabulor_plugin_host_append_loaded_simple_addons (entries);
	if (fabulor_loaded_manifest_reports)
	{
		for (i = 0; i < fabulor_loaded_manifest_reports->len; i++)
			g_ptr_array_add (entries, g_strdup (g_ptr_array_index (
				fabulor_loaded_manifest_reports, i)));
	}
	g_ptr_array_sort (entries, plugin_startup_report_compare);

	PrintTextf (sess, "\nFabulor loaded plugins and add-ons (%u):\n", entries->len);
	for (i = 0; i < entries->len; i++)
	{
		PrintTextf (sess, "  %s\n", (const char *) g_ptr_array_index (entries, i));
	}
	PrintText (sess, "\n");
	g_ptr_array_free (entries, TRUE);
}

void
plugin_auto_load (session *sess)
{
	const char *lib_dir;
	char *sub_dir;
	ps = sess;
	fabulor_plugin_host_free ();

	lib_dir = plugin_get_libdir ();
	sub_dir = g_build_filename (get_xdir (), "addons", NULL);
	fabulor_plugin_api_set_session (sess);

#ifdef WIN32
	/* a long list of bundled plugins that should be loaded automatically,
	 * user plugins should go to <config>, leave Program Files alone! */
	for_files (lib_dir, "hcchecksum.dll", plugin_auto_load_cb);
	for_files (lib_dir, "hcexec.dll", plugin_auto_load_cb);
	for_files (lib_dir, "hcfishlim.dll", plugin_auto_load_cb);
	for_files(lib_dir, "hclua.dll", plugin_auto_load_cb);
	if (plugin_prepare_python_runtime (sess))
		for_files (lib_dir, "hcpython3.dll", plugin_auto_load_cb);
	for_files (lib_dir, "hcsysinfo.dll", plugin_auto_load_cb);
#else
	for_files (lib_dir, "*."PLUGIN_SUFFIX, plugin_auto_load_cb);
#endif

	for_files (sub_dir, "*."PLUGIN_SUFFIX, plugin_auto_load_cb);

	if (fabulor_manifest_plugins_enabled ())
	{
		fabulor_plugin_host_autoload (sess);
	}
	if (!fabulor_plugin_catalog)
	{
		fabulor_plugin_catalog = fabulor_plugin_catalog_new (&fabulor_plugin_api);
		fabulor_plugin_catalog_set_safe_mode (fabulor_plugin_catalog, arg_skip_plugins);
	}
	if (!fabulor_callback_registry)
	{
		fabulor_callback_registry = fabulor_callback_registry_new (&fabulor_plugin_api, fabulor_plugin_catalog, NULL);
	}

	{
		GError *csharp_error = NULL;
		if (!fabulor_plugin_host_autoload_simple_csharp (sub_dir, &fabulor_plugin_api, &csharp_error))
		{
			fabulor_api_logf (sess, "Simple C# add-on loading failed: %s",
							  csharp_error ? csharp_error->message : "unknown error");
			g_clear_error (&csharp_error);
		}
	}

	{
		GError *tcl_error = NULL;
		if (!fabulor_plugin_host_autoload_simple_tcl (sub_dir, &fabulor_plugin_api, &tcl_error))
		{
			fabulor_api_logf (sess, "Simple Tcl add-on loading failed: %s",
							  tcl_error ? tcl_error->message : "unknown error");
			g_clear_error (&tcl_error);
		}
	}

	g_free (sub_dir);
}

int
plugin_reload (session *sess, char *name, int by_filename)
{
	GSList *list;
	char *filename;
	char *ret;
	fabulor_plugin *pl;

	list = plugin_list;
	while (list)
	{
		pl = list->data;
		/* static-plugins (plugin-timer.c) have a NULL filename */
		if ((by_filename && pl->filename && g_ascii_strcasecmp (name, pl->filename) == 0) ||
			 (by_filename && pl->filename && g_ascii_strcasecmp (name, file_part (pl->filename)) == 0) ||
			(!by_filename && g_ascii_strcasecmp (name, pl->name) == 0))
		{
			/* statically linked plugins have a NULL filename */
			if (pl->filename != NULL && !pl->fake)
			{
				filename = g_strdup (pl->filename);
				plugin_free (pl, TRUE, FALSE);
				ret = plugin_load (sess, filename, NULL);
				g_free (filename);
				if (ret == NULL)
					return 1;
				else
					return 0;
			}
			else
				return 2;
		}
		list = list->next;
	}

	return 0;
}

#endif

static GSList *
plugin_hook_find (GSList *list, int type, char *name)
{
	fabulor_hook *hook;

	while (list)
	{
		hook = list->data;
		if (hook && (hook->type & type))
		{
			if (g_ascii_strcasecmp (hook->name, name) == 0)
				return list;

			if ((type & HOOK_SERVER)
				&& g_ascii_strcasecmp (hook->name, "RAW LINE") == 0)
					return list;
		}
		list = list->next;
	}

	return NULL;
}

/* check for plugin hooks and run them */

static int
plugin_hook_run (session *sess, char *name, char *word[], char *word_eol[],
				 fabulor_event_attrs *attrs, int type)
{
	GSList *list, *next;
	fabulor_hook *hook;
	int ret, eat = 0;

	list = hook_list;
	while (1)
	{
		list = plugin_hook_find (list, type, name);
		if (!list)
			goto xit;

		hook = list->data;
		next = list->next;
		hook->pl->context = sess;

		/* run the plugin's callback function */
		switch (hook->type)
		{
		case HOOK_COMMAND:
			ret = ((fabulor_cmd_cb *)hook->callback) (word, word_eol, hook->userdata);
			break;
		case HOOK_PRINT_ATTRS:
			ret = ((fabulor_print_attrs_cb *)hook->callback) (word, attrs, hook->userdata);
			break;
		case HOOK_SERVER:
			ret = ((fabulor_serv_cb *)hook->callback) (word, word_eol, hook->userdata);
			break;
		case HOOK_SERVER_ATTRS:
			ret = ((fabulor_serv_attrs_cb *)hook->callback) (word, word_eol, attrs, hook->userdata);
			break;
		default: /*case HOOK_PRINT:*/
			ret = ((fabulor_print_cb *)hook->callback) (word, hook->userdata);
			break;
		}

		if ((ret & FABULOR_EAT_FABULOR) && (ret & FABULOR_EAT_PLUGIN))
		{
			eat = 1;
			goto xit;
		}
		if (ret & FABULOR_EAT_PLUGIN)
			goto xit;	/* stop running plugins */
		if (ret & FABULOR_EAT_FABULOR)
			eat = 1;	/* eventually we'll return 1, but continue running plugins */

		list = next;
	}

xit:
	/* really remove deleted hooks now */
	list = hook_list;
	while (list)
	{
		hook = list->data;
		next = list->next;
		if (!hook || hook->type == HOOK_DELETED)
		{
			hook_list = g_slist_remove (hook_list, hook);
			g_free (hook);
		}
		list = next;
	}

	return eat;
}

/* execute a plugged in command. Called from outbound.c */

int
plugin_emit_command (session *sess, char *name, char *word[], char *word_eol[])
{
	GError *tcl_error = NULL;

	fabulor_plugin_api_set_session (sess);
	if (fabulor_plugin_host_handle_simple_tcl_command (name,
												  word_eol && word_eol[2] ? word_eol[2] : "",
												  &tcl_error))
	{
		if (tcl_error)
		{
			fabulor_api_logf (sess, "Simple Tcl command %s failed: %s", name, tcl_error->message);
			g_clear_error (&tcl_error);
		}
		return 1;
	}

	if (name && word)
	{
		fabulor_plugin_host_fire_event (sess, "command", name, word, word_eol, 0);
	}

	return plugin_hook_run (sess, name, word, word_eol, NULL, HOOK_COMMAND);
}

fabulor_event_attrs *
fabulor_event_attrs_create (fabulor_plugin *ph)
{
	return g_new0 (fabulor_event_attrs, 1);
}

void
fabulor_event_attrs_free (fabulor_plugin *ph, fabulor_event_attrs *attrs)
{
	g_free (attrs);
}

/* got a server PRIVMSG, NOTICE, numeric etc... */

int
plugin_emit_server (session *sess, char *name, char *word[], char *word_eol[],
					time_t server_time)
{
	fabulor_event_attrs attrs;

	attrs.server_time_utc = server_time;

	if (name && word)
	{
		fabulor_plugin_host_fire_event (sess,
										g_ascii_strcasecmp (name, "PRIVMSG") == 0 ? "message" : "server",
										name,
										word,
										word_eol,
										server_time);
	}

	return plugin_hook_run (sess, name, word, word_eol, &attrs, 
							HOOK_SERVER | HOOK_SERVER_ATTRS);
}

/* see if any plugins are interested in this print event */

int
plugin_emit_print (session *sess, char *word[], time_t server_time)
{
	fabulor_event_attrs attrs;

	attrs.server_time_utc = server_time;

	if (word && word[0])
	{
		fabulor_plugin_host_fire_event (sess, "print", word[0], word, NULL, server_time);
	}

	return plugin_hook_run (sess, word[0], word, NULL, &attrs,
							HOOK_PRINT | HOOK_PRINT_ATTRS);
}

int
plugin_emit_dummy_print (session *sess, char *name)
{
	char *word[PDIWORDS];
	int i;

	word[0] = name;
	for (i = 1; i < PDIWORDS; i++)
		word[i] = "\000";

	return plugin_hook_run (sess, name, word, NULL, NULL, HOOK_PRINT);
}

int
plugin_emit_keypress (session *sess, unsigned int state, unsigned int keyval, gunichar key)
{
	char *word[PDIWORDS];
	char keyval_str[16];
	char state_str[16];
	char len_str[16];
	char key_str[7];
	int i, len;

	if (!hook_list)
		return 0;

	sprintf (keyval_str, "%u", keyval);
	sprintf (state_str, "%u", state);
	if (!key)
		len = 0;
	else
		len = g_unichar_to_utf8 (key, key_str);
	key_str[len] = '\0';
	sprintf (len_str, "%d", len);

	word[0] = "Key Press";
	word[1] = keyval_str;
	word[2] = state_str;
	word[3] = key_str;
	word[4] = len_str;
	for (i = 5; i < PDIWORDS; i++)
		word[i] = "\000";

	return plugin_hook_run (sess, word[0], word, NULL, NULL, HOOK_PRINT);
}

static int
plugin_timeout_cb (fabulor_hook *hook)
{
	int ret;

	/* timer_cb's context starts as front-most-tab */
	hook->pl->context = current_sess;

	/* call the plugin's timeout function */
	ret = ((fabulor_timer_cb *)hook->callback) (hook->userdata);

	/* the callback might have already unhooked it! */
	if (!g_slist_find (hook_list, hook) || hook->type == HOOK_DELETED)
		return 0;

	if (ret == 0)
	{
		hook->tag = 0;	/* avoid fe_timeout_remove, returning 0 is enough! */
		fabulor_unhook (hook->pl, hook);
	}

	return ret;
}

/* insert a hook into hook_list according to its priority */

static void
plugin_insert_hook (fabulor_hook *new_hook)
{
	GSList *list;
	fabulor_hook *hook;
	int new_hook_type;
 
	switch (new_hook->type)
	{
		case HOOK_PRINT:
		case HOOK_PRINT_ATTRS:
			new_hook_type = HOOK_PRINT | HOOK_PRINT_ATTRS;
			break;
		case HOOK_SERVER:
		case HOOK_SERVER_ATTRS:
			new_hook_type = HOOK_SERVER | HOOK_PRINT_ATTRS;
			break;
		default:
			new_hook_type = new_hook->type;
	}

	list = hook_list;
	while (list)
	{
		hook = list->data;
		if (hook && (hook->type & new_hook_type) && hook->pri <= new_hook->pri)
		{
			hook_list = g_slist_insert_before (hook_list, list, new_hook);
			return;
		}
		list = list->next;
	}

	hook_list = g_slist_append (hook_list, new_hook);
}

static gboolean
plugin_fd_cb (GIOChannel *source, GIOCondition condition, fabulor_hook *hook)
{
	int flags = 0, ret;
	typedef int (fabulor_fd_cb2) (int fd, int flags, void *user_data, GIOChannel *);

	if (condition & G_IO_IN)
		flags |= FABULOR_FD_READ;
	if (condition & G_IO_OUT)
		flags |= FABULOR_FD_WRITE;
	if (condition & G_IO_PRI)
		flags |= FABULOR_FD_EXCEPTION;

	ret = ((fabulor_fd_cb2 *)hook->callback) (hook->pri, flags, hook->userdata, source);

	/* the callback might have already unhooked it! */
	if (!g_slist_find (hook_list, hook) || hook->type == HOOK_DELETED)
		return 0;

	if (ret == 0)
	{
		hook->tag = 0; /* avoid fe_input_remove, returning 0 is enough! */
		fabulor_unhook (hook->pl, hook);
	}

	return ret;
}

/* allocate and add a hook to our list. Used for all 4 types */

static fabulor_hook *
plugin_add_hook (fabulor_plugin *pl, int type, int pri, const char *name,
					  const  char *help_text, void *callb, int timeout, void *userdata)
{
	fabulor_hook *hook;

	hook = g_new0 (fabulor_hook, 1);
	hook->type = type;
	hook->pri = pri;
	hook->name = g_strdup (name);
	hook->help_text = g_strdup (help_text);
	hook->callback = callb;
	hook->pl = pl;
	hook->userdata = userdata;

	/* insert it into the linked list */
	plugin_insert_hook (hook);

	if (type == HOOK_TIMER)
		hook->tag = fe_timeout_add (timeout, plugin_timeout_cb, hook);

	return hook;
}

GList *
plugin_command_list(GList *tmp_list)
{
	fabulor_hook *hook;
	GSList *list = hook_list;

	while (list)
	{
		hook = list->data;
		if (hook && hook->type == HOOK_COMMAND)
			tmp_list = g_list_prepend(tmp_list, hook->name);
		list = list->next;
	}
	return tmp_list;
}

void
plugin_command_foreach (session *sess, void *userdata,
			void (*cb) (session *sess, void *userdata, char *name, char *help))
{
	GSList *list;
	fabulor_hook *hook;

	list = hook_list;
	while (list)
	{
		hook = list->data;
		if (hook && hook->type == HOOK_COMMAND && hook->name[0])
		{
			cb (sess, userdata, hook->name, hook->help_text);
		}
		list = list->next;
	}
}

int
plugin_show_help (session *sess, char *cmd)
{
	GSList *list;
	fabulor_hook *hook;

	list = plugin_hook_find (hook_list, HOOK_COMMAND, cmd);
	if (list)
	{
		hook = list->data;
		if (hook->help_text)
		{
			PrintText (sess, hook->help_text);
			return 1;
		}
	}

	return 0;
}

session *
plugin_find_context (const char *servname, const char *channel, server *current_server)
{
	GSList *slist, *clist, *sessions = NULL;
	server *serv;
	session *sess;
	char *netname;

	if (servname == NULL && channel == NULL)
		return current_sess;

	slist = serv_list;
	while (slist)
	{
		serv = slist->data;
		netname = server_get_network (serv, TRUE);

		if (servname == NULL ||
			 rfc_casecmp (servname, serv->servername) == 0 ||
			 g_ascii_strcasecmp (servname, serv->hostname) == 0 ||
			 g_ascii_strcasecmp (servname, netname) == 0)
		{
			if (channel == NULL)
				return serv->front_session;

			clist = sess_list;
			while (clist)
			{
				sess = clist->data;
				if (sess->server == serv)
				{
					if (rfc_casecmp (channel, sess->channel) == 0)
					{
						if (sess->server == current_server)
						{
							g_slist_free (sessions);
							return sess;
						} else
						{
							sessions = g_slist_prepend (sessions, sess);
						}
					}
				}
				clist = clist->next;
			}
		}
		slist = slist->next;
	}

	if (sessions)
	{
		sessions = g_slist_reverse (sessions);
		sess = sessions->data;
		g_slist_free (sessions);
		return sess;
	}

	return NULL;
}


/* ========================================================= */
/* ===== these are the functions plugins actually call ===== */
/* ========================================================= */

void *
fabulor_unhook (fabulor_plugin *ph, fabulor_hook *hook)
{
	/* A callback may attempt to unhook an already deleted hook. */
	if (!g_slist_find (hook_list, hook) || hook->type == HOOK_DELETED)
		return NULL;

	if (hook->type == HOOK_TIMER && hook->tag != 0)
		fe_timeout_remove (hook->tag);

	if (hook->type == HOOK_FD && hook->tag != 0)
		fe_input_remove (hook->tag);

	hook->type = HOOK_DELETED;	/* expunge later */

	g_free (hook->name);	/* NULL for timers & fds */
	g_free (hook->help_text);	/* NULL for non-commands */

	return hook->userdata;
}

fabulor_hook *
fabulor_hook_command (fabulor_plugin *ph, const char *name, int pri,
						  fabulor_cmd_cb *callb, const char *help_text, void *userdata)
{
	return plugin_add_hook (ph, HOOK_COMMAND, pri, name, help_text, callb, 0,
									userdata);
}

fabulor_hook *
fabulor_hook_server (fabulor_plugin *ph, const char *name, int pri,
						 fabulor_serv_cb *callb, void *userdata)
{
	return plugin_add_hook (ph, HOOK_SERVER, pri, name, 0, callb, 0, userdata);
}

fabulor_hook *
fabulor_hook_server_attrs (fabulor_plugin *ph, const char *name, int pri,
						   fabulor_serv_attrs_cb *callb, void *userdata)
{
	return plugin_add_hook (ph, HOOK_SERVER_ATTRS, pri, name, 0, callb, 0,
							userdata);
}

fabulor_hook *
fabulor_hook_print (fabulor_plugin *ph, const char *name, int pri,
						fabulor_print_cb *callb, void *userdata)
{
	return plugin_add_hook (ph, HOOK_PRINT, pri, name, 0, callb, 0, userdata);
}

fabulor_hook *
fabulor_hook_print_attrs (fabulor_plugin *ph, const char *name, int pri,
						  fabulor_print_attrs_cb *callb, void *userdata)
{
	return plugin_add_hook (ph, HOOK_PRINT_ATTRS, pri, name, 0, callb, 0,
							userdata);
}

fabulor_hook *
fabulor_hook_timer (fabulor_plugin *ph, int timeout, fabulor_timer_cb *callb,
					   void *userdata)
{
	return plugin_add_hook (ph, HOOK_TIMER, 0, 0, 0, callb, timeout, userdata);
}

fabulor_hook *
fabulor_hook_fd (fabulor_plugin *ph, int fd, int flags,
					fabulor_fd_cb *callb, void *userdata)
{
	fabulor_hook *hook;

	hook = plugin_add_hook (ph, HOOK_FD, 0, 0, 0, callb, 0, userdata);
	hook->pri = fd;
	/* plugin hook_fd flags correspond exactly to FIA_* flags (fe.h) */
	hook->tag = fe_input_add (fd, flags, plugin_fd_cb, hook);

	return hook;
}

void
fabulor_print (fabulor_plugin *ph, const char *text)
{
	if (!is_session (ph->context))
	{
		DEBUG(PrintTextf(0, "%s\tfabulor_print called without a valid context.\n", ph->name));
		return;
	}

	PrintText (ph->context, (char *)text);
}

void
fabulor_printf (fabulor_plugin *ph, const char *format, ...)
{
	va_list args;
	char *buf;

	va_start (args, format);
	buf = g_strdup_vprintf (format, args);
	va_end (args);

	fabulor_print (ph, buf);
	g_free (buf);
}

void
fabulor_command (fabulor_plugin *ph, const char *command)
{
	char *command_utf8;

	if (!is_session (ph->context))
	{
		DEBUG(PrintTextf(0, "%s\tfabulor_command called without a valid context.\n", ph->name));
		return;
	}

	/* scripts/plugins continue to send non-UTF8... *sigh* */
	command_utf8 = text_fixup_invalid_utf8 (command, -1, NULL);
	handle_command (ph->context, command_utf8, FALSE);
	g_free (command_utf8);
}

void
fabulor_commandf (fabulor_plugin *ph, const char *format, ...)
{
	va_list args;
	char *buf;

	va_start (args, format);
	buf = g_strdup_vprintf (format, args);
	va_end (args);

	fabulor_command (ph, buf);
	g_free (buf);
}

int
fabulor_nickcmp (fabulor_plugin *ph, const char *s1, const char *s2)
{
	return ((session *)ph->context)->server->p_cmp (s1, s2);
}

fabulor_context *
fabulor_get_context (fabulor_plugin *ph)
{
	return ph->context;
}

int
fabulor_set_context (fabulor_plugin *ph, fabulor_context *context)
{
	if (is_session (context))
	{
		ph->context = context;
		return 1;
	}
	return 0;
}

fabulor_context *
fabulor_find_context (fabulor_plugin *ph, const char *servname, const char *channel)
{
	return plugin_find_context (servname, channel, ph->context->server);
}

const char *
fabulor_get_info (fabulor_plugin *ph, const char *id)
{
	session *sess;
	guint32 hash;

	/*                 1234567890 */
	if (!strncmp (id, "event_text", 10))
	{
		char *e = (char *)id + 10;
		if (*e == ' ') e++;	/* 2.8.0 only worked without a space */
		return text_find_format_string (e);
	}

	hash = str_hash (id);
	/* do the session independant ones first */
	switch (hash)
	{
		case 0x325acab5:	/* libdirfs */
#ifdef USE_PLUGIN
			return plugin_get_libdir ();
#else
			return NULL;
#endif

		case 0x14f51cd8: /* version */
			return PACKAGE_VERSION;

		case 0xdd9b1abd:	/* xchatdir */
		case 0xe33f6c4a:	/* xchatdirfs */
		case 0xd00d220b:	/* configdir */
			return get_xdir ();
	}

	sess = ph->context;
	if (!is_session (sess))
	{
		DEBUG(PrintTextf(0, "%s\tfabulor_get_info called without a valid context.\n", ph->name));
		return NULL;
	}

	switch (hash)
	{
	case 0x2de2ee: /* away */
		if (sess->server->is_away)
			return sess->server->last_away_reason;
		return NULL;

  	case 0x2c0b7d03: /* channel */
		return sess->channel;

	case 0x2c0d614c: /* charset */
		{
			const char *locale;

			if (sess->server->encoding)
				return sess->server->encoding;

			locale = NULL;
			g_get_charset (&locale);
			return locale;
		}

	case 0x30f5a8: /* host */
		return sess->server->hostname;

	case 0x1c0e99c1: /* inputbox */
		return fe_get_inputbox_contents (sess);

	case 0x633fb30:	/* modes */
		return sess->current_modes;

	case 0x6de15a2e:	/* network */
		return server_get_network (sess->server, FALSE);

	case 0x339763: /* nick */
		return sess->server->nick;

	case 0x4889ba9b: /* password */
	case 0x438fdf9: /* nickserv */
		return NULL;

	case 0xca022f43: /* server */
		if (!sess->server->connected)
			return NULL;
		return sess->server->servername;

	case 0x696cd2f: /* topic */
		return sess->topic;

	case 0x3419f12d: /* gtkwin_ptr */
		return fe_gui_info_ptr (sess, 1);

	case 0x506d600b: /* native win_ptr */
		return fe_gui_info_ptr (sess, 0);

	case 0x6d3431b5: /* win_status */
		switch (fe_gui_info (sess, 0))	/* check window status */
		{
		case 0: return "normal";
		case 1: return "active";
		case 2: return "hidden";
		}
		return NULL;
	}

	return NULL;
}

int
fabulor_get_prefs (fabulor_plugin *ph, const char *name, const char **string, int *integer)
{
	int i = 0;

	/* some special run-time info (not really prefs, but may aswell throw it in here) */
	switch (str_hash (name))
	{
		case 0xf82136c4: /* state_cursor */
			*integer = fe_get_inputbox_cursor (ph->context);
			return 2;

		case 0xd1b: /* id */
			*integer = ph->context->server->id;
			return 2;
	}
	
	do
	{
		if (!g_ascii_strcasecmp (name, vars[i].name))
		{
			switch (vars[i].type)
			{
			case TYPE_STR:
				*string = ((char *) &prefs + vars[i].offset);
				return 1;

			case TYPE_INT:
				*integer = *((int *) &prefs + vars[i].offset);
				return 2;

			default:
			/*case TYPE_BOOL:*/
				if (*((int *) &prefs + vars[i].offset))
					*integer = 1;
				else
					*integer = 0;
				return 3;
			}
		}
		i++;
	}
	while (vars[i].name);

	return 0;
}

fabulor_list *
fabulor_list_get (fabulor_plugin *ph, const char *name)
{
	fabulor_list *list;

	list = g_new0 (fabulor_list, 1);

	switch (str_hash (name))
	{
	case 0x556423d0: /* channels */
		list->type = LIST_CHANNELS;
		list->next = sess_list;
		break;

	case 0x183c4:	/* dcc */
		list->type = LIST_DCC;
		list->next = dcc_list;
		break;

	case 0xb90bfdd2:	/* ignore */
		list->type = LIST_IGNORE;
		list->next = ignore_list;
		break;

	case 0xc2079749:	/* notify */
		list->type = LIST_NOTIFY;
		list->next = notify_list;
		list->head = (void *)ph->context;	/* reuse this pointer */
		break;

	case 0x6a68e08: /* users */
		if (is_session (ph->context))
		{
			list->type = LIST_USERS;
			list->head = list->next = userlist_flat_list (ph->context);
			fe_userlist_set_selected (ph->context);
			break;
		}	/* fall through */

	default:
		g_free (list);
		return NULL;
	}

	return list;
}

void
fabulor_list_free (fabulor_plugin *ph, fabulor_list *xlist)
{
	if (xlist->type == LIST_USERS)
		g_slist_free (xlist->head);
	g_free (xlist);
}

int
fabulor_list_next (fabulor_plugin *ph, fabulor_list *xlist)
{
	if (xlist->next == NULL)
		return 0;

	xlist->pos = xlist->next;
	xlist->next = xlist->pos->next;

	/* NOTIFY LIST: Find the entry which matches the context
		of the plugin when list_get was originally called. */
	if (xlist->type == LIST_NOTIFY)
	{
		xlist->notifyps = notify_find_server_entry (xlist->pos->data,
													((session *)xlist->head)->server);
		if (!xlist->notifyps)
			return 0;
	}

	return 1;
}

const char * const *
fabulor_list_fields (fabulor_plugin *ph, const char *name)
{
	static const char * const dcc_fields[] =
	{
		"iaddress32","icps",		"sdestfile","sfile",		"snick",	"iport",
		"ipos", "iposhigh", "iresume", "iresumehigh", "isize", "isizehigh", "istatus", "itype", NULL
	};
	static const char * const channels_fields[] =
	{
		"schannel", "schannelkey", "schanmodes", "schantypes", "pcontext", "iflags", "iid", "ilag", "imaxmodes",
		"snetwork", "snickmodes", "snickprefixes", "iqueue", "sserver", "itype", "iusers",
		NULL
	};
	static const char * const ignore_fields[] =
	{
		"iflags", "smask", NULL
	};
	static const char * const notify_fields[] =
	{
		"iflags", "snetworks", "snick", "toff", "ton", "tseen", NULL
	};
	static const char * const users_fields[] =
	{
		"saccount", "iaway", "shost", "tlasttalk", "snick", "sprefix", "srealname", "iselected", NULL
	};
	static const char * const list_of_lists[] =
	{
		"channels",	"dcc", "ignore", "notify", "users", NULL
	};

	switch (str_hash (name))
	{
	case 0x556423d0:	/* channels */
		return channels_fields;
	case 0x183c4:		/* dcc */
		return dcc_fields;
	case 0xb90bfdd2:	/* ignore */
		return ignore_fields;
	case 0xc2079749:	/* notify */
		return notify_fields;
	case 0x6a68e08:	/* users */
		return users_fields;
	case 0x6236395:	/* lists */
		return list_of_lists;
	}

	return NULL;
}

time_t
fabulor_list_time (fabulor_plugin *ph, fabulor_list *xlist, const char *name)
{
	guint32 hash = str_hash (name);
	gpointer data;

	switch (xlist->type)
	{
	case LIST_NOTIFY:
		if (!xlist->notifyps)
			return (time_t) -1;
		switch (hash)
		{
		case 0x1ad6f:	/* off */
			return xlist->notifyps->lastoff;
		case 0xddf:	/* on */
			return xlist->notifyps->laston;
		case 0x35ce7b:	/* seen */
			return xlist->notifyps->lastseen;
		}
		break;

	case LIST_USERS:
		data = xlist->pos->data;
		switch (hash)
		{
		case 0xa9118c42:	/* lasttalk */
			return ((struct User *)data)->lasttalk;
		}
	}

	return (time_t) -1;
}

const char *
fabulor_list_str (fabulor_plugin *ph, fabulor_list *xlist, const char *name)
{
	guint32 hash = str_hash (name);
	gpointer data = ph->context;
	int type = LIST_CHANNELS;

	/* a NULL xlist is a shortcut to current "channels" context */
	if (xlist)
	{
		data = xlist->pos->data;
		type = xlist->type;
	}

	switch (type)
	{
	case LIST_CHANNELS:
		switch (hash)
		{
		case 0x2c0b7d03: /* channel */
			return ((session *)data)->channel;
		case 0x8cea5e7c: /* channelkey */
			return ((session *)data)->channelkey;
		case 0x5716ab1e: /* chanmodes */
			return ((session*)data)->server->chanmodes;
		case 0x577e0867: /* chantypes */
			return ((session *)data)->server->chantypes;
		case 0x38b735af: /* context */
			return data;	/* this is a session * */
		case 0x6de15a2e: /* network */
			return server_get_network (((session *)data)->server, FALSE);
		case 0x8455e723: /* nickprefixes */
			return ((session *)data)->server->nick_prefixes;
		case 0x829689ad: /* nickmodes */
			return ((session *)data)->server->nick_modes;
		case 0xca022f43: /* server */
			return ((session *)data)->server->servername;
		}
		break;

	case LIST_DCC:
		switch (hash)
		{
		case 0x3d9ad31e:	/* destfile */
			return ((struct DCC *)data)->destfile;
		case 0x2ff57c:	/* file */
			return ((struct DCC *)data)->file;
		case 0x339763: /* nick */
			return ((struct DCC *)data)->nick;
		}
		break;

	case LIST_IGNORE:
		switch (hash)
		{
		case 0x3306ec:	/* mask */
			return ((struct ignore *)data)->mask;
		}
		break;

	case LIST_NOTIFY:
		switch (hash)
		{
		case 0x4e49ec05:	/* networks */
			return ((struct notify *)data)->networks;
		case 0x339763: /* nick */
			return ((struct notify *)data)->name;
		}
		break;

	case LIST_USERS:
		switch (hash)
		{
		case 0xb9d38a2d: /* account */
			return ((struct User *)data)->account;
		case 0x339763: /* nick */
			return ((struct User *)data)->nick;
		case 0x30f5a8: /* host */
			return ((struct User *)data)->hostname;
		case 0xc594b292: /* prefix */
			return ((struct User *)data)->prefix;
		case 0xccc6d529: /* realname */
			return ((struct User *)data)->realname;
		}
		break;
	}

	return NULL;
}

int
fabulor_list_int (fabulor_plugin *ph, fabulor_list *xlist, const char *name)
{
	guint32 hash = str_hash (name);
	gpointer data = ph->context;

	int channel_flag;
	int channel_flags[CHANNEL_FLAG_COUNT];
	int channel_flags_used = 0;

	int type = LIST_CHANNELS;

	/* a NULL xlist is a shortcut to current "channels" context */
	if (xlist)
	{
		data = xlist->pos->data;
		type = xlist->type;
	}

	switch (type)
	{
	case LIST_DCC:
		switch (hash)
		{
		case 0x34207553: /* address32 */
			return ((struct DCC *)data)->addr;
		case 0x181a6: /* cps */
		{
			gint64 cps = ((struct DCC *)data)->cps;
			if (cps <= INT_MAX)
			{
				return (int) cps;
			}
			return INT_MAX;
		}
		case 0x349881: /* port */
			return ((struct DCC *)data)->port;
		case 0x1b254: /* pos */
			return ((struct DCC *)data)->pos & 0xffffffff;
		case 0xe8a945f6: /* poshigh */
			return (((struct DCC *)data)->pos >> 32) & 0xffffffff;
		case 0xc84dc82d: /* resume */
			return ((struct DCC *)data)->resumable & 0xffffffff;
		case 0xded4c74f: /* resumehigh */
			return (((struct DCC *)data)->resumable >> 32) & 0xffffffff;
		case 0x35e001: /* size */
			return ((struct DCC *)data)->size & 0xffffffff;
		case 0x3284d523: /* sizehigh */
			return (((struct DCC *)data)->size >> 32) & 0xffffffff;
		case 0xcacdcff2: /* status */
			return ((struct DCC *)data)->dccstat;
		case 0x368f3a: /* type */
			return ((struct DCC *)data)->type;
		}
		break;

	case LIST_IGNORE:
		switch (hash)
		{
		case 0x5cfee87:	/* flags */
			return ((struct ignore *)data)->type;
		}
		break;

	case LIST_CHANNELS:
		switch (hash)
		{
		case 0xd1b:	/* id */
			return ((struct session *)data)->server->id;
		case 0x5cfee87:	/* flags */
			channel_flags[0] = ((struct session *)data)->server->connected;
			channel_flags[1] = ((struct session *)data)->server->connecting;
			channel_flags[2] = ((struct session *)data)->server->is_away;
			channel_flags[3] = ((struct session *)data)->server->end_of_motd;
			channel_flags[4] = ((struct session *)data)->server->have_whox;
			channel_flags[5] = ((struct session *)data)->server->have_idmsg;
			channel_flags[6] = ((struct session *)data)->text_hidejoinpart;
			channel_flags[7] = ((struct session *)data)->text_hidejoinpart == SET_DEFAULT;
			channel_flags[8] = ((struct session *)data)->alert_beep;
			channel_flags[9] = ((struct session *)data)->alert_beep == SET_DEFAULT;
			channel_flags[10] = 0; /* unused for historical reasons */
			channel_flags[11] = ((struct session *)data)->text_logging;
			channel_flags[12] = ((struct session *)data)->text_logging == SET_DEFAULT;
			channel_flags[13] = ((struct session *)data)->text_scrollback;
			channel_flags[14] = ((struct session *)data)->text_scrollback == SET_DEFAULT;
			channel_flags[15] = ((struct session *)data)->text_strip;
			channel_flags[16] = ((struct session *)data)->text_strip == SET_DEFAULT;
			channel_flags[17] = ((struct session *)data)->alert_tray;
			channel_flags[18] = ((struct session *)data)->alert_tray == SET_DEFAULT;
			channel_flags[19] = ((struct session *)data)->alert_taskbar;
			channel_flags[20] = ((struct session *)data)->alert_taskbar == SET_DEFAULT;
			channel_flags[21] = ((struct session *)data)->alert_balloon;
			channel_flags[22] = ((struct session *)data)->alert_balloon == SET_DEFAULT;

			/* Set flags */
			for (channel_flag = 0; channel_flag < CHANNEL_FLAG_COUNT; ++channel_flag) {
				if (channel_flags[channel_flag]) {
					channel_flags_used |= 1 << channel_flag;
				}
			}

			return channel_flags_used;
		case 0x1a192: /* lag */
			return ((struct session *)data)->server->lag;
		case 0x1916144c: /* maxmodes */
			return ((struct session *)data)->server->modes_per_line;
		case 0x66f1911: /* queue */
			return ((struct session *)data)->server->sendq_len;
		case 0x368f3a:	/* type */
			return ((struct session *)data)->type;
		case 0x6a68e08: /* users */
			return ((struct session *)data)->total;
		}
		break;

	case LIST_NOTIFY:
		if (!xlist->notifyps)
			return -1;
		switch (hash)
		{
		case 0x5cfee87: /* flags */
			return xlist->notifyps->ison;
		}

	case LIST_USERS:
		switch (hash)
		{
		case 0x2de2ee:	/* away */
			return ((struct User *)data)->away;
		case 0x4705f29b: /* selected */
			return ((struct User *)data)->selected;
		}
		break;

	}

	return -1;
}

void *
fabulor_plugingui_add (fabulor_plugin *ph, const char *filename,
							const char *name, const char *desc,
							const char *version, char *reserved)
{
#ifdef USE_PLUGIN
	ph = plugin_list_add (NULL, g_strdup (filename), g_strdup (name), g_strdup (desc),
								 g_strdup (version), NULL, NULL, TRUE, TRUE);
	fe_pluginlist_update ();
#endif

	return ph;
}

void
fabulor_plugingui_remove (fabulor_plugin *ph, void *handle)
{
#ifdef USE_PLUGIN
	plugin_free (handle, FALSE, FALSE);
#endif
}

int
fabulor_emit_print (fabulor_plugin *ph, const char *event_name, ...)
{
	va_list args;
	/* currently only 4 because no events use more than 4.
		This can be easily expanded without breaking the API. */
	char *argv[4] = {NULL, NULL, NULL, NULL};
	int i = 0;

	va_start (args, event_name);
	while (1)
	{
		argv[i] = va_arg (args, char *);
		if (!argv[i])
			break;
		i++;
		if (i >= 4)
			break;
	}

	i = text_emit_by_name ((char *)event_name, ph->context, (time_t) 0,
						   argv[0], argv[1], argv[2], argv[3]);
	va_end (args);

	return i;
}

int
fabulor_emit_print_attrs (fabulor_plugin *ph, fabulor_event_attrs *attrs,
						  const char *event_name, ...)
{
	va_list args;
	/* currently only 4 because no events use more than 4.
		This can be easily expanded without breaking the API. */
	char *argv[4] = {NULL, NULL, NULL, NULL};
	int i = 0;

	va_start (args, event_name);
	while (1)
	{
		argv[i] = va_arg (args, char *);
		if (!argv[i])
			break;
		i++;
		if (i >= 4)
			break;
	}

	i = text_emit_by_name ((char *)event_name, ph->context, attrs->server_time_utc,
						   argv[0], argv[1], argv[2], argv[3]);
	va_end (args);

	return i;
}

char *
fabulor_gettext (fabulor_plugin *ph, const char *msgid)
{
	/* so that plugins can use Fabulor's internal gettext strings. */
	/* e.g. The EXEC plugin uses this on Windows. */
	return _(msgid);
}

void
fabulor_send_modes (fabulor_plugin *ph, const char **targets, int ntargets, int modes_per_line, char sign, char mode)
{
	char tbuf[514];	/* modes.c needs 512 + null */

	send_channel_modes (ph->context, tbuf, (char **)targets, 0, ntargets, sign, mode, modes_per_line);
}

char *
fabulor_strip (fabulor_plugin *ph, const char *str, int len, int flags)
{
	return strip_color ((char *)str, len, flags);
}

void
fabulor_free (fabulor_plugin *ph, void *ptr)
{
	g_free (ptr);
}

static int
fabulor_pluginpref_set_str_real (fabulor_plugin *pl, const char *var, const char *value, int mode) /* mode: 0 = delete, 1 = save */
{
	FILE *fpIn;
	int fhOut;
	int prevSetting;
	char *confname;
	char *confname_tmp;
	char *escaped_value;
	char *buffer;
	char *buffer_tmp;
	char line_buffer[512];		/* the same as in cfg_put_str */
	char *line_bufp = line_buffer;
	char *canon;

	canon = g_strdup (pl->name);
	canonalize_key (canon);
	confname = g_strdup_printf ("addon_%s.conf", canon);
	g_free (canon);
	confname_tmp = g_strdup_printf ("%s.new", confname);

	fhOut = fabulor_open_file (confname_tmp, O_TRUNC | O_WRONLY | O_CREAT, 0600, XOF_DOMODE);
	fpIn = fabulor_fopen_file (confname, "r", 0);

	if (fhOut == -1)		/* unable to save, abort */
	{
		g_free (confname);
		g_free (confname_tmp);
		if (fpIn)
			fclose (fpIn);
		return 0;
	}
	else if (fpIn == NULL)	/* no previous config file, no parsing */
	{
		if (mode)
		{
			escaped_value = g_strescape (value, NULL);
			buffer = g_strdup_printf ("%s = %s\n", var, escaped_value);
			g_free (escaped_value);
			write (fhOut, buffer, strlen (buffer));
			g_free (buffer);
			close (fhOut);

			buffer = g_build_filename (get_xdir (), confname, NULL);
			g_free (confname);
			buffer_tmp = g_build_filename (get_xdir (), confname_tmp, NULL);
			g_free (confname_tmp);

#ifdef WIN32
			g_unlink (buffer);
#endif

			if (g_rename (buffer_tmp, buffer) == 0)
			{
				g_free (buffer);
				g_free (buffer_tmp);
				return 1;
			}
			else
			{
				g_free (buffer);
				g_free (buffer_tmp);
				return 0;
			}
		}
		else
		{
			/* mode = 0, we want to delete but the config file and thus the given setting does not exist, we're ready */
			close (fhOut);
			g_free (confname);
			g_free (confname_tmp);
			return 1;
		}
	}
	else	/* existing config file, preserve settings and find & replace current var value if any */
	{
		prevSetting = 0;

		while (fscanf (fpIn, " %511[^\n]", line_bufp) != EOF)	/* read whole lines including whitespaces */
		{
			buffer_tmp = g_strdup_printf ("%s ", var);	/* add one space, this way it works against var - var2 checks too */

			if (strncmp (buffer_tmp, line_buffer, strlen (var) + 1) == 0)	/* given setting already exists */
			{
				if (mode)									/* overwrite the existing matching setting if we are in save mode */
				{
					escaped_value = g_strescape (value, NULL);
					buffer = g_strdup_printf ("%s = %s\n", var, escaped_value);
					g_free (escaped_value);
				}
				else										/* erase the setting in delete mode */
				{
					buffer = g_strdup ("");
				}

				prevSetting = 1;
			}
			else
			{
				buffer = g_strdup_printf ("%s\n", line_buffer);	/* preserve the existing different settings */
			}

			write (fhOut, buffer, strlen (buffer));

			g_free (buffer);
			g_free (buffer_tmp);
		}

		fclose (fpIn);

		if (!prevSetting && mode)	/* var doesn't exist currently, append if we're in save mode */
		{
			escaped_value = g_strescape (value, NULL);
			buffer = g_strdup_printf ("%s = %s\n", var, escaped_value);
			g_free (escaped_value);
			write (fhOut, buffer, strlen (buffer));
			g_free (buffer);
		}

		close (fhOut);

		buffer = g_build_filename (get_xdir (), confname, NULL);
		g_free (confname);
		buffer_tmp = g_build_filename (get_xdir (), confname_tmp, NULL);
		g_free (confname_tmp);

#ifdef WIN32
		g_unlink (buffer);
#endif

		if (g_rename (buffer_tmp, buffer) == 0)
		{
			g_free (buffer);
			g_free (buffer_tmp);
			return 1;
		}
		else
		{
			g_free (buffer);
			g_free (buffer_tmp);
			return 0;
		}
	}
}

int
fabulor_pluginpref_set_str (fabulor_plugin *pl, const char *var, const char *value)
{
	return fabulor_pluginpref_set_str_real (pl, var, value, 1);
}

static int
fabulor_pluginpref_get_str_real (fabulor_plugin *pl, const char *var, char *dest, int dest_len)
{
	char *confname, *canon, *cfg, *unescaped_value;
	char buf[512];

	canon = g_strdup (pl->name);
	canonalize_key (canon);
	confname = g_strdup_printf ("%s%caddon_%s.conf", get_xdir(), G_DIR_SEPARATOR, canon);
	g_free (canon);

	if (!g_file_get_contents (confname, &cfg, NULL, NULL))
	{
		g_free (confname);
		return 0;
	}
	g_free (confname);

	if (!cfg_get_str (cfg, var, buf, sizeof(buf)))
	{
		g_free (cfg);
		return 0;
	}

	unescaped_value = g_strcompress (buf);
	g_strlcpy (dest, unescaped_value, dest_len);

	g_free (unescaped_value);
	g_free (cfg);
	return 1;
}

int
fabulor_pluginpref_get_str (fabulor_plugin *pl, const char *var, char *dest)
{
	/* All users of this must ensure dest is >= 512... */
	return fabulor_pluginpref_get_str_real (pl, var, dest, 512);
}

int
fabulor_pluginpref_set_int (fabulor_plugin *pl, const char *var, int value)
{
	char buffer[12];

	g_snprintf (buffer, sizeof (buffer), "%d", value);
	return fabulor_pluginpref_set_str_real (pl, var, buffer, 1);
}

int
fabulor_pluginpref_get_int (fabulor_plugin *pl, const char *var)
{
	char buffer[12];

	if (fabulor_pluginpref_get_str_real (pl, var, buffer, sizeof(buffer)))
	{
		int ret = atoi (buffer);

		/* 0 could be success or failure, who knows */
		if (ret == 0 && *buffer != '0')
			return -1;

		return ret;
	}
	else
	{
		return -1;
	}
}

int
fabulor_pluginpref_delete (fabulor_plugin *pl, const char *var)
{
	return fabulor_pluginpref_set_str_real (pl, var, 0, 0);
}

int
fabulor_pluginpref_list (fabulor_plugin *pl, char* dest)
{
	FILE *fpIn;
	char confname[64];
	char buffer[512];										/* the same as in cfg_put_str */
	char *bufp = buffer;
	char *token;

	token = g_strdup (pl->name);
	canonalize_key (token);
	sprintf (confname, "addon_%s.conf", token);
	g_free (token);

	fpIn = fabulor_fopen_file (confname, "r", 0);

	if (fpIn == NULL)										/* no existing config file, no parsing */
	{
		return 0;
	}
	else													/* existing config file, get list of settings */
	{
		strcpy (dest, "");									/* clean up garbage */
		while (fscanf (fpIn, " %511[^\n]", bufp) != EOF)	/* read whole lines including whitespaces */
		{
			token = strtok (buffer, "=");
			g_strlcat (dest, g_strchomp (token), 4096); /* Dest must not be smaller than this */
			g_strlcat (dest, ",", 4096);
		}

		fclose (fpIn);
	}

	return 1;
}
