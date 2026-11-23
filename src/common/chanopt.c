/* ZoiteChat
 * Copyright (C) 1998-2010 Peter Zelezny.
 * Copyright (C) 2009-2013 Berke Viktor.
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

/* per-channel/dialog settings :: /CHANOPT */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "fabulor.h"

#include "cfgfiles.h"
#include "server.h"
#include "text.h"
#include "util.h"
#include "fabulorc.h"


static GSList *chanopt_list = NULL;
static gboolean chanopt_open = FALSE;
static gboolean chanopt_changed = FALSE;


typedef struct
{
	char *name;
	char *alias;	/* old names from 2.8.4 */
	int offset;
} channel_options;

#define S_F(xx) STRUCT_OFFSET_STR(struct session,xx)

static const channel_options chanopt[] =
{
	{"alert_balloon", NULL, S_F(alert_balloon)},
	{"alert_beep", "BEEP", S_F(alert_beep)},
	{"alert_taskbar", NULL, S_F(alert_taskbar)},
	{"alert_tray", "TRAY", S_F(alert_tray)},

	{"text_hidejoinpart", "CONFMODE", S_F(text_hidejoinpart)},
	{"text_logging", NULL, S_F(text_logging)},
	{"text_scrollback", NULL, S_F(text_scrollback)},
	{"text_strip", NULL, S_F(text_strip)},
};

#undef S_F

static char *
chanopt_value (guint8 val)
{
	switch (val)
	{
	case SET_OFF:
		return _("OFF");
	case SET_ON:
		return _("ON");
	case SET_DEFAULT:
		return _("{unset}");
	default:
		g_assert_not_reached ();
		return NULL;
	}
}

static guint8
str_to_chanopt (const char *str)
{
	if (!g_ascii_strcasecmp (str, "ON") || !strcmp (str, "1"))
		return SET_ON;
	else if (!g_ascii_strcasecmp (str, "OFF") || !strcmp (str, "0"))
		return SET_OFF;
	else
		return SET_DEFAULT;
}

int
chanopt_command (session *sess, char *tbuf, char *word[], char *word_eol[])
{
	int dots, i = 0, j, p = 0;
	guint8 val;
	int offset = 2;
	char *find;
	gboolean quiet = FALSE;
	int newval = -1;

	if (!strcmp (word[2], "-quiet"))
	{
		quiet = TRUE;
		offset++;
	}

	find = word[offset++];

	if (word[offset][0])
	{
		newval = str_to_chanopt (word[offset]);
	}

	if (!quiet)
		PrintTextf (sess, "\002%s\002: %s \002%s\002: %s\n",
						_("Network"),
						sess->server->network ? server_get_network (sess->server, TRUE) : _("<none>"),
						_("Channel"),
						sess->session_name[0] ? sess->session_name : _("<none>"));

	while (i < sizeof (chanopt) / sizeof (channel_options))
	{
		if (find[0] == 0 || match (find, chanopt[i].name) || (chanopt[i].alias && match (find, chanopt[i].alias)))
		{
			if (newval != -1)
			{
				*(guint8 *)G_STRUCT_MEMBER_P(sess, chanopt[i].offset) = newval;
				chanopt_changed = TRUE;
			}

			if (!quiet)
			{
				strcpy (tbuf, chanopt[i].name);
				p = strlen (tbuf);

				tbuf[p++] = 3;
				tbuf[p++] = '2';

				dots = 20 - strlen (chanopt[i].name);

				for (j = 0; j < dots; j++)
					tbuf[p++] = '.';
				tbuf[p++] = 0;

				val = G_STRUCT_MEMBER (guint8, sess, chanopt[i].offset);
				PrintTextf (sess, "%s\0033:\017 %s", tbuf, chanopt_value (val));
			}
		}
		i++;
	}

	return TRUE;
}

/* is a per-channel setting set? Or is it UNSET and
 * the global version is set? */

