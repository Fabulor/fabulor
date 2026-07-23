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
#include <string.h>

#ifdef G_OS_WIN32
#include <windows.h>
#endif

#define FABULOR_THEME_ARCHIVE_MAX_BYTES (16 * 1024 * 1024)
#define FABULOR_THEME_ARCHIVE_LIST_MAX_BYTES (1024 * 1024)
#define FABULOR_THEME_ARCHIVE_TEXT_MAX_BYTES (1024 * 1024)
#define FABULOR_THEME_ARCHIVE_MAX_DEPTH 8

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
