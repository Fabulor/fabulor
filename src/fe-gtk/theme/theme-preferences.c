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

#include <stdio.h>
#include <string.h>

#include "../../common/zoitechat.h"
#include "../../common/zoitechatc.h"

#include <gio/gio.h>
#include <glib/gstdio.h>

#include "../gtkutil.h"
#include "../gtk-compat.h"
#include "../file-chooser-path.h"
#include "../../common/fe.h"
#include "../../common/cfgfiles.h"
#include "../../common/util.h"
#include "../../common/theme-archive-reader.h"
#include "theme-manager.h"
#include "theme-preferences.h"
#include "theme-runtime.h"
#include "theme-preferences-gtk4.h"

extern void load_text_events (void);

typedef struct
{
        GWeakRef owner;
        GWeakRef parent;
        gboolean parent_watch_active;
} theme_preferences_native_import_data;

typedef struct
{
        GtkWidget *button;
        ThemeSemanticToken token;
        gboolean *color_change_flag;
        gpointer manager_ui;
} theme_color_dialog_data;

typedef struct
{
        GtkWidget *row;
        GtkWidget *button;
        GtkWidget *entry;
        GtkWidget *preview;
        ThemeSemanticToken token;
        char *search_text;
        gboolean *color_change_flag;
        GtkWindow *parent;
        gpointer manager_ui;
} theme_color_manager_row;

typedef struct
{
        theme_color_manager_row *row;
        GdkRGBA original;
        gboolean has_original;
} theme_manager_live_picker_data;

typedef struct
{
        GPtrArray *rows;
        GtkWidget *search_entry;
        gboolean *color_change_flag;
        GtkWidget *preview_window;
        GtkWidget *preview_chat;
        GtkWidget *preview_selected;
        GtkWidget *preview_marker;
        GtkWidget *preview_tab_new_data;
        GtkWidget *preview_tab_new_message;
        GtkWidget *preview_tab_highlight;
        GtkWidget *preview_tab_away;
        GtkWidget *preview_spell;
} theme_color_manager_ui;

#define COLOR_MANAGER_RESPONSE_RESET 1

typedef struct
{
        gboolean active;
        gboolean changed;
        unsigned int mode;
        gboolean snapshot_valid[THEME_TOKEN_COUNT];
        gboolean staged_valid[THEME_TOKEN_COUNT];
        GdkRGBA snapshot[THEME_TOKEN_COUNT];
        GdkRGBA staged[THEME_TOKEN_COUNT];
	char gtk4_theme_snapshot[sizeof prefs.hex_gui_gtk4_theme];
	guint gtk4_variant_snapshot;
} theme_preferences_stage_state;

static theme_preferences_stage_state theme_preferences_stage;

static unsigned int
theme_preferences_current_color_mode (void)
{
        return theme_runtime_is_dark_active () ? ZOITECHAT_DARK_MODE_DARK : ZOITECHAT_DARK_MODE_LIGHT;
}

static unsigned int
theme_preferences_stage_color_mode (void)
{
        if (theme_preferences_stage.active)
                return theme_preferences_stage.mode;

        return theme_preferences_current_color_mode ();
}

static gboolean
theme_preferences_staged_get_color (ThemeSemanticToken token, GdkRGBA *rgba)
{
        if (token < 0 || token >= THEME_TOKEN_COUNT || !rgba)
                return FALSE;

        if (theme_preferences_stage.active && theme_preferences_stage.staged_valid[token])
        {
                *rgba = theme_preferences_stage.staged[token];
                return TRUE;
        }

        return theme_get_color (token, rgba);
}

static void
theme_preferences_stage_recompute_changed (void)
{
        ThemeSemanticToken token;

        theme_preferences_stage.changed = FALSE;
        for (token = THEME_TOKEN_MIRC_0; token < THEME_TOKEN_COUNT; token++)
        {
                if (!theme_preferences_stage.snapshot_valid[token] || !theme_preferences_stage.staged_valid[token])
                        continue;
                if (!gdk_rgba_equal (&theme_preferences_stage.snapshot[token], &theme_preferences_stage.staged[token]))
                {
                        theme_preferences_stage.changed = TRUE;
                        return;
                }
        }
}

static void
theme_preferences_stage_sync_runtime_to_snapshot (void)
{
        ThemeSemanticToken token;

        for (token = THEME_TOKEN_MIRC_0; token < THEME_TOKEN_COUNT; token++)
        {
                if (theme_preferences_stage.snapshot_valid[token])
                        theme_manager_set_token_color (theme_preferences_stage.mode, token,
                                                       &theme_preferences_stage.snapshot[token], NULL);
        }
}

static void
theme_preferences_stage_sync_runtime_to_staged (void)
{
        ThemeSemanticToken token;

        for (token = THEME_TOKEN_MIRC_0; token < THEME_TOKEN_COUNT; token++)
        {
                if (theme_preferences_stage.staged_valid[token])
                        theme_manager_set_token_color (theme_preferences_stage.mode, token,
                                                       &theme_preferences_stage.staged[token], NULL);
        }
}

static void
theme_preferences_staged_set_color (ThemeSemanticToken token, const GdkRGBA *rgba,
                                    gboolean *color_change_flag, gboolean live_preview)
{
        const GdkRGBA *preview_color = rgba;

        if (token < 0 || token >= THEME_TOKEN_COUNT || !rgba)
                return;

        if (theme_preferences_stage.active)
        {
                theme_preferences_stage.staged[token] = *rgba;
                theme_preferences_stage.staged_valid[token] = TRUE;
                theme_preferences_stage_recompute_changed ();
                if (color_change_flag)
                        *color_change_flag = theme_preferences_stage.changed;

                preview_color = &theme_preferences_stage.staged[token];
        }

        if (live_preview)
                theme_manager_set_token_color (theme_preferences_stage_color_mode (), token, preview_color, NULL);
}

void
theme_preferences_stage_begin (void)
{
        ThemeSemanticToken token;

        memset (&theme_preferences_stage, 0, sizeof (theme_preferences_stage));
        theme_preferences_stage.active = TRUE;
        theme_preferences_stage.mode = theme_preferences_current_color_mode ();
	g_strlcpy (theme_preferences_stage.gtk4_theme_snapshot,
		prefs.hex_gui_gtk4_theme,
		sizeof (theme_preferences_stage.gtk4_theme_snapshot));
	theme_preferences_stage.gtk4_variant_snapshot = prefs.hex_gui_gtk4_variant;

        for (token = THEME_TOKEN_MIRC_0; token < THEME_TOKEN_COUNT; token++)
        {
                GdkRGBA rgba;

                if (!theme_preferences_staged_get_color (token, &rgba))
                        continue;

                theme_preferences_stage.snapshot[token] = rgba;
                theme_preferences_stage.staged[token] = rgba;
                theme_preferences_stage.snapshot_valid[token] = TRUE;
                theme_preferences_stage.staged_valid[token] = TRUE;
        }
}

void
theme_preferences_stage_apply (void)
{
        if (!theme_preferences_stage.active)
                return;

        theme_preferences_stage_sync_runtime_to_staged ();
}

void
theme_preferences_stage_commit (void)
{
        if (!theme_preferences_stage.active)
                return;

        theme_preferences_stage_apply ();
        memset (&theme_preferences_stage, 0, sizeof (theme_preferences_stage));
}

