#include <string.h>

#include "ircv3-message-tags.h"

struct ircv3_message_tags
{
	GHashTable *values;
};

static char *
message_tag_unescape (const char *value)
{
	GString *out;
	const char *p;

	if (!value || !*value)
		return NULL;

	out = g_string_sized_new (strlen (value));
	for (p = value; *p; p++)
	{
		if (*p != '\\')
		{
			g_string_append_c (out, *p);
			continue;
		}

		p++;
		if (!*p)
			break;

		switch (*p)
		{
		case ':': g_string_append_c (out, ';'); break;
		case 's': g_string_append_c (out, ' '); break;
		case '\\': g_string_append_c (out, '\\'); break;
		case 'r': g_string_append_c (out, '\r'); break;
		case 'n': g_string_append_c (out, '\n'); break;
		default: g_string_append_c (out, *p); break;
		}
	}

	if (!g_utf8_validate (out->str, -1, NULL))
	{
		g_string_free (out, TRUE);
		return NULL;
	}

	return g_string_free (out, FALSE);
}

ircv3_message_tags *
ircv3_message_tags_parse (const char *text)
{
	ircv3_message_tags *parsed;
	char **entries;
	gsize i;

	parsed = g_new0 (ircv3_message_tags, 1);
	parsed->values = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
	if (!text || !*text)
		return parsed;

	entries = g_strsplit (text, ";", 0);
	for (i = 0; entries[i]; i++)
	{
		char *separator;
		char *value = NULL;

		if (!entries[i][0])
			continue;

		separator = strchr (entries[i], '=');
		if (separator)
		{
			*separator = '\0';
			value = message_tag_unescape (separator + 1);
		}

		if (entries[i][0] && g_utf8_validate (entries[i], -1, NULL))
			g_hash_table_replace (parsed->values, g_strdup (entries[i]), value);
		else
			g_free (value);
	}

	g_strfreev (entries);
	return parsed;
}

const char *
ircv3_message_tags_lookup (const ircv3_message_tags *tags, const char *key,
							 gboolean *present)
{
	gpointer value = NULL;
	gboolean found;

	found = tags && key
		? g_hash_table_lookup_extended (tags->values, key, NULL, &value)
		: FALSE;
	if (present)
		*present = found;
	return found ? value : NULL;
}

void
ircv3_message_tags_free (ircv3_message_tags *tags)
{
	if (!tags)
		return;
	g_hash_table_destroy (tags->values);
	g_free (tags);
}
