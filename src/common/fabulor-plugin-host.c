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
#include <gio/gio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#include <tcl.h>
#endif

#include "fabulor.h"
#include "cfgfiles.h"
#include "plugin.h"
#include "outbound.h"
#include "server.h"
#include "util.h"
#include "fabulorc.h"
#include "fabulor-plugin-host.h"
#include "fabulor-plugin-manifest-json.h"
#include "fabulor-plugin-path-policy.h"

typedef struct
{
	FabulorCallbackRegistry *registry;
	char *event_name;
	char *event_payload_json;
} FabulorDeferredDispatch;

#define FABULOR_CALLBACK_EVENT_NAME_MAX 128U
#define FABULOR_CALLBACK_HANDLER_NAME_MAX 256U
#define FABULOR_CALLBACKS_PER_PLUGIN_MAX 64U
#define FABULOR_CALLBACKS_PER_EVENT_MAX 256U
#define FABULOR_CALLBACK_QUEUED_DISPATCH_MAX 256U
#define FABULOR_CALLBACK_PAYLOAD_MAX (1024U * 1024U)

struct _fabulor_plugin_catalog
{
	const FabulorAPI *api;
	GPtrArray *manifests;
	GHashTable *manifest_index;
	GHashTable *blacklist;
	GPtrArray *diagnostics;
	guint discovery_diagnostic_count;
	gboolean safe_mode_enabled;
};

struct _fabulor_callback_registry
{
	const FabulorAPI *api;
	const FabulorPluginCatalog *catalog;
	GHashTable *entries;
	GMainContext *main_context;
	GThread *main_thread;
	GMutex mutex;
	volatile gint ref_count;
	gboolean shutting_down;
	guint queued_dispatches;
};

#ifdef WIN32
typedef struct
{
	char *plugin_id;
	Tcl_Interp *interp;
	const FabulorAPI *api;
	GHashTable *capabilities;
	gboolean simple_addon;
} FabulorTclPluginState;

typedef struct
{
	FabulorTclPluginState *state;
	char *handler_name;
} FabulorTclCommandEntry;

typedef struct
{
	gboolean attempted_initialisation;
	HMODULE module;
	char *runtime_root;
	char *library_path;
	GHashTable *plugins;
	GHashTable *commands;
	Tcl_Interp *(*create_interp) (void);
	void (*delete_interp) (Tcl_Interp *interp);
	void (*find_executable) (const char *argv0);
	int (*init_interp) (Tcl_Interp *interp);
	int (*eval_file) (Tcl_Interp *interp, const char *file_name);
	int (*eval_ex) (Tcl_Interp *interp, const char *script, int length, int flags);
	char *(*get_string_result) (Tcl_Interp *interp);
	const char *(*set_var) (Tcl_Interp *interp, const char *var_name, const char *new_value, int flags);
	Tcl_Command (*create_command) (Tcl_Interp *interp,
								   const char *cmd_name,
								   Tcl_CmdProc *proc,
								   ClientData client_data,
								   Tcl_CmdDeleteProc *delete_proc);
	char *(*merge_args) (int argc, const char *const *argv);
	void (*free_value) (char *ptr);
	void (*set_result) (Tcl_Interp *interp, char *result, Tcl_FreeProc *free_proc);
} FabulorTclRuntime;

#define FABULOR_HOSTFXR_CALLTYPE __cdecl
#define FABULOR_CORECLR_DELEGATE_CALLTYPE __stdcall

typedef void *fabulor_hostfxr_handle;

typedef enum
{
	FABULOR_HDT_COM_ACTIVATION = 0,
	FABULOR_HDT_LOAD_IN_MEMORY_ASSEMBLY = 1,
	FABULOR_HDT_WINRT_ACTIVATION = 2,
	FABULOR_HDT_COM_REGISTER = 3,
	FABULOR_HDT_COM_UNREGISTER = 4,
	FABULOR_HDT_LOAD_ASSEMBLY_AND_GET_FUNCTION_POINTER = 5
} FabulorHostfxrDelegateType;

typedef int32_t (FABULOR_HOSTFXR_CALLTYPE *FabulorHostfxrInitializeForRuntimeConfigFn) (const wchar_t *runtime_config_path,
																						  const void *parameters,
																						  fabulor_hostfxr_handle *host_context_handle);
typedef int32_t (FABULOR_HOSTFXR_CALLTYPE *FabulorHostfxrGetRuntimeDelegateFn) (const fabulor_hostfxr_handle host_context_handle,
																			 FabulorHostfxrDelegateType type,
																			 void **delegate_handle);
typedef int32_t (FABULOR_HOSTFXR_CALLTYPE *FabulorHostfxrCloseFn) (const fabulor_hostfxr_handle host_context_handle);
typedef int (FABULOR_CORECLR_DELEGATE_CALLTYPE *FabulorLoadAssemblyAndGetFunctionPointerFn) (const wchar_t *assembly_path,
																							   const wchar_t *type_name,
																							   const wchar_t *method_name,
																							   const wchar_t *delegate_type_name,
																							   void *reserved,
																							   void **delegate_handle);
typedef int (FABULOR_CORECLR_DELEGATE_CALLTYPE *FabulorComponentEntryPointFn) (void *arg, int32_t arg_size_in_bytes);

typedef void (FABULOR_CORECLR_DELEGATE_CALLTYPE *FabulorManagedLogFn) (const char *text);
typedef int (FABULOR_CORECLR_DELEGATE_CALLTYPE *FabulorManagedSendMessageFn) (const char *plugin_id, const char *target, const char *text);
typedef unsigned int (FABULOR_CORECLR_DELEGATE_CALLTYPE *FabulorManagedGetUserCountFn) (const char *plugin_id);
typedef int (FABULOR_CORECLR_DELEGATE_CALLTYPE *FabulorManagedGetUserInfoFn) (const char *plugin_id, FabulorUserInfo *user_info);
typedef int (FABULOR_CORECLR_DELEGATE_CALLTYPE *FabulorManagedRegisterCallbackFn) (const char *plugin_id, const char *event_name, const char *handler_name);

typedef struct
{
	FabulorManagedLogFn log;
	FabulorManagedSendMessageFn send_message;
	FabulorManagedGetUserCountFn get_user_count;
	FabulorManagedGetUserInfoFn get_user_info;
	FabulorManagedRegisterCallbackFn register_callback;
} FabulorManagedHostApi;

typedef struct
{
	const char *plugin_id;
	const char *assembly_path;
} FabulorManagedLoadPluginArgs;

typedef struct
{
	const char *plugin_id;
	const char *handler_name;
	const char *event_name;
	const char *payload_json;
} FabulorManagedDispatchArgs;

typedef struct
{
	HMODULE hostfxr_module;
	char *dotnet_root;
	char *bridge_root;
	char *bridge_assembly_path;
	char *bridge_runtime_config_path;
	gboolean initialised;
	FabulorHostfxrInitializeForRuntimeConfigFn initialize_for_runtime_config;
	FabulorHostfxrGetRuntimeDelegateFn get_runtime_delegate;
	FabulorHostfxrCloseFn close_host_context;
	FabulorLoadAssemblyAndGetFunctionPointerFn load_assembly_and_get_function_pointer;
	FabulorComponentEntryPointFn initialize_bridge;
	FabulorComponentEntryPointFn load_plugin;
	FabulorComponentEntryPointFn dispatch_callback;
	FabulorComponentEntryPointFn shutdown_bridge;
} FabulorCSharpRuntime;

