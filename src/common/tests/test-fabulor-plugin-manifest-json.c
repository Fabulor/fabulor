#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>

#include "../fabulor-plugin-manifest-json.h"
#include "../fabulor-plugin-enable-policy.h"
#include "../fabulor-plugin-path-policy.h"

#ifdef WIN32
#include <windows.h>
#include <winioctl.h>
#ifndef SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
#define SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE 0x2
#endif

#ifdef WIN32
typedef struct
{
	DWORD reparse_tag;
	WORD reparse_data_length;
	WORD reserved;
	WORD substitute_name_offset;
	WORD substitute_name_length;
	WORD print_name_offset;
	WORD print_name_length;
	WCHAR path_buffer[1];
} FabulorTestJunctionBuffer;
#endif
#endif

static void
manifest_init (FabulorPluginManifest *manifest)
{
	memset (manifest, 0, sizeof (*manifest));
	manifest->dependencies = g_ptr_array_new_with_free_func (g_free);
	manifest->capabilities = g_ptr_array_new_with_free_func (g_free);
}

static void
test_manifest_autoload_policy (void)
{
	g_assert_false (fabulor_plugin_enable_policy_should_autoload (FALSE, NULL, FALSE));
	g_assert_false (fabulor_plugin_enable_policy_should_autoload (FALSE, "", FALSE));
	g_assert_false (fabulor_plugin_enable_policy_should_autoload (FALSE, "0", FALSE));
	g_assert_false (fabulor_plugin_enable_policy_should_autoload (FALSE, "true", FALSE));
	g_assert_true (fabulor_plugin_enable_policy_should_autoload (TRUE, NULL, FALSE));
	g_assert_true (fabulor_plugin_enable_policy_should_autoload (FALSE, "1", FALSE));
	g_assert_false (fabulor_plugin_enable_policy_should_autoload (TRUE, "1", TRUE));
	g_assert_false (fabulor_plugin_enable_policy_should_autoload (FALSE, "1", TRUE));
}

static void
manifest_clear (FabulorPluginManifest *manifest)
{
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
}

static char *
manifest_json (const char *api_version,
			   const char *dependencies,
			   const char *capabilities,
			   const char *description)
{
	return g_strdup_printf (
		"{"
		"\"id\":\"example.test\","
		"\"name\":\"Test\","
		"\"version\":\"1.0.0\","
		"\"language\":\"python\","
		"\"entrypoint\":\"plugin.py\","
		"\"requires_api_version\":%s,"
		"\"dependencies\":%s,"
		"\"capabilities\":%s,"
		"\"description\":\"%s\","
		"\"author\":\"Fabulor\","
		"\"homepage\":\"https://example.test\""
		"}",
		api_version,
		dependencies,
		capabilities,
		description);
}

static void
assert_parse_fails (const char *json, const char *message_fragment)
{
	FabulorPluginManifest manifest;
	GError *error = NULL;

	manifest_init (&manifest);
	g_assert_false (fabulor_plugin_manifest_parse_json (json, strlen (json), &manifest, &error));
	g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL);
	if (message_fragment)
	{
		g_assert_nonnull (strstr (error->message, message_fragment));
	}
	g_clear_error (&error);
	manifest_clear (&manifest);
}

static void
test_valid_manifest (void)
{
	FabulorPluginManifest manifest;
	GError *error = NULL;
	char *json = manifest_json ("1", "[\"example.dependency\"]", "[\"session.read\"]", "A test plugin");

	manifest_init (&manifest);
	g_assert_true (fabulor_plugin_manifest_parse_json (json, strlen (json), &manifest, &error));
	g_assert_no_error (error);
	g_assert_cmpstr (manifest.id, ==, "example.test");
	g_assert_cmpuint (manifest.requires_api_version, ==, 1);
	g_assert_cmpuint (manifest.dependencies->len, ==, 1);
	g_assert_cmpstr (g_ptr_array_index (manifest.dependencies, 0), ==, "example.dependency");
	g_assert_cmpstr (g_ptr_array_index (manifest.capabilities, 0), ==, "session.read");

	manifest_clear (&manifest);
	g_free (json);
}

