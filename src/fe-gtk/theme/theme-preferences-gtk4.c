#include "theme-preferences-gtk4.h"


struct _ThemePreferencesGtk4
{
	ThemeGtk4Controller *controller;
	gboolean owns_controller;
	GtkWidget *root;
	GtkDropDown *theme_dropdown;
	GtkDropDown *variant_dropdown;
	GtkLabel *status_label;
	gulong theme_changed_id;
	gulong variant_changed_id;
	char *config_dir;
	char *stored_id;
	char *status;
	guint stored_variant;
	gboolean system_prefers_dark;
	gboolean high_contrast;
	gboolean blocked;
	ThemePreferencesGtk4CommitFunc commit;
	gpointer user_data;
	GDestroyNotify user_data_destroy;
};

static void theme_preferences_gtk4_sync (ThemePreferencesGtk4 *preferences);

static GtkStringList *
theme_preferences_gtk4_theme_model (const ThemePreferencesGtk4 *preferences)
{
	const GPtrArray *choices = theme_gtk4_controller_choices (
		preferences->controller);
	GtkStringList *model = gtk_string_list_new (NULL);
	guint i;

	for (i = 0; choices && i < choices->len; i++)
	{
		const FabulorGtk4ThemeChoice *choice = g_ptr_array_index (choices, i);

		gtk_string_list_append (model, choice->system_default ?
			"System default" : choice->display_name);
	}
	return model;
}

static void
theme_preferences_gtk4_update_status (ThemePreferencesGtk4 *preferences,
	const char *error_message)
{
	const char *message = error_message;

	if (!message && !theme_gtk4_controller_stored_selection_available (
		preferences->controller))
		message = "The saved GTK4 theme is unavailable; system default is active.";
	else if (!message && preferences->high_contrast)
		message = "High contrast is using the system GTK4 appearance.";
	g_free (preferences->status);
	preferences->status = g_strdup (message);
	gtk_label_set_text (preferences->status_label, message ? message : "");
	gtk_widget_set_visible (GTK_WIDGET (preferences->status_label),
		message != NULL);
}

static gboolean
theme_preferences_gtk4_apply (ThemePreferencesGtk4 *preferences,
	const char *theme_id, guint variant, gboolean notify, GError **error)
{
	const FabulorGtk4ThemeChoice *choice;
	guint normalized_variant =
		fabulor_gtk4_theme_preferences_normalize_variant (variant);

	if (!theme_gtk4_controller_refresh (preferences->controller,
		preferences->config_dir, theme_id, normalized_variant,
		preferences->system_prefers_dark, preferences->high_contrast, error))
	{
		theme_preferences_gtk4_update_status (preferences,
			error && *error ? (*error)->message :
			"The selected GTK4 theme could not be applied.");
		theme_preferences_gtk4_sync (preferences);
		return FALSE;
	}

	choice = theme_gtk4_controller_selected_choice (preferences->controller);
	g_free (preferences->stored_id);
	preferences->stored_id = g_strdup (choice ? choice->id : "");
	preferences->stored_variant = normalized_variant;
	theme_preferences_gtk4_sync (preferences);
	theme_preferences_gtk4_update_status (preferences, NULL);
	if (notify && preferences->commit)
		preferences->commit (preferences->stored_id,
			preferences->stored_variant, preferences->user_data);
	return TRUE;
}

static void
theme_preferences_gtk4_sync (ThemePreferencesGtk4 *preferences)
{
	GtkStringList *model = theme_preferences_gtk4_theme_model (preferences);
	guint selected = theme_gtk4_controller_selected_index (
		preferences->controller);

	preferences->blocked = TRUE;
	gtk_drop_down_set_model (preferences->theme_dropdown, G_LIST_MODEL (model));
	gtk_drop_down_set_selected (preferences->theme_dropdown, selected);
	gtk_drop_down_set_selected (preferences->variant_dropdown,
		preferences->stored_variant);
	gtk_widget_set_sensitive (GTK_WIDGET (preferences->variant_dropdown),
		selected != 0);
	preferences->blocked = FALSE;
}

gboolean
theme_preferences_gtk4_select_theme (ThemePreferencesGtk4 *preferences,
	guint index, GError **error)
{
	const GPtrArray *choices;
	const FabulorGtk4ThemeChoice *choice;
	char *id;
	gboolean result;

	g_return_val_if_fail (preferences != NULL, FALSE);
	choices = theme_gtk4_controller_choices (preferences->controller);
	if (!choices || index >= choices->len)
	{
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,
			"The selected GTK4 theme index is invalid.");
		return FALSE;
	}
	choice = g_ptr_array_index (choices, index);
	id = g_strdup (choice->id);
	result = theme_preferences_gtk4_apply (preferences, id,
		preferences->stored_variant, TRUE, error);
	g_free (id);
	return result;
}