static FabulorTclRuntime fabulor_tcl_runtime;
static FabulorCallbackRegistry *fabulor_active_callback_registry;
G_LOCK_DEFINE_STATIC (fabulor_active_callback_registry_lock);
static const FabulorAPI *fabulor_active_api;
static FabulorCSharpRuntime fabulor_csharp_runtime;
static GHashTable *fabulor_simple_csharp_manifests;
static char *fabulor_python_manifest_token;
#endif

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
fabulor_capability_is_known (const char *capability)
{
	static const char *known[] = {
		"events.command", "events.message", "events.print", "events.server",
		"messages.write", "session.read"
	};
	guint i;

	for (i = 0; i < G_N_ELEMENTS (known); i++)
	{
		if (g_strcmp0 (capability, known[i]) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

static gboolean
fabulor_manifest_has_capability (const FabulorPluginManifest *manifest, const char *capability)
{
	guint i;

	if (!manifest || !capability)
	{
		return FALSE;
	}

	for (i = 0; i < manifest->capabilities->len; i++)
	{
		if (g_strcmp0 (g_ptr_array_index (manifest->capabilities, i), capability) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

static const FabulorPluginManifest *
fabulor_runtime_find_manifest (const FabulorPluginCatalog *catalog, const char *plugin_id)
{
	const FabulorPluginManifest *manifest = NULL;

	if (catalog)
	{
		manifest = fabulor_plugin_catalog_find_manifest (catalog, plugin_id);
	}
#ifdef WIN32
	if (!manifest && fabulor_simple_csharp_manifests)
	{
		manifest = g_hash_table_lookup (fabulor_simple_csharp_manifests, plugin_id);
	}
#endif
	return manifest;
}

static const char *
fabulor_event_capability (const char *event_name)
{
	if (g_strcmp0 (event_name, "message") == 0)
		return "events.message";
	if (g_strcmp0 (event_name, "server") == 0 || g_str_has_prefix (event_name, "server:"))
		return "events.server";
	if (g_strcmp0 (event_name, "print") == 0 || g_str_has_prefix (event_name, "print:"))
		return "events.print";
	if (g_strcmp0 (event_name, "command") == 0 || g_str_has_prefix (event_name, "command:"))
		return "events.command";
	return NULL;
}

#ifdef WIN32
static gboolean
fabulor_active_callback_registry_has_capability (const char *plugin_id, const char *capability)
{
	const FabulorPluginManifest *manifest = NULL;
	gboolean allowed;

	G_LOCK (fabulor_active_callback_registry_lock);
	if (fabulor_active_callback_registry)
	{
		manifest = fabulor_runtime_find_manifest (fabulor_active_callback_registry->catalog, plugin_id);
	}
	allowed = fabulor_manifest_has_capability (manifest, capability);
	G_UNLOCK (fabulor_active_callback_registry_lock);
	return allowed;
}

static gboolean
fabulor_active_callback_registry_register (const char *event_name,
											 const char *plugin_id,
											 FabulorPluginLanguage language,
											 const char *handler_name,
											 GError **error)
{
	gboolean success;

	G_LOCK (fabulor_active_callback_registry_lock);
	if (!fabulor_active_callback_registry)
	{
		G_UNLOCK (fabulor_active_callback_registry_lock);
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, "Callback registry is not available.");
		return FALSE;
	}
	success = fabulor_callback_registry_register (fabulor_active_callback_registry,
													 event_name,
													 plugin_id,
													 language,
													 handler_name,
													 error);
	G_UNLOCK (fabulor_active_callback_registry_lock);
	return success;
}

static void
fabulor_active_callback_registry_remove_plugin (const char *plugin_id)
{
	G_LOCK (fabulor_active_callback_registry_lock);
	if (fabulor_active_callback_registry)
	{
		fabulor_callback_registry_remove_plugin (fabulor_active_callback_registry, plugin_id);
	}
	G_UNLOCK (fabulor_active_callback_registry_lock);
}
#endif

static char *
fabulor_plugin_manifest_read_bounded (const char *manifest_path, gsize *json_length, GError **error)
{
	GFile *file;
	GFileInfo *info;
	GFileInputStream *stream;
	GByteArray *contents;
	char buffer[4096];
	gssize bytes_read;

	*json_length = 0;
	file = g_file_new_for_path (manifest_path);
	info = g_file_query_info (file,
							   G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_STANDARD_SIZE,
							   G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
							   NULL,
							   error);
	if (!info)
	{
		g_object_unref (file);
		return NULL;
	}
	if (g_file_info_get_file_type (info) != G_FILE_TYPE_REGULAR)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Plugin manifest is not a regular file: %s", manifest_path);
		g_object_unref (info);
		g_object_unref (file);
		return NULL;
	}
	if (g_file_info_get_size (info) == 0
		|| g_file_info_get_size (info) > FABULOR_PLUGIN_MANIFEST_MAX_BYTES)
	{
		g_set_error (error,
					 G_FILE_ERROR,
					 G_FILE_ERROR_INVAL,
					 "Plugin manifest must be between 1 and %u bytes: %s",
					 FABULOR_PLUGIN_MANIFEST_MAX_BYTES,
					 manifest_path);
		g_object_unref (info);
		g_object_unref (file);
		return NULL;
	}
	g_object_unref (info);

	stream = g_file_read (file, NULL, error);
	g_object_unref (file);
	if (!stream)
	{
		return NULL;
	}

	contents = g_byte_array_sized_new (4096);
	while ((bytes_read = g_input_stream_read (G_INPUT_STREAM (stream), buffer, sizeof (buffer), NULL, error)) > 0)
	{
		if (contents->len + (guint) bytes_read > FABULOR_PLUGIN_MANIFEST_MAX_BYTES)
		{
			g_set_error (error,
						 G_FILE_ERROR,
						 G_FILE_ERROR_INVAL,
						 "Plugin manifest exceeds the %u-byte limit: %s",
						 FABULOR_PLUGIN_MANIFEST_MAX_BYTES,
						 manifest_path);
			g_byte_array_free (contents, TRUE);
			g_object_unref (stream);
			return NULL;
		}
		g_byte_array_append (contents, (const guint8 *) buffer, (guint) bytes_read);
	}
	g_object_unref (stream);
	if (bytes_read < 0)
	{
		g_byte_array_free (contents, TRUE);
		return NULL;
	}
	if (contents->len == 0)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Empty plugin manifest: %s", manifest_path);
		g_byte_array_free (contents, TRUE);
		return NULL;
	}

	*json_length = contents->len;
	g_byte_array_append (contents, (const guint8 *) "", 1);
	return (char *) g_byte_array_free (contents, FALSE);
}

static FabulorPluginManifest *
fabulor_plugin_manifest_load_from_path (const char *manifest_path, GError **error)
{
	FabulorPluginManifest *manifest;
	char *json;
	gsize json_length;

	json = fabulor_plugin_manifest_read_bounded (manifest_path, &json_length, error);
	if (!json)
	{
		return NULL;
	}

	manifest = fabulor_plugin_manifest_new ();
	manifest->manifest_path = g_strdup (manifest_path);
	manifest->plugin_directory = g_path_get_dirname (manifest_path);

	if (!fabulor_plugin_manifest_parse_json (json, json_length, manifest, error))
	{
		fabulor_plugin_manifest_free (manifest);
		g_free (json);
		return NULL;
	}

	manifest->language = fabulor_plugin_language_from_string (manifest->language_name);

	g_free (json);
	return manifest;
}

static gboolean
manifest_field_missing (const char *value)
{
	return value == NULL || *value == '\0';
}

static const char *
manifest_entrypoint_extension (FabulorPluginLanguage language)
{
	switch (language)
	{
	case FABULOR_PLUGIN_LANGUAGE_CSHARP:
		return ".dll";
	case FABULOR_PLUGIN_LANGUAGE_PYTHON:
		return ".py";
	case FABULOR_PLUGIN_LANGUAGE_TCL:
		return ".tcl";
	case FABULOR_PLUGIN_LANGUAGE_UNKNOWN:
	default:
		return NULL;
	}
}

static gboolean
manifest_refresh_entrypoint_path (FabulorPluginManifest *manifest, GError **error)
{
	const char *expected_extension;
	char *resolved = NULL;

	expected_extension = manifest_entrypoint_extension (manifest->language);
	if (!expected_extension)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
					 "Unsupported manifest language '%s'.",
					 manifest->language_name ? manifest->language_name : "");
		return FALSE;
	}

	if (!fabulor_plugin_path_resolve_entrypoint (manifest->plugin_directory,
												 manifest->entrypoint,
												 expected_extension,
												 &resolved,
												 error))
	{
		return FALSE;
	}

	g_free (manifest->entrypoint_path);
	manifest->entrypoint_path = resolved;
	return TRUE;
}

static gboolean
discover_manifests_in_root (FabulorPluginCatalog *catalog, const char *plugins_root, GError **error)
{
	GDir *directory;
	const char *entry_name;
	char *canonical_root;

	if (!fabulor_plugin_path_validate_root (plugins_root, &canonical_root, error))
	{
		return FALSE;
	}

	directory = g_dir_open (canonical_root, 0, error);
	if (!directory)
	{
		g_free (canonical_root);
		return FALSE;
	}

	while ((entry_name = g_dir_read_name (directory)) != NULL)
	{
		char *entry_path = NULL;
		char *plugin_dir = NULL;
		char *manifest_path = NULL;
		GFile *manifest_file;
		GError *path_error = NULL;

		if (entry_name[0] == '.')
		{
			continue;
		}

		entry_path = g_build_filename (canonical_root, entry_name, NULL);
		if (!fabulor_plugin_path_is_directory_candidate (entry_path))
		{
			g_free (entry_path);
			continue;
		}
		g_free (entry_path);

		if (!fabulor_plugin_path_resolve_child_directory (canonical_root,
												 entry_name,
												 &plugin_dir,
												 &path_error))
		{
			fabulor_plugin_catalog_add_diagnostic (catalog,
											 "Skipping untrusted plugin directory '%s' under %s: %s",
											 entry_name,
											 canonical_root,
											 path_error ? path_error->message : "unknown path error");
			g_clear_error (&path_error);
			continue;
		}

		manifest_file = g_file_new_build_filename (plugin_dir, "plugin.json", NULL);
		if (!g_file_query_exists (manifest_file, NULL))
		{
			g_object_unref (manifest_file);
			g_free (plugin_dir);
			continue;
		}
		g_object_unref (manifest_file);

		if (!fabulor_plugin_path_resolve_regular_file (plugin_dir,
											 "plugin.json",
											 &manifest_path,
											 &path_error))
		{
			fabulor_plugin_catalog_add_diagnostic (catalog,
											 "Skipping untrusted plugin manifest under %s: %s",
											 plugin_dir,
											 path_error ? path_error->message : "unknown path error");
			g_clear_error (&path_error);
			g_free (plugin_dir);
			continue;
		}

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
	g_free (canonical_root);
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
								   FabulorPluginManifest *manifest,
								   guint api_version)
{
	GError *entrypoint_error = NULL;
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
	else if (manifest->language != FABULOR_PLUGIN_LANGUAGE_UNKNOWN)
	{
		if (!manifest_refresh_entrypoint_path (manifest, &entrypoint_error))
		{
			fabulor_plugin_manifest_add_validation (catalog, manifest,
											 "Invalid entrypoint '%s': %s",
											 manifest->entrypoint,
											 entrypoint_error ? entrypoint_error->message : "unknown path error");
			g_clear_error (&entrypoint_error);
			valid = FALSE;
		}
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

	for (i = 0; i < manifest->capabilities->len; i++)
	{
		const char *capability = g_ptr_array_index (manifest->capabilities, i);
		guint j;

		if (!fabulor_capability_is_known (capability))
		{
			fabulor_plugin_manifest_add_validation (catalog, manifest, "Unknown capability '%s'.", capability);
			valid = FALSE;
		}

		for (j = 0; j < i; j++)
		{
			if (g_strcmp0 (capability, g_ptr_array_index (manifest->capabilities, j)) == 0)
			{
				fabulor_plugin_manifest_add_validation (catalog, manifest, "Duplicate capability '%s'.", capability);
				valid = FALSE;
				break;
			}
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
					  FabulorCallbackResult *result,
					  GError **error)
{
	const FabulorAPI *api = (const FabulorAPI *) user_data;
	(void)error;
	*result = FABULOR_CALLBACK_CONTINUE;
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

	libdir = g_getenv ("FABULOR_LIBDIR");
	if (libdir && *libdir)
	{
		return libdir;
	}

	return FABULORLIBDIR;
}

static gboolean
fabulor_development_runtime_roots_enabled (void)
{
	const char *enabled = g_getenv ("FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS");

	return enabled && g_ascii_strcasecmp (enabled, "1") == 0;
}

#ifdef WIN32
static void fabulor_tcl_set_result (Tcl_Interp *interp, const char *message);

static void
fabulor_tcl_command_entry_free (FabulorTclCommandEntry *entry)
{
	if (!entry)
		return;

	g_free (entry->handler_name);
	g_free (entry);
}

static gboolean
fabulor_tcl_remove_state_command (gpointer key, gpointer value, gpointer user_data)
{
	FabulorTclCommandEntry *entry = value;

	(void) key;
	return entry && entry->state == user_data;
}

static void
fabulor_tcl_plugin_state_free (FabulorTclPluginState *state)
{
	if (!state)
	{
		return;
	}

	if (state->interp && fabulor_tcl_runtime.delete_interp)
	{
		fabulor_tcl_runtime.delete_interp (state->interp);
	}
	if (fabulor_tcl_runtime.commands)
	{
		g_hash_table_foreach_remove (fabulor_tcl_runtime.commands,
									 fabulor_tcl_remove_state_command,
									 state);
	}

	g_free (state->plugin_id);
	if (state->capabilities)
	{
		g_hash_table_unref (state->capabilities);
	}
	g_free (state);
}

static gboolean
fabulor_tcl_require_capability (FabulorTclPluginState *state, Tcl_Interp *interp, const char *capability)
{
	char *message;

	if (state && state->simple_addon)
	{
		return TRUE;
	}

	if (state && state->capabilities && g_hash_table_contains (state->capabilities, capability))
	{
		return TRUE;
	}

	message = g_strdup_printf ("Plugin '%s' lacks required capability '%s'.",
							 state && state->plugin_id ? state->plugin_id : "unknown",
							 capability);
	fabulor_tcl_set_result (interp, message);
	g_free (message);
	return FALSE;
}

static gboolean
fabulor_tcl_root_has_runtime (const char *runtime_root)
{
	char *dll_path;
	gboolean exists;

	if (!runtime_root || *runtime_root == '\0')
	{
		return FALSE;
	}

	dll_path = g_build_filename (runtime_root, "bin", "tcl86t.dll", NULL);
	exists = g_file_test (dll_path, G_FILE_TEST_IS_REGULAR);
	g_free (dll_path);
	return exists;
}

static char *
fabulor_tcl_dup_runtime_root_if_valid (const char *runtime_root)
{
	if (!fabulor_tcl_root_has_runtime (runtime_root))
	{
		return NULL;
	}

	return g_canonicalize_filename (runtime_root, NULL);
}

static char *
fabulor_tcl_executable_directory (void)
{
	char buffer[MAX_PATH];
	DWORD length;
	char *last_slash;

	length = GetModuleFileNameA (NULL, buffer, MAX_PATH);
	if (length == 0 || length >= MAX_PATH)
	{
		return NULL;
	}

	buffer[length] = '\0';
	last_slash = strrchr (buffer, '\\');
	if (!last_slash)
	{
		return NULL;
	}

	*last_slash = '\0';
	return g_strdup (buffer);
}

static DWORD
fabulor_win32_get_file_attributes_utf8 (const char *path)
{
	wchar_t *path_utf16;
	DWORD attributes;

	path_utf16 = g_utf8_to_utf16 (path, -1, NULL, NULL, NULL);
	if (!path_utf16)
		return INVALID_FILE_ATTRIBUTES;

	attributes = GetFileAttributesW (path_utf16);
	g_free (path_utf16);
	return attributes;
}

static char *
fabulor_tcl_resolve_runtime_root (void)
{
	const char *env_root;
	char *cwd;
	char *libdir_candidate;
	char *libdir_root;
	char *exe_dir;
	char *exe_runtime_root;
	char *resolved;

	exe_dir = fabulor_tcl_executable_directory ();
	if (exe_dir)
	{
		exe_runtime_root = g_build_filename (exe_dir, "Runtime", "Tcl", NULL);
		g_free (exe_dir);
		resolved = fabulor_tcl_dup_runtime_root_if_valid (exe_runtime_root);
		g_free (exe_runtime_root);
		if (resolved)
		{
			return resolved;
		}
	}

	if (!fabulor_development_runtime_roots_enabled ())
	{
		return NULL;
	}

	env_root = g_getenv ("FABULOR_TCL_RUNTIME_ROOT");
	resolved = fabulor_tcl_dup_runtime_root_if_valid (env_root);
	if (resolved)
	{
		return resolved;
	}

	cwd = g_get_current_dir ();
	libdir_candidate = g_build_filename (cwd, "Runtime", "Tcl", NULL);
	g_free (cwd);
	resolved = fabulor_tcl_dup_runtime_root_if_valid (libdir_candidate);
	g_free (libdir_candidate);
	if (resolved)
	{
		return resolved;
	}

	libdir_root = g_build_filename (manifest_plugin_get_libdir (), "..", "Runtime", "Tcl", NULL);
	resolved = fabulor_tcl_dup_runtime_root_if_valid (libdir_root);
	g_free (libdir_root);
	if (resolved)
	{
		return resolved;
	}

	return NULL;
}

static gboolean
fabulor_tcl_resolve_symbol (FARPROC *target, const char *name, GError **error)
{
	*target = GetProcAddress (fabulor_tcl_runtime.module, name);
	if (!*target)
	{
		g_set_error (error,
					 G_FILE_ERROR,
					 G_FILE_ERROR_FAILED,
					 "Missing Tcl runtime symbol '%s'.",
					 name);
		return FALSE;
	}

	return TRUE;
}

static gboolean
fabulor_tcl_runtime_ensure_loaded (GError **error)
{
	char *dll_path;

	if (fabulor_tcl_runtime.module)
	{
		return TRUE;
	}

	if (!fabulor_tcl_runtime.runtime_root)
	{
		fabulor_tcl_runtime.runtime_root = fabulor_tcl_resolve_runtime_root ();
	}

	if (!fabulor_tcl_runtime.runtime_root)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "Unable to locate the bundled Tcl runtime.");
		return FALSE;
	}

	dll_path = g_build_filename (fabulor_tcl_runtime.runtime_root, "bin", "tcl86t.dll", NULL);
	fabulor_tcl_runtime.library_path = g_build_filename (fabulor_tcl_runtime.runtime_root, "lib", "tcl8.6", NULL);

	fabulor_tcl_runtime.module = LoadLibraryExA (dll_path,
												 NULL,
												 LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	g_free (dll_path);
	if (!fabulor_tcl_runtime.module)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, "Failed to load tcl86t.dll from the bundled runtime.");
		g_clear_pointer (&fabulor_tcl_runtime.library_path, g_free);
		return FALSE;
	}

	if (!fabulor_tcl_resolve_symbol ((FARPROC *) &fabulor_tcl_runtime.create_interp, "Tcl_CreateInterp", error)
		|| !fabulor_tcl_resolve_symbol ((FARPROC *) &fabulor_tcl_runtime.delete_interp, "Tcl_DeleteInterp", error)
		|| !fabulor_tcl_resolve_symbol ((FARPROC *) &fabulor_tcl_runtime.find_executable, "Tcl_FindExecutable", error)
		|| !fabulor_tcl_resolve_symbol ((FARPROC *) &fabulor_tcl_runtime.init_interp, "Tcl_Init", error)
		|| !fabulor_tcl_resolve_symbol ((FARPROC *) &fabulor_tcl_runtime.eval_file, "Tcl_EvalFile", error)
		|| !fabulor_tcl_resolve_symbol ((FARPROC *) &fabulor_tcl_runtime.eval_ex, "Tcl_EvalEx", error)
		|| !fabulor_tcl_resolve_symbol ((FARPROC *) &fabulor_tcl_runtime.get_string_result, "Tcl_GetStringResult", error)
		|| !fabulor_tcl_resolve_symbol ((FARPROC *) &fabulor_tcl_runtime.set_var, "Tcl_SetVar", error)
		|| !fabulor_tcl_resolve_symbol ((FARPROC *) &fabulor_tcl_runtime.create_command, "Tcl_CreateCommand", error)
		|| !fabulor_tcl_resolve_symbol ((FARPROC *) &fabulor_tcl_runtime.merge_args, "Tcl_Merge", error)
		|| !fabulor_tcl_resolve_symbol ((FARPROC *) &fabulor_tcl_runtime.free_value, "Tcl_Free", error)
		|| !fabulor_tcl_resolve_symbol ((FARPROC *) &fabulor_tcl_runtime.set_result, "Tcl_SetResult", error))
	{
		FreeLibrary (fabulor_tcl_runtime.module);
		fabulor_tcl_runtime.module = NULL;
		g_clear_pointer (&fabulor_tcl_runtime.library_path, g_free);
		return FALSE;
	}

	if (!fabulor_tcl_runtime.plugins)
	{
		fabulor_tcl_runtime.plugins = g_hash_table_new_full (g_str_hash,
														 g_str_equal,
														 g_free,
														 (GDestroyNotify) fabulor_tcl_plugin_state_free);
	}
	if (!fabulor_tcl_runtime.commands)
	{
		fabulor_tcl_runtime.commands = g_hash_table_new_full (g_str_hash,
														 g_str_equal,
														 g_free,
														 (GDestroyNotify) fabulor_tcl_command_entry_free);
	}

	if (!fabulor_tcl_runtime.attempted_initialisation)
	{
		fabulor_tcl_runtime.find_executable (NULL);
		fabulor_tcl_runtime.attempted_initialisation = TRUE;
	}

	return TRUE;
}

static void
fabulor_tcl_set_result (Tcl_Interp *interp, const char *message)
{
	if (!interp || !message || !fabulor_tcl_runtime.set_result)
	{
		return;
	}

	fabulor_tcl_runtime.set_result (interp, (char *) message, TCL_VOLATILE);
}

static int
fabulor_tcl_log_cmd (ClientData client_data, Tcl_Interp *interp, int argc, const char *argv[])
{
	FabulorTclPluginState *state = client_data;

	if (argc != 2)
	{
		fabulor_tcl_set_result (interp, "wrong # args: should be \"fabulor::log text\"");
		return TCL_ERROR;
	}

	fabulor_api_log (state->api, "[Tcl:%s] %s", state->plugin_id, argv[1]);
	return TCL_OK;
}

static int
fabulor_tcl_print_cmd (ClientData client_data, Tcl_Interp *interp, int argc, const char *argv[])
{
	FabulorTclPluginState *state = client_data;

	if (argc != 2)
	{
		fabulor_tcl_set_result (interp, "wrong # args: should be \"fabulor::print text\"");
		return TCL_ERROR;
	}
	if (!fabulor_tcl_require_capability (state, interp, "ui.write"))
		return TCL_ERROR;

	fabulor_api_log (state->api, "%s", argv[1]);
	return TCL_OK;
}

static int
fabulor_tcl_command_cmd (ClientData client_data, Tcl_Interp *interp, int argc, const char *argv[])
{
	FabulorTclPluginState *state = client_data;
	session *sess = state->api ? state->api->user_data : NULL;
	char *command;

	if (argc < 2)
	{
		fabulor_tcl_set_result (interp, "wrong # args: should be \"fabulor::command command ?arg ...?\"");
		return TCL_ERROR;
	}
	if (!fabulor_tcl_require_capability (state, interp, "commands.execute"))
		return TCL_ERROR;

	if (!sess)
	{
		fabulor_tcl_set_result (interp, "No active session is available.");
		return TCL_ERROR;
	}

	command = fabulor_tcl_runtime.merge_args (argc - 1, argv + 1);
	handle_command (sess, command, FALSE);
	fabulor_tcl_runtime.free_value (command);
	return TCL_OK;
}

static int
fabulor_tcl_add_user_command_cmd (ClientData client_data, Tcl_Interp *interp, int argc, const char *argv[])
{
	FabulorTclPluginState *state = client_data;

	if (argc != 3)
	{
		fabulor_tcl_set_result (interp, "wrong # args: should be \"fabulor::add_user_command name command\"");
		return TCL_ERROR;
	}
	if (!fabulor_tcl_require_capability (state, interp, "commands.manage"))
		return TCL_ERROR;

	if (!argv[1] || !*argv[1] || !argv[2] || !*argv[2])
	{
		fabulor_tcl_set_result (interp, "User command name and command must be non-empty.");
		return TCL_ERROR;
	}

	list_delentry (&command_list, (char *) argv[1]);
	list_addentry (&command_list, (char *) argv[2], (char *) argv[1]);
	return TCL_OK;
}

static int
fabulor_tcl_remove_user_command_cmd (ClientData client_data, Tcl_Interp *interp, int argc, const char *argv[])
{
	FabulorTclPluginState *state = client_data;

	if (argc != 2)
	{
		fabulor_tcl_set_result (interp, "wrong # args: should be \"fabulor::remove_user_command name\"");
		return TCL_ERROR;
	}
	if (!fabulor_tcl_require_capability (state, interp, "commands.manage"))
		return TCL_ERROR;

	if (!argv[1] || !*argv[1])
	{
		fabulor_tcl_set_result (interp, "User command name must be non-empty.");
		return TCL_ERROR;
	}

	list_delentry (&command_list, (char *) argv[1]);
	return TCL_OK;
}

static int
fabulor_tcl_register_command_cmd (ClientData client_data, Tcl_Interp *interp, int argc, const char *argv[])
{
	FabulorTclPluginState *state = client_data;
	FabulorTclCommandEntry *entry;
	char *command_name;

	if (argc != 3)
	{
		fabulor_tcl_set_result (interp, "wrong # args: should be \"fabulor::register_command name handler\"");
		return TCL_ERROR;
	}
	if (!state || !state->simple_addon)
	{
		fabulor_tcl_set_result (interp, "Direct command registration is available only to simple Tcl add-ons.");
		return TCL_ERROR;
	}
	if (!argv[1] || !*argv[1] || !argv[2] || !*argv[2])
	{
		fabulor_tcl_set_result (interp, "Command name and handler must be non-empty.");
		return TCL_ERROR;
	}

	command_name = g_ascii_strup (argv[1], -1);
	if (strpbrk (command_name, " \t\r\n/") != NULL)
	{
		fabulor_tcl_set_result (interp, "Command names cannot contain whitespace or '/'.");
		g_free (command_name);
		return TCL_ERROR;
	}
	if (g_hash_table_contains (fabulor_tcl_runtime.commands, command_name))
	{
		fabulor_tcl_set_result (interp, "A simple Tcl add-on already registered that command.");
		g_free (command_name);
		return TCL_ERROR;
	}

	entry = g_new0 (FabulorTclCommandEntry, 1);
	entry->state = state;
	entry->handler_name = g_strdup (argv[2]);
	g_hash_table_insert (fabulor_tcl_runtime.commands, command_name, entry);
	return TCL_OK;
}

static const char *
fabulor_tcl_get_info_value (session *sess, const char *name)
{
	if (!name)
	{
		return NULL;
	}

	if (g_ascii_strcasecmp (name, "version") == 0)
	{
		return PACKAGE_VERSION;
	}

	if (g_ascii_strcasecmp (name, "xchatdir") == 0
		|| g_ascii_strcasecmp (name, "xchatdirfs") == 0
		|| g_ascii_strcasecmp (name, "configdir") == 0)
	{
		return get_xdir ();
	}

	if (g_ascii_strcasecmp (name, "libdirfs") == 0)
	{
		return manifest_plugin_get_libdir ();
	}

	if (!sess || !sess->server)
	{
		return NULL;
	}

	if (g_ascii_strcasecmp (name, "away") == 0)
	{
		return sess->server->is_away ? sess->server->last_away_reason : NULL;
	}

	if (g_ascii_strcasecmp (name, "channel") == 0)
	{
		return sess->channel;
	}

	if (g_ascii_strcasecmp (name, "host") == 0)
	{
		return sess->server->hostname;
	}

	if (g_ascii_strcasecmp (name, "modes") == 0)
	{
		return sess->current_modes;
	}

	if (g_ascii_strcasecmp (name, "network") == 0)
	{
		return server_get_network (sess->server, FALSE);
	}

	if (g_ascii_strcasecmp (name, "nick") == 0)
	{
		return sess->server->nick;
	}

	if (g_ascii_strcasecmp (name, "server") == 0)
	{
		return sess->server->connected ? sess->server->servername : NULL;
	}

	if (g_ascii_strcasecmp (name, "topic") == 0)
	{
		return sess->topic;
	}

	return NULL;
}

static int
fabulor_tcl_getinfo_cmd (ClientData client_data, Tcl_Interp *interp, int argc, const char *argv[])
{
	FabulorTclPluginState *state = client_data;
	session *sess = state->api ? state->api->user_data : NULL;
	const char *value;

	if (argc != 2)
	{
		fabulor_tcl_set_result (interp, "wrong # args: should be \"fabulor::getinfo name\"");
		return TCL_ERROR;
	}
	if (!fabulor_tcl_require_capability (state, interp, "session.read"))
		return TCL_ERROR;

	value = fabulor_tcl_get_info_value (sess, argv[1]);
	fabulor_tcl_set_result (interp, value ? value : "");
	return TCL_OK;
}

static int
fabulor_tcl_nickcmp_cmd (ClientData client_data, Tcl_Interp *interp, int argc, const char *argv[])
{
	FabulorTclPluginState *state = client_data;
	session *sess = state->api ? state->api->user_data : NULL;
	char *value;
	int result;

	if (argc != 3)
	{
		fabulor_tcl_set_result (interp, "wrong # args: should be \"fabulor::nickcmp left right\"");
		return TCL_ERROR;
	}
	if (!fabulor_tcl_require_capability (state, interp, "session.read"))
		return TCL_ERROR;

	if (!sess || !sess->server || !sess->server->p_cmp)
	{
		fabulor_tcl_set_result (interp, "No active session compare function is available.");
		return TCL_ERROR;
	}

	result = sess->server->p_cmp (argv[1], argv[2]);
	value = g_strdup_printf ("%d", result);
	fabulor_tcl_set_result (interp, value);
	g_free (value);
	return TCL_OK;
}

static int
fabulor_tcl_send_message_cmd (ClientData client_data, Tcl_Interp *interp, int argc, const char *argv[])
{
	FabulorTclPluginState *state = client_data;
	GError *error = NULL;

	if (argc != 3)
	{
		fabulor_tcl_set_result (interp, "wrong # args: should be \"fabulor::send_message target text\"");
		return TCL_ERROR;
	}
	if (!fabulor_tcl_require_capability (state, interp, "messages.write"))
		return TCL_ERROR;

	if (!state->api || !state->api->send_message
		|| !state->api->send_message (state->api->user_data, argv[1], argv[2], &error))
	{
		fabulor_tcl_set_result (interp, error ? error->message : "Failed to send message.");
		g_clear_error (&error);
		return TCL_ERROR;
	}

	return TCL_OK;
}

static int
fabulor_tcl_get_user_count_cmd (ClientData client_data, Tcl_Interp *interp, int argc, const char *argv[])
{
	FabulorTclPluginState *state = client_data;
	char *value;

	(void) argv;

	if (argc != 1)
	{
		fabulor_tcl_set_result (interp, "wrong # args: should be \"fabulor::get_user_count\"");
		return TCL_ERROR;
	}
	if (!fabulor_tcl_require_capability (state, interp, "session.read"))
		return TCL_ERROR;

	value = g_strdup_printf ("%u", state->api && state->api->get_user_count
		? state->api->get_user_count (state->api->user_data)
		: 0U);
	fabulor_tcl_set_result (interp, value);
	g_free (value);
	return TCL_OK;
}

static int
fabulor_tcl_get_user_info_cmd (ClientData client_data, Tcl_Interp *interp, int argc, const char *argv[])
{
	FabulorTclPluginState *state = client_data;
	FabulorUserInfo user_info;
	const char *pairs[8];
	char *result;

	(void) argv;

	if (argc != 1)
	{
		fabulor_tcl_set_result (interp, "wrong # args: should be \"fabulor::get_user_info\"");
		return TCL_ERROR;
	}
	if (!fabulor_tcl_require_capability (state, interp, "session.read"))
		return TCL_ERROR;

	memset (&user_info, 0, sizeof (user_info));
	if (state->api && state->api->get_user_info)
	{
		state->api->get_user_info (state->api->user_data, &user_info);
	}

	pairs[0] = "nick";
	pairs[1] = user_info.nickname ? user_info.nickname : "";
	pairs[2] = "channel";
	pairs[3] = user_info.channel ? user_info.channel : "";
	pairs[4] = "server";
	pairs[5] = user_info.server_name ? user_info.server_name : "";
	pairs[6] = "network";
	pairs[7] = user_info.network_name ? user_info.network_name : "";

	result = fabulor_tcl_runtime.merge_args (8, pairs);
	fabulor_tcl_set_result (interp, result ? result : "");
	if (result && fabulor_tcl_runtime.free_value)
	{
		fabulor_tcl_runtime.free_value (result);
	}
	return TCL_OK;
}

static int
fabulor_tcl_register_callback_cmd (ClientData client_data, Tcl_Interp *interp, int argc, const char *argv[])
{
	FabulorTclPluginState *state = client_data;
	GError *error = NULL;

	if (argc != 3)
	{
		fabulor_tcl_set_result (interp, "wrong # args: should be \"fabulor::register_callback event handler\"");
		return TCL_ERROR;
	}

	{
		const char *capability = fabulor_event_capability (argv[1]);
		if (!capability)
		{
			fabulor_tcl_set_result (interp, "Unsupported callback event.");
			return TCL_ERROR;
		}
		if (!fabulor_tcl_require_capability (state, interp, capability))
			return TCL_ERROR;
	}

	if (!fabulor_active_callback_registry_register (argv[1],
												state->plugin_id,
												FABULOR_PLUGIN_LANGUAGE_TCL,
												argv[2],
												&error))
	{
		fabulor_tcl_set_result (interp, error ? error->message : "Callback registration failed.");
		g_clear_error (&error);
		return TCL_ERROR;
	}

	return TCL_OK;
}

static gboolean
fabulor_tcl_eval_command (Tcl_Interp *interp, gint argc, const char *const *argv, GError **error)
{
	char *script;
	int result;

	script = fabulor_tcl_runtime.merge_args (argc, argv);
	result = fabulor_tcl_runtime.eval_ex (interp, script, -1, 0);
	fabulor_tcl_runtime.free_value (script);

	if (result != TCL_OK)
	{
		g_set_error (error,
					 G_FILE_ERROR,
					 G_FILE_ERROR_FAILED,
					 "%s",
					 fabulor_tcl_runtime.get_string_result (interp));
		return FALSE;
	}

	return TRUE;
}

static gboolean
fabulor_tcl_register_commands (FabulorTclPluginState *state, GError **error)
{
	const char *namespace_script = "namespace eval fabulor {}";

	if (fabulor_tcl_runtime.eval_ex (state->interp, namespace_script, -1, 0) != TCL_OK)
	{
		g_set_error (error,
					 G_FILE_ERROR,
					 G_FILE_ERROR_FAILED,
					 "%s",
					 fabulor_tcl_runtime.get_string_result (state->interp));
		return FALSE;
	}

	fabulor_tcl_runtime.create_command (state->interp, "fabulor::log", fabulor_tcl_log_cmd, state, NULL);
	fabulor_tcl_runtime.create_command (state->interp, "fabulor::send_message", fabulor_tcl_send_message_cmd, state, NULL);
	fabulor_tcl_runtime.create_command (state->interp, "fabulor::get_user_count", fabulor_tcl_get_user_count_cmd, state, NULL);
	fabulor_tcl_runtime.create_command (state->interp, "fabulor::get_user_info", fabulor_tcl_get_user_info_cmd, state, NULL);

	if (state->simple_addon)
	{
		fabulor_tcl_runtime.create_command (state->interp, "fabulor::print", fabulor_tcl_print_cmd, state, NULL);
		fabulor_tcl_runtime.create_command (state->interp, "fabulor::command", fabulor_tcl_command_cmd, state, NULL);
		fabulor_tcl_runtime.create_command (state->interp, "fabulor::add_user_command", fabulor_tcl_add_user_command_cmd, state, NULL);
		fabulor_tcl_runtime.create_command (state->interp, "fabulor::remove_user_command", fabulor_tcl_remove_user_command_cmd, state, NULL);
		fabulor_tcl_runtime.create_command (state->interp, "fabulor::register_command", fabulor_tcl_register_command_cmd, state, NULL);
		fabulor_tcl_runtime.create_command (state->interp, "fabulor::getinfo", fabulor_tcl_getinfo_cmd, state, NULL);
		fabulor_tcl_runtime.create_command (state->interp, "fabulor::nickcmp", fabulor_tcl_nickcmp_cmd, state, NULL);
	}
	else
	{
		fabulor_tcl_runtime.create_command (state->interp, "fabulor::register_callback", fabulor_tcl_register_callback_cmd, state, NULL);
	}
	return TRUE;
}

static void
fabulor_tcl_runtime_reset (void)
{
	if (fabulor_tcl_runtime.commands)
	{
		g_hash_table_remove_all (fabulor_tcl_runtime.commands);
		g_hash_table_unref (fabulor_tcl_runtime.commands);
		fabulor_tcl_runtime.commands = NULL;
	}
	if (fabulor_tcl_runtime.plugins)
	{
		g_hash_table_remove_all (fabulor_tcl_runtime.plugins);
		g_hash_table_unref (fabulor_tcl_runtime.plugins);
		fabulor_tcl_runtime.plugins = NULL;
	}

	if (fabulor_tcl_runtime.module)
	{
		FreeLibrary (fabulor_tcl_runtime.module);
		fabulor_tcl_runtime.module = NULL;
	}

	g_free (fabulor_tcl_runtime.library_path);
	g_free (fabulor_tcl_runtime.runtime_root);
	memset (&fabulor_tcl_runtime, 0, sizeof (fabulor_tcl_runtime));
}

static void
fabulor_csharp_runtime_reset (void)
{
	if (fabulor_csharp_runtime.initialised && fabulor_csharp_runtime.shutdown_bridge)
	{
		fabulor_csharp_runtime.shutdown_bridge (NULL, 0);
	}

	if (fabulor_csharp_runtime.hostfxr_module)
	{
		FreeLibrary (fabulor_csharp_runtime.hostfxr_module);
	}
	if (fabulor_simple_csharp_manifests)
	{
		g_hash_table_unref (fabulor_simple_csharp_manifests);
		fabulor_simple_csharp_manifests = NULL;
	}

	g_free (fabulor_csharp_runtime.dotnet_root);
	g_free (fabulor_csharp_runtime.bridge_root);
	g_free (fabulor_csharp_runtime.bridge_assembly_path);
	g_free (fabulor_csharp_runtime.bridge_runtime_config_path);
	memset (&fabulor_csharp_runtime, 0, sizeof (fabulor_csharp_runtime));
}

static gboolean
fabulor_csharp_root_has_hostfxr (const char *dotnet_root)
{
	char *fxr_root;
	gboolean exists;

	if (!dotnet_root || *dotnet_root == '\0')
	{
		return FALSE;
	}

	fxr_root = g_build_filename (dotnet_root, "host", "fxr", NULL);
	exists = g_file_test (fxr_root, G_FILE_TEST_IS_DIR);
	g_free (fxr_root);
	return exists;
}

static gboolean
fabulor_csharp_bridge_root_has_payload (const char *bridge_root)
{
	char *assembly_path;
	char *runtime_config_path;
	gboolean exists;

	if (!bridge_root || *bridge_root == '\0')
	{
		return FALSE;
	}

	assembly_path = g_build_filename (bridge_root, "Fabulor.PluginHost.dll", NULL);
	runtime_config_path = g_build_filename (bridge_root, "Fabulor.PluginHost.runtimeconfig.json", NULL);
	exists = g_file_test (assembly_path, G_FILE_TEST_IS_REGULAR)
		&& g_file_test (runtime_config_path, G_FILE_TEST_IS_REGULAR);
	g_free (assembly_path);
	g_free (runtime_config_path);
	return exists;
}

static char *
fabulor_csharp_dup_dotnet_root_if_valid (const char *dotnet_root)
{
	if (!fabulor_csharp_root_has_hostfxr (dotnet_root))
	{
		return NULL;
	}

	return g_canonicalize_filename (dotnet_root, NULL);
}

static char *
fabulor_csharp_dup_bridge_root_if_valid (const char *bridge_root)
{
	if (!fabulor_csharp_bridge_root_has_payload (bridge_root))
	{
		return NULL;
	}

	return g_canonicalize_filename (bridge_root, NULL);
}

static int
fabulor_version_part (const char *value)
{
	char *end = NULL;
	long result;

	if (!value || *value == '\0')
	{
		return 0;
	}

	result = strtol (value, &end, 10);
	return end == value ? 0 : (int) result;
}

static gint
fabulor_csharp_compare_versions (const char *left, const char *right)
{
	gchar **left_parts;
	gchar **right_parts;
	gint result = 0;
	guint index;

	left_parts = g_strsplit (left, ".", 4);
	right_parts = g_strsplit (right, ".", 4);

	for (index = 0; index < 4; index++)
	{
		int left_value = fabulor_version_part (left_parts[index]);
		int right_value = fabulor_version_part (right_parts[index]);

		if (left_value != right_value)
		{
			result = left_value < right_value ? -1 : 1;
			break;
		}
	}

	g_strfreev (left_parts);
	g_strfreev (right_parts);
	return result;
}

static char *
fabulor_csharp_find_latest_hostfxr_path (const char *dotnet_root)
{
	GDir *directory;
	const char *entry_name;
	char *fxr_root;
	char *best_name = NULL;

	fxr_root = g_build_filename (dotnet_root, "host", "fxr", NULL);
	directory = g_dir_open (fxr_root, 0, NULL);
	if (!directory)
	{
		g_free (fxr_root);
		return NULL;
	}

	while ((entry_name = g_dir_read_name (directory)) != NULL)
	{
		char *candidate_dir;
		char *candidate_dll;

		if (entry_name[0] == '.')
		{
			continue;
		}

		candidate_dir = g_build_filename (fxr_root, entry_name, NULL);
		candidate_dll = g_build_filename (candidate_dir, "hostfxr.dll", NULL);
		if (g_file_test (candidate_dll, G_FILE_TEST_IS_REGULAR)
			&& (!best_name || fabulor_csharp_compare_versions (entry_name, best_name) > 0))
		{
			g_free (best_name);
			best_name = g_strdup (entry_name);
		}

		g_free (candidate_dll);
		g_free (candidate_dir);
	}

	g_dir_close (directory);

	if (!best_name)
	{
		g_free (fxr_root);
		return NULL;
	}

	{
		char *best_path = g_build_filename (fxr_root, best_name, "hostfxr.dll", NULL);
		g_free (best_name);
		g_free (fxr_root);
		return best_path;
	}
}

static wchar_t *
fabulor_utf16_from_utf8 (const char *text)
{
	return (wchar_t *) g_utf8_to_utf16 (text, -1, NULL, NULL, NULL);
}

static gboolean
fabulor_csharp_resolve_symbol (FARPROC *target, HMODULE module, const char *name, GError **error)
{
	*target = GetProcAddress (module, name);
	if (!*target)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, "Missing .NET hosting symbol '%s'.", name);
		return FALSE;
	}

	return TRUE;
}