void
theme_preferences_stage_discard (void)
{
        if (!theme_preferences_stage.active)
                return;

        theme_preferences_stage_sync_runtime_to_snapshot ();
	theme_manager_gtk4_apply_selection (
		theme_preferences_stage.gtk4_theme_snapshot,
		theme_preferences_stage.gtk4_variant_snapshot, NULL);
        memset (&theme_preferences_stage, 0, sizeof (theme_preferences_stage));
}

static void
theme_preferences_show_import_error (GtkWidget *button, const char *message);

static void
theme_preferences_native_import_data_free (gpointer user_data)
{
        theme_preferences_native_import_data *data = user_data;

        if (!data)
                return;

        g_weak_ref_clear (&data->owner);
        g_weak_ref_clear (&data->parent);
        g_free (data);
}

static void
theme_preferences_native_import_parent_gone (GtkNativeDialog *dialog)
{
        theme_preferences_native_import_data *data;

        data = g_object_get_data (G_OBJECT (dialog), "fabulor-theme-native-import-data");
        if (data)
                data->parent_watch_active = FALSE;
        gtk_native_dialog_hide (dialog);
        g_object_unref (dialog);
}

static void
theme_preferences_native_import_parent_finalized_cb (gpointer user_data,
                                                      GObject *parent)
{
        (void) parent;
        theme_preferences_native_import_parent_gone (
                GTK_NATIVE_DIALOG (user_data));
}

static theme_preferences_native_import_data *
theme_preferences_native_import_data_new (GtkNativeDialog *dialog,
                                          GtkWidget *owner,
                                          GtkWindow *parent)
{
        theme_preferences_native_import_data *data;

        data = g_new0 (theme_preferences_native_import_data, 1);
        g_weak_ref_init (&data->owner, owner);
        g_weak_ref_init (&data->parent, parent);
        g_object_set_data_full (G_OBJECT (dialog), "fabulor-theme-native-import-data",
                                data, theme_preferences_native_import_data_free);
        data->parent_watch_active = TRUE;
        g_object_weak_ref (G_OBJECT (parent),
                           theme_preferences_native_import_parent_finalized_cb,
                           dialog);
        return data;
}

static GtkWidget *
theme_preferences_native_import_acquire_owner (GtkNativeDialog *dialog,
                                               theme_preferences_native_import_data *data)
{
        GtkWindow *parent;
        GtkWidget *owner = NULL;

        parent = g_weak_ref_get (&data->parent);
        if (parent)
        {
                if (data->parent_watch_active)
                {
                        g_object_weak_unref (G_OBJECT (parent),
                                             theme_preferences_native_import_parent_finalized_cb,
                                             dialog);
                        data->parent_watch_active = FALSE;
                }
                owner = g_weak_ref_get (&data->owner);
        }
        g_clear_object (&parent);
        return owner;
}

static void
theme_preferences_manager_row_free (gpointer data)
{
        theme_color_manager_row *row = data;

        if (!row)
                return;

        g_free (row->search_text);
        g_free (row);
}

static void
theme_preferences_manager_ui_free (gpointer data)
{
        theme_color_manager_ui *ui = data;

        if (!ui)
                return;

        if (ui->rows)
                g_ptr_array_unref (ui->rows);
        g_free (ui);
}

static void
theme_preferences_manager_update_preview (theme_color_manager_ui *ui)
{
        GdkRGBA text_fg;
        GdkRGBA text_bg;
        GdkRGBA sel_fg;
        GdkRGBA sel_bg;
        GdkRGBA marker;
        GdkRGBA tab_new_data;
        GdkRGBA tab_new_message;
        GdkRGBA tab_highlight;
        GdkRGBA tab_away;
        GdkRGBA spell;
        GtkWidget *label;

        if (!ui)
                return;

        if (!theme_preferences_staged_get_color (THEME_TOKEN_TEXT_FOREGROUND, &text_fg)
            || !theme_preferences_staged_get_color (THEME_TOKEN_TEXT_BACKGROUND, &text_bg)
            || !theme_preferences_staged_get_color (THEME_TOKEN_SELECTION_FOREGROUND, &sel_fg)
            || !theme_preferences_staged_get_color (THEME_TOKEN_SELECTION_BACKGROUND, &sel_bg)
            || !theme_preferences_staged_get_color (THEME_TOKEN_MARKER, &marker)
            || !theme_preferences_staged_get_color (THEME_TOKEN_TAB_NEW_DATA, &tab_new_data)
            || !theme_preferences_staged_get_color (THEME_TOKEN_TAB_NEW_MESSAGE, &tab_new_message)
            || !theme_preferences_staged_get_color (THEME_TOKEN_TAB_HIGHLIGHT, &tab_highlight)
            || !theme_preferences_staged_get_color (THEME_TOKEN_TAB_AWAY, &tab_away)
            || !theme_preferences_staged_get_color (THEME_TOKEN_SPELL, &spell))
                return;

        gtkutil_apply_palette (ui->preview_window, &text_bg, &text_fg, NULL);
        gtkutil_apply_palette (ui->preview_chat, &text_bg, &text_fg, NULL);
        label = g_object_get_data (G_OBJECT (ui->preview_chat), "zoitechat-preview-label");
        if (GTK_IS_WIDGET (label))
                gtkutil_apply_palette (label, NULL, &text_fg, NULL);

        gtkutil_apply_palette (ui->preview_selected, &sel_bg, &sel_fg, NULL);
        label = g_object_get_data (G_OBJECT (ui->preview_selected), "zoitechat-preview-label");
        if (GTK_IS_WIDGET (label))
                gtkutil_apply_palette (label, NULL, &sel_fg, NULL);

        gtkutil_apply_palette (ui->preview_marker, &marker, &text_fg, NULL);
        label = g_object_get_data (G_OBJECT (ui->preview_marker), "zoitechat-preview-label");
        if (GTK_IS_WIDGET (label))
                gtkutil_apply_palette (label, NULL, &text_fg, NULL);

        gtkutil_apply_palette (ui->preview_tab_new_data, &tab_new_data, &text_fg, NULL);
        label = g_object_get_data (G_OBJECT (ui->preview_tab_new_data), "zoitechat-preview-label");
        if (GTK_IS_WIDGET (label))
                gtkutil_apply_palette (label, NULL, &text_fg, NULL);

        gtkutil_apply_palette (ui->preview_tab_new_message, &tab_new_message, &text_fg, NULL);
        label = g_object_get_data (G_OBJECT (ui->preview_tab_new_message), "zoitechat-preview-label");
        if (GTK_IS_WIDGET (label))
                gtkutil_apply_palette (label, NULL, &text_fg, NULL);

        gtkutil_apply_palette (ui->preview_tab_highlight, &tab_highlight, &text_fg, NULL);
        label = g_object_get_data (G_OBJECT (ui->preview_tab_highlight), "zoitechat-preview-label");
        if (GTK_IS_WIDGET (label))
                gtkutil_apply_palette (label, NULL, &text_fg, NULL);

        gtkutil_apply_palette (ui->preview_tab_away, &tab_away, &text_fg, NULL);
        label = g_object_get_data (G_OBJECT (ui->preview_tab_away), "zoitechat-preview-label");
        if (GTK_IS_WIDGET (label))
                gtkutil_apply_palette (label, NULL, &text_fg, NULL);

        gtkutil_apply_palette (ui->preview_spell, &text_bg, &spell, NULL);
        label = g_object_get_data (G_OBJECT (ui->preview_spell), "zoitechat-preview-label");
        if (GTK_IS_WIDGET (label))
                gtkutil_apply_palette (label, NULL, &spell, NULL);
}

