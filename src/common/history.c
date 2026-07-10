/* X-Chat
 * Copyright (C) 1998 Peter Zelezny.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <glib.h>
#include <string.h>
#include "history.h"
#include "cfgfiles.h"
#include "zoitechatc.h"

#define HISTORY_FILE "input-history.conf"
#define HISTORY_MAX_LIMIT 10000

static struct history shared_history;
static gboolean shared_loaded = FALSE;
static gboolean shared_dirty = FALSE;

static int
history_get_limit (void)
{
	int max = prefs.hex_input_history_max;

	if (max < 0)
		max = 0;
	if (max > HISTORY_MAX_LIMIT)
		max = HISTORY_MAX_LIMIT;

	return max;
}

static void
history_clear (struct history *his)
{
	int i;

	if (!his)
		return;

	for (i = 0; i < his->len; i++)
		g_free (his->lines[i]);
	g_free (his->lines);
	his->lines = NULL;
	his->len = 0;
	his->max = 0;
	his->pos = 0;
}

static gboolean
history_ensure_limit (struct history *his)
{
	char **lines;
	int max;
	int keep;
	int start;
	int i;

	if (!his)
		return FALSE;

	max = history_get_limit ();
	if (max <= 0)
	{
		history_clear (his);
		return FALSE;
	}

	if (his->lines && his->max == max)
		return TRUE;

	keep = his->len < max ? his->len : max;
	start = his->len - keep;
	lines = g_new0 (char *, max);

	for (i = 0; i < start; i++)
		g_free (his->lines[i]);
	for (i = 0; i < keep; i++)
		lines[i] = his->lines[start + i];

	g_free (his->lines);
	his->lines = lines;
	his->len = keep;
	his->max = max;
	his->pos = his->len;

	return TRUE;
}

static void history_load_shared (void);

static void
history_add_internal (struct history *his, const char *text, gboolean update_shared)
{
	if (!his || !text)
		return;

	if (!history_ensure_limit (his))
		return;

	if (his->len == his->max)
	{
		g_free (his->lines[0]);
		memmove (his->lines, his->lines + 1, sizeof (char *) * (his->len - 1));
		his->len--;
	}

	his->lines[his->len++] = g_strdup (text);
	his->pos = his->len;

	if (update_shared && prefs.hex_input_history_save)
	{
		history_load_shared ();
		history_add_internal (&shared_history, text, FALSE);
		shared_dirty = TRUE;
	}
}

void
history_add (struct history *his, char *text)
{
	history_add_internal (his, text, TRUE);
}

void
history_free (struct history *his)
{
	history_clear (his);
}

static void
history_load_shared (void)
{
	char *path;
	char *contents = NULL;
	char **lines;
	int i;

	if (shared_loaded)
		return;

	shared_loaded = TRUE;
	if (!prefs.hex_input_history_save)
		return;

	path = g_build_filename (get_xdir (), HISTORY_FILE, NULL);
	if (!g_file_get_contents (path, &contents, NULL, NULL))
	{
		g_free (path);
		return;
	}
	g_free (path);

	lines = g_strsplit (contents, "\n", -1);
	for (i = 0; lines[i]; i++)
	{
		char *compressed;
		char *restored;

		compressed = g_strchomp (lines[i]);
		if (!compressed[0])
			continue;

		restored = g_strcompress (compressed);
		history_add_internal (&shared_history, restored, FALSE);
		g_free (restored);
	}

	g_strfreev (lines);
	g_free (contents);
	shared_dirty = FALSE;
}

void
history_restore (struct history *his)
{
	int i;

	if (!his || !prefs.hex_input_history_save)
		return;

	history_load_shared ();
	for (i = 0; i < shared_history.len; i++)
		history_add_internal (his, shared_history.lines[i], FALSE);
}

void
history_save (void)
{
	GString *out;
	char *path;
	int i;

	if (!prefs.hex_input_history_save || !shared_loaded || !shared_dirty)
		return;

	history_ensure_limit (&shared_history);

	out = g_string_new ("");
	for (i = 0; i < shared_history.len; i++)
	{
		char *escaped = g_strescape (shared_history.lines[i], NULL);

		g_string_append (out, escaped);
		g_string_append_c (out, '\n');
		g_free (escaped);
	}

	path = g_build_filename (get_xdir (), HISTORY_FILE, NULL);
	g_file_set_contents (path, out->str, out->len, NULL);
	g_free (path);
	g_string_free (out, TRUE);
	shared_dirty = FALSE;
}

char *
history_down (struct history *his)
{
	if (!history_ensure_limit (his))
		return NULL;

	if (his->pos >= his->len)
		return NULL;

	if (his->pos == his->len - 1)
	{
		his->pos = his->len;
		return "";
	}

	his->pos++;
	return his->lines[his->pos];
}

char *
history_up (struct history *his, char *current_text)
{
	if (!history_ensure_limit (his) || his->len == 0)
		return NULL;

	if (his->pos > his->len)
		his->pos = his->len;

	if (his->pos == his->len)
	{
		if (his->max > 1 && current_text && current_text[0] &&
		    strcmp (current_text, his->lines[his->len - 1]) != 0)
		{
			history_add_internal (his, current_text, TRUE);
			if (his->len < 2)
				return NULL;
			his->pos = his->len - 2;
			return his->lines[his->pos];
		}

		his->pos = his->len - 1;
		return his->lines[his->pos];
	}

	if (his->pos == 0)
		return NULL;

	his->pos--;
	return his->lines[his->pos];
}
