/* Fabulor
 * Copyright (C) 2026 deepend-tildeclub.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_THEME_ARCHIVE_READER_H
#define FABULOR_THEME_ARCHIVE_READER_H

#include <glib.h>

typedef struct
{
	char *display_name;
	char *path;
} FabulorThemeArchive;

typedef enum
{
	FABULOR_THEME_COLOR_MISSING,
	FABULOR_THEME_COLOR_VALID,
	FABULOR_THEME_COLOR_INVALID
} FabulorThemeColorResult;

GPtrArray *fabulor_theme_archive_discover (const char *config_dir);
void fabulor_theme_archive_free (FabulorThemeArchive *archive);

FabulorThemeColorResult fabulor_theme_colors_parse_token (
	const char *contents, guint token, gboolean dark,
	guint16 *red, guint16 *green, guint16 *blue);
gboolean fabulor_theme_colors_read_token (const char *contents,
	guint token, gboolean dark, guint16 *red, guint16 *green, guint16 *blue);

gboolean fabulor_theme_archive_read_text_file (const char *archive_path,
	const char *name, char **contents, GError **error);

#endif
