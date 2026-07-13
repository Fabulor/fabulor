#include "fabulor-plugin-enable-policy.h"

gboolean
fabulor_plugin_enable_policy_should_autoload (gboolean preference_enabled,
										  const char *environment_enabled,
										  gboolean safe_mode)
{
	if (safe_mode)
	{
		return FALSE;
	}

	return preference_enabled
		|| (environment_enabled && g_ascii_strcasecmp (environment_enabled, "1") == 0);
}
