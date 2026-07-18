#ifndef FABULOR_THEME_GTK4_H
#define FABULOR_THEME_GTK4_H

#include <gtk/gtk.h>

#include "../../common/gtk4-theme-discovery.h"

typedef enum
{
	THEME_GTK4_VARIANT_FOLLOW_SYSTEM = 0,
	THEME_GTK4_VARIANT_PREFER_LIGHT = 1,
	THEME_GTK4_VARIANT_PREFER_DARK = 2
} ThemeGtk4Variant;

typedef struct _ThemeGtk4Adapter ThemeGtk4Adapter;

ThemeGtk4Adapter *theme_gtk4_adapter_new (GdkDisplay *display);
void theme_gtk4_adapter_free (ThemeGtk4Adapter *adapter);
gboolean theme_gtk4_adapter_apply (ThemeGtk4Adapter *adapter,
	const FabulorGtk4Theme *theme, ThemeGtk4Variant variant,
	gboolean system_prefers_dark, GError **error);
void theme_gtk4_adapter_disable (ThemeGtk4Adapter *adapter);

gboolean theme_gtk4_variant_uses_dark (ThemeGtk4Variant variant,
	gboolean system_prefers_dark);
gboolean theme_gtk4_adapter_is_active (const ThemeGtk4Adapter *adapter);
const char *theme_gtk4_adapter_active_id (const ThemeGtk4Adapter *adapter);
ThemeGtk4Variant theme_gtk4_adapter_active_variant (
	const ThemeGtk4Adapter *adapter);
guint theme_gtk4_adapter_active_provider_count (
	const ThemeGtk4Adapter *adapter);
guint theme_gtk4_adapter_error_count (const ThemeGtk4Adapter *adapter);
guint theme_gtk4_adapter_warning_count (const ThemeGtk4Adapter *adapter);
const char *theme_gtk4_adapter_last_diagnostic (
	const ThemeGtk4Adapter *adapter);

#endif
