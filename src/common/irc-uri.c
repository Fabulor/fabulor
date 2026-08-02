#include <string.h>

#include <glib.h>

#include "irc-uri.h"

#define IRC_URI_MAX_HOST_LENGTH 127
#define IRC_URI_MAX_CHANNEL_LENGTH 255
#define IRC_URI_MAX_KEY_LENGTH 63

typedef enum
{
	IRC_URI_ERROR_INVALID
} IrcUriError;

static GQuark
irc_uri_error_quark (void)
{
	return g_quark_from_static_string ("fabulor-irc-uri-error");
}

static gboolean
set_invalid_error (GError **error, const char *message)
{
	g_set_error_literal (error, irc_uri_error_quark (), IRC_URI_ERROR_INVALID,
					 message);
	return FALSE;
}

static gboolean
component_is_safe (const char *text, gsize maximum_length)
{
	const unsigned char *current;

	if (!text || !g_utf8_validate (text, -1, NULL) || strlen (text) > maximum_length)
		return FALSE;

	for (current = (const unsigned char *)text; *current; current++)
	{
		if (*current <= 0x20 || *current == 0x7f)
			return FALSE;
	}

	return TRUE;
}

gboolean
fabulor_irc_uri_has_supported_scheme (const char *text)
{
	const char *scheme;
	gsize scheme_length;

	if (!text)
		return FALSE;

	scheme = g_uri_peek_scheme (text);
	if (!scheme)
		return FALSE;

	scheme_length = strcspn (text, ":");
	return (scheme_length == 3 && g_ascii_strncasecmp (scheme, "irc", 3) == 0) ||
		   (scheme_length == 4 && g_ascii_strncasecmp (scheme, "ircs", 4) == 0);
}

gboolean
fabulor_irc_uri_parse (const char *text, FabulorIrcUri *result, GError **error)
{
	char *scheme = NULL;
	char *userinfo = NULL;
	char *host = NULL;
	char *path = NULL;
	char *query = NULL;
	char *fragment = NULL;
	char *channel = NULL;
	GError *parse_error = NULL;
	gint port = -1;
	gboolean valid = FALSE;

	g_return_val_if_fail (result != NULL, FALSE);
	memset (result, 0, sizeof (*result));

	if (!fabulor_irc_uri_has_supported_scheme (text))
		return set_invalid_error (error, "The URI scheme must be irc or ircs.");

	if (!g_uri_split (text, G_URI_FLAGS_NONE, &scheme, &userinfo, &host, &port,
					  &path, &query, &fragment, &parse_error))
	{
		g_propagate_prefixed_error (error, parse_error, "Invalid IRC URI: ");
		goto done;
	}

	if ((g_ascii_strcasecmp (scheme, "irc") != 0 &&
		 g_ascii_strcasecmp (scheme, "ircs") != 0) || !host || !*host)
	{
		set_invalid_error (error, "The IRC URI must contain a server name.");
		goto done;
	}

	if (userinfo && *userinfo)
	{
		set_invalid_error (error, "IRC URIs must not contain user information.");
		goto done;
	}

	if (fragment)
	{
		set_invalid_error (error, "IRC URIs must not contain a fragment.");
		goto done;
	}

	if (!component_is_safe (host, IRC_URI_MAX_HOST_LENGTH))
	{
		set_invalid_error (error, "The IRC server name is invalid or too long.");
		goto done;
	}

	if (port == 0 || port > 65535)
	{
		set_invalid_error (error, "The IRC port must be between 1 and 65535.");
		goto done;
	}

	if (path && *path)
	{
		if (path[0] != '/')
		{
			set_invalid_error (error, "The IRC channel path is invalid.");
			goto done;
		}

		channel = path + 1;
		if (*channel == '#')
			channel++;

		if (*channel &&
			(!component_is_safe (channel, IRC_URI_MAX_CHANNEL_LENGTH) ||
			 strchr (channel, ',') || strchr (channel, ':')))
		{
			set_invalid_error (error, "The IRC channel name is invalid or too long.");
			goto done;
		}
	}

	if (query && *query)
	{
		if (!channel || !*channel || !component_is_safe (query, IRC_URI_MAX_KEY_LENGTH))
		{
			set_invalid_error (error, "The IRC channel key is invalid or too long.");
			goto done;
		}
	}

	result->host = g_strdup (host);
	result->port = port > 0 ? (guint16)port : 0;
	result->has_port = port > 0;
	result->channel = channel && *channel ? g_strdup (channel) : NULL;
	result->key = query && *query ? g_strdup (query) : NULL;
	result->use_tls = g_ascii_strcasecmp (scheme, "ircs") == 0;
	valid = TRUE;

done:
	g_free (scheme);
	g_free (userinfo);
	g_free (host);
	g_free (path);
	g_free (query);
	g_free (fragment);
	return valid;
}

void
fabulor_irc_uri_clear (FabulorIrcUri *uri)
{
	if (!uri)
		return;

	g_clear_pointer (&uri->host, g_free);
	g_clear_pointer (&uri->channel, g_free);
	g_clear_pointer (&uri->key, g_free);
	uri->port = 0;
	uri->has_port = FALSE;
	uri->use_tls = FALSE;
}
