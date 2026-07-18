#include "gtk4-theme-discovery.h"

#include <gio/gio.h>
#include <glib/gstdio.h>
#include <string.h>

static char *
theme_canonical_path (const char *path)
{
	char *canonical;

	if (!path || !path[0])
		return NULL;
	canonical = g_canonicalize_filename (path, NULL);
#ifdef G_OS_WIN32
	{
		char *folded = g_utf8_casefold (canonical, -1);
		g_free (canonical);
		canonical = folded;
	}
#endif
	return canonical;
}

static gboolean
theme_is_regular_file (const char *path)
{
	GFile *file;
	GFileInfo *info;
	gboolean regular;

	file = g_file_new_for_path (path);
	info = g_file_query_info (file,
		G_FILE_ATTRIBUTE_STANDARD_TYPE "," G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK,
		G_FILE_QUERY_INFO_NONE, NULL, NULL);
	g_object_unref (file);
	if (!info)
		return FALSE;
	regular = g_file_info_get_file_type (info) == G_FILE_TYPE_REGULAR;
	g_object_unref (info);
	return regular;
}

static char *
theme_display_name (const char *root)
{
	char *index_path = g_build_filename (root, "index.theme", NULL);
	GKeyFile *keyfile = g_key_file_new ();
	char *name = NULL;

	if (theme_is_regular_file (index_path) &&
		g_key_file_load_from_file (keyfile, index_path, G_KEY_FILE_NONE, NULL))
	{
		name = g_key_file_get_locale_string (keyfile, "Desktop Entry", "Name",
			NULL, NULL);
		if (!name)
			name = g_key_file_get_locale_string (keyfile, "X-GNOME-Metatheme",
				"Name", NULL, NULL);
	}
	if (!name || !name[0])
	{
		g_free (name);
		name = g_path_get_basename (root);
	}
	g_key_file_unref (keyfile);
	g_free (index_path);
	return name;
}

static char *
theme_thumbnail_path (const char *root)
{
	static const char *const names[] = {
		"thumbnail.png", "preview.png", "screenshot.png", NULL
	};
	guint i;

	for (i = 0; names[i]; i++)
	{
		char *candidate = g_build_filename (root, names[i], NULL);
		if (theme_is_regular_file (candidate))
			return candidate;
		g_free (candidate);
		candidate = g_build_filename (root, "gtk-4.0", names[i], NULL);
		if (theme_is_regular_file (candidate))
			return candidate;
		g_free (candidate);
	}
	return NULL;
}

void
fabulor_gtk4_theme_free (FabulorGtk4Theme *theme)
{
	if (!theme)
		return;
	g_free (theme->id);
	g_free (theme->display_name);
	g_free (theme->path);
	g_free (theme->css_path);
	g_free (theme->dark_css_path);
	g_free (theme->thumbnail_path);
	g_free (theme);
}

char *
fabulor_gtk4_theme_profile_dir (const char *config_dir)
{
	if (!config_dir || !config_dir[0])
		return NULL;
	return g_build_filename (config_dir, "themes", NULL);
}

static void
theme_discover_directory (GPtrArray *themes, GHashTable *seen,
	const char *base_dir, FabulorGtk4ThemeSource source)
{
	GDir *dir;
	const char *name;

	if (!base_dir || !g_file_test (base_dir, G_FILE_TEST_IS_DIR))
		return;
	dir = g_dir_open (base_dir, 0, NULL);
	if (!dir)
		return;

	while ((name = g_dir_read_name (dir)) != NULL)
	{
		FabulorGtk4Theme *theme;
		char *root;
		char *canonical;
		char *digest_input;
		char *digest;
		char *dark_path;

		if (name[0] == '.')
			continue;
		root = g_build_filename (base_dir, name, NULL);
		if (!g_file_test (root, G_FILE_TEST_IS_DIR))
		{
			g_free (root);
			continue;
		}
		canonical = theme_canonical_path (root);
		if (!canonical || g_hash_table_contains (seen, canonical))
		{
			g_free (canonical);
			g_free (root);
			continue;
		}

		theme = g_new0 (FabulorGtk4Theme, 1);
		theme->css_path = g_build_filename (root, "gtk-4.0", "gtk.css", NULL);
		if (!theme_is_regular_file (theme->css_path))
		{
			fabulor_gtk4_theme_free (theme);
			g_free (canonical);
			g_free (root);
			continue;
		}
		g_hash_table_add (seen, canonical);
		theme->path = root;
		theme->source = source;
		theme->display_name = theme_display_name (root);
		theme->thumbnail_path = theme_thumbnail_path (root);
		dark_path = g_build_filename (root, "gtk-4.0", "gtk-dark.css", NULL);
		if (theme_is_regular_file (dark_path))
			theme->dark_css_path = dark_path;
		else
			g_free (dark_path);
		digest_input = g_strdup_printf ("%u:%s", (guint) source, canonical);
		digest = g_compute_checksum_for_string (G_CHECKSUM_SHA256, digest_input, -1);
		theme->id = g_strdup_printf ("%s:%s",
			source == FABULOR_GTK4_THEME_SOURCE_PROFILE ? "profile" : "desktop",
			digest);
		g_free (digest);
		g_free (digest_input);
		g_ptr_array_add (themes, theme);
	}
	g_dir_close (dir);
}

