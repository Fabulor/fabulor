/* ZoiteChat
 * Copyright (C) 1998-2010 Peter Zelezny.
 * Copyright (C) 2009-2013 Berke Viktor.
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

#include <glib.h>
#include <stdarg.h>
#include <string.h>

#include "zoitechat.h"
#include "plugin.h"
#include "outbound.h"
#include "fabulor-plugin-host.h"

typedef struct
{
	FabulorCallbackRegistry *registry;
	char *event_name;
	char *event_payload_json;
	void *loader_user_data;
} FabulorDeferredDispatch;

struct _fabulor_plugin_catalog
{
	const FabulorAPI *api;
	GPtrArray *manifests;
	GHashTable *manifest_index;
	GHashTable *blacklist;
	GPtrArray *diagnostics;
	gboolean safe_mode_enabled;
};

struct _fabulor_callback_registry
{
	const FabulorAPI *api;
	const FabulorPluginCatalog *catalog;
	GHashTable *entries;
	GMainContext *main_context;
	GThread *main_thread;
};

static void
fabulor_api_log (const FabulorAPI *api, const char *format, ...)
{
	char *message;
	va_list args;

	if (!api || !api->log || !format)
	{
		return;
	}

	va_start (args, format);
	message = g_strdup_vprintf (format, args);
	va_end (args);

	api->log (api->user_data, message);
	g_free (message);
}

static void
fabulor_plugin_catalog_add_diagnostic (FabulorPluginCatalog *catalog, const char *format, ...)
{
	char *message;
	va_list args;

	if (!catalog || !format)
	{
		return;
	}

	va_start (args, format);
	message = g_strdup_vprintf (format, args);
	va_end (args);

	g_ptr_array_add (catalog->diagnostics, message);
	fabulor_api_log (catalog->api, "%s", message);
}

static gboolean
extract_string_field (const char *json, const char *field_name, char **value, GError **error)
{
	GRegex *regex;
	GMatchInfo *match_info;
	gboolean matched;
	gboolean success;
	char *pattern;
	char *field;

	*value = NULL;

	field = g_regex_escape_string (field_name, -1);
	pattern = g_strdup_printf ("\"%s\"\\s*:\\s*\"((?:\\\\.|[^\"\\\\])*)\"", field);
	regex = g_regex_new (pattern, G_REGEX_DOTALL, 0, error);
	g_free (field);
	g_free (pattern);

	if (!regex)
	{
		return FALSE;
	}

	matched = g_regex_match (regex, json, 0, &match_info);
	success = TRUE;

	if (matched)
	{
		char *raw = g_match_info_fetch (match_info, 1);
		*value = g_strcompress (raw);
		g_free (raw);
	}

	g_match_info_free (match_info);
	g_regex_unref (regex);
	return success;
}

static gboolean
extract_uint_field (const char *json, const char *field_name, guint *value, GError **error)
{
	GRegex *regex;
	GMatchInfo *match_info;
	char *pattern;
	char *field;
	gboolean matched;
	gboolean success;

	field = g_regex_escape_string (field_name, -1);
	pattern = g_strdup_printf ("\"%s\"\\s*:\\s*(?:\"([0-9]+)\"|([0-9]+))", field);
	regex = g_regex_new (pattern, G_REGEX_DOTALL, 0, error);
	g_free (field);
	g_free (pattern);

	if (!regex)
	{
		return FALSE;
	}

	matched = g_regex_match (regex, json, 0, &match_info);
	success = TRUE;
	*value = 0;

	if (matched)
	{
		char *raw = g_match_info_fetch (match_info, 1);
		if (!raw || *raw == '\0')
		{
			g_free (raw);
			raw = g_match_info_fetch (match_info, 2);
		}

		if (raw && *raw != '\0')
		{
			*value = (guint) g_ascii_strtoull (raw, NULL, 10);
		}

		g_free (raw);
	}

	g_match_info_free (match_info);
	g_regex_unref (regex);
	return success;
}

static gboolean
extract_string_array_field (const char *json, const char *field_name, GPtrArray *values, GError **error)
{
	GRegex *array_regex;
	GMatchInfo *array_match;
	char *array_pattern;
	char *field;
	gboolean matched;
	gboolean success;

	field = g_regex_escape_string (field_name, -1);
	array_pattern = g_strdup_printf ("\"%s\"\\s*:\\s*\\[(.*?)\\]", field);
	array_regex = g_regex_new (array_pattern, G_REGEX_DOTALL, 0, error);
	g_free (field);
	g_free (array_pattern);

	if (!array_regex)
	{
		return FALSE;
	}

	matched = g_regex_match (array_regex, json, 0, &array_match);
	success = TRUE;

	if (matched)
	{
		GRegex *item_regex;
		GMatchInfo *item_match;
		char *body = g_match_info_fetch (array_match, 1);

		item_regex = g_regex_new ("\"((?:\\\\.|[^\"\\\\])*)\"", G_REGEX_DOTALL, 0, error);
		if (!item_regex)
		{
			g_free (body);
			g_match_info_free (array_match);
			g_regex_unref (array_regex);
			return FALSE;
		}

		g_regex_match (item_regex, body, 0, &item_match);
		while (g_match_info_matches (item_match))
		{
			char *raw = g_match_info_fetch (item_match, 1);
			char *item = g_strcompress (raw);
			g_free (raw);
			g_ptr_array_add (values, item);
			g_match_info_next (item_match, NULL);
		}

		g_match_info_free (item_match);
		g_regex_unref (item_regex);
		g_free (body);
	}

	g_match_info_free (array_match);
	g_regex_unref (array_regex);
	return success;
}

static FabulorPluginManifest *
fabulor_plugin_manifest_load_from_path (const char *manifest_path, GError **error)
{
	FabulorPluginManifest *manifest;
	char *json;
	gsize json_length;

	if (!g_file_get_contents (manifest_path, &json, &json_length, error))
	{
		return NULL;
	}

	if (json_length == 0)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Empty plugin manifest: %s", manifest_path);
		g_free (json);
		return NULL;
	}

	manifest = fabulor_plugin_manifest_new ();
	manifest->manifest_path = g_strdup (manifest_path);
	manifest->plugin_directory = g_path_get_dirname (manifest_path);

	if (!extract_string_field (json, "id", &manifest->id, error)
		|| !extract_string_field (json, "name", &manifest->name, error)
		|| !extract_string_field (json, "version", &manifest->version, error)
		|| !extract_string_field (json, "language", &manifest->language_name, error)
		|| !extract_string_field (json, "entrypoint", &manifest->entrypoint, error)
		|| !extract_uint_field (json, "requires_api_version", &manifest->requires_api_version, error)
		|| !extract_string_array_field (json, "dependencies", manifest->dependencies, error)
		|| !extract_string_array_field (json, "capabilities", manifest->capabilities, error)
		|| !extract_string_field (json, "description", &manifest->description, error)
		|| !extract_string_field (json, "author", &manifest->author, error)
		|| !extract_string_field (json, "homepage", &manifest->homepage, error))
	{
		fabulor_plugin_manifest_free (manifest);
		g_free (json);
		return NULL;
	}

	manifest->language = fabulor_plugin_language_from_string (manifest->language_name);
	if (manifest->entrypoint && manifest->plugin_directory)
	{
		manifest->entrypoint_path = g_build_filename (manifest->plugin_directory, manifest->entrypoint, NULL);
	}

	g_free (json);
	return manifest;
}

static gboolean
manifest_field_missing (const char *value)
{
	return value == NULL || *value == '\0';
}

static gboolean
discover_manifests_in_root (FabulorPluginCatalog *catalog, const char *plugins_root, GError **error)
{
	GDir *directory;
	const char *entry_name;

	directory = g_dir_open (plugins_root, 0, error);
	if (!directory)
	{
		return FALSE;
	}

	while ((entry_name = g_dir_read_name (directory)) != NULL)
	{
		char *plugin_dir;
		char *manifest_path;

		if (entry_name[0] == '.')
		{
			continue;
		}

		plugin_dir = g_build_filename (plugins_root, entry_name, NULL);
		manifest_path = g_build_filename (plugin_dir, "plugin.json", NULL);

		if (g_file_test (plugin_dir, G_FILE_TEST_IS_DIR) && g_file_test (manifest_path, G_FILE_TEST_EXISTS))
		{
			FabulorPluginManifest *manifest;
			GError *manifest_error = NULL;

			manifest = fabulor_plugin_manifest_load_from_path (manifest_path, &manifest_error);
			if (!manifest)
			{
				fabulor_plugin_catalog_add_diagnostic (catalog, "Skipping invalid plugin manifest %s: %s",
													 manifest_path,
													 manifest_error ? manifest_error->message : "unknown error");
				g_clear_error (&manifest_error);
			}
			else
			{
				if (manifest->id && g_hash_table_contains (catalog->manifest_index, manifest->id))
				{
					fabulor_plugin_catalog_add_diagnostic (catalog, "Skipping duplicate plugin id '%s' from %s.", manifest->id, manifest_path);
					fabulor_plugin_manifest_free (manifest);
				}
				else
				{
					g_ptr_array_add (catalog->manifests, manifest);
					if (manifest->id)
					{
						g_hash_table_insert (catalog->manifest_index, manifest->id, manifest);
					}
				}
			}
		}

		g_free (manifest_path);
		g_free (plugin_dir);
	}

	g_dir_close (directory);
	return TRUE;
}

static gboolean
fabulor_plugin_manifest_is_enabled (const FabulorPluginCatalog *catalog, const FabulorPluginManifest *manifest)
{
	if (catalog->safe_mode_enabled)
	{
		return FALSE;
	}

	return !g_hash_table_contains (catalog->blacklist, manifest->id);
}

static gboolean
ordered_manifest_contains (GPtrArray *ordered, const FabulorPluginManifest *manifest)
{
	guint i;

	for (i = 0; i < ordered->len; i++)
	{
		if (g_ptr_array_index (ordered, i) == manifest)
		{
			return TRUE;
		}
	}

	return FALSE;
}

static void
fabulor_plugin_manifest_add_validation (FabulorPluginCatalog *catalog,
										const FabulorPluginManifest *manifest,
										const char *format,
										...)
{
	char *message;
	char *prefix;
	va_list args;

	va_start (args, format);
	message = g_strdup_vprintf (format, args);
	va_end (args);

	prefix = manifest && manifest->manifest_path
		? g_strdup_printf ("%s: %s", manifest->manifest_path, message)
		: g_strdup (message);

	g_free (message);
	g_ptr_array_add (catalog->diagnostics, prefix);
	fabulor_api_log (catalog->api, "%s", prefix);
}

static gboolean
fabulor_plugin_manifest_validate (FabulorPluginCatalog *catalog,
								   const FabulorPluginManifest *manifest,
								   guint api_version)
{
	gboolean valid = TRUE;
	guint i;

	if (manifest_field_missing (manifest->id))
	{
		fabulor_plugin_manifest_add_validation (catalog, manifest, "Missing required field 'id'.");
		valid = FALSE;
	}

	if (manifest_field_missing (manifest->name))
	{
		fabulor_plugin_manifest_add_validation (catalog, manifest, "Missing required field 'name'.");
		valid = FALSE;
	}

	if (manifest_field_missing (manifest->version))
	{
		fabulor_plugin_manifest_add_validation (catalog, manifest, "Missing required field 'version'.");
		valid = FALSE;
	}

	if (manifest->language == FABULOR_PLUGIN_LANGUAGE_UNKNOWN)
	{
		fabulor_plugin_manifest_add_validation (catalog, manifest, "Unsupported language '%s'.", manifest->language_name ? manifest->language_name : "");
		valid = FALSE;
	}

	if (manifest_field_missing (manifest->entrypoint))
	{
		fabulor_plugin_manifest_add_validation (catalog, manifest, "Missing required field 'entrypoint'.");
		valid = FALSE;
	}
	else if (!manifest->entrypoint_path || !g_file_test (manifest->entrypoint_path, G_FILE_TEST_EXISTS))
	{
		fabulor_plugin_manifest_add_validation (catalog, manifest, "Entrypoint '%s' was not found.", manifest->entrypoint);
		valid = FALSE;
	}

	if (manifest->requires_api_version > api_version)
	{
		fabulor_plugin_manifest_add_validation (catalog, manifest, "Plugin requires API version %u but the host only exposes version %u.",
												manifest->requires_api_version,
												api_version);
		valid = FALSE;
	}

	if (manifest_field_missing (manifest->description))
	{
		fabulor_plugin_manifest_add_validation (catalog, manifest, "Missing required field 'description'.");
		valid = FALSE;
	}

	if (manifest_field_missing (manifest->author))
	{
		fabulor_plugin_manifest_add_validation (catalog, manifest, "Missing required field 'author'.");
		valid = FALSE;
	}

	if (manifest_field_missing (manifest->homepage))
	{
		fabulor_plugin_manifest_add_validation (catalog, manifest, "Missing required field 'homepage'.");
		valid = FALSE;
	}

	for (i = 0; i < manifest->dependencies->len; i++)
	{
		const char *dependency = g_ptr_array_index (manifest->dependencies, i);
		if (!g_hash_table_contains (catalog->manifest_index, dependency))
		{
			fabulor_plugin_manifest_add_validation (catalog, manifest, "Declared dependency '%s' was not discovered.", dependency);
			valid = FALSE;
		}
	}

	return valid;
}

static gboolean
loader_stub_load (const FabulorPluginManifest *manifest, const FabulorAPI *api, void *user_data, GError **error)
{
	(void)user_data;
	(void)error;
	fabulor_api_log (api, "Plugin loader scaffold accepted %s (%s).", manifest->id, fabulor_plugin_language_to_string (manifest->language));
	return TRUE;
}

static gboolean
loader_stub_dispatch (const FabulorPluginManifest *manifest,
					  const char *handler_name,
					  const char *event_name,
					  const char *event_payload_json,
					  void *user_data,
					  GError **error)
{
	const FabulorAPI *api = (const FabulorAPI *) user_data;
	(void)error;
	fabulor_api_log (api, "Dispatch scaffold for %s handler '%s' on event '%s' with payload %s.",
					 manifest->id,
					 handler_name,
					 event_name,
					 event_payload_json ? event_payload_json : "{}");
	return TRUE;
}

static const char *
manifest_plugin_get_libdir (void)
{
	const char *libdir;

	libdir = g_getenv ("ZOITECHAT_LIBDIR");
	if (libdir && *libdir)
	{
		return libdir;
	}

	return ZOITECHATLIBDIR;
}

static gboolean
ensure_python_runtime_loaded (session *sess, GError **error)
{
	char *runtime_path;
	char *load_error;
	gboolean success;

	runtime_path = g_build_filename (manifest_plugin_get_libdir (), "hcpython3.dll", NULL);
	load_error = plugin_load (sess, runtime_path, NULL);
	success = load_error == NULL;

	if (!success)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, "Python runtime load failed: %s", load_error);
	}

	g_free (runtime_path);
	return success;
}

static gboolean
load_python_manifest (const FabulorPluginManifest *manifest,
					  const FabulorAPI *api,
					  void *user_data,
					  GError **error)
{
	session *sess = (session *) user_data;
	char *command;

	if (!sess)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Python plugin loading requires a valid session context.");
		return FALSE;
	}

	if (!ensure_python_runtime_loaded (sess, error))
	{
		return FALSE;
	}

	command = g_strdup_printf ("LOAD \"%s\"", manifest->entrypoint_path);
	handle_command (sess, command, FALSE);
	g_free (command);

	fabulor_api_log (api, "Requested Python manifest load for %s from %s.", manifest->id, manifest->entrypoint_path);
	return TRUE;
}

static gboolean
load_tcl_manifest (const FabulorPluginManifest *manifest,
				   const FabulorAPI *api,
				   void *user_data,
				   GError **error)
{
	(void)user_data;
	(void)api;
	g_set_error (error,
				 G_FILE_ERROR,
				 G_FILE_ERROR_NOSYS,
				 "Tcl manifest loading is not wired into the runtime yet for %s.",
				 manifest->id);
	return FALSE;
}

static gboolean
load_csharp_manifest (const FabulorPluginManifest *manifest,
					  const FabulorAPI *api,
					  void *user_data,
					  GError **error)
{
	(void)user_data;
	(void)api;
	g_set_error (error,
				 G_FILE_ERROR,
				 G_FILE_ERROR_NOSYS,
				 "C# manifest loading is not wired into the runtime yet for %s.",
				 manifest->id);
	return FALSE;
}

static const FabulorPluginLoader csharp_loader =
{
	FABULOR_PLUGIN_LANGUAGE_CSHARP,
	"csharp",
	load_csharp_manifest,
	loader_stub_dispatch
};

static const FabulorPluginLoader python_loader =
{
	FABULOR_PLUGIN_LANGUAGE_PYTHON,
	"python",
	load_python_manifest,
	loader_stub_dispatch
};

static const FabulorPluginLoader tcl_loader =
{
	FABULOR_PLUGIN_LANGUAGE_TCL,
	"tcl",
	load_tcl_manifest,
	loader_stub_dispatch
};

static void
fabulor_callback_entry_free (FabulorCallbackEntry *entry)
{
	if (!entry)
	{
		return;
	}

	g_free (entry->plugin_id);
	g_free (entry->handler_name);
	g_free (entry);
}

static void
fabulor_callback_entry_array_free (GPtrArray *entries)
{
	if (!entries)
	{
		return;
	}

	g_ptr_array_free (entries, TRUE);
}

static void
fabulor_deferred_dispatch_free (FabulorDeferredDispatch *dispatch)
{
	if (!dispatch)
	{
		return;
	}

	g_free (dispatch->event_name);
	g_free (dispatch->event_payload_json);
	g_free (dispatch);
}

FabulorPluginManifest *
fabulor_plugin_manifest_new (void)
{
	FabulorPluginManifest *manifest = g_new0 (FabulorPluginManifest, 1);
	manifest->dependencies = g_ptr_array_new_with_free_func (g_free);
	manifest->capabilities = g_ptr_array_new_with_free_func (g_free);
	return manifest;
}

void
fabulor_plugin_manifest_free (FabulorPluginManifest *manifest)
{
	if (!manifest)
	{
		return;
	}

	g_free (manifest->id);
	g_free (manifest->name);
	g_free (manifest->version);
	g_free (manifest->language_name);
	g_free (manifest->entrypoint);
	g_ptr_array_free (manifest->dependencies, TRUE);
	g_ptr_array_free (manifest->capabilities, TRUE);
	g_free (manifest->description);
	g_free (manifest->author);
	g_free (manifest->homepage);
	g_free (manifest->plugin_directory);
	g_free (manifest->manifest_path);
	g_free (manifest->entrypoint_path);
	g_free (manifest);
}

FabulorPluginCatalog *
fabulor_plugin_catalog_new (const FabulorAPI *api)
{
	FabulorPluginCatalog *catalog = g_new0 (FabulorPluginCatalog, 1);
	catalog->api = api;
	catalog->manifests = g_ptr_array_new_with_free_func ((GDestroyNotify) fabulor_plugin_manifest_free);
	catalog->manifest_index = g_hash_table_new (g_str_hash, g_str_equal);
	catalog->blacklist = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	catalog->diagnostics = g_ptr_array_new_with_free_func (g_free);
	return catalog;
}

void
fabulor_plugin_catalog_clear (FabulorPluginCatalog *catalog)
{
	if (!catalog)
	{
		return;
	}

	g_hash_table_remove_all (catalog->manifest_index);
	g_ptr_array_set_size (catalog->manifests, 0);
	g_ptr_array_set_size (catalog->diagnostics, 0);
}

void
fabulor_plugin_catalog_free (FabulorPluginCatalog *catalog)
{
	if (!catalog)
	{
		return;
	}

	g_ptr_array_free (catalog->manifests, TRUE);
	g_hash_table_unref (catalog->manifest_index);
	g_hash_table_unref (catalog->blacklist);
	g_ptr_array_free (catalog->diagnostics, TRUE);
	g_free (catalog);
}

void
fabulor_plugin_catalog_set_safe_mode (FabulorPluginCatalog *catalog, gboolean enabled)
{
	if (catalog)
	{
		catalog->safe_mode_enabled = enabled;
	}
}

void
fabulor_plugin_catalog_blacklist_plugin (FabulorPluginCatalog *catalog, const char *plugin_id)
{
	if (!catalog || !plugin_id || *plugin_id == '\0')
	{
		return;
	}

	g_hash_table_add (catalog->blacklist, g_strdup (plugin_id));
}

gboolean
fabulor_plugin_catalog_discover (FabulorPluginCatalog *catalog, const char *plugins_root, GError **error)
{
	if (!catalog || !plugins_root)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Plugin discovery requires a valid catalog and root path.");
		return FALSE;
	}

	fabulor_plugin_catalog_clear (catalog);
	return discover_manifests_in_root (catalog, plugins_root, error);
}

gboolean
fabulor_plugin_catalog_discover_root (FabulorPluginCatalog *catalog, const char *plugins_root, GError **error)
{
	if (!catalog || !plugins_root)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Plugin discovery requires a valid catalog and root path.");
		return FALSE;
	}

	return discover_manifests_in_root (catalog, plugins_root, error);
}

GPtrArray *
fabulor_plugin_catalog_resolve_load_order (FabulorPluginCatalog *catalog, guint api_version, GError **error)
{
	GPtrArray *ordered;
	GPtrArray *enabled_manifests;
	GHashTable *in_degrees;
	GHashTable *dependents;
	GQueue queue = G_QUEUE_INIT;
	GHashTableIter iter;
	gpointer key;
	gpointer value;
	guint i;
	gboolean has_validation_errors = FALSE;

	if (!catalog)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Plugin load-order resolution requires a valid catalog.");
		return NULL;
	}

	g_ptr_array_set_size (catalog->diagnostics, 0);

	enabled_manifests = g_ptr_array_new ();
	for (i = 0; i < catalog->manifests->len; i++)
	{
		FabulorPluginManifest *manifest = g_ptr_array_index (catalog->manifests, i);

		if (!fabulor_plugin_manifest_is_enabled (catalog, manifest))
		{
			if (catalog->safe_mode_enabled)
			{
				fabulor_plugin_manifest_add_validation (catalog, manifest, "Skipped because safe mode is enabled.");
			}
			else
			{
				fabulor_plugin_manifest_add_validation (catalog, manifest, "Skipped because the plugin is blacklisted.");
			}
			continue;
		}

		if (!fabulor_plugin_manifest_validate (catalog, manifest, api_version))
		{
			has_validation_errors = TRUE;
			continue;
		}

		g_ptr_array_add (enabled_manifests, manifest);
	}

	if (has_validation_errors)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Plugin validation failed. Review diagnostics for details.");
		g_ptr_array_free (enabled_manifests, TRUE);
		return NULL;
	}

	ordered = g_ptr_array_new ();
	in_degrees = g_hash_table_new (g_str_hash, g_str_equal);
	dependents = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_ptr_array_unref);

	for (i = 0; i < enabled_manifests->len; i++)
	{
		FabulorPluginManifest *manifest = g_ptr_array_index (enabled_manifests, i);
		g_hash_table_insert (in_degrees, manifest->id, GINT_TO_POINTER ((gint) manifest->dependencies->len));
	}

	for (i = 0; i < enabled_manifests->len; i++)
	{
		FabulorPluginManifest *manifest = g_ptr_array_index (enabled_manifests, i);
		guint dependency_index;

		for (dependency_index = 0; dependency_index < manifest->dependencies->len; dependency_index++)
		{
			const char *dependency = g_ptr_array_index (manifest->dependencies, dependency_index);
			GPtrArray *downstream = g_hash_table_lookup (dependents, dependency);

			if (!downstream)
			{
				downstream = g_ptr_array_new ();
				g_hash_table_insert (dependents, g_strdup (dependency), downstream);
			}

			g_ptr_array_add (downstream, manifest);
		}
	}

	g_hash_table_iter_init (&iter, in_degrees);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		if (GPOINTER_TO_INT (value) == 0)
		{
			g_queue_push_tail (&queue, g_hash_table_lookup (catalog->manifest_index, key));
		}
	}

	while (!g_queue_is_empty (&queue))
	{
		FabulorPluginManifest *manifest = g_queue_pop_head (&queue);
		GPtrArray *downstream = g_hash_table_lookup (dependents, manifest->id);

		g_ptr_array_add (ordered, manifest);

		if (!downstream)
		{
			continue;
		}

		for (i = 0; i < downstream->len; i++)
		{
			FabulorPluginManifest *dependent = g_ptr_array_index (downstream, i);
			gint new_degree = GPOINTER_TO_INT (g_hash_table_lookup (in_degrees, dependent->id)) - 1;

			g_hash_table_replace (in_degrees, dependent->id, GINT_TO_POINTER (new_degree));
			if (new_degree == 0)
			{
				g_queue_push_tail (&queue, dependent);
			}
		}
	}

	if (ordered->len != enabled_manifests->len)
	{
		GString *cycle_list = g_string_new ("");

		for (i = 0; i < enabled_manifests->len; i++)
		{
			FabulorPluginManifest *manifest = g_ptr_array_index (enabled_manifests, i);
			if (!ordered_manifest_contains (ordered, manifest))
			{
				if (cycle_list->len > 0)
				{
					g_string_append (cycle_list, ", ");
				}

				g_string_append (cycle_list, manifest->id);
			}
		}

		fabulor_plugin_catalog_add_diagnostic (catalog, "Dependency cycle detected across: %s.", cycle_list->str);
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Dependency cycle detected across: %s.", cycle_list->str);
		g_string_free (cycle_list, TRUE);
		g_ptr_array_free (ordered, TRUE);
		ordered = NULL;
	}

	g_hash_table_unref (dependents);
	g_hash_table_unref (in_degrees);
	g_ptr_array_free (enabled_manifests, TRUE);
	return ordered;
}

gboolean
fabulor_plugin_catalog_load (FabulorPluginCatalog *catalog, GPtrArray *ordered_manifests, void *loader_user_data, GError **error)
{
	guint i;

	if (!catalog || !ordered_manifests)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Plugin loading requires a valid catalog and load order.");
		return FALSE;
	}

	for (i = 0; i < ordered_manifests->len; i++)
	{
		const FabulorPluginManifest *manifest = g_ptr_array_index (ordered_manifests, i);
		const FabulorPluginLoader *loader = fabulor_plugin_loader_for_language (manifest->language);

		if (!loader)
		{
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "No plugin loader is registered for language '%s'.",
						 fabulor_plugin_language_to_string (manifest->language));
			return FALSE;
		}

		if (!loader->load (manifest, catalog->api, loader_user_data, error))
		{
			return FALSE;
		}
	}

	return TRUE;
}

const FabulorPluginManifest *
fabulor_plugin_catalog_find_manifest (const FabulorPluginCatalog *catalog, const char *plugin_id)
{
	if (!catalog || !plugin_id)
	{
		return NULL;
	}

	return g_hash_table_lookup (catalog->manifest_index, plugin_id);
}

const GPtrArray *
fabulor_plugin_catalog_get_manifests (const FabulorPluginCatalog *catalog)
{
	return catalog ? catalog->manifests : NULL;
}

const GPtrArray *
fabulor_plugin_catalog_get_diagnostics (const FabulorPluginCatalog *catalog)
{
	return catalog ? catalog->diagnostics : NULL;
}

const FabulorPluginLoader *
fabulor_plugin_loader_for_language (FabulorPluginLanguage language)
{
	switch (language)
	{
	case FABULOR_PLUGIN_LANGUAGE_CSHARP:
		return &csharp_loader;
	case FABULOR_PLUGIN_LANGUAGE_PYTHON:
		return &python_loader;
	case FABULOR_PLUGIN_LANGUAGE_TCL:
		return &tcl_loader;
	default:
		return NULL;
	}
}

const char *
fabulor_plugin_language_to_string (FabulorPluginLanguage language)
{
	switch (language)
	{
	case FABULOR_PLUGIN_LANGUAGE_CSHARP:
		return "csharp";
	case FABULOR_PLUGIN_LANGUAGE_PYTHON:
		return "python";
	case FABULOR_PLUGIN_LANGUAGE_TCL:
		return "tcl";
	default:
		return "unknown";
	}
}

FabulorPluginLanguage
fabulor_plugin_language_from_string (const char *language_name)
{
	if (!language_name)
	{
		return FABULOR_PLUGIN_LANGUAGE_UNKNOWN;
	}

	if (g_ascii_strcasecmp (language_name, "csharp") == 0)
	{
		return FABULOR_PLUGIN_LANGUAGE_CSHARP;
	}

	if (g_ascii_strcasecmp (language_name, "python") == 0)
	{
		return FABULOR_PLUGIN_LANGUAGE_PYTHON;
	}

	if (g_ascii_strcasecmp (language_name, "tcl") == 0)
	{
		return FABULOR_PLUGIN_LANGUAGE_TCL;
	}

	return FABULOR_PLUGIN_LANGUAGE_UNKNOWN;
}

FabulorCallbackRegistry *
fabulor_callback_registry_new (const FabulorAPI *api, const FabulorPluginCatalog *catalog, GMainContext *main_context)
{
	FabulorCallbackRegistry *registry = g_new0 (FabulorCallbackRegistry, 1);
	registry->api = api;
	registry->catalog = catalog;
	registry->entries = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) fabulor_callback_entry_array_free);
	registry->main_context = main_context ? g_main_context_ref (main_context) : g_main_context_ref (g_main_context_default ());
	registry->main_thread = g_thread_self ();
	return registry;
}

void
fabulor_callback_registry_free (FabulorCallbackRegistry *registry)
{
	if (!registry)
	{
		return;
	}

	g_hash_table_unref (registry->entries);
	g_main_context_unref (registry->main_context);
	g_free (registry);
}

gboolean
fabulor_callback_registry_register (FabulorCallbackRegistry *registry,
									 const char *event_name,
									 const char *plugin_id,
									 FabulorPluginLanguage language,
									 const char *handler_name,
									 GError **error)
{
	FabulorCallbackEntry *entry;
	GPtrArray *entries;
	const FabulorPluginManifest *manifest;

	if (!registry || !event_name || !plugin_id || !handler_name)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Callback registration requires an event name, plugin id, and handler name.");
		return FALSE;
	}

	manifest = fabulor_plugin_catalog_find_manifest (registry->catalog, plugin_id);
	if (!manifest)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "Cannot register callback for unknown plugin '%s'.", plugin_id);
		return FALSE;
	}

	if (manifest->language != language)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Plugin '%s' was registered as %s, not %s.",
					 plugin_id,
					 fabulor_plugin_language_to_string (manifest->language),
					 fabulor_plugin_language_to_string (language));
		return FALSE;
	}

	entries = g_hash_table_lookup (registry->entries, event_name);
	if (!entries)
	{
		entries = g_ptr_array_new_with_free_func ((GDestroyNotify) fabulor_callback_entry_free);
		g_hash_table_insert (registry->entries, g_strdup (event_name), entries);
	}

	entry = g_new0 (FabulorCallbackEntry, 1);
	entry->plugin_id = g_strdup (plugin_id);
	entry->language = language;
	entry->handler_name = g_strdup (handler_name);
	g_ptr_array_add (entries, entry);
	return TRUE;
}

static gboolean
fabulor_callback_registry_dispatch_now (FabulorCallbackRegistry *registry,
										const char *event_name,
										const char *event_payload_json,
										void *loader_user_data,
										GError **error)
{
	GPtrArray *entries;
	guint i;

	entries = g_hash_table_lookup (registry->entries, event_name);
	if (!entries)
	{
		return TRUE;
	}

	for (i = 0; i < entries->len; i++)
	{
		FabulorCallbackEntry *entry = g_ptr_array_index (entries, i);
		const FabulorPluginManifest *manifest = fabulor_plugin_catalog_find_manifest (registry->catalog, entry->plugin_id);
		const FabulorPluginLoader *loader = fabulor_plugin_loader_for_language (entry->language);
		GError *dispatch_error = NULL;

		if (!manifest || !loader)
		{
			continue;
		}

		if (!loader->dispatch_callback (manifest,
										 entry->handler_name,
										 event_name,
										 event_payload_json,
										 loader_user_data ? loader_user_data : (void *) registry->api,
										 &dispatch_error))
		{
			fabulor_api_log (registry->api, "Callback dispatch failed for %s/%s: %s",
							 entry->plugin_id,
							 entry->handler_name,
							 dispatch_error ? dispatch_error->message : "unknown error");
			g_clear_error (&dispatch_error);
		}
	}

	return TRUE;
}

static gboolean
fabulor_callback_registry_invoke_main_thread (gpointer user_data)
{
	FabulorDeferredDispatch *dispatch = user_data;
	fabulor_callback_registry_dispatch_now (dispatch->registry,
										 dispatch->event_name,
										 dispatch->event_payload_json,
										 dispatch->loader_user_data,
										 NULL);
	fabulor_deferred_dispatch_free (dispatch);
	return G_SOURCE_REMOVE;
}

gboolean
fabulor_callback_registry_fire_event (FabulorCallbackRegistry *registry,
									  const char *event_name,
									  const char *event_payload_json,
									  void *loader_user_data,
									  GError **error)
{
	if (!registry || !event_name)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Event dispatch requires a registry and event name.");
		return FALSE;
	}

	if (g_thread_self () == registry->main_thread)
	{
		return fabulor_callback_registry_dispatch_now (registry,
													 event_name,
													 event_payload_json ? event_payload_json : "{}",
													 loader_user_data,
													 error);
	}

	{
		FabulorDeferredDispatch *dispatch = g_new0 (FabulorDeferredDispatch, 1);
		dispatch->registry = registry;
		dispatch->event_name = g_strdup (event_name);
		dispatch->event_payload_json = g_strdup (event_payload_json ? event_payload_json : "{}");
		dispatch->loader_user_data = loader_user_data;
		g_main_context_invoke_full (registry->main_context,
									G_PRIORITY_DEFAULT,
									fabulor_callback_registry_invoke_main_thread,
									dispatch,
									NULL);
	}

	return TRUE;
}
