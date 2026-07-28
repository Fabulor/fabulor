#include <gio/gio.h>
#include <glib/gstdio.h>
#include <string.h>

#include "../theme-archive-reader.h"

static void
remove_test_tree (const char *path)
{
	GDir *dir;
	const char *name;

	if (!g_file_test (path, G_FILE_TEST_IS_DIR))
	{
		g_remove (path);
		return;
	}
	dir = g_dir_open (path, 0, NULL);
	if (dir)
	{
		while ((name = g_dir_read_name (dir)) != NULL)
		{
			char *child = g_build_filename (path, name, NULL);
			remove_test_tree (child);
			g_free (child);
		}
		g_dir_close (dir);
	}
	g_rmdir (path);
}

static char *
test_tar_program (void)
{
#ifdef G_OS_WIN32
	return g_build_filename (g_getenv ("SystemRoot"), "System32", "tar.exe", NULL);
#else
	return g_strdup ("/usr/bin/tar");
#endif
}

static char *
create_test_archive (const char *root, gboolean duplicate, gsize text_size)
{
	char *input = g_build_filename (root, "input", NULL);
	char *first = g_build_filename (input, "first", NULL);
	char *second = g_build_filename (input, "second", NULL);
	char *colors = g_build_filename (first, "colors.conf", NULL);
	char *events = g_build_filename (first, "pevents.conf", NULL);
	char *archive = g_build_filename (root, "theme.hct", NULL);
	char *program = test_tar_program ();
	char *text = g_malloc (text_size + 1);
	char *argv[] = {program, "-cf", archive, "--format", "zip", "-C", input,
		".", NULL};
	GError *error = NULL;
	int status = 0;

	memset (text, 'a', text_size);
	text[text_size] = '\0';
	g_mkdir_with_parents (first, 0700);
	g_assert_true (g_file_set_contents (colors, text, text_size, &error));
	g_assert_no_error (error);
	g_assert_true (g_file_set_contents (events, "events", -1, &error));
	g_assert_no_error (error);
	if (duplicate)
	{
		char *duplicate_colors = g_build_filename (second, "colors.conf", NULL);
		g_mkdir_with_parents (second, 0700);
		g_assert_true (g_file_set_contents (duplicate_colors, "duplicate", -1, &error));
		g_assert_no_error (error);
		g_free (duplicate_colors);
	}
	g_assert_true (g_spawn_sync (NULL, argv, NULL, 0, NULL, NULL, NULL, NULL,
		&status, &error));
	g_assert_no_error (error);
	g_assert_true (g_spawn_check_wait_status (status, &error));
	g_assert_no_error (error);

	g_free (text);
	g_free (program);
	g_free (events);
	g_free (colors);
	g_free (second);
	g_free (first);
	g_free (input);
	return archive;
}

static char *
create_test_gtk4_archive_with_css (const char *root, const char *css_contents,
	const char *dark_css_contents)
{
	char *input = g_build_filename (root, "gtk4-input", NULL);
	char *theme = g_build_filename (input, "Orchis-Test", NULL);
	char *gtk4 = g_build_filename (theme, "gtk-4.0", NULL);
	char *assets = g_build_filename (gtk4, "assets", NULL);
	char *css = g_build_filename (gtk4, "gtk.css", NULL);
	char *dark_css = g_build_filename (gtk4, "gtk-dark.css", NULL);
	char *asset = g_build_filename (assets, "button.png", NULL);
	char *index = g_build_filename (theme, "index.theme", NULL);
	char *archive = g_build_filename (root, "gtk4-theme.tar.xz", NULL);
	char *program = test_tar_program ();
	char *argv[] = {program, "-cJf", archive, "-C", input, "Orchis-Test",
		NULL};
	GError *error = NULL;
	int status = 0;

	g_mkdir_with_parents (assets, 0700);
	g_assert_true (g_file_set_contents (css, css_contents, -1, &error));
	g_assert_no_error (error);
	if (dark_css_contents)
	{
		g_assert_true (g_file_set_contents (dark_css, dark_css_contents, -1,
			&error));
		g_assert_no_error (error);
	}
	g_assert_true (g_file_set_contents (asset, "png", -1, &error));
	g_assert_no_error (error);
	g_assert_true (g_file_set_contents (index,
		"[Desktop Entry]\nName=Orchis Test\n", -1, &error));
	g_assert_no_error (error);
	g_assert_true (g_spawn_sync (NULL, argv, NULL, 0, NULL, NULL, NULL, NULL,
		&status, &error));
	g_assert_no_error (error);
	g_assert_true (g_spawn_check_wait_status (status, &error));
	g_assert_no_error (error);

	g_free (program);
	g_free (index);
	g_free (asset);
	g_free (dark_css);
	g_free (css);
	g_free (assets);
	g_free (gtk4);
	g_free (theme);
	g_free (input);
	return archive;
}