static void
test_unicode_escapes (void)
{
	FabulorPluginManifest manifest;
	GError *error = NULL;
	char *json = manifest_json ("1", "[]", "[]", "Hello \\uD83D\\uDE00");

	manifest_init (&manifest);
	g_assert_true (fabulor_plugin_manifest_parse_json (json, strlen (json), &manifest, &error));
	g_assert_no_error (error);
	g_assert_true (g_utf8_validate (manifest.description, -1, NULL));
	g_assert_cmpuint (g_utf8_get_char (g_utf8_offset_to_pointer (manifest.description, 6)), ==, 0x1f600);

	manifest_clear (&manifest);
	g_free (json);
}

static void
test_malformed_json (void)
{
	char *json = manifest_json ("1", "[]", "[]", "Test");
	gsize length = strlen (json);
	char *invalid;

	json[length - 1] = '\0';
	assert_parse_fails (json, "unterminated manifest object");
	g_free (json);

	json = manifest_json ("1", "[]", "[]", "Test");
	json[strlen (json) - 1] = '\0';
	json = g_realloc (json, strlen (json) + 3);
	strcat (json, ",}");
	assert_parse_fails (json, "trailing commas");
	g_free (json);

	json = manifest_json ("1", "[]", "[]", "Test");
	invalid = g_strconcat (json, " trailing", NULL);
	assert_parse_fails (invalid, "trailing content");
	g_free (invalid);
	g_free (json);

	assert_parse_fails ("[]", "expected '{'");
}

static void
test_strict_types (void)
{
	char *json = manifest_json ("\"1\"", "[]", "[]", "Test");
	assert_parse_fails (json, "expected an unsigned integer");
	g_free (json);

	json = manifest_json ("1", "[1]", "[]", "Test");
	assert_parse_fails (json, "expected '\"'");
	g_free (json);

	json = manifest_json ("true", "[]", "[]", "Test");
	assert_parse_fails (json, "expected an unsigned integer");
	g_free (json);
}

static void
test_duplicate_and_unknown_fields (void)
{
	char *valid = manifest_json ("1", "[]", "[]", "Test");
	char *json = g_strconcat ("{\"id\":\"duplicate\",", valid + 1, NULL);
	assert_parse_fails (json, "duplicate field 'id'");
	g_free (json);

	json = g_strconcat ("{\"future_field\":true,", valid + 1, NULL);
	assert_parse_fails (json, "unknown field 'future_field'");
	g_free (json);
	g_free (valid);
}

static void
test_missing_field (void)
{
	const char *json =
		"{\"id\":\"example.test\",\"name\":\"Test\",\"version\":\"1.0.0\","
		"\"language\":\"python\",\"entrypoint\":\"plugin.py\",\"requires_api_version\":1,"
		"\"dependencies\":[],\"capabilities\":[],\"description\":\"Test\",\"author\":\"Fabulor\"}";
	assert_parse_fails (json, "missing required field 'homepage'");
}

static void
test_invalid_strings (void)
{
	char *json = manifest_json ("1", "[]", "[]", "Line\\nBreak");
	char *marker;
	assert_parse_fails (json, "control characters");
	g_free (json);

	json = manifest_json ("1", "[]", "[]", "\\uD800x");
	assert_parse_fails (json, "high surrogate");
	g_free (json);

	json = manifest_json ("1", "[]", "[]", "\\q");
	assert_parse_fails (json, "invalid string escape");
	g_free (json);

	json = manifest_json ("1", "[]", "[]", "InvalidX");
	marker = strstr (json, "InvalidX");
	g_assert_nonnull (marker);
	marker[7] = (char) 0xff;
	assert_parse_fails (json, "not valid UTF-8");
	g_free (json);
}

static void
test_duplicate_array_item (void)
{
	char *json = manifest_json ("1", "[\"same\",\"same\"]", "[]", "Test");
	assert_parse_fails (json, "duplicate string");
	g_free (json);
}