static char *
fabulor_csharp_resolve_dotnet_root (void)
{
	const char *env_root;
	char *cwd;
	char *candidate;
	char *exe_dir;
	char *resolved;

	exe_dir = fabulor_tcl_executable_directory ();
	if (exe_dir)
	{
		candidate = g_build_filename (exe_dir, "Runtime", "DotNet", NULL);
		g_free (exe_dir);
		resolved = fabulor_csharp_dup_dotnet_root_if_valid (candidate);
		g_free (candidate);
		if (resolved)
		{
			return resolved;
		}
	}

	if (!fabulor_development_runtime_roots_enabled ())
	{
		return NULL;
	}

	env_root = g_getenv ("FABULOR_DOTNET_ROOT");
	resolved = fabulor_csharp_dup_dotnet_root_if_valid (env_root);
	if (resolved)
	{
		return resolved;
	}

	env_root = g_getenv ("DOTNET_ROOT");
	resolved = fabulor_csharp_dup_dotnet_root_if_valid (env_root);
	if (resolved)
	{
		return resolved;
	}

	cwd = g_get_current_dir ();
	candidate = g_build_filename (cwd, "Runtime", "DotNet", NULL);
	g_free (cwd);
	resolved = fabulor_csharp_dup_dotnet_root_if_valid (candidate);
	g_free (candidate);
	if (resolved)
	{
		return resolved;
	}

	candidate = g_strdup ("C:\\Program Files\\dotnet");
	resolved = fabulor_csharp_dup_dotnet_root_if_valid (candidate);
	g_free (candidate);
	if (resolved)
	{
		return resolved;
	}
	return NULL;
}

