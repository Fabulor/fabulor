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

#ifndef FABULOR_PLUGIN_HOST_H
#define FABULOR_PLUGIN_HOST_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FABULOR_PLUGIN_API_VERSION 1U

typedef enum
{
	FABULOR_PLUGIN_LANGUAGE_UNKNOWN = 0,
	FABULOR_PLUGIN_LANGUAGE_CSHARP,
	FABULOR_PLUGIN_LANGUAGE_PYTHON,
	FABULOR_PLUGIN_LANGUAGE_TCL
} FabulorPluginLanguage;

typedef struct _fabulor_user_info
{
	const char *nickname;
	const char *channel;
	const char *server_name;
	const char *network_name;
} FabulorUserInfo;

typedef struct _fabulor_api
{
	guint api_version;
	void *user_data;
	gboolean (*send_message) (void *user_data, const char *target, const char *text, GError **error);
	void (*log) (void *user_data, const char *text);
	guint (*get_user_count) (void *user_data);
	gboolean (*get_user_info) (void *user_data, FabulorUserInfo *user_info);
} FabulorAPI;

typedef FabulorAPI ZoiteChatAPI;

typedef struct _fabulor_plugin_manifest
{
	char *id;
	char *name;
	char *version;
	FabulorPluginLanguage language;
	char *language_name;
	char *entrypoint;
	guint requires_api_version;
	GPtrArray *dependencies;
	GPtrArray *capabilities;
	char *description;
	char *author;
	char *homepage;
	char *plugin_directory;
	char *manifest_path;
	char *entrypoint_path;
} FabulorPluginManifest;

typedef struct _fabulor_plugin_catalog FabulorPluginCatalog;
typedef struct _fabulor_plugin_loader FabulorPluginLoader;
typedef struct _fabulor_callback_entry FabulorCallbackEntry;
typedef struct _fabulor_callback_registry FabulorCallbackRegistry;

typedef gboolean (*FabulorPluginLoadFunc) (const FabulorPluginManifest *manifest,
										   const FabulorAPI *api,
										   void *user_data,
										   GError **error);
typedef gboolean (*FabulorPluginDispatchFunc) (const FabulorPluginManifest *manifest,
											   const char *handler_name,
											   const char *event_name,
											   const char *event_payload_json,
											   void *user_data,
											   GError **error);

struct _fabulor_plugin_loader
{
	FabulorPluginLanguage language;
	const char *language_name;
	FabulorPluginLoadFunc load;
	FabulorPluginDispatchFunc dispatch_callback;
};

struct _fabulor_callback_entry
{
	char *plugin_id;
	FabulorPluginLanguage language;
	char *handler_name;
};

FabulorPluginManifest *fabulor_plugin_manifest_new (void);
void fabulor_plugin_manifest_free (FabulorPluginManifest *manifest);

FabulorPluginCatalog *fabulor_plugin_catalog_new (const FabulorAPI *api);
void fabulor_plugin_catalog_free (FabulorPluginCatalog *catalog);
void fabulor_plugin_catalog_clear (FabulorPluginCatalog *catalog);
void fabulor_plugin_catalog_set_safe_mode (FabulorPluginCatalog *catalog, gboolean enabled);
void fabulor_plugin_catalog_blacklist_plugin (FabulorPluginCatalog *catalog, const char *plugin_id);
gboolean fabulor_plugin_catalog_discover (FabulorPluginCatalog *catalog, const char *plugins_root, GError **error);
gboolean fabulor_plugin_catalog_discover_root (FabulorPluginCatalog *catalog, const char *plugins_root, GError **error);
GPtrArray *fabulor_plugin_catalog_resolve_load_order (FabulorPluginCatalog *catalog, guint api_version, GError **error);
gboolean fabulor_plugin_catalog_load (FabulorPluginCatalog *catalog,
									  GPtrArray *ordered_manifests,
									  void *loader_user_data,
									  GError **error);
const FabulorPluginManifest *fabulor_plugin_catalog_find_manifest (const FabulorPluginCatalog *catalog, const char *plugin_id);
const GPtrArray *fabulor_plugin_catalog_get_manifests (const FabulorPluginCatalog *catalog);
const GPtrArray *fabulor_plugin_catalog_get_diagnostics (const FabulorPluginCatalog *catalog);

const FabulorPluginLoader *fabulor_plugin_loader_for_language (FabulorPluginLanguage language);
const char *fabulor_plugin_language_to_string (FabulorPluginLanguage language);
FabulorPluginLanguage fabulor_plugin_language_from_string (const char *language_name);

FabulorCallbackRegistry *fabulor_callback_registry_new (const FabulorAPI *api,
														const FabulorPluginCatalog *catalog,
														GMainContext *main_context);
void fabulor_callback_registry_free (FabulorCallbackRegistry *registry);
gboolean fabulor_callback_registry_register (FabulorCallbackRegistry *registry,
											 const char *event_name,
											 const char *plugin_id,
											 FabulorPluginLanguage language,
											 const char *handler_name,
											 GError **error);
gboolean fabulor_callback_registry_has_event (FabulorCallbackRegistry *registry,
											  const char *event_name);
gboolean fabulor_callback_registry_fire_event (FabulorCallbackRegistry *registry,
											   const char *event_name,
											   const char *event_payload_json,
											   void *loader_user_data,
											   GError **error);
gboolean fabulor_plugin_host_autoload_simple_tcl (const char *addons_root,
											   const FabulorAPI *api,
											   GError **error);
gboolean fabulor_plugin_host_autoload_simple_csharp (const char *addons_root,
												  const FabulorAPI *api,
												  GError **error);
gboolean fabulor_plugin_host_handle_simple_tcl_command (const char *command_name,
												 const char *arguments,
												 GError **error);
void fabulor_plugin_host_append_loaded_simple_addons (GPtrArray *entries);
void fabulor_plugin_host_shutdown (void);

#ifdef __cplusplus
}
#endif

#endif