gboolean
theme_preferences_gtk4_select_variant (ThemePreferencesGtk4 *preferences,
	guint variant, GError **error)
{
	g_return_val_if_fail (preferences != NULL, FALSE);
	return theme_preferences_gtk4_apply (preferences, preferences->stored_id,
		variant, TRUE, error);
}

static void
theme_preferences_gtk4_theme_changed (GtkDropDown *dropdown,
	GParamSpec *pspec, gpointer user_data)
{
	ThemePreferencesGtk4 *preferences = user_data;
	GError *error = NULL;
	(void) pspec;

	if (preferences->blocked)
		return;
	theme_preferences_gtk4_select_theme (preferences,
		gtk_drop_down_get_selected (dropdown), &error);
	g_clear_error (&error);
}

static void
theme_preferences_gtk4_variant_changed (GtkDropDown *dropdown,
	GParamSpec *pspec, gpointer user_data)
{
	ThemePreferencesGtk4 *preferences = user_data;
	GError *error = NULL;
	(void) pspec;

	if (preferences->blocked)
		return;
	theme_preferences_gtk4_select_variant (preferences,
		gtk_drop_down_get_selected (dropdown), &error);
	g_clear_error (&error);
}

static GtkWidget *
theme_preferences_gtk4_row (const char *title, GtkWidget *control)
{
	GtkWidget *row = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
	GtkWidget *label = gtk_label_new (title);

	gtk_widget_set_hexpand (label, TRUE);
	gtk_label_set_xalign (GTK_LABEL (label), 0.0f);
	gtk_box_append (GTK_BOX (row), label);
	gtk_box_append (GTK_BOX (row), control);
	return row;
}

static ThemePreferencesGtk4 *
theme_preferences_gtk4_new_internal (ThemeGtk4Controller *controller,
	gboolean owns_controller, const char *config_dir,
	const char *stored_id, guint stored_variant, gboolean system_prefers_dark,
	gboolean high_contrast, ThemePreferencesGtk4CommitFunc commit,
	gpointer user_data, GDestroyNotify user_data_destroy, GError **error)
{
	static const char *variants[] = {
		"Follow system", "Prefer light", "Prefer dark", NULL
	};
	ThemePreferencesGtk4 *preferences = g_new0 (ThemePreferencesGtk4, 1);
	GtkStringList *theme_model;
	GtkStringList *variant_model;

	preferences->controller = controller;
	preferences->owns_controller = owns_controller;
	preferences->config_dir = g_strdup (config_dir);
	preferences->stored_id = g_strdup (stored_id ? stored_id : "");
	preferences->stored_variant =
		fabulor_gtk4_theme_preferences_normalize_variant (stored_variant);
	preferences->system_prefers_dark = system_prefers_dark;
	preferences->high_contrast = high_contrast;
	preferences->commit = commit;
	preferences->user_data = user_data;
	preferences->user_data_destroy = user_data_destroy;
	if (!theme_gtk4_controller_refresh (preferences->controller, config_dir,
		preferences->stored_id, preferences->stored_variant,
		system_prefers_dark, high_contrast, error))
	{
		theme_preferences_gtk4_free (preferences);
		return NULL;
	}

	preferences->root = g_object_ref_sink (
		gtk_box_new (GTK_ORIENTATION_VERTICAL, 12));
	theme_model = theme_preferences_gtk4_theme_model (preferences);
	preferences->theme_dropdown = GTK_DROP_DOWN (gtk_drop_down_new (
		G_LIST_MODEL (theme_model), NULL));
	variant_model = gtk_string_list_new (variants);
	preferences->variant_dropdown = GTK_DROP_DOWN (gtk_drop_down_new (
		G_LIST_MODEL (variant_model), NULL));
	preferences->status_label = GTK_LABEL (gtk_label_new (NULL));
	gtk_label_set_wrap (preferences->status_label, TRUE);
	gtk_label_set_xalign (preferences->status_label, 0.0f);
	gtk_box_append (GTK_BOX (preferences->root), theme_preferences_gtk4_row (
		"Desktop theme", GTK_WIDGET (preferences->theme_dropdown)));
	gtk_box_append (GTK_BOX (preferences->root), theme_preferences_gtk4_row (
		"Variant", GTK_WIDGET (preferences->variant_dropdown)));
	gtk_box_append (GTK_BOX (preferences->root),
		GTK_WIDGET (preferences->status_label));
	preferences->theme_changed_id = g_signal_connect (
		preferences->theme_dropdown, "notify::selected",
		G_CALLBACK (theme_preferences_gtk4_theme_changed), preferences);
	preferences->variant_changed_id = g_signal_connect (
		preferences->variant_dropdown, "notify::selected",
		G_CALLBACK (theme_preferences_gtk4_variant_changed), preferences);
	theme_preferences_gtk4_sync (preferences);
	theme_preferences_gtk4_update_status (preferences, NULL);
	return preferences;
}

