#ifndef FABULOR_IRCV3_CAPABILITY_H
#define FABULOR_IRCV3_CAPABILITY_H

#include <glib.h>

typedef struct
{
	char *name;
	char *value;
	gboolean disable;
} ircv3_capability_token;

gboolean ircv3_capability_token_parse (const char *text,
										 ircv3_capability_token *token);
void ircv3_capability_token_clear (ircv3_capability_token *token);

char *ircv3_capability_build_request (const char *advertised,
										const char * const *supported,
										gsize supported_count);

gboolean ircv3_sasl_mechanism_available (const char *advertised,
										 const char *mechanism);

#endif
