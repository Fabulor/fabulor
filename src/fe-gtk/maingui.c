/* X-Chat
 * Copyright (C) 1998-2005 Peter Zelezny.
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "fe-gtk.h"

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkcairo.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "../common/zoitechat.h"
#include "../common/fe.h"
#include "../common/server.h"
#include "../common/zoitechatc.h"
#include "../common/outbound.h"
#include "../common/inbound.h"
#include "../common/plugin.h"
#include "../common/modes.h"
#include "../common/servlist.h"
#include "../common/url.h"
#include "../common/util.h"
#include "../common/text.h"
#include "../common/chanopt.h"
#include "../common/cfgfiles.h"

#include "theme/theme-manager.h"
#include "theme/theme-css.h"
#include "banlist.h"
#include "gtkutil.h"
#include "context-menu-presenter-gtk4.h"
#include "tab-context-menu-model.h"
#include "gtk-compat.h"
#include "emoji-picker.h"
#include "icon-resolver.h"
#include "joind.h"
#include "theme/theme-access.h"
#include "theme/theme-palette.h"
#include "maingui.h"
#include "menu.h"
#include "preferences-persistence.h"
#include "window-state.h"
#include "window-geometry.h"
#include "fkeys.h"
#include "userlistgui.h"
#include "user-list-model.h"
#include "user-list-view.h"
#include "chanview.h"
#include "pixmaps.h"
#include "plugin-tray.h"
#include "xtext.h"
#include "ui-performance-profile.h"
#include "sexy-spell-entry.h"
#include "servlistgui.h"
#include "gtkutil.h"

#ifdef G_OS_WIN32
#include <windows.h>
#include <gdk/win32/gdkwin32.h>
#include <glib/gwin32.h>
#endif

#define ICON_TAB_DETACH "zc-menu-detach"
#define ICON_TAB_CLOSE "zc-menu-close"
#define ICON_TAB_PREVIOUS "zc-menu-previous"
#define ICON_TAB_NEXT "zc-menu-next"
#define ICON_ENTRY_ERROR "dialog-error"

#define GUI_SPACING (3)
#define GUI_BORDER (0)

enum
{
        POS_INVALID = 0,
        POS_TOPLEFT = 1,
        POS_BOTTOMLEFT = 2,
        POS_TOPRIGHT = 3,
        POS_BOTTOMRIGHT = 4,
        POS_TOP = 5,    /* for tabs only */
        POS_BOTTOM = 6,
        POS_HIDDEN = 7
};

/* two different types of tabs */
#define TAG_IRC 0               /* server, channel, dialog */
#define TAG_UTIL 1      /* dcc, notify, chanlist */

static void mg_apply_emoji_fallback_widget (GtkWidget *widget);
static void mg_inputbox_insert_emoji_cb (GtkEntry *entry, gpointer user_data);
static void mg_inputbox_icon_release_cb (GtkEntry *entry, GtkEntryIconPosition icon_pos, gpointer user_data);
static void mg_schedule_rightpane_restore (session_gui *gui);
#define MG_CONFIG_SAVE_DEBOUNCE_MS 250

static guint mg_config_save_source_id = 0;
static gboolean mg_config_prefs_dirty = FALSE;

static void
mg_show_save_failure (const PreferencesPersistenceResult *save_result)
{
        char buffer[192];

        if (!save_result || save_result->success)
                return;

        if (save_result->partial_failure)
        {
                fe_message (_("Could not fully save preferences. fabulor.conf was written, but colors.conf failed. Retry is possible."), FE_MSG_ERROR);
                return;
        }

        g_snprintf (buffer, sizeof (buffer), _("Could not save preferences (%s). Retry is possible."), save_result->failed_file ? save_result->failed_file : _("unknown file"));
        fe_message (buffer, FE_MSG_ERROR);
}

static gboolean
mg_config_save_timeout_cb (gpointer userdata)
{
        PreferencesPersistenceResult save_result;

        mg_config_save_source_id = 0;

        if (!mg_config_prefs_dirty)
                return G_SOURCE_REMOVE;

        save_result = preferences_persistence_save_all ();
        if (!save_result.success)
                mg_show_save_failure (&save_result);
        mg_config_prefs_dirty = FALSE;

        return G_SOURCE_REMOVE;
}

static void
mg_schedule_config_save (void)
{
        if (!mg_config_prefs_dirty)
                return;

        if (mg_config_save_source_id != 0)
                g_source_remove (mg_config_save_source_id);

        mg_config_save_source_id = g_timeout_add (MG_CONFIG_SAVE_DEBOUNCE_MS,
                                                                                           mg_config_save_timeout_cb,
                                                                                           NULL);
}

static void
mg_flush_config_save (void)
{
        PreferencesPersistenceResult save_result;

        if (mg_config_save_source_id != 0)
        {
                g_source_remove (mg_config_save_source_id);
                mg_config_save_source_id = 0;
        }

        if (mg_config_prefs_dirty)
        {
                save_result = preferences_persistence_save_all ();
                if (!save_result.success)
                        mg_show_save_failure (&save_result);
                mg_config_prefs_dirty = FALSE;
        }
}

static inline void
mg_set_source_color (cairo_t *cr, const XTextColor *color)
{
	cairo_set_source_rgba (cr, color->red, color->green, color->blue, color->alpha);
}

static inline guint16
mg_color_component_to_pango (double value)
{
	if (value < 0.0)
		value = 0.0;
	if (value > 1.0)
		value = 1.0;

	return (guint16)(value * 65535.0 + 0.5);
}

static void
mg_apply_font_css (GtkWidget *widget, const PangoFontDescription *desc,
				   const char *class_name, const char *provider_key)
{
	GtkStyleContext *context;
	GtkCssProvider *provider;
	GString *css;

	if (!widget || !desc)
		return;

	context = gtk_widget_get_style_context (widget);
	if (!context)
		return;

	provider = g_object_get_data (G_OBJECT (widget), provider_key);
	if (!provider)
	{
		provider = gtk_css_provider_new ();
		g_object_set_data_full (G_OBJECT (widget), provider_key, provider, g_object_unref);
	}

	css = g_string_new (".");
	g_string_append (css, class_name);
	g_string_append (css, " {");
	gtkutil_append_font_css (css, desc);
	g_string_append (css, " }");
	gtk_css_provider_load_from_data (provider, css->str, -1);
	g_string_free (css, TRUE);

	gtk_style_context_add_class (context, class_name);
	theme_css_apply_widget_provider (widget, GTK_STYLE_PROVIDER (provider));
}

static void
mg_set_label_alignment_start (GtkWidget *widget)
{
	gtk_widget_set_halign (widget, GTK_ALIGN_START);
	gtk_widget_set_valign (widget, GTK_ALIGN_CENTER);
}

static void
mg_apply_compact_mode_css (GtkWidget *widget)
{
	GtkStyleContext *context;
	GtkCssProvider *provider;

	if (!widget)
		return;

	context = gtk_widget_get_style_context (widget);
	if (!context)
		return;

	provider = g_object_get_data (G_OBJECT (widget), "mg-mode-css-provider");
	if (!provider)
	{
		provider = gtk_css_provider_new ();
		g_object_set_data_full (G_OBJECT (widget), "mg-mode-css-provider", provider, g_object_unref);
	}

	gtk_css_provider_load_from_data (provider,
		".zoitechat-mode-control { min-height: 11px; padding-top: 0; padding-bottom: 0; }"
		".zoitechat-mode-control label { padding-top: 0; padding-bottom: 0; }",
		-1);
	gtk_style_context_add_class (context, "zoitechat-mode-control");
	theme_css_apply_widget_provider (widget, GTK_STYLE_PROVIDER (provider));
}

static GtkWidget *
mg_box_new (GtkOrientation orientation, gboolean homogeneous, gint spacing)
{
	GtkWidget *box = gtk_box_new (orientation, spacing);

	gtk_box_set_homogeneous (GTK_BOX (box), homogeneous);
	return box;
}


static void mg_create_entry (session *sess, GtkWidget *box);
static void mg_create_search (session *sess, GtkWidget *box);
static void mg_link_irctab (session *sess, int focus);
static void mg_topwindow_lifecycle_connect (GtkWidget *window, session *sess);
static void mg_topwindow_lifecycle_disconnect (GtkWidget *window, session *sess);
static void mg_tabwindow_lifecycle_connect (GtkWidget *window, session_gui *gui);
static void mg_tabwindow_lifecycle_disconnect (GtkWidget *window, session_gui *gui);
static void mg_theme_window_cleanup (GtkWidget *window, session_gui *gui);

static session_gui static_mg_gui;
static session_gui *mg_gui = NULL;      /* the shared irc tab */
static int ignore_chanmode = FALSE;
static const char chan_flags[] = { 'c', 'n', 't', 'i', 'm', 'l', 'k' };
typedef struct
{
	int server_id;
	char channel[CHANLEN];
	char key[64];
}
mg_closed_channel_tab;
static GSList *mg_closed_channel_tabs;

static chan *active_tab = NULL; /* active tab */
GtkWidget *parent_window = NULL;                        /* the master window */
static GtkWidget *quit_dialog = NULL;
static GtkWidget *font_error_dialog = NULL;

#ifdef G_OS_WIN32
static GdkDisplay *mg_win32_filter_display;
static gboolean mg_win32_filter_installed;
#endif

InputStyle *input_style;

static PangoAttrList *away_list;
static PangoAttrList *newdata_list;
static PangoAttrList *nickseen_list;
static PangoAttrList *newmsg_list;
static PangoAttrList *plain_list = NULL;

static PangoAttrList *
mg_attr_list_create (const XTextColor *col, int size)
{
        PangoAttribute *attr;
        PangoAttrList *list;

        list = pango_attr_list_new ();

        if (col)
        {
                attr = pango_attr_foreground_new (
                        mg_color_component_to_pango (col->red),
                        mg_color_component_to_pango (col->green),
                        mg_color_component_to_pango (col->blue));
                attr->start_index = 0;
                attr->end_index = 0xffff;
                pango_attr_list_insert (list, attr);
        }

        if (size > 0)
        {
                attr = pango_attr_scale_new (size == 1 ? PANGO_SCALE_SMALL : PANGO_SCALE_X_SMALL);
                attr->start_index = 0;
                attr->end_index = 0xffff;
                pango_attr_list_insert (list, attr);
        }

        return list;
}

static void
mg_create_tab_colors (void)
{
        XTextColor gui_palette[THEME_TOKEN_COUNT];

        if (plain_list)
        {
                pango_attr_list_unref (plain_list);
                pango_attr_list_unref (newmsg_list);
                pango_attr_list_unref (newdata_list);
                pango_attr_list_unref (nickseen_list);
                pango_attr_list_unref (away_list);
        }

        theme_get_xtext_colors (gui_palette, G_N_ELEMENTS (gui_palette));
        plain_list = mg_attr_list_create (NULL, prefs.hex_gui_tab_small);
        newdata_list = mg_attr_list_create (&gui_palette[THEME_TOKEN_TAB_NEW_DATA], prefs.hex_gui_tab_small);
        nickseen_list = mg_attr_list_create (&gui_palette[THEME_TOKEN_TAB_HIGHLIGHT], prefs.hex_gui_tab_small);
        newmsg_list = mg_attr_list_create (&gui_palette[THEME_TOKEN_TAB_NEW_MESSAGE], prefs.hex_gui_tab_small);
        away_list = mg_attr_list_create (&gui_palette[THEME_TOKEN_TAB_AWAY], FALSE);
}

static void
set_window_urgency (GtkWidget *win, gboolean set)
{
        fabulor_gtk_window_set_urgent (GTK_WINDOW (win), set);
}

static gboolean
is_wayland_display (void)
{
        GdkDisplay *display = gdk_display_get_default ();
        const char *name;

        if (!display)
                return FALSE;

        name = gdk_display_get_name (display);
        if (!name)
                return FALSE;

        return g_str_has_prefix (name, "wayland");
}

static gboolean
is_kde_desktop (void)
{
        const char *desktop = g_getenv ("XDG_CURRENT_DESKTOP");

        if (desktop && strstr (desktop, "KDE"))
                return TRUE;

        return g_getenv ("KDE_FULL_SESSION") != NULL;
}

static gboolean
is_kde_wayland (void)
{
        return is_wayland_display () && is_kde_desktop ();
}

static void
flash_window (GtkWidget *win)
{
        if (is_kde_wayland ())
                gtk_window_present (GTK_WINDOW (win));
        set_window_urgency (win, TRUE);
}

static void
unflash_window (GtkWidget *win)
{
        set_window_urgency (win, FALSE);
}

/* flash the taskbar button */

void
fe_flash_window (session *sess)
{
        if (fe_gui_info (sess, 0) != 1) /* only do it if not focused */
                flash_window (sess->gui->window);
}

/* set a tab plain, red, light-red, or blue */

void
fe_set_tab_color (struct session *sess, tabcolor col)
{
        struct session *server_sess = sess->server->server_session;
        int col_noflags = (col & ~FE_COLOR_ALLFLAGS);
        int col_shouldoverride = !(col & FE_COLOR_FLAG_NOOVERRIDE);

        if (sess->res->tab && sess->gui->is_tab && (col == 0 || sess != current_tab))
        {
                switch (col_noflags)
                {
                case 0: /* no particular color (theme default) */
                        sess->tab_state = TAB_STATE_NONE;
                        chan_set_color (sess->res->tab, plain_list);
                        break;
                case 1: /* new data has been displayed (dark red) */
                        if (col_shouldoverride || !((sess->tab_state & TAB_STATE_NEW_MSG)
                                                                                || (sess->tab_state & TAB_STATE_NEW_HILIGHT))) {
                                sess->tab_state = TAB_STATE_NEW_DATA;
                                chan_set_color (sess->res->tab, newdata_list);
                        }

                        if (chan_is_collapsed (sess->res->tab)
                                && !((server_sess->tab_state & TAB_STATE_NEW_MSG)
                                         || (server_sess->tab_state & TAB_STATE_NEW_HILIGHT))
                                && !(server_sess == current_tab))
                        {
                                server_sess->tab_state = TAB_STATE_NEW_DATA;
                                chan_set_color (chan_get_parent (sess->res->tab), newdata_list);
                        }

                        break;
                case 2: /* new message arrived in channel (light red) */
                        if (col_shouldoverride || !(sess->tab_state & TAB_STATE_NEW_HILIGHT)) {
                                sess->tab_state = TAB_STATE_NEW_MSG;
                                chan_set_color (sess->res->tab, newmsg_list);
                        }

                        if (chan_is_collapsed (sess->res->tab)
                                && !(server_sess->tab_state & TAB_STATE_NEW_HILIGHT)
                                && !(server_sess == current_tab))
                        {
                                server_sess->tab_state = TAB_STATE_NEW_MSG;
                                chan_set_color (chan_get_parent (sess->res->tab), newmsg_list);
                        }

                        break;
                case 3: /* your nick has been seen (blue) */
                        sess->tab_state = TAB_STATE_NEW_HILIGHT;
                        chan_set_color (sess->res->tab, nickseen_list);

                        if (chan_is_collapsed (sess->res->tab) && !(server_sess == current_tab))
                        {
                                server_sess->tab_state = TAB_STATE_NEW_MSG;
                                chan_set_color (chan_get_parent (sess->res->tab), nickseen_list);
                        }

                        break;
                }
                lastact_update (sess);
                sess->last_tab_state = sess->tab_state; /* For plugins handling future prints */
        }
}

static void
mg_set_myself_away (session_gui *gui, gboolean away)
{
        GtkWidget *label = fabulor_gtk_button_get_child (GTK_BUTTON (gui->nick_label));

        if (GTK_IS_LABEL (label))
                gtk_label_set_attributes (GTK_LABEL (label), away ? away_list : NULL);
}

/* change the little icon to the left of your nickname */

void
mg_set_access_icon (session_gui *gui, GdkPixbuf *pix, gboolean away)
{
        if (gui->op_xpm)
        {
                if (pix == fabulor_gtk_image_get_source_pixbuf (GTK_IMAGE (gui->op_xpm)))
                {
                        mg_set_myself_away (gui, away);
                        return;
                }

                fabulor_gtk_box_remove_child (GTK_BOX (gui->nick_box), gui->op_xpm);
                gui->op_xpm = NULL;
        }

        if (pix && prefs.hex_gui_input_icon)
        {
                gui->op_xpm = fabulor_gtk_image_new_from_pixbuf (pix);
                fabulor_gtk_box_insert_before_trailing (GTK_BOX (gui->nick_box),
                                                       gui->op_xpm, gui->nick_label);
                gtk_widget_show (gui->op_xpm);
        }

        mg_set_myself_away (gui, away);
}

static void
mg_inputbox_focus (GtkWidget *widget, gpointer user_data)
{
        GSList *list;
        session *sess;
        session_gui *gui = user_data;

        (void) widget;

        if (gui->is_tab)
                return;

        list = sess_list;
        while (list)
        {
                sess = list->data;
                if (sess->gui == gui)
                {
                        current_sess = sess;
                        if (!sess->server->server_session)
                                sess->server->server_session = sess;
                        break;
                }
                list = list->next;
        }
}


static gboolean
mg_client_tag_allowed (server *serv, const char *tag)
{
        char **deny;
        int i;

        if (!serv->have_message_tags)
                return FALSE;

        if (!serv->clienttagdeny || !*serv->clienttagdeny)
                return TRUE;

        deny = g_strsplit (serv->clienttagdeny, ",", 0);
        for (i = 0; deny[i]; i++)
        {
                if (!strcmp (deny[i], "*") || !strcmp (deny[i], tag) || (deny[i][0] == '+' && !strcmp (deny[i] + 1, tag)))
                {
                        g_strfreev (deny);
                        return FALSE;
                }
        }

        g_strfreev (deny);
        return TRUE;
}

static void
mg_send_typing (session *sess, const char *state)
{
        char tags[32];

        if (!sess || !sess->server->connected || !mg_client_tag_allowed (sess->server, "typing") || !sess->channel[0])
                return;

        if (sess->type != SESS_CHANNEL && sess->type != SESS_DIALOG)
                return;

        g_snprintf (tags, sizeof (tags), "+typing=%s", state);
        sess->server->p_tagmsg (sess->server, tags, sess->channel);
}

static int
mg_typing_pause_cb (session *sess)
{
        sess->typing_timeout_tag = 0;
        if (sess->typing_status == 1)
        {
                mg_send_typing (sess, "paused");
                sess->typing_status = 2;
        }
        return 0;
}

static void
mg_typing_update (session *sess, const char *text)
{
        if (!sess)
                return;

        if (sess->typing_timeout_tag)
        {
                fe_timeout_remove (sess->typing_timeout_tag);
                sess->typing_timeout_tag = 0;
        }

        if (!text || !*text || text[0] == prefs.hex_input_command_char[0])
        {
                if (sess->typing_status)
                        mg_send_typing (sess, "done");
                sess->typing_status = 0;
                return;
        }

        if (sess->typing_status != 1)
        {
                mg_send_typing (sess, "active");
                sess->typing_status = 1;
        }
        sess->typing_timeout_tag = fe_timeout_add_seconds (6, mg_typing_pause_cb, sess);
}


void
mg_reply_update (session *sess)
{
	char *nick;
	char *text;
	char *markup;

	if (!sess || !sess->gui || !sess->gui->reply_box || !sess->gui->reply_label)
		return;

	if (!sess->reply_msgid)
	{
		gtk_widget_hide (sess->gui->reply_box);
		return;
	}

	nick = g_markup_escape_text (sess->reply_nick ? sess->reply_nick : _("message"), -1);
	text = g_markup_escape_text (sess->reply_text ? sess->reply_text : _("Original message unavailable"), -1);
	markup = g_strdup_printf ("<span foreground='#7d8790'>↪ Replying to <b>%s</b> · %.160s</span>", nick, text);
	gtk_label_set_markup (GTK_LABEL (sess->gui->reply_label), markup);
	fabulor_gtk_widget_reveal_children (sess->gui->reply_box);
	gtk_widget_show (sess->gui->reply_box);
	g_free (markup);
	g_free (text);
	g_free (nick);
}

static void
mg_reply_cancel_cb (GtkWidget *wid, session *sess)
{
	reply_state_clear (sess);
	mg_reply_update (sess);
}

static void
mg_send_reply_or_text (session *sess, char *cmd)
{
	char *reply_cmd;

	if (!sess->reply_msgid || cmd[0] == prefs.hex_input_command_char[0])
	{
		handle_multiline (sess, cmd, TRUE, FALSE);
		return;
	}

	if (!sess->server->connected || !mg_client_tag_allowed (sess->server, "reply"))
	{
		PrintText (sess, _("Replies are not supported on this server. Sending normally.\n"));
		reply_state_clear (sess);
		mg_reply_update (sess);
		handle_multiline (sess, cmd, TRUE, FALSE);
		return;
	}

	reply_cmd = g_strdup_printf ("%cREPLY %s %s", prefs.hex_input_command_char[0], sess->reply_msgid, cmd);
	handle_multiline (sess, reply_cmd, TRUE, FALSE);
	g_free (reply_cmd);
	reply_state_clear (sess);
	mg_reply_update (sess);
}

static void
mg_inputbox_changed (GtkEditable *editable, session_gui *gui)
{
        key_check_replace_on_change (editable, NULL);
        if (current_sess && current_sess->gui == gui)
                mg_typing_update (current_sess, fabulor_gtk_entry_get_text (GTK_ENTRY (editable)));
}

void
mg_inputbox_cb (GtkWidget *igad, session_gui *gui)
{
        char *cmd;
        static int ignore = FALSE;
        GSList *list;
        session *sess = NULL;

        if (ignore)
                return;

        cmd = SPELL_ENTRY_GET_TEXT (igad);
        if (cmd[0] == 0)
                return;

        cmd = g_strdup (cmd);

        /* avoid recursive loop */
        ignore = TRUE;
        SPELL_ENTRY_SET_TEXT (igad, "");
        ignore = FALSE;

        /* where did this event come from? */
        if (gui->is_tab)
        {
                sess = current_tab;
        } else
        {
                list = sess_list;
                while (list)
                {
                        sess = list->data;
                        if (sess->gui == gui)
                                break;
                        list = list->next;
                }
                if (!list)
                        sess = NULL;
        }

        if (sess)
                mg_send_reply_or_text (sess, cmd);

        g_free (cmd);
}

static gboolean
mg_spellcheck_cb (SexySpellEntry *entry, gchar *word, gpointer data)
{
        /* This can cause freezes on long words, nicks arn't very long anyway. */
        if (strlen (word) > 20)
                return FALSE;

        /* Ignore anything we think is a valid url */
        if (url_check_word (word) != 0)
                return FALSE;

        return TRUE;
}

#if 0
static gboolean
has_key (char *modes)
{
        if (!modes)
                return FALSE;
        /* this is a crude check, but "-k" can't exist, so it works. */
        while (*modes)
        {
                if (*modes == 'k')
                        return TRUE;
                if (*modes == ' ')
                        return FALSE;
                modes++;
        }
        return FALSE;
}
#endif

void
fe_set_title (session *sess)
{
        char tbuf[512];
        int type;

        if (sess->gui->is_tab && sess != current_tab)
                return;

        type = sess->type;

        if (sess->server->connected == FALSE && sess->type != SESS_DIALOG)
                goto def;

        switch (type)
        {
        case SESS_DIALOG:
                g_snprintf (tbuf, sizeof (tbuf), "%s %s @ %s - %s",
                                         _("Dialog with"), sess->channel, server_get_network (sess->server, TRUE),
                                         _(DISPLAY_NAME));
                break;
        case SESS_SERVER:
                g_snprintf (tbuf, sizeof (tbuf), "%s%s%s - %s",
                                         prefs.hex_gui_win_nick ? sess->server->nick : "",
                                         prefs.hex_gui_win_nick ? " @ " : "", server_get_network (sess->server, TRUE),
                                         _(DISPLAY_NAME));
                break;
        case SESS_CHANNEL:
                /* don't display keys in the titlebar */
                        g_snprintf (tbuf, sizeof (tbuf),
                                         "%s%s%s / %s%s%s%s - %s",
                                         prefs.hex_gui_win_nick ? sess->server->nick : "",
                                         prefs.hex_gui_win_nick ? " @ " : "",
                                         server_get_network (sess->server, TRUE), sess->channel,
                                         prefs.hex_gui_win_modes && sess->current_modes ? " (" : "",
                                         prefs.hex_gui_win_modes && sess->current_modes ? sess->current_modes : "",
                                         prefs.hex_gui_win_modes && sess->current_modes ? ")" : "",
                                         _(DISPLAY_NAME));
                if (prefs.hex_gui_win_ucount)
                {
                        g_snprintf (tbuf + strlen (tbuf), 9, " (%d)", sess->total);
                }
                break;
        case SESS_NOTICES:
        case SESS_SNOTICES:
                g_snprintf (tbuf, sizeof (tbuf), "%s%s%s (notices) - %s",
                                         prefs.hex_gui_win_nick ? sess->server->nick : "",
                                         prefs.hex_gui_win_nick ? " @ " : "", server_get_network (sess->server, TRUE),
                                         _(DISPLAY_NAME));
                break;
        default:
        def:
                g_snprintf (tbuf, sizeof (tbuf), _(DISPLAY_NAME));
                gtk_window_set_title (GTK_WINDOW (sess->gui->window), tbuf);
                return;
        }

        gtk_window_set_title (GTK_WINDOW (sess->gui->window), tbuf);
}

static void
mg_topicbar_update_height (GtkWidget *topic);
static void
mg_topicbar_queue_relayout (GtkWidget *topic);
static void
mg_queue_window_relayout (GtkWidget *window);

static session *
mg_session_from_window (GtkWidget *wid)
{
        GSList *list;
        session *sess;

        list = sess_list;
        while (list)
        {
                sess = list->data;
                if (sess && sess->gui && sess->gui->window == wid)
                        return sess;
                list = list->next;
        }

        return current_sess;
}

static gboolean
mg_window_relayout_idle_cb (gpointer userdata)
{
        GtkWidget *window = GTK_WIDGET (userdata);
        session *sess;

        g_object_set_data (G_OBJECT (window), "mg-window-relayout-source", NULL);

        sess = mg_session_from_window (window);
        if (sess && sess->gui)
        {
                if (GTK_IS_WIDGET (sess->gui->topic_entry))
                        mg_topicbar_queue_relayout (sess->gui->topic_entry);

                if (GTK_IS_XTEXT (sess->gui->xtext))
                {
                        gtk_xtext_refresh (GTK_XTEXT (sess->gui->xtext));
                        gtk_widget_queue_resize (sess->gui->xtext);
                        gtk_widget_queue_draw (sess->gui->xtext);
                }

                if (GTK_IS_WIDGET (sess->gui->window))
                {
                        gtk_widget_queue_resize (sess->gui->window);
                        gtk_widget_queue_draw (sess->gui->window);
                }
        }

        g_object_unref (window);
        return G_SOURCE_REMOVE;
}

static void
mg_queue_window_relayout (GtkWidget *window)
{
        guint source_id;

        if (!window || !GTK_IS_WIDGET (window))
                return;

        if (g_object_get_data (G_OBJECT (window), "mg-window-relayout-source") != NULL)
                return;

        source_id = g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
                                     mg_window_relayout_idle_cb,
                                     g_object_ref (window),
                                     NULL);
        g_object_set_data (G_OBJECT (window), "mg-window-relayout-source",
                           GUINT_TO_POINTER (source_id));
}

