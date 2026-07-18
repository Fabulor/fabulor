/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "tray-action-model.h"

#include <string.h>

#define TRAY_ACTION_COUNT 8

typedef struct
{
	const char *name;
	FabulorTrayAction action;
	gboolean stateful;
} TrayActionDefinition;

typedef struct
{
	char *hide_window;
	char *restore_window;
	char *blink_on;
	char *channel_message;
	char *private_message;
	char *highlighted_message;
	char *change_status;
	char *away;
	char *back;
	char *preferences;
	char *quit;
} TrayActionLabels;

struct _FabulorTrayActionModel
{
	GMenu *menu;
	GSimpleActionGroup *actions;
	TrayActionLabels labels;
	FabulorTrayActionState state;
	FabulorTrayActionFunc activate;
	gpointer user_data;
	GDestroyNotify destroy_notify;
	gboolean initialized;
	gulong action_handler_ids[TRAY_ACTION_COUNT];
};

static const TrayActionDefinition tray_action_definitions[] = {
	{ "toggle-visibility", FABULOR_TRAY_ACTION_TOGGLE_VISIBILITY, FALSE },
	{ "set-away", FABULOR_TRAY_ACTION_SET_AWAY, FALSE },
	{ "set-back", FABULOR_TRAY_ACTION_SET_BACK, FALSE },
	{ "blink-channel", FABULOR_TRAY_ACTION_BLINK_CHANNEL, TRUE },
	{ "blink-private", FABULOR_TRAY_ACTION_BLINK_PRIVATE, TRUE },
	{ "blink-highlight", FABULOR_TRAY_ACTION_BLINK_HIGHLIGHT, TRUE },
	{ "preferences", FABULOR_TRAY_ACTION_PREFERENCES, FALSE },
	{ "quit", FABULOR_TRAY_ACTION_QUIT, FALSE }
};

G_STATIC_ASSERT (G_N_ELEMENTS (tray_action_definitions) == TRAY_ACTION_COUNT);

static gboolean
tray_action_state_value (const FabulorTrayActionState *state,
	FabulorTrayAction action)
{
	switch (action)
	{
	case FABULOR_TRAY_ACTION_BLINK_CHANNEL:
		return state->blink_channel;
	case FABULOR_TRAY_ACTION_BLINK_PRIVATE:
		return state->blink_private;
	case FABULOR_TRAY_ACTION_BLINK_HIGHLIGHT:
		return state->blink_highlight;
	default:
		return FALSE;
	}
}

static void
tray_action_state_set (FabulorTrayActionState *state,
	FabulorTrayAction action, gboolean value)
{
	switch (action)
	{
	case FABULOR_TRAY_ACTION_BLINK_CHANNEL:
		state->blink_channel = value;
		break;
	case FABULOR_TRAY_ACTION_BLINK_PRIVATE:
		state->blink_private = value;
		break;
	case FABULOR_TRAY_ACTION_BLINK_HIGHLIGHT:
		state->blink_highlight = value;
		break;
	default:
		break;
	}
}

static GSimpleAction *
tray_action_lookup (FabulorTrayActionModel *model, const char *name)
{
	return G_SIMPLE_ACTION (g_action_map_lookup_action (
		G_ACTION_MAP (model->actions), name));
}

static void
tray_action_activate_cb (GSimpleAction *simple_action, GVariant *parameter,
	gpointer user_data)
{
	FabulorTrayActionModel *model = user_data;
	FabulorTrayAction action;
	gboolean value = FALSE;
	guint i;

	(void)parameter;
	for (i = 0; i < G_N_ELEMENTS (tray_action_definitions); i++)
	{
		if (!strcmp (g_action_get_name (G_ACTION (simple_action)),
		             tray_action_definitions[i].name))
		{
			action = tray_action_definitions[i].action;
			if (tray_action_definitions[i].stateful)
			{
				value = !tray_action_state_value (&model->state, action);
				tray_action_state_set (&model->state, action, value);
				g_simple_action_set_state (simple_action,
				                           g_variant_new_boolean (value));
			}
			if (model->activate)
				model->activate (action, value, model->user_data);
			return;
		}
	}
}