static char *
create_test_gtk4_archive (const char *root)
{
	return create_test_gtk4_archive_with_css (root,
		"button { background-image: url('assets/button.png'); }\n", NULL);
}

static void
test_theme_archive_reads_supported_text (void)
{
	char *root = g_dir_make_tmp ("fabulor-theme-archive-test-XXXXXX", NULL);
	char *archive = create_test_archive (root, FALSE, 7);
	char *contents = NULL;
	GError *error = NULL;

	g_assert_true (fabulor_theme_archive_read_text_file (archive,
		"colors.conf", &contents, &error));
	g_assert_no_error (error);
	g_assert_cmpstr (contents, ==, "aaaaaaa");
	g_free (contents);
	g_free (archive);
	remove_test_tree (root);
	g_free (root);
}

static void
test_theme_archive_rejects_duplicate_text (void)
{
	char *root = g_dir_make_tmp ("fabulor-theme-archive-test-XXXXXX", NULL);
	char *archive = create_test_archive (root, TRUE, 7);
	char *contents = NULL;
	GError *error = NULL;

	g_assert_false (fabulor_theme_archive_read_text_file (archive,
		"colors.conf", &contents, &error));
	g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL);
	g_assert_null (contents);
	g_clear_error (&error);
	g_free (archive);
	remove_test_tree (root);
	g_free (root);
}

static void
test_theme_archive_rejects_oversize_text (void)
{
	char *root = g_dir_make_tmp ("fabulor-theme-archive-test-XXXXXX", NULL);
	char *archive = create_test_archive (root, FALSE, (1024 * 1024) + 1);
	char *contents = NULL;
	GError *error = NULL;

	g_assert_false (fabulor_theme_archive_read_text_file (archive,
		"colors.conf", &contents, &error));
	g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED);
	g_assert_null (contents);
	g_clear_error (&error);
	g_free (archive);
	remove_test_tree (root);
	g_free (root);
}

static void
test_theme_archive_rejects_unsupported_name (void)
{
	char *contents = NULL;
	GError *error = NULL;

	g_assert_false (fabulor_theme_archive_read_text_file ("C:\\missing.hct",
		"gtk.css", &contents, &error));
	g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL);
	g_assert_null (contents);
	g_clear_error (&error);
}

