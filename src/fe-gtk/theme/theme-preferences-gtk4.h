#ifndef FABULOR_THEME_PREFERENCES_GTK4_H
#define FABULOR_THEME_PREFERENCES_GTK4_H

#include "theme-gtk4-controller.h"

typedef struct _ThemePreferencesGtk4 ThemePreferencesGtk4;

typedef void (*ThemePreferencesGtk4CommitFunc) (const char *theme_id,
	guint variant, gpointer user_data);

ThemePreferencesGtk4 *theme_preferences_gtk4_new (GdkDisplay *display,
	const char *config_dir, const char *stored_id, guint stored_variant,
	gboolean system_prefers_dark, gboolean high_contrast,
	ThemePreferencesGtk4CommitFunc commit, gpointer user_data,
	GDestroyNotify user_data_destroy, GError **error);
void theme_preferences_gtk4_free (ThemePreferencesGtk4 *preferences);

GtkWidget *theme_preferences_gtk4_widget (
	const ThemePreferencesGtk4 *preferences);
gboolean theme_preferences_gtk4_refresh (ThemePreferencesGtk4 *preferences,
	gboolean system_prefers_dark, gboolean high_contrast, GError **error);
gboolean theme_preferences_gtk4_select_theme (
	ThemePreferencesGtk4 *preferences, guint index, GError **error);
gboolean theme_preferences_gtk4_select_variant (
	ThemePreferencesGtk4 *preferences, guint variant, GError **error);

const char *theme_preferences_gtk4_stored_id (
	const ThemePreferencesGtk4 *preferences);
guint theme_preferences_gtk4_stored_variant (
	const ThemePreferencesGtk4 *preferences);
const char *theme_preferences_gtk4_status (
	const ThemePreferencesGtk4 *preferences);
ThemeGtk4Controller *theme_preferences_gtk4_controller (
	const ThemePreferencesGtk4 *preferences);

#endif