static char *
fabulor_csharp_resolve_bridge_root (void)
{
	const char *env_root;
	char *cwd;
	char *candidate;
	char *exe_dir;
	char *resolved;

	exe_dir = fabulor_tcl_executable_directory ();
	if (exe_dir)
	{
		candidate = g_build_filename (exe_dir, "Runtime", "DotNet", NULL);
		g_free (exe_dir);
		resolved = fabulor_csharp_dup_bridge_root_if_valid (candidate);
		g_free (candidate);
		if (resolved)
		{
			return resolved;
		}
	}

	if (!fabulor_development_runtime_roots_enabled ())
	{
		return NULL;
	}

	env_root = g_getenv ("FABULOR_CSHARP_BRIDGE_ROOT");
	resolved = fabulor_csharp_dup_bridge_root_if_valid (env_root);
	if (resolved)
	{
		return resolved;
	}

	cwd = g_get_current_dir ();

	candidate = g_build_filename (cwd, "Runtime", "DotNet", NULL);
	resolved = fabulor_csharp_dup_bridge_root_if_valid (candidate);
	g_free (candidate);
	if (resolved)
	{
		g_free (cwd);
		return resolved;
	}

	candidate = g_build_filename (cwd, "src", "managed", "Fabulor.PluginHost", "bin", "Release", "net8.0", NULL);
	resolved = fabulor_csharp_dup_bridge_root_if_valid (candidate);
	g_free (candidate);
	if (resolved)
	{
		g_free (cwd);
		return resolved;
	}

	candidate = g_build_filename (cwd, "src", "managed", "Fabulor.PluginHost", "bin", "Debug", "net8.0", NULL);
	g_free (cwd);
	resolved = fabulor_csharp_dup_bridge_root_if_valid (candidate);
	g_free (candidate);
	if (resolved)
	{
		return resolved;
	}

	return NULL;
}

