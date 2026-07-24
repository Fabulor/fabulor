/* X-Chat
 * Copyright (C) 2006-2007 Peter Zelezny.
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

#include <string.h>
#include "../common/zoitechat-plugin.h"
#include "../common/zoitechat.h"
#include "../common/zoitechatc.h"
#include "../common/inbound.h"
#include "../common/server.h"
#include "../common/fe.h"
#include "../common/util.h"
#include "../common/outbound.h"
#include "fe-gtk.h"
#include "pixmaps.h"
#include "maingui.h"
#include "menu.h"
#include "gtkutil.h"
#include "gtk-compat.h"
#include "plugin-tray.h"
#include "tray-action-model.h"
#include "tray-backend-policy.h"
#include "tray-menu-composition.h"
#include "window-state.h"

#include <gio/gio.h>

#define ICON_TRAY_PREFERENCES "zc-menu-preferences"
#define ICON_TRAY_QUIT "zc-menu-quit"

#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

typedef enum	/* current icon status */
{
	TRAY_ICON_NONE,
	TRAY_ICON_NORMAL,
	TRAY_ICON_MESSAGE,
	TRAY_ICON_HIGHLIGHT,
	TRAY_ICON_FILEOFFER,
	TRAY_ICON_CUSTOM1,
	TRAY_ICON_CUSTOM2
} TrayIconState;

typedef enum
{
	WS_FOCUSED,
	WS_NORMAL,
	WS_HIDDEN
} WinStatus;

typedef GdkPixbuf* TrayIcon;
typedef GdkPixbuf* TrayCustomIcon;
#define tray_icon_from_file(f) gdk_pixbuf_new_from_file(f,NULL)
#define tray_icon_free(i) g_object_unref(i)

#define ICON_NORMAL pix_tray_normal
#define ICON_MSG pix_tray_message
#define ICON_HILIGHT pix_tray_highlight
#define ICON_FILE pix_tray_fileoffer
#define TIMEOUT 500

static void tray_cleanup (void);
static void tray_init (void);
static void tray_set_icon_state (TrayIcon icon, TrayIconState state);
static int tray_find_away_status (void);
static void tray_foreach_server (GtkWidget *item, char *cmd);
static void tray_menu_quit_cb (GtkWidget *item, gpointer userdata);
static void tray_menu_settings (GtkWidget *wid, gpointer none);
static WinStatus tray_get_window_status (void);
static void tray_window_state_cb (GtkWindow *window,
	const FabulorWindowState *state, gpointer userdata);
static void tray_window_visibility_cb (GtkWidget *widget, gpointer userdata);

typedef struct
{
	gboolean (*init)(void);
	void (*set_icon)(TrayIcon icon);
	void (*set_tooltip)(const char *text);
	void (*cleanup)(void);
} TrayBackendOps;

static gboolean tray_backend_active = FALSE;

static FabulorTrayBackendKind
tray_backend_select_for_window (GtkWindow *window)
{
	FabulorTrayBackendEnvironment environment = { 0 };

	environment.enabled = prefs.hex_gui_tray != 0;
	environment.toolkit_major = 4;
#ifdef WIN32
	environment.windows = TRUE;
	environment.windows_shell_available =
		gtkutil_tray_icon_supported (window);
#endif
	return fabulor_tray_backend_select (&environment);
}

static gint flash_tag;
static TrayIconState tray_icon_state;
static TrayIcon tray_flash_icon;
static TrayIconState tray_flash_state;
#if defined(WIN32)
static guint tray_menu_timer;
static gint64 tray_menu_inactivetime;
#endif
static zoitechat_plugin *ph;
static FabulorTrayActionModel *tray_actions;
static GObject *tray_plugin_model_owner;

static TrayCustomIcon custom_icon1;
static TrayCustomIcon custom_icon2;

static int tray_priv_count = 0;
static int tray_pub_count = 0;
static int tray_hilight_count = 0;
static int tray_file_count = 0;

static FabulorTrayAwayState
tray_action_away_state (void)
{
	switch (tray_find_away_status ())
	{
	case 1:
		return FABULOR_TRAY_AWAY_ALL_AWAY;
	case 2:
		return FABULOR_TRAY_AWAY_ALL_BACK;
	default:
		return FABULOR_TRAY_AWAY_MIXED;
	}
}

