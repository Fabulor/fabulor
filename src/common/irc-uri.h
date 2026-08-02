#ifndef FABULOR_IRC_URI_H
#define FABULOR_IRC_URI_H

#include <glib.h>

typedef struct
{
	char *host;
	guint16 port;
	gboolean has_port;
	char *channel;
	char *key;
	gboolean use_tls;
} FabulorIrcUri;

gboolean fabulor_irc_uri_has_supported_scheme (const char *text);
gboolean fabulor_irc_uri_parse (const char *text, FabulorIrcUri *result,
								GError **error);
void fabulor_irc_uri_clear (FabulorIrcUri *uri);

#endif