static void FABULOR_CORECLR_DELEGATE_CALLTYPE
fabulor_csharp_native_log (const char *text)
{
	if (fabulor_active_api)
	{
		fabulor_api_log (fabulor_active_api, "[C#] %s", text ? text : "");
	}
}

static gboolean
fabulor_csharp_require_capability (const char *plugin_id, const char *capability)
{
	if (fabulor_active_callback_registry_has_capability (plugin_id, capability))
	{
		return TRUE;
	}

	fabulor_api_log (fabulor_active_api, "[C#:%s] denied operation requiring capability '%s'.",
					 plugin_id ? plugin_id : "unknown", capability);
	return FALSE;
}

static int FABULOR_CORECLR_DELEGATE_CALLTYPE
fabulor_csharp_native_send_message (const char *plugin_id, const char *target, const char *text)
{
	GError *error = NULL;
	gboolean success;

	if (!fabulor_csharp_require_capability (plugin_id, "messages.write")
		|| !fabulor_active_api || !fabulor_active_api->send_message)
	{
		return 0;
	}

	success = fabulor_active_api->send_message (fabulor_active_api->user_data, target, text, &error);
	if (!success)
	{
		fabulor_api_log (fabulor_active_api, "[C#] send_message failed: %s", error ? error->message : "unknown error");
		g_clear_error (&error);
	}

	return success ? 1 : 0;
}

static unsigned int FABULOR_CORECLR_DELEGATE_CALLTYPE
fabulor_csharp_native_get_user_count (const char *plugin_id)
{
	return fabulor_csharp_require_capability (plugin_id, "session.read")
		&& fabulor_active_api && fabulor_active_api->get_user_count
		? fabulor_active_api->get_user_count (fabulor_active_api->user_data)
		: 0U;
}

static int FABULOR_CORECLR_DELEGATE_CALLTYPE
fabulor_csharp_native_get_user_info (const char *plugin_id, FabulorUserInfo *user_info)
{
	if (!user_info)
	{
		return 0;
	}

	memset (user_info, 0, sizeof (*user_info));
	return fabulor_csharp_require_capability (plugin_id, "session.read")
		&& fabulor_active_api && fabulor_active_api->get_user_info
		&& fabulor_active_api->get_user_info (fabulor_active_api->user_data, user_info);
}

static int FABULOR_CORECLR_DELEGATE_CALLTYPE
fabulor_csharp_native_register_callback (const char *plugin_id, const char *event_name, const char *handler_name)
{
	GError *error = NULL;
	gboolean success;

	{
		const char *capability = fabulor_event_capability (event_name);
		if (!capability || !fabulor_csharp_require_capability (plugin_id, capability))
		{
			return 0;
		}
	}

	success = fabulor_active_callback_registry_register (event_name,
												  plugin_id,
												  FABULOR_PLUGIN_LANGUAGE_CSHARP,
												  handler_name,
												  &error);
	if (!success)
	{
		fabulor_api_log (fabulor_active_api, "[C#] register_callback failed: %s", error ? error->message : "unknown error");
		g_clear_error (&error);
	}

	return success ? 1 : 0;
}

static gboolean
fabulor_csharp_load_entry_point (const wchar_t *assembly_path,
								 const wchar_t *type_name,
								 const wchar_t *method_name,
								 const char *method_name_log,
								 FabulorComponentEntryPointFn *entry_point,
								 GError **error)
{
	void *delegate_handle = NULL;
	int rc;

	rc = fabulor_csharp_runtime.load_assembly_and_get_function_pointer (assembly_path,
																 type_name,
																 method_name,
																 NULL,
																 NULL,
																 &delegate_handle);
	if (rc != 0 || !delegate_handle)
	{
		g_set_error (error,
					 G_FILE_ERROR,
					 G_FILE_ERROR_FAILED,
					 "Failed to resolve managed entry point '%s' (0x%x).",
					 method_name_log,
					 rc);
		return FALSE;
	}

	*entry_point = (FabulorComponentEntryPointFn) delegate_handle;
	return TRUE;
}

static gboolean
fabulor_csharp_runtime_ensure_loaded (const FabulorAPI *api, GError **error)
{
	char *hostfxr_path;
	wchar_t *runtime_config_path_wide;
	wchar_t *bridge_assembly_path_wide;
	fabulor_hostfxr_handle host_context = NULL;
	int rc;
	FabulorManagedHostApi managed_api;
	static const wchar_t native_exports_type[] = L"Fabulor.PluginHost.NativeExports, Fabulor.PluginHost";
	static const wchar_t initialize_method[] = L"Initialize";
	static const wchar_t load_plugin_method[] = L"LoadPlugin";
	static const wchar_t dispatch_callback_method[] = L"DispatchCallback";
	static const wchar_t shutdown_method[] = L"Shutdown";

	fabulor_active_api = api;

	if (fabulor_csharp_runtime.initialised)
	{
		return TRUE;
	}

	fabulor_csharp_runtime.dotnet_root = fabulor_csharp_resolve_dotnet_root ();
	fabulor_csharp_runtime.bridge_root = fabulor_csharp_resolve_bridge_root ();
	if (!fabulor_csharp_runtime.dotnet_root || !fabulor_csharp_runtime.bridge_root)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "Unable to locate the .NET runtime or Fabulor.PluginHost bridge output.");
		return FALSE;
	}

	fabulor_csharp_runtime.bridge_assembly_path = g_build_filename (fabulor_csharp_runtime.bridge_root, "Fabulor.PluginHost.dll", NULL);
	fabulor_csharp_runtime.bridge_runtime_config_path = g_build_filename (fabulor_csharp_runtime.bridge_root, "Fabulor.PluginHost.runtimeconfig.json", NULL);
	if (!g_file_test (fabulor_csharp_runtime.bridge_assembly_path, G_FILE_TEST_EXISTS)
		|| !g_file_test (fabulor_csharp_runtime.bridge_runtime_config_path, G_FILE_TEST_EXISTS))
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "The Fabulor.PluginHost bridge output is incomplete.");
		return FALSE;
	}

	hostfxr_path = fabulor_csharp_find_latest_hostfxr_path (fabulor_csharp_runtime.dotnet_root);
	if (!hostfxr_path)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "Unable to locate hostfxr.dll under the selected .NET root.");
		return FALSE;
	}

	fabulor_csharp_runtime.hostfxr_module = LoadLibraryExA (hostfxr_path,
													 NULL,
													 LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	g_free (hostfxr_path);
	if (!fabulor_csharp_runtime.hostfxr_module)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, "Failed to load hostfxr.dll.");
		return FALSE;
	}

	if (!fabulor_csharp_resolve_symbol ((FARPROC *) &fabulor_csharp_runtime.initialize_for_runtime_config, fabulor_csharp_runtime.hostfxr_module, "hostfxr_initialize_for_runtime_config", error)
		|| !fabulor_csharp_resolve_symbol ((FARPROC *) &fabulor_csharp_runtime.get_runtime_delegate, fabulor_csharp_runtime.hostfxr_module, "hostfxr_get_runtime_delegate", error)
		|| !fabulor_csharp_resolve_symbol ((FARPROC *) &fabulor_csharp_runtime.close_host_context, fabulor_csharp_runtime.hostfxr_module, "hostfxr_close", error))
	{
		fabulor_csharp_runtime_reset ();
		return FALSE;
	}

	runtime_config_path_wide = fabulor_utf16_from_utf8 (fabulor_csharp_runtime.bridge_runtime_config_path);
	rc = fabulor_csharp_runtime.initialize_for_runtime_config (runtime_config_path_wide, NULL, &host_context);
	g_free (runtime_config_path_wide);
	if (rc != 0 || !host_context)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, "hostfxr initialisation failed (0x%x).", rc);
		fabulor_csharp_runtime_reset ();
		return FALSE;
	}

	rc = fabulor_csharp_runtime.get_runtime_delegate (host_context,
												 FABULOR_HDT_LOAD_ASSEMBLY_AND_GET_FUNCTION_POINTER,
												 (void **) &fabulor_csharp_runtime.load_assembly_and_get_function_pointer);
	fabulor_csharp_runtime.close_host_context (host_context);
	if (rc != 0 || !fabulor_csharp_runtime.load_assembly_and_get_function_pointer)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, "Failed to resolve the load_assembly_and_get_function_pointer delegate (0x%x).", rc);
		fabulor_csharp_runtime_reset ();
		return FALSE;
	}

	bridge_assembly_path_wide = fabulor_utf16_from_utf8 (fabulor_csharp_runtime.bridge_assembly_path);
	if (!fabulor_csharp_load_entry_point (bridge_assembly_path_wide, native_exports_type, initialize_method, "Initialize", &fabulor_csharp_runtime.initialize_bridge, error)
		|| !fabulor_csharp_load_entry_point (bridge_assembly_path_wide, native_exports_type, load_plugin_method, "LoadPlugin", &fabulor_csharp_runtime.load_plugin, error)
		|| !fabulor_csharp_load_entry_point (bridge_assembly_path_wide, native_exports_type, dispatch_callback_method, "DispatchCallback", &fabulor_csharp_runtime.dispatch_callback, error)
		|| !fabulor_csharp_load_entry_point (bridge_assembly_path_wide, native_exports_type, shutdown_method, "Shutdown", &fabulor_csharp_runtime.shutdown_bridge, error))
	{
		g_free (bridge_assembly_path_wide);
		fabulor_csharp_runtime_reset ();
		return FALSE;
	}
	g_free (bridge_assembly_path_wide);

	managed_api.log = fabulor_csharp_native_log;
	managed_api.send_message = fabulor_csharp_native_send_message;
	managed_api.get_user_count = fabulor_csharp_native_get_user_count;
	managed_api.get_user_info = fabulor_csharp_native_get_user_info;
	managed_api.register_callback = fabulor_csharp_native_register_callback;

	rc = fabulor_csharp_runtime.initialize_bridge (&managed_api, sizeof (managed_api));
	if (rc != 0)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, "Managed bridge initialisation failed (0x%x).", rc);
		fabulor_csharp_runtime_reset ();
		return FALSE;
	}

	fabulor_csharp_runtime.initialised = TRUE;
	return TRUE;
}
#endif

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
	else
	{
		char *command;

		if (!fabulor_python_manifest_token)
		{
			fabulor_python_manifest_token = g_uuid_string_random ();
		}

		command = g_strdup_printf ("PY MANIFEST_INIT %s", fabulor_python_manifest_token);
		handle_command (sess, command, FALSE);
		g_free (command);
	}

	g_free (runtime_path);
	return success;
}

static gboolean
manifest_command_path_is_safe (const char *path)
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