void
tray_action_model_refresh (void)
{
	FabulorTrayActionState state;

	if (!tray_actions || !ph)
		return;

	state.window_hidden = tray_get_window_status () == WS_HIDDEN;
	state.away_state = tray_action_away_state ();
	state.blink_channel = prefs.hex_input_tray_chans != 0;
	state.blink_private = prefs.hex_input_tray_priv != 0;
	state.blink_highlight = prefs.hex_input_tray_hilight != 0;
	fabulor_tray_action_model_update (tray_actions, &state);
}

gboolean
tray_action_projection_create (GMenuModel **menu,
	GActionGroup **built_in_actions, GActionGroup **plugin_actions)
{
	GMenuModel *built_in_menu;
	GMenuModel *plugin_menu;
	GMenuModel *projection;

	if (menu)
		*menu = NULL;
	if (built_in_actions)
		*built_in_actions = NULL;
	if (plugin_actions)
		*plugin_actions = NULL;

	tray_action_model_refresh ();
	if (!tray_actions || !tray_plugin_model_owner)
		return FALSE;

	menu_add_plugin_model (tray_plugin_model_owner, "\x5$TRAY", NULL);
	built_in_menu = fabulor_tray_action_model_get_menu (tray_actions);
	plugin_menu = menu_plugin_context_model (tray_plugin_model_owner);
	projection = fabulor_tray_menu_compose (built_in_menu,
		plugin_menu, 2);
	if (!projection)
		return FALSE;

	if (menu)
		*menu = g_object_ref (projection);
	if (built_in_actions)
		*built_in_actions = g_object_ref (
			fabulor_tray_action_model_get_actions (tray_actions));
	if (plugin_actions)
		*plugin_actions = g_object_ref (
			menu_plugin_context_actions (tray_plugin_model_owner));
	g_object_unref (projection);
	return TRUE;
}

static void
tray_action_activate (FabulorTrayAction action, gboolean state,
	gpointer user_data)
{
	(void)user_data;

	switch (action)
	{
	case FABULOR_TRAY_ACTION_TOGGLE_VISIBILITY:
		tray_toggle_visibility (FALSE);
		break;
	case FABULOR_TRAY_ACTION_SET_AWAY:
		tray_foreach_server (NULL, "away");
		break;
	case FABULOR_TRAY_ACTION_SET_BACK:
		tray_foreach_server (NULL, "back");
		break;
	case FABULOR_TRAY_ACTION_BLINK_CHANNEL:
		prefs.hex_input_tray_chans = state;
		break;
	case FABULOR_TRAY_ACTION_BLINK_PRIVATE:
		prefs.hex_input_tray_priv = state;
		break;
	case FABULOR_TRAY_ACTION_BLINK_HIGHLIGHT:
		prefs.hex_input_tray_hilight = state;
		break;
	case FABULOR_TRAY_ACTION_PREFERENCES:
		tray_menu_settings (NULL, NULL);
		break;
	case FABULOR_TRAY_ACTION_QUIT:
		tray_menu_quit_cb (NULL, NULL);
		break;
	default:
		break;
	}

	tray_action_model_refresh ();
}

static void
tray_action_model_init (void)
{
	const FabulorTrayActionLabels labels = {
		_("_Hide Window"),
		_("_Restore Window"),
		_("_Blink on"),
		_("Channel Message"),
		_("Private Message"),
		_("Highlighted Message"),
		_("_Change status"),
		_("_Away"),
		_("_Back"),
		_("_Preferences"),
		_("_Quit")
	};
	FabulorTrayActionState state = { 0 };

	if (tray_actions)
		return;

	tray_actions = fabulor_tray_action_model_new (&labels, &state,
		tray_action_activate, NULL, NULL);
	tray_plugin_model_owner = g_object_new (G_TYPE_OBJECT, NULL);
	tray_action_model_refresh ();
}



static const TrayBackendOps tray_backend_ops = { 0 };

static gboolean
tray_backend_init (void)
{
	if (!tray_backend_ops.init)
		return FALSE;

	tray_backend_active = tray_backend_ops.init ();
	return tray_backend_active;
}