static GtkWidget *
theme_preferences_manager_preview_item_new (const char *text)
{
        GtkWidget *box;
        GtkWidget *label;

        box = fabulor_gtk_content_surface_new (TRUE);
        fabulor_gtk_container_set_uniform_inset (box, 3);

        label = gtk_label_new (text);
        gtk_widget_set_halign (label, GTK_ALIGN_START);
        fabulor_gtk_content_surface_set_child (box, label);
        g_object_set_data (G_OBJECT (box), "zoitechat-preview-label", label);

        return box;
}

static GtkWidget *
theme_preferences_manager_create_preview (theme_color_manager_ui *ui)
{
        GtkWidget *frame;
        GtkWidget *vbox;
        GtkWidget *header;
        GtkWidget *chat_box;
        GtkWidget *tabs_box;
        GtkWidget *label;

        frame = gtk_frame_new (_("Live preview"));
        vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
        fabulor_gtk_container_set_uniform_inset (vbox, 8);
        fabulor_gtk_frame_set_child (GTK_FRAME (frame), vbox);

        ui->preview_window = fabulor_gtk_content_surface_new (TRUE);
        fabulor_gtk_container_set_uniform_inset (ui->preview_window, 8);
        fabulor_gtk_box_append (GTK_BOX (vbox), ui->preview_window, TRUE, TRUE, 0);

        chat_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
        fabulor_gtk_content_surface_set_child (ui->preview_window, chat_box);

        header = gtk_label_new (_("#zoitechat-preview"));
        gtk_widget_set_halign (header, GTK_ALIGN_START);
        fabulor_gtk_box_append (GTK_BOX (chat_box), header, FALSE, FALSE, 0);

        ui->preview_chat = theme_preferences_manager_preview_item_new (_("<alice> Example chat message"));
        fabulor_gtk_box_append (GTK_BOX (chat_box), ui->preview_chat, FALSE, FALSE, 0);

        ui->preview_selected = theme_preferences_manager_preview_item_new (_("Selected text example"));
        fabulor_gtk_box_append (GTK_BOX (chat_box), ui->preview_selected, FALSE, FALSE, 0);

        ui->preview_marker = theme_preferences_manager_preview_item_new (_("Marker line"));
        gtk_widget_set_hexpand (ui->preview_marker, TRUE);
        fabulor_gtk_box_append (GTK_BOX (chat_box), ui->preview_marker, FALSE, FALSE, 0);

        ui->preview_spell = theme_preferences_manager_preview_item_new (_("mispelled wrd"));
        fabulor_gtk_box_append (GTK_BOX (chat_box), ui->preview_spell, FALSE, FALSE, 0);

        label = gtk_label_new (_("Tab states"));
        gtk_widget_set_halign (label, GTK_ALIGN_START);
        fabulor_gtk_box_append (GTK_BOX (vbox), label, FALSE, FALSE, 0);

        tabs_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
        fabulor_gtk_box_append (GTK_BOX (vbox), tabs_box, FALSE, FALSE, 0);

        ui->preview_tab_new_data = theme_preferences_manager_preview_item_new (_("New data"));
        gtk_widget_set_hexpand (ui->preview_tab_new_data, TRUE);
        fabulor_gtk_box_append (GTK_BOX (tabs_box), ui->preview_tab_new_data, TRUE, TRUE, 0);

        ui->preview_tab_new_message = theme_preferences_manager_preview_item_new (_("New message"));
        gtk_widget_set_hexpand (ui->preview_tab_new_message, TRUE);
        fabulor_gtk_box_append (GTK_BOX (tabs_box), ui->preview_tab_new_message, TRUE, TRUE, 0);

        ui->preview_tab_highlight = theme_preferences_manager_preview_item_new (_("Highlight"));
        gtk_widget_set_hexpand (ui->preview_tab_highlight, TRUE);
        fabulor_gtk_box_append (GTK_BOX (tabs_box), ui->preview_tab_highlight, TRUE, TRUE, 0);

        ui->preview_tab_away = theme_preferences_manager_preview_item_new (_("Away"));
        gtk_widget_set_hexpand (ui->preview_tab_away, TRUE);
        fabulor_gtk_box_append (GTK_BOX (tabs_box), ui->preview_tab_away, TRUE, TRUE, 0);

        theme_preferences_manager_update_preview (ui);

        return frame;
}

#define LABEL_INDENT 12

static void
theme_preferences_color_button_apply (GtkWidget *button, const GdkRGBA *color)
{
        GtkWidget *target = g_object_get_data (G_OBJECT (button), "zoitechat-color-box");
        GtkWidget *apply_widget = GTK_IS_WIDGET (target) ? target : button;

        gtkutil_apply_palette (apply_widget, color, NULL, NULL);

        if (apply_widget != button)
                gtkutil_apply_palette (button, color, NULL, NULL);

        gtk_widget_queue_draw (button);
}

static void
theme_preferences_color_response_cb (GtkDialog *dialog, gint response_id, gpointer user_data)
{
        theme_color_dialog_data *data = user_data;

        if (response_id == GTK_RESPONSE_OK)
        {
                GdkRGBA rgba;

                gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (dialog), &rgba);
                theme_preferences_staged_set_color (data->token,
                                                    &rgba,
                                                    data->color_change_flag,
                                                    TRUE);
                theme_preferences_color_button_apply (data->button, &rgba);
                theme_preferences_manager_update_preview ((theme_color_manager_ui *) data->manager_ui);
        }

        fabulor_gtk_window_destroy (GTK_WINDOW (dialog));
        g_free (data);
}

static void
theme_preferences_color_cb (GtkWidget *button, gpointer userdata)
{
        GtkWidget *dialog;
        ThemeSemanticToken token;
        GdkRGBA rgba;
        theme_color_dialog_data *data;

        token = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "zoitechat-theme-token"));

        if (!theme_preferences_staged_get_color (token, &rgba))
                return;
        dialog = gtk_color_chooser_dialog_new (_("Select color"), GTK_WINDOW (userdata));
	theme_manager_attach_window (dialog);
        gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (dialog), &rgba);
        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

        data = g_new0 (theme_color_dialog_data, 1);
        data->button = button;
        data->token = token;
        data->color_change_flag = g_object_get_data (G_OBJECT (button), "zoitechat-theme-color-change");
        data->manager_ui = g_object_get_data (G_OBJECT (button), "zoitechat-theme-color-manager-ui");
        g_signal_connect (dialog, "response", G_CALLBACK (theme_preferences_color_response_cb), data);
        gtk_widget_show (dialog);
}

static char *
theme_preferences_format_hex (const GdkRGBA *color)
{
        return g_strdup_printf ("#%02X%02X%02X",
                                (guint) CLAMP (color->red * 255.0 + 0.5, 0.0, 255.0),
                                (guint) CLAMP (color->green * 255.0 + 0.5, 0.0, 255.0),
                                (guint) CLAMP (color->blue * 255.0 + 0.5, 0.0, 255.0));
}

