#ifndef FABULOR_PLUGIN_PATH_POLICY_H
#define FABULOR_PLUGIN_PATH_POLICY_H

#include <glib.h>

gboolean fabulor_plugin_path_validate_root (const char *root_path,
											 char **canonical_root,
											 GError **error);
gboolean fabulor_plugin_path_is_directory_candidate (const char *path);
gboolean fabulor_plugin_path_resolve_child_directory (const char *canonical_root,
													 const char *child_name,
													 char **canonical_child,
													 GError **error);
gboolean fabulor_plugin_path_resolve_regular_file (const char *canonical_directory,
												  const char *file_name,
												  char **canonical_file,
												  GError **error);

#endif