static void
tray_backend_set_icon (TrayIcon icon)
{
	if (tray_backend_active && tray_backend_ops.set_icon)
		tray_backend_ops.set_icon (icon);
}

static void
tray_backend_set_tooltip (const char *text)
{
	if (tray_backend_active && tray_backend_ops.set_tooltip)
		tray_backend_ops.set_tooltip (text);
}

static void
tray_backend_cleanup (void)
{
	if (tray_backend_ops.cleanup)
		tray_backend_ops.cleanup ();

	tray_backend_active = FALSE;
}

static WinStatus
tray_get_window_status (void)
{
	FabulorWindowState window_state;
	GtkWindow *win;
	const char *st;

	win = GTK_WINDOW (zoitechat_get_info (ph, "gtkwin_ptr"));
	if (win)
	{
		fabulor_window_state_get (win, &window_state);
		if (!window_state.visible || window_state.minimized)
			return WS_HIDDEN;
	}

	st = zoitechat_get_info (ph, "win_status");

	if (!st)
		return WS_HIDDEN;

	if (!strcmp (st, "active"))
		return WS_FOCUSED;

	if (!strcmp (st, "hidden"))
		return WS_HIDDEN;

	return WS_NORMAL;
}

static int
tray_count_channels (void)
{
	int cons = 0;
	GSList *list;
	session *sess;

	for (list = sess_list; list; list = list->next)
	{
		sess = list->data;
		if (sess->server->connected && sess->channel[0] &&
			 sess->type == SESS_CHANNEL)
			cons++;
	}
	return cons;
}

static int
tray_count_networks (void)
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

static void
tray_set_icon_state (TrayIcon icon, TrayIconState state)
{
	tray_backend_set_icon (icon);
	tray_icon_state = state;
}

static void
tray_set_custom_icon_state (TrayCustomIcon icon, TrayIconState state)
{
	tray_backend_set_icon (icon);
	tray_icon_state = state;
}

void
fe_tray_set_tooltip (const char *text)
{
	if (!tray_backend_active)
		return;

	tray_backend_set_tooltip (text);
}

static void
tray_set_tipf (const char *format, ...)
{
	va_list args;
	char *buf;

	va_start (args, format);
	buf = g_strdup_vprintf (format, args);
	va_end (args);

	fe_tray_set_tooltip (buf);
	g_free (buf);
}

static void
tray_stop_flash (void)
{
	int nets, chans;

	if (flash_tag)
	{
		g_source_remove (flash_tag);
		flash_tag = 0;
	}

	if (tray_backend_active)
	{
		tray_set_icon_state (ICON_NORMAL, TRAY_ICON_NORMAL);
		nets = tray_count_networks ();
		chans = tray_count_channels ();
		if (nets)
			tray_set_tipf (_("Connected to %u networks and %u channels - %s"),
								nets, chans, _(DISPLAY_NAME));
		else
			tray_set_tipf ("%s - %s", _("Not connected."), _(DISPLAY_NAME));
	}

	if (custom_icon1)
	{
		tray_icon_free (custom_icon1);
		custom_icon1 = NULL;
	}

	if (custom_icon2)
	{
		tray_icon_free (custom_icon2);
		custom_icon2 = NULL;
	}

	tray_flash_icon = NULL;
	tray_flash_state = TRAY_ICON_NONE;
}

static void
tray_reset_counts (void)
{
	tray_priv_count = 0;
	tray_pub_count = 0;
	tray_hilight_count = 0;
	tray_file_count = 0;
}

static int
tray_timeout_cb (gpointer userdata)
{
	(void)userdata;

	if (custom_icon1)
	{
		if (tray_icon_state == TRAY_ICON_CUSTOM1)
		{
			if (custom_icon2)
				tray_set_custom_icon_state (custom_icon2, TRAY_ICON_CUSTOM2);
			else
				tray_set_icon_state (ICON_NORMAL, TRAY_ICON_NORMAL);
		}
		else
		{
			tray_set_custom_icon_state (custom_icon1, TRAY_ICON_CUSTOM1);
		}
	}
	else
	{
		if (tray_icon_state == TRAY_ICON_NORMAL)
			tray_set_icon_state (tray_flash_icon, tray_flash_state);
		else
			tray_set_icon_state (ICON_NORMAL, TRAY_ICON_NORMAL);
	}
	return 1;
}

