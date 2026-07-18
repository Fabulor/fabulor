#ifndef FABULOR_GTK4_THEME_PREFERENCES_H
#define FABULOR_GTK4_THEME_PREFERENCES_H

#include "gtk4-theme-discovery.h"

typedef enum
{
	FABULOR_GTK4_THEME_VARIANT_FOLLOW_SYSTEM = 0,
	FABULOR_GTK4_THEME_VARIANT_PREFER_LIGHT = 1,
	FABULOR_GTK4_THEME_VARIANT_PREFER_DARK = 2
} FabulorGtk4ThemeVariant;

typedef struct
{
	char *id;
	char *display_name;
	FabulorGtk4ThemeSource source;
	gboolean system_default;
	gboolean has_dark_variant;
} FabulorGtk4ThemeChoice;

GPtrArray *fabulor_gtk4_theme_preferences_project (const GPtrArray *themes);
guint fabulor_gtk4_theme_preferences_resolve_index (const GPtrArray *choices,
	const char *stored_id, gboolean *stored_selection_available);
FabulorGtk4ThemeVariant fabulor_gtk4_theme_preferences_normalize_variant (
	guint stored_variant);
void fabulor_gtk4_theme_choice_free (FabulorGtk4ThemeChoice *choice);

#endif