static void
test_theme_archive_discovers_profile_files (void)
{
	char *root = g_dir_make_tmp ("fabulor-theme-discovery-test-XXXXXX", NULL);
	char *themes = g_build_filename (root, "themes", NULL);
	char *blue = g_build_filename (themes, "Blues.hct", NULL);
	char *fire = g_build_filename (themes, "fire.HCT", NULL);
	char *ignored = g_build_filename (themes, "colors.conf", NULL);
	char *nested = g_build_filename (themes, "nested", NULL);
	char *nested_archive = g_build_filename (nested, "hidden.hct", NULL);
	GPtrArray *archives;
	FabulorThemeArchive *first;
	FabulorThemeArchive *second;
	GError *error = NULL;

	g_mkdir_with_parents (nested, 0700);
	g_assert_true (g_file_set_contents (blue, "blue", -1, &error));
	g_assert_no_error (error);
	g_assert_true (g_file_set_contents (fire, "fire", -1, &error));
	g_assert_no_error (error);
	g_assert_true (g_file_set_contents (ignored, "ignored", -1, &error));
	g_assert_no_error (error);
	g_assert_true (g_file_set_contents (nested_archive, "hidden", -1, &error));
	g_assert_no_error (error);

	archives = fabulor_theme_archive_discover (root);
	g_assert_cmpuint (archives->len, ==, 2);
	first = g_ptr_array_index (archives, 0);
	second = g_ptr_array_index (archives, 1);
	g_assert_cmpstr (first->display_name, ==, "Blues");
	g_assert_cmpstr (first->path, ==, blue);
	g_assert_cmpstr (second->display_name, ==, "fire");
	g_assert_cmpstr (second->path, ==, fire);

	g_ptr_array_unref (archives);
	g_free (nested_archive);
	g_free (nested);
	g_free (ignored);
	g_free (fire);
	g_free (blue);
	g_free (themes);
	remove_test_tree (root);
	g_free (root);
}

static void
test_gtk4_theme_archive_imports_contained_tree (void)
{
	char *root = g_dir_make_tmp ("fabulor-gtk4-import-test-XXXXXX", NULL);
	char *config = g_build_filename (root, "profile", NULL);
	char *archive = create_test_gtk4_archive (root);
	char *css = g_build_filename (config, "themes", "Orchis-Test",
		"gtk-4.0", "gtk.css", NULL);
	char *asset = g_build_filename (config, "themes", "Orchis-Test",
		"gtk-4.0", "assets", "button.png", NULL);
	GPtrArray *installed = NULL;
	GError *error = NULL;

	g_assert_true (fabulor_gtk4_theme_archive_import (archive, config,
		&installed, &error));
	g_assert_no_error (error);
	g_assert_nonnull (installed);
	g_assert_cmpuint (installed->len, ==, 1);
	g_assert_cmpstr (g_ptr_array_index (installed, 0), ==, "Orchis-Test");
	g_assert_true (g_file_test (css, G_FILE_TEST_IS_REGULAR));
	g_assert_true (g_file_test (asset, G_FILE_TEST_IS_REGULAR));

	g_ptr_array_unref (installed);
	g_free (asset);
	g_free (css);
	g_free (archive);
	g_free (config);
	remove_test_tree (root);
	g_free (root);
}

static void
test_gtk4_theme_archive_refuses_overwrite (void)
{
	char *root = g_dir_make_tmp ("fabulor-gtk4-collision-test-XXXXXX", NULL);
	char *config = g_build_filename (root, "profile", NULL);
	char *archive = create_test_gtk4_archive (root);
	GPtrArray *installed = NULL;
	GError *error = NULL;

	g_assert_true (fabulor_gtk4_theme_archive_import (archive, config,
		&installed, &error));
	g_assert_no_error (error);
	g_clear_pointer (&installed, g_ptr_array_unref);
	g_assert_false (fabulor_gtk4_theme_archive_import (archive, config,
		&installed, &error));
	g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_EXIST);
	g_assert_null (installed);
	g_clear_error (&error);

	g_free (archive);
	g_free (config);
	remove_test_tree (root);
	g_free (root);
}

static void
test_gtk4_theme_archive_rejects_uncompiled_css (void)
{
	char *root = g_dir_make_tmp ("fabulor-gtk4-source-test-XXXXXX", NULL);
	char *config = g_build_filename (root, "profile", NULL);
	char *archive = create_test_gtk4_archive_with_css (root,
		":root { --blue-1: $blue_1; }\n", NULL);
	char *destination = g_build_filename (config, "themes", "Orchis-Test",
		NULL);
	GPtrArray *installed = NULL;
	GError *error = NULL;

	g_assert_false (fabulor_gtk4_theme_archive_import (archive, config,
		&installed, &error));
	g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL);
	g_assert_nonnull (strstr (error->message, "uncompiled stylesheet"));
	g_assert_null (installed);
	g_assert_false (g_file_test (destination, G_FILE_TEST_EXISTS));
	g_clear_error (&error);

	g_free (destination);
	g_free (archive);
	g_free (config);
	remove_test_tree (root);
	g_free (root);
}

