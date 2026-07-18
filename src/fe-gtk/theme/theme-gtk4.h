#ifndef FABULOR_THEME_GTK4_H
#define FABULOR_THEME_GTK4_H

#include <gtk/gtk.h>

#include "../../common/gtk4-theme-discovery.h"
#include "../../common/gtk4-theme-preferences.h"

typedef FabulorGtk4ThemeVariant ThemeGtk4Variant;

#define THEME_GTK4_VARIANT_FOLLOW_SYSTEM FABULOR_GTK4_THEME_VARIANT_FOLLOW_SYSTEM
#define THEME_GTK4_VARIANT_PREFER_LIGHT FABULOR_GTK4_THEME_VARIANT_PREFER_LIGHT
#define THEME_GTK4_VARIANT_PREFER_DARK FABULOR_GTK4_THEME_VARIANT_PREFER_DARK

typedef struct _ThemeGtk4Adapter ThemeGtk4Adapter;

ThemeGtk4Adapter *theme_gtk4_adapter_new (GdkDisplay *display);
void theme_gtk4_adapter_free (ThemeGtk4Adapter *adapter);
gboolean theme_gtk4_adapter_apply (ThemeGtk4Adapter *adapter,
	const FabulorGtk4Theme *theme, ThemeGtk4Variant variant,
	gboolean system_prefers_dark, GError **error);
gboolean theme_gtk4_adapter_apply_decision (ThemeGtk4Adapter *adapter,
	const FabulorGtk4Theme *theme,
	const FabulorGtk4ThemeAppearanceDecision *decision, GError **error);
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