static void
test_array_limit (void)
{
	GString *dependencies = g_string_new ("[");
	char *json;
	guint i;

	for (i = 0; i < 65; i++)
	{
		g_string_append_printf (dependencies, "%s\"dependency.%u\"", i == 0 ? "" : ",", i);
	}
	g_string_append_c (dependencies, ']');
	json = manifest_json ("1", dependencies->str, "[]", "Test");
	assert_parse_fails (json, "64-item limit");

	g_free (json);
	g_string_free (dependencies, TRUE);
}

static void
test_string_limit (void)
{
	char *description = g_strnfill (2049, 'a');
	char *json = manifest_json ("1", "[]", "[]", description);
	assert_parse_fails (json, "2048-byte field limit");
	g_free (json);
	g_free (description);
}

static void
test_integer_rules (void)
{
	char *json = manifest_json ("0", "[]", "[]", "Test");
	assert_parse_fails (json, "at least 1");
	g_free (json);

	json = manifest_json ("01", "[]", "[]", "Test");
	assert_parse_fails (json, "leading zeroes");
	g_free (json);

	json = manifest_json ("4294967296", "[]", "[]", "Test");
	assert_parse_fails (json, "exceeds the host range");
	g_free (json);
}

static void
test_input_size_limit (void)
{
	FabulorPluginManifest manifest;
	GError *error = NULL;
	char *json = g_strnfill (FABULOR_PLUGIN_MANIFEST_MAX_BYTES + 1, ' ');

	manifest_init (&manifest);
	g_assert_false (fabulor_plugin_manifest_parse_json (json,
											FABULOR_PLUGIN_MANIFEST_MAX_BYTES + 1,
											&manifest,
											&error));
	g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL);
	g_assert_nonnull (strstr (error->message, "65536 bytes"));
	g_clear_error (&error);
	manifest_clear (&manifest);
	g_free (json);
}

static void
remove_test_tree (const char *tmp_root,
				  const char *link_root,
				  const char *link_child,
				  const char *linked_manifest,
				  const char *manifest,
				  const char *plugin_dir,
				  const char *plugins_root,
				  const char *external_dir)
{
	if (linked_manifest) g_remove (linked_manifest);
	if (link_child) g_rmdir (link_child);
	if (link_root) g_rmdir (link_root);
	if (manifest) g_remove (manifest);
	if (plugin_dir) g_rmdir (plugin_dir);
	if (plugins_root) g_rmdir (plugins_root);
	if (external_dir) g_rmdir (external_dir);
	if (tmp_root) g_rmdir (tmp_root);
}

static void
test_path_policy_valid_tree (void)
{
	GError *error = NULL;
	char *tmp_root = g_dir_make_tmp ("fabulor-manifest-paths-XXXXXX", &error);
	char *plugins_root;
	char *plugin_dir;
	char *manifest;
	char *canonical_root = NULL;
	char *canonical_plugin = NULL;
	char *canonical_manifest = NULL;

	g_assert_no_error (error);
	g_assert_nonnull (tmp_root);
	plugins_root = g_build_filename (tmp_root, "plugins", NULL);
	plugin_dir = g_build_filename (plugins_root, "example.test", NULL);
	manifest = g_build_filename (plugin_dir, "plugin.json", NULL);
	g_assert_cmpint (g_mkdir (plugins_root, 0700), ==, 0);
	g_assert_cmpint (g_mkdir (plugin_dir, 0700), ==, 0);
	g_assert_true (g_file_set_contents (manifest, "{}", 2, &error));
	g_assert_no_error (error);

	g_assert_true (fabulor_plugin_path_validate_root (plugins_root, &canonical_root, &error));
	g_assert_no_error (error);
	g_assert_true (g_path_is_absolute (canonical_root));
	g_assert_true (fabulor_plugin_path_resolve_child_directory (canonical_root,
														 "example.test",
														 &canonical_plugin,
														 &error));
	g_assert_no_error (error);
	g_assert_true (fabulor_plugin_path_resolve_regular_file (canonical_plugin,
													 "plugin.json",
													 &canonical_manifest,
													 &error));
	g_assert_no_error (error);
	g_assert_cmpstr (canonical_manifest, ==, manifest);

	g_free (canonical_manifest);
	g_free (canonical_plugin);
	g_free (canonical_root);
	remove_test_tree (tmp_root, NULL, NULL, NULL, manifest, plugin_dir, plugins_root, NULL);
	g_free (manifest);
	g_free (plugin_dir);
	g_free (plugins_root);
	g_free (tmp_root);
}