static void
test_gtk4_theme_archive_rejects_define_color_var (void)
{
	char *root = g_dir_make_tmp ("fabulor-gtk4-define-test-XXXXXX", NULL);
	char *config = g_build_filename (root, "profile", NULL);
	char *archive = create_test_gtk4_archive_with_css (root,
		"button { color: red; }\n",
		"@define-color selected_bg_color var(--accent-bg-color);\n");
	GPtrArray *installed = NULL;
	GError *error = NULL;

	g_assert_false (fabulor_gtk4_theme_archive_import (archive, config,
		&installed, &error));
	g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL);
	g_assert_nonnull (strstr (error->message, "unsupported @define-color"));
	g_assert_null (installed);
	g_clear_error (&error);

	g_free (archive);
	g_free (config);
	remove_test_tree (root);
	g_free (root);
}

static void
test_gtk4_theme_archive_imports_external_fixture (void)
{
	const char *archive = g_getenv ("FABULOR_TEST_GTK4_ARCHIVE");
	char *root;
	GPtrArray *installed = NULL;
	GError *error = NULL;

	if (!archive || !archive[0])
	{
		g_test_skip ("No external GTK4 theme archive was supplied.");
		return;
	}
	root = g_dir_make_tmp ("fabulor-gtk4-external-test-XXXXXX", NULL);
	g_assert_true (fabulor_gtk4_theme_archive_import (archive, root,
		&installed, &error));
	g_assert_no_error (error);
	g_assert_nonnull (installed);
	g_assert_cmpuint (installed->len, >, 0);

	g_ptr_array_unref (installed);
	remove_test_tree (root);
	g_free (root);
}

static void
test_gtk4_theme_archive_rejects_external_fixture (void)
{
	const char *archive = g_getenv ("FABULOR_TEST_INVALID_GTK4_ARCHIVE");
	char *root;
	GPtrArray *installed = NULL;
	GError *error = NULL;

	if (!archive || !archive[0])
	{
		g_test_skip ("No invalid external GTK4 theme archive was supplied.");
		return;
	}
	root = g_dir_make_tmp ("fabulor-gtk4-invalid-external-test-XXXXXX", NULL);
	g_assert_false (fabulor_gtk4_theme_archive_import (archive, root,
		&installed, &error));
	g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL);
	g_assert_nonnull (strstr (error->message, "uncompiled stylesheet"));
	g_assert_null (installed);
	g_clear_error (&error);

	remove_test_tree (root);
	g_free (root);
}

static void
test_theme_colors_reads_legacy_palette_in_dark_mode (void)
{
	const char *contents =
		"color_0 = 1111 2222 3333\n"
		"color_256 = aaaa bbbb cccc\n";
	guint16 red = 0;
	guint16 green = 0;
	guint16 blue = 0;

	g_assert_true (fabulor_theme_colors_read_token (contents, 0, TRUE,
		&red, &green, &blue));
	g_assert_cmpuint (red, ==, 0x1111);
	g_assert_cmpuint (green, ==, 0x2222);
	g_assert_cmpuint (blue, ==, 0x3333);
	g_assert_true (fabulor_theme_colors_read_token (contents, 32, TRUE,
		&red, &green, &blue));
	g_assert_cmpuint (red, ==, 0xaaaa);
	g_assert_cmpuint (green, ==, 0xbbbb);
	g_assert_cmpuint (blue, ==, 0xcccc);
}

