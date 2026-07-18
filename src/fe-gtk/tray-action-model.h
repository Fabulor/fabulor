/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_TRAY_ACTION_MODEL_H
#define FABULOR_TRAY_ACTION_MODEL_H

#include <gio/gio.h>

#define FABULOR_TRAY_ACTION_NAMESPACE "tray"

typedef enum
{
	FABULOR_TRAY_ACTION_TOGGLE_VISIBILITY,
	FABULOR_TRAY_ACTION_SET_AWAY,
	FABULOR_TRAY_ACTION_SET_BACK,
	FABULOR_TRAY_ACTION_BLINK_CHANNEL,
	FABULOR_TRAY_ACTION_BLINK_PRIVATE,
	FABULOR_TRAY_ACTION_BLINK_HIGHLIGHT,
	FABULOR_TRAY_ACTION_PREFERENCES,
	FABULOR_TRAY_ACTION_QUIT
} FabulorTrayAction;

typedef enum
{
	FABULOR_TRAY_AWAY_MIXED,
	FABULOR_TRAY_AWAY_ALL_AWAY,
	FABULOR_TRAY_AWAY_ALL_BACK
} FabulorTrayAwayState;

typedef struct
{
	gboolean window_hidden;
	FabulorTrayAwayState away_state;
	gboolean blink_channel;
	gboolean blink_private;
	gboolean blink_highlight;
} FabulorTrayActionState;

typedef struct
{
	const char *hide_window;
	const char *restore_window;
	const char *blink_on;
	const char *channel_message;
	const char *private_message;
	const char *highlighted_message;
	const char *change_status;
	const char *away;
	const char *back;
	const char *preferences;
	const char *quit;
} FabulorTrayActionLabels;

typedef struct _FabulorTrayActionModel FabulorTrayActionModel;

typedef void (*FabulorTrayActionFunc) (FabulorTrayAction action,
	gboolean state, gpointer user_data);

FabulorTrayActionModel *fabulor_tray_action_model_new (
	const FabulorTrayActionLabels *labels,
	const FabulorTrayActionState *state,
	FabulorTrayActionFunc activate,
	gpointer user_data,
	GDestroyNotify destroy_notify);
void fabulor_tray_action_model_free (FabulorTrayActionModel *model);
void fabulor_tray_action_model_update (FabulorTrayActionModel *model,
	const FabulorTrayActionState *state);
GMenuModel *fabulor_tray_action_model_get_menu (FabulorTrayActionModel *model);
GActionGroup *fabulor_tray_action_model_get_actions (FabulorTrayActionModel *model);

#endif
