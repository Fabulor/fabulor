#include <string.h>

#include "ircv3-capability.h"

gboolean
ircv3_capability_token_parse (const char *text, ircv3_capability_token *token)
{
	const char *name;
	const char *separator;

	g_return_val_if_fail (token != NULL, FALSE);

	token->name = NULL;
	token->value = NULL;
	token->disable = FALSE;

	if (!text || !*text)
		return FALSE;

	name = text;
	if (*name == '-')
	{
		token->disable = TRUE;
		name++;
	}

	if (!*name)
		return FALSE;

	separator = strchr (name, '=');
	if (separator)
	{
		if (separator == name)
			return FALSE;
		token->name = g_strndup (name, separator - name);
		token->value = g_strdup (separator + 1);
	}
	else
	{
		token->name = g_strdup (name);
	}

	return TRUE;
}

void
ircv3_capability_token_clear (ircv3_capability_token *token)
{
	if (!token)
		return;

	g_clear_pointer (&token->name, g_free);
	g_clear_pointer (&token->value, g_free);
	token->disable = FALSE;
}

static gboolean
capability_is_supported (const char *name, const char * const *supported,
							 gsize supported_count)
{
	gsize i;

	for (i = 0; i < supported_count; i++)
	{
		if (!strcmp (name, supported[i]))
			return TRUE;
	}

	return FALSE;
}

char *
ircv3_capability_build_request (const char *advertised,
								const char * const *supported,
								gsize supported_count)
{
	GHashTable *seen;
	GString *request;
	char **tokens;
	gsize i;

	if (!advertised || !*advertised || !supported || supported_count == 0)
		return NULL;

	seen = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	request = g_string_new (NULL);
	tokens = g_strsplit (advertised, " ", 0);

	for (i = 0; tokens[i]; i++)
	{
		ircv3_capability_token token;

		if (!ircv3_capability_token_parse (tokens[i], &token))
			continue;

		if (!token.disable
			&& capability_is_supported (token.name, supported, supported_count)
			&& !g_hash_table_contains (seen, token.name))
		{
			if (request->len)
				g_string_append_c (request, ' ');
			g_string_append (request, token.name);
			g_hash_table_add (seen, g_strdup (token.name));
		}

		ircv3_capability_token_clear (&token);
	}

	g_strfreev (tokens);
	g_hash_table_destroy (seen);

	if (!request->len)
	{
		g_string_free (request, TRUE);
		return NULL;
	}

	return g_string_free (request, FALSE);
}