static void
tray_set_flash (TrayIcon icon, TrayIconState state)
{
	if (!tray_backend_active)
		return;

	/* already flashing the same icon */
	if (flash_tag && tray_icon_state == state)
		return;

	/* no flashing if window is focused */
	if (tray_get_window_status () == WS_FOCUSED)
		return;

	tray_stop_flash ();

	tray_flash_icon = icon;
	tray_flash_state = state;
	tray_set_icon_state (icon, state);
	if (prefs.hex_gui_tray_blink)
		flash_tag = g_timeout_add (TIMEOUT, (GSourceFunc) tray_timeout_cb, NULL);
}

void
fe_tray_set_flash (const char *filename1, const char *filename2, int tout)
{
	tray_apply_setup ();
	if (!tray_backend_active)
		return;

	tray_stop_flash ();

	if (tout == -1)
		tout = TIMEOUT;

	custom_icon1 = tray_icon_from_file (filename1);
	if (filename2)
		custom_icon2 = tray_icon_from_file (filename2);

	tray_set_custom_icon_state (custom_icon1, TRAY_ICON_CUSTOM1);
	flash_tag = g_timeout_add (tout, (GSourceFunc) tray_timeout_cb, NULL);
}

void
fe_tray_set_icon (feicon icon)
{
	tray_apply_setup ();
	if (!tray_backend_active)
		return;

	tray_stop_flash ();

	switch (icon)
	{
	case FE_ICON_NORMAL:
		break;
	case FE_ICON_MESSAGE:
	case FE_ICON_PRIVMSG:
		tray_set_flash (ICON_MSG, TRAY_ICON_MESSAGE);
		break;
	case FE_ICON_HIGHLIGHT:
		tray_set_flash (ICON_HILIGHT, TRAY_ICON_HIGHLIGHT);
		break;
	case FE_ICON_FILEOFFER:
		tray_set_flash (ICON_FILE, TRAY_ICON_FILEOFFER);
	}
}

void
fe_tray_set_file (const char *filename)
{
	tray_apply_setup ();
	if (!tray_backend_active)
		return;

	tray_stop_flash ();

	if (filename)
	{
		custom_icon1 = tray_icon_from_file (filename);
		tray_set_custom_icon_state (custom_icon1, TRAY_ICON_CUSTOM1);
	}
}

gboolean
tray_toggle_visibility (gboolean force_hide)
{
	static FabulorGtkWindowPlacement placement;
	static int maximized;
	static int fullscreen;
	GtkWindow *win;
	WinStatus status;

	if (!tray_backend_active)
		return FALSE;

	/* ph may have an invalid context now */
	zoitechat_set_context (ph, zoitechat_find_context (ph, NULL, NULL));

	win = GTK_WINDOW (zoitechat_get_info (ph, "gtkwin_ptr"));

	tray_stop_flash ();
	tray_reset_counts ();

	if (!win)
		return FALSE;

	status = tray_get_window_status ();

	if (force_hide || status != WS_HIDDEN)
	{
		if (prefs.hex_gui_tray_away)
			zoitechat_command (ph, "ALLSERV AWAY");
		fabulor_gtk_window_placement_capture (win, &placement);
		maximized = prefs.hex_gui_win_state;
		fullscreen = prefs.hex_gui_win_fullscreen;
		fabulor_window_hide (win);
	}
	else
	{
		if (prefs.hex_gui_tray_away)
			zoitechat_command (ph, "ALLSERV BACK");
		fabulor_gtk_window_placement_restore (win, &placement);
		if (maximized)
			gtk_window_maximize (win);
		if (fullscreen)
			gtk_window_fullscreen (win);
		fabulor_window_present (win);
	}

	tray_action_model_refresh ();

	return TRUE;
}

static void
tray_menu_quit_cb (GtkWidget *item, gpointer userdata)
{
	(void)item;
	(void)userdata;

	mg_open_quit_dialog (FALSE);
}

/* returns 0-mixed 1-away 2-back */

static int
tray_find_away_status (void)
{
	GSList *list;
	server *serv;
	int away = 0;
	int back = 0;

	for (list = serv_list; list; list = list->next)
	{
		serv = list->data;

		if (serv->is_away || serv->reconnect_away)
			away++;
		else
			back++;
	}

	if (away && back)
		return 0;

	if (away)
		return 1;

	return 2;
}

