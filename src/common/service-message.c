#include <glib.h>
#include <string.h>

#include "service-message.h"

static gboolean
service_target_is_nickserv (const char *target)
{
	const char *separator;
	gsize name_length;

	if (!target)
		return FALSE;

	separator = strchr (target, '@');
	name_length = separator ? (gsize)(separator - target) : strlen (target);

	return name_length == strlen ("nickserv")
		&& g_ascii_strncasecmp (target, "nickserv", name_length) == 0;
}

static gboolean
message_starts_with_command (const char *message, const char *command)
{
	gsize command_length = strlen (command);

	return g_ascii_strncasecmp (message, command, command_length) == 0
		&& g_ascii_isspace (message[command_length]);
}

char *
service_message_for_display (const char *target, char *message)
{
	if (!message || !service_target_is_nickserv (target))
		return message;

	if (message_starts_with_command (message, "identify"))
		return "identify ****";
	if (message_starts_with_command (message, "ghost"))
		return "ghost ****";
	if (message_starts_with_command (message, "register"))
		return "register ****";
	if (message_starts_with_command (message, "recover"))
		return "recover ****";
	if (message_starts_with_command (message, "release"))
		return "release ****";
	if (message_starts_with_command (message, "regain"))
		return "regain ****";

	return message;
}
