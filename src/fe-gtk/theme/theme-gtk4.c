#include "theme-gtk4.h"


#include <glib/gstdio.h>

struct _ThemeGtk4Adapter
{
	GdkDisplay *display;
	GtkCssProvider *base_provider;
	GtkCssProvider *variant_provider;
	char *active_id;
	ThemeGtk4Variant active_variant;
	guint error_count;
	guint warning_count;
	char *last_diagnostic;
};

static void
theme_gtk4_adapter_clear_diagnostics (ThemeGtk4Adapter *adapter)
{
	adapter->error_count = 0;
	adapter->warning_count = 0;
	g_clear_pointer (&adapter->last_diagnostic, g_free);
}

static void
theme_gtk4_adapter_parsing_error_cb (GtkCssProvider *provider,
	GtkCssSection *section, const GError *error, gpointer user_data)
{
	ThemeGtk4Adapter *adapter = user_data;
	char *section_text;

	(void) provider;
	if (error->domain == GTK_CSS_PARSER_WARNING)
		adapter->warning_count++;
	else
		adapter->error_count++;
	section_text = section ? gtk_css_section_to_string (section) : NULL;
	g_free (adapter->last_diagnostic);
	adapter->last_diagnostic = g_strdup_printf ("%s%s%s",
		section_text ? section_text : "GTK4 CSS",
		section_text ? ": " : "",
		error->message ? error->message : "CSS parser diagnostic");
	g_free (section_text);
}

static GtkCssProvider *
theme_gtk4_adapter_load_provider (ThemeGtk4Adapter *adapter, const char *path,
	GError **error)
{
	GtkCssProvider *provider;
	guint errors_before;

	if (!path || !g_file_test (path, G_FILE_TEST_IS_REGULAR))
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_NOENT,
			"GTK4 theme CSS file was not found: %s", path ? path : "(null)");
		return NULL;
	}
	provider = gtk_css_provider_new ();
	g_signal_connect (provider, "parsing-error",
		G_CALLBACK (theme_gtk4_adapter_parsing_error_cb), adapter);
	errors_before = adapter->error_count;
	gtk_css_provider_load_from_path (provider, path);
	if (adapter->error_count != errors_before)
	{
		g_set_error (error, G_FILE_ERROR, G_FILE_ERROR_FAILED,
			"GTK4 theme CSS failed to parse: %s",
			adapter->last_diagnostic ? adapter->last_diagnostic : path);
		g_object_unref (provider);
		return NULL;
	}
	return provider;
}

static void
theme_gtk4_adapter_remove_active (ThemeGtk4Adapter *adapter)
{
	if (adapter->display && adapter->variant_provider)
		gtk_style_context_remove_provider_for_display (adapter->display,
			GTK_STYLE_PROVIDER (adapter->variant_provider));
	if (adapter->display && adapter->base_provider)
		gtk_style_context_remove_provider_for_display (adapter->display,
			GTK_STYLE_PROVIDER (adapter->base_provider));
	g_clear_object (&adapter->variant_provider);
	g_clear_object (&adapter->base_provider);
	g_clear_pointer (&adapter->active_id, g_free);
	adapter->active_variant = THEME_GTK4_VARIANT_FOLLOW_SYSTEM;
}

ThemeGtk4Adapter *
theme_gtk4_adapter_new (GdkDisplay *display)
{
	ThemeGtk4Adapter *adapter = g_new0 (ThemeGtk4Adapter, 1);

	if (display)
		adapter->display = g_object_ref (display);
	adapter->active_variant = THEME_GTK4_VARIANT_FOLLOW_SYSTEM;
	return adapter;
}

void
theme_gtk4_adapter_free (ThemeGtk4Adapter *adapter)
{
	if (!adapter)
		return;
	theme_gtk4_adapter_remove_active (adapter);
	g_clear_object (&adapter->display);
	g_free (adapter->last_diagnostic);
	g_free (adapter);
}

gboolean
theme_gtk4_variant_uses_dark (ThemeGtk4Variant variant,
	gboolean system_prefers_dark)
{
	if (variant == THEME_GTK4_VARIANT_PREFER_DARK)
		return TRUE;
	if (variant == THEME_GTK4_VARIANT_FOLLOW_SYSTEM)
		return system_prefers_dark;
	return FALSE;
}

