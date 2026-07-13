#ifndef FABULOR_PLUGIN_ENABLE_POLICY_H
#define FABULOR_PLUGIN_ENABLE_POLICY_H

#include <glib.h>

gboolean fabulor_plugin_enable_policy_should_autoload (gboolean preference_enabled,
												const char *environment_enabled,
												gboolean safe_mode);

#endif