gboolean
chanopt_is_set (unsigned int global, guint8 per_chan_setting)
{
	if (per_chan_setting == SET_ON || per_chan_setting == SET_OFF)
		return per_chan_setting;
	else
		return global;
}

typedef struct
{
	guint8 alert_balloon;
	guint8 alert_beep;
	guint8 alert_taskbar;
	guint8 alert_tray;

	guint8 text_hidejoinpart;
	guint8 text_logging;
	guint8 text_scrollback;
	guint8 text_strip;

	char *network;
	char *channel;

	ActivityPolicy activity_policy;
} chanopt_in_memory;


static chanopt_in_memory *
chanopt_find (char *network, char *channel, gboolean add_new)
{
	GSList *list;
	chanopt_in_memory *co;
	int i;

	for (list = chanopt_list; list; list = list->next)
	{
		co = list->data;
		if (!g_ascii_strcasecmp (co->channel, channel) &&
			 !g_ascii_strcasecmp (co->network, network))
			return co;
	}

	if (!add_new)
		return NULL;

	co = g_new0 (chanopt_in_memory, 1);
	co->channel = g_strdup (channel);
	co->network = g_strdup (network);

	i = 0;
	while (i < sizeof (chanopt) / sizeof (channel_options))
	{
		*(guint8 *)G_STRUCT_MEMBER_P(co, chanopt[i].offset) = SET_DEFAULT;
		i++;
	}

	chanopt_list = g_slist_prepend (chanopt_list, co);
	chanopt_changed = TRUE;

	return co;
}

static void
chanopt_add_opt (chanopt_in_memory *co, char *var, int new_value)
{
	int i;

	i = 0;
	while (i < sizeof (chanopt) / sizeof (channel_options))
	{
		if (!strcmp (var, chanopt[i].name))
		{
			*(guint8 *)G_STRUCT_MEMBER_P(co, chanopt[i].offset) = new_value;

		}
		i++;
	}
}

static void
chanopt_load_all (void)
{
	int fh;
	char buf[CHANLEN + 64];
	char *eq;
	char *network = NULL;
	chanopt_in_memory *current = NULL;

	fh = fabulor_open_file ("chanopt.conf", O_RDONLY, 0, 0);
	if (fh != -1)
	{
		while (waitline (fh, buf, sizeof buf, FALSE) != -1)
		{
			char *value;
			eq = strchr (buf, '=');
			if (!eq)
				continue;
			*eq = 0;
			g_strstrip (buf);
			value = g_strstrip (eq + 1);

			if (!strcmp (buf, "network"))
			{
				g_free (network);
				network = g_strdup (value);
				current = NULL;
			}
			else if (!strcmp (buf, "channel"))
			{
				current = network && *network && *value ?
					chanopt_find (network, value, TRUE) : NULL;
			}
			else
			{
				if (current && !strcmp (buf, "activity_policy"))
				{
					ActivityPolicy parsed;
					if (activity_policy_parse (value, &parsed))
						current->activity_policy = parsed;
				}
				else if (current)
					chanopt_add_opt (current, buf, str_to_chanopt (value));
			}

		}
		close (fh);
		g_free (network);
		chanopt_changed = FALSE;
	}
}

void
chanopt_initialise (void)
{
	if (!chanopt_open)
	{
		chanopt_open = TRUE;
		chanopt_load_all ();
	}
}

static chanopt_in_memory *
chanopt_activity_find (session *sess, const char *network, const char *context)
{
	for (GSList *entry = chanopt_list; entry; entry = entry->next)
	{
		chanopt_in_memory *co = entry->data;
		if (!g_ascii_strcasecmp (co->network, network) &&
			!sess->server->p_cmp (co->channel, context))
			return co;
	}
	return NULL;
}

ActivityPolicy
chanopt_activity_get (session *sess, const char *network, const char *context)
{
	chanopt_in_memory *co = chanopt_activity_find (sess, network, context);

	return co ? co->activity_policy : ACTIVITY_POLICY_AUTO;
}

