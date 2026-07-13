#include "fabulor-plugin-path-policy.h"

#include <gio/gio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#endif

static gboolean
fabulor_plugin_path_is_direct_name (const char *name)
{
	if (!name || *name == '\0'
		|| g_strcmp0 (name, ".") == 0
		|| g_strcmp0 (name, "..") == 0
		|| g_path_is_absolute (name)
		|| strchr (name, '/')
		|| strchr (name, '\\'))
	{
		return FALSE;
	}

#ifdef WIN32
	if (strchr (name, ':'))
	{
		return FALSE;
	}
#endif

	return TRUE;
}

static gboolean
fabulor_plugin_path_is_strict_child (const char *canonical_parent, const char *canonical_child)
{
	gsize parent_length;

	if (!canonical_parent || !canonical_child)
	{
		return FALSE;
	}

	parent_length = strlen (canonical_parent);
	if (strlen (canonical_child) <= parent_length)
	{
		return FALSE;
	}

#ifdef WIN32
	if (g_ascii_strncasecmp (canonical_parent, canonical_child, parent_length) != 0)
#else
	if (strncmp (canonical_parent, canonical_child, parent_length) != 0)
#endif
	{
		return FALSE;
	}

	return canonical_child[parent_length] == G_DIR_SEPARATOR
		|| canonical_child[parent_length] == '/'
		|| canonical_child[parent_length] == '\\';
}

static gboolean
fabulor_plugin_path_query_trusted_type (const char *path, GFileType expected_type, GError **error)
{
	GFile *file;
	GFileInfo *info;
	GFileType actual_type;
	gboolean is_symlink;

	file = g_file_new_for_path (path);
	info = g_file_query_info (file,
						  G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK,
						  G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
						  NULL,
						  error);
	g_object_unref (file);
	if (!info)
	{
		return FALSE;
	}

	actual_type = g_file_info_get_file_type (info);
	is_symlink = g_file_info_get_is_symlink (info);
	g_object_unref (info);

	if (is_symlink || actual_type == G_FILE_TYPE_SYMBOLIC_LINK)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
					 "Plugin path is a symbolic link: %s", path);
		return FALSE;
	}

#ifdef WIN32
	{
		wchar_t *path_utf16 = g_utf8_to_utf16 (path, -1, NULL, NULL, NULL);
		DWORD attributes;

		if (!path_utf16)
		{
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
						 "Plugin path is not valid UTF-8: %s", path);
			return FALSE;
		}
		attributes = GetFileAttributesW (path_utf16);
		g_free (path_utf16);
		if (attributes == INVALID_FILE_ATTRIBUTES)
		{
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_NOENT,
						 "Unable to inspect plugin path: %s", path);
			return FALSE;
		}
		if ((attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0)
		{
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
						 "Plugin path is a Windows reparse point: %s", path);
			return FALSE;
		}
	}
#endif

	if (actual_type != expected_type)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
					 "Plugin path has the wrong file type: %s", path);
		return FALSE;
	}

	return TRUE;
}

gboolean
fabulor_plugin_path_validate_root (const char *root_path, char **canonical_root, GError **error)
{
	char *resolved;

	if (canonical_root)
	{
		*canonical_root = NULL;
	}
	if (!root_path || *root_path == '\0' || !canonical_root)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
						 "Plugin root validation requires a root path and output destination.");
		return FALSE;
	}

	resolved = g_canonicalize_filename (root_path, NULL);
	if (!fabulor_plugin_path_query_trusted_type (resolved, G_FILE_TYPE_DIRECTORY, error))
	{
		g_free (resolved);
		return FALSE;
	}

	*canonical_root = resolved;
	return TRUE;
}

gboolean
fabulor_plugin_path_resolve_child_directory (const char *canonical_root,
												 const char *child_name,
												 char **canonical_child,
												 GError **error)
{
	char *candidate;

	if (canonical_child)
	{
		*canonical_child = NULL;
	}
	if (!canonical_root || !canonical_child || !fabulor_plugin_path_is_direct_name (child_name))
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
					 "Plugin directory name is not a direct child name: %s",
					 child_name ? child_name : "(null)");
		return FALSE;
	}

	candidate = g_build_filename (canonical_root, child_name, NULL);
	if (!fabulor_plugin_path_is_strict_child (canonical_root, candidate)
		|| !fabulor_plugin_path_query_trusted_type (candidate, G_FILE_TYPE_DIRECTORY, error))
	{
		if (!error || !*error)
		{
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
						 "Plugin directory escapes its approved root: %s", candidate);
		}
		g_free (candidate);
		return FALSE;
	}

	*canonical_child = candidate;
	return TRUE;
}

gboolean
fabulor_plugin_path_resolve_regular_file (const char *canonical_directory,
												  const char *file_name,
												  char **canonical_file,
												  GError **error)
{
	char *candidate;

	if (canonical_file)
	{
		*canonical_file = NULL;
	}
	if (!canonical_directory || !canonical_file || !fabulor_plugin_path_is_direct_name (file_name))
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
					 "Plugin file name is not a direct child name: %s",
					 file_name ? file_name : "(null)");
		return FALSE;
	}

	candidate = g_build_filename (canonical_directory, file_name, NULL);
	if (!fabulor_plugin_path_is_strict_child (canonical_directory, candidate)
		|| !fabulor_plugin_path_query_trusted_type (candidate, G_FILE_TYPE_REGULAR, error))
	{
		if (!error || !*error)
		{
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
						 "Plugin file escapes its approved directory: %s", candidate);
		}
		g_free (candidate);
		return FALSE;
	}

	*canonical_file = candidate;
	return TRUE;
}
