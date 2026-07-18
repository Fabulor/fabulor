#include "gtk4-theme-preferences.h"

void
fabulor_gtk4_theme_choice_free (FabulorGtk4ThemeChoice *choice)
{
	if (!choice)
		return;
	g_free (choice->id);
	g_free (choice->display_name);
	g_free (choice);
}

static FabulorGtk4ThemeChoice *
theme_choice_new (const FabulorGtk4Theme *theme)
{
	FabulorGtk4ThemeChoice *choice = g_new0 (FabulorGtk4ThemeChoice, 1);

	if (!theme)
	{
		choice->id = g_strdup ("");
		choice->system_default = TRUE;
		return choice;
	}
	choice->id = g_strdup (theme->id);
	choice->display_name = g_strdup (theme->display_name);
	choice->source = theme->source;
	choice->has_dark_variant = theme->dark_css_path != NULL;
	return choice;
}

GPtrArray *
fabulor_gtk4_theme_preferences_project (const GPtrArray *themes)
{
	GPtrArray *choices = g_ptr_array_new_with_free_func (
		(GDestroyNotify) fabulor_gtk4_theme_choice_free);
	guint i;

	g_ptr_array_add (choices, theme_choice_new (NULL));
	for (i = 0; themes && i < themes->len; i++)
	{
		const FabulorGtk4Theme *theme = g_ptr_array_index (themes, i);

		if (!theme || !theme->id || !theme->id[0] || !theme->display_name)
			continue;
		g_ptr_array_add (choices, theme_choice_new (theme));
	}
	return choices;
}

guint
fabulor_gtk4_theme_preferences_resolve_index (const GPtrArray *choices,
	const char *stored_id, gboolean *stored_selection_available)
{
	guint i;

	if (stored_selection_available)
		*stored_selection_available = !stored_id || !stored_id[0];
	if (!stored_id || !stored_id[0])
		return 0;
	for (i = 1; choices && i < choices->len; i++)
	{
		const FabulorGtk4ThemeChoice *choice = g_ptr_array_index (choices, i);

		if (choice && g_strcmp0 (choice->id, stored_id) == 0)
		{
			if (stored_selection_available)
				*stored_selection_available = TRUE;
			return i;
		}
	}
	return 0;
}

FabulorGtk4ThemeVariant
fabulor_gtk4_theme_preferences_normalize_variant (guint stored_variant)
{
	if (stored_variant <= FABULOR_GTK4_THEME_VARIANT_PREFER_DARK)
		return (FabulorGtk4ThemeVariant) stored_variant;
	return FABULOR_GTK4_THEME_VARIANT_FOLLOW_SYSTEM;
}