static char *
theme_preferences_token_display_name (ThemeSemanticToken token)
{
        if (token >= THEME_TOKEN_MIRC_0 && token <= THEME_TOKEN_MIRC_15)
                return g_strdup_printf (_("mIRC color %d"), token - THEME_TOKEN_MIRC_0);

        if (token >= THEME_TOKEN_MIRC_16 && token <= THEME_TOKEN_MIRC_31)
                return g_strdup_printf (_("Local color %d"), token - THEME_TOKEN_MIRC_0);

        switch (token)
        {
        case THEME_TOKEN_SELECTION_FOREGROUND:
                return g_strdup (_("Selected text foreground"));
        case THEME_TOKEN_SELECTION_BACKGROUND:
                return g_strdup (_("Selected text background"));
        case THEME_TOKEN_TEXT_FOREGROUND:
                return g_strdup (_("Text foreground"));
        case THEME_TOKEN_TEXT_BACKGROUND:
                return g_strdup (_("Text background"));
        case THEME_TOKEN_MARKER:
                return g_strdup (_("Marker line"));
        case THEME_TOKEN_TAB_NEW_DATA:
                return g_strdup (_("Tab: new data"));
        case THEME_TOKEN_TAB_HIGHLIGHT:
                return g_strdup (_("Tab: highlight"));
        case THEME_TOKEN_TAB_NEW_MESSAGE:
                return g_strdup (_("Tab: new message"));
        case THEME_TOKEN_TAB_AWAY:
                return g_strdup (_("Tab: away"));
        case THEME_TOKEN_SPELL:
                return g_strdup (_("Spell checker"));
        default:
                return g_strdup (_("Unknown color"));
        }
}

static void
theme_preferences_manager_row_apply (theme_color_manager_row *row, const GdkRGBA *rgba)
{
        char *hex;

        theme_preferences_color_button_apply (row->button, rgba);
        gtkutil_apply_palette (row->preview, rgba, NULL, NULL);
        hex = theme_preferences_format_hex (rgba);
        fabulor_gtk_entry_set_text (GTK_ENTRY (row->entry), hex);
        g_free (hex);
}

static void
theme_preferences_manager_row_commit (theme_color_manager_row *row, const GdkRGBA *rgba)
{
        theme_preferences_staged_set_color (row->token, rgba, row->color_change_flag, TRUE);
        theme_preferences_manager_row_apply (row, rgba);
        theme_preferences_manager_update_preview ((theme_color_manager_ui *) row->manager_ui);
}

static void
theme_preferences_manager_entry_commit (theme_color_manager_row *row)
{
        GdkRGBA rgba;
        const char *text = fabulor_gtk_entry_get_text (GTK_ENTRY (row->entry));

        if (!gdk_rgba_parse (&rgba, text))
        {
                if (theme_preferences_staged_get_color (row->token, &rgba))
                        theme_preferences_manager_row_apply (row, &rgba);
                return;
        }

        theme_preferences_manager_row_commit (row, &rgba);
}

static void
theme_preferences_manager_entry_activate_cb (GtkEntry *entry, gpointer user_data)
{
        (void) entry;
        theme_preferences_manager_entry_commit ((theme_color_manager_row *) user_data);
}

static void
theme_preferences_manager_entry_focus_out_cb (GtkWidget *widget, gpointer user_data)
{
        (void) widget;
        theme_preferences_manager_entry_commit ((theme_color_manager_row *) user_data);
}

static void
theme_preferences_manager_picker_notify_rgba_cb (GObject *object, GParamSpec *pspec, gpointer user_data)
{
        theme_manager_live_picker_data *data = user_data;
        GdkRGBA rgba;

        (void) pspec;
        if (!data || !data->row)
                return;

        gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (object), &rgba);
        theme_preferences_manager_row_commit (data->row, &rgba);
}

static void
theme_preferences_manager_picker_response_cb (GtkDialog *dialog, gint response_id, gpointer user_data)
{
        theme_manager_live_picker_data *data = user_data;

        if (data && data->row && data->has_original && response_id != GTK_RESPONSE_OK)
                theme_preferences_manager_row_commit (data->row, &data->original);

        fabulor_gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
theme_preferences_manager_pick_cb (GtkWidget *button, gpointer user_data)
{
        theme_color_manager_row *row = user_data;
        GtkWidget *dialog;
        GdkRGBA rgba;
        theme_manager_live_picker_data *data;

        if (!theme_preferences_staged_get_color (row->token, &rgba))
                return;

        dialog = gtk_color_chooser_dialog_new (_("Select color"), row->parent);
        theme_manager_attach_window (dialog);
        gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (dialog), &rgba);
        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
        gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);

        data = g_new0 (theme_manager_live_picker_data, 1);
        data->row = row;
        data->original = rgba;
        data->has_original = TRUE;
        g_object_set_data_full (G_OBJECT (dialog),
                                "zoitechat-theme-manager-live-picker",
                                data, g_free);

        g_signal_connect (G_OBJECT (dialog), "notify::rgba",
                          G_CALLBACK (theme_preferences_manager_picker_notify_rgba_cb), data);
        g_signal_connect (G_OBJECT (dialog), "response",
                          G_CALLBACK (theme_preferences_manager_picker_response_cb), data);
        gtk_widget_show (dialog);
        (void) button;
}

static gboolean
theme_preferences_manager_row_matches (theme_color_manager_row *row, const char *needle)
{
        if (!needle || !needle[0])
                return TRUE;

        return strstr (row->search_text, needle) != NULL;
}

static void
theme_preferences_manager_search_changed_cb (GtkEditable *editable, gpointer user_data)
{
        theme_color_manager_ui *ui = user_data;
        char *needle_lower;
        size_t i;

        needle_lower = g_utf8_strdown (fabulor_gtk_entry_get_text (GTK_ENTRY (editable)), -1);
        for (i = 0; i < ui->rows->len; i++)
        {
                theme_color_manager_row *row = g_ptr_array_index (ui->rows, i);
                gtk_widget_set_visible (row->row, theme_preferences_manager_row_matches (row, needle_lower));
        }
        g_free (needle_lower);
}

static void
theme_preferences_manager_refresh_rows (theme_color_manager_ui *ui)
{
        size_t i;

        if (!ui || !ui->rows)
                return;

        for (i = 0; i < ui->rows->len; i++)
        {
                theme_color_manager_row *row = g_ptr_array_index (ui->rows, i);
                GdkRGBA rgba;

                if (theme_preferences_staged_get_color (row->token, &rgba))
                        theme_preferences_manager_row_apply (row, &rgba);
        }

        theme_preferences_manager_update_preview (ui);
}

static void
theme_preferences_manager_dialog_response_cb (GtkDialog *dialog, gint response_id, gpointer user_data)
{
        theme_color_manager_ui *ui = user_data;

        if (response_id != COLOR_MANAGER_RESPONSE_RESET)
        {
                if (ui->color_change_flag && theme_preferences_stage.active)
                        *ui->color_change_flag = theme_preferences_stage.changed;
                fabulor_gtk_window_destroy (GTK_WINDOW (dialog));
                return;
        }

        {
                gboolean changed = FALSE;

                theme_manager_reset_mode_colors (theme_preferences_stage_color_mode (), &changed);
                if (theme_preferences_stage.active)
                {
                        ThemeSemanticToken token;
                        ThemeWidgetStyleValues style_values;

                        for (token = THEME_TOKEN_MIRC_0; token < THEME_TOKEN_COUNT; token++)
                        {
                                GdkRGBA rgba;

                                if (!theme_get_color (token, &rgba))
                                        continue;
                                theme_preferences_stage.staged[token] = rgba;
                                theme_preferences_stage.staged_valid[token] = TRUE;
                        }
                        theme_get_widget_style_values_for_widget (GTK_WIDGET (dialog), &style_values);
                        theme_preferences_stage.staged[THEME_TOKEN_TEXT_FOREGROUND] = style_values.foreground;
                        theme_preferences_stage.staged_valid[THEME_TOKEN_TEXT_FOREGROUND] = TRUE;
                        theme_preferences_stage.staged[THEME_TOKEN_TEXT_BACKGROUND] = style_values.background;
                        theme_preferences_stage.staged_valid[THEME_TOKEN_TEXT_BACKGROUND] = TRUE;
                        theme_preferences_stage_sync_runtime_to_staged ();
                        theme_preferences_stage_recompute_changed ();
                        if (ui->color_change_flag)
                                *ui->color_change_flag = theme_preferences_stage.changed;
                }
                else if (ui->color_change_flag)
                        *ui->color_change_flag = *ui->color_change_flag || changed;
        }

        theme_preferences_manager_refresh_rows (ui);
        g_signal_stop_emission_by_name (dialog, "response");
}