static void
tray_foreach_server (GtkWidget *item, char *cmd)
{
	GSList *list;
	server *serv;

	for (list = serv_list; list; list = list->next)
	{
		serv = list->data;
		if (serv->connected)
			handle_command (serv->server_session, cmd, FALSE);
	}
}




static void
tray_menu_settings (GtkWidget * wid, gpointer none)
{
	(void)wid;
	(void)none;

	extern void setup_open (void);
	setup_open ();
}

#ifdef WIN32
#define TRAY_WIN32_HIDE 1
#define TRAY_WIN32_AWAY 2
#define TRAY_WIN32_BACK 3
#define TRAY_WIN32_PREFS 4
#define TRAY_WIN32_QUIT 5

static WCHAR *
tray_win32_menu_label (const char *label)
{
	char *plain;
	char *src;
	char *dst;
	WCHAR *wide;

	plain = g_strdup (label ? label : "");
	for (src = plain, dst = plain; *src; src++)
	{
		if (*src == '_')
			continue;
		*dst++ = *src;
	}
	*dst = 0;
	wide = g_utf8_to_utf16 (plain, -1, NULL, NULL, NULL);
	g_free (plain);

	return wide;
}

static void
tray_win32_append_item (HMENU menu, UINT id, const char *label, gboolean enabled)
{
	WCHAR *wide;

	wide = tray_win32_menu_label (label);
	AppendMenuW (menu, MF_STRING | (enabled ? MF_ENABLED : MF_GRAYED), id, wide);
	g_free (wide);
}

static HWND
tray_win32_get_hwnd (void)
{
	HWND hwnd;
	GtkWindow *win;

	win = GTK_WINDOW (zoitechat_get_info (ph, "gtkwin_ptr"));
	if (!win)
		return GetActiveWindow ();

	hwnd = (HWND) fabulor_window_native_handle (win);
	return hwnd ? hwnd : GetActiveWindow ();
}

static void
tray_win32_menu_cb (void)
{
	HMENU menu;
	POINT point;
	UINT command;
	HWND hwnd;
	int away_status;
	GMenuModel *projection = NULL;

	zoitechat_set_context (ph, zoitechat_find_context (ph, NULL, NULL));

	menu = CreatePopupMenu ();
	if (!menu)
		return;
	tray_action_projection_create (&projection, NULL, NULL);
	g_clear_object (&projection);

	away_status = tray_find_away_status ();
	tray_win32_append_item (menu, TRAY_WIN32_HIDE,
		tray_get_window_status () == WS_HIDDEN ? _("_Restore Window") : _("_Hide Window"), TRUE);
	AppendMenuW (menu, MF_SEPARATOR, 0, NULL);
	tray_win32_append_item (menu, TRAY_WIN32_AWAY, _("_Away"), away_status != 1);
	tray_win32_append_item (menu, TRAY_WIN32_BACK, _("_Back"), away_status != 2);
	AppendMenuW (menu, MF_SEPARATOR, 0, NULL);
	tray_win32_append_item (menu, TRAY_WIN32_PREFS, _("_Preferences"), TRUE);
	AppendMenuW (menu, MF_SEPARATOR, 0, NULL);
	tray_win32_append_item (menu, TRAY_WIN32_QUIT, _("_Quit"), TRUE);

	GetCursorPos (&point);
	hwnd = tray_win32_get_hwnd ();
	if (hwnd)
		SetForegroundWindow (hwnd);
	command = TrackPopupMenu (menu, TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
		point.x, point.y, 0, hwnd, NULL);
	DestroyMenu (menu);

	switch (command)
	{
	case TRAY_WIN32_HIDE:
		tray_toggle_visibility (FALSE);
		break;
	case TRAY_WIN32_AWAY:
		tray_foreach_server (NULL, "away");
		break;
	case TRAY_WIN32_BACK:
		tray_foreach_server (NULL, "back");
		break;
	case TRAY_WIN32_PREFS:
		tray_menu_settings (NULL, NULL);
		break;
	case TRAY_WIN32_QUIT:
		tray_menu_quit_cb (NULL, NULL);
		break;
	default:
		break;
	}
}
#endif


