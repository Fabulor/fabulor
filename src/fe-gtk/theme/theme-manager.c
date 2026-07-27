/* ZoiteChat
 * Copyright (C) 1998-2010 Peter Zelezny.
 * Copyright (C) 2009-2013 Berke Viktor.
 * Copyright (C) 2026 deepend-tildeclub.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "../fe-gtk.h"
#include "theme-manager.h"

#include <gtk/gtk.h>
#include <string.h>

#include "theme-application.h"
#include "theme-policy.h"
#include "theme-runtime.h"
#include "theme-access.h"
#include "theme-css.h"
#include "../gtkutil.h"
#include "../maingui.h"
#include "../setup.h"
#include "../../common/zoitechat.h"
#include "../../common/zoitechatc.h"
#include "../../common/cfgfiles.h"
#include "theme-appearance-monitor-gtk4.h"
#include "theme-gtk4-controller.h"

void theme_runtime_reset_mode_colors (gboolean dark_mode);

typedef struct
{
	guint id;
	char *component_id;
	ThemeChangedCallback callback;
	gpointer userdata;
} ThemeListener;

static GHashTable *theme_manager_listeners;
static guint theme_manager_next_listener_id = 1;
static guint theme_manager_setup_listener_id;
static const char theme_manager_window_destroy_handler_key[] = "theme-manager-window-destroy-handler";
static const char theme_manager_window_weak_owner[] = "theme-manager-window-weak-owner";

typedef struct
{
	gboolean initialized;
	gboolean resolved_dark_preference;
	unsigned int dark_mode;
} ThemeManagerAutoRefreshCache;

static ThemeManagerAutoRefreshCache theme_manager_auto_refresh_cache;
static ThemeGtk4Controller *theme_manager_gtk4_theme_controller;
static ThemeAppearanceMonitorGtk4 *theme_manager_gtk4_appearance_monitor;

static void theme_manager_apply_platform_window_theme (GtkWidget *window);

static gboolean
theme_manager_gtk4_apply_appearance (gboolean prefer_dark,
	gboolean high_contrast, gpointer user_data, GError **error)
{
	const FabulorGtk4ThemeChoice *choice;
	const char *theme_id;
	guint variant;
	(void) user_data;

	if (!theme_manager_gtk4_theme_controller)
		return FALSE;
	choice = theme_gtk4_controller_selected_choice (
		theme_manager_gtk4_theme_controller);
	theme_id = choice ? choice->id : prefs.hex_gui_gtk4_theme;
	variant = choice ? theme_gtk4_controller_selected_variant (
		theme_manager_gtk4_theme_controller) : prefs.hex_gui_gtk4_variant;
	if (!theme_gtk4_controller_refresh (theme_manager_gtk4_theme_controller,
		get_xdir (), theme_id, variant, prefer_dark, high_contrast, error))
		return FALSE;
	if (theme_manager_gtk4_appearance_monitor)
		theme_manager_dispatch_changed (THEME_CHANGED_REASON_THEME_PACK |
			THEME_CHANGED_REASON_WIDGET_STYLE | THEME_CHANGED_REASON_MODE);
	return TRUE;
}

static void
theme_manager_apply_to_toplevel_windows (void)
{
	GList *toplevels;
	GList *iter;

	toplevels = gtk_window_list_toplevels ();
	for (iter = toplevels; iter != NULL; iter = iter->next)
	{
		GtkWidget *window = GTK_WIDGET (iter->data);

		if (!GTK_IS_WINDOW (window) || gtk_widget_get_mapped (window) == FALSE)
			continue;

		theme_manager_apply_platform_window_theme (window);
	}
	g_list_free (toplevels);
}

static void
theme_listener_free (gpointer data)
{
	ThemeListener *listener = data;

	if (!listener)
		return;

	g_free (listener->component_id);
	g_free (listener);
}

static void
theme_manager_setup_apply_listener (const ThemeChangedEvent *event, gpointer userdata)
{
	(void) userdata;
	theme_manager_dispatch_setup_apply (event);
}

static ThemeChangedReason
theme_manager_synthesize_preference_reasons (const struct zoitechatprefs *old_prefs,
					      const struct zoitechatprefs *new_prefs,
					      gboolean color_change)
{
	ThemeChangedReason reasons = THEME_CHANGED_REASON_NONE;

	if (!old_prefs || !new_prefs)
		return reasons;

	if (strcmp (old_prefs->hex_text_background, new_prefs->hex_text_background) != 0)
		reasons |= THEME_CHANGED_REASON_PIXMAP;
	if (old_prefs->hex_gui_tab_dots != new_prefs->hex_gui_tab_dots ||
	    old_prefs->hex_gui_tab_layout != new_prefs->hex_gui_tab_layout)
		reasons |= THEME_CHANGED_REASON_LAYOUT;
	if (color_change ||
	    old_prefs->hex_gui_ulist_color != new_prefs->hex_gui_ulist_color ||
	    old_prefs->hex_text_color_nicks != new_prefs->hex_text_color_nicks ||
	    old_prefs->hex_away_size_max != new_prefs->hex_away_size_max ||
	    old_prefs->hex_away_track != new_prefs->hex_away_track)
		reasons |= THEME_CHANGED_REASON_USERLIST;
	if (strcmp (old_prefs->hex_text_font, new_prefs->hex_text_font) != 0 ||
	    strcmp (old_prefs->hex_text_font_main, new_prefs->hex_text_font_main) != 0 ||
	    strcmp (old_prefs->hex_text_font_alternative, new_prefs->hex_text_font_alternative) != 0)
		reasons |= THEME_CHANGED_REASON_WIDGET_STYLE;
	if (g_strcmp0 (old_prefs->hex_gui_gtk4_theme,
		new_prefs->hex_gui_gtk4_theme) != 0 ||
		old_prefs->hex_gui_gtk4_variant != new_prefs->hex_gui_gtk4_variant)
		reasons |= THEME_CHANGED_REASON_THEME_PACK |
			THEME_CHANGED_REASON_WIDGET_STYLE | THEME_CHANGED_REASON_MODE;

	if (reasons != THEME_CHANGED_REASON_NONE)
		reasons |= THEME_CHANGED_REASON_WIDGET_STYLE;

	return reasons;
}

static void
theme_manager_auto_dark_mode_changed (GtkSettings *settings, GParamSpec *pspec, gpointer data)
{
	gboolean color_change = FALSE;
	gboolean resolved_dark_preference;
	static gboolean in_handler = FALSE;

	(void) settings;
	(void) pspec;
	(void) data;

	resolved_dark_preference = theme_policy_system_prefers_dark ();

	if (theme_manager_auto_refresh_cache.initialized &&
	    theme_manager_auto_refresh_cache.resolved_dark_preference == resolved_dark_preference &&
	    theme_manager_auto_refresh_cache.dark_mode == prefs.hex_gui_dark_mode)
		return;

	theme_manager_auto_refresh_cache.initialized = TRUE;
	theme_manager_auto_refresh_cache.resolved_dark_preference = resolved_dark_preference;
	theme_manager_auto_refresh_cache.dark_mode = prefs.hex_gui_dark_mode;

	if (prefs.hex_gui_dark_mode != ZOITECHAT_DARK_MODE_AUTO)
		return;
	if (in_handler)
		return;

	in_handler = TRUE;

	if (prefs.hex_gui_dark_mode == ZOITECHAT_DARK_MODE_AUTO)
	{
		fe_set_auto_dark_mode_state (resolved_dark_preference);
		theme_manager_commit_preferences (prefs.hex_gui_dark_mode, &color_change);
		if (color_change)
			theme_manager_dispatch_changed (THEME_CHANGED_REASON_PALETTE | THEME_CHANGED_REASON_WIDGET_STYLE | THEME_CHANGED_REASON_USERLIST | THEME_CHANGED_REASON_MODE);
	}

	in_handler = FALSE;
}

static guint theme_manager_auto_refresh_source = 0;
static ThemeManagerIdleAddFunc theme_manager_idle_add_func = g_idle_add;

static gboolean
theme_manager_run_auto_refresh (gpointer data)
{
	theme_manager_auto_refresh_source = 0;
	theme_manager_auto_dark_mode_changed (NULL, NULL, data);
	return G_SOURCE_REMOVE;
}

static void
theme_manager_queue_auto_refresh (GtkSettings *settings, GParamSpec *pspec, gpointer data)
{
	(void) settings;
	(void) pspec;

	if (theme_manager_auto_refresh_source != 0)
		return;

	theme_manager_auto_refresh_source = theme_manager_idle_add_func (theme_manager_run_auto_refresh, data);
}

void
theme_manager_init (void)
{
	if (!theme_manager_listeners)
		theme_manager_listeners = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL,
									 theme_listener_free);

	if (!theme_manager_setup_listener_id)
		theme_manager_setup_listener_id = theme_listener_register ("setup.apply", theme_manager_setup_apply_listener, NULL);

	fe_set_auto_dark_mode_state (theme_policy_is_dark_mode_active (ZOITECHAT_DARK_MODE_AUTO));
	theme_application_apply_mode (prefs.hex_gui_dark_mode, NULL);
	if (!theme_manager_gtk4_theme_controller)
	{
		GError *error = NULL;

		theme_manager_gtk4_theme_controller = theme_gtk4_controller_new (
			gdk_display_get_default ());
		if (!theme_gtk4_controller_refresh (theme_manager_gtk4_theme_controller,
			get_xdir (), prefs.hex_gui_gtk4_theme, prefs.hex_gui_gtk4_variant,
			theme_policy_system_prefers_dark (), FALSE, &error))
		{
			g_warning ("Unable to apply the saved GTK4 theme: %s",
				error ? error->message : "unknown error");
			g_clear_error (&error);
			theme_gtk4_controller_refresh (theme_manager_gtk4_theme_controller,
				get_xdir (), "", 0, theme_policy_system_prefers_dark (),
				FALSE, NULL);
		}
		theme_manager_gtk4_appearance_monitor =
			theme_appearance_monitor_gtk4_new (gdk_display_get_default (),
				theme_manager_gtk4_apply_appearance, NULL, NULL, &error);
		if (!theme_manager_gtk4_appearance_monitor)
		{
			g_warning ("Unable to monitor GTK4 appearance changes: %s",
				error ? error->message : "unknown error");
			g_clear_error (&error);
		}
	}
	zoitechat_set_theme_post_apply_callback (theme_manager_handle_theme_applied);
}

void
theme_manager_shutdown (void)
{
	g_clear_pointer (&theme_manager_gtk4_appearance_monitor,
		theme_appearance_monitor_gtk4_free);
	g_clear_pointer (&theme_manager_gtk4_theme_controller,
		theme_gtk4_controller_free);
	zoitechat_set_theme_post_apply_callback (NULL);
	if (theme_manager_auto_refresh_source)
	{
		g_source_remove (theme_manager_auto_refresh_source);
		theme_manager_auto_refresh_source = 0;
	}
	if (theme_manager_listeners)
	{
		g_hash_table_destroy (theme_manager_listeners);
		theme_manager_listeners = NULL;
	}
	theme_manager_setup_listener_id = 0;
	memset (&theme_manager_auto_refresh_cache, 0,
		sizeof (theme_manager_auto_refresh_cache));
}

ThemeGtk4Controller *
theme_manager_gtk4_controller (void)
{
	return theme_manager_gtk4_theme_controller;
}

gboolean
theme_manager_gtk4_apply_selection (const char *theme_id, guint variant,
	GError **error)
{
	gboolean prefer_dark = theme_manager_gtk4_prefers_dark ();
	gboolean high_contrast = theme_manager_gtk4_high_contrast ();

	g_return_val_if_fail (theme_manager_gtk4_theme_controller != NULL, FALSE);
	if (!theme_gtk4_controller_refresh (theme_manager_gtk4_theme_controller,
		get_xdir (), theme_id, variant, prefer_dark, high_contrast, error))
		return FALSE;
	theme_manager_dispatch_changed (THEME_CHANGED_REASON_THEME_PACK |
		THEME_CHANGED_REASON_WIDGET_STYLE | THEME_CHANGED_REASON_MODE);
	return TRUE;
}

gboolean
theme_manager_gtk4_prefers_dark (void)
{
	return theme_manager_gtk4_appearance_monitor ?
		theme_appearance_monitor_gtk4_prefers_dark (
			theme_manager_gtk4_appearance_monitor) :
		theme_policy_system_prefers_dark ();
}

gboolean
theme_manager_gtk4_high_contrast (void)
{
	return theme_manager_gtk4_appearance_monitor &&
		theme_appearance_monitor_gtk4_high_contrast (
			theme_manager_gtk4_appearance_monitor);
}

gboolean
theme_manager_apply_mode (unsigned int mode, gboolean *palette_changed)
{
	return theme_application_apply_mode (mode, palette_changed);
}

void
theme_manager_set_mode (unsigned int mode, gboolean *palette_changed)
{
	theme_application_apply_mode (mode, palette_changed);
}

void
theme_manager_set_token_color (unsigned int mode, ThemeSemanticToken token, const GdkRGBA *color, gboolean *palette_changed)
{
	gboolean changed = FALSE;
	gboolean dark_mode;

	if (!color)
		return;

	dark_mode = theme_policy_is_dark_mode_active (mode);
	if (dark_mode)
		theme_runtime_dark_set_color (token, color);
	else
		theme_runtime_user_set_color (token, color);

	theme_runtime_apply_mode (mode, &changed);
	if (palette_changed)
		*palette_changed = changed;

	if (changed)
		theme_manager_dispatch_changed (THEME_CHANGED_REASON_PALETTE | THEME_CHANGED_REASON_WIDGET_STYLE | THEME_CHANGED_REASON_USERLIST);

	theme_application_reload_input_style ();
}

gboolean
theme_manager_apply_palette_candidate (unsigned int mode,
	const ThemePaletteCandidate *candidate, gboolean *palette_changed)
{
	gboolean changed = FALSE;

	if (!candidate || !candidate->initialized
		|| candidate->dark_mode != theme_policy_is_dark_mode_active (mode))
		return FALSE;
	if (!theme_runtime_apply_palette_candidate (candidate, &changed))
		return FALSE;
	if (palette_changed)
		*palette_changed = changed;
	if (changed)
	{
		theme_manager_dispatch_changed (THEME_CHANGED_REASON_PALETTE |
			THEME_CHANGED_REASON_WIDGET_STYLE |
			THEME_CHANGED_REASON_USERLIST);
		theme_application_reload_input_style ();
	}
	return TRUE;
}

void
theme_manager_reset_mode_colors (unsigned int mode, gboolean *palette_changed)
{
	gboolean changed;
	gboolean dark_mode;

	dark_mode = theme_policy_is_dark_mode_active (mode);
	theme_runtime_reset_mode_colors (dark_mode);
	theme_runtime_apply_mode (mode, &changed);
	changed = TRUE;
	if (palette_changed)
		*palette_changed = changed;
	theme_manager_dispatch_changed (THEME_CHANGED_REASON_PALETTE | THEME_CHANGED_REASON_WIDGET_STYLE | THEME_CHANGED_REASON_USERLIST);

	theme_application_reload_input_style ();
}

void
theme_manager_commit_preferences (unsigned int old_mode, gboolean *color_change)
{
	gboolean palette_changed = FALSE;

	theme_application_apply_mode (prefs.hex_gui_dark_mode, &palette_changed);
	if (color_change && (prefs.hex_gui_dark_mode != old_mode || palette_changed))
		*color_change = TRUE;

	if (prefs.hex_gui_dark_mode == ZOITECHAT_DARK_MODE_AUTO)
		fe_set_auto_dark_mode_state (theme_policy_is_dark_mode_active (ZOITECHAT_DARK_MODE_AUTO));
}

gboolean
theme_manager_save_preferences (void)
{
	return theme_runtime_save ();
}

gboolean
theme_changed_event_has_reason (const ThemeChangedEvent *event, ThemeChangedReason reason)
{
	if (!event)
		return FALSE;

	return (event->reasons & reason) != 0;
}

void
theme_manager_apply_and_dispatch (unsigned int mode, ThemeChangedReason reasons, gboolean *palette_changed)
{
	theme_application_apply_mode (mode, palette_changed);
	theme_manager_dispatch_changed (reasons);
}

void
theme_manager_dispatch_changed (ThemeChangedReason reasons)
{
	GHashTableIter iter;
	gpointer key;
	gpointer value;
	ThemeChangedEvent event;

	event.reasons = reasons;

	if ((reasons & (THEME_CHANGED_REASON_MODE |
			    THEME_CHANGED_REASON_THEME_PACK |
			    THEME_CHANGED_REASON_WIDGET_STYLE)) != 0)
	{
		theme_manager_apply_to_toplevel_windows ();
	}

	if (!theme_manager_listeners)
		return;

	g_hash_table_iter_init (&iter, theme_manager_listeners);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		ThemeListener *listener = value;

		if (listener->callback)
			listener->callback (&event, listener->userdata);
	}
}

guint
theme_listener_register (const char *component_id, ThemeChangedCallback callback, gpointer userdata)
{
	ThemeListener *listener;
	guint id;

	if (!callback)
		return 0;

	if (!theme_manager_listeners)
		theme_manager_listeners = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL,
								     theme_listener_free);

	id = theme_manager_next_listener_id++;
	if (theme_manager_next_listener_id == 0)
		theme_manager_next_listener_id = 1;

	listener = g_new0 (ThemeListener, 1);
	listener->id = id;
	listener->component_id = g_strdup (component_id ? component_id : "theme.listener");
	listener->callback = callback;
	listener->userdata = userdata;

	g_hash_table_insert (theme_manager_listeners, GUINT_TO_POINTER (id), listener);

	return id;
}

void
theme_listener_unregister (guint listener_id)
{
	if (!theme_manager_listeners || listener_id == 0)
		return;

	g_hash_table_remove (theme_manager_listeners, GUINT_TO_POINTER (listener_id));
}

void
theme_manager_handle_theme_applied (void)
{
	theme_application_apply_mode (prefs.hex_gui_dark_mode, NULL);
	theme_manager_dispatch_changed (THEME_CHANGED_REASON_THEME_PACK | THEME_CHANGED_REASON_PALETTE | THEME_CHANGED_REASON_WIDGET_STYLE | THEME_CHANGED_REASON_USERLIST | THEME_CHANGED_REASON_MODE);
}


static void
theme_manager_apply_platform_window_theme (GtkWidget *window)
{
	gboolean dark;

	if (!window)
		return;

	{
		const FabulorGtk4ThemeAppearanceDecision *appearance =
			theme_gtk4_controller_appearance (
				theme_manager_gtk4_theme_controller);

		dark = appearance ? appearance->prefer_dark :
			theme_runtime_is_dark_active ();
	}

	gtk_widget_remove_css_class (window, "zoitechat-dark");
	gtk_widget_remove_css_class (window, "zoitechat-light");
	gtk_widget_add_css_class (window,
		dark ? "zoitechat-dark" : "zoitechat-light");
#ifdef G_OS_WIN32
	fe_win32_apply_native_titlebar (window, dark);
#endif
}

static void
theme_manager_window_finalized_cb (gpointer userdata, GObject *window)
{
	(void) userdata;
	(void) window;
}

void
theme_manager_apply_to_window (GtkWidget *window)
{
	if (!window)
		return;

	theme_manager_apply_platform_window_theme (window);
}

void
theme_manager_attach_window (GtkWidget *window)
{
	gpointer owner;

	if (!window)
		return;

	owner = g_object_get_data (G_OBJECT (window), theme_manager_window_destroy_handler_key);
	if (!owner)
	{
		g_object_weak_ref (G_OBJECT (window), theme_manager_window_finalized_cb, NULL);
		g_object_set_data (G_OBJECT (window), theme_manager_window_destroy_handler_key,
			(gpointer) theme_manager_window_weak_owner);
	}

	theme_manager_apply_to_window (window);
}

void
theme_manager_detach_window (GtkWidget *window)
{
	gpointer owner;

	if (!window)
		return;

	owner = g_object_get_data (G_OBJECT (window), theme_manager_window_destroy_handler_key);
	if (owner)
	{
		g_object_weak_unref (G_OBJECT (window), theme_manager_window_finalized_cb, NULL);
		g_object_set_data (G_OBJECT (window), theme_manager_window_destroy_handler_key, NULL);
	}
}

void
theme_manager_apply_palette_widget (GtkWidget *widget, const GdkRGBA *bg, const GdkRGBA *fg,
			       const PangoFontDescription *font_desc)
{
	theme_css_apply_palette_widget (widget, bg, fg, font_desc);
}

void
theme_manager_apply_entry_palette (GtkWidget *widget, const PangoFontDescription *font_desc)
{
	ThemeWidgetStyleValues style_values;

	if (!widget || !font_desc)
		return;

	theme_get_widget_style_values_for_widget (widget, &style_values);
	gtkutil_apply_palette (widget, &style_values.background, &style_values.foreground, font_desc);
}

ThemePaletteBehavior
theme_manager_get_userlist_palette_behavior (const PangoFontDescription *font_desc)
{
	ThemePaletteBehavior behavior;

	behavior.font_desc = font_desc;
	behavior.apply_background = TRUE;
	behavior.apply_foreground = (prefs.hex_gui_ulist_color || prefs.hex_text_color_nicks) ? FALSE : TRUE;

	return behavior;
}

ThemePaletteBehavior
theme_manager_get_channel_tree_palette_behavior (const PangoFontDescription *font_desc)
{
	ThemePaletteBehavior behavior;

	behavior.font_desc = font_desc;
	behavior.apply_background = TRUE;
	behavior.apply_foreground = TRUE;

	return behavior;
}

void
theme_manager_apply_userlist_palette (GtkWidget *widget, const PangoFontDescription *font_desc,
				       gboolean prefer_background, gboolean prefer_foreground)
{
	ThemePaletteBehavior behavior;

	behavior.font_desc = font_desc;
	behavior.apply_background = prefer_background;
	behavior.apply_foreground = prefer_foreground;
	theme_manager_apply_userlist_style (widget, behavior);
}

void
theme_manager_apply_userlist_style (GtkWidget *widget, ThemePaletteBehavior behavior)
{
	ThemeWidgetStyleValues style_values;
	const GdkRGBA *background = NULL;
	const GdkRGBA *foreground = NULL;

	if (!widget)
		return;

	theme_get_widget_style_values_for_widget (widget, &style_values);
	if (behavior.apply_background)
		background = &style_values.background;
	if (behavior.apply_foreground)
		foreground = &style_values.foreground;

	gtkutil_apply_palette (widget, background, foreground, behavior.font_desc);
}

void
theme_manager_apply_channel_tree_style (GtkWidget *widget, ThemePaletteBehavior behavior)
{
	theme_manager_apply_userlist_style (widget, behavior);
}

void
theme_manager_apply_input_style (gboolean enabled, const PangoFontDescription *font_desc)
{
	theme_css_reload_input_style (enabled, font_desc);
}

void
theme_manager_reload_input_style (void)
{
	theme_application_reload_input_style ();
}

void
theme_manager_refresh_auto_mode (void)
{
	theme_manager_queue_auto_refresh (NULL, NULL, NULL);
}

ThemeChangedEvent
theme_manager_on_preferences_changed (const struct zoitechatprefs *old_prefs,
				      const struct zoitechatprefs *new_prefs,
				      unsigned int old_mode,
				      gboolean *color_change)
{
	ThemeChangedEvent event;
	gboolean had_color_change = color_change && *color_change;

	theme_manager_commit_preferences (old_mode, color_change);
	event.reasons = theme_manager_synthesize_preference_reasons (old_prefs, new_prefs,
						     had_color_change || (color_change && *color_change));

	return event;
}

void
theme_manager_dispatch_setup_apply (const ThemeChangedEvent *event)
{
	if (!event)
		return;

	setup_apply_real (event);
}

void
theme_manager_set_idle_add_func (ThemeManagerIdleAddFunc idle_add_func)
{
	theme_manager_idle_add_func = idle_add_func ? idle_add_func : g_idle_add;
	theme_manager_auto_refresh_source = 0;
}
