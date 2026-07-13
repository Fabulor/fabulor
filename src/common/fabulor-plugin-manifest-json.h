#ifndef FABULOR_PLUGIN_MANIFEST_JSON_H
#define FABULOR_PLUGIN_MANIFEST_JSON_H

#include "fabulor-plugin-host.h"

#define FABULOR_PLUGIN_MANIFEST_MAX_BYTES (64U * 1024U)

gboolean fabulor_plugin_manifest_parse_json (const char *json,
											  gsize json_length,
											  FabulorPluginManifest *manifest,
											  GError **error);

#endif
