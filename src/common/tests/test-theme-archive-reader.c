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
}