static GtkWidget *
theme_preferences_create_color_manager_dialog (GtkWindow *parent, gboolean *color_change_flag)
{
        GtkWidget *dialog;
        GtkWidget *content;
        GtkWidget *vbox;
        GtkWidget *content_hbox;
        GtkWidget *left_box;
        GtkWidget *search;
        GtkWidget *scroller;
        GtkWidget *list;
        GtkWidget *preview_frame;
        theme_color_manager_ui *ui;
        ThemeSemanticToken token;

        dialog = gtk_dialog_new_with_buttons (_("Manage client colors"),
                                              parent,
                                              GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                              _("_Reset to GTK3 defaults"),
                                              COLOR_MANAGER_RESPONSE_RESET,
                                              _("_Close"),
                                              GTK_RESPONSE_CLOSE,
                                              NULL);
        theme_manager_attach_window (dialog);
        gtk_window_set_default_size (GTK_WINDOW (dialog), 760, 560);

        ui = g_new0 (theme_color_manager_ui, 1);
        ui->rows = g_ptr_array_new_with_free_func (theme_preferences_manager_row_free);
        ui->color_change_flag = color_change_flag;
        g_object_set_data_full (G_OBJECT (dialog), "zoitechat-theme-color-manager", ui, theme_preferences_manager_ui_free);
        g_signal_connect (G_OBJECT (dialog), "response", G_CALLBACK (theme_preferences_manager_dialog_response_cb), ui);

        content = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
        vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
        fabulor_gtk_container_set_uniform_inset (vbox, 8);
        gtk_widget_set_hexpand (vbox, TRUE);
        gtk_widget_set_vexpand (vbox, TRUE);
        fabulor_gtk_box_append (GTK_BOX (content), vbox, TRUE, TRUE, 0);

        content_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
        gtk_widget_set_hexpand (content_hbox, TRUE);
        gtk_widget_set_vexpand (content_hbox, TRUE);
        fabulor_gtk_box_append (GTK_BOX (vbox), content_hbox, TRUE, TRUE, 0);

        left_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
        gtk_widget_set_hexpand (left_box, TRUE);
        gtk_widget_set_vexpand (left_box, TRUE);
        fabulor_gtk_box_append (GTK_BOX (content_hbox), left_box, TRUE, TRUE, 0);

        search = gtk_search_entry_new ();
        gtk_entry_set_placeholder_text (GTK_ENTRY (search), _("Search colors by name"));
        fabulor_gtk_box_append (GTK_BOX (left_box), search, FALSE, FALSE, 0);
        ui->search_entry = search;

        scroller = gtk_scrolled_window_new (NULL, NULL);
        gtk_widget_set_hexpand (scroller, TRUE);
        gtk_widget_set_vexpand (scroller, TRUE);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
        fabulor_gtk_box_append (GTK_BOX (left_box), scroller, TRUE, TRUE, 0);

        list = gtk_list_box_new ();
        gtk_widget_set_hexpand (list, TRUE);
        gtk_widget_set_vexpand (list, TRUE);
        gtk_list_box_set_selection_mode (GTK_LIST_BOX (list), GTK_SELECTION_NONE);
        fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroller), list);

        preview_frame = theme_preferences_manager_create_preview (ui);
        gtk_widget_set_size_request (preview_frame, 300, -1);
        gtk_widget_set_hexpand (preview_frame, FALSE);
        gtk_widget_set_vexpand (preview_frame, TRUE);
        fabulor_gtk_box_append (GTK_BOX (content_hbox), preview_frame, FALSE, TRUE, 0);

        for (token = THEME_TOKEN_MIRC_0; token < THEME_TOKEN_COUNT; token++)
        {
                theme_color_manager_row *row;
                GtkWidget *list_row;
                GtkWidget *hbox;
                GtkWidget *name;
                GtkWidget *preview;
                GtkWidget *button;
                GtkWidget *entry;
                GdkRGBA rgba;
                char *display;
                char *search_text;
                char *token_code;

                list_row = gtk_list_box_row_new ();
                hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
                fabulor_gtk_container_set_uniform_inset (hbox, 4);
                fabulor_gtk_list_box_row_set_child (GTK_LIST_BOX_ROW (list_row),
                        hbox);

                display = theme_preferences_token_display_name (token);
                name = gtk_label_new (display);
                gtk_widget_set_halign (name, GTK_ALIGN_START);
                gtk_widget_set_hexpand (name, TRUE);
                fabulor_gtk_box_append (GTK_BOX (hbox), name, TRUE, TRUE, 0);

                preview = gtk_label_new (_("Preview"));
                gtk_widget_set_size_request (preview, 90, -1);
                gtk_widget_set_halign (preview, GTK_ALIGN_CENTER);
                fabulor_gtk_box_append (GTK_BOX (hbox), preview, FALSE, FALSE, 0);

                button = gtk_button_new_with_label (_("Choose…"));
                fabulor_gtk_box_append (GTK_BOX (hbox), button, FALSE, FALSE, 0);

                entry = gtk_entry_new ();
                fabulor_gtk_entry_set_width_chars (GTK_ENTRY (entry), 9);
                gtk_entry_set_max_length (GTK_ENTRY (entry), 9);
                fabulor_gtk_box_append (GTK_BOX (hbox), entry, FALSE, FALSE, 0);

                row = g_new0 (theme_color_manager_row, 1);
                row->row = list_row;
                row->button = button;
                row->entry = entry;
                row->preview = preview;
                row->token = token;
                row->color_change_flag = color_change_flag;
                row->parent = GTK_WINDOW (dialog);
                row->manager_ui = ui;

                token_code = g_strdup_printf ("token_%d", token);
                search_text = g_strconcat (display, " ", token_code, NULL);
                row->search_text = g_utf8_strdown (search_text, -1);
                g_free (token_code);
                g_free (search_text);

                if (theme_preferences_staged_get_color (token, &rgba))
                        theme_preferences_manager_row_apply (row, &rgba);

                g_signal_connect (G_OBJECT (button), "clicked",
                                  G_CALLBACK (theme_preferences_manager_pick_cb), row);
                g_object_set_data (G_OBJECT (button), "zoitechat-theme-color-manager-ui", ui);
                g_signal_connect (G_OBJECT (entry), "activate",
                                  G_CALLBACK (theme_preferences_manager_entry_activate_cb), row);
                fabulor_gtk_widget_on_focus_leave (
                        entry, theme_preferences_manager_entry_focus_out_cb, row);

                fabulor_gtk_list_box_append (GTK_LIST_BOX (list), list_row);
                g_ptr_array_add (ui->rows, row);
                g_free (display);
        }

        g_signal_connect (G_OBJECT (search), "changed",
                          G_CALLBACK (theme_preferences_manager_search_changed_cb), ui);

        theme_preferences_manager_update_preview (ui);

        fabulor_gtk_widget_reveal_tree (dialog);
        return dialog;
}

