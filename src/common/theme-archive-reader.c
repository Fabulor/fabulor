/* Fabulor
 * Copyright (C) 2026 deepend-tildeclub.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "theme-archive-reader.h"

#include <errno.h>
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
#define FABULOR_GTK4_ARCHIVE_MAX_BYTES (64 * 1024 * 1024)
#define FABULOR_GTK4_ARCHIVE_LIST_MAX_BYTES (8 * 1024 * 1024)
#define FABULOR_GTK4_ARCHIVE_MAX_ENTRIES 10000
#define FABULOR_GTK4_ARCHIVE_MAX_THEMES 32
#define FABULOR_GTK4_ARCHIVE_MAX_FILE_BYTES (16 * 1024 * 1024)
#define FABULOR_GTK4_ARCHIVE_MAX_OUTPUT_BYTES (128 * 1024 * 1024)
#define FABULOR_GTK4_ARCHIVE_MAX_DEPTH 16

typedef struct
{
	char *name;
	char type;
	guint64 size;
} FabulorGtk4ArchiveEntry;

#ifdef G_OS_WIN32
static void
theme_archive_win32_append_char (GArray *command, gunichar2 value)
{
	g_array_append_val (command, value);
}

static gboolean
theme_archive_win32_append_argument (GArray *command, const char *argument,
	GError **error)
{
	gunichar2 *wide;
	const gunichar2 *cursor;
	gsize backslashes = 0;
	gunichar2 quote = L'"';

	wide = g_utf8_to_utf16 (argument, -1, NULL, NULL, error);
	if (!wide)
		return FALSE;
	theme_archive_win32_append_char (command, quote);
	for (cursor = wide; *cursor; cursor++)
	{
		if (*cursor == L'\\')
		{
			backslashes++;
			continue;
		}
		if (*cursor == L'"')
		{
			for (gsize i = 0; i < (backslashes * 2) + 1; i++)
				theme_archive_win32_append_char (command, L'\\');
			theme_archive_win32_append_char (command, *cursor);
		}
		else
		{
			for (gsize i = 0; i < backslashes; i++)
				theme_archive_win32_append_char (command, L'\\');
			theme_archive_win32_append_char (command, *cursor);
		}
		backslashes = 0;
	}
	for (gsize i = 0; i < backslashes * 2; i++)
		theme_archive_win32_append_char (command, L'\\');
	theme_archive_win32_append_char (command, quote);
	g_free (wide);
	return TRUE;
}

static gboolean
theme_archive_win32_start_hidden (const char * const *argv,
	gboolean capture_stdout, HANDLE *process_out, HANDLE *stdout_out,
	GError **error)
{
	SECURITY_ATTRIBUTES security = {
		sizeof (SECURITY_ATTRIBUTES), NULL, TRUE
	};
	STARTUPINFOW startup = {0};
	PROCESS_INFORMATION process = {0};
	GArray *command;
	gunichar2 *program = NULL;
	HANDLE stdout_read = INVALID_HANDLE_VALUE;
	HANDLE stdout_write = INVALID_HANDLE_VALUE;
	HANDLE null_input = INVALID_HANDLE_VALUE;
	HANDLE null_output = INVALID_HANDLE_VALUE;
	DWORD win32_error = ERROR_SUCCESS;
	gboolean started = FALSE;

	*process_out = INVALID_HANDLE_VALUE;
	*stdout_out = INVALID_HANDLE_VALUE;
	command = g_array_new (FALSE, FALSE, sizeof (gunichar2));
	for (guint i = 0; argv[i]; i++)
	{
		gunichar2 separator = L' ';

		if (i > 0)
			theme_archive_win32_append_char (command, separator);
		if (!theme_archive_win32_append_argument (command, argv[i], error))
			goto cleanup;
	}
	theme_archive_win32_append_char (command, (gunichar2)0);
	program = g_utf8_to_utf16 (argv[0], -1, NULL, NULL, error);
	if (!program)
		goto cleanup;
	if (capture_stdout &&
		(!CreatePipe (&stdout_read, &stdout_write, &security, 0) ||
		 !SetHandleInformation (stdout_read, HANDLE_FLAG_INHERIT, 0)))
	{
		win32_error = GetLastError ();
		goto cleanup;
	}
	null_input = CreateFileW (L"NUL", GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, &security, OPEN_EXISTING, 0, NULL);
	null_output = CreateFileW (L"NUL", GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, &security, OPEN_EXISTING, 0, NULL);
	if (null_input == INVALID_HANDLE_VALUE ||
		null_output == INVALID_HANDLE_VALUE)
	{
		win32_error = GetLastError ();
		goto cleanup;
	}

	startup.cb = sizeof startup;
	startup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	startup.wShowWindow = SW_HIDE;
	startup.hStdInput = null_input;
	startup.hStdOutput = capture_stdout ? stdout_write : null_output;
	startup.hStdError = null_output;
	if (!CreateProcessW ((const wchar_t *)program,
		(wchar_t *)command->data, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL,
		NULL, &startup, &process))
	{
		win32_error = GetLastError ();
		goto cleanup;
	}
	CloseHandle (process.hThread);
	process.hThread = NULL;
	*process_out = process.hProcess;
	process.hProcess = NULL;
	if (capture_stdout)
	{
		CloseHandle (stdout_write);
		stdout_write = INVALID_HANDLE_VALUE;
		*stdout_out = stdout_read;
		stdout_read = INVALID_HANDLE_VALUE;
	}
	started = TRUE;

cleanup:
	if (process.hThread)
		CloseHandle (process.hThread);
	if (process.hProcess)
		CloseHandle (process.hProcess);
	if (stdout_write != INVALID_HANDLE_VALUE)
		CloseHandle (stdout_write);
	if (stdout_read != INVALID_HANDLE_VALUE)
		CloseHandle (stdout_read);
	if (null_output != INVALID_HANDLE_VALUE)
		CloseHandle (null_output);
	if (null_input != INVALID_HANDLE_VALUE)
		CloseHandle (null_input);
	g_free (program);
	g_array_unref (command);
	if (!started && win32_error != ERROR_SUCCESS && (!error || !*error))
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
			"Could not start the theme archive tool (Win32 error %lu).",
			(unsigned long)win32_error);
	return started;
}

static gboolean
theme_archive_win32_wait_success (HANDLE process, GError **error)
{
	DWORD status;
	DWORD exit_code;

	status = WaitForSingleObject (process, INFINITE);
	if (status != WAIT_OBJECT_0 || !GetExitCodeProcess (process, &exit_code))
	{
		DWORD win32_error = GetLastError ();

		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
			"Theme archive tool wait failed (Win32 error %lu).",
			(unsigned long)win32_error);
		return FALSE;
	}
	if (exit_code != 0)
	{
		g_set_error (error, G_SPAWN_EXIT_ERROR, (gint)exit_code,
			"Theme archive tool exited with status %lu.",
			(unsigned long)exit_code);
		return FALSE;
	}
	return TRUE;
}
#endif

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

static gboolean
theme_archive_path_is_directory (const char *path)
{
	GFile *file;
	GFileInfo *info;
	GError *error = NULL;
	gboolean directory = FALSE;

	file = g_file_new_for_path (path);
	info = g_file_query_info (file,
		G_FILE_ATTRIBUTE_STANDARD_TYPE ","
		G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK,
		G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);
	if (info)
	{
		directory = g_file_info_get_file_type (info) == G_FILE_TYPE_DIRECTORY
			&& !g_file_info_get_is_symlink (info);
		g_object_unref (info);
	}
	g_clear_error (&error);
	g_object_unref (file);

#ifdef G_OS_WIN32
	if (directory)
	{
		gunichar2 *wide_path = g_utf8_to_utf16 (path, -1, NULL, NULL, NULL);
		DWORD attributes = wide_path ?
			GetFileAttributesW ((const wchar_t *)wide_path) :
			INVALID_FILE_ATTRIBUTES;

		directory = attributes != INVALID_FILE_ATTRIBUTES
			&& (attributes & FILE_ATTRIBUTE_DIRECTORY)
			&& !(attributes & FILE_ATTRIBUTE_REPARSE_POINT);
		g_free (wide_path);
	}
#endif
	return directory;
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
#ifdef G_OS_WIN32
	GByteArray *bytes;
	HANDLE process;
	HANDLE stdout_read;
	guint8 buffer[8192];
	DWORD count;
	gboolean ok = FALSE;

	*output = NULL;
	if (!theme_archive_win32_start_hidden (argv, TRUE, &process,
		&stdout_read, error))
		return FALSE;
	bytes = g_byte_array_sized_new ((guint)MIN (limit, sizeof buffer));
	while (ReadFile (stdout_read, buffer, sizeof buffer, &count, NULL))
	{
		if (bytes->len > limit || (gsize)count > limit - bytes->len)
		{
			TerminateProcess (process, 1);
			WaitForSingleObject (process, INFINITE);
			g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
				"Theme archive output exceeds the permitted size.");
			goto cleanup;
		}
		g_byte_array_append (bytes, buffer, count);
	}
	if (GetLastError () != ERROR_BROKEN_PIPE)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
			"Could not read theme archive output (Win32 error %lu).",
			(unsigned long)GetLastError ());
		goto cleanup;
	}
	if (!theme_archive_win32_wait_success (process, error))
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
	CloseHandle (stdout_read);
	CloseHandle (process);
	return ok;
#else
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
#endif
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

static void
gtk4_archive_entry_free (FabulorGtk4ArchiveEntry *entry)
{
	if (!entry)
		return;
	g_free (entry->name);
	g_free (entry);
}

static gboolean
gtk4_archive_name_is_supported (const char *path)
{
	char *lower;
	gboolean supported;

	if (!path)
		return FALSE;
	lower = g_ascii_strdown (path, -1);
	supported = g_str_has_suffix (lower, ".tar") ||
		g_str_has_suffix (lower, ".tar.gz") ||
		g_str_has_suffix (lower, ".tgz") ||
		g_str_has_suffix (lower, ".tar.xz") ||
		g_str_has_suffix (lower, ".txz") ||
		g_str_has_suffix (lower, ".zip");
	g_free (lower);
	return supported;
}

static gboolean
gtk4_archive_component_is_safe (const char *component, gsize length)
{
	gsize i;

	if (length == 0 || (length == 1 && component[0] == '.') ||
		(length == 2 && component[0] == '.' && component[1] == '.'))
		return FALSE;
	if (component[length - 1] == '.' || component[length - 1] == ' ')
		return FALSE;
	for (i = 0; i < length; i++)
	{
		guchar c = (guchar)component[i];

		if (c < 0x20 || c == 0x7f || c == '<' || c == '>' || c == ':' ||
			c == '"' || c == '\\' || c == '|' || c == '?' || c == '*')
			return FALSE;
	}
	return TRUE;
}

static gboolean
gtk4_archive_entry_name_is_safe (const char *name)
{
	const char *component;
	const char *cursor;
	guint depth = 0;
	gsize length;

	if (!name || !name[0] || name[0] == '/' ||
		strlen (name) > 1024 || !g_utf8_validate (name, -1, NULL))
		return FALSE;
	component = name;
	for (cursor = name; ; cursor++)
	{
		if (*cursor != '/' && *cursor != '\0')
			continue;
		length = (gsize)(cursor - component);
		if (*cursor == '\0' && length == 0 && cursor > name)
			break;
		if (!gtk4_archive_component_is_safe (component, length))
			return FALSE;
		depth++;
		if (depth > FABULOR_GTK4_ARCHIVE_MAX_DEPTH)
			return FALSE;
		if (*cursor == '\0')
			break;
		component = cursor + 1;
	}
	return depth > 0;
}

static gboolean
gtk4_archive_parse_verbose_line (const char *line, const char *name,
	char *type, guint64 *size)
{
	char *needle;
	char *position;
	char *prefix;
	char **fields;
	char *end = NULL;
	guint64 parsed_size;
	const char *size_field = NULL;
	guint field_index = 0;
	gboolean valid = FALSE;

	if (!line || !line[0] || !name || !name[0])
		return FALSE;
	needle = g_strdup_printf (" %s", name);
	position = g_strrstr (line, needle);
	if (!position)
		goto cleanup;
	if (position[strlen (needle)] != '\0' &&
		!g_str_has_prefix (position + strlen (needle), " -> ") &&
		!g_str_has_prefix (position + strlen (needle), " link to "))
		goto cleanup;

	prefix = g_strndup (line, (gsize)(position - line));
	fields = g_strsplit_set (prefix, " \t", -1);
	for (guint i = 0; fields[i]; i++)
	{
		if (fields[i][0])
		{
			if (field_index == 4)
				size_field = fields[i];
			field_index++;
		}
	}
	if (!size_field || !line[0])
		goto fields_cleanup;
	parsed_size = g_ascii_strtoull (size_field, &end, 10);
	if (!end || *end != '\0')
		goto fields_cleanup;
	*type = line[0];
	*size = parsed_size;
	valid = TRUE;

fields_cleanup:
	g_strfreev (fields);
	g_free (prefix);
cleanup:
	g_free (needle);
	return valid;
}

static GPtrArray *
gtk4_archive_inventory (const char *program, const char *archive_path,
	GError **error)
{
	const char *list_argv[] = {program, "-tf", archive_path, NULL};
	const char *verbose_argv[] = {
		program, "--numeric-owner", "-tvf", archive_path, NULL
	};
	GPtrArray *entries = NULL;
	GHashTable *seen = NULL;
	char *listing = NULL;
	char *verbose = NULL;
	char **names = NULL;
	char **details = NULL;
	guint name_count = 0;
	guint detail_count = 0;
	guint i;

	if (!theme_archive_capture_limited (list_argv,
		FABULOR_GTK4_ARCHIVE_LIST_MAX_BYTES, &listing, error) ||
		!theme_archive_capture_limited (verbose_argv,
		FABULOR_GTK4_ARCHIVE_LIST_MAX_BYTES, &verbose, error))
		goto cleanup;
	if (!g_utf8_validate (listing, -1, NULL) ||
		!g_utf8_validate (verbose, -1, NULL))
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"GTK4 theme archive listing is not valid UTF-8.");
		goto cleanup;
	}

	names = g_strsplit (listing, "\n", -1);
	details = g_strsplit (verbose, "\n", -1);
	while (names[name_count])
	{
		g_strchomp (names[name_count]);
		if (!names[name_count][0])
			break;
		name_count++;
	}
	while (details[detail_count])
	{
		g_strchomp (details[detail_count]);
		if (!details[detail_count][0])
			break;
		detail_count++;
	}
	if (name_count == 0 || name_count != detail_count ||
		name_count > FABULOR_GTK4_ARCHIVE_MAX_ENTRIES)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"GTK4 theme archive has an invalid or excessive entry listing.");
		goto cleanup;
	}

	entries = g_ptr_array_new_with_free_func (
		(GDestroyNotify)gtk4_archive_entry_free);
	seen = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	for (i = 0; i < name_count; i++)
	{
		FabulorGtk4ArchiveEntry *entry;
		char *identity;

		if (!gtk4_archive_entry_name_is_safe (names[i]))
		{
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
				"GTK4 theme archive contains an unsafe path: %s", names[i]);
			goto cleanup;
		}
#ifdef G_OS_WIN32
		identity = g_utf8_casefold (names[i], -1);
#else
		identity = g_strdup (names[i]);
#endif
		if (g_hash_table_contains (seen, identity))
		{
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
				"GTK4 theme archive contains a duplicate path: %s", names[i]);
			g_free (identity);
			goto cleanup;
		}
		g_hash_table_add (seen, identity);

		entry = g_new0 (FabulorGtk4ArchiveEntry, 1);
		entry->name = g_strdup (names[i]);
		if (!gtk4_archive_parse_verbose_line (details[i], names[i],
			&entry->type, &entry->size))
		{
			gtk4_archive_entry_free (entry);
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
				"GTK4 theme archive metadata is invalid for: %s", names[i]);
			goto cleanup;
		}
		g_ptr_array_add (entries, entry);
	}

cleanup:
	if (seen)
		g_hash_table_destroy (seen);
	g_strfreev (details);
	g_strfreev (names);
	g_free (verbose);
	g_free (listing);
	if (error && *error)
		g_clear_pointer (&entries, g_ptr_array_unref);
	return entries;
}

static gboolean
gtk4_archive_entry_is_file (const FabulorGtk4ArchiveEntry *entry)
{
	return entry && (entry->type == '-' || entry->type == '0');
}

static FabulorGtk4ArchiveEntry *
gtk4_archive_find_entry (const GPtrArray *entries, const char *name)
{
	guint i;

	for (i = 0; entries && i < entries->len; i++)
	{
		FabulorGtk4ArchiveEntry *entry = g_ptr_array_index (entries, i);

		if (strcmp (entry->name, name) == 0)
			return entry;
	}
	return NULL;
}

static GPtrArray *
gtk4_archive_find_theme_roots (const GPtrArray *entries, GError **error)
{
	static const char suffix[] = "/gtk-4.0/gtk.css";
	GPtrArray *roots = g_ptr_array_new_with_free_func (g_free);
	GHashTable *seen = g_hash_table_new_full (g_str_hash, g_str_equal,
		g_free, NULL);
	guint i;

	for (i = 0; entries && i < entries->len; i++)
	{
		FabulorGtk4ArchiveEntry *entry = g_ptr_array_index (entries, i);
		gsize length;
		gsize root_length;
		char *root;
		char *identity;

		if (!gtk4_archive_entry_is_file (entry) ||
			!g_str_has_suffix (entry->name, suffix))
			continue;
		length = strlen (entry->name);
		root_length = length - (sizeof suffix - 1);
		if (root_length == 0 ||
			memchr (entry->name, '/', root_length) != NULL)
			continue;
		root = g_strndup (entry->name, root_length);
#ifdef G_OS_WIN32
		identity = g_utf8_casefold (root, -1);
#else
		identity = g_strdup (root);
#endif
		if (!g_hash_table_contains (seen, identity))
		{
			g_hash_table_add (seen, identity);
			g_ptr_array_add (roots, root);
		}
		else
		{
			g_free (identity);
			g_free (root);
		}
	}
	g_hash_table_destroy (seen);
	if (roots->len == 0 || roots->len > FABULOR_GTK4_ARCHIVE_MAX_THEMES)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"Archive does not contain a supported number of GTK4 themes.");
		g_ptr_array_unref (roots);
		return NULL;
	}
	return roots;
}

static gboolean
gtk4_archive_validate_theme (const GPtrArray *entries, const char *root,
	GError **error)
{
	char *gtk_root = g_strdup_printf ("%s/gtk-4.0", root);
	char *gtk_prefix = g_strdup_printf ("%s/", gtk_root);
	FabulorGtk4ArchiveEntry *directory = gtk4_archive_find_entry (entries,
		gtk_root);
	guint64 total = 0;
	guint selected = 0;
	guint i;
	gboolean valid = FALSE;

	if (!directory)
	{
		char *with_slash = g_strconcat (gtk_root, "/", NULL);
		directory = gtk4_archive_find_entry (entries, with_slash);
		g_free (with_slash);
	}
	if (!directory || directory->type != 'd')
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"GTK4 theme '%s' has no ordinary gtk-4.0 directory.", root);
		goto cleanup;
	}

	for (i = 0; i < entries->len; i++)
	{
		FabulorGtk4ArchiveEntry *entry = g_ptr_array_index (entries, i);

		if (!g_str_has_prefix (entry->name, gtk_prefix))
			continue;
		if (entry->type != 'd' && !gtk4_archive_entry_is_file (entry))
		{
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
				"GTK4 theme '%s' contains a link or unsupported entry: %s",
				root, entry->name);
			goto cleanup;
		}
		if (entry->size > FABULOR_GTK4_ARCHIVE_MAX_FILE_BYTES ||
			total > FABULOR_GTK4_ARCHIVE_MAX_OUTPUT_BYTES - entry->size)
		{
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
				"GTK4 theme '%s' exceeds the extraction size limit.", root);
			goto cleanup;
		}
		total += entry->size;
		selected++;
	}
	if (selected == 0)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"GTK4 theme '%s' is empty.", root);
		goto cleanup;
	}
	valid = TRUE;

cleanup:
	g_free (gtk_prefix);
	g_free (gtk_root);
	return valid;
}

static gboolean
gtk4_archive_remove_tree (const char *path)
{
	GDir *directory;
	const char *name;
	gboolean ok = TRUE;

	if (!theme_archive_path_is_directory (path))
		return g_remove (path) == 0 || errno == ENOENT;
	directory = g_dir_open (path, 0, NULL);
	if (!directory)
		return FALSE;
	while ((name = g_dir_read_name (directory)) != NULL)
	{
		char *child = g_build_filename (path, name, NULL);

		if (!gtk4_archive_remove_tree (child))
			ok = FALSE;
		g_free (child);
	}
	g_dir_close (directory);
	return g_rmdir (path) == 0 && ok;
}

static gboolean
gtk4_archive_copy_bounded (const char *source_path,
	const char *destination_path, GError **error)
{
	GFile *source = g_file_new_for_path (source_path);
	GFile *destination = g_file_new_for_path (destination_path);
	GFileInputStream *input = NULL;
	GFileOutputStream *output = NULL;
	guint8 buffer[8192];
	guint64 total = 0;
	gssize count;
	gboolean ok = FALSE;

	input = g_file_read (source, NULL, error);
	if (!input)
		goto cleanup;
	output = g_file_create (destination, G_FILE_CREATE_PRIVATE, NULL, error);
	if (!output)
		goto cleanup;
	while ((count = g_input_stream_read (G_INPUT_STREAM (input), buffer,
		sizeof buffer, NULL, error)) > 0)
	{
		if (total > FABULOR_GTK4_ARCHIVE_MAX_BYTES ||
			(guint64)count > FABULOR_GTK4_ARCHIVE_MAX_BYTES - total)
		{
			g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
				"GTK4 theme archive exceeds the permitted size.");
			goto cleanup;
		}
		if (!g_output_stream_write_all (G_OUTPUT_STREAM (output), buffer,
			(gsize)count, NULL, NULL, error))
			goto cleanup;
		total += (guint64)count;
	}
	if (count < 0 || !g_output_stream_close (G_OUTPUT_STREAM (output),
		NULL, error))
		goto cleanup;
	ok = total > 0 && theme_archive_path_is_regular (destination_path);
	if (!ok && (!error || !*error))
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"GTK4 theme archive copy is empty or unsafe.");

cleanup:
	g_clear_object (&output);
	g_clear_object (&input);
	g_object_unref (destination);
	g_object_unref (source);
	return ok;
}

static gboolean
gtk4_archive_validate_tree (const char *path, guint *entries,
	guint64 *bytes, GError **error)
{
	GDir *directory;
	const char *name;

	if (!theme_archive_path_is_directory (path))
		return g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"Extracted GTK4 theme directory is unsafe: %s", path), FALSE;
	directory = g_dir_open (path, 0, error);
	if (!directory)
		return FALSE;
	while ((name = g_dir_read_name (directory)) != NULL)
	{
		char *child = g_build_filename (path, name, NULL);
		GStatBuf stat_buffer;

		(*entries)++;
		if (*entries > FABULOR_GTK4_ARCHIVE_MAX_ENTRIES)
		{
			g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
				"Extracted GTK4 theme contains too many files.");
			g_free (child);
			g_dir_close (directory);
			return FALSE;
		}
		if (theme_archive_path_is_directory (child))
		{
			if (!gtk4_archive_validate_tree (child, entries, bytes, error))
			{
				g_free (child);
				g_dir_close (directory);
				return FALSE;
			}
		}
		else if (!theme_archive_path_is_regular (child) ||
			g_stat (child, &stat_buffer) != 0 || stat_buffer.st_size < 0)
		{
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
				"Extracted GTK4 theme contains an unsafe file: %s", child);
			g_free (child);
			g_dir_close (directory);
			return FALSE;
		}
		else
		{
			guint64 size = (guint64)stat_buffer.st_size;

			if (size > FABULOR_GTK4_ARCHIVE_MAX_FILE_BYTES ||
				*bytes > FABULOR_GTK4_ARCHIVE_MAX_OUTPUT_BYTES - size)
			{
				g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
					"Extracted GTK4 theme exceeds the size limit.");
				g_free (child);
				g_dir_close (directory);
				return FALSE;
			}
			*bytes += size;
		}
		g_free (child);
	}
	g_dir_close (directory);
	return TRUE;
}

static gboolean
gtk4_archive_css_has_uncompiled_token (const char *contents)
{
	const char *cursor = contents;
	gboolean in_comment = FALSE;
	char quote = '\0';

	while (cursor && *cursor)
	{
		if (in_comment)
		{
			if (cursor[0] == '*' && cursor[1] == '/')
			{
				in_comment = FALSE;
				cursor += 2;
			}
			else
				cursor++;
			continue;
		}
		if (quote)
		{
			if (*cursor == '\\' && cursor[1])
				cursor += 2;
			else if (*cursor == quote)
			{
				quote = '\0';
				cursor++;
			}
			else
				cursor++;
			continue;
		}
		if (cursor[0] == '/' && cursor[1] == '*')
		{
			in_comment = TRUE;
			cursor += 2;
			continue;
		}
		if (*cursor == '\'' || *cursor == '"')
		{
			quote = *cursor;
			cursor++;
			continue;
		}
		if (*cursor == '$' &&
			(g_ascii_isalpha (cursor[1]) || cursor[1] == '_'))
			return TRUE;
		cursor++;
	}
	return FALSE;
}

static gboolean
gtk4_archive_css_has_invalid_define_var (const char *contents)
{
	const char *cursor = contents;
	gboolean in_comment = FALSE;
	gboolean in_define = FALSE;
	char quote = '\0';

	while (cursor && *cursor)
	{
		if (in_comment)
		{
			if (cursor[0] == '*' && cursor[1] == '/')
			{
				in_comment = FALSE;
				cursor += 2;
			}
			else
				cursor++;
			continue;
		}
		if (quote)
		{
			if (*cursor == '\\' && cursor[1])
				cursor += 2;
			else if (*cursor == quote)
			{
				quote = '\0';
				cursor++;
			}
			else
				cursor++;
			continue;
		}
		if (cursor[0] == '/' && cursor[1] == '*')
		{
			in_comment = TRUE;
			cursor += 2;
			continue;
		}
		if (*cursor == '\'' || *cursor == '"')
		{
			quote = *cursor;
			cursor++;
			continue;
		}
		if (!in_define &&
			strncmp (cursor, "@define-color", strlen ("@define-color")) == 0)
		{
			in_define = TRUE;
			cursor += strlen ("@define-color");
			continue;
		}
		if (in_define && strncmp (cursor, "var(", 4) == 0)
			return TRUE;
		if (in_define && *cursor == ';')
			in_define = FALSE;
		cursor++;
	}
	return FALSE;
}

static gboolean
gtk4_archive_validate_css (const char *path, const char *root,
	GError **error)
{
	char *contents = NULL;
	gsize length = 0;
	gboolean valid = FALSE;

	if (!theme_archive_path_is_regular (path))
		return TRUE;
	if (!g_file_get_contents (path, &contents, &length, error))
		return FALSE;
	if (length > FABULOR_GTK4_ARCHIVE_MAX_FILE_BYTES)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
			"GTK4 theme '%s' has an oversized stylesheet.", root);
		goto cleanup;
	}
	if (!g_utf8_validate (contents, (gssize)length, NULL))
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"GTK4 theme '%s' has a stylesheet that is not valid UTF-8.",
			root);
		goto cleanup;
	}
	if (gtk4_archive_css_has_uncompiled_token (contents))
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"GTK4 theme '%s' contains uncompiled stylesheet source "
			"tokens. Install a precompiled GTK4 theme archive.", root);
		goto cleanup;
	}
	if (gtk4_archive_css_has_invalid_define_var (contents))
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"GTK4 theme '%s' uses an unsupported @define-color value.",
			root);
		goto cleanup;
	}
	valid = TRUE;

cleanup:
	g_free (contents);
	return valid;
}

static void
gtk4_archive_extract_argument (GPtrArray *argv, GPtrArray *owned,
	char *argument)
{
	g_ptr_array_add (owned, argument);
	g_ptr_array_add (argv, argument);
}

static gboolean
gtk4_archive_extract_roots (const char *program, const char *archive_path,
	const GPtrArray *entries, const GPtrArray *roots, const char *staging,
	GError **error)
{
	static const char *optional_names[] = {
		"index.theme", "thumbnail.png", "preview.png", "screenshot.png"
	};
	GPtrArray *argv = g_ptr_array_new ();
	GPtrArray *owned = g_ptr_array_new_with_free_func (g_free);
	gboolean ok;
	guint i;
	guint j;

	g_ptr_array_add (argv, (gpointer)program);
	g_ptr_array_add (argv, (gpointer)"-xf");
	g_ptr_array_add (argv, (gpointer)archive_path);
	g_ptr_array_add (argv, (gpointer)"-C");
	g_ptr_array_add (argv, (gpointer)staging);
	g_ptr_array_add (argv, (gpointer)"--");
	for (i = 0; i < roots->len; i++)
	{
		const char *root = g_ptr_array_index (roots, i);

		gtk4_archive_extract_argument (argv, owned,
			g_strdup_printf ("%s/gtk-4.0", root));
		for (j = 0; j < G_N_ELEMENTS (optional_names); j++)
		{
			char *path = g_strdup_printf ("%s/%s", root, optional_names[j]);

			if (gtk4_archive_entry_is_file (
				gtk4_archive_find_entry (entries, path)))
				gtk4_archive_extract_argument (argv, owned, path);
			else
				g_free (path);
		}
	}
	g_ptr_array_add (argv, NULL);

#ifdef G_OS_WIN32
	{
		HANDLE process;
		HANDLE unused_stdout;

		ok = theme_archive_win32_start_hidden (
			(const char * const *)argv->pdata, FALSE, &process,
			&unused_stdout, error);
		if (ok)
		{
			ok = theme_archive_win32_wait_success (process, error);
			CloseHandle (process);
		}
	}
#else
	{
		GSubprocess *process;

	process = g_subprocess_newv (
		(const char * const *)argv->pdata,
		G_SUBPROCESS_FLAGS_STDOUT_SILENCE |
		G_SUBPROCESS_FLAGS_STDERR_SILENCE, error);
	ok = process && g_subprocess_wait_check (process, NULL, error);
	g_clear_object (&process);
	}
#endif
	g_ptr_array_unref (owned);
	g_ptr_array_unref (argv);
	return ok;
}

gboolean
fabulor_gtk4_theme_archive_import (const char *archive_path,
	const char *config_dir, GPtrArray **installed_names, GError **error)
{
	GStatBuf stat_buffer;
	char *program = NULL;
	GPtrArray *entries = NULL;
	GPtrArray *roots = NULL;
	GPtrArray *installed = NULL;
	char *themes_dir = NULL;
	char *staging = NULL;
	char *private_archive = NULL;
	guint i;
	guint moved = 0;
	guint extracted_entries = 0;
	guint64 extracted_bytes = 0;
	gboolean ok = FALSE;

	if (installed_names)
		*installed_names = NULL;
	if (!archive_path || !g_path_is_absolute (archive_path) ||
		!config_dir || !config_dir[0] || !installed_names)
		return g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"GTK4 theme import arguments are invalid."), FALSE;
	if (!gtk4_archive_name_is_supported (archive_path))
		return g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"Unsupported GTK4 theme archive format."), FALSE;
	if (!theme_archive_path_is_regular (archive_path) ||
		g_stat (archive_path, &stat_buffer) != 0 || stat_buffer.st_size < 0)
		return g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_NOENT,
			"GTK4 theme archive is unavailable or unsafe."), FALSE;
	if ((guint64)stat_buffer.st_size > FABULOR_GTK4_ARCHIVE_MAX_BYTES)
		return g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
			"GTK4 theme archive exceeds the permitted size."), FALSE;

	themes_dir = g_build_filename (config_dir, "themes", NULL);
	if (!themes_dir || (g_mkdir_with_parents (themes_dir, 0700) != 0 &&
		errno != EEXIST) || !theme_archive_path_is_directory (themes_dir))
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
			"Fabulor theme directory is unavailable or unsafe.");
		goto cleanup;
	}
	{
		char *uuid = g_uuid_string_random ();
		char *name = g_strconcat (".fabulor-theme-import-", uuid, NULL);

		staging = g_build_filename (themes_dir, name, NULL);
		g_free (name);
		g_free (uuid);
	}
	if (g_mkdir (staging, 0700) != 0 ||
		!theme_archive_path_is_directory (staging))
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
			"Could not create a private GTK4 theme staging directory.");
		goto cleanup;
	}
	private_archive = g_build_filename (staging, "source.archive", NULL);
	if (!gtk4_archive_copy_bounded (archive_path, private_archive, error))
		goto cleanup;
	program = theme_archive_tar_program (error);
	if (!program)
		goto cleanup;
	entries = gtk4_archive_inventory (program, private_archive, error);
	if (!entries)
		goto cleanup;
	roots = gtk4_archive_find_theme_roots (entries, error);
	if (!roots)
		goto cleanup;
	for (i = 0; i < roots->len; i++)
	{
		if (!gtk4_archive_validate_theme (entries,
			g_ptr_array_index (roots, i), error))
			goto cleanup;
	}
	for (i = 0; i < roots->len; i++)
	{
		char *destination = g_build_filename (themes_dir,
			g_ptr_array_index (roots, i), NULL);
		gboolean exists = g_file_test (destination, G_FILE_TEST_EXISTS);

		g_free (destination);
		if (exists)
		{
			g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_EXIST,
				"GTK4 theme '%s' is already installed.",
				(const char *)g_ptr_array_index (roots, i));
			goto cleanup;
		}
	}
	if (!gtk4_archive_extract_roots (program, private_archive, entries,
		roots, staging, error))
		goto cleanup;
	for (i = 0; i < roots->len; i++)
	{
		char *root = g_ptr_array_index (roots, i);
		char *staged_root;
		char *css;
		char *dark_css;

		staged_root = g_build_filename (staging, root, NULL);
		css = g_build_filename (staged_root, "gtk-4.0", "gtk.css", NULL);
		dark_css = g_build_filename (staged_root, "gtk-4.0",
			"gtk-dark.css", NULL);
		if (!gtk4_archive_validate_tree (staged_root, &extracted_entries,
			&extracted_bytes, error) || !theme_archive_path_is_regular (css))
		{
			if (!error || !*error)
				g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
					"GTK4 theme '%s' has no safe gtk.css file.", root);
			g_free (dark_css);
			g_free (css);
			g_free (staged_root);
			goto cleanup;
		}
		if (!gtk4_archive_validate_css (css, root, error) ||
			!gtk4_archive_validate_css (dark_css, root, error))
		{
			g_free (dark_css);
			g_free (css);
			g_free (staged_root);
			goto cleanup;
		}
		g_free (dark_css);
		g_free (css);
		g_free (staged_root);
	}

	installed = g_ptr_array_new_with_free_func (g_free);
	for (i = 0; i < roots->len; i++)
	{
		const char *root = g_ptr_array_index (roots, i);
		char *source = g_build_filename (staging, root, NULL);
		char *destination = g_build_filename (themes_dir, root, NULL);

		if (g_rename (source, destination) != 0)
		{
			g_set_error (error, G_FILE_ERROR,
				g_file_error_from_errno (errno),
				"Could not install GTK4 theme '%s': %s",
				root, g_strerror (errno));
			g_free (destination);
			g_free (source);
			goto cleanup;
		}
		moved++;
		g_ptr_array_add (installed, g_strdup (root));
		g_free (destination);
		g_free (source);
	}
	gtk4_archive_remove_tree (staging);
	g_clear_pointer (&staging, g_free);
	*installed_names = installed;
	installed = NULL;
	ok = TRUE;

cleanup:
	if (!ok && themes_dir && roots)
	{
		for (i = 0; i < moved; i++)
		{
			char *destination = g_build_filename (themes_dir,
				g_ptr_array_index (roots, i), NULL);
			gtk4_archive_remove_tree (destination);
			g_free (destination);
		}
	}
	if (staging)
		gtk4_archive_remove_tree (staging);
	g_free (staging);
	g_free (private_archive);
	g_free (themes_dir);
	g_clear_pointer (&installed, g_ptr_array_unref);
	g_clear_pointer (&roots, g_ptr_array_unref);
	g_clear_pointer (&entries, g_ptr_array_unref);
	g_free (program);
	return ok;
}