static void
mg_windowstate_cb (GtkWindow *wid, const FabulorWindowState *state,
	gpointer userdata)
{
	guint win_state;
	guint win_fullscreen;
	gboolean changed = FALSE;
        session *sess;

	if ((state->changed & FABULOR_WINDOW_STATE_MINIMIZED) && state->minimized &&
	 prefs.hex_gui_tray_minimize && prefs.hex_gui_tray &&
	 gtkutil_tray_icon_supported (wid))
	{
		tray_toggle_visibility (TRUE);
	}

	win_state = state->maximized ? 1 : 0;
	win_fullscreen = state->fullscreen ? 1 : 0;

	if (prefs.hex_gui_win_state != win_state)
	{
		prefs.hex_gui_win_state = win_state;
		changed = TRUE;
	}

	if (prefs.hex_gui_win_fullscreen != win_fullscreen)
	{
		prefs.hex_gui_win_fullscreen = win_fullscreen;
		changed = TRUE;
	}

	if (changed)
	{
		mg_config_prefs_dirty = TRUE;
		mg_schedule_config_save ();
	}

        sess = mg_session_from_window (GTK_WIDGET (wid));
        if (sess && sess->gui && GTK_IS_WIDGET (sess->gui->window))
                mg_queue_window_relayout (sess->gui->window);
        else
                mg_queue_window_relayout (GTK_WIDGET (wid));

        if (current_sess && current_sess->gui)
                menu_set_fullscreen (current_sess->gui, prefs.hex_gui_win_fullscreen);

#ifdef G_OS_WIN32
	if (state->changed &
		(FABULOR_WINDOW_STATE_MAXIMIZED | FABULOR_WINDOW_STATE_FULLSCREEN))
		fabulor_window_state_allow_autohide_taskbar (wid, state);
#endif
}

static void
mg_geometry_cb (GtkWindow *wid, const FabulorWindowGeometry *geometry,
	gpointer user_data)
{
        gboolean changed = FALSE;
	session *sess = user_data;
        session *target_sess;

        if (sess == NULL)
        {
                if (mg_gui)
                {
                        if (prefs.hex_gui_win_save && !prefs.hex_gui_win_state && !prefs.hex_gui_win_fullscreen)
                        {
                                if (geometry->has_position &&
					prefs.hex_gui_win_left != geometry->x)
                                {
                                        prefs.hex_gui_win_left = geometry->x;
                                        changed = TRUE;
                                }

                                if (geometry->has_position &&
					prefs.hex_gui_win_top != geometry->y)
                                {
                                        prefs.hex_gui_win_top = geometry->y;
                                        changed = TRUE;
                                }

                                if (geometry->width > 0 &&
					prefs.hex_gui_win_width != geometry->width)
                                {
                                        prefs.hex_gui_win_width = geometry->width;
                                        changed = TRUE;
                                }

                                if (geometry->height > 0 &&
					prefs.hex_gui_win_height != geometry->height)
                                {
                                        prefs.hex_gui_win_height = geometry->height;
                                        changed = TRUE;
                                }
                        }
                }
        }
        else if (sess->type == SESS_DIALOG && prefs.hex_gui_win_save)
        {
                if (geometry->has_position &&
			prefs.hex_gui_dialog_left != geometry->x)
                {
                        prefs.hex_gui_dialog_left = geometry->x;
                        changed = TRUE;
                }

                if (geometry->has_position &&
			prefs.hex_gui_dialog_top != geometry->y)
                {
                        prefs.hex_gui_dialog_top = geometry->y;
                        changed = TRUE;
                }

                if (geometry->width > 0 &&
			prefs.hex_gui_dialog_width != geometry->width)
                {
                        prefs.hex_gui_dialog_width = geometry->width;
                        changed = TRUE;
                }

                if (geometry->height > 0 &&
			prefs.hex_gui_dialog_height != geometry->height)
                {
                        prefs.hex_gui_dialog_height = geometry->height;
                        changed = TRUE;
                }
        }

        if (changed)
        {
                mg_config_prefs_dirty = TRUE;
                mg_schedule_config_save ();
        }

	target_sess = mg_session_from_window (GTK_WIDGET (wid));
        if (target_sess && target_sess->gui)
                mg_schedule_rightpane_restore (target_sess->gui);
        if (target_sess && target_sess->gui && GTK_IS_WIDGET (target_sess->gui->window))
                mg_queue_window_relayout (target_sess->gui->window);
        else
		mg_queue_window_relayout (GTK_WIDGET (wid));
}

/* move to a non-irc tab */

static void
mg_show_generic_tab (GtkWidget *box)
{
        int num;
        GtkWidget *f = NULL;

        if (current_sess && gtk_widget_has_focus (current_sess->gui->input_box))
                f = current_sess->gui->input_box;

        num = gtk_notebook_page_num (GTK_NOTEBOOK (mg_gui->note_book), box);
        gtk_notebook_set_current_page (GTK_NOTEBOOK (mg_gui->note_book), num);
        fabulor_user_list_view_set_model (mg_gui->user_tree, NULL);
        gtk_window_set_title (GTK_WINDOW (mg_gui->window),
                                                                 g_object_get_data (G_OBJECT (box), "title"));
        gtk_widget_set_sensitive (mg_gui->menu, FALSE);

        if (f)
                gtk_widget_grab_focus (f);
}

/* a channel has been focused */

static void
mg_focus (session *sess)
{
        if (sess->gui->is_tab)
                current_tab = sess;
        current_sess = sess;

        /* dirty trick to avoid auto-selection */
        SPELL_ENTRY_SET_EDITABLE (sess->gui->input_box, FALSE);
        gtk_widget_grab_focus (sess->gui->input_box);
        SPELL_ENTRY_SET_EDITABLE (sess->gui->input_box, TRUE);

        sess->server->front_session = sess;

        if (sess->server->server_session != NULL)
        {
                if (sess->server->server_session->type != SESS_SERVER)
                        sess->server->server_session = sess;
        } else
        {
                sess->server->server_session = sess;
        }

        /* when called via mg_changui_new, is_tab might be true, but
                sess->res->tab is still NULL. */
        if (sess->res->tab)
                fe_set_tab_color (sess, FE_COLOR_NONE);
}

static int
mg_progressbar_update (GtkWidget *bar)
{
        static int type = 0;
        static gdouble pos = 0;

        pos += 0.05;
        if (pos >= 0.99)
        {
                if (type == 0)
                {
                        type = 1;
                        gtk_progress_bar_set_inverted (GTK_PROGRESS_BAR (bar), TRUE);
                } else
                {
                        type = 0;
                        gtk_progress_bar_set_inverted (GTK_PROGRESS_BAR (bar), FALSE);
                }
                pos = 0.05;
        }
        gtk_progress_bar_set_fraction ((GtkProgressBar *) bar, pos);
        return 1;
}

void
mg_progressbar_create (session_gui *gui)
{
        gui->bar = gtk_progress_bar_new ();
        fabulor_gtk_box_insert_before_trailing (GTK_BOX (gui->nick_box), gui->bar,
                                               gui->nick_label);
        gtk_widget_show (gui->bar);
        gui->bartag = fe_timeout_add (50, mg_progressbar_update, gui->bar);
}

void
mg_progressbar_destroy (session_gui *gui)
{
        fe_timeout_remove (gui->bartag);
        fabulor_gtk_box_remove_child (GTK_BOX (gui->nick_box), gui->bar);
        gui->bar = 0;
        gui->bartag = 0;
}

/* switching tabs away from this one, so remember some info about it! */

static void
mg_unpopulate (session *sess)
{
        restore_gui *res;
        session_gui *gui;
        GtkTextBuffer *topic_buffer;
        GtkTextIter start;
        GtkTextIter end;
        int i;

        gui = sess->gui;
        res = sess->res;

        res->input_text = g_strdup (SPELL_ENTRY_GET_TEXT (gui->input_box));
        topic_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (gui->topic_entry));
        gtk_text_buffer_get_bounds (topic_buffer, &start, &end);
        res->topic_text = gtk_text_buffer_get_text (topic_buffer, &start, &end, FALSE);
        res->limit_text = g_strdup (fabulor_gtk_entry_get_text (GTK_ENTRY (gui->limit_entry)));
        res->key_text = g_strdup (fabulor_gtk_entry_get_text (GTK_ENTRY (gui->key_entry)));
        if (gui->laginfo)
                res->lag_text = g_strdup (gtk_label_get_text (GTK_LABEL (gui->laginfo)));
        if (gui->throttleinfo)
                res->queue_text = g_strdup (gtk_label_get_text (GTK_LABEL (gui->throttleinfo)));

        for (i = 0; i < NUM_FLAG_WIDS - 1; i++)
                res->flag_wid_state[i] = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (gui->flag_wid[i]));

        res->old_ul_value = userlist_get_value (gui->user_tree);
        if (gui->lagometer)
                res->lag_value = gtk_progress_bar_get_fraction (
                                                                                                        GTK_PROGRESS_BAR (gui->lagometer));
        if (gui->throttlemeter)
                res->queue_value = gtk_progress_bar_get_fraction (
                                                                                                        GTK_PROGRESS_BAR (gui->throttlemeter));

        if (gui->bar)
        {
                res->c_graph = TRUE;    /* still have a graph, just not visible now */
                mg_progressbar_destroy (gui);
        }
}

static void
mg_restore_label (GtkWidget *label, char **text)
{
        if (!label)
                return;

        if (*text)
        {
                gtk_label_set_text (GTK_LABEL (label), *text);
                g_free (*text);
                *text = NULL;
        } else
        {
                gtk_label_set_text (GTK_LABEL (label), "");
        }
}

static void
mg_restore_entry (GtkWidget *entry, char **text)
{
        if (*text)
        {
                fabulor_gtk_entry_set_text (GTK_ENTRY (entry), *text);
                g_free (*text);
                *text = NULL;
        } else
        {
                fabulor_gtk_entry_set_text (GTK_ENTRY (entry), "");
        }
        gtk_editable_set_position (GTK_EDITABLE (entry), -1);
}

static void
mg_restore_speller (GtkWidget *entry, char **text)
{
        if (*text)
        {
                SPELL_ENTRY_SET_TEXT (entry, *text);
                g_free (*text);
                *text = NULL;
        } else
        {
                SPELL_ENTRY_SET_TEXT (entry, "");
        }
        SPELL_ENTRY_SET_POS (entry, -1);
}

void
mg_set_topic_tip (session *sess)
{
        char *text;
        GtkTextBuffer *topic_buffer;
        GtkTextIter start;
        GtkTextIter end;

        switch (sess->type)
        {
        case SESS_CHANNEL:
                if (sess->topic)
                {
                        text = g_strdup_printf (_("Topic for %s is: %s"), sess->channel,
                                                 sess->topic);
                        gtk_widget_set_tooltip_text (sess->gui->topic_entry, text);
                        g_free (text);
                } else
                        gtk_widget_set_tooltip_text (sess->gui->topic_entry, _("No topic is set"));
                break;
        default:
                topic_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (sess->gui->topic_entry));
                gtk_text_buffer_get_bounds (topic_buffer, &start, &end);
                text = gtk_text_buffer_get_text (topic_buffer, &start, &end, FALSE);
                if (text[0])
                        gtk_widget_set_tooltip_text (sess->gui->topic_entry, text);
                else
                        gtk_widget_set_tooltip_text (sess->gui->topic_entry, NULL);
                g_free (text);
        }
}

static void
mg_hide_empty_pane (GtkPaned *pane)
{
        GtkWidget *start_child = fabulor_gtk_paned_get_start_child (pane);
        GtkWidget *end_child = fabulor_gtk_paned_get_end_child (pane);

        if ((start_child == NULL || !gtk_widget_get_visible (start_child)) &&
                (end_child == NULL || !gtk_widget_get_visible (end_child)))
        {
                gtk_widget_hide (GTK_WIDGET (pane));
                return;
        }

        gtk_widget_show (GTK_WIDGET (pane));
}

static void
mg_hide_empty_boxes (session_gui *gui)
{
        /* hide empty vpanes - so the handle is not shown */
        mg_hide_empty_pane ((GtkPaned*)gui->vpane_right);
        mg_hide_empty_pane ((GtkPaned*)gui->vpane_left);
}

static void
mg_userlist_showhide (session *sess, int show)
{
        session_gui *gui = sess->gui;
        int handle_size;
        int right_size;
        int min_right_size;
        int pane_width;

        gtk_widget_get_size_request (gui->user_box, &min_right_size, NULL);
        if (min_right_size < 1)
                min_right_size = 1;

        right_size = MAX (prefs.hex_gui_pane_right_size, min_right_size);

        if (show)
        {
                gtk_widget_show (gui->user_box);
                gui->ul_hidden = 0;

                pane_width = fabulor_gtk_widget_get_allocated_width (
                        gui->hpane_right);
                handle_size = fabulor_gtk_paned_get_handle_size (
                        GTK_PANED (gui->hpane_right));
                gtk_paned_set_position (GTK_PANED (gui->hpane_right),
                        pane_width - (right_size + handle_size));
        }
        else
        {
                gtk_widget_hide (gui->user_box);
                gui->ul_hidden = 1;
        }

        mg_hide_empty_boxes (gui);
}

static gboolean
mg_is_userlist_and_tree_combined (void)
{
        if (prefs.hex_gui_tab_pos == POS_TOPLEFT && prefs.hex_gui_ulist_pos == POS_BOTTOMLEFT)
                return TRUE;
        if (prefs.hex_gui_tab_pos == POS_BOTTOMLEFT && prefs.hex_gui_ulist_pos == POS_TOPLEFT)
                return TRUE;

        if (prefs.hex_gui_tab_pos == POS_TOPRIGHT && prefs.hex_gui_ulist_pos == POS_BOTTOMRIGHT)
                return TRUE;
        if (prefs.hex_gui_tab_pos == POS_BOTTOMRIGHT && prefs.hex_gui_ulist_pos == POS_TOPRIGHT)
                return TRUE;

        return FALSE;
}

/* decide if the userlist should be shown or hidden for this tab */

void
mg_decide_userlist (session *sess, gboolean switch_to_current)
{
        /* when called from menu.c we need this */
        if (sess->gui == mg_gui && switch_to_current)
                sess = current_tab;

        if (prefs.hex_gui_ulist_hide)
        {
                mg_userlist_showhide (sess, FALSE);
                return;
        }

        switch (sess->type)
        {
        case SESS_SERVER:
        case SESS_DIALOG:
        case SESS_NOTICES:
        case SESS_SNOTICES:
                if (mg_is_userlist_and_tree_combined ())
                        mg_userlist_showhide (sess, TRUE);      /* show */
                else
                        mg_userlist_showhide (sess, FALSE);     /* hide */
                break;
        default:
                mg_userlist_showhide (sess, TRUE);      /* show */
        }
}

static gboolean
mg_populate_userlist (session *sess)
{
        gint64 started = fabulor_ui_profile_enabled () ?
                g_get_monotonic_time () : 0;
        gint64 model_attached = 0;

        if (!sess)
                sess = current_tab;

        if (is_session (sess))
        {
                if (sess->type == SESS_DIALOG)
                        mg_set_access_icon (sess->gui, NULL, sess->server->is_away);
                else
                        mg_set_access_icon (sess->gui, get_user_icon (sess->server, sess->me), sess->server->is_away);
                userlist_show (sess);
                if (started)
                        model_attached = g_get_monotonic_time ();
                userlist_set_value (sess->gui->user_tree, sess->res->old_ul_value);
        }
        if (started)
                fabulor_ui_profile_log ("user-list",
                                       "total_us=%" G_GINT64_FORMAT
                                       " model_us=%" G_GINT64_FORMAT
                                       " scroll_us=%" G_GINT64_FORMAT
                                       " channel=\"%s\"",
                                       g_get_monotonic_time () - started,
                                       model_attached ? model_attached - started : 0,
                                       model_attached ?
                                               g_get_monotonic_time () - model_attached : 0,
                                       sess ? sess->channel : "");

        return 0;
}

/* fill the irc tab with a new channel */

static void
mg_populate (session *sess)
{
        session_gui *gui = sess->gui;
        restore_gui *res = sess->res;
        int i, render = TRUE;
        guint16 vis = gui->ul_hidden;
        GtkAllocation allocation;

        switch (sess->type)
        {
        case SESS_DIALOG:
                /* show the dialog buttons */
                gtk_widget_show (gui->dialogbutton_box);
                /* hide the chan-mode buttons */
                gtk_widget_hide (gui->topicbutton_box);
                /* hide the userlist */
                mg_decide_userlist (sess, FALSE);
                /* shouldn't edit the topic */
                gtk_text_view_set_editable (GTK_TEXT_VIEW (gui->topic_entry), FALSE);
                /* might be hidden from server tab */
                if (prefs.hex_gui_topicbar)
                        gtk_widget_show (gui->topic_bar);
                break;
        case SESS_SERVER:
                if (prefs.hex_gui_mode_buttons)
                        gtk_widget_show (gui->topicbutton_box);
                /* hide the dialog buttons */
                gtk_widget_hide (gui->dialogbutton_box);
                /* hide the userlist */
                mg_decide_userlist (sess, FALSE);
                /* servers don't have topics */
                gtk_widget_hide (gui->topic_bar);
                break;
        default:
                /* hide the dialog buttons */
                gtk_widget_hide (gui->dialogbutton_box);
                if (prefs.hex_gui_mode_buttons)
                        gtk_widget_show (gui->topicbutton_box);
                /* show the userlist */
                mg_decide_userlist (sess, FALSE);
                /* let the topic be edited */
                gtk_text_view_set_editable (GTK_TEXT_VIEW (gui->topic_entry), TRUE);
                if (prefs.hex_gui_topicbar)
                        gtk_widget_show (gui->topic_bar);
        }

	gtk_widget_set_visible (gui->nick_box,
		prefs.hex_gui_input_nick && sess->type != SESS_SERVER);

        /* move to THE irc tab */
        if (gui->is_tab)
                gtk_notebook_set_current_page (GTK_NOTEBOOK (gui->note_book), 0);

        /* xtext size change? Then don't render, wait for the expose caused
      by showing/hidding the userlist */
        gtk_widget_get_allocation (gui->user_box, &allocation);
        if (vis != gui->ul_hidden && allocation.width > 1)
                render = FALSE;

        gtk_xtext_buffer_show (GTK_XTEXT (gui->xtext), res->buffer, render);

        if (gui->is_tab)
                gtk_widget_set_sensitive (gui->menu, TRUE);

        if (res->topic_text)
        {
                GtkTextBuffer *topic_buffer;

                topic_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (gui->topic_entry));
                gtk_text_buffer_set_text (topic_buffer, res->topic_text, -1);
                g_free (res->topic_text);
                res->topic_text = NULL;
        } else
        {
                GtkTextBuffer *topic_buffer;

                topic_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (gui->topic_entry));
                gtk_text_buffer_set_text (topic_buffer, "", -1);
        }
        mg_restore_speller (gui->input_box, &res->input_text);
        mg_restore_entry (gui->key_entry, &res->key_text);
        mg_restore_entry (gui->limit_entry, &res->limit_text);
        mg_restore_label (gui->laginfo, &res->lag_text);
        mg_restore_label (gui->throttleinfo, &res->queue_text);

        mg_focus (sess);
        fe_set_title (sess);

        /* this one flickers, so only change if necessary */
        if (strcmp (sess->server->nick, gtk_button_get_label (GTK_BUTTON (gui->nick_label))) != 0)
                gtk_button_set_label (GTK_BUTTON (gui->nick_label), sess->server->nick);

        /*
         * Keep transcript and user-list replacement in one switch transaction.
         * Deferring this model swap makes the list visibly trail the transcript.
         */
        mg_populate_userlist (sess);

        fe_userlist_numbers (sess);

        /* restore all the channel mode buttons */
        ignore_chanmode = TRUE;
        for (i = 0; i < NUM_FLAG_WIDS - 1; i++)
        {
                /* Hide if mode not supported */
                if (sess->server && strchr (sess->server->chanmodes, chan_flags[i]) == NULL)
                        gtk_widget_hide (sess->gui->flag_wid[i]);
                else
                        gtk_widget_show (sess->gui->flag_wid[i]);

                /* Update state */
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gui->flag_wid[i]),
                                                                        res->flag_wid_state[i]);
        }
        ignore_chanmode = FALSE;

        if (gui->lagometer)
        {
                gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (gui->lagometer),
                                                                                                 res->lag_value);
                if (res->lag_tip)
                        gtk_widget_set_tooltip_text (gtk_widget_get_parent (sess->gui->lagometer), res->lag_tip);
        }
        if (gui->throttlemeter)
        {
                gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (gui->throttlemeter),
                                                                                                 res->queue_value);
                if (res->queue_tip)
                        gtk_widget_set_tooltip_text (gtk_widget_get_parent (sess->gui->throttlemeter), res->queue_tip);
        }

        /* did this tab have a connecting graph? restore it.. */
        if (res->c_graph)
        {
                res->c_graph = FALSE;
                mg_progressbar_create (gui);
        }

        /* menu items */
        menu_set_away (gui, sess->server->is_away);
        menu_set_away_sensitive (gui, sess->server->connected);
        menu_set_join_sensitive (gui, sess->server->end_of_motd);
        menu_set_disconnect_sensitive (gui,
                                                                          sess->server->connected || sess->server->recondelay_tag);

        mg_set_topic_tip (sess);

        plugin_emit_dummy_print (sess, "Focus Tab");
}

void
mg_bring_tofront_sess (session *sess)   /* IRC tab or window */
{
        if (sess->gui->is_tab)
                chan_focus (sess->res->tab);
        else
                gtk_window_present (GTK_WINDOW (sess->gui->window));
}

void
mg_bring_tofront (GtkWidget *vbox)      /* non-IRC tab or window */
{
        chan *ch;
        GtkWindow *window;

        ch = g_object_get_data (G_OBJECT (vbox), "ch");
        if (ch)
                chan_focus (ch);
        else
        {
                window = fabulor_gtk_widget_get_root_window (vbox);
                if (window)
                        gtk_window_present (window);
        }
}

void
mg_switch_page (int relative, int num)
{
        if (mg_gui)
                chanview_move_focus (mg_gui->chanview, relative, num);
}

/* cleanup an IRC tab */

static void
mg_ircdestroy (session *sess)
{
        GSList *list;

        session_free (sess);    /* tell zoitechat.c about it */

        if (mg_gui == NULL)
        {
                return;
        }

        list = sess_list;
        while (list)
        {
                sess = list->data;
                if (sess->gui->is_tab)
                {
                        return;
                }
                list = list->next;
        }

        fabulor_gtk_window_destroy (GTK_WINDOW (mg_gui->window));
        active_tab = NULL;
        mg_gui = NULL;
        parent_window = NULL;
}

static void
mg_tab_close_cb (GtkWidget *dialog, gint arg1, session *sess)
{
        GSList *list, *next;

        fabulor_gtk_window_destroy (GTK_WINDOW (dialog));
        if (arg1 == GTK_RESPONSE_OK && is_session (sess))
        {
                /* force it NOT to send individual PARTs */
                sess->server->sent_quit = TRUE;

                for (list = sess_list; list;)
                {
                        next = list->next;
                        if (((session *)list->data)->server == sess->server &&
                                 ((session *)list->data) != sess)
                                fe_close_window ((session *)list->data);
                        list = next;
                }

                /* just send one QUIT - better for BNCs */
                sess->server->sent_quit = FALSE;
                fe_close_window (sess);
        }
}

static void
mg_closed_channel_tabs_add (session *sess)
{
	mg_closed_channel_tab *item;
	GSList *last;

	if (!sess || sess->type != SESS_CHANNEL || !sess->channel[0])
		return;

	item = g_new0 (mg_closed_channel_tab, 1);
	item->server_id = sess->server->id;
	g_strlcpy (item->channel, sess->channel, sizeof (item->channel));
	g_strlcpy (item->key, sess->channelkey, sizeof (item->key));
	mg_closed_channel_tabs = g_slist_prepend (mg_closed_channel_tabs, item);
	if (g_slist_length (mg_closed_channel_tabs) > 20)
	{
		last = g_slist_last (mg_closed_channel_tabs);
		g_free (last->data);
		mg_closed_channel_tabs = g_slist_delete_link (mg_closed_channel_tabs, last);
	}
}

void
mg_reopen_closed_channel_tab (void)
{
	mg_closed_channel_tab *item;
	GSList *head;
	GSList *list;
	server *serv;
	session *sess;

	if (!mg_closed_channel_tabs)
		return;

	head = mg_closed_channel_tabs;
	item = head->data;
	mg_closed_channel_tabs = g_slist_delete_link (mg_closed_channel_tabs, head);
	if (!item)
		return;

	serv = NULL;
	for (list = serv_list; list; list = list->next)
	{
		server *candidate = list->data;
		if (candidate->id == item->server_id)
		{
			serv = candidate;
			break;
		}
	}
	if (serv && serv->connected && item->channel[0])
	{
		sess = find_channel (serv, item->channel);
		if (sess)
			fe_ctrl_gui (sess, 2, 0);
		else
		{
			new_ircwindow (serv, item->channel, SESS_CHANNEL, 1);
			serv->p_join (serv, item->channel, item->key);
		}
	}

	g_free (item);
}

void
mg_tab_close (session *sess)
{
        GtkWidget *dialog;
        GSList *list;
        int i;

	if (chan_remove (sess->res->tab, FALSE))
	{
		mg_closed_channel_tabs_add (sess);
		sess->res->tab = NULL;
		mg_ircdestroy (sess);
	}
        else
        {
                for (i = 0, list = sess_list; list; list = list->next)
                {
                        session *s = (session*)list->data;
                        if (s->server == sess->server && (s->type == SESS_CHANNEL || s->type == SESS_DIALOG))
                                i++;
                }
                dialog = gtk_message_dialog_new (GTK_WINDOW (parent_window), 0,
                                                GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL,
                                                _("This server still has %d channels or dialogs associated with it. "
                                                  "Close them all?"), i);
	theme_manager_attach_window (dialog);
                g_signal_connect (G_OBJECT (dialog), "response",
                                                                G_CALLBACK (mg_tab_close_cb), sess);
                if (prefs.hex_gui_tab_layout)
                {
                        fabulor_gtk_window_position_at_pointer (GTK_WINDOW (dialog));
                }
                else
                {
                        fabulor_gtk_window_position_center_on_parent (GTK_WINDOW (dialog));
                }
                gtk_widget_show (dialog);
        }
}


static int
mg_count_networks (void)
{
        int cons = 0;
        GSList *list;

        for (list = serv_list; list; list = list->next)
        {
                if (((server *)list->data)->connected)
                        cons++;
        }
        return cons;
}

static int
mg_count_dccs (void)
{
        GSList *list;
        struct DCC *dcc;
        int dccs = 0;

        list = dcc_list;
        while (list)
        {
                dcc = list->data;
                if ((dcc->type == TYPE_SEND || dcc->type == TYPE_RECV) &&
                         dcc->dccstat == STAT_ACTIVE)
                        dccs++;
                list = list->next;
        }

        return dccs;
}


static void
mg_quit_dialog_response (GtkDialog *dialog, gint response_id,
                         gpointer user_data)
{
        GtkToggleButton *checkbutton = GTK_TOGGLE_BUTTON (user_data);
        gboolean dont_ask = gtk_toggle_button_get_active (checkbutton);
        gboolean should_quit = response_id == 0;
        gboolean should_minimize = response_id == 1;

        if (should_quit && dont_ask)
                prefs.hex_gui_quit_dialog = 0;
        else if (should_minimize && dont_ask)
                prefs.hex_gui_tray_close = 1;

        fabulor_gtk_window_destroy (GTK_WINDOW (dialog));

        if (should_quit)
        {
                zoitechat_exit ();
                return;
        }

        if (!should_minimize)
                return;

        if (!prefs.hex_gui_tray)
        {
                prefs.hex_gui_tray = 1;
                tray_apply_setup ();
        }
        tray_toggle_visibility (TRUE);
}