void
chanopt_activity_set (session *sess, const char *network, const char *context,
					 ActivityPolicy policy)
{
	chanopt_in_memory *co = chanopt_activity_find (sess, network, context);

	if (!co && policy != ACTIVITY_POLICY_AUTO)
		co = chanopt_find ((char *) network, (char *) context, TRUE);
	if (co && co->activity_policy != policy)
	{
		co->activity_policy = policy;
		chanopt_changed = TRUE;
	}
}

void
chanopt_load (session *sess)
{
	int i;
	guint8 val;
	chanopt_in_memory *co;
	char *network;

	if (sess->session_name[0] == 0)
		return;

	network = server_get_network (sess->server, FALSE);
	if (!network)
		return;

	chanopt_initialise ();

	co = chanopt_find (network, sess->session_name, FALSE);
	if (!co)
		return;

	i = 0;
	while (i < sizeof (chanopt) / sizeof (channel_options))
	{
		val = G_STRUCT_MEMBER(guint8, co, chanopt[i].offset);
		*(guint8 *)G_STRUCT_MEMBER_P(sess, chanopt[i].offset) = val;
		i++;
	}
}

void
chanopt_save (session *sess)
{
	int i;
	guint8 vals;
	guint8 valm;
	chanopt_in_memory *co;
	char *network;

	if (sess->session_name[0] == 0)
		return;

	network = server_get_network (sess->server, FALSE);
	if (!network)
		return;

	co = chanopt_find (network, sess->session_name, TRUE);

	i = 0;
	while (i < sizeof (chanopt) / sizeof (channel_options))
	{
		vals = G_STRUCT_MEMBER(guint8, sess, chanopt[i].offset);
		valm = G_STRUCT_MEMBER(guint8, co, chanopt[i].offset);

		if (vals != valm)
		{
			*(guint8 *)G_STRUCT_MEMBER_P(co, chanopt[i].offset) = vals;
			chanopt_changed = TRUE;
		}

		i++;
	}
}

static void
chanopt_append_channel (chanopt_in_memory *co, GString *data)
{
	gboolean has_settings = co->activity_policy != ACTIVITY_POLICY_AUTO;
	for (guint i = 0; i < G_N_ELEMENTS (chanopt) && !has_settings; ++i)
		has_settings = G_STRUCT_MEMBER (guint8, co, chanopt[i].offset) != SET_DEFAULT;
	if (!has_settings)
		return;

	if (data->len)
		g_string_append_c (data, '\n');
	g_string_append_printf (data, "network = %s\nchannel = %s\n", co->network, co->channel);
	if (co->activity_policy != ACTIVITY_POLICY_AUTO)
		g_string_append_printf (data, "activity_policy = %s\n",
			activity_policy_name (co->activity_policy));
	for (guint i = 0; i < G_N_ELEMENTS (chanopt); ++i)
	{
		guint8 val = G_STRUCT_MEMBER (guint8, co, chanopt[i].offset);
		if (val != SET_DEFAULT)
			g_string_append_printf (data, "%s = %d\n", chanopt[i].name, val);
	}
}

gboolean
chanopt_save_all (gboolean flush)
{
	if (chanopt_changed)
	{
		GString *data = g_string_new (NULL);
		char *path = g_build_filename (get_xdir (), "chanopt.conf", NULL);
		gboolean saved;
		for (GSList *item = chanopt_list; item; item = item->next)
			chanopt_append_channel (item->data, data);
		saved = g_file_set_contents_full (path, data->str, data->len,
			G_FILE_SET_CONTENTS_CONSISTENT, 0600, NULL);
		g_string_free (data, TRUE);
		g_free (path);
		if (!saved)
			return FALSE;
		chanopt_changed = FALSE;
	}

	if (flush)
	{
		for (GSList *item = chanopt_list; item; item = item->next)
		{
			chanopt_in_memory *co = item->data;
			g_free (co->network);
			g_free (co->channel);
			g_free (co);
		}
		g_slist_free (chanopt_list);
		chanopt_list = NULL;
		chanopt_open = FALSE;
	}
	return TRUE;
}