ThemePreferencesGtk4 *
theme_preferences_gtk4_new (GdkDisplay *display, const char *config_dir,
	const char *stored_id, guint stored_variant, gboolean system_prefers_dark,
	gboolean high_contrast, ThemePreferencesGtk4CommitFunc commit,
	gpointer user_data, GDestroyNotify user_data_destroy, GError **error)
{
	return theme_preferences_gtk4_new_internal (
		theme_gtk4_controller_new (display), TRUE, config_dir, stored_id,
		stored_variant, system_prefers_dark, high_contrast, commit, user_data,
		user_data_destroy, error);
}

ThemePreferencesGtk4 *
theme_preferences_gtk4_new_with_controller (ThemeGtk4Controller *controller,
	const char *config_dir, const char *stored_id, guint stored_variant,
	gboolean system_prefers_dark, gboolean high_contrast,
	ThemePreferencesGtk4CommitFunc commit, gpointer user_data,
	GDestroyNotify user_data_destroy, GError **error)
{
	g_return_val_if_fail (controller != NULL, NULL);
	return theme_preferences_gtk4_new_internal (controller, FALSE, config_dir,
		stored_id, stored_variant, system_prefers_dark, high_contrast, commit,
		user_data, user_data_destroy, error);
}

void
theme_preferences_gtk4_free (ThemePreferencesGtk4 *preferences)
{
	GtkWidget *parent;

	if (!preferences)
		return;
	if (preferences->theme_changed_id && preferences->theme_dropdown)
		g_signal_handler_disconnect (preferences->theme_dropdown,
			preferences->theme_changed_id);
	if (preferences->variant_changed_id && preferences->variant_dropdown)
		g_signal_handler_disconnect (preferences->variant_dropdown,
			preferences->variant_changed_id);
	if (preferences->root)
	{
		parent = gtk_widget_get_parent (preferences->root);
		if (parent)
			gtk_widget_unparent (preferences->root);
		g_object_unref (preferences->root);
	}
	if (preferences->owns_controller)
		theme_gtk4_controller_free (preferences->controller);
	if (preferences->user_data_destroy)
		preferences->user_data_destroy (preferences->user_data);
	g_free (preferences->status);
	g_free (preferences->stored_id);
	g_free (preferences->config_dir);
	g_free (preferences);
}

GtkWidget *
theme_preferences_gtk4_widget (const ThemePreferencesGtk4 *preferences)
{
	return preferences ? preferences->root : NULL;
}

gboolean
theme_preferences_gtk4_refresh (ThemePreferencesGtk4 *preferences,
	gboolean system_prefers_dark, gboolean high_contrast, GError **error)
{
	gboolean previous_system_prefers_dark;
	gboolean previous_high_contrast;
	gboolean result;

	g_return_val_if_fail (preferences != NULL, FALSE);
	previous_system_prefers_dark = preferences->system_prefers_dark;
	previous_high_contrast = preferences->high_contrast;
	preferences->system_prefers_dark = system_prefers_dark;
	preferences->high_contrast = high_contrast;
	result = theme_preferences_gtk4_apply (preferences, preferences->stored_id,
		preferences->stored_variant, FALSE, error);
	if (!result)
	{
		preferences->system_prefers_dark = previous_system_prefers_dark;
		preferences->high_contrast = previous_high_contrast;
	}
	return result;
}

const char *
theme_preferences_gtk4_stored_id (const ThemePreferencesGtk4 *preferences)
{
	return preferences ? preferences->stored_id : NULL;
}

guint
theme_preferences_gtk4_stored_variant (
	const ThemePreferencesGtk4 *preferences)
{
	return preferences ? preferences->stored_variant : 0;
}

const char *
theme_preferences_gtk4_status (const ThemePreferencesGtk4 *preferences)
{
	return preferences ? preferences->status : NULL;
}

ThemeGtk4Controller *
theme_preferences_gtk4_controller (const ThemePreferencesGtk4 *preferences)
{
	return preferences ? preferences->controller : NULL;
}