static void
test_path_policy_rejects_non_child_names (void)
{
	GError *error = NULL;
	char *resolved = NULL;

	g_assert_false (fabulor_plugin_path_resolve_child_directory ("C:\\approved", "..\\outside", &resolved, &error));
	g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL);
	g_clear_error (&error);
	g_assert_null (resolved);

	g_assert_false (fabulor_plugin_path_resolve_regular_file ("C:\\approved", "C:\\outside\\plugin.json", &resolved, &error));
	g_assert_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL);
	g_clear_error (&error);
	g_assert_null (resolved);
}

static void
test_path_policy_filters_regular_root_files (void)
{
	GError *error = NULL;
	char *tmp_root = g_dir_make_tmp ("fabulor-manifest-filter-XXXXXX", &error);
	char *legacy_plugin;
	char *manifest_plugin;

	g_assert_no_error (error);
	g_assert_nonnull (tmp_root);
	legacy_plugin = g_build_filename (tmp_root, "hcpython3.dll", NULL);
	manifest_plugin = g_build_filename (tmp_root, "example.python.greeter", NULL);
	g_assert_true (g_file_set_contents (legacy_plugin, "legacy", 6, &error));
	g_assert_no_error (error);
	g_assert_cmpint (g_mkdir (manifest_plugin, 0700), ==, 0);

	g_assert_false (fabulor_plugin_path_is_directory_candidate (legacy_plugin));
	g_assert_true (fabulor_plugin_path_is_directory_candidate (manifest_plugin));

	g_remove (legacy_plugin);
	g_rmdir (manifest_plugin);
	g_rmdir (tmp_root);
	g_free (manifest_plugin);
	g_free (legacy_plugin);
	g_free (tmp_root);
}