void
mg_open_quit_dialog (gboolean minimize_button)
{
	GtkWidget *dialog;
	GtkWidget *dialog_vbox1;
	GtkWidget *table1;
	GtkWidget *image;
	GtkWidget *checkbutton1;
	GtkWidget *label;
	GtkWidget *button;
	char *text, *connecttext;
	int cons;
	int dccs;

        if (quit_dialog)
        {
                gtk_window_present (GTK_WINDOW (quit_dialog));
                return;
        }

        dccs = mg_count_dccs ();
        cons = mg_count_networks ();
        if (dccs + cons == 0 || !prefs.hex_gui_quit_dialog)
        {
                zoitechat_exit ();
                return;
        }

        dialog = gtk_dialog_new ();
	quit_dialog = dialog;
	theme_manager_attach_window (dialog);
        fabulor_gtk_container_set_uniform_inset (dialog, 6);
        gtk_window_set_title (GTK_WINDOW (dialog), _("Quit " DISPLAY_NAME "?"));
        gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent_window));
        gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

        dialog_vbox1 = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
        gtk_widget_show (dialog_vbox1);

        table1 = gtk_grid_new ();
        gtk_widget_show (table1);
        fabulor_gtk_box_append (GTK_BOX (dialog_vbox1), table1, TRUE, TRUE, 0);
        fabulor_gtk_container_set_uniform_inset (table1, 6);
        gtk_grid_set_row_spacing (GTK_GRID (table1), 12);
        gtk_grid_set_column_spacing (GTK_GRID (table1), 12);

        image = fabulor_gtk_dialog_icon_new ("dialog-warning");
        gtk_widget_show (image);
        gtk_widget_set_hexpand (image, FALSE);
        gtk_widget_set_vexpand (image, FALSE);
        gtk_widget_set_halign (image, GTK_ALIGN_FILL);
        gtk_widget_set_valign (image, GTK_ALIGN_FILL);
        gtk_grid_attach (GTK_GRID (table1), image, 0, 0, 1, 1);

        checkbutton1 = gtk_check_button_new_with_mnemonic (_("Don't ask next time."));
        gtk_widget_show (checkbutton1);
        gtk_widget_set_hexpand (checkbutton1, TRUE);
        gtk_widget_set_vexpand (checkbutton1, FALSE);
        gtk_widget_set_halign (checkbutton1, GTK_ALIGN_FILL);
        gtk_widget_set_valign (checkbutton1, GTK_ALIGN_CENTER);
        gtk_widget_set_margin_top (checkbutton1, 4);
        gtk_widget_set_margin_bottom (checkbutton1, 4);
        gtk_grid_attach (GTK_GRID (table1), checkbutton1, 0, 1, 2, 1);

        connecttext = g_strdup_printf (_("You are connected to %i IRC networks."), cons);
        text = g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>\n\n%s\n%s",
                                                                _("Are you sure you want to quit?"),
                                                                cons ? connecttext : "",
                                                                dccs ? _("Some file transfers are still active.") : "");
        g_free (connecttext);
        label = gtk_label_new (text);
        g_free (text);
        gtk_widget_show (label);
        mg_set_label_alignment_start (label);
        gtk_widget_set_hexpand (label, TRUE);
        gtk_widget_set_vexpand (label, TRUE);
        gtk_widget_set_halign (label, GTK_ALIGN_FILL);
        gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
        gtk_label_set_xalign (GTK_LABEL (label), 0.0f);
        gtk_grid_attach (GTK_GRID (table1), label, 1, 0, 1, 1);
        gtk_label_set_use_markup (GTK_LABEL (label), TRUE);


	if (minimize_button && gtkutil_tray_icon_supported (GTK_WINDOW(dialog)))
	{
		button = gtk_button_new_with_mnemonic (_("_Minimize to Tray"));
		gtk_widget_show (button);
                gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, 1);
        }

        button = gtk_button_new_with_mnemonic (_("_Cancel"));
        gtk_widget_show (button);
        gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button,
                                                                                        GTK_RESPONSE_CANCEL);
        gtk_widget_grab_focus (button);

        button = gtk_button_new_with_mnemonic (_("_Quit"));
        gtk_widget_show (button);
        gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, 0);

        g_signal_connect (G_OBJECT (dialog), "response",
                          G_CALLBACK (mg_quit_dialog_response), checkbutton1);
        g_object_add_weak_pointer (G_OBJECT (dialog), (gpointer *) &quit_dialog);
        gtk_widget_show (dialog);
}

void
mg_close_sess (session *sess)
{
        if (sess_list->next == NULL)
        {
                mg_open_quit_dialog (FALSE);
                return;
        }

        fe_close_window (sess);
}

static int
mg_chan_remove (chan *ch)
{
        /* remove the tab from chanview */
        chan_remove (ch, TRUE);
        /* any tabs left? */
        if (chanview_get_size (mg_gui->chanview) < 1)
        {
                /* if not, destroy the main tab window */
                fabulor_gtk_window_destroy (GTK_WINDOW (mg_gui->window));
                current_tab = NULL;
                active_tab = NULL;
                mg_gui = NULL;
                parent_window = NULL;
                return TRUE;
        }
        return FALSE;
}

/* destroy non-irc tab/window */

static void
mg_close_gen (chan *ch, GtkWidget *box)
{
        GtkWindow *window;

        if (!ch)
                ch = g_object_get_data (G_OBJECT (box), "ch");
        if (ch)
        {
                /* remove from notebook */
                int page = gtk_notebook_page_num (
                        GTK_NOTEBOOK (mg_gui->note_book), box);
                if (page >= 0)
                        gtk_notebook_remove_page (
                                GTK_NOTEBOOK (mg_gui->note_book), page);
                /* remove the tab from chanview */
                mg_chan_remove (ch);
        } else
        {
                window = fabulor_gtk_widget_get_root_window (box);
                if (window)
                        fabulor_gtk_window_destroy (window);
        }
}

/* the "X" close button has been pressed (tab-view) */

static void
mg_xbutton_cb (chanview *cv, chan *ch, int tag, gpointer userdata)
{
        if (tag == TAG_IRC)     /* irc tab */
                mg_close_sess (userdata);
        else                                            /* non-irc utility tab */
                mg_close_gen (ch, userdata);
}

static void
mg_link_gentab (chan *ch, GtkWidget *box)
{
        int num;
        GtkWidget *win;

        g_object_ref (box);

        num = gtk_notebook_page_num (GTK_NOTEBOOK (mg_gui->note_book), box);
        gtk_notebook_remove_page (GTK_NOTEBOOK (mg_gui->note_book), num);
        mg_chan_remove (ch);

        win = gtkutil_window_new (g_object_get_data (G_OBJECT (box), "title"), "",
                                                                          GPOINTER_TO_INT (g_object_get_data (G_OBJECT (box), "w")),
                                                                          GPOINTER_TO_INT (g_object_get_data (G_OBJECT (box), "h")),
                                                                          2);
        /* so it doesn't try to chan_remove (there's no tab anymore) */
        g_object_steal_data (G_OBJECT (box), "ch");
        fabulor_gtk_container_set_uniform_inset (box, 0);
        fabulor_gtk_window_set_child (GTK_WINDOW (win), box);
        gtk_widget_show (win);

        g_object_unref (box);
}

static void
mg_detach_tab_cb (GtkWidget *item, chan *ch)
{
        if (chan_get_tag (ch) == TAG_IRC)       /* IRC tab */
        {
                /* userdata is session * */
                mg_link_irctab (chan_get_userdata (ch), 1);
                return;
        }

        /* userdata is GtkWidget * */
        mg_link_gentab (ch, chan_get_userdata (ch));    /* non-IRC tab */
}

static void
mg_destroy_tab_cb (GtkWidget *item, chan *ch)
{
        /* treat it just like the X button press */
        mg_xbutton_cb (mg_gui->chanview, ch, chan_get_tag (ch), chan_get_userdata (ch));
}


#define FABULOR_TAB_CONTEXT_POPUP "fabulor-tab-context-popup"

typedef struct
{
	FabulorTabContextMenuModel *model;
	FabulorContextMenuPresenterGtk4 *presenter;
	session *sess;
	chan *ch;
} FabulorTabContextPopup;

static gboolean
mg_tab_context_popup_free_idle (gpointer data)
{
	FabulorTabContextPopup *popup = data;
	fabulor_context_menu_presenter_gtk4_free (popup->presenter);
	fabulor_tab_context_menu_model_free (popup->model);
	g_free (popup);
	return G_SOURCE_REMOVE;
}

static void
mg_tab_context_popup_release (gpointer data)
{
	g_idle_add (mg_tab_context_popup_free_idle, data);
}

static gboolean
mg_tab_option_active (guint8 value, guint global)
{
	return (value == SET_DEFAULT ? global : value) != SET_OFF;
}

static guint8 *
mg_tab_option_setting (session *sess, FabulorTabOption option)
{
	switch (option)
	{
	case FABULOR_TAB_OPTION_NOTIFICATION: return &sess->alert_balloon;
	case FABULOR_TAB_OPTION_BEEP: return &sess->alert_beep;
	case FABULOR_TAB_OPTION_TRAY: return &sess->alert_tray;
	case FABULOR_TAB_OPTION_TASKBAR: return &sess->alert_taskbar;
	case FABULOR_TAB_OPTION_LOGGING: return &sess->text_logging;
	case FABULOR_TAB_OPTION_SCROLLBACK: return &sess->text_scrollback;
	case FABULOR_TAB_OPTION_STRIP_COLORS: return &sess->text_strip;
	case FABULOR_TAB_OPTION_HIDE_JOIN_PART: return &sess->text_hidejoinpart;
	default: return NULL;
	}
}

static void
mg_tab_context_dispatch (FabulorTabContextAction action,
	FabulorTabOption option, gboolean state, const char *command,
	gpointer user_data)
{
	FabulorTabContextPopup *popup = user_data;
	session *sess = is_session (popup->sess) ? popup->sess : NULL;

	if (action == FABULOR_TAB_CONTEXT_DETACH)
	{
		mg_detach_tab_cb (NULL, popup->ch);
		return;
	}
	if (action == FABULOR_TAB_CONTEXT_CLOSE)
	{
		mg_destroy_tab_cb (NULL, popup->ch);
		return;
	}
	if (!sess)
		return;
	if (action == FABULOR_TAB_CONTEXT_OPTION)
	{
		guint8 *setting = mg_tab_option_setting (sess, option);
		guint8 logging = sess->text_logging;
		if (!setting)
			return;
		*setting = state ? SET_ON : SET_OFF;
		if (logging != sess->text_logging)
			log_open_or_close (sess);
		chanopt_save (sess);
		chanopt_save_all (FALSE);
		return;
	}
	if (action == FABULOR_TAB_CONTEXT_AUTOJOIN && sess->server->network)
	{
		servlist_autojoinedit (sess->server->network, sess->channel, state);
		return;
	}
	if (action == FABULOR_TAB_CONTEXT_AUTOCONNECT && sess->server->network)
	{
		if (state)
			((ircnet *)sess->server->network)->flags |= FLAG_AUTO_CONNECT;
		else
			((ircnet *)sess->server->network)->flags &= ~FLAG_AUTO_CONNECT;
		servlist_save ();
		return;
	}
	if (action == FABULOR_TAB_CONTEXT_TOGGLE && command)
	{
		char buffer[256];
		g_snprintf (buffer, sizeof (buffer), "set %s %d", command, state);
		handle_command (sess, buffer, FALSE);
		return;
	}
	if (action == FABULOR_TAB_CONTEXT_COMMAND && command)
		nick_command_parse (sess, (char *)command, sess->channel, sess->channel);
}

static void
mg_tab_configured_clear (gpointer data)
{
	FabulorTabConfiguredItem *item = data;
	g_free ((char *)item->label);
	g_free ((char *)item->icon);
}

static GArray *
mg_tab_configured_snapshot (void)
{
	GArray *items = g_array_new (FALSE, FALSE,
		sizeof (FabulorTabConfiguredItem));
	GSList *list;
	g_array_set_clear_func (items, mg_tab_configured_clear);
	for (list = tabmenu_list; list; list = list->next)
	{
		struct popup *pop = list->data;
		FabulorTabConfiguredItem item = { 0 };
		if (!g_ascii_strncasecmp (pop->name, "SUB", 3))
		{
			item.kind = FABULOR_TAB_CONFIG_SUBMENU_BEGIN;
			item.label = g_strdup (pop->cmd);
		}
		else if (!g_ascii_strncasecmp (pop->name, "TOGGLE", 6))
		{
			item.kind = FABULOR_TAB_CONFIG_TOGGLE;
			item.label = g_strdup (pop->name + 7);
			item.command = pop->cmd;
			item.active = cfg_get_bool (pop->cmd);
		}
		else if (!g_ascii_strncasecmp (pop->name, "ENDSUB", 6))
			item.kind = FABULOR_TAB_CONFIG_SUBMENU_END;
		else if (!g_ascii_strncasecmp (pop->name, "SEP", 3))
			item.kind = FABULOR_TAB_CONFIG_SEPARATOR;
		else
		{
			char *icon;
			char *label;

			item.kind = FABULOR_TAB_CONFIG_COMMAND;
			menu_parse_icon_label (pop->name, &label, &icon);
			item.label = label;
			item.icon = icon;
			item.command = pop->cmd;
		}
		g_array_append_val (items, item);
	}
	return items;
}

static void
mg_tab_context_state_snapshot (session *sess, FabulorTabContextState *state)
{
	guint notification;
	guint beep;
	guint tray;
	guint taskbar;

	memset (state, 0, sizeof (*state));
	if (!sess)
		return;
	state->has_session = TRUE;
	state->is_channel = sess->type == SESS_CHANNEL;
	state->is_server = sess->type == SESS_SERVER;
	state->has_network = sess->server->network != NULL;
	if (sess->type == SESS_DIALOG)
	{
		notification = prefs.hex_input_balloon_priv;
		beep = prefs.hex_input_beep_priv;
		tray = prefs.hex_input_tray_priv;
		taskbar = prefs.hex_input_flash_priv;
	}
	else
	{
		notification = prefs.hex_input_balloon_chans;
		beep = prefs.hex_input_beep_chans;
		tray = prefs.hex_input_tray_chans;
		taskbar = prefs.hex_input_flash_chans;
	}
	state->options[FABULOR_TAB_OPTION_NOTIFICATION] =
		mg_tab_option_active (sess->alert_balloon, notification);
	state->options[FABULOR_TAB_OPTION_BEEP] =
		mg_tab_option_active (sess->alert_beep, beep);
	state->options[FABULOR_TAB_OPTION_TRAY] =
		mg_tab_option_active (sess->alert_tray, tray);
	state->options[FABULOR_TAB_OPTION_TASKBAR] =
		mg_tab_option_active (sess->alert_taskbar, taskbar);
	state->options[FABULOR_TAB_OPTION_LOGGING] =
		mg_tab_option_active (sess->text_logging, prefs.hex_irc_logging);
	state->options[FABULOR_TAB_OPTION_SCROLLBACK] =
		mg_tab_option_active (sess->text_scrollback, prefs.hex_text_replay);
	state->options[FABULOR_TAB_OPTION_STRIP_COLORS] =
		mg_tab_option_active (sess->text_strip, prefs.hex_text_stripcolor_msg);
	state->options[FABULOR_TAB_OPTION_HIDE_JOIN_PART] =
		mg_tab_option_active (sess->text_hidejoinpart, prefs.hex_irc_conf_mode);
	if (state->is_channel && state->has_network)
		state->autojoin = joinlist_is_in_list (sess->server, sess->channel);
	if (state->is_server && state->has_network)
		state->autoconnect =
			(((ircnet *)sess->server->network)->flags & FLAG_AUTO_CONNECT) != 0;
}

static void
mg_create_tabmenu (session *sess, GtkWidget *source, chan *ch,
	gdouble x, gdouble y)
{
	FabulorTabContextLabels labels = {
		_("_Extra Alerts"), _("_Settings"), _("Show Notifications"),
		_("Beep on _Message"), _("Blink Tray _Icon"),
		_("Blink Task _Bar"), _("_Log to Disk"),
		_("_Reload Scrollback"), _("Strip _Colors"),
		_("_Hide Join/Part Messages"), _("_Autojoin"),
		_("_Auto-Connect"), _("_Detach"), _("_Close")
	};
	FabulorTabContextPopup *popup;
	FabulorTabContextState state;
	GActionGroup *plugin_actions = NULL;
	GMenuModel *plugin_model = NULL;
	GArray *configured;
	const char *heading = NULL;

	mg_tab_context_state_snapshot (sess, &state);
	configured = sess ? mg_tab_configured_snapshot () :
		g_array_new (FALSE, FALSE, sizeof (FabulorTabConfiguredItem));
	if (sess)
	{
		heading = sess->channel[0] ? sess->channel : _("<none>");
		menu_add_plugin_model (G_OBJECT (source), "\x4$TAB", sess->channel);
		plugin_model = menu_plugin_context_model (G_OBJECT (source));
		plugin_actions = menu_plugin_context_actions (G_OBJECT (source));
	}
	popup = g_new0 (FabulorTabContextPopup, 1);
	popup->sess = sess;
	popup->ch = ch;
	popup->model = fabulor_tab_context_menu_model_new (heading, &state, &labels,
		(FabulorTabConfiguredItem *)configured->data, configured->len,
		plugin_model, mg_tab_context_dispatch, popup);
	g_array_unref (configured);
	if (!popup->model)
	{
		g_free (popup);
		return;
	}
	if (plugin_actions)
		popup->presenter = fabulor_context_menu_presenter_gtk4_new (
			fabulor_tab_context_menu_model_get_menu (popup->model),
			fabulor_tab_context_menu_model_get_actions (popup->model),
			plugin_actions);
	else
		popup->presenter =
			fabulor_context_menu_presenter_gtk4_new_with_namespaces (
				fabulor_tab_context_menu_model_get_menu (popup->model),
				FABULOR_CONTEXT_ACTION_NAMESPACE,
				fabulor_tab_context_menu_model_get_actions (popup->model),
				NULL, NULL);
	if (!popup->presenter)
	{
		fabulor_tab_context_menu_model_free (popup->model);
		g_free (popup);
		return;
	}
	g_object_set_data_full (G_OBJECT (source), FABULOR_TAB_CONTEXT_POPUP,
		popup, mg_tab_context_popup_release);
	fabulor_context_menu_presenter_gtk4_popup_at (popup->presenter, source, x, y);
}

static gboolean
mg_tab_contextmenu_cb (chanview *cv, chan *ch, int tag, gpointer ud,
        GtkWidget *source, guint button, gdouble x, gdouble y,
        GdkModifierType state)
{
        (void) state;
        /* middle-click to close a tab */
        if (prefs.hex_gui_tab_middleclose && button == 2)
        {
                mg_xbutton_cb (cv, ch, tag, ud);
                return TRUE;
        }

        if (button != 3)
                return FALSE;

        if (tag == TAG_IRC)
                mg_create_tabmenu (ud, source, ch
                        , x, y
                );
        else
                mg_create_tabmenu (NULL, source, ch
                        , x, y
                );

        return TRUE;
}

gboolean
mg_dnd_drop_file (session *sess, char *target, const char *uri_list)
{
        gchar **uris;
        gchar **uri;
        gboolean handled = FALSE;

        uris = g_uri_list_extract_uris (uri_list);
        for (uri = uris; uri && *uri; uri++)
        {
                if (g_ascii_strncasecmp ("file:", *uri, 5) == 0)
                {
                        char *fname = g_filename_from_uri (*uri, NULL, NULL);

                        if (fname)
                        {
                                char *utf8_name;

                                /* dcc_send() expects utf-8 */
                                utf8_name = g_filename_from_utf8 (fname, -1, 0, 0, 0);
                                if (utf8_name)
                                {
                                        dcc_send (sess, target, utf8_name, prefs.hex_dcc_max_send_cps, 0);
                                        g_free (utf8_name);
                                        handled = TRUE;
                                }
                                g_free (fname);
                        }
                }
        }
        g_strfreev (uris);
        return handled;
}

static gboolean
mg_dialog_file_drop (GtkWidget *widget, gdouble x, gdouble y,
                     const gchar *uri_list, gpointer user_data)
{
        (void) widget;
        (void) x;
        (void) y;
        (void) user_data;

        if (current_sess->type == SESS_DIALOG)
                /* sess->channel is really the nickname of dialogs */
                return mg_dnd_drop_file (current_sess, current_sess->channel, uri_list);

        return FALSE;
}

/* add a tabbed channel */

static void
mg_add_chan (session *sess)
{
        GdkPixbuf *icon;
        char *name = _("<none>");

        if (sess->res->buffer == NULL)
        {
                sess->res->buffer = gtk_xtext_buffer_new (GTK_XTEXT (sess->gui->xtext));
                gtk_xtext_set_time_stamp (sess->res->buffer, prefs.hex_stamp_text);
                sess->res->user_model = userlist_create_model (sess);
        }

        if (sess->channel[0])
                name = sess->channel;

        switch (sess->type)
        {
        case SESS_CHANNEL:
                icon = pix_tree_channel;
                break;
        case SESS_SERVER:
                icon = pix_tree_server;
                break;
        default:
                icon = pix_tree_dialog;
        }

        sess->res->tab = chanview_add (sess->gui->chanview, name, sess->server, sess,
                                                                                         sess->type == SESS_SERVER ? FALSE : TRUE,
                                                                                         TAG_IRC, icon);
        if (plain_list == NULL)
                mg_create_tab_colors ();

        chan_set_color (sess->res->tab, plain_list);
}

static void
mg_userlist_button (GtkWidget * box, char *label, char *cmd,
                                                  int a, int b, int c, int d)
{
        GtkWidget *wid = gtk_button_new_with_label (label);
        g_signal_connect (G_OBJECT (wid), "clicked",
                                                        G_CALLBACK (userlist_button_cb), cmd);
        gtk_widget_set_hexpand (wid, TRUE);
        gtk_widget_set_vexpand (wid, FALSE);
        gtk_widget_set_halign (wid, GTK_ALIGN_FILL);
        gtk_widget_set_valign (wid, GTK_ALIGN_CENTER);
        gtk_grid_attach (GTK_GRID (box), wid, a, c, b - a, d - c);
        show_and_unfocus (wid);
}

static GtkWidget *
mg_create_userlistbuttons (GtkWidget *box)
{
        struct popup *pop;
        GSList *list = button_list;
        int a = 0, b = 0;
        GtkWidget *tab;

        tab = gtk_grid_new ();
        fabulor_gtk_box_append (GTK_BOX (box), tab, FALSE, FALSE, 0);

        while (list)
        {
                pop = list->data;
                if (pop->cmd[0])
                {
                        mg_userlist_button (tab, pop->name, pop->cmd, a, a + 1, b, b + 1);
                        a++;
                        if (a == 2)
                        {
                                a = 0;
                                b++;
                        }
                }
                list = list->next;
        }

        return tab;
}

static void
mg_topic_cb (GtkWidget *entry)
{
        session *sess = current_sess;
        GtkTextBuffer *topic_buffer;
        GtkTextIter start;
        GtkTextIter end;
        char *text;

        if (sess->channel[0] && sess->server->connected && sess->type == SESS_CHANNEL)
        {
                topic_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (entry));
                gtk_text_buffer_get_bounds (topic_buffer, &start, &end);
                text = gtk_text_buffer_get_text (topic_buffer, &start, &end, FALSE);
                if (text[0] == 0)
                        sess->server->p_topic (sess->server, sess->channel, NULL);
                else
                        sess->server->p_topic (sess->server, sess->channel, text);
                g_free (text);
        } else
        {
                topic_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (entry));
                gtk_text_buffer_set_text (topic_buffer, "", -1);
        }
        /* restore focus to the input widget, where the next input will most
likely be */
        gtk_widget_grab_focus (sess->gui->input_box);
}

static gboolean
mg_topic_key_press_cb (GtkWidget *entry, guint keyval, GdkModifierType state,
                       gpointer user_data)
{
        (void) state;
        (void) user_data;

        if (keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter)
        {
                mg_topic_cb (entry);
                return TRUE;
        }

        return FALSE;
}

static char *
mg_topic_get_word_at_pos (GtkWidget *entry, gdouble event_x, gdouble event_y, int *word_pos)
{
        GtkTextBuffer *buffer;
        GtkTextIter iter;
        GtkTextIter start;
        GtkTextIter end;
        GtkTextIter cursor;
        int x;
        int y;

        x = (int)event_x;
        y = (int)event_y;
        gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (entry), GTK_TEXT_WINDOW_TEXT,
                                               x, y, &x, &y);
        gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (entry), &iter, x, y);

        cursor = iter;
        start = iter;
        while (!gtk_text_iter_starts_line (&start))
        {
                GtkTextIter prev = start;
                gunichar ch;

                gtk_text_iter_backward_char (&prev);
                ch = gtk_text_iter_get_char (&prev);
                if (g_unichar_isspace (ch))
                        break;
                start = prev;
        }

        end = iter;
        while (!gtk_text_iter_ends_line (&end))
        {
                gunichar ch;

                ch = gtk_text_iter_get_char (&end);
                if (ch == 0 || g_unichar_isspace (ch))
                        break;
                gtk_text_iter_forward_char (&end);
        }

        if (gtk_text_iter_equal (&start, &end))
                return NULL;

        if (word_pos)
        {
                char *prefix;

                buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (entry));
                prefix = gtk_text_buffer_get_text (buffer, &start, &cursor, FALSE);
                *word_pos = (int)strlen (prefix);
                g_free (prefix);
        }

        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (entry));
        return gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
}

static gboolean
mg_topic_word_is_clickable (const char *word, int word_pos)
{
        int start;
        int end;
        int word_type;

        if (!word || word[0] == 0)
                return FALSE;

        if (strcmp (word, "/") == 0)
                return FALSE;

        word_type = url_check_word (word);
        if (word_type != WORD_URL && word_type != WORD_HOST && word_type != WORD_HOST6)
                return FALSE;

        url_last (&start, &end);
        return word_pos >= start && word_pos < end;
}

static void
mg_topic_motion_cb (GtkWidget *entry, gdouble x, gdouble y,
                    gpointer user_data)
{
        char *word;
        int word_pos;
        gboolean word_is_clickable;

        (void) user_data;

        word_pos = 0;
        word = mg_topic_get_word_at_pos (entry, x, y, &word_pos);
        word_is_clickable = mg_topic_word_is_clickable (word, word_pos);
        fabulor_gtk_text_view_set_pointing_cursor (GTK_TEXT_VIEW (entry),
                                                   word_is_clickable);
        g_free (word);
}

static void
mg_topic_leave_cb (GtkWidget *entry, gpointer user_data)
{
        (void) user_data;
        fabulor_gtk_text_view_set_pointing_cursor (GTK_TEXT_VIEW (entry), FALSE);
}

static gboolean
mg_topic_button_release_cb (GtkWidget *entry, guint button, gdouble x,
                            gdouble y, GdkModifierType state,
                            gpointer user_data)
{
        char *word;
        int word_pos;
        int start;
        int end;

        (void) user_data;

        if (button != 1)
                return FALSE;

        word_pos = 0;
        word = mg_topic_get_word_at_pos (entry, x, y, &word_pos);
        if (!word)
                return FALSE;

        if ((state & 13) == prefs.hex_gui_url_mod &&
            mg_topic_word_is_clickable (word, word_pos))
        {
                url_last (&start, &end);
                word[end] = 0;
                fe_open_url (word + start);
                g_free (word);
                return TRUE;
        }

        g_free (word);
        return FALSE;
}

static void
mg_tabwindow_kill_cb (GtkWidget *win, gpointer userdata)
{
        GSList *list, *next;
        session *sess;

        mg_flush_config_save ();

        zoitechat_is_quitting = TRUE;

        /* see if there's any non-tab windows left */
        list = sess_list;
        while (list)
        {
                sess = list->data;
                next = list->next;
                if (!sess->gui->is_tab)
                {
                        zoitechat_is_quitting = FALSE;
                } else
                {
                        mg_ircdestroy (sess);
                }
                list = next;
        }

        current_tab = NULL;
        active_tab = NULL;
        mg_gui = NULL;
        parent_window = NULL;
}

static GtkWidget *
mg_changui_destroy (session *sess)
{
        GtkWidget *ret = NULL;
        GtkWidget *window = sess->gui->window;
        session_gui *gui = sess->gui;

        if (gui->is_tab)
        {
                /* This window may be rebuilt around the same live session. */
                mg_tabwindow_lifecycle_disconnect (window, gui);
                if (chanview_get_size (gui->chanview) == 1)
                        mg_theme_window_cleanup (window, gui);
                /* remove the tab from the chanview */
                if (!mg_chan_remove (sess->res->tab))
                        /* if the window still exists, restore its lifecycle owner */
                        mg_tabwindow_lifecycle_connect (window, gui);
        } else
        {
                mg_topwindow_lifecycle_disconnect (window, sess);
                mg_theme_window_cleanup (window, gui);
                /* don't destroy until the new one is created. Not sure why, but */
                /* it fixes: Gdk-CRITICAL **: gdk_colormap_get_screen: */
                /*           assertion `GDK_IS_COLORMAP (cmap)' failed */
                ret = window;
                sess->gui = NULL;
        }
        return ret;
}