static void
tray_action_append (GMenu *menu, const char *label, const char *name)
{
	char *detailed_action;

	detailed_action = g_strconcat (FABULOR_TRAY_ACTION_NAMESPACE, ".", name, NULL);
	g_menu_append (menu, label, detailed_action);
	g_free (detailed_action);
}

static void
tray_action_rebuild_menu (FabulorTrayActionModel *model)
{
	GMenu *section;
	GMenu *submenu;

	g_menu_remove_all (model->menu);

	section = g_menu_new ();
	tray_action_append (section,
		model->state.window_hidden ? model->labels.restore_window : model->labels.hide_window,
		"toggle-visibility");
	g_menu_append_section (model->menu, NULL, G_MENU_MODEL (section));
	g_object_unref (section);

	section = g_menu_new ();
	submenu = g_menu_new ();
	tray_action_append (submenu, model->labels.channel_message, "blink-channel");
	tray_action_append (submenu, model->labels.private_message, "blink-private");
	tray_action_append (submenu, model->labels.highlighted_message, "blink-highlight");
	g_menu_append_submenu (section, model->labels.blink_on, G_MENU_MODEL (submenu));
	g_object_unref (submenu);

	submenu = g_menu_new ();
	tray_action_append (submenu, model->labels.away, "set-away");
	tray_action_append (submenu, model->labels.back, "set-back");
	g_menu_append_submenu (section, model->labels.change_status, G_MENU_MODEL (submenu));
	g_object_unref (submenu);
	g_menu_append_section (model->menu, NULL, G_MENU_MODEL (section));
	g_object_unref (section);

	section = g_menu_new ();
	tray_action_append (section, model->labels.preferences, "preferences");
	tray_action_append (section, model->labels.quit, "quit");
	g_menu_append_section (model->menu, NULL, G_MENU_MODEL (section));
	g_object_unref (section);
}

static gboolean
tray_labels_valid (const FabulorTrayActionLabels *labels)
{
	return labels && labels->hide_window && labels->restore_window &&
		labels->blink_on && labels->channel_message && labels->private_message &&
		labels->highlighted_message && labels->change_status && labels->away &&
		labels->back && labels->preferences && labels->quit;
}

static void
tray_labels_copy (TrayActionLabels *destination,
	const FabulorTrayActionLabels *source)
{
	destination->hide_window = g_strdup (source->hide_window);
	destination->restore_window = g_strdup (source->restore_window);
	destination->blink_on = g_strdup (source->blink_on);
	destination->channel_message = g_strdup (source->channel_message);
	destination->private_message = g_strdup (source->private_message);
	destination->highlighted_message = g_strdup (source->highlighted_message);
	destination->change_status = g_strdup (source->change_status);
	destination->away = g_strdup (source->away);
	destination->back = g_strdup (source->back);
	destination->preferences = g_strdup (source->preferences);
	destination->quit = g_strdup (source->quit);
}

static void
tray_labels_clear (TrayActionLabels *labels)
{
	g_free (labels->hide_window);
	g_free (labels->restore_window);
	g_free (labels->blink_on);
	g_free (labels->channel_message);
	g_free (labels->private_message);
	g_free (labels->highlighted_message);
	g_free (labels->change_status);
	g_free (labels->away);
	g_free (labels->back);
	g_free (labels->preferences);
	g_free (labels->quit);
}