static void
test_theme_colors_prefers_explicit_dark_palette (void)
{
	const char *contents =
		"color_4 = 1111 2222 3333\n"
		"dark_color_4 = 4444 5555 6666\n";
	guint16 red = 0;
	guint16 green = 0;
	guint16 blue = 0;

	g_assert_true (fabulor_theme_colors_read_token (contents, 4, TRUE,
		&red, &green, &blue));
	g_assert_cmpuint (red, ==, 0x4444);
	g_assert_cmpuint (green, ==, 0x5555);
	g_assert_cmpuint (blue, ==, 0x6666);
	g_assert_true (fabulor_theme_colors_read_token (contents, 4, FALSE,
		&red, &green, &blue));
	g_assert_cmpuint (red, ==, 0x1111);
}

static void
test_theme_colors_rejects_malformed_explicit_value (void)
{
	const char *contents =
		"theme.mode.dark.token.mirc_4 = not-a-colour\n"
		"dark_color_4 = 4444 5555 6666\n";
	guint16 red = 0;
	guint16 green = 0;
	guint16 blue = 0;

	g_assert_cmpint (fabulor_theme_colors_parse_token (contents, 4, TRUE,
		&red, &green, &blue), ==, FABULOR_THEME_COLOR_INVALID);
}

static void
test_theme_colors_rejects_duplicate_value (void)
{
	const char *contents =
		"color_4 = 1111 2222 3333\n"
		"color_4 = 4444 5555 6666\n";
	guint16 red = 0;
	guint16 green = 0;
	guint16 blue = 0;

	g_assert_cmpint (fabulor_theme_colors_parse_token (contents, 4, FALSE,
		&red, &green, &blue), ==, FABULOR_THEME_COLOR_INVALID);
}

static void
test_theme_colors_reports_missing_value (void)
{
	guint16 red = 0;
	guint16 green = 0;
	guint16 blue = 0;

	g_assert_cmpint (fabulor_theme_colors_parse_token ("color_3x = 1 2 3\n",
		3, FALSE, &red, &green, &blue), ==,
		FABULOR_THEME_COLOR_MISSING);
}

void
register_theme_archive_reader_tests (void)
{
	g_test_add_func ("/theme-archive/reads-supported-text",
		test_theme_archive_reads_supported_text);
	g_test_add_func ("/theme-archive/rejects-duplicate-text",
		test_theme_archive_rejects_duplicate_text);
	g_test_add_func ("/theme-archive/rejects-oversize-text",
		test_theme_archive_rejects_oversize_text);
	g_test_add_func ("/theme-archive/rejects-unsupported-name",
		test_theme_archive_rejects_unsupported_name);
	g_test_add_func ("/theme-archive/discovers-profile-files",
		test_theme_archive_discovers_profile_files);
	g_test_add_func ("/theme-archive/gtk4-import-contained-tree",
		test_gtk4_theme_archive_imports_contained_tree);
	g_test_add_func ("/theme-archive/gtk4-refuses-overwrite",
		test_gtk4_theme_archive_refuses_overwrite);
	g_test_add_func ("/theme-archive/gtk4-rejects-uncompiled-css",
		test_gtk4_theme_archive_rejects_uncompiled_css);
	g_test_add_func ("/theme-archive/gtk4-rejects-define-color-var",
		test_gtk4_theme_archive_rejects_define_color_var);
	g_test_add_func ("/theme-archive/gtk4-external-fixture",
		test_gtk4_theme_archive_imports_external_fixture);
	g_test_add_func ("/theme-archive/gtk4-invalid-external-fixture",
		test_gtk4_theme_archive_rejects_external_fixture);
	g_test_add_func ("/theme-archive/colors-read-legacy-in-dark-mode",
		test_theme_colors_reads_legacy_palette_in_dark_mode);
	g_test_add_func ("/theme-archive/colors-prefer-explicit-dark",
		test_theme_colors_prefers_explicit_dark_palette);
	g_test_add_func ("/theme-archive/colors-reject-malformed-explicit",
		test_theme_colors_rejects_malformed_explicit_value);
	g_test_add_func ("/theme-archive/colors-reject-duplicate",
		test_theme_colors_rejects_duplicate_value);
	g_test_add_func ("/theme-archive/colors-report-missing",
		test_theme_colors_reports_missing_value);
}