static void
mg_link_irctab (session *sess, int focus)
{
        GtkWidget *win;

        if (sess->gui->is_tab)
        {
                win = mg_changui_destroy (sess);
                mg_changui_new (sess, sess->res, 0, focus);
                mg_populate (sess);
                zoitechat_is_quitting = FALSE;
                if (win)
                        fabulor_gtk_window_destroy (GTK_WINDOW (win));
                return;
        }

        session_gui *old_gui;

        mg_unpopulate (sess);
        old_gui = sess->gui;
        win = mg_changui_destroy (sess);
        mg_changui_new (sess, sess->res, 1, focus);
        /* the buffer is now attached to a different widget */
        ((xtext_buffer *)sess->res->buffer)->xtext = (GtkXText *)sess->gui->xtext;
        if (win)
                fabulor_gtk_window_destroy (GTK_WINDOW (win));
        g_free (old_gui);
}

void
mg_detach (session *sess, int mode)
{
        switch (mode)
        {
        /* detach only */
        case 1:
                if (sess->gui->is_tab)
                        mg_link_irctab (sess, 1);
                break;
        /* attach only */
        case 2:
                if (!sess->gui->is_tab)
                        mg_link_irctab (sess, 1);
                break;
        /* toggle */
        default:
                mg_link_irctab (sess, 1);
        }
}

static int
check_is_number (char *t)
{
        while (*t)
        {
                if (*t < '0' || *t > '9')
                        return FALSE;
                t++;
        }
        return TRUE;
}

static void
mg_change_flag (GtkWidget * wid, session *sess, char flag)
{
        server *serv = sess->server;
        char mode[3];

        mode[1] = flag;
        mode[2] = '\0';
        if (serv->connected && sess->channel[0])
        {
                if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wid)))
                        mode[0] = '+';
                else
                        mode[0] = '-';
                serv->p_mode (serv, sess->channel, mode);
                serv->p_join_info (serv, sess->channel);
                sess->ignore_mode = TRUE;
                sess->ignore_date = TRUE;
        }
}

static void
flagl_hit (GtkWidget * wid, struct session *sess)
{
        char modes[512];
        const char *limit_str;
        server *serv = sess->server;

        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wid)))
        {
                if (serv->connected && sess->channel[0])
                {
                        limit_str = fabulor_gtk_entry_get_text (GTK_ENTRY (sess->gui->limit_entry));
                        if (check_is_number ((char *)limit_str) == FALSE)
                        {
                                fe_message (_("User limit must be a number!\n"), FE_MSG_ERROR);
                                fabulor_gtk_entry_set_text (GTK_ENTRY (sess->gui->limit_entry), "");
                                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wid), FALSE);
                                return;
                        }
                        g_snprintf (modes, sizeof (modes), "+l %d", atoi (limit_str));
                        serv->p_mode (serv, sess->channel, modes);
                        serv->p_join_info (serv, sess->channel);
                }
        } else
                mg_change_flag (wid, sess, 'l');
}

static void
flagk_hit (GtkWidget * wid, struct session *sess)
{
        char modes[512];
        server *serv = sess->server;

        if (serv->connected && sess->channel[0])
        {
                g_snprintf (modes, sizeof (modes), "-k %s", 
                          fabulor_gtk_entry_get_text (GTK_ENTRY (sess->gui->key_entry)));

                if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wid)))
                        modes[0] = '+';

                serv->p_mode (serv, sess->channel, modes);
        }
}

static void
mg_flagbutton_cb (GtkWidget *but, char *flag)
{
        session *sess;
        char mode;

        if (ignore_chanmode)
                return;

        sess = current_sess;
        mode = tolower ((unsigned char) flag[0]);

        switch (mode)
        {
        case 'l':
                flagl_hit (but, sess);
                break;
        case 'k':
                flagk_hit (but, sess);
                break;
        case 'b':
                ignore_chanmode = TRUE;
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sess->gui->flag_b), FALSE);
                ignore_chanmode = FALSE;
                banlist_opengui (sess);
                break;
        default:
                mg_change_flag (but, sess, mode);
        }
}

static GtkWidget *
mg_create_flagbutton (char *tip, GtkWidget *box, char *face)
{
        GtkWidget *btn, *lbl;
        char label_markup[16];

        g_snprintf (label_markup, sizeof(label_markup), "<tt>%s</tt>", face);
        lbl = gtk_label_new (NULL);
        gtk_label_set_markup (GTK_LABEL(lbl), label_markup);

        btn = gtk_toggle_button_new ();
        gtk_widget_set_size_request (btn, -1, 11);
        gtk_widget_set_tooltip_text (btn, tip);
        fabulor_gtk_button_set_flat (GTK_BUTTON (btn));
        mg_apply_compact_mode_css (btn);
        fabulor_gtk_button_set_child (GTK_BUTTON (btn), lbl);

        fabulor_gtk_box_append (GTK_BOX (box), btn, FALSE, FALSE, 0);
        g_signal_connect (G_OBJECT (btn), "toggled",
                                                        G_CALLBACK (mg_flagbutton_cb), face);
        show_and_unfocus (btn);

        return btn;
}

static void
mg_key_entry_cb (GtkWidget * igad, gpointer userdata)
{
        char modes[512];
        session *sess = current_sess;
        server *serv = sess->server;

        if (serv->connected && sess->channel[0])
        {
                g_snprintf (modes, sizeof (modes), "+k %s",
                                fabulor_gtk_entry_get_text (GTK_ENTRY (igad)));
                serv->p_mode (serv, sess->channel, modes);
                serv->p_join_info (serv, sess->channel);
        }
}

static void
mg_limit_entry_cb (GtkWidget * igad, gpointer userdata)
{
        char modes[512];
        session *sess = current_sess;
        server *serv = sess->server;

        if (serv->connected && sess->channel[0])
        {
                if (check_is_number ((char *)fabulor_gtk_entry_get_text (GTK_ENTRY (igad))) == FALSE)
                {
                        fabulor_gtk_entry_set_text (GTK_ENTRY (igad), "");
                        fe_message (_("User limit must be a number!\n"), FE_MSG_ERROR);
                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sess->gui->flag_l), FALSE);
                        return;
                }
                g_snprintf (modes, sizeof(modes), "+l %d", 
                                atoi (fabulor_gtk_entry_get_text (GTK_ENTRY (igad))));
                serv->p_mode (serv, sess->channel, modes);
                serv->p_join_info (serv, sess->channel);
        }
}

static void
mg_apply_entry_style (GtkWidget *entry)
{
	theme_manager_apply_entry_palette (entry, input_style->font_desc);
}

static void
mg_apply_entry_scroll_artifact_fix (GtkWidget *entry)
{
	GtkStyleContext *context;
	GtkCssProvider *provider;

	if (!entry || !GTK_IS_ENTRY (entry))
		return;

	context = gtk_widget_get_style_context (entry);
	if (!context)
		return;

	provider = g_object_get_data (G_OBJECT (entry), "mg-entry-scroll-artifact-provider");
	if (!provider)
	{
		provider = gtk_css_provider_new ();
		g_object_set_data_full (G_OBJECT (entry), "mg-entry-scroll-artifact-provider", provider, g_object_unref);
		gtk_css_provider_load_from_data (provider,
			"entry.zoitechat-no-undershoot undershoot,\n"
			"entry.zoitechat-no-undershoot undershoot.left,\n"
			"entry.zoitechat-no-undershoot undershoot.right,\n"
			".zoitechat-no-undershoot undershoot,\n"
			".zoitechat-no-undershoot undershoot.left,\n"
			".zoitechat-no-undershoot undershoot.right {\n"
			"  background-image: none;\n"
			"  background-color: transparent;\n"
			"  border: none;\n"
			"  box-shadow: none;\n"
			"}\n",
			-1);
	}

	gtk_style_context_add_class (context, "zoitechat-no-undershoot");
	theme_css_apply_widget_provider (entry, GTK_STYLE_PROVIDER (provider));
}

static gboolean
mg_entry_select_all (GtkWidget *entry, guint keyval, GdkModifierType state,
					 gpointer user_data)
{
	(void) user_data;

	if ((state & GDK_CONTROL_MASK) &&
		!(state & (STATE_SHIFT | STATE_ALT | GDK_META_MASK)) &&
		(keyval == GDK_KEY_a || keyval == GDK_KEY_A))
	{
		gtk_editable_select_region (GTK_EDITABLE (entry), 0, -1);
		return TRUE;
	}

	return FALSE;
}

static void
mg_create_chanmodebuttons (session_gui *gui, GtkWidget *box)
{
        gui->flag_c = mg_create_flagbutton (_("Filter Colors"), box, "c");
        gui->flag_n = mg_create_flagbutton (_("No outside messages"), box, "n");
        gui->flag_t = mg_create_flagbutton (_("Topic Protection"), box, "t");
        gui->flag_i = mg_create_flagbutton (_("Invite Only"), box, "i");
        gui->flag_m = mg_create_flagbutton (_("Moderated"), box, "m");
        gui->flag_b = mg_create_flagbutton (_("Ban List"), box, "b");

        gui->flag_k = mg_create_flagbutton (_("Keyword"), box, "k");
        gui->key_entry = gtk_entry_new ();
        gtk_widget_set_name (gui->key_entry, "zoitechat-inputbox");
        gtk_entry_set_max_length (GTK_ENTRY (gui->key_entry), 23);
        gtk_widget_set_size_request (gui->key_entry, 58, 11);
        fabulor_gtk_box_append (GTK_BOX (box), gui->key_entry, FALSE, FALSE, 0);
        mg_apply_emoji_fallback_widget (gui->key_entry);
        mg_apply_compact_mode_css (gui->key_entry);
        g_signal_connect (G_OBJECT (gui->key_entry), "activate",
                                                        G_CALLBACK (mg_key_entry_cb), NULL);
        fabulor_gtk_widget_on_key_pressed (gui->key_entry,
                                          mg_entry_select_all, NULL);

        if (prefs.hex_gui_input_style)
                mg_apply_entry_style (gui->key_entry);
        mg_apply_entry_scroll_artifact_fix (gui->key_entry);

        gui->flag_l = mg_create_flagbutton (_("User Limit"), box, "l");
        gui->limit_entry = gtk_entry_new ();
        gtk_widget_set_name (gui->limit_entry, "zoitechat-inputbox");
        gtk_entry_set_max_length (GTK_ENTRY (gui->limit_entry), 10);
	fabulor_gtk_entry_set_width_chars (GTK_ENTRY (gui->limit_entry), 5);
        gtk_widget_set_size_request (gui->limit_entry, 45, 11);
        fabulor_gtk_box_append (GTK_BOX (box), gui->limit_entry, FALSE, FALSE, 0);
        mg_apply_emoji_fallback_widget (gui->limit_entry);
        mg_apply_compact_mode_css (gui->limit_entry);
        g_signal_connect (G_OBJECT (gui->limit_entry), "activate",
                                                        G_CALLBACK (mg_limit_entry_cb), NULL);
        fabulor_gtk_widget_on_key_pressed (gui->limit_entry,
                                          mg_entry_select_all, NULL);

        if (prefs.hex_gui_input_style)
                mg_apply_entry_style (gui->limit_entry);
        mg_apply_entry_scroll_artifact_fix (gui->limit_entry);
}

/*static void
mg_create_link_buttons (GtkWidget *box, gpointer userdata)
{
        gtkutil_button (box, ICON_TAB_CLOSE, _("Close this tab/window"),
                                                 mg_x_click_cb, userdata, 0);

        if (!userdata)
        gtkutil_button (box, ICON_TAB_DETACH, _("Attach/Detach this tab"),
                                                 mg_link_cb, userdata, 0);
}*/

static void
mg_dialog_button_cb (GtkWidget *wid, char *cmd)
{
        /* the longest cmd is 12, and the longest nickname is 64 */
        char buf[128];
        char *host = "";
        char *topic;
        char *topic_text;
        GtkTextBuffer *topic_buffer;
        GtkTextIter start;
        GtkTextIter end;

        if (!current_sess)
                return;

        topic_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (current_sess->gui->topic_entry));
        gtk_text_buffer_get_bounds (topic_buffer, &start, &end);
        topic_text = gtk_text_buffer_get_text (topic_buffer, &start, &end, FALSE);
        topic = strrchr (topic_text, '@');
        if (topic)
                host = topic + 1;

        auto_insert (buf, sizeof (buf), cmd, 0, 0, "", "", "",
                                         server_get_network (current_sess->server, TRUE), host, "",
                                         current_sess->channel, "");

        handle_command (current_sess, buf, TRUE);
        g_free (topic_text);

        /* dirty trick to avoid auto-selection */
        SPELL_ENTRY_SET_EDITABLE (current_sess->gui->input_box, FALSE);
        gtk_widget_grab_focus (current_sess->gui->input_box);
        SPELL_ENTRY_SET_EDITABLE (current_sess->gui->input_box, TRUE);
}

static void
mg_dialog_button (GtkWidget *box, char *name, char *cmd)
{
        GtkWidget *wid;

        wid = gtk_button_new_with_label (name);
        fabulor_gtk_box_append (GTK_BOX (box), wid, FALSE, FALSE, 0);
        g_signal_connect (G_OBJECT (wid), "clicked",
                                                        G_CALLBACK (mg_dialog_button_cb), cmd);
        gtk_widget_set_size_request (wid, -1, 0);
}

static void
mg_create_dialogbuttons (GtkWidget *box)
{
        struct popup *pop;
        GSList *list = dlgbutton_list;

        while (list)
        {
                pop = list->data;
                if (pop->cmd[0])
                        mg_dialog_button (box, pop->name, pop->cmd);
                list = list->next;
        }
}

static void
mg_topicbar_update_height (GtkWidget *topic)
{
	GtkWidget *parent;
	GtkWidget *grandparent;
	GtkTextBuffer *buffer;
	GtkTextIter start;
	GtkTextIter end;
	GtkTextView *view;
	PangoLayout *layout;
	char *text;
	int width;
	int line_height;
	int line_count;
	int target_height;
	int margin_left;
	int margin_right;
	int margin_top;
	int margin_bottom;
	int old_height;
	PangoContext *context;
	PangoFontMetrics *metrics;

	if (!topic || !GTK_IS_TEXT_VIEW (topic))
		return;

	view = GTK_TEXT_VIEW (topic);
	parent = gtk_widget_get_parent (topic);
	grandparent = parent ? gtk_widget_get_parent (parent) : NULL;

	margin_left = gtk_text_view_get_left_margin (view);
	margin_right = gtk_text_view_get_right_margin (view);
	margin_top = gtk_text_view_get_top_margin (view);
	margin_bottom = gtk_text_view_get_bottom_margin (view);

	buffer = gtk_text_view_get_buffer (view);
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
	layout = gtk_widget_create_pango_layout (topic, text && text[0] ? text : " ");
	g_free (text);

	width = gtk_widget_get_allocated_width (topic);
	if (width <= 1 && parent)
		width = gtk_widget_get_allocated_width (parent);
	width -= margin_left + margin_right;
	if (width < 1)
		width = 1;
	if (prefs.hex_gui_topicbar_multiline && !prefs.hex_gui_mode_buttons_inline)
	{
		pango_layout_set_width (layout, width * PANGO_SCALE);
		pango_layout_set_wrap (layout, PANGO_WRAP_WORD_CHAR);
	}

	context = gtk_widget_get_pango_context (topic);
	metrics = pango_context_get_metrics (context,
		pango_context_get_font_description (context),
		pango_context_get_language (context));
	line_height = PANGO_PIXELS (pango_font_metrics_get_ascent (metrics) +
		pango_font_metrics_get_descent (metrics));
	pango_font_metrics_unref (metrics);
	if (line_height <= 0)
		line_height = 16;

	line_count = prefs.hex_gui_topicbar_multiline && !prefs.hex_gui_mode_buttons_inline ?
		pango_layout_get_line_count (layout) : 1;
	if (line_count <= 0)
		line_count = 1;

	target_height = (line_height * line_count) + margin_top + margin_bottom;
	if (target_height < line_height + margin_top + margin_bottom)
		target_height = line_height + margin_top + margin_bottom;

	old_height = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (topic),
		"mg-topicbar-target-height"));
	if (old_height != target_height)
	{
		g_object_set_data (G_OBJECT (topic), "mg-topicbar-target-height",
			GINT_TO_POINTER (target_height));
		gtk_widget_set_size_request (topic, -1, target_height);

		if (parent && GTK_IS_SCROLLED_WINDOW (parent))
		{
			gtk_scrolled_window_set_max_content_height (GTK_SCROLLED_WINDOW (parent), -1);
			gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (parent), -1);
			gtk_scrolled_window_set_max_content_height (GTK_SCROLLED_WINDOW (parent), target_height);
			gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (parent), target_height);
			gtk_widget_set_size_request (parent, -1, target_height);
		}
	}

	gtk_widget_queue_resize (topic);
	if (parent)
		gtk_widget_queue_resize (parent);
	if (grandparent)
		gtk_widget_queue_resize (grandparent);
	gtk_widget_queue_draw (topic);
	g_object_unref (layout);
}

static gboolean
mg_topicbar_relayout_idle_cb (gpointer userdata)
{
	GtkWidget *topic = GTK_WIDGET (userdata);

	g_object_set_data (G_OBJECT (topic), "mg-topicbar-relayout-source", NULL);
	mg_topicbar_update_height (topic);
	g_object_unref (topic);

	return G_SOURCE_REMOVE;
}

static void
mg_topicbar_queue_relayout (GtkWidget *topic)
{
	guint source_id;

	if (!topic || !GTK_IS_TEXT_VIEW (topic))
		return;

	if (g_object_get_data (G_OBJECT (topic), "mg-topicbar-relayout-source") != NULL)
		return;

	source_id = g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
		mg_topicbar_relayout_idle_cb,
		g_object_ref (topic),
		NULL);
	g_object_set_data (G_OBJECT (topic), "mg-topicbar-relayout-source",
		GUINT_TO_POINTER (source_id));
}

static void
mg_topicbar_buffer_changed_cb (GtkTextBuffer *buffer, gpointer userdata)
{
	(void) buffer;
	mg_topicbar_queue_relayout (GTK_WIDGET (userdata));
}

static void
mg_topicbar_size_allocate_cb (GtkWidget *widget, GtkAllocation *allocation, gpointer userdata)
{
	int old_width;

	(void) userdata;

	old_width = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget),
		"mg-topicbar-allocated-width"));
	if (allocation->width == old_width)
		return;

	g_object_set_data (G_OBJECT (widget), "mg-topicbar-allocated-width",
		GINT_TO_POINTER (allocation->width));
	mg_topicbar_queue_relayout (widget);
}

static void
mg_apply_main_font_widget (GtkWidget *widget, const PangoFontDescription *font)
{
	if (!widget)
		return;

	theme_manager_apply_palette_widget (widget, NULL, NULL, font);
	gtk_widget_queue_resize (widget);
}

static void
mg_apply_main_font_menu (GtkWidget *menu, const PangoFontDescription *font)
{

	if (!menu || !GTK_IS_WIDGET (menu))
		return;

	mg_apply_main_font_widget (menu, font);
}

void
mg_apply_session_font_prefs (session_gui *gui)
{
	const PangoFontDescription *font = NULL;

	if (!gui)
		return;

	if (input_style)
		font = input_style->font_desc;

	if (gui->topic_entry)
	{
		theme_manager_apply_entry_palette (gui->topic_entry, font);
		mg_topicbar_update_height (gui->topic_entry);
	}

	if (gui->input_box && prefs.hex_gui_input_style)
		theme_manager_apply_entry_palette (gui->input_box, font);

	if (gui->nick_label)
		mg_apply_main_font_widget (gui->nick_label, font);

	if (gui->menu)
		mg_apply_main_font_menu (gui->menu, font);

	if (gui->chanview)
		chanview_apply_theme (gui->chanview);

	if (gui->user_tree)
		theme_manager_apply_userlist_style (gui->user_tree,
			theme_manager_get_userlist_palette_behavior (font));
}


static void
mg_create_topicbar (session *sess, GtkWidget *box)
{
	GtkWidget *vbox, *hbox, *mode_hbox, *topic, *topic_scroll, *bbox;
	session_gui *gui = sess->gui;

	gui->topic_bar = vbox = mg_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 0);
	fabulor_gtk_box_append (GTK_BOX (box), vbox, FALSE, FALSE, 0);

	hbox = mg_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
	fabulor_gtk_box_append (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

        if (!gui->is_tab)
                sess->res->tab = NULL;

        gui->topic_entry = topic = gtk_text_view_new ();
        gtk_widget_set_name (topic, "zoitechat-topicbox");
        gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (topic),
		prefs.hex_gui_topicbar_multiline && !prefs.hex_gui_mode_buttons_inline ?
		GTK_WRAP_WORD_CHAR : GTK_WRAP_NONE);
        gtk_text_view_set_left_margin (GTK_TEXT_VIEW (topic), 4);
        gtk_text_view_set_right_margin (GTK_TEXT_VIEW (topic), 4);
        gtk_text_view_set_top_margin (GTK_TEXT_VIEW (topic),
		prefs.hex_gui_mode_buttons_inline ? 0 : 4);
        gtk_text_view_set_bottom_margin (GTK_TEXT_VIEW (topic),
		prefs.hex_gui_mode_buttons_inline ? 0 : 4);
        gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (topic), 0);
        gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (topic), 0);
        gtk_text_view_set_pixels_inside_wrap (GTK_TEXT_VIEW (topic), 0);
        theme_manager_apply_entry_palette (topic, input_style ? input_style->font_desc : NULL);
        g_signal_connect (gtk_text_view_get_buffer (GTK_TEXT_VIEW (topic)), "changed",
                                                        G_CALLBACK (mg_topicbar_buffer_changed_cb), topic);
        g_signal_connect (G_OBJECT (topic), "size-allocate",
                                                        G_CALLBACK (mg_topicbar_size_allocate_cb), NULL);
        topic_scroll = gtk_scrolled_window_new ();
	gtk_widget_set_hexpand (topic_scroll, TRUE);
	gtk_widget_set_size_request (topic_scroll, 1, -1);
	gtk_widget_set_size_request (topic, 1, -1);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (topic_scroll),
		GTK_POLICY_EXTERNAL, GTK_POLICY_NEVER);
	gtk_scrolled_window_set_propagate_natural_width (GTK_SCROLLED_WINDOW (topic_scroll), FALSE);
	fabulor_gtk_scrolled_window_set_framed (
		GTK_SCROLLED_WINDOW (topic_scroll), FALSE);
	fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (topic_scroll), topic);
        mg_topicbar_update_height (topic);
	fabulor_gtk_box_append (GTK_BOX (hbox), topic_scroll, TRUE, TRUE, 0);
        fabulor_gtk_widget_on_key_pressed (topic, mg_topic_key_press_cb, NULL);
        fabulor_gtk_widget_on_click_released (topic,
                                              mg_topic_button_release_cb, NULL);
        fabulor_gtk_widget_on_pointer_motion (topic, mg_topic_motion_cb,
                                              mg_topic_leave_cb, NULL);

	gui->dialogbutton_box = bbox = mg_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
	fabulor_gtk_box_append (GTK_BOX (hbox), bbox, FALSE, FALSE, 0);
	mg_create_dialogbuttons (bbox);

	mode_hbox = mg_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
	if (prefs.hex_gui_mode_buttons_inline)
		fabulor_gtk_box_append (GTK_BOX (hbox), mode_hbox, FALSE, FALSE, 0);
	else
		fabulor_gtk_box_append (GTK_BOX (vbox), mode_hbox, FALSE, FALSE, 0);

	gui->topicbutton_box = bbox = mg_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
	gtk_widget_set_valign (bbox, GTK_ALIGN_CENTER);
	fabulor_gtk_horizontal_box_append_trailing (GTK_BOX (mode_hbox), bbox);
	mg_create_chanmodebuttons (gui, bbox);
}

/* check if a word is clickable */

static int
mg_word_check (GtkWidget * xtext, char *word)
{
        session *sess = current_sess;
        int ret;

        ret = url_check_word (word);
        if (ret == 0 && sess->type == SESS_DIALOG)
                return WORD_DIALOG;

        return ret;
}

/* mouse click inside text area */

static void
mg_word_clicked (GtkWidget *xtext, const FabulorXTextHit *hit,
                 const FabulorXTextClick *click)
{
        session *sess = current_sess;
        char *word = hit ? hit->word : NULL;
        int word_type = hit ? hit->type : 0;
        char *matched;
        char *tmp;

        if (click->button == 1)                  /* left button */
        {
                if (word == NULL)
                {
                        mg_focus (sess);
                        return;
                }

                if ((click->state & 13) == prefs.hex_gui_url_mod)
                {
                        switch (word_type)
                        {
                        case WORD_URL:
                        case WORD_HOST6:
                        case WORD_HOST:
                                matched = fabulor_xtext_hit_dup_match (hit);
                                if (matched)
                                {
                                        fe_open_url (matched);
                                        g_free (matched);
                                }
                        }
                }
                return;
        }

        if (click->button == 2)
        {
                if (sess->type == SESS_DIALOG)
                        menu_middlemenu_at (sess, xtext, click->x, click->y,
                                            click->state);
                else if (click->n_press == 2)
                        userlist_select (sess, word);
                return;
        }
        if (word == NULL)
                return;

        switch (word_type)
        {
        case 0:
        case WORD_PATH:
                menu_middlemenu_at (sess, xtext, click->x, click->y,
                                    click->state);
                break;
        case WORD_URL:
        case WORD_HOST6:
        case WORD_HOST:
                matched = fabulor_xtext_hit_dup_match (hit);
                if (matched)
                {
                        menu_urlmenu_at (xtext, click->x, click->y,
                                        click->state, matched);
                        g_free (matched);
                }
                break;
        case WORD_NICK:
                matched = fabulor_xtext_hit_dup_match (hit);
                if (matched)
                {
                        menu_nickmenu_at (sess, xtext, click->x, click->y,
                                          click->state, matched, FALSE);
                        g_free (matched);
                }
                break;
        case WORD_CHANNEL:
                matched = fabulor_xtext_hit_dup_match (hit);
                if (matched)
                {
                        menu_chanmenu_at (sess, xtext, click->x, click->y,
                                          click->state, matched);
                        g_free (matched);
                }
                break;
        case WORD_EMAIL:
                matched = fabulor_xtext_hit_dup_match (hit);
                if (matched)
                {
                        tmp = g_strdup_printf ("mailto:%s",
                                               matched + (ispunct (*matched) ? 1 : 0));
                        menu_urlmenu_at (xtext, click->x, click->y,
                                        click->state, tmp);
                        g_free (tmp);
                        g_free (matched);
                }
                break;
        case WORD_DIALOG:
                menu_nickmenu_at (sess, xtext, click->x, click->y,
                                  click->state, sess->channel, FALSE);
                break;
        }
}


static void
mg_font_error_dialog_response (GtkDialog *dialog, gint response_id,
                               gpointer user_data)
{
        (void)response_id;
        (void)user_data;
        fabulor_gtk_window_destroy (GTK_WINDOW (dialog));
        exit (1);
}

static void
mg_show_font_error (GtkWidget *xtext)
{
        if (font_error_dialog)
        {
                gtk_window_present (GTK_WINDOW (font_error_dialog));
                return;
        }

        gtk_widget_hide (xtext);
        font_error_dialog = gtk_message_dialog_new (
                parent_window ? GTK_WINDOW (parent_window) : NULL,
                GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "%s",
                _("Failed to open any font. I'm out of here!"));
        theme_manager_attach_window (font_error_dialog);
        gtk_window_set_resizable (GTK_WINDOW (font_error_dialog), FALSE);
        g_signal_connect (G_OBJECT (font_error_dialog), "response",
                          G_CALLBACK (mg_font_error_dialog_response), NULL);
        g_object_add_weak_pointer (G_OBJECT (font_error_dialog),
                                   (gpointer *) &font_error_dialog);
        gtk_widget_show (font_error_dialog);
}