FabulorTrayActionModel *
fabulor_tray_action_model_new (const FabulorTrayActionLabels *labels,
	const FabulorTrayActionState *state, FabulorTrayActionFunc activate,
	gpointer user_data, GDestroyNotify destroy_notify)
{
	FabulorTrayActionModel *model;
	guint i;

	g_return_val_if_fail (tray_labels_valid (labels), NULL);
	g_return_val_if_fail (state != NULL, NULL);

	model = g_new0 (FabulorTrayActionModel, 1);
	model->menu = g_menu_new ();
	model->actions = g_simple_action_group_new ();
	tray_labels_copy (&model->labels, labels);
	model->activate = activate;
	model->user_data = user_data;
	model->destroy_notify = destroy_notify;

	for (i = 0; i < G_N_ELEMENTS (tray_action_definitions); i++)
	{
		GSimpleAction *action;

		if (tray_action_definitions[i].stateful)
			action = g_simple_action_new_stateful (tray_action_definitions[i].name,
				NULL, g_variant_new_boolean (FALSE));
		else
			action = g_simple_action_new (tray_action_definitions[i].name, NULL);
		model->action_handler_ids[i] = g_signal_connect (action, "activate",
			G_CALLBACK (tray_action_activate_cb), model);
		g_action_map_add_action (G_ACTION_MAP (model->actions), G_ACTION (action));
		g_object_unref (action);
	}

	fabulor_tray_action_model_update (model, state);
	return model;
}

void
fabulor_tray_action_model_free (FabulorTrayActionModel *model)
{
	guint i;

	if (!model)
		return;

	for (i = 0; i < G_N_ELEMENTS (tray_action_definitions); i++)
	{
		GSimpleAction *action = tray_action_lookup (
			model, tray_action_definitions[i].name);

		g_simple_action_set_enabled (action, FALSE);
		if (model->action_handler_ids[i])
			g_signal_handler_disconnect (action, model->action_handler_ids[i]);
	}
	g_clear_object (&model->menu);
	g_clear_object (&model->actions);
	tray_labels_clear (&model->labels);
	if (model->destroy_notify)
		model->destroy_notify (model->user_data);
	g_free (model);
}

void
fabulor_tray_action_model_update (FabulorTrayActionModel *model,
	const FabulorTrayActionState *state)
{
	FabulorTrayActionState normalized;
	FabulorTrayActionState previous;
	gboolean away_changed;
	gboolean blink_changed;
	gboolean window_changed;
	guint i;

	g_return_if_fail (model != NULL);
	g_return_if_fail (state != NULL);

	normalized = *state;
	if (normalized.away_state < FABULOR_TRAY_AWAY_MIXED ||
	    normalized.away_state > FABULOR_TRAY_AWAY_ALL_BACK)
		normalized.away_state = FABULOR_TRAY_AWAY_MIXED;

	previous = model->state;
	window_changed = !model->initialized ||
		previous.window_hidden != normalized.window_hidden;
	away_changed = !model->initialized ||
		previous.away_state != normalized.away_state;
	blink_changed = !model->initialized ||
		previous.blink_channel != normalized.blink_channel ||
		previous.blink_private != normalized.blink_private ||
		previous.blink_highlight != normalized.blink_highlight;
	if (!window_changed && !away_changed && !blink_changed)
		return;

	model->state = normalized;

	if (blink_changed)
	{
		for (i = 0; i < G_N_ELEMENTS (tray_action_definitions); i++)
		{
			GSimpleAction *action = tray_action_lookup (
				model, tray_action_definitions[i].name);

			if (tray_action_definitions[i].stateful)
				g_simple_action_set_state (action, g_variant_new_boolean (
					tray_action_state_value (&model->state,
					                         tray_action_definitions[i].action)));
		}
	}

	if (away_changed)
	{
		g_simple_action_set_enabled (tray_action_lookup (model, "set-away"),
			model->state.away_state != FABULOR_TRAY_AWAY_ALL_AWAY);
		g_simple_action_set_enabled (tray_action_lookup (model, "set-back"),
			model->state.away_state != FABULOR_TRAY_AWAY_ALL_BACK);
	}
	if (window_changed)
		tray_action_rebuild_menu (model);
	model->initialized = TRUE;
}

GMenuModel *
fabulor_tray_action_model_get_menu (FabulorTrayActionModel *model)
{
	g_return_val_if_fail (model != NULL, NULL);
	return G_MENU_MODEL (model->menu);
}

GActionGroup *
fabulor_tray_action_model_get_actions (FabulorTrayActionModel *model)
{
	g_return_val_if_fail (model != NULL, NULL);
	return G_ACTION_GROUP (model->actions);
}
