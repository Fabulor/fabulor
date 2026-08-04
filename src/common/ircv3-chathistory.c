#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "ircv3-chathistory.h"

unsigned int
ircv3_chathistory_parse_limit (const char *text)
{
	char *end = NULL;
	unsigned long limit;

	if (!text || !*text)
		return IRCV3_CHATHISTORY_DEFAULT_LIMIT;

	errno = 0;
	limit = strtoul (text, &end, 10);
	if (errno != 0 || !end || *end || limit == 0)
		return IRCV3_CHATHISTORY_DEFAULT_LIMIT;

	return (unsigned int)MIN (limit, IRCV3_CHATHISTORY_MAX_LIMIT);
}

char *
ircv3_chathistory_latest_command (const char *target, unsigned int limit)
{
	if (!target || !*target || strpbrk (target, " \r\n"))
		return NULL;

	if (limit == 0)
		limit = IRCV3_CHATHISTORY_DEFAULT_LIMIT;
	limit = MIN (limit, IRCV3_CHATHISTORY_MAX_LIMIT);
	return g_strdup_printf ("CHATHISTORY LATEST %s * %u", target, limit);
}