static void
mg_update_xtext_internal (GtkWidget *wid, gboolean update_font)
{
        GtkXText *xtext = GTK_XTEXT (wid);
        const gchar *font_name;
        XTextColor xtext_palette[XTEXT_COLS];

        theme_get_xtext_colors_for_widget (wid, xtext_palette, XTEXT_COLS);
        gtk_xtext_set_palette (xtext, xtext_palette);
        gtk_xtext_set_max_lines (xtext, prefs.hex_text_max_lines);
        gtk_xtext_set_background (xtext, channelwin_pix);
        gtk_xtext_set_wordwrap (xtext, prefs.hex_text_wordwrap);
        gtk_xtext_set_show_marker (xtext, prefs.hex_text_show_marker);
        gtk_xtext_set_show_separator (xtext, prefs.hex_text_indent ? prefs.hex_text_show_sep : 0);
        gtk_xtext_set_indent (xtext, prefs.hex_text_indent);

        if (update_font)
        {
                font_name = *prefs.hex_text_font
                        ? prefs.hex_text_font
                        : "Sans 10";
                if (!gtk_xtext_set_font (xtext, (char *)font_name))
                {
                        mg_show_font_error (wid);
                        return;
                }
        }

        gtk_xtext_refresh (xtext);
}

void
mg_update_xtext (GtkWidget *wid)
{
        mg_update_xtext_internal (wid, TRUE);
}

void
mg_update_xtext_for_setup (GtkWidget *wid, gboolean update_font)
{
        mg_update_xtext_internal (wid, update_font);
}

void
mg_update_scroll_to_bottom_button (session_gui *gui)
{
        GtkAdjustment *adj;

        if (!gui || !GTK_IS_WIDGET (gui->scroll_bottom_button) ||
                !GTK_IS_SCROLLABLE (gui->xtext))
                return;

        adj = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (gui->xtext));
        if (!prefs.hex_gui_scroll_bottom_button ||
                gtk_xtext_is_at_bottom (GTK_XTEXT (gui->xtext)) ||
                fabulor_gtk_adjustment_is_at_end (adj, 0.5))
                gtk_widget_hide (gui->scroll_bottom_button);
        else
                gtk_widget_show (gui->scroll_bottom_button);
}

static void
mg_scroll_to_bottom_adjustment_changed (GtkAdjustment *adj, gpointer userdata)
{
        GtkWidget *button = userdata;
        session_gui *gui;

        if (!GTK_IS_WIDGET (button))
                return;

        gui = g_object_get_data (G_OBJECT (button), "mg-session-gui");

        mg_update_scroll_to_bottom_button (gui);
}

static void
mg_scroll_to_bottom_activate (session_gui *gui)
{
        if (!gui || !GTK_IS_XTEXT (gui->xtext))
                return;
	gtk_xtext_scroll_to_bottom (GTK_XTEXT (gui->xtext));
	mg_update_scroll_to_bottom_button (gui);
}

static void
mg_scroll_to_bottom_clicked (GtkButton *button, gpointer user_data)
{
	(void) button;
	mg_scroll_to_bottom_activate (user_data);
}

static void
mg_create_scroll_to_bottom_button (session_gui *gui, GtkOverlay *overlay)
{
	GtkAdjustment *adj;
	GtkWidget *icon;
	const char *label = _("Scroll to bottom");

	if (!gui || !overlay || !GTK_IS_SCROLLABLE (gui->xtext))
		return;

	gui->scroll_bottom_button = gtk_button_new ();
	icon = fabulor_gtk_chevron_down_new (24, 18);
	gtk_button_set_child (GTK_BUTTON (gui->scroll_bottom_button), icon);
	g_object_set_data (G_OBJECT (gui->scroll_bottom_button), "mg-session-gui", gui);
	gtk_widget_set_size_request (gui->scroll_bottom_button, 34, 30);
	fabulor_gtk_button_set_flat (GTK_BUTTON (gui->scroll_bottom_button));
	gtk_widget_set_tooltip_text (gui->scroll_bottom_button, label);
	fabulor_gtk_widget_set_accessible_label (gui->scroll_bottom_button, label);
        gtk_widget_set_halign (gui->scroll_bottom_button, GTK_ALIGN_END);
        gtk_widget_set_valign (gui->scroll_bottom_button, GTK_ALIGN_END);
        gtk_widget_set_margin_end (gui->scroll_bottom_button, 22);
        gtk_widget_set_margin_bottom (gui->scroll_bottom_button, 12);
        fabulor_gtk_widget_add_css_class (gui->scroll_bottom_button,
                                          "zoitechat-scroll-bottom-button");
        gtk_overlay_add_overlay (overlay, gui->scroll_bottom_button);
        gtk_overlay_set_clip_overlay (overlay, gui->scroll_bottom_button, FALSE);

	g_signal_connect (G_OBJECT (gui->scroll_bottom_button), "clicked",
	                  G_CALLBACK (mg_scroll_to_bottom_clicked), gui);

        adj = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (gui->xtext));
        g_signal_connect_object (G_OBJECT (adj), "value-changed",
                                 G_CALLBACK (mg_scroll_to_bottom_adjustment_changed), gui->scroll_bottom_button, 0);
        g_signal_connect_object (G_OBJECT (adj), "changed",
                                 G_CALLBACK (mg_scroll_to_bottom_adjustment_changed), gui->scroll_bottom_button, 0);

        fabulor_gtk_widget_reveal_tree (gui->scroll_bottom_button);
        mg_update_scroll_to_bottom_button (gui);
}

static void
mg_create_textarea (session *sess, GtkWidget *box)
{
        GtkWidget *inbox, *vbox, *frame, *overlay;
        GtkXText *xtext;
        XTextColor xtext_palette[XTEXT_COLS];
        session_gui *gui = sess->gui;

        vbox = mg_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 0);
        fabulor_gtk_box_append (GTK_BOX (box), vbox, TRUE, TRUE, 0);
        inbox = mg_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 2);
        fabulor_gtk_box_append (GTK_BOX (vbox), inbox, TRUE, TRUE, 0);

        overlay = gtk_overlay_new ();
        gtk_widget_set_hexpand (overlay, TRUE);
        gtk_widget_set_vexpand (overlay, TRUE);
        fabulor_gtk_box_append (GTK_BOX (inbox), overlay, TRUE, TRUE, 0);

        frame = gtk_scrolled_window_new ();
        gtk_widget_set_hexpand (frame, TRUE);
        gtk_widget_set_vexpand (frame, TRUE);
        fabulor_gtk_scrolled_window_set_framed (
                GTK_SCROLLED_WINDOW (frame), TRUE);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (frame),
                                        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
        fabulor_gtk_overlay_set_child (GTK_OVERLAY (overlay), frame);

        theme_get_xtext_colors_for_widget (frame, xtext_palette, XTEXT_COLS);
        gui->xtext = gtk_xtext_new (xtext_palette, TRUE);
        xtext = GTK_XTEXT (gui->xtext);
        gtk_xtext_set_max_indent (xtext, prefs.hex_text_max_indent);
        gtk_xtext_set_thin_separator (xtext, prefs.hex_text_thin_sep);
        gtk_xtext_set_urlcheck_function (xtext, mg_word_check);
        gtk_xtext_set_max_lines (xtext, prefs.hex_text_max_lines);
        fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (frame), GTK_WIDGET (xtext));

        mg_update_xtext (GTK_WIDGET (xtext));

        g_signal_connect (G_OBJECT (xtext), "word_click",
                                                        G_CALLBACK (mg_word_clicked), NULL);

        gui->vscrollbar = gtk_scrolled_window_get_vscrollbar (GTK_SCROLLED_WINDOW (frame));
        mg_create_scroll_to_bottom_button (gui, GTK_OVERLAY (overlay));

        fabulor_gtk_widget_enable_internal_drop_target (gui->vscrollbar,
                FABULOR_GTK_INTERNAL_DRAG_ACCEPT (FABULOR_GTK_INTERNAL_DRAG_CHANNEL_VIEW) |
                FABULOR_GTK_INTERNAL_DRAG_ACCEPT (FABULOR_GTK_INTERNAL_DRAG_USER_LIST),
                mg_internal_drag_motion, NULL, mg_internal_drag_drop,
                gui->vscrollbar);

        fabulor_gtk_widget_on_file_drop (gui->xtext,
                                         GDK_ACTION_MOVE | GDK_ACTION_COPY | GDK_ACTION_LINK,
                                         mg_dialog_file_drop, NULL);
}

static GtkWidget *
mg_create_infoframe (GtkWidget *box)
{
        GtkWidget *frame, *label, *hbox;

        frame = gtk_frame_new (0);
        fabulor_gtk_frame_set_outlined (GTK_FRAME (frame));
        fabulor_gtk_box_append (GTK_BOX (box), frame, TRUE, TRUE, 0);

        hbox = mg_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
        fabulor_gtk_frame_set_child (GTK_FRAME (frame), hbox);

        label = gtk_label_new (NULL);
        fabulor_gtk_box_append (GTK_BOX (hbox), label, TRUE, TRUE, 0);

        return label;
}

static void
mg_create_meters (session_gui *gui, GtkWidget *parent_box)
{
        GtkWidget *infbox, *wid, *box;

        gui->meter_box = infbox = box = mg_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 1);
        fabulor_gtk_box_append (GTK_BOX (parent_box), box, FALSE, FALSE, 0);

        if ((prefs.hex_gui_lagometer & 2) || (prefs.hex_gui_throttlemeter & 2))
        {
                infbox = mg_box_new (GTK_ORIENTATION_HORIZONTAL, TRUE, 0);
                fabulor_gtk_box_append (GTK_BOX (box), infbox, FALSE, FALSE, 0);
        }

        if (prefs.hex_gui_lagometer & 1)
        {
                gui->lagometer = wid = gtk_progress_bar_new ();
#ifdef WIN32
                gtk_widget_set_size_request (wid, 1, 10);
#else
                gtk_widget_set_size_request (wid, 1, 8);
#endif

                wid = fabulor_gtk_content_surface_new (FALSE);
                fabulor_gtk_content_surface_set_child (wid, gui->lagometer);
                fabulor_gtk_box_append (GTK_BOX (box), wid, FALSE, FALSE, 0);
        }
        if (prefs.hex_gui_lagometer & 2)
        {
                gui->laginfo = wid = mg_create_infoframe (infbox);
                gtk_label_set_text ((GtkLabel *) wid, "Lag");
        }

        if (prefs.hex_gui_throttlemeter & 1)
        {
                gui->throttlemeter = wid = gtk_progress_bar_new ();
#ifdef WIN32
                gtk_widget_set_size_request (wid, 1, 10);
#else
                gtk_widget_set_size_request (wid, 1, 8);
#endif

                wid = fabulor_gtk_content_surface_new (FALSE);
                fabulor_gtk_content_surface_set_child (wid, gui->throttlemeter);
                fabulor_gtk_box_append (GTK_BOX (box), wid, FALSE, FALSE, 0);
        }
        if (prefs.hex_gui_throttlemeter & 2)
        {
                gui->throttleinfo = wid = mg_create_infoframe (infbox);
                gtk_label_set_text ((GtkLabel *) wid, "Throttle");
        }
}

void
mg_update_meters (session_gui *gui)
{
        fabulor_gtk_box_remove_child (GTK_BOX (gui->button_box_parent), gui->meter_box);
        gui->lagometer = NULL;
        gui->laginfo = NULL;
        gui->throttlemeter = NULL;
        gui->throttleinfo = NULL;

        mg_create_meters (gui, gui->button_box_parent);
        fabulor_gtk_widget_reveal_tree (gui->meter_box);
}

static void
mg_theme_apply_userlist_style (session_gui *gui)
{
	const PangoFontDescription *font = NULL;

	if (!gui || !gui->user_tree)
		return;

	if (input_style)
		font = input_style->font_desc;

	theme_manager_apply_userlist_style (gui->user_tree,
			theme_manager_get_userlist_palette_behavior (font));
}

static void
mg_theme_userlist_changed (const ThemeChangedEvent *event, gpointer userdata)
{
	session_gui *gui = userdata;

	if (!theme_changed_event_has_reason (event, THEME_CHANGED_REASON_USERLIST) &&
	    !theme_changed_event_has_reason (event, THEME_CHANGED_REASON_PALETTE) &&
	    !theme_changed_event_has_reason (event, THEME_CHANGED_REASON_WIDGET_STYLE) &&
	    !theme_changed_event_has_reason (event, THEME_CHANGED_REASON_MODE) &&
	    !theme_changed_event_has_reason (event, THEME_CHANGED_REASON_THEME_PACK))
		return;

	mg_theme_apply_userlist_style (gui);
}

static void
mg_theme_refresh_menu_widget (GtkWidget *widget)
{
	GtkRequisition minimum;
	GtkRequisition natural;

	if (!widget)
		return;

	gtk_widget_queue_resize (widget);
	gtk_widget_get_preferred_size (widget, &minimum, &natural);
}

static void
mg_theme_refresh_menu_tree (GtkWidget *menu)
{

	if (!menu || !GTK_IS_WIDGET (menu))
		return;

	mg_theme_refresh_menu_widget (menu);
}

static void
mg_theme_window_changed (const ThemeChangedEvent *event, gpointer userdata)
{
	session_gui *gui = userdata;

	if (!theme_changed_event_has_reason (event, THEME_CHANGED_REASON_MODE) &&
	    !theme_changed_event_has_reason (event, THEME_CHANGED_REASON_THEME_PACK) &&
	    !theme_changed_event_has_reason (event, THEME_CHANGED_REASON_WIDGET_STYLE))
		return;

	if (!gui)
		return;

	theme_manager_apply_to_window (gui->window);
	mg_theme_refresh_menu_tree (gui->menu);
}

static void
mg_theme_userlist_cleanup (session_gui *gui)
{
	if (!gui)
		return;
	if (gui->theme_userlist_listener_id)
	{
		theme_listener_unregister (gui->theme_userlist_listener_id);
		gui->theme_userlist_listener_id = 0;
	}
}

static void
mg_theme_userlist_finalized_cb (gpointer userdata, GObject *widget)
{
	session_gui *gui = userdata;

	if ((gpointer) gui->user_tree == (gpointer) widget)
		gui->user_tree = NULL;
	mg_theme_userlist_cleanup (gui);
}

static void
mg_theme_userlist_lifecycle_connect (session_gui *gui)
{
	if (!gui || !gui->user_tree)
		return;
	g_object_weak_ref (G_OBJECT (gui->user_tree),
		mg_theme_userlist_finalized_cb, gui);
}

static void
mg_theme_userlist_lifecycle_disconnect (session_gui *gui)
{
	if (!gui || !gui->user_tree)
		return;
	g_object_weak_unref (G_OBJECT (gui->user_tree),
		mg_theme_userlist_finalized_cb, gui);
	gui->user_tree = NULL;
	mg_theme_userlist_cleanup (gui);
}

static void
mg_theme_window_cleanup (GtkWidget *window, session_gui *gui)
{
	if (!gui)
		return;
	mg_theme_userlist_lifecycle_disconnect (gui);
	if (window)
		theme_manager_detach_window (window);
	if (gui->theme_window_listener_id)
	{
		theme_listener_unregister (gui->theme_window_listener_id);
		gui->theme_window_listener_id = 0;
	}
}

static void
mg_topwindow_finalized_cb (gpointer userdata, GObject *window)
{
	session *sess = userdata;

	(void) window;
	mg_theme_window_cleanup (NULL, sess->gui);
	session_free (sess);
}

static void
mg_tabwindow_finalized_cb (gpointer userdata, GObject *window)
{
	session_gui *gui = userdata;

	(void) window;
	mg_theme_window_cleanup (NULL, gui);
	mg_tabwindow_kill_cb (NULL, NULL);
}

static void
mg_topwindow_lifecycle_connect (GtkWidget *window, session *sess)
{
	g_object_weak_ref (G_OBJECT (window), mg_topwindow_finalized_cb, sess);
}

static void
mg_topwindow_lifecycle_disconnect (GtkWidget *window, session *sess)
{
	g_object_weak_unref (G_OBJECT (window), mg_topwindow_finalized_cb, sess);
}

static void
mg_tabwindow_lifecycle_connect (GtkWidget *window, session_gui *gui)
{
	g_object_weak_ref (G_OBJECT (window), mg_tabwindow_finalized_cb, gui);
}

static void
mg_tabwindow_lifecycle_disconnect (GtkWidget *window, session_gui *gui)
{
	g_object_weak_unref (G_OBJECT (window), mg_tabwindow_finalized_cb, gui);
}

static void
mg_create_userlist (session_gui *gui, GtkWidget *box)
{
        GtkWidget *ulist, *vbox;

        vbox = mg_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 1);
        fabulor_gtk_box_append (GTK_BOX (box), vbox, TRUE, TRUE, 0);

        gui->namelistinfo = gtk_label_new (NULL);
        gtk_label_set_xalign (GTK_LABEL (gui->namelistinfo), 0.5f);
        gtk_label_set_justify (GTK_LABEL (gui->namelistinfo), GTK_JUSTIFY_CENTER);
        gtk_label_set_ellipsize (GTK_LABEL (gui->namelistinfo), PANGO_ELLIPSIZE_END);
        gtk_label_set_width_chars (GTK_LABEL (gui->namelistinfo), 1);
        gtk_widget_set_margin_start (gui->namelistinfo, 0);
        gtk_widget_set_margin_end (gui->namelistinfo, 0);
        gtk_widget_set_hexpand (gui->namelistinfo, TRUE);
        gtk_widget_set_halign (gui->namelistinfo, GTK_ALIGN_FILL);
        if (prefs.hex_gui_ulist_count)
                fabulor_gtk_box_append (GTK_BOX (vbox), gui->namelistinfo, FALSE, FALSE, 0);

        gui->user_tree = ulist = userlist_create (GTK_BOX (vbox));

        if (!gui->theme_userlist_listener_id)
                gui->theme_userlist_listener_id = theme_listener_register ("maingui.userlist", mg_theme_userlist_changed, gui);
        mg_theme_userlist_lifecycle_connect (gui);
        mg_theme_apply_userlist_style (gui);

        gui->button_box_parent = vbox;
        gui->button_box = mg_create_userlistbuttons (vbox);
        mg_create_meters (gui, vbox);
}

static void
mg_vpane_cb (GtkPaned *pane, GParamSpec *param, session_gui *gui)
{
        prefs.hex_gui_pane_divider_position = gtk_paned_get_position (pane);
}

static void
mg_leftpane_cb (GtkPaned *pane, GParamSpec *param, session_gui *gui)
{
        prefs.hex_gui_pane_left_size = gtk_paned_get_position (pane);
}

static void
mg_rightpane_cb (GtkPaned *pane, GParamSpec *param, session_gui *gui)
{
        int handle_size;
        int pane_width;
        int right_size;

        (void) param;
        if (gui->pane_right_restoring || !gui->user_box ||
                !gtk_widget_get_visible (gui->user_box) ||
                fabulor_gtk_widget_get_allocated_width (gui->user_box) < 1)
                return;
        handle_size = fabulor_gtk_paned_get_handle_size (pane);
        /* record the position from the RIGHT side */
        pane_width = fabulor_gtk_widget_get_allocated_width (GTK_WIDGET (pane));
        right_size = pane_width - gtk_paned_get_position (pane) - handle_size;
        prefs.hex_gui_pane_right_size = fabulor_pane_clamp_end_size (right_size,
                prefs.hex_gui_pane_right_size_min, pane_width, handle_size);
}

static void
mg_restore_rightpane (GtkPaned *pane, int pane_width, gpointer data)
{
        int fallback_size;
        int handle_size;
        int saved_size;
        /* use the value captured at connect time, since notify::position may
         * have already overwritten prefs.hex_gui_pane_right_size during layout */
        saved_size = GPOINTER_TO_INT (data);
        handle_size = fabulor_gtk_paned_get_handle_size (pane);
        fallback_size = MAX (prefs.hex_gui_ulist_nick_width,
                prefs.hex_gui_pane_right_size_min);
        saved_size = fabulor_pane_restore_end_size (saved_size, fallback_size,
                prefs.hex_gui_pane_right_size_min, pane_width, handle_size);
        if (saved_size < 1)
                return;
        prefs.hex_gui_pane_right_size = saved_size;
        gtk_paned_set_position (pane, pane_width - saved_size - handle_size);
}

static gboolean
mg_restore_rightpane_tick_cb (GtkWidget *widget, GdkFrameClock *frame_clock,
                              gpointer data)
{
        session_gui *gui = data;
        GtkPaned *pane = GTK_PANED (widget);
        GtkWidget *end_child = fabulor_gtk_paned_get_end_child (pane);
        int actual_size;
        int desired_size;
        int handle_size;
        int pane_width = fabulor_gtk_widget_get_allocated_width (widget);

        (void) frame_clock;
        if (!gtk_widget_get_mapped (widget) || pane_width < 1 || !end_child ||
                !gui->user_box || !gtk_widget_get_visible (gui->user_box) ||
                fabulor_gtk_widget_get_allocated_width (gui->user_box) < 1)
                return G_SOURCE_CONTINUE;

        mg_restore_rightpane (pane, pane_width,
                GINT_TO_POINTER (gui->pane_right_size));

        handle_size = fabulor_gtk_paned_get_handle_size (pane);
        desired_size = fabulor_pane_restore_end_size (gui->pane_right_size,
                MAX (prefs.hex_gui_ulist_nick_width,
                        prefs.hex_gui_pane_right_size_min),
                prefs.hex_gui_pane_right_size_min, pane_width, handle_size);
        gui->pane_right_size = desired_size;
        actual_size = pane_width - gtk_paned_get_position (pane) - handle_size;
        if (gui->pane_right_last_width != pane_width ||
                ABS (actual_size - desired_size) > 1)
        {
                gui->pane_right_last_width = pane_width;
                gui->pane_right_stable_frames = 0;
                return G_SOURCE_CONTINUE;
        }

        gui->pane_right_stable_frames++;
        if (gui->pane_right_stable_frames < 3)
                return G_SOURCE_CONTINUE;
        gui->pane_right_restoring = 0;
        gui->pane_right_restore_tick_id = 0;
        return G_SOURCE_REMOVE;
}

static void
mg_schedule_rightpane_restore (session_gui *gui)
{
        if (!gui || !GTK_IS_WIDGET (gui->hpane_right))
                return;

        gui->pane_right_size = prefs.hex_gui_pane_right_size;
        gui->pane_right_restoring = 1;
        gui->pane_right_last_width = 0;
        gui->pane_right_stable_frames = 0;
        if (!gui->pane_right_restore_tick_id)
                gui->pane_right_restore_tick_id =
                        gtk_widget_add_tick_callback (gui->hpane_right,
                                mg_restore_rightpane_tick_cb, gui, NULL);
}

static gboolean
mg_add_pane_signals (session_gui *gui)
{
        g_signal_connect (G_OBJECT (gui->hpane_right), "notify::position",
                                                        G_CALLBACK (mg_rightpane_cb), gui);
        g_signal_connect (G_OBJECT (gui->hpane_left), "notify::position",
                                                        G_CALLBACK (mg_leftpane_cb), gui);
        g_signal_connect (G_OBJECT (gui->vpane_left), "notify::position",
                                                        G_CALLBACK (mg_vpane_cb), gui);
        g_signal_connect (G_OBJECT (gui->vpane_right), "notify::position",
                                                        G_CALLBACK (mg_vpane_cb), gui);
        return FALSE;
}

static void
mg_create_center (session *sess, session_gui *gui, GtkWidget *box)
{
        GtkWidget *vbox, *hbox, *book;

        /* sep between top and bottom of left side */
        gui->vpane_left = gtk_paned_new (GTK_ORIENTATION_VERTICAL);

        /* sep between top and bottom of right side */
        gui->vpane_right = gtk_paned_new (GTK_ORIENTATION_VERTICAL);

	/* sep between left and xtext */
	gui->hpane_left = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_paned_set_wide_handle (GTK_PANED (gui->hpane_left), FALSE);
	gtk_paned_set_position (GTK_PANED (gui->hpane_left), prefs.hex_gui_pane_left_size);

	/* sep between xtext and right side */
	gui->hpane_right = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);

	/* Restore after the first complete allocation. Capture the saved size before
	 * notify::position can overwrite it during initial layout. */
        mg_schedule_rightpane_restore (gui);

        if (prefs.hex_gui_win_swap)
        {
                fabulor_gtk_paned_set_end_child (GTK_PANED (gui->hpane_left),
                        gui->vpane_left, FALSE, FALSE);
                fabulor_gtk_paned_set_start_child (GTK_PANED (gui->hpane_left),
                        gui->hpane_right, TRUE, TRUE);
        }
        else
        {
                fabulor_gtk_paned_set_start_child (GTK_PANED (gui->hpane_left),
                        gui->vpane_left, FALSE, FALSE);
                fabulor_gtk_paned_set_end_child (GTK_PANED (gui->hpane_left),
                        gui->hpane_right, TRUE, TRUE);
        }
        fabulor_gtk_paned_set_end_child (GTK_PANED (gui->hpane_right),
                gui->vpane_right, prefs.hex_gui_ulist_resizable, TRUE);

        fabulor_gtk_box_append (GTK_BOX (box), gui->hpane_left, TRUE, TRUE, 0);

        gui->note_book = book = gtk_notebook_new ();
        gtk_notebook_set_show_tabs (GTK_NOTEBOOK (book), FALSE);
        gtk_notebook_set_show_border (GTK_NOTEBOOK (book), FALSE);
        fabulor_gtk_paned_set_start_child (GTK_PANED (gui->hpane_right),
                book, TRUE, TRUE);

        hbox = mg_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
        gtk_widget_set_size_request (hbox,
                MAX (prefs.hex_gui_pane_right_size_min, 1), -1);
        fabulor_gtk_paned_set_start_child (GTK_PANED (gui->vpane_right),
                hbox, FALSE, TRUE);
        mg_create_userlist (gui, hbox);

        gui->user_box = hbox;

        vbox = mg_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 3);
        gtk_notebook_append_page (GTK_NOTEBOOK (book), vbox, NULL);
        mg_create_topicbar (sess, vbox);

        if (prefs.hex_gui_search_pos)
        {
                mg_create_search (sess, vbox);
                mg_create_textarea (sess, vbox);
        }
        else
        {
                mg_create_textarea (sess, vbox);
                mg_create_search (sess, vbox);
        }

        mg_create_entry (sess, vbox);

        mg_add_pane_signals (gui);
}

static void
mg_change_nick (int cancel, char *text, gpointer userdata)
{
        char buf[256];

        if (!cancel)
        {
                g_snprintf (buf, sizeof (buf), "nick %s", text);
                handle_command (current_sess, buf, FALSE);
        }
}

static void
mg_nickclick_cb (GtkWidget *button, gpointer userdata)
{
        fe_get_str (_("Enter new nickname:"), current_sess->server->nick,
                                        mg_change_nick, (void *) 1);
}

/* make sure chanview and userlist positions are sane */

static void
mg_sanitize_positions (int *cv, int *ul)
{
        if (prefs.hex_gui_tab_layout == 2)
        {
                /* treeview can't be on TOP or BOTTOM */
                if (*cv == POS_TOP || *cv == POS_BOTTOM)
                        *cv = POS_TOPLEFT;
        }

        /* userlist can't be on TOP or BOTTOM */
        if (*ul == POS_TOP || *ul == POS_BOTTOM)
                *ul = POS_TOPRIGHT;

        /* can't have both in the same place */
        if (*cv == *ul)
        {
                *cv = POS_TOPRIGHT;
                if (*ul == POS_TOPRIGHT)
                        *cv = POS_BOTTOMRIGHT;
        }
}