static void
theme_preferences_manage_colors_cb (GtkWidget *button, gpointer user_data)
{
        gboolean *color_change_flag = user_data;

        theme_preferences_create_color_manager_dialog (
                fabulor_gtk_widget_get_root_window (button), color_change_flag);
}

static void
theme_preferences_show_import_error (GtkWidget *button, const char *message)
{
        GtkWidget *dialog;

        dialog = gtk_message_dialog_new (fabulor_gtk_widget_get_root_window (button),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_ERROR,
                                         GTK_BUTTONS_CLOSE,
                                         "%s",
                                         message);
        g_signal_connect (dialog, "response",
                          G_CALLBACK (fabulor_gtk_dialog_destroy_on_response), NULL);
        gtk_widget_show (dialog);
}

static void
theme_preferences_show_import_info (GtkWidget *button, const char *message)
{
        GtkWidget *dialog;

        dialog = gtk_message_dialog_new (fabulor_gtk_widget_get_root_window (button),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_INFO,
                                         GTK_BUTTONS_CLOSE,
                                         "%s",
                                         message);
        g_signal_connect (dialog, "response",
                          G_CALLBACK (fabulor_gtk_dialog_destroy_on_response), NULL);
        gtk_widget_show (dialog);
}

static gboolean
theme_preferences_parse_cfg_color (const char *cfg,
                                   const char *key,
                                   guint16 *red,
                                   guint16 *green,
                                   guint16 *blue)
{
        const char *line;
        size_t key_len;

        if (!cfg || !key || !red || !green || !blue)
                return FALSE;

        key_len = strlen (key);
        line = cfg;

        while (*line)
        {
                const char *line_end;
                const char *p;

                while (*line == '\n' || *line == '\r')
                        line++;
                if (!*line)
                        break;

                line_end = strchr (line, '\n');
                if (!line_end)
                        line_end = line + strlen (line);

                p = line;
                while (p < line_end && g_ascii_isspace (*p))
                        p++;

                if ((size_t) (line_end - p) > key_len &&
                    strncmp (p, key, key_len) == 0)
                {
                        unsigned int r;
                        unsigned int g;
                        unsigned int b;

                        p += key_len;
                        while (p < line_end && g_ascii_isspace (*p))
                                p++;
                        if (p < line_end && *p == '=')
                                p++;
                        while (p < line_end && g_ascii_isspace (*p))
                                p++;

                        if (sscanf (p, "%x %x %x", &r, &g, &b) == 3)
                        {
                                *red = (guint16) CLAMP (r, 0, 0xffff);
                                *green = (guint16) CLAMP (g, 0, 0xffff);
                                *blue = (guint16) CLAMP (b, 0, 0xffff);
                                return TRUE;
                        }
                }

                line = line_end;
        }

        return FALSE;
}

static gboolean
theme_preferences_read_import_color (const char *cfg,
                                     ThemeSemanticToken token,
                                     unsigned int mode,
                                     GdkRGBA *rgba)
{
        static const char *token_names[] = {
                "mirc_0", "mirc_1", "mirc_2", "mirc_3", "mirc_4", "mirc_5", "mirc_6", "mirc_7",
                "mirc_8", "mirc_9", "mirc_10", "mirc_11", "mirc_12", "mirc_13", "mirc_14", "mirc_15",
                "mirc_16", "mirc_17", "mirc_18", "mirc_19", "mirc_20", "mirc_21", "mirc_22", "mirc_23",
                "mirc_24", "mirc_25", "mirc_26", "mirc_27", "mirc_28", "mirc_29", "mirc_30", "mirc_31",
                "selection_foreground", "selection_background", "text_foreground", "text_background", "marker",
                "tab_new_data", "tab_highlight", "tab_new_message", "tab_away", "spell"
        };
        char key[256];
        const char *mode_name;
        const char *legacy_prefix;
        guint16 red;
        guint16 green;
        guint16 blue;
        int legacy_key;

        if (token < 0 || token >= THEME_TOKEN_COUNT)
                return FALSE;

        if (mode == ZOITECHAT_DARK_MODE_DARK)
        {
                mode_name = "dark";
                legacy_prefix = "dark_color";
        }
        else
        {
                mode_name = "light";
                legacy_prefix = "color";
        }

        g_snprintf (key, sizeof key, "theme.mode.%s.token.%s", mode_name, token_names[token]);
        if (!theme_preferences_parse_cfg_color (cfg, key, &red, &green, &blue))
        {
                legacy_key = token < 32 ? token : (token - 32) + 256;
                g_snprintf (key, sizeof key, "%s_%d", legacy_prefix, legacy_key);
                if (!theme_preferences_parse_cfg_color (cfg, key, &red, &green, &blue))
                        return FALSE;
        }

        rgba->red = red / 65535.0;
        rgba->green = green / 65535.0;
        rgba->blue = blue / 65535.0;
        rgba->alpha = 1.0;
        return TRUE;
}

static void
theme_preferences_import_colors_conf_path (GtkWidget *button,
                                           gboolean *color_change_flag,
                                           char *path)
{
        char *lower_path;
        char *cfg;
        char *pevents_cfg = NULL;
        GError *error = NULL;
        gboolean any_imported = FALSE;
        gboolean imported_from_hct = FALSE;
        gboolean imported_pevents = FALSE;
        unsigned int import_mode;
        ThemeSemanticToken token;

        lower_path = g_ascii_strdown (path, -1);
        if (g_str_has_suffix (lower_path, ".hct"))
        {
                imported_from_hct = TRUE;
                if (!fabulor_theme_archive_read_text_file (path, "colors.conf", &cfg, &error))
                {
                        theme_preferences_show_import_error (button, _("Failed to read colors.conf from .hct file."));
                        g_clear_error (&error);
                        g_free (lower_path);
                        g_free (path);
                        return;
                }
                if (fabulor_theme_archive_read_text_file (path, "pevents.conf", &pevents_cfg, &error))
                {
                        char *pevents_path = g_build_filename (get_xdir (), "pevents.conf", NULL);
                        if (g_file_set_contents (pevents_path, pevents_cfg, -1, &error))
                        {
                                load_text_events ();
                                imported_pevents = TRUE;
                        }
                        g_free (pevents_path);
                        g_clear_error (&error);
                }
                else
                        g_clear_error (&error);
        }
        else if (!g_file_get_contents (path, &cfg, NULL, &error))
        {
                theme_preferences_show_import_error (button, _("Failed to read colors.conf file."));
                g_clear_error (&error);
                g_free (lower_path);
                g_free (path);
                return;
        }
        g_free (lower_path);

        import_mode = theme_preferences_stage_color_mode ();
        for (token = THEME_TOKEN_MIRC_0; token < THEME_TOKEN_COUNT; token++)
        {
                GdkRGBA rgba;

                if (!theme_preferences_read_import_color (cfg, token, import_mode, &rgba))
                        continue;

                theme_preferences_staged_set_color (token, &rgba, color_change_flag, TRUE);
                any_imported = TRUE;
        }

        if (!any_imported)
                theme_preferences_show_import_error (button, _("No importable colors were found in that colors.conf file."));
        else if (imported_from_hct)
        {
                char *message = g_strdup_printf (imported_pevents ?
                                                 _("Imported colors.conf and pevents.conf from %s.") :
                                                 _("Imported colors.conf from %s."),
                                                 path);
                theme_preferences_show_import_info (button, message);
                g_free (message);
        }
        else if (color_change_flag)
                *color_change_flag = theme_preferences_stage.active ? theme_preferences_stage.changed : *color_change_flag;

        if (any_imported && color_change_flag)
                *color_change_flag = theme_preferences_stage.active ? theme_preferences_stage.changed : *color_change_flag;

        g_free (pevents_cfg);
        g_free (cfg);
        g_free (path);
}

