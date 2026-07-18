#ifndef FABULOR_THEME_APPEARANCE_MONITOR_GTK4_H
#define FABULOR_THEME_APPEARANCE_MONITOR_GTK4_H

#include "theme-preferences-gtk4.h"

typedef struct _ThemeAppearanceMonitorGtk4 ThemeAppearanceMonitorGtk4;

typedef gboolean (*ThemeAppearanceGtk4QueryFunc) (gboolean *prefer_dark,
	gboolean *high_contrast, gpointer user_data);

ThemeAppearanceMonitorGtk4 *theme_appearance_monitor_gtk4_new (
	GdkDisplay *display, ThemePreferencesGtk4 *preferences, GError **error);
ThemeAppearanceMonitorGtk4 *theme_appearance_monitor_gtk4_new_with_query (
	GdkDisplay *display, ThemePreferencesGtk4 *preferences,
	ThemeAppearanceGtk4QueryFunc query, gpointer user_data,
	GDestroyNotify user_data_destroy, GError **error);
void theme_appearance_monitor_gtk4_free (
	ThemeAppearanceMonitorGtk4 *monitor);

gboolean theme_appearance_monitor_gtk4_queue_refresh (
	ThemeAppearanceMonitorGtk4 *monitor);
gboolean theme_appearance_monitor_gtk4_refresh_now (
	ThemeAppearanceMonitorGtk4 *monitor, GError **error);
gboolean theme_appearance_monitor_gtk4_is_pending (
	const ThemeAppearanceMonitorGtk4 *monitor);
gboolean theme_appearance_monitor_gtk4_filter_is_installed (
	const ThemeAppearanceMonitorGtk4 *monitor);
guint theme_appearance_monitor_gtk4_refresh_count (
	const ThemeAppearanceMonitorGtk4 *monitor);
gboolean theme_appearance_monitor_gtk4_prefers_dark (
	const ThemeAppearanceMonitorGtk4 *monitor);
gboolean theme_appearance_monitor_gtk4_high_contrast (
	const ThemeAppearanceMonitorGtk4 *monitor);
const char *theme_appearance_monitor_gtk4_last_diagnostic (
	const ThemeAppearanceMonitorGtk4 *monitor);

#endif