static void
mg_place_userlist_and_chanview_real (session_gui *gui, GtkWidget *userlist, GtkWidget *chanview)
{
        gboolean unref_userlist = FALSE;
        gboolean unref_chanview = FALSE;

        /* first, remove userlist/treeview from their containers */
        if (userlist)
                unref_userlist =
                        fabulor_gtk_layout_retain_and_detach_child (userlist);

        if (chanview)
                unref_chanview =
                        fabulor_gtk_layout_retain_and_detach_child (chanview);

        if (chanview)
        {
                /* incase the previous pos was POS_HIDDEN */
                gtk_widget_show (chanview);

                gtk_widget_set_margin_top (chanview, 0);
                gtk_widget_set_margin_bottom (chanview, 0);

                /* then place them back in their new positions */
                switch (prefs.hex_gui_tab_pos)
                {
                case POS_TOPLEFT:
                        fabulor_gtk_paned_set_start_child (
                                GTK_PANED (gui->vpane_left), chanview, FALSE, TRUE);
                        break;
                case POS_BOTTOMLEFT:
                        fabulor_gtk_paned_set_end_child (
                                GTK_PANED (gui->vpane_left), chanview, FALSE, TRUE);
                        break;
                case POS_TOPRIGHT:
                        fabulor_gtk_paned_set_start_child (
                                GTK_PANED (gui->vpane_right), chanview, FALSE, TRUE);
                        break;
                case POS_BOTTOMRIGHT:
                        fabulor_gtk_paned_set_end_child (
                                GTK_PANED (gui->vpane_right), chanview, FALSE, TRUE);
                        break;
                case POS_TOP:
                        gtk_widget_set_margin_bottom (chanview, GUI_SPACING - 1);
                        gtk_widget_set_hexpand (chanview, FALSE);
                        gtk_widget_set_vexpand (chanview, FALSE);
                        gtk_widget_set_halign (chanview, GTK_ALIGN_FILL);
                        gtk_widget_set_valign (chanview, GTK_ALIGN_FILL);
                        gtk_grid_attach (GTK_GRID (gui->main_table), chanview,
                                                                        1, 1, 1, 1);
                        break;
                case POS_HIDDEN:
                        gtk_widget_hide (chanview);
                        /* always attach it to something to avoid ref_count=0 */
                        if (prefs.hex_gui_ulist_pos == POS_TOP)
                        {
                                gtk_widget_set_hexpand (chanview, FALSE);
                                gtk_widget_set_vexpand (chanview, FALSE);
                                gtk_widget_set_halign (chanview, GTK_ALIGN_FILL);
                                gtk_widget_set_valign (chanview, GTK_ALIGN_FILL);
                                gtk_grid_attach (GTK_GRID (gui->main_table), chanview,
                                                                                1, 3, 1, 1);
                        }

                        else
                        {
                                gtk_widget_set_hexpand (chanview, FALSE);
                                gtk_widget_set_vexpand (chanview, FALSE);
                                gtk_widget_set_halign (chanview, GTK_ALIGN_FILL);
                                gtk_widget_set_valign (chanview, GTK_ALIGN_FILL);
                                gtk_grid_attach (GTK_GRID (gui->main_table), chanview,
                                                                                1, 1, 1, 1);
                        }
                        break;
                default:/* POS_BOTTOM */
                        gtk_widget_set_margin_top (chanview, 3);
                        gtk_widget_set_hexpand (chanview, FALSE);
                        gtk_widget_set_vexpand (chanview, FALSE);
                        gtk_widget_set_halign (chanview, GTK_ALIGN_FILL);
                        gtk_widget_set_valign (chanview, GTK_ALIGN_FILL);
                        gtk_grid_attach (GTK_GRID (gui->main_table), chanview,
                                                                        1, 3, 1, 1);
                }
        }

        if (userlist)
        {
                switch (prefs.hex_gui_ulist_pos)
                {
                case POS_TOPLEFT:
                        fabulor_gtk_paned_set_start_child (
                                GTK_PANED (gui->vpane_left), userlist, FALSE, TRUE);
                        break;
                case POS_BOTTOMLEFT:
                        fabulor_gtk_paned_set_end_child (
                                GTK_PANED (gui->vpane_left), userlist, FALSE, TRUE);
                        break;
                case POS_BOTTOMRIGHT:
                        fabulor_gtk_paned_set_end_child (
                                GTK_PANED (gui->vpane_right), userlist, FALSE, TRUE);
                        break;
                /*case POS_HIDDEN:
                        break;*/        /* Hide using the VIEW menu instead */
                default:/* POS_TOPRIGHT */
                        fabulor_gtk_paned_set_start_child (
                                GTK_PANED (gui->vpane_right), userlist, FALSE, TRUE);
                }
        }

        if (mg_is_userlist_and_tree_combined () && prefs.hex_gui_pane_divider_position != 0)
        {
                gtk_paned_set_position (GTK_PANED (gui->vpane_left), prefs.hex_gui_pane_divider_position);
                gtk_paned_set_position (GTK_PANED (gui->vpane_right), prefs.hex_gui_pane_divider_position);
        }

        if (unref_chanview)
                g_object_unref (chanview);
        if (unref_userlist)
                g_object_unref (userlist);

        mg_hide_empty_boxes (gui);
}

static void
mg_place_userlist_and_chanview (session_gui *gui)
{
        GtkOrientation orientation;
        GtkWidget *chanviewbox = NULL;
        gboolean restore_right_pane;
        int saved_right_size;
        int pos;

        restore_right_pane =
                gtk_widget_get_mapped (gui->hpane_right) &&
                fabulor_gtk_widget_get_allocated_width (gui->hpane_right) > 0;
        saved_right_size = prefs.hex_gui_pane_right_size;
        if (restore_right_pane)
                gui->pane_right_restoring = 1;

        mg_sanitize_positions (&prefs.hex_gui_tab_pos, &prefs.hex_gui_ulist_pos);

        if (gui->chanview)
        {
                pos = prefs.hex_gui_tab_pos;

                orientation = chanview_get_orientation (gui->chanview);
                if ((pos == POS_BOTTOM || pos == POS_TOP) && orientation == GTK_ORIENTATION_VERTICAL)
                        chanview_set_orientation (gui->chanview, FALSE);
                else if ((pos == POS_TOPLEFT || pos == POS_BOTTOMLEFT || pos == POS_TOPRIGHT || pos == POS_BOTTOMRIGHT) && orientation == GTK_ORIENTATION_HORIZONTAL)
                        chanview_set_orientation (gui->chanview, TRUE);
                chanviewbox = chanview_get_box (gui->chanview);
        }

        mg_place_userlist_and_chanview_real (gui, gui->user_box, chanviewbox);

        if (restore_right_pane)
        {
                prefs.hex_gui_pane_right_size = saved_right_size;
                mg_schedule_rightpane_restore (gui);
        }
}

void
mg_change_layout (int type)
{
        if (mg_gui)
        {
                /* put tabs at the bottom */
                if (type == 0 && prefs.hex_gui_tab_pos != POS_BOTTOM && prefs.hex_gui_tab_pos != POS_TOP)
                        prefs.hex_gui_tab_pos = POS_BOTTOM;

                mg_place_userlist_and_chanview (mg_gui);
                chanview_set_impl (mg_gui->chanview, type);
        }
}


typedef struct
{
        GtkEntry *entry;
        GtkWidget *popover;
        char *sequence;
} EmojiFlagInsert;

typedef struct
{
        GtkEntry *entry;
        GtkWidget *popover;
        FabulorEmojiPickerPage *page;
} EmojiPickerPage;

#define MG_EMOJI_PAGE_DATA "fabulor-emoji-page"
#define MG_EMOJI_BUTTON_WIDTH 52
#define MG_EMOJI_BUTTON_HEIGHT 48
#define MG_EMOJI_FLAG_WIDTH 42
#define MG_EMOJI_FLAG_HEIGHT 28
#define MG_EMOJI_FLAG_SOURCE_WIDTH (MG_EMOJI_FLAG_WIDTH * 2)
#define MG_EMOJI_FLAG_SOURCE_HEIGHT (MG_EMOJI_FLAG_HEIGHT * 2)

static const char *mg_emoji_flag_codes[] = {
        "AC", "AD", "AE", "AF", "AG", "AI", "AL", "AM", "AO", "AQ", "AR", "AS",
        "AT", "AU", "AW", "AX", "AZ", "BA", "BB", "BD", "BE", "BF", "BG", "BH",
        "BI", "BJ", "BL", "BM", "BN", "BO", "BQ", "BR", "BS", "BT", "BV", "BW",
        "BY", "BZ", "CA", "CC", "CD", "CF", "CG", "CH", "CI", "CK", "CL", "CM",
        "CN", "CO", "CR", "CU", "CV", "CW", "CX", "CY", "CZ", "DE", "DJ", "DK",
        "DM", "DO", "DZ", "EC", "EE", "EG", "EH", "ER", "ES",
        "ET", "EU", "FI", "FJ", "FK", "FM", "FO", "FR", "GA", "GB", "GD", "GE",
        "GF", "GG", "GH", "GI", "GL", "GM", "GN", "GP", "GQ", "GR", "GS", "GT",
        "GU", "GW", "GY", "HK", "HM", "HN", "HR", "HT", "HU", "ID", "IE",
        "IL", "IM", "IN", "IO", "IQ", "IR", "IS", "IT", "JE", "JM", "JO", "JP",
        "KE", "KG", "KH", "KI", "KM", "KN", "KP", "KR", "KW", "KY", "KZ", "LA",
        "LB", "LC", "LI", "LK", "LR", "LS", "LT", "LU", "LV", "LY", "MA", "MC",
        "MD", "ME", "MF", "MG", "MH", "MK", "ML", "MM", "MN", "MO", "MP", "MQ",
        "MR", "MS", "MT", "MU", "MV", "MW", "MX", "MY", "MZ", "NA", "NC", "NE",
        "NF", "NG", "NI", "NL", "NO", "NP", "NR", "NU", "NZ", "OM", "PA", "PE",
        "PF", "PG", "PH", "PK", "PL", "PM", "PN", "PR", "PS", "PT", "PW", "PY",
        "QA", "RE", "RO", "RS", "RU", "RW", "SA", "SB", "SC", "SD", "SE", "SG",
        "SH", "SI", "SJ", "SK", "SL", "SM", "SN", "SO", "SR", "SS", "ST", "SV",
        "SX", "SY", "SZ", "TC", "TD", "TF", "TG", "TH", "TJ", "TK", "TL",
        "TM", "TN", "TO", "TR", "TT", "TV", "TW", "TZ", "UA", "UG", "UM", "UN",
        "US", "UY", "UZ", "VA", "VC", "VE", "VG", "VI", "VN", "VU", "WF", "WS",
        "XK", "YE", "YT", "ZA", "ZM", "ZW", NULL
};

static const gunichar mg_emoji_smileys[] = {
        0x1F600, 0x1F603, 0x1F604, 0x1F601, 0x1F606, 0x1F605, 0x1F923, 0x1F602,
        0x1F642, 0x1F643, 0x1F609, 0x1F60A, 0x1F607, 0x1F970, 0x1F60D, 0x1F929,
        0x1F618, 0x1F617, 0x1F61A, 0x1F619, 0x1F60B, 0x1F61B, 0x1F61C, 0x1F92A,
        0x1F61D, 0x1F911, 0x1F917, 0x1F92D, 0x1F92B, 0x1F914, 0x1F910, 0x1F928,
        0x1F610, 0x1F611, 0x1F636, 0x1F60F, 0x1F612, 0x1F644, 0x1F62C, 0x1F925,
        0x1F60C, 0x1F614, 0x1F62A, 0x1F924, 0x1F634, 0x1F637, 0x1F912, 0x1F915,
        0x1F922, 0x1F92E, 0x1F927, 0x1F975, 0x1F976, 0x1F974, 0x1F635, 0x1F92F,
        0x1F920, 0x1F973, 0x1F978, 0x1F60E, 0x1F913, 0x1F9D0, 0
};

static const gunichar mg_emoji_people[] = {
        0x1F44B, 0x1F91A, 0x1F590, 0x270B, 0x1F596, 0x1F44C, 0x1F90C, 0x1F90F,
        0x270C, 0x1F91E, 0x1F91F, 0x1F918, 0x1F919, 0x1F448, 0x1F449, 0x1F446,
        0x1F595, 0x1F447, 0x261D, 0x1F44D, 0x1F44E, 0x270A, 0x1F44A, 0x1F44F,
        0x1F64C, 0x1F450, 0x1F932, 0x1F64F, 0x1F4AA, 0x1F9BE, 0x1F9BF, 0x1F9B5,
        0x1F9B6, 0x1F442, 0x1F9BB, 0x1F443, 0x1F9E0, 0x1FAC0, 0x1FAC1, 0x1F9B7,
        0x1F9B4, 0x1F440, 0x1F445, 0x1F444, 0x1F476, 0x1F9D2, 0x1F466, 0x1F467,
        0x1F9D1, 0x1F471, 0x1F468, 0x1F469, 0x1F9D4, 0x1F474, 0x1F475, 0x1F64D,
        0x1F64E, 0x1F645, 0x1F646, 0x1F481, 0x1F64B, 0
};

static const gunichar mg_emoji_animals[] = {
        0x1F436, 0x1F431, 0x1F42D, 0x1F439, 0x1F430, 0x1F98A, 0x1F43B, 0x1F43C,
        0x1F43B, 0x1F428, 0x1F42F, 0x1F981, 0x1F42E, 0x1F437, 0x1F43D, 0x1F438,
        0x1F435, 0x1F648, 0x1F649, 0x1F64A, 0x1F412, 0x1F414, 0x1F427, 0x1F426,
        0x1F424, 0x1F986, 0x1F985, 0x1F989, 0x1F987, 0x1F43A, 0x1F417, 0x1F434,
        0x1F984, 0x1F41D, 0x1FAB1, 0x1F41B, 0x1F98B, 0x1F40C, 0x1F41E, 0x1F41C,
        0x1FAB0, 0x1F422, 0x1F40D, 0x1F98E, 0x1F419, 0x1F991, 0x1F980, 0x1F99E,
        0x1F420, 0x1F41F, 0x1F42C, 0x1F433, 0x1F40B, 0x1F98D, 0x1F405, 0x1F406,
        0x1F993, 0x1F98C, 0x1F999, 0x1F992, 0
};

static const gunichar mg_emoji_food[] = {
        0x1F347, 0x1F348, 0x1F349, 0x1F34A, 0x1F34B, 0x1F34C, 0x1F34D, 0x1F96D,
        0x1F34E, 0x1F34F, 0x1F350, 0x1F351, 0x1F352, 0x1F353, 0x1FAD0, 0x1F95D,
        0x1F345, 0x1FAD2, 0x1F965, 0x1F951, 0x1F346, 0x1F954, 0x1F955, 0x1F33D,
        0x1F336, 0x1F952, 0x1F96C, 0x1F966, 0x1F9C4, 0x1F9C5, 0x1F344, 0x1F95C,
        0x1F330, 0x1F35E, 0x1F950, 0x1F956, 0x1FAD3, 0x1F968, 0x1F96F, 0x1F95E,
        0x1F9C7, 0x1F9C0, 0x1F356, 0x1F357, 0x1F969, 0x1F953, 0x1F354, 0x1F35F,
        0x1F355, 0x1F32D, 0x1F96A, 0x1F32E, 0x1F32F, 0x1F959, 0x1F9C6, 0x1F95A,
        0x1F373, 0x1F958, 0x1F372, 0x1F35C, 0x1F363, 0x1F364, 0x1F366, 0x1F370, 0
};

static const gunichar mg_emoji_travel[] = {
        0x1F697, 0x1F695, 0x1F699, 0x1F68C, 0x1F68E, 0x1F3CE, 0x1F693, 0x1F691,
        0x1F692, 0x1F690, 0x1F69A, 0x1F69B, 0x1F69C, 0x1F9AF, 0x1F9BD, 0x1F9BC,
        0x1F6F4, 0x1F6B2, 0x1F6F5, 0x1F3CD, 0x1F6FA, 0x1F6A8, 0x1F694, 0x1F68D,
        0x1F698, 0x1F696, 0x1F6A1, 0x1F6A0, 0x1F69F, 0x1F683, 0x1F68B, 0x1F69E,
        0x1F682, 0x1F686, 0x1F684, 0x1F685, 0x1F688, 0x1F687, 0x1F69D, 0x1F68A,
        0x1F689, 0x2708, 0x1F6EB, 0x1F6EC, 0x1FA82, 0x1F4BA, 0x1F681, 0x1F69F,
        0x1F6A0, 0x1F6F0, 0x1F680, 0x1F6F8, 0x1F6CE, 0x1F9F3, 0x231B, 0x23F0,
        0x1F30D, 0x1F30E, 0x1F30F, 0x1F5FA, 0x1F5FD, 0x1F3F0, 0
};

static const gunichar mg_emoji_objects[] = {
        0x231A, 0x1F4F1, 0x1F4F2, 0x1F4BB, 0x2328, 0x1F5A5, 0x1F5A8, 0x1F5B1,
        0x1F5B2, 0x1F579, 0x1F5DC, 0x1F4BD, 0x1F4BE, 0x1F4BF, 0x1F4C0, 0x1F4FC,
        0x1F4F7, 0x1F4F8, 0x1F4F9, 0x1F3A5, 0x1F4DE, 0x260E, 0x1F4DF, 0x1F4E0,
        0x1F4FA, 0x1F4FB, 0x1F399, 0x1F39A, 0x1F39B, 0x1F9ED, 0x23F1, 0x23F2,
        0x1F570, 0x1F4A1, 0x1F526, 0x1F56F, 0x1FA94, 0x1F9EF, 0x1F6E2, 0x1F4B8,
        0x1F4B5, 0x1F4B4, 0x1F4B6, 0x1F4B7, 0x1F4B0, 0x1F4B3, 0x1F48E, 0x2696,
        0x1FA9C, 0x1F9F0, 0x1FA9B, 0x1F527, 0x1FA9A, 0x1F528, 0x2692, 0x1F6E0,
        0x1F5E1, 0x2694, 0x1F52B, 0x1F3F9, 0x1F6E1, 0x1F9F2, 0x1F9EA, 0x1F9EC, 0
};

static const gunichar mg_emoji_symbols[] = {
        0x2764, 0x1F9E1, 0x1F49B, 0x1F49A, 0x1F499, 0x1F49C, 0x1F90E, 0x1F5A4,
        0x1F90D, 0x1F494, 0x2763, 0x1F495, 0x1F49E, 0x1F493, 0x1F497, 0x1F496,
        0x1F498, 0x1F49D, 0x1F49F, 0x262E, 0x271D, 0x262A, 0x1F549, 0x2638,
        0x2721, 0x1F52F, 0x1F54E, 0x262F, 0x2626, 0x1F6D0, 0x26CE, 0x2648,
        0x2649, 0x264A, 0x264B, 0x264C, 0x264D, 0x264E, 0x264F, 0x2650,
        0x2651, 0x2652, 0x2653, 0x1F194, 0x1F19A, 0x1F4A2, 0x1F4A5, 0x1F4AB,
        0x1F4A6, 0x1F4A8, 0x1F573, 0x1F4AC, 0x1F441, 0x1F5E8, 0x1F5EF, 0x1F4AD, 0
};

typedef struct
{
        const char *title;
        const gunichar *items;
} EmojiCategory;

static const EmojiCategory mg_emoji_categories[] = {
        { N_("Smileys"), mg_emoji_smileys },
        { N_("People"), mg_emoji_people },
        { N_("Animals"), mg_emoji_animals },
        { N_("Food"), mg_emoji_food },
        { N_("Travel"), mg_emoji_travel },
        { N_("Objects"), mg_emoji_objects },
        { N_("Symbols"), mg_emoji_symbols },
        { NULL, NULL }
};

static char *
mg_emoji_flag_sequence (const char *code)
{
        return fabulor_emoji_picker_flag_sequence (code);
}

static char *
mg_emoji_codepoint_sequence (gunichar codepoint)
{
        return fabulor_emoji_picker_codepoint_sequence (codepoint);
}

static GdkPixbuf *
mg_emoji_flag_load_pixbuf (const char *code)
{
        char *name;
        char *lower;
        char *path;
        char *base_path = NULL;
        GdkPixbuf *pixbuf;
        GError *error = NULL;

        lower = g_ascii_strdown (code, -1);
        name = g_strdup_printf ("%s.png", lower);

#ifdef G_OS_WIN32
        base_path = g_win32_get_package_installation_directory_of_module (NULL);
        if (base_path)
        {
                path = g_build_filename (base_path, "share", "emoji-flags", name, NULL);
                pixbuf = gdk_pixbuf_new_from_file_at_scale (
                                path, MG_EMOJI_FLAG_SOURCE_WIDTH,
                                MG_EMOJI_FLAG_SOURCE_HEIGHT, TRUE, &error);
                g_free (path);
                if (pixbuf)
                        goto cleanup;
                g_clear_error (&error);
        }
#endif

        path = g_build_filename ("share", "emoji-flags", name, NULL);
        pixbuf = gdk_pixbuf_new_from_file_at_scale (
                        path, MG_EMOJI_FLAG_SOURCE_WIDTH,
                        MG_EMOJI_FLAG_SOURCE_HEIGHT, TRUE, &error);

        if (!pixbuf)
        {
                g_clear_error (&error);
        }

        g_free (path);
cleanup:
        g_free (base_path);
        g_free (name);
        g_free (lower);
        return pixbuf;
}

static void
mg_emoji_flag_insert_free (EmojiFlagInsert *insert)
{
        if (!insert)
                return;

        g_free (insert->sequence);
        g_free (insert);
}

static void
mg_emoji_flag_insert_cb (GtkWidget *button, gpointer user_data)
{
        EmojiFlagInsert *insert = user_data;
        gint pos;

        if (!insert || !insert->entry || !insert->sequence)
                return;

        pos = gtk_editable_get_position (GTK_EDITABLE (insert->entry));
        gtk_editable_insert_text (GTK_EDITABLE (insert->entry), insert->sequence, -1, &pos);
        gtk_editable_set_position (GTK_EDITABLE (insert->entry), pos);
        gtk_widget_grab_focus (GTK_WIDGET (insert->entry));

        if (insert->popover)
                gtk_popover_popdown (GTK_POPOVER (insert->popover));
}

static GtkWidget *
mg_emoji_scroller_new (GtkWidget **flow_out)
{
        GtkWidget *scrolled;
        GtkWidget *flow;

        scrolled = gtk_scrolled_window_new ();
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                        GTK_POLICY_NEVER,
                                        GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_overlay_scrolling (
                                        GTK_SCROLLED_WINDOW (scrolled), FALSE);
        gtk_widget_set_hexpand (scrolled, TRUE);
        gtk_widget_set_vexpand (scrolled, TRUE);

        flow = gtk_flow_box_new ();
        gtk_flow_box_set_selection_mode (GTK_FLOW_BOX (flow),
                                         GTK_SELECTION_NONE);
        gtk_flow_box_set_homogeneous (GTK_FLOW_BOX (flow), TRUE);
        gtk_flow_box_set_row_spacing (GTK_FLOW_BOX (flow), 4);
        gtk_flow_box_set_column_spacing (GTK_FLOW_BOX (flow), 4);
        gtk_flow_box_set_min_children_per_line (GTK_FLOW_BOX (flow), 1);
        gtk_flow_box_set_max_children_per_line (GTK_FLOW_BOX (flow), 10);
        gtk_widget_set_valign (flow, GTK_ALIGN_START);
        fabulor_gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled),
                                               flow);

        *flow_out = flow;
        return scrolled;
}

static void
mg_emoji_add_button_sized (GtkWidget *flow, GtkEntry *entry,
                           GtkWidget *popover, GtkWidget *child,
                           const char *sequence, const char *tooltip,
                           int width, int height)
{
        GtkWidget *button;
        EmojiFlagInsert *insert;

        button = gtk_button_new ();
        fabulor_gtk_button_set_flat (GTK_BUTTON (button));
        gtk_widget_set_can_focus (button, FALSE);
        gtk_widget_set_size_request (button, width, height);
        if (tooltip)
                gtk_widget_set_tooltip_text (button, tooltip);
        fabulor_gtk_button_set_child (GTK_BUTTON (button), child);

        insert = g_new0 (EmojiFlagInsert, 1);
        insert->entry = entry;
        insert->popover = popover;
        insert->sequence = g_strdup (sequence);
        g_signal_connect_data (G_OBJECT (button), "clicked",
                               G_CALLBACK (mg_emoji_flag_insert_cb),
                               insert,
                               (GClosureNotify) mg_emoji_flag_insert_free,
                               0);

        gtk_flow_box_append (GTK_FLOW_BOX (flow), button);
}

static void
mg_emoji_add_button (GtkWidget *flow, GtkEntry *entry, GtkWidget *popover,
                     GtkWidget *child, const char *sequence,
                     const char *tooltip)
{
        mg_emoji_add_button_sized (flow, entry, popover, child, sequence,
                                   tooltip, MG_EMOJI_BUTTON_WIDTH,
                                   MG_EMOJI_BUTTON_HEIGHT);
}

static GtkWidget *
mg_emoji_codepoint_page_new (GtkEntry *entry, GtkWidget *popover, const gunichar *items)
{
        GtkWidget *scrolled;
        GtkWidget *flow;
        GtkWidget *label;
        char *sequence;
        int i;

        scrolled = mg_emoji_scroller_new (&flow);

        for (i = 0; items[i] != 0; i++)
        {
                PangoAttrList *attrs;

                sequence = mg_emoji_codepoint_sequence (items[i]);
                label = gtk_label_new (sequence);
                attrs = pango_attr_list_new ();
                pango_attr_list_insert (attrs, pango_attr_scale_new (PANGO_SCALE_XX_LARGE));
                gtk_label_set_attributes (GTK_LABEL (label), attrs);
                pango_attr_list_unref (attrs);
                mg_apply_emoji_fallback_widget (label);
                mg_emoji_add_button (flow, entry, popover, label, sequence,
                                     sequence);
                g_free (sequence);
        }

        return scrolled;
}

static GtkWidget *
mg_emoji_flags_page_new (GtkEntry *entry, GtkWidget *popover)
{
        GtkWidget *scrolled;
        GtkWidget *flow;
        GtkWidget *box;
        GtkWidget *image;
        GtkWidget *label;
        GdkPixbuf *pixbuf;
        char *sequence;
        char *tooltip;
        int i;

        scrolled = mg_emoji_scroller_new (&flow);

        for (i = 0; mg_emoji_flag_codes[i] != NULL; i++)
        {
                sequence = mg_emoji_flag_sequence (mg_emoji_flag_codes[i]);
                if (sequence == NULL)
                        continue;

                pixbuf = mg_emoji_flag_load_pixbuf (mg_emoji_flag_codes[i]);
                if (pixbuf)
                {
                        PangoAttrList *attrs;

                        box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);
                        {
                                GdkTexture *texture = gdk_texture_new_for_pixbuf (pixbuf);
                                image = gtk_picture_new_for_paintable (
                                                GDK_PAINTABLE (texture));
                                gtk_picture_set_content_fit (GTK_PICTURE (image),
                                                GTK_CONTENT_FIT_CONTAIN);
                                gtk_picture_set_can_shrink (GTK_PICTURE (image),
                                                FALSE);
                                gtk_widget_set_size_request (image,
                                                MG_EMOJI_FLAG_WIDTH,
                                                MG_EMOJI_FLAG_HEIGHT);
                                g_object_unref (texture);
                        }
                        g_object_unref (pixbuf);

                        label = gtk_label_new (mg_emoji_flag_codes[i]);
                        attrs = pango_attr_list_new ();
                        pango_attr_list_insert (attrs, pango_attr_scale_new (PANGO_SCALE_SMALL));
                        gtk_label_set_attributes (GTK_LABEL (label), attrs);
                        pango_attr_list_unref (attrs);
                        gtk_widget_set_name (label, "fabulor-emoji-flag-code");
                        fabulor_gtk_box_append (GTK_BOX (box), image, FALSE, FALSE, 0);
                        fabulor_gtk_box_append (GTK_BOX (box), label, FALSE, FALSE, 0);

                        tooltip = g_strdup_printf (_("Insert %s flag."), mg_emoji_flag_codes[i]);
                        mg_emoji_add_button (flow, entry, popover, box,
                                             sequence, tooltip);
                        g_free (tooltip);
                }
                else
                {
                        label = gtk_label_new (mg_emoji_flag_codes[i]);
                        mg_emoji_add_button (flow, entry, popover, label,
                                             sequence,
                                             mg_emoji_flag_codes[i]);
                }

                g_free (sequence);
        }

        return scrolled;
}