static gboolean
load_python_manifest (const FabulorPluginManifest *manifest,
					  const FabulorAPI *api,
					  void *user_data,
					  GError **error)
{
	session *sess = (session *) user_data;
	GString *capabilities;
	char *encoded_capabilities;
	char *encoded_entrypoint;
	char *encoded_plugin_id;
	char *command;
	guint i;

	if (!sess)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Python plugin loading requires a valid session context.");
		return FALSE;
	}

	if (!ensure_python_runtime_loaded (sess, error))
	{
		return FALSE;
	}

	if (!manifest_command_path_is_safe (manifest->entrypoint_path))
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Python plugin entrypoint path contains unsupported command characters.");
		return FALSE;
	}

	capabilities = g_string_new (NULL);
	for (i = 0; i < manifest->capabilities->len; i++)
	{
		if (i > 0)
		{
			g_string_append_c (capabilities, ',');
		}
		g_string_append (capabilities, g_ptr_array_index (manifest->capabilities, i));
	}

	encoded_plugin_id = g_base64_encode ((const guchar *) manifest->id, strlen (manifest->id));
	g_string_prepend (capabilities, "capabilities:");
	encoded_capabilities = g_base64_encode ((const guchar *) capabilities->str, capabilities->len);
	encoded_entrypoint = g_base64_encode ((const guchar *) manifest->entrypoint_path, strlen (manifest->entrypoint_path));
	command = g_strdup_printf ("PY MANIFEST_LOAD %s %s %s %s",
							 fabulor_python_manifest_token,
							 encoded_plugin_id,
							 encoded_capabilities,
							 encoded_entrypoint);
	handle_command (sess, command, FALSE);
	g_free (command);
	g_free (encoded_entrypoint);
	g_free (encoded_capabilities);
	g_free (encoded_plugin_id);
	g_string_free (capabilities, TRUE);

	fabulor_api_log (api, "Requested policy-bound Python manifest load for %s from %s.", manifest->id, manifest->entrypoint_path);
	return TRUE;
}

static gboolean
load_tcl_manifest (const FabulorPluginManifest *manifest,
				   const FabulorAPI *api,
				   void *user_data,
				   GError **error)
{
#ifdef WIN32
	FabulorTclPluginState *state;
	const char *init_script = "if {[llength [info commands init]]} { init }";
	guint i;

	(void) user_data;

	if (!fabulor_tcl_runtime_ensure_loaded (error))
	{
		return FALSE;
	}

	state = g_new0 (FabulorTclPluginState, 1);
	state->plugin_id = g_strdup (manifest->id);
	state->api = api;
	state->capabilities = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	for (i = 0; i < manifest->capabilities->len; i++)
	{
		g_hash_table_add (state->capabilities, g_strdup (g_ptr_array_index (manifest->capabilities, i)));
	}
	state->interp = fabulor_tcl_runtime.create_interp ();
	if (!state->interp)
	{
		fabulor_tcl_plugin_state_free (state);
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, "Failed to create a Tcl interpreter for %s.", manifest->id);
		return FALSE;
	}

	if (!fabulor_tcl_runtime.set_var (state->interp, "tcl_library", fabulor_tcl_runtime.library_path, TCL_GLOBAL_ONLY))
	{
		g_set_error (error,
					 G_FILE_ERROR,
					 G_FILE_ERROR_FAILED,
					 "Failed to configure Tcl library path for %s.",
					 manifest->id);
		fabulor_tcl_plugin_state_free (state);
		return FALSE;
	}

	if (fabulor_tcl_runtime.init_interp (state->interp) != TCL_OK)
	{
		g_set_error (error,
					 G_FILE_ERROR,
					 G_FILE_ERROR_FAILED,
					 "Tcl runtime initialisation failed for %s: %s",
					 manifest->id,
					 fabulor_tcl_runtime.get_string_result (state->interp));
		fabulor_tcl_plugin_state_free (state);
		return FALSE;
	}

	g_hash_table_replace (fabulor_tcl_runtime.plugins, g_strdup (manifest->id), state);

	if (!fabulor_tcl_register_commands (state, error)
		|| fabulor_tcl_runtime.eval_file (state->interp, manifest->entrypoint_path) != TCL_OK
		|| fabulor_tcl_runtime.eval_ex (state->interp, init_script, -1, 0) != TCL_OK)
	{
		if (!error || !*error)
		{
			g_set_error (error,
						 G_FILE_ERROR,
						 G_FILE_ERROR_FAILED,
						 "Tcl manifest load failed for %s: %s",
						 manifest->id,
						 fabulor_tcl_runtime.get_string_result (state->interp));
		}

		fabulor_active_callback_registry_remove_plugin (manifest->id);
		g_hash_table_remove (fabulor_tcl_runtime.plugins, manifest->id);
		return FALSE;
	}

	fabulor_api_log (api, "Loaded Tcl manifest plugin %s from %s.", manifest->id, manifest->entrypoint_path);
	return TRUE;
#else
	(void) manifest;
	(void) api;
	(void) user_data;
	g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOSYS, "Tcl manifest loading is only available on Windows builds.");
	return FALSE;
#endif
}

gboolean
fabulor_plugin_host_autoload_simple_tcl (const char *addons_root,
											 const FabulorAPI *api,
											 GError **error)
{
#ifdef WIN32
	GDir *directory;
	GList *names = NULL;
	GList *item;
	const char *entry_name;
	char *canonical_root;
	gsize root_length;
	DWORD root_attributes;

	if (!addons_root || !api || !g_file_test (addons_root, G_FILE_TEST_IS_DIR))
		return TRUE;

	canonical_root = g_canonicalize_filename (addons_root, NULL);
	root_length = strlen (canonical_root);
	root_attributes = fabulor_win32_get_file_attributes_utf8 (canonical_root);
	if (root_attributes == INVALID_FILE_ATTRIBUTES
		|| !(root_attributes & FILE_ATTRIBUTE_DIRECTORY)
		|| (root_attributes & FILE_ATTRIBUTE_REPARSE_POINT))
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
					 "Simple Tcl add-ons root is not a trusted regular directory: %s", canonical_root);
		g_free (canonical_root);
		return FALSE;
	}
	directory = g_dir_open (canonical_root, 0, error);
	if (!directory)
	{
		g_free (canonical_root);
		return FALSE;
	}

	while ((entry_name = g_dir_read_name (directory)) != NULL)
	{
		if (entry_name[0] != '.')
			names = g_list_insert_sorted (names, g_strdup (entry_name), (GCompareFunc) g_ascii_strcasecmp);
	}
	g_dir_close (directory);

	for (item = names; item; item = item->next)
	{
		const char *addon_name = item->data;
		FabulorTclPluginState *state;
		GError *load_error = NULL;
		char *addon_dir = g_build_filename (canonical_root, addon_name, NULL);
		char *script_name = g_strconcat (addon_name, ".tcl", NULL);
		char *script_path = g_build_filename (addon_dir, script_name, NULL);
		char *canonical_script = g_canonicalize_filename (script_path, NULL);
		char *plugin_id = g_strdup_printf ("simple-tcl:%s", addon_name);
		DWORD addon_attributes;
		DWORD script_attributes;
		const char *init_script = "if {[llength [info commands init]]} { init }";

		addon_attributes = fabulor_win32_get_file_attributes_utf8 (addon_dir);
		script_attributes = fabulor_win32_get_file_attributes_utf8 (canonical_script);
		if (addon_attributes == INVALID_FILE_ATTRIBUTES
			|| script_attributes == INVALID_FILE_ATTRIBUTES
			|| !(addon_attributes & FILE_ATTRIBUTE_DIRECTORY)
			|| (script_attributes & FILE_ATTRIBUTE_DIRECTORY)
			|| (addon_attributes & FILE_ATTRIBUTE_REPARSE_POINT)
			|| (script_attributes & FILE_ATTRIBUTE_REPARSE_POINT)
			|| g_ascii_strncasecmp (canonical_script, canonical_root, root_length) != 0
			|| (canonical_script[root_length] != G_DIR_SEPARATOR && canonical_script[root_length] != '/'))
		{
			g_free (plugin_id);
			g_free (canonical_script);
			g_free (script_path);
			g_free (script_name);
			g_free (addon_dir);
			continue;
		}

		if (!fabulor_tcl_runtime_ensure_loaded (error))
		{
			g_free (plugin_id);
			g_free (canonical_script);
			g_free (script_path);
			g_free (script_name);
			g_free (addon_dir);
			g_list_free_full (names, g_free);
			g_free (canonical_root);
			return FALSE;
		}

		state = g_new0 (FabulorTclPluginState, 1);
		state->plugin_id = g_strdup (plugin_id);
		state->api = api;
		state->simple_addon = TRUE;
		state->interp = fabulor_tcl_runtime.create_interp ();

		g_hash_table_replace (fabulor_tcl_runtime.plugins, g_strdup (plugin_id), state);
		if (!state->interp
			|| !fabulor_tcl_runtime.set_var (state->interp, "tcl_library", fabulor_tcl_runtime.library_path, TCL_GLOBAL_ONLY)
			|| fabulor_tcl_runtime.init_interp (state->interp) != TCL_OK
			|| !fabulor_tcl_register_commands (state, &load_error)
			|| fabulor_tcl_runtime.eval_file (state->interp, canonical_script) != TCL_OK
			|| fabulor_tcl_runtime.eval_ex (state->interp, init_script, -1, 0) != TCL_OK)
		{
			fabulor_api_log (api, "Skipping simple Tcl add-on %s: %s",
							 addon_name,
							 load_error ? load_error->message
								: (state->interp ? fabulor_tcl_runtime.get_string_result (state->interp) : "interpreter creation failed"));
			g_clear_error (&load_error);
			g_hash_table_remove (fabulor_tcl_runtime.plugins, plugin_id);
		}
		else
		{
			fabulor_api_log (api, "Loaded simple Tcl add-on %s from %s.", addon_name, canonical_script);
		}

		g_free (plugin_id);
		g_free (canonical_script);
		g_free (script_path);
		g_free (script_name);
		g_free (addon_dir);
	}

	g_list_free_full (names, g_free);
	g_free (canonical_root);
	return TRUE;
#else
	(void) addons_root;
	(void) api;
	(void) error;
	return TRUE;
#endif
}

gboolean
fabulor_plugin_host_handle_simple_tcl_command (const char *command_name,
												const char *arguments,
												GError **error)
{
#ifdef WIN32
	FabulorTclCommandEntry *entry;
	const char *argv[2];
	char *normalized_name;

	if (!fabulor_tcl_runtime.commands || !command_name || !*command_name)
		return FALSE;

	normalized_name = g_ascii_strup (command_name, -1);
	entry = g_hash_table_lookup (fabulor_tcl_runtime.commands, normalized_name);
	g_free (normalized_name);
	if (!entry || !entry->state || !entry->state->interp)
		return FALSE;

	argv[0] = entry->handler_name;
	argv[1] = arguments ? arguments : "";
	fabulor_tcl_eval_command (entry->state->interp, 2, argv, error);
	return TRUE;
#else
	(void) command_name;
	(void) arguments;
	(void) error;
	return FALSE;
#endif
}

void
fabulor_plugin_host_append_loaded_simple_addons (GPtrArray *entries)
{
#ifdef WIN32
	GHashTableIter iterator;
	gpointer value;

	if (!entries)
		return;

	if (fabulor_tcl_runtime.plugins)
	{
		g_hash_table_iter_init (&iterator, fabulor_tcl_runtime.plugins);
		while (g_hash_table_iter_next (&iterator, NULL, &value))
		{
			FabulorTclPluginState *state = value;
			const char *name;

			if (!state || !state->simple_addon || !state->plugin_id)
				continue;

			name = g_str_has_prefix (state->plugin_id, "simple-tcl:")
				? state->plugin_id + strlen ("simple-tcl:")
				: state->plugin_id;
			g_ptr_array_add (entries, g_strdup_printf ("Tcl add-on: %s [%s.tcl]", name, name));
		}
	}

	if (fabulor_simple_csharp_manifests)
	{
		g_hash_table_iter_init (&iterator, fabulor_simple_csharp_manifests);
		while (g_hash_table_iter_next (&iterator, NULL, &value))
		{
			FabulorPluginManifest *manifest = value;
			if (manifest && manifest->name && manifest->entrypoint)
			{
				g_ptr_array_add (entries, g_strdup_printf ("C# add-on: %s [%s]", manifest->name, manifest->entrypoint));
			}
		}
	}
#else
	(void) entries;
#endif
}

static gboolean
dispatch_tcl_callback (const FabulorPluginManifest *manifest,
					   const char *handler_name,
					   const char *event_name,
					   const char *event_payload_json,
					   void *user_data,
					   FabulorCallbackResult *result,
					   GError **error)
{
#ifdef WIN32
	FabulorTclPluginState *state;
	const char *argv[2];

	(void) event_name;
	(void) user_data;

	if (!fabulor_tcl_runtime.plugins)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "Tcl runtime is not active.");
		return FALSE;
	}

	state = g_hash_table_lookup (fabulor_tcl_runtime.plugins, manifest->id);
	if (!state || !state->interp)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "Tcl plugin '%s' is not loaded.", manifest->id);
		return FALSE;
	}

	argv[0] = handler_name;
	argv[1] = event_payload_json ? event_payload_json : "{}";
	if (!fabulor_tcl_eval_command (state->interp, 2, argv, error))
		return FALSE;

	{
		const char *callback_result = fabulor_tcl_runtime.get_string_result (state->interp);
		*result = callback_result
			&& (g_ascii_strcasecmp (callback_result, "consume") == 0
				|| strcmp (callback_result, "1") == 0)
			? FABULOR_CALLBACK_CONSUME
			: FABULOR_CALLBACK_CONTINUE;
	}
	return TRUE;
#else
	(void) manifest;
	(void) handler_name;
	(void) event_name;
	(void) event_payload_json;
	(void) user_data;
	(void) result;
	g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOSYS, "Tcl callback dispatch is only available on Windows builds.");
	return FALSE;
#endif
}

