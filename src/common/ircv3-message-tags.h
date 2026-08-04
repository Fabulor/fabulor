#ifndef FABULOR_IRCV3_MESSAGE_TAGS_H
#define FABULOR_IRCV3_MESSAGE_TAGS_H

#include <glib.h>

typedef struct ircv3_message_tags ircv3_message_tags;

ircv3_message_tags *ircv3_message_tags_parse (const char *text);
const char *ircv3_message_tags_lookup (const ircv3_message_tags *tags,
										 const char *key, gboolean *present);
void ircv3_message_tags_free (ircv3_message_tags *tags);

#endif