gboolean
theme_gtk4_adapter_apply (ThemeGtk4Adapter *adapter,
	const FabulorGtk4Theme *theme, ThemeGtk4Variant variant,
	gboolean system_prefers_dark, GError **error)
{
	GtkCssProvider *base_provider;
	GtkCssProvider *variant_provider = NULL;
	gboolean use_dark;

	g_return_val_if_fail (adapter != NULL, FALSE);
	if (!theme || !theme->id || !theme->id[0] || !theme->css_path ||
		variant < THEME_GTK4_VARIANT_FOLLOW_SYSTEM ||
		variant > THEME_GTK4_VARIANT_PREFER_DARK)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"GTK4 theme application requires valid metadata and variant policy.");
		return FALSE;
	}
	theme_gtk4_adapter_clear_diagnostics (adapter);
	base_provider = theme_gtk4_adapter_load_provider (adapter,
		theme->css_path, error);
	if (!base_provider)
		return FALSE;
	use_dark = theme_gtk4_variant_uses_dark (variant, system_prefers_dark);
	if (use_dark && theme->dark_css_path)
	{
		variant_provider = theme_gtk4_adapter_load_provider (adapter,
			theme->dark_css_path, error);
		if (!variant_provider)
		{
			g_object_unref (base_provider);
			return FALSE;
		}
	}

	theme_gtk4_adapter_remove_active (adapter);
	adapter->base_provider = base_provider;
	adapter->variant_provider = variant_provider;
	adapter->active_id = g_strdup (theme->id);
	adapter->active_variant = variant;
	if (adapter->display)
	{
		gtk_style_context_add_provider_for_display (adapter->display,
			GTK_STYLE_PROVIDER (adapter->base_provider),
			GTK_STYLE_PROVIDER_PRIORITY_USER);
		if (adapter->variant_provider)
			gtk_style_context_add_provider_for_display (adapter->display,
				GTK_STYLE_PROVIDER (adapter->variant_provider),
				GTK_STYLE_PROVIDER_PRIORITY_USER + 1);
	}
	return TRUE;
}

gboolean
theme_gtk4_adapter_apply_decision (ThemeGtk4Adapter *adapter,
	const FabulorGtk4Theme *theme,
	const FabulorGtk4ThemeAppearanceDecision *decision, GError **error)
{
	g_return_val_if_fail (adapter != NULL, FALSE);
	if (!decision)
	{
		g_set_error_literal (error, G_FILE_ERROR, G_FILE_ERROR_INVAL,
			"GTK4 theme application requires an appearance decision.");
		return FALSE;
	}
	if (!decision->use_custom_theme)
	{
		theme_gtk4_adapter_disable (adapter);
		return TRUE;
	}
	return theme_gtk4_adapter_apply (adapter, theme, decision->variant,
		decision->prefer_dark, error);
}

void
theme_gtk4_adapter_disable (ThemeGtk4Adapter *adapter)
{
	if (!adapter)
		return;
	theme_gtk4_adapter_remove_active (adapter);
}

gboolean
theme_gtk4_adapter_is_active (const ThemeGtk4Adapter *adapter)
{
	return adapter && adapter->base_provider != NULL;
}

const char *
theme_gtk4_adapter_active_id (const ThemeGtk4Adapter *adapter)
{
	return adapter ? adapter->active_id : NULL;
}

ThemeGtk4Variant
theme_gtk4_adapter_active_variant (const ThemeGtk4Adapter *adapter)
{
	return adapter ? adapter->active_variant : THEME_GTK4_VARIANT_FOLLOW_SYSTEM;
}

guint
theme_gtk4_adapter_active_provider_count (const ThemeGtk4Adapter *adapter)
{
	if (!adapter)
		return 0;
	return (adapter->base_provider ? 1U : 0U) +
		(adapter->variant_provider ? 1U : 0U);
}

guint
theme_gtk4_adapter_error_count (const ThemeGtk4Adapter *adapter)
{
	return adapter ? adapter->error_count : 0;
}

guint
theme_gtk4_adapter_warning_count (const ThemeGtk4Adapter *adapter)
{
	return adapter ? adapter->warning_count : 0;
}

const char *
theme_gtk4_adapter_last_diagnostic (const ThemeGtk4Adapter *adapter)
{
	return adapter ? adapter->last_diagnostic : NULL;
}