static void
test_path_policy_resolves_entrypoints (void)
{
	GError *error = NULL;
	char *tmp_root = g_dir_make_tmp ("fabulor-entrypoints-XXXXXX", &error);
	char *plugin_dir;
	char *python_file;
	char *csharp_file;
	char *tcl_file;
	char *text_file;
	char *wrong_type;
	char *resolved = NULL;

	g_assert_no_error (error);
	g_assert_nonnull (tmp_root);
	plugin_dir = g_build_filename (tmp_root, "example.plugin", NULL);
	python_file = g_build_filename (plugin_dir, "plugin.py", NULL);
	csharp_file = g_build_filename (plugin_dir, "plugin.dll", NULL);
	tcl_file = g_build_filename (plugin_dir, "plugin.tcl", NULL);
	text_file = g_build_filename (plugin_dir, "plugin.txt", NULL);
	wrong_type = g_build_filename (plugin_dir, "directory.py", NULL);
	g_assert_cmpint (g_mkdir (plugin_dir, 0700), ==, 0);
	g_assert_true (g_file_set_contents (python_file, "pass\n", 5, &error));
	g_assert_no_error (error);
	g_assert_true (g_file_set_contents (csharp_file, "assembly", 8, &error));
	g_assert_no_error (error);
	g_assert_true (g_file_set_contents (tcl_file, "return\n", 7, &error));
	g_assert_no_error (error);
	g_assert_true (g_file_set_contents (text_file, "text", 4, &error));
	g_assert_no_error (error);
	g_assert_cmpint (g_mkdir (wrong_type, 0700), ==, 0);

	g_assert_true (fabulor_plugin_path_resolve_entrypoint (plugin_dir, "plugin.py", ".py", &resolved, &error));
	g_assert_no_error (error);
	g_assert_cmpstr (resolved, ==, python_file);
	g_clear_pointer (&resolved, g_free);
	g_assert_true (fabulor_plugin_path_resolve_entrypoint (plugin_dir, "plugin.dll", ".dll", &resolved, &error));
	g_assert_no_error (error);
	g_clear_pointer (&resolved, g_free);
	g_assert_true (fabulor_plugin_path_resolve_entrypoint (plugin_dir, "plugin.tcl", ".tcl", &resolved, &error));
	g_assert_no_error (error);
	g_clear_pointer (&resolved, g_free);

	g_assert_false (fabulor_plugin_path_resolve_entrypoint (plugin_dir, "plugin.txt", ".py", &resolved, &error));
	g_assert_nonnull (strstr (error->message, "must use the .py extension"));
	g_clear_error (&error);
	g_assert_false (fabulor_plugin_path_resolve_entrypoint (plugin_dir, "../outside.py", ".py", &resolved, &error));
	g_assert_nonnull (strstr (error->message, "not a direct child name"));
	g_clear_error (&error);
	g_assert_false (fabulor_plugin_path_resolve_entrypoint (plugin_dir, "nested/plugin.py", ".py", &resolved, &error));
	g_assert_nonnull (strstr (error->message, "not a direct child name"));
	g_clear_error (&error);
	g_assert_false (fabulor_plugin_path_resolve_entrypoint (plugin_dir, python_file, ".py", &resolved, &error));
	g_assert_nonnull (strstr (error->message, "not a direct child name"));
	g_clear_error (&error);
	g_assert_false (fabulor_plugin_path_resolve_entrypoint (plugin_dir, "missing.py", ".py", &resolved, &error));
	g_assert_nonnull (error);
	g_clear_error (&error);
	g_assert_false (fabulor_plugin_path_resolve_entrypoint (plugin_dir, "directory.py", ".py", &resolved, &error));
	g_assert_nonnull (strstr (error->message, "wrong file type"));
	g_clear_error (&error);
	g_assert_null (resolved);

	g_rmdir (wrong_type);
	g_remove (text_file);
	g_remove (tcl_file);
	g_remove (csharp_file);
	g_remove (python_file);
	g_rmdir (plugin_dir);
	g_rmdir (tmp_root);
	g_free (wrong_type);
	g_free (text_file);
	g_free (tcl_file);
	g_free (csharp_file);
	g_free (python_file);
	g_free (plugin_dir);
	g_free (tmp_root);
}

static gboolean
create_test_symlink (const char *link_path, const char *target_path, gboolean directory)
{
#ifdef WIN32
	wchar_t *link_utf16 = g_utf8_to_utf16 (link_path, -1, NULL, NULL, NULL);
	wchar_t *target_utf16 = g_utf8_to_utf16 (target_path, -1, NULL, NULL, NULL);
	DWORD flags = (directory ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0)
		| SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
	gboolean created = FALSE;

	if (link_utf16 && target_utf16)
	{
		created = CreateSymbolicLinkW (link_utf16, target_utf16, flags) != 0;
	}
	g_free (target_utf16);
	g_free (link_utf16);
	return created;
#else
	GFile *link = g_file_new_for_path (link_path);
	GError *error = NULL;
	gboolean created = g_file_make_symbolic_link (link, target_path, NULL, &error);
	g_clear_error (&error);
	g_object_unref (link);
	return created;
#endif
}

