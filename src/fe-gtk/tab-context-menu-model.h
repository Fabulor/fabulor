/* Copyright (C) 2026 Fabulor contributors */
#ifndef FABULOR_TAB_CONTEXT_MENU_MODEL_H
#define FABULOR_TAB_CONTEXT_MENU_MODEL_H

#include <gio/gio.h>

typedef enum
{
	FABULOR_TAB_CONTEXT_OPTION,
	FABULOR_TAB_CONTEXT_AUTOJOIN,
	FABULOR_TAB_CONTEXT_AUTOCONNECT,
	FABULOR_TAB_CONTEXT_DETACH,
	FABULOR_TAB_CONTEXT_CLOSE,
	FABULOR_TAB_CONTEXT_COMMAND,
	FABULOR_TAB_CONTEXT_TOGGLE
} FabulorTabContextAction;

typedef enum
{
	FABULOR_TAB_OPTION_NOTIFICATION,
	FABULOR_TAB_OPTION_BEEP,
	FABULOR_TAB_OPTION_TRAY,
	FABULOR_TAB_OPTION_TASKBAR,
	FABULOR_TAB_OPTION_LOGGING,
	FABULOR_TAB_OPTION_SCROLLBACK,
	FABULOR_TAB_OPTION_STRIP_COLORS,
	FABULOR_TAB_OPTION_HIDE_JOIN_PART,
	FABULOR_TAB_OPTION_COUNT
} FabulorTabOption;

typedef enum
{
	FABULOR_TAB_CONFIG_COMMAND,
	FABULOR_TAB_CONFIG_TOGGLE,
	FABULOR_TAB_CONFIG_SUBMENU_BEGIN,
	FABULOR_TAB_CONFIG_SUBMENU_END,
	FABULOR_TAB_CONFIG_SEPARATOR
} FabulorTabConfiguredKind;

typedef struct
{
	FabulorTabConfiguredKind kind;
	const char *label;
	const char *command;
	const char *icon;
	gboolean active;
} FabulorTabConfiguredItem;

typedef struct
{
	const char *alerts;
	const char *settings;
	const char *notification;
	const char *beep;
	const char *tray;
	const char *taskbar;
	const char *logging;
	const char *scrollback;
	const char *strip_colors;
	const char *hide_join_part;
	const char *autojoin;
	const char *autoconnect;
	const char *detach;
	const char *close;
} FabulorTabContextLabels;

typedef struct
{
	gboolean has_session;
	gboolean is_channel;
	gboolean is_server;
	gboolean has_network;
	gboolean autojoin;
	gboolean autoconnect;
	gboolean options[FABULOR_TAB_OPTION_COUNT];
} FabulorTabContextState;

typedef void (*FabulorTabContextDispatch) (FabulorTabContextAction action,
	FabulorTabOption option, gboolean state, const char *command,
	gpointer user_data);

typedef struct _FabulorTabContextMenuModel FabulorTabContextMenuModel;

FabulorTabContextMenuModel *fabulor_tab_context_menu_model_new (
	const char *heading, const FabulorTabContextState *state,
	const FabulorTabContextLabels *labels,
	const FabulorTabConfiguredItem *configured, gsize configured_count,
	GMenuModel *plugin_model, FabulorTabContextDispatch dispatch,
	gpointer user_data);
void fabulor_tab_context_menu_model_free (FabulorTabContextMenuModel *model);
GMenuModel *fabulor_tab_context_menu_model_get_menu (
	FabulorTabContextMenuModel *model);
GActionGroup *fabulor_tab_context_menu_model_get_actions (
	FabulorTabContextMenuModel *model);

#endif