static void
mg_emoji_picker_page_free (EmojiPickerPage *state)
{
        if (!state)
                return;
        fabulor_emoji_picker_page_free (state->page);
        g_free (state);
}

static GtkWidget *
mg_emoji_lazy_page_new (GtkEntry *entry, GtkWidget *popover,
                        const gunichar *items, gboolean flags)
{
        EmojiPickerPage *state;
        GtkWidget *page;

        page = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        state = g_new0 (EmojiPickerPage, 1);
        state->entry = entry;
        state->popover = popover;
        state->page = fabulor_emoji_picker_page_new (items, flags);
        g_object_set_data_full (G_OBJECT (page), MG_EMOJI_PAGE_DATA, state,
                                (GDestroyNotify) mg_emoji_picker_page_free);
        return page;
}

static void
mg_emoji_lazy_page_load (GtkWidget *page)
{
        EmojiPickerPage *state;
        GtkWidget *contents;

        if (!page)
                return;
        state = g_object_get_data (G_OBJECT (page), MG_EMOJI_PAGE_DATA);
        if (!state || !fabulor_emoji_picker_page_claim_load (state->page))
                return;

        if (fabulor_emoji_picker_page_has_flags (state->page))
                contents = mg_emoji_flags_page_new (state->entry, state->popover);
        else
                contents = mg_emoji_codepoint_page_new (state->entry, state->popover,
                                fabulor_emoji_picker_page_items (state->page));
        fabulor_gtk_box_append (GTK_BOX (page), contents, TRUE, TRUE, 0);
        fabulor_gtk_widget_reveal_tree (page);
}

static void
mg_emoji_stack_visible_child_cb (GObject *object, GParamSpec *pspec,
                                 gpointer user_data)
{
        GtkWidget *page = gtk_stack_get_visible_child (GTK_STACK (object));

        (void) pspec;
        (void) user_data;
        if (page)
                mg_emoji_lazy_page_load (page);
}

static void
mg_emoji_category_changed_cb (GtkDropDown *dropdown, GParamSpec *pspec,
                              gpointer user_data)
{
        GtkStack *stack = GTK_STACK (user_data);
        guint selected = gtk_drop_down_get_selected (dropdown);
        guint category_count = G_N_ELEMENTS (mg_emoji_categories) - 1;
        gchar *name;

        (void) pspec;
        if (selected > category_count)
                return;

        if (selected == category_count)
                gtk_stack_set_visible_child_name (stack, "emoji-flags");
        else
        {
                name = g_strdup_printf ("emoji-category-%u", selected);
                gtk_stack_set_visible_child_name (stack, name);
                g_free (name);
        }
}

static void
mg_emoji_popover_close_cb (GtkButton *button, gpointer user_data)
{
        (void) button;
        gtk_popover_popdown (GTK_POPOVER (user_data));
}

static void
mg_emoji_popover_apply_size (GtkEntry *entry, GtkPopover *popover)
{
        GtkWidget *outer;
        GtkWidget *root;
        int viewport_width;
        int viewport_height;

        outer = gtk_popover_get_child (popover);
        if (!outer)
                return;
        root = GTK_WIDGET (gtk_widget_get_root (GTK_WIDGET (entry)));
        fabulor_emoji_picker_viewport_size (
                root ? fabulor_gtk_widget_get_allocated_width (root) : 0,
                root ? gtk_widget_get_height (root) : 0,
                &viewport_width, &viewport_height);
        gtk_widget_set_size_request (outer, viewport_width, viewport_height);
}

static void
mg_show_emoji_popover (GtkEntry *entry)
{
        GtkWidget *popover;
        GtkWidget *outer;
        GtkWidget *stack;
        GtkWidget *category_row;
        GtkWidget *category_label;
        GtkWidget *category_selector;
        GtkWidget *close_button;
        GtkWidget *page;
        GtkStringList *category_model;
        gchar *name;
        int i;

        popover = GTK_WIDGET (fabulor_emoji_picker_popover_get (entry));
        if (popover)
        {
                mg_emoji_popover_apply_size (entry, GTK_POPOVER (popover));
                gtk_popover_popup (GTK_POPOVER (popover));
                return;
        }

        popover = GTK_WIDGET (fabulor_emoji_picker_popover_ensure (entry));
        if (!popover)
                return;

        outer = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
        gtk_widget_set_margin_start (outer, 8);
        gtk_widget_set_margin_end (outer, 8);
        gtk_widget_set_margin_top (outer, 8);
        gtk_widget_set_margin_bottom (outer, 8);
        fabulor_gtk_popover_set_child (GTK_POPOVER (popover), outer);

        stack = gtk_stack_new ();
        gtk_widget_set_hexpand (stack, TRUE);
        gtk_widget_set_vexpand (stack, TRUE);
        gtk_stack_set_transition_type (GTK_STACK (stack), GTK_STACK_TRANSITION_TYPE_NONE);

        category_model = gtk_string_list_new (NULL);
        for (i = 0; mg_emoji_categories[i].title != NULL; i++)
                gtk_string_list_append (category_model,
                                        _(mg_emoji_categories[i].title));
        gtk_string_list_append (category_model, _("Flags"));
        category_selector = gtk_drop_down_new (G_LIST_MODEL (category_model),
                                               NULL);
        g_object_unref (category_model);
        gtk_widget_set_hexpand (category_selector, TRUE);
        category_label = gtk_label_new (_("Category"));
        gtk_widget_set_halign (category_label, GTK_ALIGN_START);
        close_button = gtk_button_new_from_icon_name ("window-close-symbolic");
        fabulor_gtk_button_set_flat (GTK_BUTTON (close_button));
        gtk_widget_set_tooltip_text (close_button, _("Close emoji picker."));
        gtk_widget_set_can_focus (close_button, FALSE);
        g_signal_connect (G_OBJECT (close_button), "clicked",
                          G_CALLBACK (mg_emoji_popover_close_cb), popover);
        category_row = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
        fabulor_gtk_box_append (GTK_BOX (category_row), category_label,
                               FALSE, FALSE, 0);
        fabulor_gtk_box_append (GTK_BOX (category_row), category_selector,
                               TRUE, TRUE, 0);
        fabulor_gtk_box_append (GTK_BOX (category_row), close_button,
                               FALSE, FALSE, 0);
        fabulor_gtk_box_append (GTK_BOX (outer), category_row,
                               FALSE, FALSE, 0);
        fabulor_gtk_box_append (GTK_BOX (outer), stack, TRUE, TRUE, 0);

        for (i = 0; mg_emoji_categories[i].title != NULL; i++)
        {
                page = mg_emoji_lazy_page_new (entry, popover, mg_emoji_categories[i].items, FALSE);
                name = g_strdup_printf ("emoji-category-%d", i);
                gtk_stack_add_titled (GTK_STACK (stack), page, name,
                                      _(mg_emoji_categories[i].title));
                g_free (name);
        }

        page = mg_emoji_lazy_page_new (entry, popover, NULL, TRUE);
        gtk_stack_add_titled (GTK_STACK (stack), page, "emoji-flags", _("Flags"));
        g_signal_connect (G_OBJECT (stack), "notify::visible-child",
                          G_CALLBACK (mg_emoji_stack_visible_child_cb), NULL);
        g_signal_connect_object (G_OBJECT (category_selector),
                                 "notify::selected",
                                 G_CALLBACK (mg_emoji_category_changed_cb),
                                 G_OBJECT (stack), 0);
        page = gtk_stack_get_visible_child (GTK_STACK (stack));
        mg_emoji_lazy_page_load (page);

        mg_emoji_popover_apply_size (entry, GTK_POPOVER (popover));
        fabulor_gtk_widget_reveal_tree (popover);
        gtk_popover_popup (GTK_POPOVER (popover));
}


static void
mg_inputbox_icon_release_cb (GtkEntry *entry, GtkEntryIconPosition icon_pos,
                             gpointer user_data)
{
        (void) user_data;
        if (icon_pos != GTK_ENTRY_ICON_SECONDARY)
                return;

        mg_show_emoji_popover (entry);
}

/* ------------------------------------------------------------------------- *
 * Emoji font handling
 *
 * Goal: prefer color emoji fonts when available, without changing existing
 *       font size/style/weight, and without breaking user-configured fonts.
 * ------------------------------------------------------------------------- */

static const char *mg_emoji_family_fallback =
#ifdef G_OS_WIN32
        "Noto Color Emoji, Segoe UI Emoji, Segoe UI Symbol, Apple Color Emoji, Twemoji Mozilla, EmojiOne Color";
#else
        "Noto Color Emoji, Segoe UI Emoji, Apple Color Emoji, Twemoji Mozilla, EmojiOne Color";
#endif

static const char *
mg_find_available_icon_name (const char *const *icon_names)
{
        GtkIconTheme *theme;
        int i;

        theme = fabulor_gtk_icon_theme_get_default ();
        if (!theme || !icon_names)
                return NULL;

        for (i = 0; icon_names[i] != NULL; i++)
        {
                int action;

                if (gtk_icon_theme_has_icon (theme, icon_names[i]))
                        return icon_names[i];

                if (icon_resolver_menu_action_from_name (icon_names[i], &action))
                {
                        char *resource_path = icon_resolver_resolve_path (ICON_RESOLVER_ROLE_MENU_ACTION, action, FABULOR_GTK_ICON_SIZE_MENU, "menu", ICON_RESOLVER_THEME_SYSTEM, NULL);

                        if (resource_path)
                        {
                                g_free (resource_path);
                                return icon_names[i];
                        }
                }
        }

        return NULL;
}

static gboolean
mg_family_already_has_emoji (const gchar *family)
{
        if (!family || !*family)
                return FALSE;

        /* cheap but effective */
        return (strstr (family, "Noto Color Emoji") != NULL) ||
               (strstr (family, "Segoe UI Emoji") != NULL) ||
               (strstr (family, "Apple Color Emoji") != NULL) ||
               (strstr (family, "Twemoji") != NULL) ||
               (strstr (family, "EmojiOne") != NULL);
}

static PangoFontDescription *
mg_fontdesc_with_fallback (const PangoFontDescription *base_desc, gboolean emoji_first)
{
        PangoFontDescription *desc;
        const gchar *base_family;
        gchar *family_list;

        if (!base_desc)
                return NULL;

        desc = pango_font_description_copy (base_desc);
        base_family = pango_font_description_get_family (desc);

        if (mg_family_already_has_emoji (base_family))
                return desc;

        if (emoji_first)
        {
                family_list = g_strdup_printf ("%s, %s",
                        mg_emoji_family_fallback,
                        (base_family && *base_family) ? base_family : "Sans");
        }
        else
        {
                family_list = g_strdup_printf ("%s, %s",
                        (base_family && *base_family) ? base_family : "Sans",
                        mg_emoji_family_fallback);
        }

        pango_font_description_set_family (desc, family_list);
        g_free (family_list);

        return desc;
}

static void
mg_apply_emoji_fallback_widget (GtkWidget *widget)
{
        PangoFontDescription *desc;
        PangoFontDescription *base_desc;

        if (!widget)
                return;

        base_desc = fabulor_gtk_widget_dup_font_description (widget);
        if (!base_desc)
                return;

        desc = mg_fontdesc_with_fallback (base_desc, FALSE);
        pango_font_description_free (base_desc);
        if (!desc)
                return;

        mg_apply_font_css (widget, desc, "zoitechat-emoji-font",
                           "zoitechat-emoji-font-provider");
        pango_font_description_free (desc);
}

/* Search bar adapted from Conspire's by William Pitcock */

#define SEARCH_CHANGE           1
#define SEARCH_NEXT                     2
#define SEARCH_PREVIOUS         3
#define SEARCH_REFRESH          4

static session *
search_find_channel (session *sess, const gchar *text)
{
	GSList *list;
	session *item;

	if (!text || !text[0])
		return NULL;

	list = sess_list;
	while (list)
	{
		item = list->data;
		if (item->server == sess->server &&
			item->type == SESS_CHANNEL &&
			nocasestrstr (item->channel, text))
			return item;
		list = list->next;
	}

	return NULL;
}

static void
search_handle_event(int search_type, session *sess)
{
        textentry *last;
        const gchar *text = NULL;
        gtk_xtext_search_flags flags;
        GError *err = NULL;
        gboolean backwards = FALSE;

        /* When just typing show most recent first */
        if (search_type == SEARCH_PREVIOUS || search_type == SEARCH_CHANGE)
                backwards = TRUE;

        flags = ((prefs.hex_text_search_case_match == 1? case_match: 0) |
                                (backwards? backward: 0) |
                                (prefs.hex_text_search_highlight_all == 1? highlight: 0) |
                                (prefs.hex_text_search_follow == 1? follow: 0) |
                                (prefs.hex_text_search_regexp == 1? regexp: 0));

        if (search_type != SEARCH_REFRESH)
                text = fabulor_gtk_entry_get_text (GTK_ENTRY(sess->gui->shentry));

        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sess->gui->shchan)))
        {
                if (search_type == SEARCH_CHANGE || search_type == SEARCH_REFRESH)
                {
                        gtk_entry_set_icon_from_icon_name (GTK_ENTRY (sess->gui->shentry), GTK_ENTRY_ICON_SECONDARY, NULL);
                        return;
                }
                session *match = search_find_channel (sess, text);
                if (match)
                {
                        gtk_entry_set_icon_from_icon_name (GTK_ENTRY (sess->gui->shentry), GTK_ENTRY_ICON_SECONDARY, NULL);
                        mg_bring_tofront_sess (match);
                }
                else if (text && text[0])
                {
                        gtk_entry_set_icon_from_icon_name (GTK_ENTRY (sess->gui->shentry), GTK_ENTRY_ICON_SECONDARY, ICON_ENTRY_ERROR);
                        gtk_entry_set_icon_tooltip_text (GTK_ENTRY (sess->gui->shentry), GTK_ENTRY_ICON_SECONDARY, _("No channels found."));
                }
                else
                {
                        gtk_entry_set_icon_from_icon_name (GTK_ENTRY (sess->gui->shentry), GTK_ENTRY_ICON_SECONDARY, NULL);
                }
                return;
        }

        last = gtk_xtext_search (GTK_XTEXT (sess->gui->xtext), text, flags, &err);

        if (err)
        {
                gtk_entry_set_icon_from_icon_name (GTK_ENTRY (sess->gui->shentry), GTK_ENTRY_ICON_SECONDARY, ICON_ENTRY_ERROR);
                gtk_entry_set_icon_tooltip_text (GTK_ENTRY (sess->gui->shentry), GTK_ENTRY_ICON_SECONDARY, _(err->message));
                g_error_free (err);
        }
        else if (!last)
        {
                if (text && text[0] == 0) /* empty string, no error */
                {
                        gtk_entry_set_icon_from_icon_name (GTK_ENTRY (sess->gui->shentry), GTK_ENTRY_ICON_SECONDARY, NULL);
                }
                else
                {
                        /* Either end of search or not found, try again to wrap if only end */
                        last = gtk_xtext_search (GTK_XTEXT (sess->gui->xtext), text, flags, &err);
                        if (!last) /* Not found error */
                        {
                                gtk_entry_set_icon_from_icon_name (GTK_ENTRY (sess->gui->shentry), GTK_ENTRY_ICON_SECONDARY, ICON_ENTRY_ERROR);
                                gtk_entry_set_icon_tooltip_text (GTK_ENTRY (sess->gui->shentry), GTK_ENTRY_ICON_SECONDARY, _("No results found."));
                        }
                }
        }
        else
        {
                gtk_entry_set_icon_from_icon_name (GTK_ENTRY (sess->gui->shentry), GTK_ENTRY_ICON_SECONDARY, NULL);
        }
}

static void
search_handle_change(GtkWidget *wid, session *sess)
{
        search_handle_event(SEARCH_CHANGE, sess);
}

static void
search_handle_refresh(GtkWidget *wid, session *sess)
{
        search_handle_event(SEARCH_REFRESH, sess);
}

void
mg_search_handle_previous(GtkWidget *wid, session *sess)
{
        search_handle_event(SEARCH_PREVIOUS, sess);
}

void
mg_search_handle_next(GtkWidget *wid, session *sess)
{
        search_handle_event(SEARCH_NEXT, sess);
}

static void
search_set_option (GtkToggleButton *but, guint *pref)
{
        *pref = gtk_toggle_button_get_active(but);
        if (!save_config ())
                fe_message (_("Could not save fabulor.conf."), FE_MSG_WARN);
}

void
mg_search_toggle(session *sess)
{
        if (gtk_widget_get_visible(sess->gui->shbox))
        {
                gtk_widget_hide(sess->gui->shbox);
                gtk_widget_grab_focus(sess->gui->input_box);
                fabulor_gtk_entry_set_text(GTK_ENTRY(sess->gui->shentry), "");
        }
        else
        {
                /* Reset search state */
                gtk_entry_set_icon_from_icon_name (GTK_ENTRY (sess->gui->shentry), GTK_ENTRY_ICON_SECONDARY, NULL);

                /* Show and focus */
                gtk_widget_show(sess->gui->shbox);
                gtk_widget_grab_focus(sess->gui->shentry);
        }
}

static gboolean
search_handle_esc (GtkWidget *widget, guint keyval, GdkModifierType state,
                   gpointer user_data)
{
        session *sess = user_data;

        (void) widget;
        (void) state;

        if (keyval == GDK_KEY_Escape)
                mg_search_toggle(sess);

        return FALSE;
}

static void
mg_create_search(session *sess, GtkWidget *box)
{
        GtkWidget *entry, *label, *next, *previous, *highlight, *matchcase, *regex, *close, *channels;
        session_gui *gui = sess->gui;

        gui->shbox = mg_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 5);
        fabulor_gtk_box_append (GTK_BOX (box), gui->shbox, FALSE, FALSE, 0);

        close = gtk_button_new ();
        fabulor_gtk_button_set_child (GTK_BUTTON (close),
                gtkutil_image_new_from_stock (ICON_TAB_CLOSE,
                                              FABULOR_GTK_ICON_SIZE_MENU));
        fabulor_gtk_button_set_flat (GTK_BUTTON (close));
        gtk_widget_set_can_focus (close, FALSE);
        fabulor_gtk_box_append (GTK_BOX (gui->shbox), close, FALSE, FALSE, 0);
        g_signal_connect_swapped(G_OBJECT(close), "clicked", G_CALLBACK(mg_search_toggle), sess);

        label = gtk_label_new(_("Find:"));
        fabulor_gtk_box_append (GTK_BOX (gui->shbox), label, FALSE, FALSE, 0);

        gui->shentry = entry = gtk_entry_new();
        fabulor_gtk_box_append (GTK_BOX (gui->shbox), entry, FALSE, FALSE, 0);
        gtk_widget_set_size_request (gui->shentry, 180, -1);
        mg_apply_emoji_fallback_widget (entry);
        mg_apply_entry_scroll_artifact_fix (entry);
        gui->search_changed_signal = g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(search_handle_change), sess);
        fabulor_gtk_widget_on_key_pressed (entry, search_handle_esc, sess);
        g_signal_connect(G_OBJECT(entry), "activate", G_CALLBACK(mg_search_handle_next), sess);
        gtk_entry_set_icon_activatable (GTK_ENTRY (entry), GTK_ENTRY_ICON_SECONDARY, FALSE);
        gtk_entry_set_icon_tooltip_text (GTK_ENTRY (sess->gui->shentry), GTK_ENTRY_ICON_SECONDARY, _("Search hit end or not found."));

        previous = gtk_button_new ();
        fabulor_gtk_button_set_child (GTK_BUTTON (previous),
                gtkutil_image_new_from_stock (ICON_TAB_PREVIOUS,
                                              FABULOR_GTK_ICON_SIZE_MENU));
        fabulor_gtk_button_set_flat (GTK_BUTTON (previous));
        gtk_widget_set_can_focus (previous, FALSE);
        fabulor_gtk_box_append (GTK_BOX (gui->shbox), previous, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(previous), "clicked", G_CALLBACK(mg_search_handle_previous), sess);

        next = gtk_button_new ();
        fabulor_gtk_button_set_child (GTK_BUTTON (next),
                gtkutil_image_new_from_stock (ICON_TAB_NEXT,
                                              FABULOR_GTK_ICON_SIZE_MENU));
        fabulor_gtk_button_set_flat (GTK_BUTTON (next));
        gtk_widget_set_can_focus (next, FALSE);
        fabulor_gtk_box_append (GTK_BOX (gui->shbox), next, FALSE, FALSE, 0);
        g_signal_connect(G_OBJECT(next), "clicked", G_CALLBACK(mg_search_handle_next), sess);

        highlight = gtk_check_button_new_with_mnemonic (_("_Highlight all"));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(highlight), prefs.hex_text_search_highlight_all);
        gtk_widget_set_can_focus (highlight, FALSE);
        g_signal_connect (G_OBJECT (highlight), "toggled", G_CALLBACK (search_set_option), &prefs.hex_text_search_highlight_all);
        g_signal_connect (G_OBJECT (highlight), "toggled", G_CALLBACK (search_handle_refresh), sess);
        fabulor_gtk_box_append (GTK_BOX (gui->shbox), highlight, FALSE, FALSE, 0);
        gtk_widget_set_tooltip_text (highlight, _("Highlight all occurrences, and underline the current occurrence."));

        matchcase = gtk_check_button_new_with_mnemonic (_("Mat_ch case"));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(matchcase), prefs.hex_text_search_case_match);
        gtk_widget_set_can_focus (matchcase, FALSE);
        g_signal_connect (G_OBJECT (matchcase), "toggled", G_CALLBACK (search_set_option), &prefs.hex_text_search_case_match);
        fabulor_gtk_box_append (GTK_BOX (gui->shbox), matchcase, FALSE, FALSE, 0);
        gtk_widget_set_tooltip_text (matchcase, _("Perform a case-sensitive search."));

        regex = gtk_check_button_new_with_mnemonic (_("_Regex"));
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(regex), prefs.hex_text_search_regexp);
        gtk_widget_set_can_focus (regex, FALSE);
        g_signal_connect (G_OBJECT (regex), "toggled", G_CALLBACK (search_set_option), &prefs.hex_text_search_regexp);
        fabulor_gtk_box_append (GTK_BOX (gui->shbox), regex, FALSE, FALSE, 0);
        gtk_widget_set_tooltip_text (regex, _("Regard search string as a regular expression."));

        gui->shchan = channels = gtk_check_button_new_with_mnemonic (_("_Channels only"));
        gtk_widget_set_can_focus (channels, FALSE);
        fabulor_gtk_box_append (GTK_BOX (gui->shbox), channels, FALSE, FALSE, 0);
        gtk_widget_set_tooltip_text (channels, _("Search channel names in your current channel list."));
        g_signal_connect (G_OBJECT (channels), "toggled", G_CALLBACK (search_handle_refresh), sess);
}

static void
mg_create_entry (session *sess, GtkWidget *box)
{
        GtkWidget *hbox, *but, *entry;
        session_gui *gui = sess->gui;
        const char *emoji_fallback_icon_names[] = {
                "emoji-people-symbolic",
                "face-smile-symbolic",
                "face-smile",
                "insert-emoticon-symbolic",
                "insert-emoticon",
                "zc-menu-emoji",
                NULL
        };
        const char *emoji_fallback_icon_name;

        gui->reply_box = mg_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 6);
        gtk_widget_set_name (gui->reply_box, "zoitechat-replybar");
        fabulor_gtk_widget_hide_until_explicitly_shown (gui->reply_box);
        fabulor_gtk_box_append (GTK_BOX (box), gui->reply_box, FALSE, FALSE, 0);
        gui->reply_label = gtk_label_new ("");
        gtk_label_set_ellipsize (GTK_LABEL (gui->reply_label), PANGO_ELLIPSIZE_END);
        fabulor_gtk_box_append (GTK_BOX (gui->reply_box), gui->reply_label, TRUE, TRUE, 8);
        but = gtk_button_new_with_label ("×");
        fabulor_gtk_button_set_flat (GTK_BUTTON (but));
        gtk_widget_set_can_focus (but, FALSE);
        fabulor_gtk_box_append (GTK_BOX (gui->reply_box), but, FALSE, FALSE, 0);
        g_signal_connect (G_OBJECT (but), "clicked", G_CALLBACK (mg_reply_cancel_cb), sess);

        hbox = mg_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
        fabulor_gtk_box_append (GTK_BOX (box), hbox, FALSE, FALSE, 0);

        gui->nick_box = mg_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
        fabulor_gtk_box_append (GTK_BOX (hbox), gui->nick_box, FALSE, FALSE, 0);

        gui->nick_label = but = gtk_button_new_with_label (sess->server->nick);
        fabulor_gtk_button_set_flat (GTK_BUTTON (but));
        gtk_widget_set_can_focus (but, FALSE);
        fabulor_gtk_horizontal_box_append_trailing (GTK_BOX (gui->nick_box), but);
        g_signal_connect (G_OBJECT (but), "clicked",
                                                        G_CALLBACK (mg_nickclick_cb), NULL);

        mg_apply_main_font_widget (but, input_style ? input_style->font_desc : NULL);

        gui->input_box = entry = sexy_spell_entry_new ();
        sexy_spell_entry_set_checked ((SexySpellEntry *)entry, prefs.hex_gui_input_spell);
        sexy_spell_entry_set_parse_attributes ((SexySpellEntry *)entry, prefs.hex_gui_input_attr);

        gtk_entry_set_max_length (GTK_ENTRY (gui->input_box), 0);
        g_signal_connect (G_OBJECT (entry), "activate",
                                                        G_CALLBACK (mg_inputbox_cb), gui);
        g_signal_connect (G_OBJECT (entry), "changed",
                                                        G_CALLBACK (mg_inputbox_changed), gui);
        fabulor_gtk_box_append (GTK_BOX (hbox), entry, TRUE, TRUE, 0);

        gtk_widget_set_name (entry, "zoitechat-inputbox");
        fabulor_gtk_widget_on_key_pressed (entry, key_handle_key_press, NULL);
        fabulor_gtk_widget_on_focus_enter (entry, mg_inputbox_focus, gui);
        g_signal_connect (G_OBJECT (entry), "icon-release",
                                                        G_CALLBACK (mg_inputbox_icon_release_cb), NULL);
        g_signal_connect (G_OBJECT (entry), "word-check",
                                                        G_CALLBACK (mg_spellcheck_cb), NULL);
        gtk_widget_grab_focus (entry);

        if (prefs.hex_gui_input_style)
                mg_apply_entry_style (entry);
        mg_apply_entry_scroll_artifact_fix (entry);

        g_object_set (G_OBJECT (entry), "show-emoji-icon", FALSE, NULL);

        emoji_fallback_icon_name = mg_find_available_icon_name (emoji_fallback_icon_names);
        gtk_entry_set_icon_activatable (GTK_ENTRY (entry), GTK_ENTRY_ICON_SECONDARY, TRUE);
        gtk_entry_set_icon_sensitive (GTK_ENTRY (entry), GTK_ENTRY_ICON_SECONDARY, TRUE);
        gtk_entry_set_icon_tooltip_text (GTK_ENTRY (entry), GTK_ENTRY_ICON_SECONDARY, _("Insert emoji."));
        if (emoji_fallback_icon_name)
                gtk_entry_set_icon_from_icon_name (GTK_ENTRY (entry), GTK_ENTRY_ICON_SECONDARY, emoji_fallback_icon_name);
}

static void
mg_switch_tab_cb (chanview *cv, chan *ch, int tag, gpointer ud)
{
        chan *old;
        session *sess = ud;
        gint64 started = fabulor_ui_profile_enabled () ?
                g_get_monotonic_time () : 0;

        old = active_tab;
        active_tab = ch;

        if (tag == TAG_IRC)
        {
                if (active_tab != old)
                {
                        if (old && current_tab)
                                mg_unpopulate (current_tab);
                        mg_populate (sess);
                }
        } else if (old != active_tab)
        {
                /* userdata for non-irc tabs is actually the GtkBox */
                mg_show_generic_tab (ud);
                if (!mg_is_userlist_and_tree_combined ())
                        mg_userlist_showhide (current_sess, FALSE);     /* hide */
        }
        if (started)
                fabulor_ui_profile_log ("tab-switch",
                                       "total_us=%" G_GINT64_FORMAT
                                       " tag=%d channel=\"%s\"",
                                       g_get_monotonic_time () - started, tag,
                                       tag == TAG_IRC && sess ? sess->channel : "");
}