static gint
theme_compare (gconstpointer left, gconstpointer right)
{
	const FabulorGtk4Theme *a = *(FabulorGtk4Theme *const *) left;
	const FabulorGtk4Theme *b = *(FabulorGtk4Theme *const *) right;
	gint result = g_utf8_collate (a->display_name, b->display_name);

	if (result != 0)
		return result;
	if (a->source != b->source)
		return a->source == FABULOR_GTK4_THEME_SOURCE_PROFILE ? -1 : 1;
	return g_strcmp0 (a->path, b->path);
}

GPtrArray *
fabulor_gtk4_theme_discover_roots (const char *profile_root,
	const char *const *desktop_roots)
{
	GPtrArray *themes = g_ptr_array_new_with_free_func (
		(GDestroyNotify) fabulor_gtk4_theme_free);
	GHashTable *seen = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	guint i;

	theme_discover_directory (themes, seen, profile_root,
		FABULOR_GTK4_THEME_SOURCE_PROFILE);
	for (i = 0; desktop_roots && desktop_roots[i]; i++)
		theme_discover_directory (themes, seen, desktop_roots[i],
			FABULOR_GTK4_THEME_SOURCE_DESKTOP);
	g_ptr_array_sort (themes, theme_compare);
	g_hash_table_destroy (seen);
	return themes;
}

static void
theme_add_desktop_root (GPtrArray *roots, GHashTable *seen, const char *path)
{
	char *canonical;

	if (!path || !path[0])
		return;
	canonical = theme_canonical_path (path);
	if (!canonical || g_hash_table_contains (seen, canonical))
	{
		g_free (canonical);
		return;
	}
	g_hash_table_add (seen, canonical);
	g_ptr_array_add (roots, g_strdup (path));
}

GPtrArray *
fabulor_gtk4_theme_discover (const char *config_dir)
{
	GPtrArray *roots = g_ptr_array_new_with_free_func (g_free);
	GHashTable *seen = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	const char *const *system_dirs = g_get_system_data_dirs ();
	const char *prefix = g_getenv ("GTK_DATA_PREFIX");
	char *profile_root = fabulor_gtk4_theme_profile_dir (config_dir);
	char *path;
	GPtrArray *themes;
	guint i;

	path = g_build_filename (g_get_home_dir (), ".themes", NULL);
	theme_add_desktop_root (roots, seen, path);
	g_free (path);
	path = g_build_filename (g_get_user_data_dir (), "themes", NULL);
	theme_add_desktop_root (roots, seen, path);
	g_free (path);
	for (i = 0; system_dirs && system_dirs[i]; i++)
	{
		path = g_build_filename (system_dirs[i], "themes", NULL);
		theme_add_desktop_root (roots, seen, path);
		g_free (path);
	}
	if (prefix && prefix[0])
	{
		path = g_build_filename (prefix, "share", "themes", NULL);
		theme_add_desktop_root (roots, seen, path);
		g_free (path);
	}
	g_ptr_array_add (roots, NULL);
	if (profile_root)
		g_mkdir_with_parents (profile_root, 0700);
	themes = fabulor_gtk4_theme_discover_roots (profile_root,
		(const char *const *) roots->pdata);
	g_ptr_array_unref (roots);
	g_hash_table_destroy (seen);
	g_free (profile_root);
	return themes;
}
