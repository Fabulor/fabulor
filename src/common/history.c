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
#include <glib/gstdio.h>
#include <string.h>
#include "fabulor.h"
#include "history.h"
#include "cfgfiles.h"
#include "server.h"
#include "util.h"
#include "fabulorc.h"

#define HISTORY_DIRECTORY "history"
#define HISTORY_EXTENSION ".log"
#define HISTORY_MAX_LIMIT 10000

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
history_clear_lines (struct history *his)
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

static char *
history_filename_component (const char *name)
{
	char *cursor;
	char *component;
	int length;

	component = g_strdup (name ? name : "");
	cursor = component;
	while (*cursor)
	{
		length = g_utf8_skip[((unsigned char *)cursor)[0]];
		if (length == 1)
		{
#ifndef WIN32
			*cursor = rfc_tolower (*cursor);
			if (*cursor == '/')
#else
			if (*cursor == '\\' || *cursor == '|' || *cursor == '/' ||
				 *cursor == '>' || *cursor == '<' || *cursor == ':' ||
				 *cursor == '"' || *cursor == '*' || *cursor == '?')
#endif
				*cursor = '_';
		}
		cursor += length;
	}

	return component;
}

static char *
history_resolve_path (struct history *his)
{
	session *sess;
	char *network;
	const char *target;
	char *network_file;
	char *target_file;
	char *target_filename;
	char *filename;

	if (!his || !his->owner)
		return NULL;

	sess = his->owner;
	network = server_get_network (sess->server, FALSE);
	if (!network || !network[0] || !rfc_casecmp (network, "NETWORK"))
		return NULL;

	if (sess->type == SESS_SERVER)
		target = "server";
	else
		target = sess->channel;
	if (!target || !target[0])
		return NULL;

	network_file = history_filename_component (network);
	target_file = history_filename_component (target);
	if (!network_file[0] || !target_file[0])
	{
		g_free (network_file);
		g_free (target_file);
		return NULL;
	}

	target_filename = g_strconcat (target_file, HISTORY_EXTENSION, NULL);
	filename = g_build_filename (get_xdir (), HISTORY_DIRECTORY, network_file,
										 target_filename, NULL);
	g_free (network_file);
	g_free (target_file);
	g_free (target_filename);
	return filename;
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
		history_clear_lines (his);
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

static void
history_append (struct history *his, const char *text)
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
}

static gboolean
history_write (struct history *his)
{
	GString *out;
	char *directory;
	int i;
	gboolean success;

	if (!his || !prefs.hex_input_history_save || !his->dirty ||
		 !his->storage_path)
		return TRUE;

	history_ensure_limit (his);
	if (his->len == 0)
	{
		if (g_remove (his->storage_path) == 0 ||
			 !g_file_test (his->storage_path, G_FILE_TEST_EXISTS))
		{
			his->dirty = FALSE;
			return TRUE;
		}
		return FALSE;
	}

	out = g_string_new ("");
	for (i = 0; i < his->len; i++)
	{
		char *escaped = g_strescape (his->lines[i], NULL);

		g_string_append (out, escaped);
		g_string_append_c (out, '\n');
		g_free (escaped);
	}

	directory = g_path_get_dirname (his->storage_path);
	success = g_mkdir_with_parents (directory, 0700) == 0 &&
		g_file_set_contents (his->storage_path, out->str, out->len, NULL);
	g_free (directory);
	g_string_free (out, TRUE);
	if (success)
		his->dirty = FALSE;
	else
		g_warning ("Could not save input history to %s", his->storage_path);
	return success;
}

static void
history_load (struct history *his)
{
	char *contents = NULL;
	char **lines;
	int i;

	his->loaded = TRUE;
	if (!g_file_get_contents (his->storage_path, &contents, NULL, NULL))
		return;

	lines = g_strsplit (contents, "\n", -1);
	for (i = 0; lines[i]; i++)
	{
		char *compressed;
		char *restored;

		compressed = g_strchomp (lines[i]);
		if (!compressed[0])
			continue;

		restored = g_strcompress (compressed);
		history_append (his, restored);
		g_free (restored);
	}

	g_strfreev (lines);
	g_free (contents);
	his->dirty = FALSE;
}

static gboolean
history_sync_context (struct history *his)
{
	char *path;
	char **pending_lines = NULL;
	int pending_len = 0;
	gboolean pending_dirty = FALSE;
	int i;

	if (!his || !prefs.hex_input_history_save)
		return FALSE;

	path = history_resolve_path (his);
	if (!path)
		return FALSE;
	if (his->storage_path && !strcmp (his->storage_path, path))
	{
		g_free (path);
		if (!his->loaded)
			history_load (his);
		return TRUE;
	}

	if (his->storage_path)
	{
		if (!history_write (his))
		{
			g_free (path);
			return FALSE;
		}
		history_clear_lines (his);
		g_free (his->storage_path);
		his->storage_path = NULL;
		his->loaded = FALSE;
		his->dirty = FALSE;
	}
	else if (his->len > 0)
	{
		pending_lines = his->lines;
		pending_len = his->len;
		pending_dirty = his->dirty;
		his->lines = NULL;
		his->len = 0;
		his->max = 0;
		his->pos = 0;
	}

	his->storage_path = path;
	history_load (his);
	for (i = 0; i < pending_len; i++)
	{
		history_append (his, pending_lines[i]);
		g_free (pending_lines[i]);
	}
	g_free (pending_lines);
	if (pending_len > 0)
		his->dirty = pending_dirty || prefs.hex_input_history_save;
	return TRUE;
}

void
history_add (struct history *his, char *text)
{
	if (!his || !text)
		return;

	history_sync_context (his);
	history_append (his, text);
	if (prefs.hex_input_history_save)
		his->dirty = TRUE;
}

void
history_erase (struct history *his)
{
	char *path;

	if (!his)
		return;

	history_sync_context (his);
	history_clear_lines (his);
	path = his->storage_path ? g_strdup (his->storage_path) :
		history_resolve_path (his);
	if (path)
	{
		if (g_remove (path) == 0 || !g_file_test (path, G_FILE_TEST_EXISTS))
			his->dirty = FALSE;
		else
		{
			his->dirty = TRUE;
			g_warning ("Could not clear input history at %s", path);
		}
		his->loaded = TRUE;
		g_free (path);
	}
}

void
history_free (struct history *his)
{
	if (!his)
		return;

	history_sync_context (his);
	history_write (his);
	history_clear_lines (his);
	g_free (his->storage_path);
	his->storage_path = NULL;
	his->owner = NULL;
	his->loaded = FALSE;
	his->dirty = FALSE;
}

void
history_restore (struct history *his, struct session *owner)
{
	if (!his)
		return;

	his->owner = owner;
	history_sync_context (his);
}

void
history_save (void)
{
	GSList *list;

	if (!prefs.hex_input_history_save)
		return;

	for (list = sess_list; list; list = list->next)
	{
		session *sess = list->data;

		history_sync_context (&sess->history);
		history_write (&sess->history);
	}
}

char *
history_down (struct history *his)
{
	history_sync_context (his);
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
	history_sync_context (his);
	if (!history_ensure_limit (his) || his->len == 0)
		return NULL;

	if (his->pos > his->len)
		his->pos = his->len;

	if (his->pos == his->len)
	{
		if (his->max > 1 && current_text && current_text[0] &&
		    strcmp (current_text, his->lines[his->len - 1]) != 0)
		{
			history_append (his, current_text);
			if (prefs.hex_input_history_save)
				his->dirty = TRUE;
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