static void
tray_window_state_cb (GtkWindow *window, const FabulorWindowState *state,
	gpointer userdata)
{
	(void)window;
	(void)state;
	(void)userdata;

	tray_action_model_refresh ();
}

static void
tray_window_visibility_cb (GtkWidget *widget, gpointer userdata)
{
	(void)widget;
	(void)userdata;

	tray_action_model_refresh ();
}


static void
tray_init (void)
{
	flash_tag = 0;
	tray_icon_state = TRAY_ICON_NONE;
	tray_flash_icon = NULL;
	tray_flash_state = TRAY_ICON_NONE;
	custom_icon1 = NULL;
	custom_icon2 = NULL;

	if (!tray_backend_init ())
		return;
	tray_icon_state = TRAY_ICON_NORMAL;
	tray_set_icon_state (ICON_NORMAL, TRAY_ICON_NORMAL);
}

static int
tray_hilight_cb (char *word[], void *userdata)
{
	/*if (tray_icon_state == TRAY_ICON_HIGHLIGHT)
		return ZOITECHAT_EAT_NONE;*/

	if (prefs.hex_input_tray_hilight)
	{
		tray_set_flash (ICON_HILIGHT, TRAY_ICON_HIGHLIGHT);

		/* FIXME: hides any previous private messages */
		tray_hilight_count++;
		if (tray_hilight_count == 1)
			tray_set_tipf (_("Highlighted message from: %s (%s) - %s"),
								word[1], zoitechat_get_info (ph, "channel"), _(DISPLAY_NAME));
		else
			tray_set_tipf (_("%u highlighted messages, latest from: %s (%s) - %s"),
								tray_hilight_count, word[1], zoitechat_get_info (ph, "channel"),
								_(DISPLAY_NAME));
	}

	return ZOITECHAT_EAT_NONE;
}

static int
tray_message_cb (char *word[], void *userdata)
{
	if (/*tray_icon_state == TRAY_ICON_MESSAGE ||*/ tray_icon_state == TRAY_ICON_HIGHLIGHT)
		return ZOITECHAT_EAT_NONE;
		
	if (prefs.hex_input_tray_chans)
	{
		tray_set_flash (ICON_MSG, TRAY_ICON_MESSAGE);

		tray_pub_count++;
		if (tray_pub_count == 1)
			tray_set_tipf (_("Channel message from: %s (%s) - %s"),
								word[1], zoitechat_get_info (ph, "channel"), _(DISPLAY_NAME));
		else
			tray_set_tipf (_("%u channel messages. - %s"), tray_pub_count, _(DISPLAY_NAME));
	}

	return ZOITECHAT_EAT_NONE;
}

static void
tray_priv (char *from, char *text)
{
	const char *network;

	if (alert_match_word (from, prefs.hex_irc_no_hilight))
		return;

	network = zoitechat_get_info (ph, "network");
	if (!network)
		network = zoitechat_get_info (ph, "server");

	if (prefs.hex_input_tray_priv)
	{
		tray_set_flash (ICON_MSG, TRAY_ICON_MESSAGE);

		tray_priv_count++;
		if (tray_priv_count == 1)
			tray_set_tipf (_("Private message from: %s (%s) - %s"), from,
								network, _(DISPLAY_NAME));
		else
			tray_set_tipf (_("%u private messages, latest from: %s (%s) - %s"),
								tray_priv_count, from, network, _(DISPLAY_NAME));
	}
}

static int
tray_priv_cb (char *word[], void *userdata)
{
	tray_priv (word[1], word[2]);

	return ZOITECHAT_EAT_NONE;
}

static int
tray_invited_cb (char *word[], void *userdata)
{
	if (!prefs.hex_away_omit_alerts || tray_find_away_status () != 1)
		tray_priv (word[2], "Invited");

	return ZOITECHAT_EAT_NONE;
}