/* compare two tabs (for tab sorting function) */

static int
mg_tabs_compare (session *a, session *b)
{
        /* server tabs always go first */
        if (a->type == SESS_SERVER)
                return -1;

        /* then channels */
        if (a->type == SESS_CHANNEL && b->type != SESS_CHANNEL)
                return -1;
        if (a->type != SESS_CHANNEL && b->type == SESS_CHANNEL)
                return 1;

        return g_ascii_strcasecmp (a->channel, b->channel);
}

static void
mg_create_tabs (session_gui *gui)
{
        gboolean use_icons = FALSE;

        /* if any one of these PNGs exist, the chanview will create
         * the extra column for icons. */
        if (prefs.hex_gui_tab_icons && (pix_tree_channel || pix_tree_dialog || pix_tree_server || pix_tree_util))
        {
                use_icons = TRUE;
        }

        gui->chanview = chanview_new (prefs.hex_gui_tab_layout, prefs.hex_gui_tab_trunc,
                                                                                        prefs.hex_gui_tab_sort, use_icons,
                                                                                        input_style ? input_style->font_desc : NULL
        );
        chanview_set_callbacks (gui->chanview, mg_switch_tab_cb, mg_xbutton_cb,
                                                                        mg_tab_contextmenu_cb, (void *)mg_tabs_compare);
        mg_place_userlist_and_chanview (gui);
}

static void
mg_tabwin_focus_cb (GtkWidget *win, gpointer user_data)
{
        (void) user_data;
        current_sess = current_tab;
        if (current_sess)
        {
                gtk_xtext_check_marker_visibility (GTK_XTEXT (current_sess->gui->xtext));
                plugin_emit_dummy_print (current_sess, "Focus Window");
        }
        unflash_window (win);
}

static void
mg_topwin_focus_cb (GtkWidget *win, gpointer user_data)
{
        session *sess = user_data;

        current_sess = sess;
        if (!sess->server->server_session)
                sess->server->server_session = sess;
        gtk_xtext_check_marker_visibility(GTK_XTEXT (current_sess->gui->xtext));
        unflash_window (win);
        plugin_emit_dummy_print (sess, "Focus Window");
}

static void
mg_create_menu (session *sess, GtkWidget *table)
{
        session_gui *gui = sess->gui;

        gui->menu = menu_create_main (TRUE, sess->server->is_away,
                                                                                   sess->server->connected,
                                                                                   sess->server->connected || sess->server->recondelay_tag,
                                                                                   sess->server->end_of_motd, !gui->is_tab,
                                                                                   gui->menu_item);
        gtk_widget_set_hexpand (gui->menu, TRUE);
        gtk_widget_set_vexpand (gui->menu, FALSE);
        gtk_widget_set_halign (gui->menu, GTK_ALIGN_START);
        gtk_widget_set_valign (gui->menu, GTK_ALIGN_FILL);
        gtk_grid_attach (GTK_GRID (table), gui->menu, 0, 0, 3, 1);
}

static void
mg_create_irctab (session *sess, GtkWidget *table)
{
        GtkWidget *vbox;
        session_gui *gui = sess->gui;

        vbox = mg_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 0);
        gtk_widget_set_hexpand (vbox, TRUE);
        gtk_widget_set_vexpand (vbox, TRUE);
        gtk_widget_set_halign (vbox, GTK_ALIGN_FILL);
        gtk_widget_set_valign (vbox, GTK_ALIGN_FILL);
        gtk_grid_attach (GTK_GRID (table), vbox, 1, 2, 1, 1);
        mg_create_center (sess, gui, vbox);
}

static void
mg_create_topwindow (session *sess)
{
	GtkWidget *win;
	GtkWidget *table;

        if (sess->type == SESS_DIALOG)
                win = gtkutil_window_new (DISPLAY_NAME, NULL,
                                                                                  prefs.hex_gui_dialog_width, prefs.hex_gui_dialog_height, 0);
        else
                win = gtkutil_window_new (DISPLAY_NAME, NULL,
                                                                                  prefs.hex_gui_win_width,
                                                                                  prefs.hex_gui_win_height, 0);
        sess->gui->window = win;
        fabulor_gtk_container_set_uniform_inset (win, GUI_BORDER);
        gtk_widget_set_opacity (win, (prefs.hex_gui_transparency / 255.));

        fabulor_gtk_widget_on_focus_enter (win, mg_topwin_focus_cb, sess);
	fabulor_window_geometry_watch (GTK_WINDOW (win), mg_geometry_cb, sess);


        table = gtk_grid_new ();
        /* spacing under the menubar */
        gtk_grid_set_row_spacing (GTK_GRID (table), GUI_SPACING);
        /* left and right borders */
        gtk_grid_set_column_spacing (GTK_GRID (table), 1);
        fabulor_gtk_window_set_child (GTK_WINDOW (win), table);

        mg_create_irctab (sess, table);
        mg_create_menu (sess, table);

        if (sess->res->buffer == NULL)
        {
                sess->res->buffer = gtk_xtext_buffer_new (GTK_XTEXT (sess->gui->xtext));
                gtk_xtext_buffer_show (GTK_XTEXT (sess->gui->xtext), sess->res->buffer, TRUE);
                gtk_xtext_set_time_stamp (sess->res->buffer, prefs.hex_stamp_text);
                sess->res->user_model = userlist_create_model (sess);
        }

        userlist_show (sess);

        fabulor_gtk_widget_reveal_tree (table);

        if (prefs.hex_gui_hide_menu)
                gtk_widget_hide (sess->gui->menu);

        /* Will be shown when needed */
        gtk_widget_hide (sess->gui->topic_bar);

        if (!prefs.hex_gui_ulist_buttons)
                gtk_widget_hide (sess->gui->button_box);

        if (!prefs.hex_gui_input_nick)
                gtk_widget_hide (sess->gui->nick_box);

        gtk_widget_hide(sess->gui->shbox);

        mg_decide_userlist (sess, FALSE);

        if (sess->type == SESS_DIALOG)
        {
                /* hide the chan-mode buttons */
                gtk_widget_hide (sess->gui->topicbutton_box);
        } else
        {
                gtk_widget_hide (sess->gui->dialogbutton_box);

                if (!prefs.hex_gui_mode_buttons)
                        gtk_widget_hide (sess->gui->topicbutton_box);
        }

        mg_place_userlist_and_chanview (sess->gui);

	gtk_widget_show (win);
	if (!sess->gui->theme_window_listener_id)
		sess->gui->theme_window_listener_id = theme_listener_register ("maingui.window", mg_theme_window_changed, sess->gui);
	theme_manager_attach_window (win);
	mg_topwindow_lifecycle_connect (win, sess);

}

static gboolean
mg_tabwindow_close_request (GtkWindow *win)
{
        GSList *list;
        session *sess;

        if (prefs.hex_gui_tray_close && gtkutil_tray_icon_supported (win) && tray_toggle_visibility (FALSE))
                return TRUE;

        /* check for remaining toplevel windows */
        list = sess_list;
        while (list)
        {
                sess = list->data;
                if (!sess->gui->is_tab)
                        return FALSE;
                list = list->next;
        }

        mg_open_quit_dialog (TRUE);
        return TRUE;
}

static gboolean
mg_tabwindow_close_request_cb (GtkWindow *win, gpointer user_data)
{
        (void) user_data;
        return mg_tabwindow_close_request (win);
}

#ifdef G_OS_WIN32
#define MG_WIN32_COPYDATA_MAX_BYTES (64 * 1024)

static gboolean
mg_win32_message_dispatch (MSG *msg)
{
	if (!msg)
		return FALSE;

	if (tray_win32_message_dispatch (msg->message, msg->wParam, msg->lParam))
		return TRUE;

	if (msg->message == WM_TIMECHANGE)
	{
		_tzset();
		return FALSE;
	}


	if (msg->message == WM_COPYDATA)
	{
		COPYDATASTRUCT *copy_data = (COPYDATASTRUCT *)msg->lParam;

		if (copy_data && copy_data->dwData == 0 && copy_data->lpData &&
			copy_data->cbData > 1 &&
			copy_data->cbData <= MG_WIN32_COPYDATA_MAX_BYTES && current_sess)
		{
			const char *payload = copy_data->lpData;
			char *command;

			if (payload[copy_data->cbData - 1] != '\0')
				return FALSE;
			command = g_strndup (payload, copy_data->cbData - 1);

			if (command)
			{
				if (strcmp (command, "__WIN32_TASKBAR_TOGGLE__") == 0)
				{
					FabulorWindowState state;

					fabulor_window_state_get (
						GTK_WINDOW (current_sess->gui->window), &state);
					if (state.visible
						&& !state.minimized)
						fe_ctrl_gui (current_sess, FE_GUI_ICONIFY, 0);
					else
						fe_ctrl_gui (current_sess, FE_GUI_SHOW, 0);
				}
				else
				{
					handle_command (current_sess, command, FALSE);
				}
				g_free (command);
				return TRUE;
			}
		}
	}

	if (msg->message == WM_MOUSEWHEEL || msg->message == WM_MOUSEHWHEEL)
	{
		POINT cursor_pos;
		HWND hover_hwnd;
		DWORD hover_pid = 0;

		if (!GetCursorPos (&cursor_pos))
			return FALSE;

		hover_hwnd = WindowFromPoint (cursor_pos);
		if (!hover_hwnd || hover_hwnd == msg->hwnd)
			return FALSE;

		GetWindowThreadProcessId (hover_hwnd, &hover_pid);
		if (hover_pid != GetCurrentProcessId ())
			return FALSE;

		return PostMessage (hover_hwnd, msg->message, msg->wParam,
			msg->lParam) != 0;
	}

	return FALSE;
}

static GdkWin32MessageFilterReturn
mg_win32_display_filter (GdkWin32Display *display, MSG *message,
	int *return_value, gpointer user_data)
{
	(void) display;
	(void) return_value;
	(void) user_data;
	return mg_win32_message_dispatch (message) ?
		GDK_WIN32_MESSAGE_FILTER_REMOVE : GDK_WIN32_MESSAGE_FILTER_CONTINUE;
}
#endif

void
mg_win32_message_filter_init (void)
{
#ifdef G_OS_WIN32
	GdkDisplay *display;

	if (mg_win32_filter_installed)
		return;
	display = gdk_display_get_default ();
	if (!display || !GDK_IS_WIN32_DISPLAY (display))
		return;
	mg_win32_filter_display = g_object_ref (display);
	gdk_win32_display_add_filter (GDK_WIN32_DISPLAY (display),
		mg_win32_display_filter, NULL);
	mg_win32_filter_installed = TRUE;
#endif
}

void
mg_win32_message_filter_shutdown (void)
{
#ifdef G_OS_WIN32
	if (mg_win32_filter_installed)
	{
		gdk_win32_display_remove_filter (
			GDK_WIN32_DISPLAY (mg_win32_filter_display),
			mg_win32_display_filter, NULL);
		mg_win32_filter_installed = FALSE;
	}
	g_clear_object (&mg_win32_filter_display);
#endif
}

static void
mg_create_tabwindow (session *sess)
{
        GtkWidget *win;
        GtkWidget *table;

        win = gtkutil_window_new (DISPLAY_NAME, NULL, prefs.hex_gui_win_width,
                                                                          prefs.hex_gui_win_height, 0);
        sess->gui->window = win;
        fabulor_gtk_window_move (GTK_WINDOW (win), prefs.hex_gui_win_left,
                                                  prefs.hex_gui_win_top);
        if (prefs.hex_gui_win_state)
                gtk_window_maximize (GTK_WINDOW (win));
        if (prefs.hex_gui_win_fullscreen)
                gtk_window_fullscreen (GTK_WINDOW (win));
        gtk_widget_set_opacity (win, (prefs.hex_gui_transparency / 255.));
        fabulor_gtk_container_set_uniform_inset (win, GUI_BORDER);

        g_signal_connect (G_OBJECT (win), "close-request",
                          G_CALLBACK (mg_tabwindow_close_request_cb), NULL);
        fabulor_gtk_widget_on_focus_enter (win, mg_tabwin_focus_cb, NULL);
	fabulor_window_geometry_watch (GTK_WINDOW (win), mg_geometry_cb, NULL);
		fabulor_window_state_watch (GTK_WINDOW (win), mg_windowstate_cb, NULL);


        sess->gui->main_table = table = gtk_grid_new ();
        /* spacing under the menubar */
        gtk_grid_set_row_spacing (GTK_GRID (table), GUI_SPACING);
        /* left and right borders */
        gtk_grid_set_column_spacing (GTK_GRID (table), 1);
        fabulor_gtk_window_set_child (GTK_WINDOW (win), table);

        mg_create_irctab (sess, table);
        mg_create_tabs (sess->gui);
        mg_create_menu (sess, table);

        mg_focus (sess);

        fabulor_gtk_widget_reveal_tree (table);

        if (prefs.hex_gui_hide_menu)
                gtk_widget_hide (sess->gui->menu);

        mg_decide_userlist (sess, FALSE);

        /* Will be shown when needed */
        gtk_widget_hide (sess->gui->topic_bar);

        if (!prefs.hex_gui_mode_buttons)
                gtk_widget_hide (sess->gui->topicbutton_box);

        if (!prefs.hex_gui_ulist_buttons)
                gtk_widget_hide (sess->gui->button_box);

        if (!prefs.hex_gui_input_nick)
                gtk_widget_hide (sess->gui->nick_box);

        gtk_widget_hide (sess->gui->shbox);

        mg_place_userlist_and_chanview (sess->gui);

        gtk_widget_show (win);
        if (!sess->gui->theme_window_listener_id)
                sess->gui->theme_window_listener_id = theme_listener_register ("maingui.window", mg_theme_window_changed, sess->gui);
        theme_manager_attach_window (win);
        mg_tabwindow_lifecycle_connect (win, sess->gui);

}

void
mg_apply_setup (gboolean recalculate_transcript_metrics)
{
        GSList *list = sess_list;
        session *sess;
        int done_main = FALSE;

        mg_create_tab_colors ();

        while (list)
        {
                sess = list->data;
                gtk_xtext_set_time_stamp (sess->res->buffer, prefs.hex_stamp_text);
                if (recalculate_transcript_metrics &&
                        GTK_XTEXT (sess->gui->xtext)->buffer != sess->res->buffer)
                        ((xtext_buffer *)sess->res->buffer)->needs_recalc = TRUE;
                if (!sess->gui->is_tab || !done_main)
                        mg_place_userlist_and_chanview (sess->gui);
                if (sess->gui->is_tab)
                        done_main = TRUE;
                list = list->next;
        }
}

static chan *
mg_add_generic_tab (char *name, char *title, void *family, GtkWidget *box)
{
        chan *ch;

        gtk_notebook_append_page (GTK_NOTEBOOK (mg_gui->note_book), box, NULL);
        gtk_widget_show (box);

        ch = chanview_add (mg_gui->chanview, name, NULL, box, TRUE, TAG_UTIL, pix_tree_util);
        chan_set_color (ch, plain_list);

        g_object_set_data_full (G_OBJECT (box), "title", g_strdup (title), g_free);
        g_object_set_data (G_OBJECT (box), "ch", ch);

        if (prefs.hex_gui_tab_newtofront)
                chan_focus (ch);

        return ch;
}

void
fe_buttons_update (session *sess)
{
        session_gui *gui = sess->gui;

        fabulor_gtk_box_remove_child (GTK_BOX (gui->button_box_parent),
                                      gui->button_box);
        gui->button_box = mg_create_userlistbuttons (gui->button_box_parent);

        if (prefs.hex_gui_ulist_buttons)
                gtk_widget_show (sess->gui->button_box);
        else
                gtk_widget_hide (sess->gui->button_box);
}

void
fe_clear_channel (session *sess)
{
        char tbuf[CHANLEN+6];
        session_gui *gui = sess->gui;

        if (sess->gui->is_tab)
        {
                if (sess->waitchannel[0])
                {
                        if (prefs.hex_gui_tab_trunc > 2 && g_utf8_strlen (sess->waitchannel, -1) > prefs.hex_gui_tab_trunc)
                        {
                                /* truncate long channel names */
                                tbuf[0] = '(';
                                strcpy (tbuf + 1, sess->waitchannel);
                                g_utf8_offset_to_pointer(tbuf, prefs.hex_gui_tab_trunc)[0] = 0;
                                strcat (tbuf, "..)");
                        } else
                        {
                                sprintf (tbuf, "(%s)", sess->waitchannel);
                        }
                }
                else
                        strcpy (tbuf, _("<none>"));
                chan_rename (sess->res->tab, tbuf, prefs.hex_gui_tab_trunc);
        }

        if (!sess->gui->is_tab || sess == current_tab)
        {
                gtk_text_buffer_set_text (
                        gtk_text_view_get_buffer (GTK_TEXT_VIEW (gui->topic_entry)), "", -1);

                if (gui->op_xpm)
                {
                        fabulor_gtk_box_remove_child (GTK_BOX (gui->nick_box), gui->op_xpm);
                        gui->op_xpm = 0;
                }
        } else
        {
                if (sess->res->topic_text)
                {
                        g_free (sess->res->topic_text);
                        sess->res->topic_text = NULL;
                }
        }
}

void
fe_set_nonchannel (session *sess, int state)
{
}

void
fe_dlgbuttons_update (session *sess)
{
        GtkWidget *box;
        session_gui *gui = sess->gui;

        fabulor_gtk_box_remove_child (
                GTK_BOX (gtk_widget_get_parent (gui->dialogbutton_box)),
                gui->dialogbutton_box);

        gui->dialogbutton_box = box = mg_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
        fabulor_gtk_box_append (GTK_BOX (gui->topic_bar), box, FALSE, FALSE, 0);
        mg_create_dialogbuttons (box);

        fabulor_gtk_widget_reveal_tree (box);

        if (current_tab && current_tab->type != SESS_DIALOG)
                gtk_widget_hide (current_tab->gui->dialogbutton_box);
}

void
fe_update_mode_buttons (session *sess, char mode, char sign)
{
        int state, i;

        if (sign == '+')
                state = TRUE;
        else
                state = FALSE;

        for (i = 0; i < NUM_FLAG_WIDS - 1; i++)
        {
                if (chan_flags[i] == mode)
                {
                        if (!sess->gui->is_tab || sess == current_tab)
                        {
                                ignore_chanmode = TRUE;
                                if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sess->gui->flag_wid[i])) != state)
                                        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sess->gui->flag_wid[i]), state);
                                ignore_chanmode = FALSE;
                        } else
                        {
                                sess->res->flag_wid_state[i] = state;
                        }
                        return;
                }
        }
}

void
fe_set_nick (server *serv, char *newnick)
{
        GSList *list = sess_list;
        session *sess;

        while (list)
        {
                sess = list->data;
                if (sess->server == serv)
                {
                        if (current_tab == sess || !sess->gui->is_tab)
                                gtk_button_set_label (GTK_BUTTON (sess->gui->nick_label), newnick);
                }
                list = list->next;
        }
}

void
fe_set_away (server *serv)
{
        GSList *list = sess_list;
        session *sess;

        while (list)
        {
                sess = list->data;
                if (sess->server == serv)
                {
                        if (!sess->gui->is_tab || sess == current_tab)
                        {
                                menu_set_away (sess->gui, serv->is_away);
                                /* gray out my nickname */
                                mg_set_myself_away (sess->gui, serv->is_away);
                        }
                }
                list = list->next;
        }
        tray_action_model_refresh ();
}

void
fe_set_channel (session *sess)
{
        if (sess->res->tab != NULL)
                chan_rename (sess->res->tab, sess->channel, prefs.hex_gui_tab_trunc);
}

void
fe_set_typing (session *sess, const char *nick, const char *state)
{
        fe_userlist_set_typing (sess, nick, state);
}

void
mg_changui_new (session *sess, restore_gui *res, int tab, int focus)
{
        int first_run = FALSE;
        session_gui *gui;

        if (res == NULL)
        {
                res = g_new0 (restore_gui, 1);
        }

        sess->res = res;

        if (sess->server->front_session == NULL)
        {
                sess->server->front_session = sess;
        }

        if (!tab)
        {
                gui = g_new0 (session_gui, 1);
                gui->is_tab = FALSE;
                sess->gui = gui;
                mg_create_topwindow (sess);
                fe_set_title (sess);
                return;
        }

        if (mg_gui == NULL)
        {
                first_run = TRUE;
                gui = &static_mg_gui;
                memset (gui, 0, sizeof (session_gui));
                gui->is_tab = TRUE;
                sess->gui = gui;
                mg_create_tabwindow (sess);
                mg_gui = gui;
                parent_window = gui->window;
        } else
        {
                sess->gui = gui = mg_gui;
                gui->is_tab = TRUE;
        }

        mg_add_chan (sess);

        if (first_run || (prefs.hex_gui_tab_newtofront == FOCUS_NEW_ONLY_ASKED && focus)
                        || prefs.hex_gui_tab_newtofront == FOCUS_NEW_ALL )
                chan_focus (res->tab);
}

typedef struct
{
        GDestroyNotify callback;
        gpointer userdata;
} MgGenericTabLifecycle;

static void
mg_generic_tab_lifecycle_invoke (MgGenericTabLifecycle *lifecycle)
{
        lifecycle->callback (lifecycle->userdata);
        g_free (lifecycle);
}

static void
mg_generic_tab_finalized_cb (gpointer userdata, GObject *widget)
{
        (void) widget;
        mg_generic_tab_lifecycle_invoke (userdata);
}

static void
mg_generic_tab_lifecycle_connect (GtkWidget *widget,
                                  GDestroyNotify callback,
                                  gpointer userdata)
{
        MgGenericTabLifecycle *lifecycle;

        if (!callback)
                return;

        lifecycle = g_new (MgGenericTabLifecycle, 1);
        lifecycle->callback = callback;
        lifecycle->userdata = userdata;
        g_object_weak_ref (G_OBJECT (widget), mg_generic_tab_finalized_cb,
                           lifecycle);
}

GtkWidget *
mg_create_generic_tab (char *name, char *title, int force_toplevel,
                                                          int link_buttons,
                                                          GDestroyNotify close_callback,
                                                          gpointer userdata,
                                                          int width, int height, GtkWidget **vbox_ret,
                                                          void *family)
{
        GtkWidget *vbox, *win;

        if (prefs.hex_gui_tab_pos == POS_HIDDEN && prefs.hex_gui_tab_utils)
                prefs.hex_gui_tab_utils = 0;

        if (force_toplevel || !prefs.hex_gui_tab_utils)
        {
                win = gtkutil_window_new (title, name, width, height, 2);
                vbox = mg_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 0);
                *vbox_ret = vbox;
                fabulor_gtk_window_set_child (GTK_WINDOW (win), vbox);
                gtk_widget_show (vbox);
                mg_generic_tab_lifecycle_connect (win, close_callback, userdata);
                return win;
        }

        vbox = mg_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 2);
        g_object_set_data (G_OBJECT (vbox), "w", GINT_TO_POINTER (width));
        g_object_set_data (G_OBJECT (vbox), "h", GINT_TO_POINTER (height));
        fabulor_gtk_container_set_uniform_inset (vbox, 3);
        *vbox_ret = vbox;

        mg_generic_tab_lifecycle_connect (vbox, close_callback, userdata);

        mg_add_generic_tab (name, title, family, vbox);

        return vbox;
}

void
mg_move_tab (session *sess, int delta)
{
        if (sess->gui->is_tab)
                chan_move (sess->res->tab, delta);
}

void
mg_move_tab_family (session *sess, int delta)
{
        if (sess->gui->is_tab)
                chan_move_family (sess->res->tab, delta);
}

void
mg_set_title (GtkWidget *vbox, char *title) /* for non-irc tab/window only */
{
        char *old;

        old = g_object_get_data (G_OBJECT (vbox), "title");
        if (old)
        {
                g_object_set_data_full (G_OBJECT (vbox), "title", g_strdup (title), g_free);
        } else
        {
                gtk_window_set_title (GTK_WINDOW (vbox), title);
        }
}

void
fe_server_callback (server *serv)
{
        joind_close (serv);

        if (serv->gui->chanlist_window)
                mg_close_gen (NULL, serv->gui->chanlist_window);

        if (serv->gui->rawlog_window)
                mg_close_gen (NULL, serv->gui->rawlog_window);

        g_free (serv->gui);
}

/* called when a session is being killed */

void
fe_session_callback (session *sess)
{
        gtk_xtext_buffer_free (sess->res->buffer);
        fabulor_user_list_model_free (sess->res->user_model);

        if (sess->res->banlist && sess->res->banlist->window)
                mg_close_gen (NULL, sess->res->banlist->window);

        g_free (sess->res->input_text);
        g_free (sess->res->topic_text);
        g_free (sess->res->limit_text);
        g_free (sess->res->key_text);
        g_free (sess->res->queue_text);
        g_free (sess->res->queue_tip);
        g_free (sess->res->lag_text);
        g_free (sess->res->lag_tip);

        if (sess->gui->bartag)
                fe_timeout_remove (sess->gui->bartag);

        if (sess->gui != &static_mg_gui)
                g_free (sess->gui);
        g_free (sess->res);
}

/* ===== DRAG AND DROP STUFF ===== */

static gboolean
is_child_of (GtkWidget *widget, GtkWidget *parent)
{
        while (widget)
        {
                if (gtk_widget_get_parent (widget) == parent)
                        return TRUE;
                widget = gtk_widget_get_parent (widget);
        }
        return FALSE;
}

static void
mg_handle_drop (GtkWidget *widget, int y, int *pos, int *other_pos)
{
        int height;
        session_gui *gui = current_sess->gui;

        height = gtk_widget_get_allocated_height (widget);
        if (height <= 0)
                return;

        if (y < height / 2)
        {
                if (is_child_of (widget, gui->vpane_left))
                        *pos = 1;       /* top left */
                else
                        *pos = 3;       /* top right */
        }
        else
        {
                if (is_child_of (widget, gui->vpane_left))
                        *pos = 2;       /* bottom left */
                else
                        *pos = 4;       /* bottom right */
        }

        /* both in the same pos? must move one */
        if (*pos == *other_pos)
        {
                switch (*other_pos)
                {
                case 1:
                        *other_pos = 2;
                        break;
                case 2:
                        *other_pos = 1;
                        break;
                case 3:
                        *other_pos = 4;
                        break;
                case 4:
                        *other_pos = 3;
                        break;
                }
        }

        mg_place_userlist_and_chanview (gui);
}

GdkPixbuf *
mg_internal_drag_icon (GtkWidget *widget, gpointer user_data)
{
        (void) widget;
        (void) user_data;
        return NULL;
}

gboolean
mg_internal_drag_drop (GtkWidget *widget, FabulorGtkInternalDragKind kind,
                       gdouble x, gdouble y, gpointer user_data)
{
        (void) x;
        (void) user_data;

        switch (kind)
        {
        case FABULOR_GTK_INTERNAL_DRAG_USER_LIST:
                mg_handle_drop (widget, (int) y, &prefs.hex_gui_ulist_pos,
                                &prefs.hex_gui_tab_pos);
                break;
        case FABULOR_GTK_INTERNAL_DRAG_CHANNEL_VIEW:
                mg_handle_drop (widget, (int) y, &prefs.hex_gui_tab_pos,
                                &prefs.hex_gui_ulist_pos);
                break;
        default:
                return FALSE;
        }

        return TRUE;
}

gboolean
mg_internal_drag_motion (GtkWidget *widget, FabulorGtkInternalDragKind kind,
                         gdouble x, gdouble y, gpointer scbar)
{
        (void) widget;
        (void) kind;
        (void) x;
        (void) y;
        (void) scbar;
        return TRUE;
}
