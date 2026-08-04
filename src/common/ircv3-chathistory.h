#ifndef FABULOR_IRCV3_CHATHISTORY_H
#define FABULOR_IRCV3_CHATHISTORY_H

#include <glib.h>

#define IRCV3_CHATHISTORY_DEFAULT_LIMIT 50
#define IRCV3_CHATHISTORY_MAX_LIMIT 500

unsigned int ircv3_chathistory_parse_limit (const char *text);
char *ircv3_chathistory_latest_command (const char *target,
										 unsigned int limit);

#endif