static int
tray_dcc_cb (char *word[], void *userdata)
{
	const char *network;

/*	if (tray_icon_state == TRAY_ICON_FILEOFFER)
		return ZOITECHAT_EAT_NONE;*/

	network = zoitechat_get_info (ph, "network");
	if (!network)
		network = zoitechat_get_info (ph, "server");

	if (prefs.hex_input_tray_priv && (!prefs.hex_away_omit_alerts || tray_find_away_status () != 1))
	{
		tray_set_flash (ICON_FILE, TRAY_ICON_FILEOFFER);

		tray_file_count++;
		if (tray_file_count == 1)
			tray_set_tipf (_("File offer from: %s (%s) - %s"), word[1], network,
								_(DISPLAY_NAME));
		else
			tray_set_tipf (_("%u file offers, latest from: %s (%s) - %s"),
								tray_file_count, word[1], network, _(DISPLAY_NAME));
	}

	return ZOITECHAT_EAT_NONE;
}

static int
tray_focus_cb (char *word[], void *userdata)
{
	tray_stop_flash ();
	tray_reset_counts ();
	return ZOITECHAT_EAT_NONE;
}

static void
tray_cleanup (void)
{
	tray_stop_flash ();

	if (tray_backend_active)
		tray_backend_cleanup ();
}

void
tray_apply_setup (void)
{
	GtkWindow *window;

	tray_action_model_refresh ();
	if (tray_backend_active)
	{
		if (!prefs.hex_gui_tray)
			tray_cleanup ();
	}
	else
	{
		window = GTK_WINDOW (zoitechat_get_info (ph, "gtkwin_ptr"));
		if (fabulor_tray_backend_is_usable (
			tray_backend_select_for_window (window)))
			tray_init ();
	}
}

int
tray_plugin_init (zoitechat_plugin *plugin_handle, char **plugin_name,
				char **plugin_desc, char **plugin_version, char *arg)
{
	/* we need to save this for use with any zoitechat_* functions */
	ph = plugin_handle;

	*plugin_name = "Tray";
	*plugin_desc = "System tray integration";
	*plugin_version = "";
	tray_action_model_init ();

	zoitechat_hook_print (ph, "Channel Msg Hilight", -1, tray_hilight_cb, NULL);
	zoitechat_hook_print (ph, "Channel Action Hilight", -1, tray_hilight_cb, NULL);

	zoitechat_hook_print (ph, "Channel Message", -1, tray_message_cb, NULL);
	zoitechat_hook_print (ph, "Channel Action", -1, tray_message_cb, NULL);
	zoitechat_hook_print (ph, "Channel Notice", -1, tray_message_cb, NULL);

	zoitechat_hook_print (ph, "Private Message", -1, tray_priv_cb, NULL);
	zoitechat_hook_print (ph, "Private Message to Dialog", -1, tray_priv_cb, NULL);
	zoitechat_hook_print (ph, "Private Action", -1, tray_priv_cb, NULL);
	zoitechat_hook_print (ph, "Private Action to Dialog", -1, tray_priv_cb, NULL);
	zoitechat_hook_print (ph, "Notice", -1, tray_priv_cb, NULL);
	zoitechat_hook_print (ph, "Invited", -1, tray_invited_cb, NULL);

	zoitechat_hook_print (ph, "DCC Offer", -1, tray_dcc_cb, NULL);

	zoitechat_hook_print (ph, "Focus Window", -1, tray_focus_cb, NULL);

	GtkWindow *window = GTK_WINDOW(zoitechat_get_info (ph, "gtkwin_ptr"));
	GtkWidget *window_widget;

	if (window)
	{
		window_widget = GTK_WIDGET (window);
		fabulor_window_state_watch (window, tray_window_state_cb, NULL);
		g_signal_connect (G_OBJECT (window_widget), "show",
			G_CALLBACK (tray_window_visibility_cb), NULL);
		g_signal_connect (G_OBJECT (window_widget), "hide",
			G_CALLBACK (tray_window_visibility_cb), NULL);
	}

	if (fabulor_tray_backend_is_usable (
		tray_backend_select_for_window (window)))
		tray_init ();

	return 1;       /* return 1 for success */
}

int
tray_plugin_deinit (zoitechat_plugin *plugin_handle)
{
	(void)plugin_handle;
#ifdef WIN32
	tray_cleanup ();
#endif
	fabulor_tray_action_model_free (tray_actions);
	tray_actions = NULL;
	g_clear_object (&tray_plugin_model_owner);
	ph = NULL;
	return 1;
}
