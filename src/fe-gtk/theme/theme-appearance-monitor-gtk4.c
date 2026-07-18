#include "theme-appearance-monitor-gtk4.h"

#if GTK_MAJOR_VERSION < 4
#error The GTK4 appearance monitor must compile against GTK 4.
#endif

#ifdef G_OS_WIN32
#include <gdk/win32/gdkwin32.h>
#include <windows.h>
#endif

struct _ThemeAppearanceMonitorGtk4
{
	GdkDisplay *display;
	ThemePreferencesGtk4 *preferences;
	ThemeAppearanceGtk4QueryFunc query;
	gpointer user_data;
	GDestroyNotify user_data_destroy;
	guint refresh_source_id;
	guint refresh_count;
	char *last_diagnostic;
	gboolean prefer_dark;
	gboolean high_contrast;
	gboolean state_known;
	gboolean filter_installed;
};

static gboolean
theme_appearance_monitor_gtk4_settings_prefers_dark (void)
{
	GtkSettings *settings = gtk_settings_get_default ();
	GParamSpec *property;
	gboolean prefer_dark = FALSE;

	if (!settings)
		return FALSE;
	property = g_object_class_find_property (G_OBJECT_GET_CLASS (settings),
		"gtk-application-prefer-dark-theme");
	if (property)
		g_object_get (settings, "gtk-application-prefer-dark-theme",
			&prefer_dark, NULL);
	return prefer_dark;
}

static gboolean
theme_appearance_monitor_gtk4_platform_query (gboolean *prefer_dark,
	gboolean *high_contrast, gpointer user_data)
{
	(void) user_data;
#ifdef G_OS_WIN32
	HIGHCONTRASTW contrast = { 0 };
	DWORD value = 1;
	DWORD value_size = sizeof (value);
	LSTATUS status;

	contrast.cbSize = sizeof (contrast);
	*high_contrast = SystemParametersInfoW (SPI_GETHIGHCONTRAST,
		sizeof (contrast), &contrast, 0) &&
		(contrast.dwFlags & HCF_HIGHCONTRASTON) != 0;
	status = RegGetValueW (HKEY_CURRENT_USER,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
		L"AppsUseLightTheme", RRF_RT_REG_DWORD, NULL, &value, &value_size);
	if (status != ERROR_SUCCESS)
	{
		value_size = sizeof (value);
		status = RegGetValueW (HKEY_CURRENT_USER,
			L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
			L"SystemUsesLightTheme", RRF_RT_REG_DWORD, NULL, &value,
			&value_size);
	}
	*prefer_dark = status == ERROR_SUCCESS ? value == 0 :
		theme_appearance_monitor_gtk4_settings_prefers_dark ();
	return TRUE;
#else
	*prefer_dark = theme_appearance_monitor_gtk4_settings_prefers_dark ();
	*high_contrast = FALSE;
	return TRUE;
#endif
}

gboolean
theme_appearance_monitor_gtk4_refresh_now (
	ThemeAppearanceMonitorGtk4 *monitor, GError **error)
{
	gboolean prefer_dark;
	gboolean high_contrast;

	g_return_val_if_fail (monitor != NULL, FALSE);
	if (!monitor->query (&prefer_dark, &high_contrast, monitor->user_data))
	{
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED,
			"The system appearance could not be queried.");
		g_free (monitor->last_diagnostic);
		monitor->last_diagnostic = g_strdup (
			"The system appearance could not be queried.");
		return FALSE;
	}
	prefer_dark = prefer_dark ? TRUE : FALSE;
	high_contrast = high_contrast ? TRUE : FALSE;
	if (monitor->state_known && monitor->prefer_dark == prefer_dark &&
		monitor->high_contrast == high_contrast)
	{
		g_clear_pointer (&monitor->last_diagnostic, g_free);
		return TRUE;
	}
	if (!theme_preferences_gtk4_refresh (monitor->preferences, prefer_dark,
		high_contrast, error))
	{
		g_free (monitor->last_diagnostic);
		monitor->last_diagnostic = g_strdup (error && *error ?
			(*error)->message : "The GTK4 appearance could not be refreshed.");
		return FALSE;
	}
	monitor->prefer_dark = prefer_dark;
	monitor->high_contrast = high_contrast;
	monitor->state_known = TRUE;
	monitor->refresh_count++;
	g_clear_pointer (&monitor->last_diagnostic, g_free);
	return TRUE;
}

static gboolean
theme_appearance_monitor_gtk4_refresh_idle (gpointer user_data)
{
	ThemeAppearanceMonitorGtk4 *monitor = user_data;
	GError *error = NULL;

	monitor->refresh_source_id = 0;
	theme_appearance_monitor_gtk4_refresh_now (monitor, &error);
	g_clear_error (&error);
	return G_SOURCE_REMOVE;
}

