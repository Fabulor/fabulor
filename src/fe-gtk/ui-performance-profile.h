#ifndef FABULOR_UI_PERFORMANCE_PROFILE_H
#define FABULOR_UI_PERFORMANCE_PROFILE_H

#include <glib.h>

gboolean fabulor_ui_profile_enabled (void);
void fabulor_ui_profile_log (const char *category, const char *format, ...)
	G_GNUC_PRINTF (2, 3);

#endif