static void
theme_preferences_import_colors_conf_response_cb (GtkNativeDialog *dialog,
                                                  gint response_id,
                                                  gpointer user_data)
{
        theme_preferences_native_import_data *data = user_data;
        GtkWidget *button;
        gboolean *color_change_flag;
        char *path = NULL;

        button = theme_preferences_native_import_acquire_owner (dialog, data);
        if (button && response_id == GTK_RESPONSE_ACCEPT)
        {
                path = fabulor_gtk_file_chooser_dup_filename (
                        GTK_FILE_CHOOSER (dialog));
                color_change_flag = g_object_get_data (
                        G_OBJECT (button), "fabulor-theme-colors-import-context");
                if (path)
                        theme_preferences_import_colors_conf_path (button, color_change_flag, path);
        }

        g_clear_object (&button);
        g_object_unref (dialog);
}

static void
theme_preferences_import_colors_conf_cb (GtkWidget *button, gpointer user_data)
{
        GtkFileChooserNative *dialog;
        GtkFileFilter *filter;
        GtkWindow *parent;
        theme_preferences_native_import_data *data;

        parent = fabulor_gtk_widget_get_root_window (button);
        if (!parent)
                return;

        g_object_set_data (G_OBJECT (button), "fabulor-theme-colors-import-context", user_data);
        dialog = gtk_file_chooser_native_new (_("Import colors.conf colors"),
                                              parent,
                                              GTK_FILE_CHOOSER_ACTION_OPEN,
                                              _("_Import"),
                                              _("_Cancel"));
        gtk_native_dialog_set_modal (GTK_NATIVE_DIALOG (dialog), TRUE);
        fabulor_gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), TRUE);
        gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), FALSE);
        filter = gtk_file_filter_new ();
        gtk_file_filter_set_name (filter, _("Theme colors (*.conf, *.hct)"));
        gtk_file_filter_add_pattern (filter, "*.conf");
        gtk_file_filter_add_pattern (filter, "*.hct");
        gtk_file_filter_add_pattern (filter, "*.HCT");
        gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

        data = theme_preferences_native_import_data_new (
                GTK_NATIVE_DIALOG (dialog), button, parent);
        g_signal_connect (dialog, "response",
                          G_CALLBACK (theme_preferences_import_colors_conf_response_cb), data);
        gtk_native_dialog_show (GTK_NATIVE_DIALOG (dialog));
}

static void
theme_preferences_create_color_button (GtkWidget *table,
                                       ThemeSemanticToken token,
                                       int row,
                                       int col,
                                       GtkWindow *parent,
                                       gboolean *color_change_flag)
{
        GtkWidget *but;
        GtkWidget *label;
        GtkWidget *box;
        char buf[64];
        GdkRGBA color;

        if (token > THEME_TOKEN_MIRC_31)
                strcpy (buf, "<span size=\"x-small\">&#x2007;&#x2007;</span>");
        else if (token < 10)
                sprintf (buf, "<span size=\"x-small\">&#x2007;%d</span>", token);
        else
                sprintf (buf, "<span size=\"x-small\">%d</span>", token);

        but = gtk_button_new ();
        label = gtk_label_new (" ");
        gtk_label_set_markup (GTK_LABEL (label), buf);
        box = fabulor_gtk_content_surface_new (TRUE);
        fabulor_gtk_content_surface_set_child (box, label);
        fabulor_gtk_button_set_child (GTK_BUTTON (but), box);
        gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
        gtk_widget_set_valign (box, GTK_ALIGN_CENTER);
        gtk_widget_show (label);
        gtk_widget_show (box);
        g_object_set_data (G_OBJECT (but), "zoitechat-color", (gpointer)1);
        g_object_set_data (G_OBJECT (but), "zoitechat-color-box", box);
        g_object_set_data (G_OBJECT (but), "zoitechat-theme-token", GINT_TO_POINTER (token));
        g_object_set_data (G_OBJECT (but), "zoitechat-theme-color-change", color_change_flag);
        gtk_grid_attach (GTK_GRID (table), but, col, row, 1, 1);
        g_signal_connect (G_OBJECT (but), "clicked", G_CALLBACK (theme_preferences_color_cb), parent);
        if (theme_preferences_staged_get_color (token, &color))
                theme_preferences_color_button_apply (but, &color);
}

static void
theme_preferences_create_header (GtkWidget *table, int row, const char *labeltext)
{
        GtkWidget *label;
        char buf[128];

        if (row == 0)
                g_snprintf (buf, sizeof (buf), "<b>%s</b>", _(labeltext));
        else
                g_snprintf (buf, sizeof (buf), "\n<b>%s</b>", _(labeltext));

        label = gtk_label_new (NULL);
        gtk_label_set_markup (GTK_LABEL (label), buf);
        gtk_widget_set_halign (label, GTK_ALIGN_START);
        gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
        gtk_grid_attach (GTK_GRID (table), label, 0, row, 4, 1);
        gtk_widget_set_margin_bottom (label, 5);
}

static void
theme_preferences_create_other_color_l (GtkWidget *tab,
                                        const char *text,
                                        ThemeSemanticToken token,
                                        int row,
                                        GtkWindow *parent,
                                        gboolean *color_change_flag)
{
        GtkWidget *label;

        label = gtk_label_new (text);
        gtk_widget_set_halign (label, GTK_ALIGN_START);
        gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
        gtk_widget_set_margin_start (label, LABEL_INDENT);
        gtk_grid_attach (GTK_GRID (tab), label, 2, row, 1, 1);
        theme_preferences_create_color_button (tab, token, row, 3, parent, color_change_flag);
}

static void
theme_preferences_create_other_color_r (GtkWidget *tab,
                                        const char *text,
                                        ThemeSemanticToken token,
                                        int row,
                                        GtkWindow *parent,
                                        gboolean *color_change_flag)
{
        GtkWidget *label;

        label = gtk_label_new (text);
        gtk_widget_set_halign (label, GTK_ALIGN_START);
        gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
        gtk_widget_set_margin_start (label, LABEL_INDENT);
        gtk_grid_attach (GTK_GRID (tab), label, 5, row, 4, 1);
        theme_preferences_create_color_button (tab, token, row, 9, parent, color_change_flag);
}

static void
theme_preferences_strip_toggle_cb (GtkToggleButton *toggle, gpointer user_data)
{
        int *field = user_data;

        *field = gtk_toggle_button_get_active (toggle);
}

static void
theme_preferences_create_strip_toggle (GtkWidget *tab,
                                       int row,
                                       const char *text,
                                       int *field)
{
        GtkWidget *toggle;

        toggle = gtk_check_button_new_with_label (text);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), *field);
        g_signal_connect (G_OBJECT (toggle), "toggled",
                          G_CALLBACK (theme_preferences_strip_toggle_cb), field);
        gtk_grid_attach (GTK_GRID (tab), toggle, 2, row, 1, 1);
}