gboolean
theme_appearance_monitor_gtk4_queue_refresh (
	ThemeAppearanceMonitorGtk4 *monitor)
{
	g_return_val_if_fail (monitor != NULL, FALSE);
	if (monitor->refresh_source_id)
		return FALSE;
	monitor->refresh_source_id = g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
		theme_appearance_monitor_gtk4_refresh_idle, monitor, NULL);
	return monitor->refresh_source_id != 0;
}

#ifdef G_OS_WIN32
static GdkWin32MessageFilterReturn
theme_appearance_monitor_gtk4_filter (GdkWin32Display *display, MSG *message,
	int *return_value, gpointer user_data)
{
	ThemeAppearanceMonitorGtk4 *monitor = user_data;
	(void) display;
	(void) return_value;

	if (message && (message->message == WM_SETTINGCHANGE ||
		message->message == WM_THEMECHANGED))
		theme_appearance_monitor_gtk4_queue_refresh (monitor);
	return GDK_WIN32_MESSAGE_FILTER_CONTINUE;
}
#endif

ThemeAppearanceMonitorGtk4 *
theme_appearance_monitor_gtk4_new_with_query (GdkDisplay *display,
	ThemePreferencesGtk4 *preferences, ThemeAppearanceGtk4QueryFunc query,
	gpointer user_data, GDestroyNotify user_data_destroy, GError **error)
{
	ThemeAppearanceMonitorGtk4 *monitor;

	g_return_val_if_fail (preferences != NULL, NULL);
	g_return_val_if_fail (query != NULL, NULL);
	monitor = g_new0 (ThemeAppearanceMonitorGtk4, 1);
	monitor->display = display ? g_object_ref (display) : NULL;
	monitor->preferences = preferences;
	monitor->query = query;
	monitor->user_data = user_data;
	monitor->user_data_destroy = user_data_destroy;
#ifdef G_OS_WIN32
	if (display && GDK_IS_WIN32_DISPLAY (display))
	{
		gdk_win32_display_add_filter (GDK_WIN32_DISPLAY (display),
			theme_appearance_monitor_gtk4_filter, monitor);
		monitor->filter_installed = TRUE;
	}
#endif
	if (!theme_appearance_monitor_gtk4_refresh_now (monitor, error))
	{
		theme_appearance_monitor_gtk4_free (monitor);
		return NULL;
	}
	return monitor;
}

ThemeAppearanceMonitorGtk4 *
theme_appearance_monitor_gtk4_new (GdkDisplay *display,
	ThemePreferencesGtk4 *preferences, GError **error)
{
	return theme_appearance_monitor_gtk4_new_with_query (display, preferences,
		theme_appearance_monitor_gtk4_platform_query, NULL, NULL, error);
}

void
theme_appearance_monitor_gtk4_free (ThemeAppearanceMonitorGtk4 *monitor)
{
	if (!monitor)
		return;
	if (monitor->refresh_source_id)
		g_source_remove (monitor->refresh_source_id);
#ifdef G_OS_WIN32
	if (monitor->filter_installed)
		gdk_win32_display_remove_filter (
			GDK_WIN32_DISPLAY (monitor->display),
			theme_appearance_monitor_gtk4_filter, monitor);
#endif
	if (monitor->user_data_destroy)
		monitor->user_data_destroy (monitor->user_data);
	g_clear_object (&monitor->display);
	g_free (monitor->last_diagnostic);
	g_free (monitor);
}

gboolean
theme_appearance_monitor_gtk4_is_pending (
	const ThemeAppearanceMonitorGtk4 *monitor)
{
	return monitor && monitor->refresh_source_id != 0;
}

gboolean
theme_appearance_monitor_gtk4_filter_is_installed (
	const ThemeAppearanceMonitorGtk4 *monitor)
{
	return monitor && monitor->filter_installed;
}

guint
theme_appearance_monitor_gtk4_refresh_count (
	const ThemeAppearanceMonitorGtk4 *monitor)
{
	return monitor ? monitor->refresh_count : 0;
}

gboolean
theme_appearance_monitor_gtk4_prefers_dark (
	const ThemeAppearanceMonitorGtk4 *monitor)
{
	return monitor && monitor->prefer_dark;
}

gboolean
theme_appearance_monitor_gtk4_high_contrast (
	const ThemeAppearanceMonitorGtk4 *monitor)
{
	return monitor && monitor->high_contrast;
}

const char *
theme_appearance_monitor_gtk4_last_diagnostic (
	const ThemeAppearanceMonitorGtk4 *monitor)
{
	return monitor ? monitor->last_diagnostic : NULL;
}