static gboolean
dispatch_csharp_callback (const FabulorPluginManifest *manifest,
						  const char *handler_name,
						  const char *event_name,
						  const char *event_payload_json,
						  void *user_data,
						  FabulorCallbackResult *result,
						  GError **error)
{
#ifdef WIN32
	FabulorManagedDispatchArgs args;
	int rc;

	(void) user_data;

	if (!fabulor_csharp_runtime.initialised || !fabulor_csharp_runtime.dispatch_callback)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "The managed C# bridge is not active.");
		return FALSE;
	}

	args.plugin_id = manifest->id;
	args.handler_name = handler_name;
	args.event_name = event_name;
	args.payload_json = event_payload_json ? event_payload_json : "{}";
	rc = fabulor_csharp_runtime.dispatch_callback (&args, sizeof (args));
	if (rc != FABULOR_CALLBACK_CONTINUE && rc != FABULOR_CALLBACK_CONSUME)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, "Managed callback dispatch failed for %s/%s (0x%x).",
					 manifest->id,
					 handler_name,
					 rc);
		return FALSE;
	}

	*result = (FabulorCallbackResult) rc;
	return TRUE;
#else
	(void) manifest;
	(void) handler_name;
	(void) event_name;
	(void) event_payload_json;
	(void) user_data;
	(void) result;
	g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOSYS, "C# callback dispatch is only available on Windows builds.");
	return FALSE;
#endif
}

static gboolean
load_csharp_manifest (const FabulorPluginManifest *manifest,
					  const FabulorAPI *api,
					  void *user_data,
					  GError **error)
{
#ifdef WIN32
	FabulorManagedLoadPluginArgs args;
	int rc;

	(void) user_data;

	if (!fabulor_csharp_runtime_ensure_loaded (api, error))
	{
		return FALSE;
	}

	args.plugin_id = manifest->id;
	args.assembly_path = manifest->entrypoint_path;
	rc = fabulor_csharp_runtime.load_plugin (&args, sizeof (args));
	if (rc != 0)
	{
		fabulor_active_callback_registry_remove_plugin (manifest->id);
		g_set_error (error,
					 G_FILE_ERROR,
					 G_FILE_ERROR_FAILED,
					 "Managed plugin load failed for %s from %s (0x%x).",
					 manifest->id,
					 manifest->entrypoint_path,
					 rc);
		return FALSE;
	}

	fabulor_api_log (api, "Loaded C# manifest plugin %s from %s.", manifest->id, manifest->entrypoint_path);
	return TRUE;
#else
	(void) manifest;
	(void) api;
	(void) user_data;
	g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOSYS, "C# manifest loading is only available on Windows builds.");
	return FALSE;
#endif
}

gboolean
fabulor_plugin_host_autoload_simple_csharp (const char *addons_root,
												 const FabulorAPI *api,
												 GError **error)
{
#ifdef WIN32
	static const char *trusted_capabilities[] = {
		"events.command", "events.message", "events.print", "events.server",
		"messages.write", "session.read"
	};
	GDir *directory;
	GList *names = NULL;
	GList *item;
	const char *entry_name;
	char *canonical_root;
	gsize root_length;
	DWORD root_attributes;

	if (!addons_root || !api || !g_file_test (addons_root, G_FILE_TEST_IS_DIR))
		return TRUE;

	canonical_root = g_canonicalize_filename (addons_root, NULL);
	root_length = strlen (canonical_root);
	root_attributes = fabulor_win32_get_file_attributes_utf8 (canonical_root);
	if (root_attributes == INVALID_FILE_ATTRIBUTES
		|| !(root_attributes & FILE_ATTRIBUTE_DIRECTORY)
		|| (root_attributes & FILE_ATTRIBUTE_REPARSE_POINT))
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
					 "Simple C# add-ons root is not a trusted regular directory: %s", canonical_root);
		g_free (canonical_root);
		return FALSE;
	}

	directory = g_dir_open (canonical_root, 0, error);
	if (!directory)
	{
		g_free (canonical_root);
		return FALSE;
	}
	while ((entry_name = g_dir_read_name (directory)) != NULL)
	{
		if (entry_name[0] != '.')
			names = g_list_insert_sorted (names, g_strdup (entry_name), (GCompareFunc) g_ascii_strcasecmp);
	}
	g_dir_close (directory);

	if (!fabulor_simple_csharp_manifests)
	{
		fabulor_simple_csharp_manifests = g_hash_table_new_full (g_str_hash,
															 g_str_equal,
															 g_free,
															 (GDestroyNotify) fabulor_plugin_manifest_free);
	}

	for (item = names; item; item = item->next)
	{
		const char *addon_name = item->data;
		FabulorPluginManifest *manifest;
		FabulorManagedLoadPluginArgs args;
		GError *load_error = NULL;
		char *addon_dir = g_build_filename (canonical_root, addon_name, NULL);
		char *assembly_name = g_strconcat (addon_name, ".dll", NULL);
		char *assembly_path = g_build_filename (addon_dir, assembly_name, NULL);
		char *canonical_assembly = g_canonicalize_filename (assembly_path, NULL);
		char *plugin_id = g_strdup_printf ("simple-csharp:%s", addon_name);
		DWORD addon_attributes = fabulor_win32_get_file_attributes_utf8 (addon_dir);
		DWORD assembly_attributes = fabulor_win32_get_file_attributes_utf8 (canonical_assembly);
		guint capability_index;
		int rc;

		if (addon_attributes == INVALID_FILE_ATTRIBUTES
			|| assembly_attributes == INVALID_FILE_ATTRIBUTES
			|| !(addon_attributes & FILE_ATTRIBUTE_DIRECTORY)
			|| (assembly_attributes & FILE_ATTRIBUTE_DIRECTORY)
			|| (addon_attributes & FILE_ATTRIBUTE_REPARSE_POINT)
			|| (assembly_attributes & FILE_ATTRIBUTE_REPARSE_POINT)
			|| strlen (canonical_assembly) <= root_length
			|| g_ascii_strncasecmp (canonical_assembly, canonical_root, root_length) != 0
			|| (canonical_assembly[root_length] != G_DIR_SEPARATOR && canonical_assembly[root_length] != '/'))
		{
			g_free (plugin_id);
			g_free (canonical_assembly);
			g_free (assembly_path);
			g_free (assembly_name);
			g_free (addon_dir);
			continue;
		}

		if (!fabulor_csharp_runtime_ensure_loaded (api, &load_error))
		{
			fabulor_api_log (api, "Skipping simple C# add-on %s: %s", addon_name,
							 load_error ? load_error->message : "managed runtime initialisation failed");
			g_clear_error (&load_error);
			g_free (plugin_id);
			g_free (canonical_assembly);
			g_free (assembly_path);
			g_free (assembly_name);
			g_free (addon_dir);
			continue;
		}

		manifest = fabulor_plugin_manifest_new ();
		manifest->id = g_strdup (plugin_id);
		manifest->name = g_strdup (addon_name);
		manifest->language = FABULOR_PLUGIN_LANGUAGE_CSHARP;
		manifest->language_name = g_strdup ("csharp");
		manifest->plugin_directory = g_strdup (addon_dir);
		manifest->entrypoint = g_strdup (assembly_name);
		manifest->entrypoint_path = g_strdup (canonical_assembly);
		for (capability_index = 0; capability_index < G_N_ELEMENTS (trusted_capabilities); capability_index++)
		{
			g_ptr_array_add (manifest->capabilities, g_strdup (trusted_capabilities[capability_index]));
		}

		g_hash_table_insert (fabulor_simple_csharp_manifests, g_strdup (plugin_id), manifest);
		args.plugin_id = manifest->id;
		args.assembly_path = manifest->entrypoint_path;
		rc = fabulor_csharp_runtime.load_plugin (&args, sizeof (args));
		if (rc != 0)
		{
			fabulor_api_log (api, "Skipping simple C# add-on %s: managed plugin load failed (0x%x).", addon_name, rc);
			fabulor_active_callback_registry_remove_plugin (plugin_id);
			g_hash_table_remove (fabulor_simple_csharp_manifests, plugin_id);
		}
		else
		{
			fabulor_api_log (api, "Loaded simple C# add-on %s from %s.", addon_name, canonical_assembly);
		}

		g_free (plugin_id);
		g_free (canonical_assembly);
		g_free (assembly_path);
		g_free (assembly_name);
		g_free (addon_dir);
	}

	g_list_free_full (names, g_free);
	g_free (canonical_root);
	return TRUE;
#else
	(void) addons_root;
	(void) api;
	(void) error;
	return TRUE;
#endif
}

static const FabulorPluginLoader csharp_loader =
{
	FABULOR_PLUGIN_LANGUAGE_CSHARP,
	"csharp",
	load_csharp_manifest,
	dispatch_csharp_callback
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
	dispatch_tcl_callback
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

static FabulorCallbackEntry *
fabulor_callback_entry_copy (const FabulorCallbackEntry *entry)
{
	FabulorCallbackEntry *copy = g_new0 (FabulorCallbackEntry, 1);
	copy->plugin_id = g_strdup (entry->plugin_id);
	copy->language = entry->language;
	copy->handler_name = g_strdup (entry->handler_name);
	return copy;
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
fabulor_callback_registry_unref (FabulorCallbackRegistry *registry)
{
	if (!g_atomic_int_dec_and_test (&registry->ref_count))
	{
		return;
	}

	g_hash_table_unref (registry->entries);
	g_main_context_unref (registry->main_context);
	g_mutex_clear (&registry->mutex);
	g_free (registry);
}

static FabulorCallbackRegistry *
fabulor_callback_registry_ref (FabulorCallbackRegistry *registry)
{
	g_atomic_int_inc (&registry->ref_count);
	return registry;
}

static gboolean
fabulor_callback_text_is_valid (const char *text, gsize maximum_length)
{
	const char *cursor;

	if (!text || *text == '\0' || strlen (text) > maximum_length || !g_utf8_validate (text, -1, NULL))
	{
		return FALSE;
	}

	for (cursor = text; *cursor; cursor = g_utf8_next_char (cursor))
	{
		if (g_unichar_iscntrl (g_utf8_get_char (cursor)))
		{
			return FALSE;
		}
	}

	return TRUE;
}

static gboolean
fabulor_callback_event_is_valid (const char *event_name)
{
	const char *suffix = NULL;

	if (!fabulor_callback_text_is_valid (event_name, FABULOR_CALLBACK_EVENT_NAME_MAX))
	{
		return FALSE;
	}

	if (g_strcmp0 (event_name, "message") == 0
		|| g_strcmp0 (event_name, "server") == 0
		|| g_strcmp0 (event_name, "print") == 0
		|| g_strcmp0 (event_name, "command") == 0)
	{
		return TRUE;
	}

	if (g_str_has_prefix (event_name, "server:"))
		suffix = event_name + strlen ("server:");
	else if (g_str_has_prefix (event_name, "print:"))
		suffix = event_name + strlen ("print:");
	else if (g_str_has_prefix (event_name, "command:"))
		suffix = event_name + strlen ("command:");

	return suffix && *suffix != '\0';
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

	g_mutex_lock (&dispatch->registry->mutex);
	g_assert (dispatch->registry->queued_dispatches > 0);
	dispatch->registry->queued_dispatches--;
	g_mutex_unlock (&dispatch->registry->mutex);
	fabulor_callback_registry_unref (dispatch->registry);
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
	catalog->discovery_diagnostic_count = 0;
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
	gboolean success;

	if (!catalog || !plugins_root)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Plugin discovery requires a valid catalog and root path.");
		return FALSE;
	}

	fabulor_plugin_catalog_clear (catalog);
	success = discover_manifests_in_root (catalog, plugins_root, error);
	catalog->discovery_diagnostic_count = catalog->diagnostics->len;
	return success;
}

gboolean
fabulor_plugin_catalog_discover_root (FabulorPluginCatalog *catalog, const char *plugins_root, GError **error)
{
	gboolean success;

	if (!catalog || !plugins_root)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Plugin discovery requires a valid catalog and root path.");
		return FALSE;
	}

	success = discover_manifests_in_root (catalog, plugins_root, error);
	catalog->discovery_diagnostic_count = catalog->diagnostics->len;
	return success;
}