GtkWidget *
theme_preferences_create_color_page (GtkWindow *parent,
                                     struct zoitechatprefs *setup_prefs,
                                     gboolean *color_change_flag)
{
        GtkWidget *tab;
        GtkWidget *box;
        GtkWidget *label;
        GtkWidget *manage_colors_button;
        int i;

        box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        fabulor_gtk_container_set_uniform_inset (box, 6);

        tab = gtk_grid_new ();
        fabulor_gtk_container_set_uniform_inset (tab, 6);
        gtk_grid_set_row_spacing (GTK_GRID (tab), 2);
        gtk_grid_set_column_spacing (GTK_GRID (tab), 3);
        fabulor_gtk_box_append (GTK_BOX (box), tab, FALSE, TRUE, 0);

        theme_preferences_create_header (tab, 0, N_("Text Colors"));

        label = gtk_label_new (_("mIRC colors:"));
        gtk_widget_set_halign (label, GTK_ALIGN_START);
        gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
        gtk_widget_set_margin_start (label, LABEL_INDENT);
        gtk_grid_attach (GTK_GRID (tab), label, 2, 1, 1, 1);

        for (i = 0; i < 16; i++)
                theme_preferences_create_color_button (tab,
                                                       THEME_TOKEN_MIRC_0 + i,
                                                       1,
                                                       i + 3,
                                                       parent,
                                                       color_change_flag);

        label = gtk_label_new (_("Local colors:"));
        gtk_widget_set_halign (label, GTK_ALIGN_START);
        gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
        gtk_widget_set_margin_start (label, LABEL_INDENT);
        gtk_grid_attach (GTK_GRID (tab), label, 2, 2, 1, 1);

        for (i = 16; i < 32; i++)
                theme_preferences_create_color_button (tab,
                                                       THEME_TOKEN_MIRC_0 + i,
                                                       2,
                                                       (i + 3) - 16,
                                                       parent,
                                                       color_change_flag);

        theme_preferences_create_other_color_l (tab, _("Foreground:"), THEME_TOKEN_TEXT_FOREGROUND, 3,
                                                parent, color_change_flag);
        theme_preferences_create_other_color_r (tab, _("Background:"), THEME_TOKEN_TEXT_BACKGROUND, 3,
                                                parent, color_change_flag);

        theme_preferences_create_header (tab, 5, N_("Selected Text"));
        theme_preferences_create_other_color_l (tab, _("Foreground:"), THEME_TOKEN_SELECTION_FOREGROUND, 6,
                                                parent, color_change_flag);
        theme_preferences_create_other_color_r (tab, _("Background:"), THEME_TOKEN_SELECTION_BACKGROUND, 6,
                                                parent, color_change_flag);

        theme_preferences_create_header (tab, 8, N_("Interface Colors"));
        theme_preferences_create_other_color_l (tab, _("New data:"), THEME_TOKEN_TAB_NEW_DATA, 9,
                                                parent, color_change_flag);
        theme_preferences_create_other_color_r (tab, _("Marker line:"), THEME_TOKEN_MARKER, 9,
                                                parent, color_change_flag);
        theme_preferences_create_other_color_l (tab, _("New message:"), THEME_TOKEN_TAB_NEW_MESSAGE, 10,
                                                parent, color_change_flag);
        theme_preferences_create_other_color_r (tab, _("Away user:"), THEME_TOKEN_TAB_AWAY, 10,
                                                parent, color_change_flag);
        theme_preferences_create_other_color_l (tab, _("Highlight:"), THEME_TOKEN_TAB_HIGHLIGHT, 11,
                                                parent, color_change_flag);
        theme_preferences_create_other_color_r (tab, _("Spell checker:"), THEME_TOKEN_SPELL, 11,
                                                parent, color_change_flag);
        theme_preferences_create_header (tab, 15, N_("Color Stripping"));
        theme_preferences_create_strip_toggle (tab, 16, _("Messages"), &setup_prefs->hex_text_stripcolor_msg);
        theme_preferences_create_strip_toggle (tab, 17, _("Scrollback"), &setup_prefs->hex_text_stripcolor_replay);
        theme_preferences_create_strip_toggle (tab, 18, _("Topic"), &setup_prefs->hex_text_stripcolor_topic);

        manage_colors_button = gtk_button_new_with_label (_("Manage all client colors…"));
        gtk_widget_set_halign (manage_colors_button, GTK_ALIGN_START);
        gtk_widget_set_margin_start (manage_colors_button, LABEL_INDENT);
        gtk_widget_set_margin_top (manage_colors_button, 10);
        fabulor_gtk_box_append (GTK_BOX (box), manage_colors_button, FALSE, FALSE, 0);
        g_signal_connect (G_OBJECT (manage_colors_button), "clicked",
                          G_CALLBACK (theme_preferences_manage_colors_cb), color_change_flag);

        return box;
}

static void
theme_preferences_gtk4_commit (const char *theme_id, guint variant,
	gpointer user_data)
{
	struct zoitechatprefs *setup_prefs = user_data;

	g_strlcpy (setup_prefs->hex_gui_gtk4_theme, theme_id ? theme_id : "",
		sizeof (setup_prefs->hex_gui_gtk4_theme));
	setup_prefs->hex_gui_gtk4_variant = variant;
	theme_manager_dispatch_changed (THEME_CHANGED_REASON_THEME_PACK |
		THEME_CHANGED_REASON_WIDGET_STYLE | THEME_CHANGED_REASON_MODE);
}

GtkWidget *
theme_preferences_create_page (GtkWindow *parent,
                               struct zoitechatprefs *setup_prefs,
                               gboolean *color_change_flag)
{
	ThemePreferencesGtk4 *preferences;
	GtkWidget *box;
	GtkWidget *surface;
	GError *error = NULL;
	(void) parent;
	(void) color_change_flag;

	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
	preferences = theme_preferences_gtk4_new_with_controller (
		theme_manager_gtk4_controller (), get_xdir (),
		setup_prefs->hex_gui_gtk4_theme, setup_prefs->hex_gui_gtk4_variant,
		theme_manager_gtk4_prefers_dark (),
		theme_manager_gtk4_high_contrast (), theme_preferences_gtk4_commit,
		setup_prefs, NULL, &error);
	if (!preferences)
	{
		GtkWidget *label = gtk_label_new (error ? error->message :
			_("GTK4 theme preferences are unavailable."));

		gtk_label_set_wrap (GTK_LABEL (label), TRUE);
		gtk_label_set_xalign (GTK_LABEL (label), 0.0f);
		gtk_box_append (GTK_BOX (box), label);
		g_clear_error (&error);
		return box;
	}
	surface = theme_preferences_gtk4_widget (preferences);
	gtk_box_append (GTK_BOX (box), surface);
	g_object_set_data_full (G_OBJECT (box), "theme-preferences-gtk4",
		preferences, (GDestroyNotify) theme_preferences_gtk4_free);
	return box;
}
void
theme_preferences_apply_to_session (session_gui *gui, InputStyle *input_style)
{
        if (gui->user_tree)
        {
                theme_manager_apply_userlist_style (gui->user_tree,
                                                    theme_manager_get_userlist_palette_behavior (input_style ? input_style->font_desc : NULL));
        }
}
