#ifndef FABULOR_GTK4_THEME_DISCOVERY_H
#define FABULOR_GTK4_THEME_DISCOVERY_H

#include <glib.h>

typedef enum
{
	FABULOR_GTK4_THEME_SOURCE_DESKTOP = 0,
	FABULOR_GTK4_THEME_SOURCE_PROFILE = 1
} FabulorGtk4ThemeSource;

typedef struct
{
	char *id;
	char *display_name;
	char *path;
	char *css_path;
	char *dark_css_path;
	char *thumbnail_path;
	FabulorGtk4ThemeSource source;
} FabulorGtk4Theme;

char *fabulor_gtk4_theme_profile_dir (const char *config_dir);
GPtrArray *fabulor_gtk4_theme_discover (const char *config_dir);
GPtrArray *fabulor_gtk4_theme_discover_roots (
	const char *profile_root, const char *const *desktop_roots);
void fabulor_gtk4_theme_free (FabulorGtk4Theme *theme);

#endif
