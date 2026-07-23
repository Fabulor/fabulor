#include "theme-gtk4-controller.h"


struct _ThemeGtk4Controller
{
	ThemeGtk4Adapter *adapter;
	GPtrArray *choices;
	guint selected_index;
	guint selected_variant;
	gboolean stored_selection_available;
	FabulorGtk4ThemeAppearanceDecision appearance;
};

static const FabulorGtk4Theme *
theme_find_by_id (const GPtrArray *themes, const char *id)
{
	guint i;

	for (i = 0; themes && id && id[0] && i < themes->len; i++)
	{
		const FabulorGtk4Theme *theme = g_ptr_array_index (themes, i);

		if (theme && g_strcmp0 (theme->id, id) == 0)
			return theme;
	}
	return NULL;
}

ThemeGtk4Controller *
theme_gtk4_controller_new (GdkDisplay *display)
{
	ThemeGtk4Controller *controller = g_new0 (ThemeGtk4Controller, 1);

	controller->adapter = theme_gtk4_adapter_new (display);
	controller->choices = fabulor_gtk4_theme_preferences_project (NULL);
	controller->stored_selection_available = TRUE;
	return controller;
}

void
theme_gtk4_controller_free (ThemeGtk4Controller *controller)
{
	if (!controller)
		return;
	theme_gtk4_adapter_free (controller->adapter);
	g_ptr_array_unref (controller->choices);
	g_free (controller);
}

gboolean
theme_gtk4_controller_refresh_from_themes (ThemeGtk4Controller *controller,
	const GPtrArray *themes, const char *stored_id, guint stored_variant,
	gboolean system_prefers_dark, gboolean high_contrast, GError **error)
{
	GPtrArray *choices;
	guint selected_index;
	gboolean stored_selection_available;
	FabulorGtk4ThemeAppearanceDecision appearance;
	const FabulorGtk4ThemeChoice *choice;
	const FabulorGtk4Theme *theme = NULL;

	g_return_val_if_fail (controller != NULL, FALSE);
	choices = fabulor_gtk4_theme_preferences_project (themes);
	selected_index = fabulor_gtk4_theme_preferences_resolve_index (choices,
		stored_id, &stored_selection_available);
	choice = g_ptr_array_index (choices, selected_index);
	fabulor_gtk4_theme_preferences_resolve_appearance (
		!choice->system_default, stored_variant, system_prefers_dark,
		high_contrast, &appearance);
	if (appearance.use_custom_theme)
		theme = theme_find_by_id (themes, choice->id);
	if (!theme_gtk4_adapter_apply_decision (controller->adapter, theme,
		&appearance, error))
	{
		g_ptr_array_unref (choices);
		return FALSE;
	}

	g_ptr_array_unref (controller->choices);
	controller->choices = choices;
	controller->selected_index = selected_index;
	controller->selected_variant = appearance.variant;
	controller->stored_selection_available = stored_selection_available;
	controller->appearance = appearance;
	return TRUE;
}

gboolean
theme_gtk4_controller_refresh (ThemeGtk4Controller *controller,
	const char *config_dir, const char *stored_id, guint stored_variant,
	gboolean system_prefers_dark, gboolean high_contrast, GError **error)
{
	GPtrArray *themes;
	gboolean result;

	g_return_val_if_fail (controller != NULL, FALSE);
	themes = fabulor_gtk4_theme_discover (config_dir);
	result = theme_gtk4_controller_refresh_from_themes (controller, themes,
		stored_id, stored_variant, system_prefers_dark, high_contrast, error);
	g_ptr_array_unref (themes);
	return result;
}

const GPtrArray *
theme_gtk4_controller_choices (const ThemeGtk4Controller *controller)
{
	return controller ? controller->choices : NULL;
}

guint
theme_gtk4_controller_selected_index (const ThemeGtk4Controller *controller)
{
	return controller ? controller->selected_index : 0;
}

guint
theme_gtk4_controller_selected_variant (const ThemeGtk4Controller *controller)
{
	return controller ? controller->selected_variant : 0;
}

const FabulorGtk4ThemeChoice *
theme_gtk4_controller_selected_choice (const ThemeGtk4Controller *controller)
{
	if (!controller || !controller->choices ||
		controller->selected_index >= controller->choices->len)
		return NULL;
	return g_ptr_array_index (controller->choices, controller->selected_index);
}

gboolean
theme_gtk4_controller_stored_selection_available (
	const ThemeGtk4Controller *controller)
{
	return controller && controller->stored_selection_available;
}

const FabulorGtk4ThemeAppearanceDecision *
theme_gtk4_controller_appearance (const ThemeGtk4Controller *controller)
{
	return controller ? &controller->appearance : NULL;
}

gboolean
theme_gtk4_controller_theme_is_active (const ThemeGtk4Controller *controller)
{
	return controller && theme_gtk4_adapter_is_active (controller->adapter);
}

const char *
theme_gtk4_controller_active_id (const ThemeGtk4Controller *controller)
{
	return controller ? theme_gtk4_adapter_active_id (controller->adapter) : NULL;
}

guint
theme_gtk4_controller_active_provider_count (
	const ThemeGtk4Controller *controller)
{
	return controller ?
		theme_gtk4_adapter_active_provider_count (controller->adapter) : 0;
}

const char *
theme_gtk4_controller_last_diagnostic (const ThemeGtk4Controller *controller)
{
	return controller ?
		theme_gtk4_adapter_last_diagnostic (controller->adapter) : NULL;
}
