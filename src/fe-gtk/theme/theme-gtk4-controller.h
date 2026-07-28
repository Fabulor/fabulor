#ifndef FABULOR_THEME_GTK4_CONTROLLER_H
#define FABULOR_THEME_GTK4_CONTROLLER_H

#include "theme-gtk4.h"

typedef struct _ThemeGtk4Controller ThemeGtk4Controller;

ThemeGtk4Controller *theme_gtk4_controller_new (GdkDisplay *display);
void theme_gtk4_controller_free (ThemeGtk4Controller *controller);
gboolean theme_gtk4_controller_refresh (ThemeGtk4Controller *controller,
	const char *config_dir, const char *stored_id, guint stored_variant,
	gboolean system_prefers_dark, gboolean high_contrast, GError **error);
void theme_gtk4_controller_reload_catalog (ThemeGtk4Controller *controller,
	const char *config_dir, const char *stored_id, guint stored_variant,
	gboolean system_prefers_dark, gboolean high_contrast);
gboolean theme_gtk4_controller_refresh_from_themes (
	ThemeGtk4Controller *controller, const GPtrArray *themes,
	const char *stored_id, guint stored_variant, gboolean system_prefers_dark,
	gboolean high_contrast, GError **error);

const GPtrArray *theme_gtk4_controller_choices (
	const ThemeGtk4Controller *controller);
guint theme_gtk4_controller_selected_index (
	const ThemeGtk4Controller *controller);
guint theme_gtk4_controller_selected_variant (
	const ThemeGtk4Controller *controller);
const FabulorGtk4ThemeChoice *theme_gtk4_controller_selected_choice (
	const ThemeGtk4Controller *controller);
gboolean theme_gtk4_controller_stored_selection_available (
	const ThemeGtk4Controller *controller);
const FabulorGtk4ThemeAppearanceDecision *theme_gtk4_controller_appearance (
	const ThemeGtk4Controller *controller);
gboolean theme_gtk4_controller_theme_is_active (
	const ThemeGtk4Controller *controller);
const char *theme_gtk4_controller_active_id (
	const ThemeGtk4Controller *controller);
guint theme_gtk4_controller_active_provider_count (
	const ThemeGtk4Controller *controller);
const char *theme_gtk4_controller_last_diagnostic (
	const ThemeGtk4Controller *controller);

#endif
