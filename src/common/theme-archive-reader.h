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

gboolean fabulor_theme_archive_read_text_file (const char *archive_path,
	const char *name, char **contents, GError **error);

#endif