static gboolean
create_test_directory_link (const char *link_path, const char *target_path)
{
#ifdef WIN32
	wchar_t *link_utf16 = g_utf8_to_utf16 (link_path, -1, NULL, NULL, NULL);
	wchar_t *target_utf16 = g_utf8_to_utf16 (target_path, -1, NULL, NULL, NULL);
	wchar_t *substitute_name;
	gsize substitute_bytes;
	gsize print_bytes;
	gsize buffer_size;
	FabulorTestJunctionBuffer *buffer;
	HANDLE handle;
	DWORD bytes_returned;
	gboolean created = FALSE;

	if (!link_utf16 || !target_utf16)
	{
		g_free (target_utf16);
		g_free (link_utf16);
		return FALSE;
	}
	substitute_name = g_new (wchar_t, wcslen (target_utf16) + 5);
	wcscpy (substitute_name, L"\\??\\");
	wcscat (substitute_name, target_utf16);
	substitute_bytes = wcslen (substitute_name) * sizeof (wchar_t);
	print_bytes = wcslen (target_utf16) * sizeof (wchar_t);
	buffer_size = G_STRUCT_OFFSET (FabulorTestJunctionBuffer, path_buffer)
		+ substitute_bytes + sizeof (wchar_t) + print_bytes + sizeof (wchar_t);
	buffer = g_malloc0 (buffer_size);
	buffer->reparse_tag = IO_REPARSE_TAG_MOUNT_POINT;
	buffer->substitute_name_offset = 0;
	buffer->substitute_name_length = (WORD) substitute_bytes;
	buffer->print_name_offset = (WORD) (substitute_bytes + sizeof (wchar_t));
	buffer->print_name_length = (WORD) print_bytes;
	memcpy (buffer->path_buffer, substitute_name, substitute_bytes);
	memcpy ((char *) buffer->path_buffer + buffer->print_name_offset, target_utf16, print_bytes);
	buffer->reparse_data_length = (WORD) (buffer_size - G_STRUCT_OFFSET (FabulorTestJunctionBuffer, substitute_name_offset));

	if (CreateDirectoryW (link_utf16, NULL))
	{
		handle = CreateFileW (link_utf16,
						  GENERIC_WRITE,
						  0,
						  NULL,
						  OPEN_EXISTING,
						  FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
						  NULL);
		if (handle != INVALID_HANDLE_VALUE)
		{
			created = DeviceIoControl (handle,
								   FSCTL_SET_REPARSE_POINT,
								   buffer,
								   buffer->reparse_data_length + 8,
								   NULL,
								   0,
								   &bytes_returned,
								   NULL) != 0;
			CloseHandle (handle);
		}
		if (!created)
		{
			RemoveDirectoryW (link_utf16);
		}
	}

	g_free (buffer);
	g_free (substitute_name);
	g_free (target_utf16);
	g_free (link_utf16);
	return created;
#else
	return create_test_symlink (link_path, target_path, TRUE);
#endif
}