GPtrArray *
fabulor_plugin_catalog_resolve_load_order (FabulorPluginCatalog *catalog, guint api_version, GError **error)
{
	GPtrArray *ordered;
	GPtrArray *enabled_manifests;
	GHashTable *available_ids;
	GHashTable *in_degrees;
	GHashTable *dependents;
	GQueue queue = G_QUEUE_INIT;
	GHashTableIter iter;
	gpointer key;
	gpointer value;
	guint i;
	gboolean removed_dependency;

	if (!catalog)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Plugin load-order resolution requires a valid catalog.");
		return NULL;
	}
	g_ptr_array_set_size (catalog->diagnostics, catalog->discovery_diagnostic_count);

	enabled_manifests = g_ptr_array_new ();
	available_ids = g_hash_table_new (g_str_hash, g_str_equal);
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
			continue;
		}

		g_ptr_array_add (enabled_manifests, manifest);
		g_hash_table_add (available_ids, manifest->id);
	}

	do
	{
		removed_dependency = FALSE;
		for (i = enabled_manifests->len; i > 0; i--)
		{
			FabulorPluginManifest *manifest = g_ptr_array_index (enabled_manifests, i - 1);
			guint dependency_index;

			for (dependency_index = 0; dependency_index < manifest->dependencies->len; dependency_index++)
			{
				const char *dependency = g_ptr_array_index (manifest->dependencies, dependency_index);
				if (!g_hash_table_contains (available_ids, dependency))
				{
					fabulor_plugin_manifest_add_validation (catalog,
														manifest,
														"Skipped because dependency '%s' is disabled or invalid.",
														dependency);
					g_hash_table_remove (available_ids, manifest->id);
					g_ptr_array_remove_index (enabled_manifests, i - 1);
					removed_dependency = TRUE;
					break;
				}
			}
		}
	} while (removed_dependency);
	g_hash_table_unref (available_ids);

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
		FabulorPluginManifest *manifest = g_ptr_array_index (ordered_manifests, i);
		const FabulorPluginLoader *loader = fabulor_plugin_loader_for_language (manifest->language);
		GError *entrypoint_error = NULL;

		if (!manifest_refresh_entrypoint_path (manifest, &entrypoint_error))
		{
			g_propagate_prefixed_error (error,
								entrypoint_error,
								"Entrypoint revalidation failed for '%s': ",
								manifest->id ? manifest->id : "(unknown)");
			return FALSE;
		}

		if (!loader)
		{
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "No plugin loader is registered for language '%s'.",
						 fabulor_plugin_language_to_string (manifest->language));
			return FALSE;
		}

		if (!loader->load (manifest, catalog->api, loader_user_data, error))
		{
#ifdef WIN32
			fabulor_active_callback_registry_remove_plugin (manifest->id);
#endif
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
	registry->ref_count = 1;
	g_mutex_init (&registry->mutex);
#ifdef WIN32
	G_LOCK (fabulor_active_callback_registry_lock);
	fabulor_active_callback_registry = registry;
	G_UNLOCK (fabulor_active_callback_registry_lock);
#endif
	return registry;
}

void
fabulor_callback_registry_shutdown (FabulorCallbackRegistry *registry)
{
	if (!registry)
	{
		return;
	}

#ifdef WIN32
	G_LOCK (fabulor_active_callback_registry_lock);
	if (fabulor_active_callback_registry == registry)
	{
		fabulor_active_callback_registry = NULL;
	}
	G_UNLOCK (fabulor_active_callback_registry_lock);
#endif
	g_mutex_lock (&registry->mutex);
	registry->shutting_down = TRUE;
	g_hash_table_remove_all (registry->entries);
	g_mutex_unlock (&registry->mutex);
}

void
fabulor_callback_registry_free (FabulorCallbackRegistry *registry)
{
	if (!registry)
	{
		return;
	}

	fabulor_callback_registry_shutdown (registry);
	fabulor_callback_registry_unref (registry);
}

guint
fabulor_callback_registry_remove_plugin (FabulorCallbackRegistry *registry, const char *plugin_id)
{
	GHashTableIter iter;
	gpointer value;
	guint removed = 0;

	if (!registry || !plugin_id || *plugin_id == '\0')
	{
		return 0;
	}

	g_mutex_lock (&registry->mutex);
	g_hash_table_iter_init (&iter, registry->entries);
	while (g_hash_table_iter_next (&iter, NULL, &value))
	{
		GPtrArray *entries = value;
		guint i = entries->len;

		while (i > 0)
		{
			FabulorCallbackEntry *entry = g_ptr_array_index (entries, --i);
			if (g_strcmp0 (entry->plugin_id, plugin_id) == 0)
			{
				g_ptr_array_remove_index (entries, i);
				removed++;
			}
		}

		if (entries->len == 0)
		{
			g_hash_table_iter_remove (&iter);
		}
	}
	g_mutex_unlock (&registry->mutex);

	return removed;
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
	guint plugin_callback_count = 0;
	GHashTableIter iter;
	gpointer value;

	if (!registry || !event_name || !plugin_id || !handler_name)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Callback registration requires an event name, plugin id, and handler name.");
		return FALSE;
	}
	if (g_thread_self () != registry->main_thread)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Callback registration must run on the plugin host thread.");
		return FALSE;
	}
	if (!fabulor_callback_event_is_valid (event_name))
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Unsupported or invalid callback event '%s'.", event_name);
		return FALSE;
	}
	if (!fabulor_callback_text_is_valid (handler_name, FABULOR_CALLBACK_HANDLER_NAME_MAX))
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Callback handler names must be valid UTF-8 without control characters and at most 256 bytes.");
		return FALSE;
	}

	manifest = fabulor_runtime_find_manifest (registry->catalog, plugin_id);
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

	{
		const char *capability = fabulor_event_capability (event_name);
		if (!capability)
		{
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Unsupported callback event '%s'.", event_name);
			return FALSE;
		}
		if (!fabulor_manifest_has_capability (manifest, capability))
		{
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_ACCES, "Plugin '%s' lacks required capability '%s'.", plugin_id, capability);
			return FALSE;
		}
	}

	g_mutex_lock (&registry->mutex);
	if (registry->shutting_down)
	{
		g_mutex_unlock (&registry->mutex);
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED, "Callback registration is closed during plugin host shutdown.");
		return FALSE;
	}

	g_hash_table_iter_init (&iter, registry->entries);
	while (g_hash_table_iter_next (&iter, NULL, &value))
	{
		GPtrArray *registered = value;
		guint i;
		for (i = 0; i < registered->len; i++)
		{
			FabulorCallbackEntry *registered_entry = g_ptr_array_index (registered, i);
			if (g_strcmp0 (registered_entry->plugin_id, plugin_id) == 0)
			{
				plugin_callback_count++;
			}
		}
	}

	if (plugin_callback_count >= FABULOR_CALLBACKS_PER_PLUGIN_MAX)
	{
		g_mutex_unlock (&registry->mutex);
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_NOSPC, "Plugin '%s' reached the 64-callback limit.", plugin_id);
		return FALSE;
	}

	entries = g_hash_table_lookup (registry->entries, event_name);
	if (!entries)
	{
		entries = g_ptr_array_new_with_free_func ((GDestroyNotify) fabulor_callback_entry_free);
		g_hash_table_insert (registry->entries, g_strdup (event_name), entries);
	}
	else
	{
		guint i;
		if (entries->len >= FABULOR_CALLBACKS_PER_EVENT_MAX)
		{
			g_mutex_unlock (&registry->mutex);
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_NOSPC, "Callback event '%s' reached the 256-callback limit.", event_name);
			return FALSE;
		}

		for (i = 0; i < entries->len; i++)
		{
			FabulorCallbackEntry *registered_entry = g_ptr_array_index (entries, i);
			if (registered_entry->language == language
				&& g_strcmp0 (registered_entry->plugin_id, plugin_id) == 0
				&& g_strcmp0 (registered_entry->handler_name, handler_name) == 0)
			{
				g_mutex_unlock (&registry->mutex);
				g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_EXIST, "Callback '%s' is already registered for plugin '%s' and event '%s'.",
						 handler_name, plugin_id, event_name);
				return FALSE;
			}
		}
	}

	entry = g_new0 (FabulorCallbackEntry, 1);
	entry->plugin_id = g_strdup (plugin_id);
	entry->language = language;
	entry->handler_name = g_strdup (handler_name);
	g_ptr_array_add (entries, entry);
	g_mutex_unlock (&registry->mutex);
	return TRUE;
}

gboolean
fabulor_callback_registry_has_event (FabulorCallbackRegistry *registry, const char *event_name)
{
	GPtrArray *entries;
	gboolean has_event;

	if (!registry || !event_name)
	{
		return FALSE;
	}

	g_mutex_lock (&registry->mutex);
	entries = registry->shutting_down ? NULL : g_hash_table_lookup (registry->entries, event_name);
	has_event = entries && entries->len > 0;
	g_mutex_unlock (&registry->mutex);
	return has_event;
}

static gboolean
fabulor_callback_registry_dispatch_now (FabulorCallbackRegistry *registry,
										const char *event_name,
										const char *event_payload_json,
										void *loader_user_data,
										gboolean *consumed,
										GError **error)
{
	GPtrArray *entries;
	GPtrArray *registered_entries;
	guint i;

	entries = g_ptr_array_new_with_free_func ((GDestroyNotify) fabulor_callback_entry_free);
	if (consumed)
		*consumed = FALSE;
	g_mutex_lock (&registry->mutex);
	if (registry->shutting_down)
	{
		g_mutex_unlock (&registry->mutex);
		g_ptr_array_free (entries, TRUE);
		return TRUE;
	}

	registered_entries = g_hash_table_lookup (registry->entries, event_name);
	if (registered_entries)
	{
		for (i = 0; i < registered_entries->len; i++)
		{
			g_ptr_array_add (entries, fabulor_callback_entry_copy (g_ptr_array_index (registered_entries, i)));
		}
	}
	g_mutex_unlock (&registry->mutex);

	for (i = 0; i < entries->len; i++)
	{
		FabulorCallbackEntry *entry = g_ptr_array_index (entries, i);
		const FabulorPluginManifest *manifest = fabulor_runtime_find_manifest (registry->catalog, entry->plugin_id);
		const FabulorPluginLoader *loader = fabulor_plugin_loader_for_language (entry->language);
		GError *dispatch_error = NULL;
		FabulorCallbackResult callback_result = FABULOR_CALLBACK_CONTINUE;

		if (!manifest || !loader)
		{
			continue;
		}

		if (!loader->dispatch_callback (manifest,
										 entry->handler_name,
										 event_name,
										 event_payload_json,
										 loader_user_data ? loader_user_data : (void *) registry->api,
										 &callback_result,
										 &dispatch_error))
		{
			fabulor_api_log (registry->api, "Callback dispatch failed for %s/%s: %s",
							 entry->plugin_id,
							 entry->handler_name,
							 dispatch_error ? dispatch_error->message : "unknown error");
			g_clear_error (&dispatch_error);
		}
		else if (consumed
			&& manifest->requires_api_version >= FABULOR_PLUGIN_CALLBACK_RESULTS_API_VERSION
			&& callback_result == FABULOR_CALLBACK_CONSUME)
		{
			*consumed = TRUE;
		}
	}

	g_ptr_array_free (entries, TRUE);
	return TRUE;
}

static gboolean
fabulor_callback_registry_invoke_main_thread (gpointer user_data)
{
	FabulorDeferredDispatch *dispatch = user_data;
	fabulor_callback_registry_dispatch_now (dispatch->registry,
										 dispatch->event_name,
										 dispatch->event_payload_json,
										 NULL,
										 NULL,
										 NULL);
	return G_SOURCE_REMOVE;
}

gboolean
fabulor_callback_registry_fire_event (FabulorCallbackRegistry *registry,
									  const char *event_name,
									  const char *event_payload_json,
									  void *loader_user_data,
									  gboolean *consumed,
									  GError **error)
{
	gsize payload_length;

	if (!registry || !event_name)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Event dispatch requires a registry and event name.");
		return FALSE;
	}
	if (consumed)
		*consumed = FALSE;
	if (!fabulor_callback_event_is_valid (event_name))
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Unsupported or invalid callback event '%s'.", event_name);
		return FALSE;
	}

	payload_length = event_payload_json ? strlen (event_payload_json) : 2;
	if (payload_length > FABULOR_CALLBACK_PAYLOAD_MAX)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOSPC, "Callback event payload exceeds the 1 MiB limit.");
		return FALSE;
	}

	if (g_thread_self () == registry->main_thread)
	{
		return fabulor_callback_registry_dispatch_now (registry,
													 event_name,
											 event_payload_json ? event_payload_json : "{}",
											 loader_user_data,
											 consumed,
											 error);
	}
	if (loader_user_data)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Queued callback dispatch cannot retain caller-owned loader data.");
		return FALSE;
	}

	{
		FabulorDeferredDispatch *dispatch = g_new0 (FabulorDeferredDispatch, 1);

		g_mutex_lock (&registry->mutex);
		if (registry->shutting_down)
		{
			g_mutex_unlock (&registry->mutex);
			g_free (dispatch);
			return TRUE;
		}
		if (registry->queued_dispatches >= FABULOR_CALLBACK_QUEUED_DISPATCH_MAX)
		{
			g_mutex_unlock (&registry->mutex);
			g_free (dispatch);
			g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOSPC, "Callback dispatch queue reached its 256-event limit.");
			return FALSE;
		}
		registry->queued_dispatches++;
		dispatch->registry = fabulor_callback_registry_ref (registry);
		g_mutex_unlock (&registry->mutex);
		dispatch->event_name = g_strdup (event_name);
		dispatch->event_payload_json = g_strdup (event_payload_json ? event_payload_json : "{}");
		g_main_context_invoke_full (registry->main_context,
									G_PRIORITY_DEFAULT,
									fabulor_callback_registry_invoke_main_thread,
									dispatch,
									(GDestroyNotify) fabulor_deferred_dispatch_free);
	}

	return TRUE;
}

void
fabulor_plugin_host_shutdown (void)
{
#ifdef WIN32
	fabulor_callback_registry_shutdown (fabulor_active_callback_registry);
	fabulor_csharp_runtime_reset ();
	fabulor_tcl_runtime_reset ();
	fabulor_active_api = NULL;
#endif
}
