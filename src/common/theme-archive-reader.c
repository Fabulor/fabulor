/* Fabulor
 * Copyright (C) 2026 deepend-tildeclub.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "theme-archive-reader.h"

#include <gio/gio.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>

#ifdef G_OS_WIN32
#include <windows.h>
#endif

#define FABULOR_THEME_ARCHIVE_MAX_BYTES (16 * 1024 * 1024)
#define FABULOR_THEME_ARCHIVE_LIST_MAX_BYTES (1024 * 1024)
#define FABULOR_THEME_ARCHIVE_TEXT_MAX_BYTES (1024 * 1024)
#define FABULOR_THEME_ARCHIVE_MAX_DEPTH 8

void
fabulor_theme_archive_free (FabulorThemeArchive *archive)
{
	if (!archive)
		return;
	g_free (archive->display_name);
	g_free (archive->path);
	g_free (archive);
}

static gboolean
theme_archive_path_is_regular (const char *path)
{
	GFile *file;
	GFileInfo *info;
	GError *error = NULL;
	gboolean regular = FALSE;

	file = g_file_new_for_path (path);
	info = g_file_query_info (file,
		G_FILE_ATTRIBUTE_STANDARD_TYPE ","
		G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK,
		G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);
	if (info)
	{
		regular = g_file_info_get_file_type (info) == G_FILE_TYPE_REGULAR
			&& !g_file_info_get_is_symlink (info);
		g_object_unref (info);
	}
	g_clear_error (&error);
	g_object_unref (file);

#ifdef G_OS_WIN32
	if (regular)
	{
		gunichar2 *wide_path = g_utf8_to_utf16 (path, -1, NULL, NULL, NULL);
		DWORD attributes = wide_path ?
			GetFileAttributesW ((const wchar_t *)wide_path) :
			INVALID_FILE_ATTRIBUTES;

		regular = attributes != INVALID_FILE_ATTRIBUTES
			&& !(attributes & FILE_ATTRIBUTE_DIRECTORY)
			&& !(attributes & FILE_ATTRIBUTE_REPARSE_POINT);
		g_free (wide_path);
	}
#endif
	return regular;
}

static gint
theme_archive_compare (gconstpointer left, gconstpointer right)
{
	const FabulorThemeArchive *a =
		*(FabulorThemeArchive * const *)left;
	const FabulorThemeArchive *b =
		*(FabulorThemeArchive * const *)right;
	char *a_folded = g_utf8_casefold (a->display_name, -1);
	char *b_folded = g_utf8_casefold (b->display_name, -1);
	gint result = g_strcmp0 (a_folded, b_folded);

	g_free (b_folded);
	g_free (a_folded);
	return result;
}

GPtrArray *
fabulor_theme_archive_discover (const char *config_dir)
{
	GPtrArray *archives = g_ptr_array_new_with_free_func (
		(GDestroyNotify)fabulor_theme_archive_free);
	char *root;
	GDir *directory;
	const char *name;

	if (!config_dir || !config_dir[0])
		return archives;
	root = g_build_filename (config_dir, "themes", NULL);
	if (g_mkdir_with_parents (root, 0700) != 0)
	{
		g_free (root);
		return archives;
	}
	directory = g_dir_open (root, 0, NULL);
	if (!directory)
	{
		g_free (root);
		return archives;
	}

	while ((name = g_dir_read_name (directory)) != NULL)
	{
		FabulorThemeArchive *archive;
		char *lower_name;
		char *path;
		gsize name_length;

		lower_name = g_ascii_strdown (name, -1);
		if (!g_str_has_suffix (lower_name, ".hct"))
		{
			g_free (lower_name);
			continue;
		}
		g_free (lower_name);
		path = g_build_filename (root, name, NULL);
		if (!theme_archive_path_is_regular (path))
		{
			g_free (path);
			continue;
		}

		name_length = strlen (name);
		archive = g_new0 (FabulorThemeArchive, 1);
		archive->display_name = g_strndup (name, name_length - 4);
		archive->path = path;
		g_ptr_array_add (archives, archive);
	}
	g_dir_close (directory);
	g_free (root);
	g_ptr_array_sort (archives, theme_archive_compare);
	return archives;
}

static FabulorThemeColorResult
theme_colors_parse_value (const char *contents, const char *key,
	guint16 *red, guint16 *green, guint16 *blue)
{
	const char *line;
	gsize key_length;
	gboolean found = FALSE;
	guint16 parsed_red = 0;
	guint16 parsed_green = 0;
	guint16 parsed_blue = 0;

	if (!contents || !key || !red || !green || !blue)
		return FABULOR_THEME_COLOR_INVALID;
	key_length = strlen (key);
	line = contents;
	while (*line)
	{
		const char *line_end;
		const char *value;

		while (*line == '\n' || *line == '\r')
			line++;
		if (!*line)
			break;
		line_end = strchr (line, '\n');
		if (!line_end)
			line_end = line + strlen (line);
		value = line;
		while (value < line_end && g_ascii_isspace (*value))
			value++;
		if ((gsize)(line_end - value) >= key_length
			&& strncmp (value, key, key_length) == 0
			&& (value + key_length == line_end
				|| value[key_length] == '='
				|| g_ascii_isspace (value[key_length])))
		{
			unsigned int r;
			unsigned int g;
			unsigned int b;
			int consumed = 0;
			const char *tail;

			if (found)
				return FABULOR_THEME_COLOR_INVALID;
			found = TRUE;
			value += key_length;
			while (value < line_end && g_ascii_isspace (*value))
				value++;
			if (value >= line_end || *value != '=')
				return FABULOR_THEME_COLOR_INVALID;
			value++;
			while (value < line_end && g_ascii_isspace (*value))
				value++;
			if (sscanf (value, "%x %x %x%n", &r, &g, &b, &consumed) != 3
				|| r > 0xffff || g > 0xffff || b > 0xffff)
				return FABULOR_THEME_COLOR_INVALID;
			tail = value + consumed;
			while (tail < line_end && g_ascii_isspace (*tail))
				tail++;
			if (tail != line_end)
				return FABULOR_THEME_COLOR_INVALID;
			parsed_red = (guint16)r;
			parsed_green = (guint16)g;
			parsed_blue = (guint16)b;
		}
		line = line_end;
	}
	if (!found)
		return FABULOR_THEME_COLOR_MISSING;
	*red = parsed_red;
	*green = parsed_green;
	*blue = parsed_blue;
	return FABULOR_THEME_COLOR_VALID;
}

FabulorThemeColorResult
fabulor_theme_colors_parse_token (const char *contents, guint token,
	gboolean dark, guint16 *red, guint16 *green, guint16 *blue)
{
	static const char *token_names[] = {
		"mirc_0", "mirc_1", "mirc_2", "mirc_3", "mirc_4", "mirc_5",
		"mirc_6", "mirc_7", "mirc_8", "mirc_9", "mirc_10", "mirc_11",
		"mirc_12", "mirc_13", "mirc_14", "mirc_15", "mirc_16", "mirc_17",
		"mirc_18", "mirc_19", "mirc_20", "mirc_21", "mirc_22", "mirc_23",
		"mirc_24", "mirc_25", "mirc_26", "mirc_27", "mirc_28", "mirc_29",
		"mirc_30", "mirc_31", "selection_foreground",
		"selection_background", "text_foreground", "text_background",
		"marker", "tab_new_data", "tab_highlight", "tab_new_message",
		"tab_away", "spell"
	};
	char key[256];
	guint legacy_key;
	FabulorThemeColorResult result;

	if (token >= G_N_ELEMENTS (token_names))
		return FABULOR_THEME_COLOR_INVALID;
	g_snprintf (key, sizeof key, "theme.mode.%s.token.%s",
		dark ? "dark" : "light", token_names[token]);
	result = theme_colors_parse_value (contents, key, red, green, blue);
	if (result != FABULOR_THEME_COLOR_MISSING)
		return result;

	legacy_key = token < 32 ? token : (token - 32) + 256;
	if (dark)
	{
		g_snprintf (key, sizeof key, "dark_color_%u", legacy_key);
		result = theme_colors_parse_value (contents, key, red, green, blue);
		if (result != FABULOR_THEME_COLOR_MISSING)
			return result;
	}
	g_snprintf (key, sizeof key, "color_%u", legacy_key);
	return theme_colors_parse_value (contents, key, red, green, blue);
}

gboolean
fabulor_theme_colors_read_token (const char *contents, guint token,
	gboolean dark, guint16 *red, guint16 *green, guint16 *blue)
{
	return fabulor_theme_colors_parse_token (contents, token, dark,
		red, green, blue) == FABULOR_THEME_COLOR_VALID;
}

static char *
theme_archive_tar_program (GError **error)
{
#ifdef G_OS_WIN32
	gunichar2 system_dir[MAX_PATH + 1];
	UINT length;
	char *system_dir_utf8;
	char *program;

	length = GetSystemDirectoryW ((wchar_t *)system_dir,
		G_N_ELEMENTS (system_dir));
	if (length == 0 || length >= G_N_ELEMENTS (system_dir))
		return g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOENT,
			"The Windows system directory is unavailable."), NULL;
	system_dir_utf8 = g_utf16_to_utf8 (system_dir, length, NULL, NULL, error);
	if (!system_dir_utf8)
		return NULL;
	program = g_build_filename (system_dir_utf8, "tar.exe", NULL);
	g_free (system_dir_utf8);
#else
	char *program = g_strdup ("/usr/bin/tar");

	if (!g_file_test (program, G_FILE_TEST_IS_EXECUTABLE))
	{
		g_free (program);
		program = g_strdup ("/bin/tar");
	}
#endif

	if (!g_path_is_absolute (program) ||
		!g_file_test (program, G_FILE_TEST_IS_EXECUTABLE))
	{
		g_free (program);
		return g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOENT,
			"The system archive reader is unavailable."), NULL;
	}
	return program;
}

static gboolean
theme_archive_capture_limited (const char * const *argv, gsize limit,
	char **output, GError **error)
{
	GSubprocess *process;
	GInputStream *stream;
	GByteArray *bytes;
	guint8 buffer[8192];
	gssize count;
	gboolean ok = FALSE;

	*output = NULL;
	process = g_subprocess_newv (argv,
		G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_SILENCE,
		error);
	if (!process)
		return FALSE;

	stream = g_subprocess_get_stdout_pipe (process);
	bytes = g_byte_array_sized_new ((guint)MIN (limit, sizeof buffer));
	while ((count = g_input_stream_read (stream, buffer, sizeof buffer,
		NULL, error)) > 0)
	{
		if (bytes->len > limit || (gsize)count > limit - bytes->len)
		{
			g_subprocess_force_exit (process);
			g_subprocess_wait (process, NULL, NULL);
			g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
				"Theme archive output exceeds the permitted size.");
			goto cleanup;
		}
		g_byte_array_append (bytes, buffer, (guint)count);
	}
	if (count < 0)
	{
		g_subprocess_force_exit (process);
		g_subprocess_wait (process, NULL, NULL);
		goto cleanup;
	}
	if (!g_subprocess_wait_check (process, NULL, error))
		goto cleanup;
	if (memchr (bytes->data, '\0', bytes->len) != NULL)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"Theme archive text contains a NUL byte.");
		goto cleanup;
	}

	g_byte_array_append (bytes, (const guint8 *)"", 1);
	*output = (char *)g_byte_array_free (bytes, FALSE);
	bytes = NULL;
	ok = TRUE;

cleanup:
	if (bytes)
		g_byte_array_unref (bytes);
	g_object_unref (process);
	return ok;
}

static gboolean
theme_archive_entry_is_safe (const char *entry, const char *name)
{
	char **parts;
	guint i;
	guint depth = 0;
	const char *base = NULL;
	gboolean safe = FALSE;

	if (!entry || !entry[0] || g_path_is_absolute (entry) ||
		strchr (entry, '\\') != NULL || strchr (entry, ':') != NULL)
		return FALSE;
	for (const char *p = entry; *p; p++)
	{
		if ((guchar)*p < 0x20 || *p == 0x7f)
			return FALSE;
	}

	parts = g_strsplit (entry, "/", -1);
	for (i = 0; parts[i]; i++)
	{
		if (!parts[i][0] || strcmp (parts[i], "..") == 0)
			goto cleanup;
		if (strcmp (parts[i], ".") == 0)
			continue;
		base = parts[i];
		depth++;
		if (depth > FABULOR_THEME_ARCHIVE_MAX_DEPTH)
			goto cleanup;
	}
	safe = base && g_ascii_strcasecmp (base, name) == 0;

cleanup:
	g_strfreev (parts);
	return safe;
}

static char *
theme_archive_find_entry (char *listing, const char *name, GError **error)
{
	char **lines;
	char *match = NULL;
	guint i;

	if (!g_utf8_validate (listing, -1, NULL))
		return g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"Theme archive listing is not valid UTF-8."), NULL;

	lines = g_strsplit (listing, "\n", -1);
	for (i = 0; lines[i]; i++)
	{
		g_strchomp (lines[i]);
		if (!theme_archive_entry_is_safe (lines[i], name))
			continue;
		if (match)
		{
			g_free (match);
			match = NULL;
			g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
				"Theme archive contains duplicate requested files.");
			break;
		}
		match = g_strdup (lines[i]);
	}
	g_strfreev (lines);
	if (!match && (!error || !*error))
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOENT,
			"Requested file was not found in the theme archive.");
	return match;
}

gboolean
fabulor_theme_archive_read_text_file (const char *archive_path,
	const char *name, char **contents, GError **error)
{
	GStatBuf stat_buffer;
	char *program;
	char *listing = NULL;
	char *entry = NULL;
	char *text = NULL;
	const char *list_argv[4];
	const char *read_argv[6];
	gboolean ok = FALSE;

	if (contents)
		*contents = NULL;
	if (!archive_path || !archive_path[0] || !g_path_is_absolute (archive_path))
		return g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"Theme archive path must be absolute."), FALSE;
	if (!name || (g_ascii_strcasecmp (name, "colors.conf") != 0 &&
		g_ascii_strcasecmp (name, "pevents.conf") != 0))
		return g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"Unsupported theme archive file requested."), FALSE;
	if (!contents)
		return g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"No output buffer provided."), FALSE;
	if (g_stat (archive_path, &stat_buffer) != 0 ||
		!g_file_test (archive_path, G_FILE_TEST_IS_REGULAR))
		return g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOENT,
			"Theme archive is unavailable."), FALSE;
	if (stat_buffer.st_size < 0 ||
		(guint64)stat_buffer.st_size > FABULOR_THEME_ARCHIVE_MAX_BYTES)
		return g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
			"Theme archive exceeds the permitted size."), FALSE;

	program = theme_archive_tar_program (error);
	if (!program)
		return FALSE;
	list_argv[0] = program;
	list_argv[1] = "-tf";
	list_argv[2] = archive_path;
	list_argv[3] = NULL;
	if (!theme_archive_capture_limited (list_argv,
		FABULOR_THEME_ARCHIVE_LIST_MAX_BYTES, &listing, error))
		goto cleanup;
	entry = theme_archive_find_entry (listing, name, error);
	if (!entry)
		goto cleanup;

	read_argv[0] = program;
	read_argv[1] = "-xOf";
	read_argv[2] = archive_path;
	read_argv[3] = "--";
	read_argv[4] = entry;
	read_argv[5] = NULL;
	if (!theme_archive_capture_limited (read_argv,
		FABULOR_THEME_ARCHIVE_TEXT_MAX_BYTES, &text, error))
		goto cleanup;

	*contents = text;
	text = NULL;
	ok = TRUE;

cleanup:
	g_free (text);
	g_free (entry);
	g_free (listing);
	g_free (program);
	return ok;
}