static void
test_path_policy_rejects_links (void)
{
	GError *error = NULL;
	char *tmp_root = g_dir_make_tmp ("fabulor-manifest-links-XXXXXX", &error);
	char *plugins_root;
	char *external_dir;
	char *link_root;
	char *link_child;
	char *plugin_dir;
	char *manifest;
	char *linked_manifest;
	char *canonical_root = NULL;
	char *canonical_plugin = NULL;
	char *resolved = NULL;
	gboolean directory_links_created;
	gboolean manifest_link_created;

	g_assert_no_error (error);
	g_assert_nonnull (tmp_root);
	plugins_root = g_build_filename (tmp_root, "plugins", NULL);
	external_dir = g_build_filename (tmp_root, "external", NULL);
	link_root = g_build_filename (tmp_root, "linked-root", NULL);
	link_child = g_build_filename (plugins_root, "linked-child", NULL);
	plugin_dir = g_build_filename (plugins_root, "example.test", NULL);
	manifest = g_build_filename (external_dir, "target.json", NULL);
	linked_manifest = g_build_filename (plugin_dir, "plugin.json", NULL);
	g_assert_cmpint (g_mkdir (plugins_root, 0700), ==, 0);
	g_assert_cmpint (g_mkdir (external_dir, 0700), ==, 0);
	g_assert_cmpint (g_mkdir (plugin_dir, 0700), ==, 0);
	g_assert_true (g_file_set_contents (manifest, "{}", 2, &error));
	g_assert_no_error (error);

	directory_links_created = create_test_directory_link (link_root, plugins_root)
		&& create_test_directory_link (link_child, external_dir);
	if (!directory_links_created)
	{
		g_test_skip ("Directory reparse-point creation is unavailable in this environment.");
		remove_test_tree (tmp_root, link_root, link_child, linked_manifest,
						  manifest, plugin_dir, plugins_root, external_dir);
		g_free (linked_manifest);
		g_free (manifest);
		g_free (plugin_dir);
		g_free (link_child);
		g_free (link_root);
		g_free (external_dir);
		g_free (plugins_root);
		g_free (tmp_root);
		return;
	}
	manifest_link_created = create_test_symlink (linked_manifest, manifest, FALSE);

	g_assert_false (fabulor_plugin_path_validate_root (link_root, &resolved, &error));
	g_assert_nonnull (error);
	g_clear_error (&error);
	g_assert_null (resolved);

	g_assert_true (fabulor_plugin_path_validate_root (plugins_root, &canonical_root, &error));
	g_assert_no_error (error);
	g_assert_false (fabulor_plugin_path_resolve_child_directory (canonical_root, "linked-child", &resolved, &error));
	g_assert_nonnull (error);
	g_clear_error (&error);
	g_assert_null (resolved);

	if (manifest_link_created)
	{
		g_assert_true (fabulor_plugin_path_resolve_child_directory (canonical_root, "example.test", &canonical_plugin, &error));
		g_assert_no_error (error);
		g_assert_false (fabulor_plugin_path_resolve_regular_file (canonical_plugin, "plugin.json", &resolved, &error));
		g_assert_nonnull (error);
		g_clear_error (&error);
		g_assert_null (resolved);
		g_assert_false (fabulor_plugin_path_resolve_entrypoint (canonical_plugin, "plugin.json", ".json", &resolved, &error));
		g_assert_nonnull (error);
		g_clear_error (&error);
		g_assert_null (resolved);
	}
	else
	{
		g_test_message ("File symbolic-link creation is unavailable; directory reparse checks completed.");
	}

	remove_test_tree (tmp_root, link_root, link_child, linked_manifest,
					  manifest, plugin_dir, plugins_root, external_dir);
	g_free (canonical_root);
	g_free (canonical_plugin);
	g_free (linked_manifest);
	g_free (manifest);
	g_free (plugin_dir);
	g_free (link_child);
	g_free (link_root);
	g_free (external_dir);
	g_free (plugins_root);
	g_free (tmp_root);
}

void register_theme_archive_reader_tests (void);
void service_message_register_tests (void);
void irc_uri_register_tests (void);
void win32_ipc_register_tests (void);
void ircv3_capability_register_tests (void);

int
main (int argc, char **argv)
{
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/manifest-policy/autoload", test_manifest_autoload_policy);
	g_test_add_func ("/manifest-json/valid", test_valid_manifest);
	g_test_add_func ("/manifest-json/unicode-escapes", test_unicode_escapes);
	g_test_add_func ("/manifest-json/malformed", test_malformed_json);
	g_test_add_func ("/manifest-json/strict-types", test_strict_types);
	g_test_add_func ("/manifest-json/duplicate-and-unknown-fields", test_duplicate_and_unknown_fields);
	g_test_add_func ("/manifest-json/missing-field", test_missing_field);
	g_test_add_func ("/manifest-json/invalid-strings", test_invalid_strings);
	g_test_add_func ("/manifest-json/duplicate-array-item", test_duplicate_array_item);
	g_test_add_func ("/manifest-json/array-limit", test_array_limit);
	g_test_add_func ("/manifest-json/string-limit", test_string_limit);
	g_test_add_func ("/manifest-json/integer-rules", test_integer_rules);
	g_test_add_func ("/manifest-json/input-size-limit", test_input_size_limit);
	g_test_add_func ("/manifest-paths/valid-tree", test_path_policy_valid_tree);
	g_test_add_func ("/manifest-paths/rejects-non-child-names", test_path_policy_rejects_non_child_names);
	g_test_add_func ("/manifest-paths/filters-regular-root-files", test_path_policy_filters_regular_root_files);
	g_test_add_func ("/manifest-paths/resolves-entrypoints", test_path_policy_resolves_entrypoints);
	g_test_add_func ("/manifest-paths/rejects-links", test_path_policy_rejects_links);
	register_theme_archive_reader_tests ();
	service_message_register_tests ();
	irc_uri_register_tests ();
	win32_ipc_register_tests ();
	ircv3_capability_register_tests ();
	return g_test_run ();
}
