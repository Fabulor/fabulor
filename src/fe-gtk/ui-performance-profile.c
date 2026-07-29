#include "ui-performance-profile.h"

#include <stdarg.h>
#include <stdio.h>

#include <glib/gstdio.h>

#include "../common/cfgfiles.h"

static gboolean profile_checked;
static gboolean profile_enabled;
static FILE *profile_file;

gboolean
fabulor_ui_profile_enabled (void)
{
	const char *value;

	if (profile_checked)
		return profile_enabled;

	profile_checked = TRUE;
	value = g_getenv ("FABULOR_PROFILE_UI");
	profile_enabled = value && value[0] != '\0' &&
		g_ascii_strcasecmp (value, "0") != 0 &&
		g_ascii_strcasecmp (value, "false") != 0 &&
		g_ascii_strcasecmp (value, "off") != 0;
	return profile_enabled;
}

void
fabulor_ui_profile_log (const char *category, const char *format, ...)
{
	GDateTime *now;
	char *timestamp;
	char *message;
	char *path;
	va_list args;

	if (!fabulor_ui_profile_enabled ())
		return;

	if (!profile_file)
	{
		path = g_build_filename (get_xdir (), "ui-performance.log", NULL);
		profile_file = g_fopen (path, "a");
		g_free (path);
		if (!profile_file)
			return;
	}

	va_start (args, format);
	message = g_strdup_vprintf (format, args);
	va_end (args);
	now = g_date_time_new_now_local ();
	timestamp = g_date_time_format (now, "%Y-%m-%dT%H:%M:%S.%f%z");
	fprintf (profile_file, "%s [%s] %s\n", timestamp,
		category ? category : "ui", message);
	fflush (profile_file);
	g_free (timestamp);
	g_date_time_unref (now);
	g_free (message);
}
