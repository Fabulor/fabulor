/* Copyright (C) 2026 Fabulor contributors */
#include "channel-context-menu-model.h"
#include "menu-action-namespaces.h"

typedef struct
{
	FabulorChannelContextMenuModel *owner;
	FabulorChannelContextAction action;
	gboolean stateful;
} ChannelActionData;

struct _FabulorChannelContextMenuModel
{
	GMenuModel *menu;
	GSimpleActionGroup *actions;
	char *channel;
	FabulorChannelContextDispatch dispatch;
	gpointer user_data;
};

static void
channel_action_activate (GSimpleAction *action, GVariant *parameter,
	gpointer user_data)
{
	ChannelActionData *data = user_data;
	gboolean state = FALSE;
	(void)parameter;
	if (data->stateful)
	{
		GVariant *current = g_action_get_state (G_ACTION (action));
		state = !g_variant_get_boolean (current);
		g_variant_unref (current);
		g_simple_action_set_state (action, g_variant_new_boolean (state));
	}
	if (data->owner->dispatch)
		data->owner->dispatch (data->action, data->owner->channel, state,
			data->owner->user_data);
}

static void
channel_action_add (FabulorChannelContextMenuModel *model, const char *name,
	FabulorChannelContextAction action_id, gboolean stateful, gboolean state)
{
	GSimpleAction *action;
	ChannelActionData *data = g_new0 (ChannelActionData, 1);
	if (stateful)
		action = g_simple_action_new_stateful (name, NULL,
			g_variant_new_boolean (state));
	else
		action = g_simple_action_new (name, NULL);
	data->owner = model;
	data->action = action_id;
	data->stateful = stateful;
	g_signal_connect_data (action, "activate", G_CALLBACK (channel_action_activate),
		data, (GClosureNotify)g_free, 0);
	g_action_map_add_action (G_ACTION_MAP (model->actions), G_ACTION (action));
	g_object_unref (action);
}

FabulorChannelContextMenuModel *
fabulor_channel_context_menu_model_new (const char *channel, gboolean joined,
	gboolean current, gboolean has_network, gboolean autojoin,
	const char *join_label, const char *focus_label, const char *part_label,
	const char *cycle_label, const char *autojoin_label,
	GMenuModel *plugin_model, FabulorChannelContextDispatch dispatch,
	gpointer user_data)
{
	FabulorChannelContextMenuModel *result;
	GMenu *menu = g_menu_new ();
	GMenu *heading = g_menu_new ();
	GMenu *commands = g_menu_new ();

	g_return_val_if_fail (channel && *channel, NULL);
	g_return_val_if_fail (join_label && focus_label && part_label &&
		cycle_label && autojoin_label, NULL);
	g_return_val_if_fail (!plugin_model || G_IS_MENU_MODEL (plugin_model), NULL);
	result = g_new0 (FabulorChannelContextMenuModel, 1);
	result->channel = g_strdup (channel);
	result->dispatch = dispatch;
	result->user_data = user_data;
	result->actions = g_simple_action_group_new ();
	g_menu_append (heading, channel, NULL);
	if (!joined)
	{
		channel_action_add (result, "join", FABULOR_CHANNEL_CONTEXT_JOIN,
			FALSE, FALSE);
		g_menu_append (commands, join_label,
			FABULOR_CONTEXT_ACTION_NAMESPACE ".join");
	}
	else
	{
		if (!current)
		{
			channel_action_add (result, "focus", FABULOR_CHANNEL_CONTEXT_FOCUS,
				FALSE, FALSE);
			g_menu_append (commands, focus_label,
				FABULOR_CONTEXT_ACTION_NAMESPACE ".focus");
		}
		channel_action_add (result, "part", FABULOR_CHANNEL_CONTEXT_PART,
			FALSE, FALSE);
		channel_action_add (result, "cycle", FABULOR_CHANNEL_CONTEXT_CYCLE,
			FALSE, FALSE);
		g_menu_append (commands, part_label,
			FABULOR_CONTEXT_ACTION_NAMESPACE ".part");
		g_menu_append (commands, cycle_label,
			FABULOR_CONTEXT_ACTION_NAMESPACE ".cycle");
	}
	g_menu_append_section (menu, NULL, G_MENU_MODEL (heading));
	g_menu_append_section (menu, NULL, G_MENU_MODEL (commands));
	if (has_network)
	{
		GMenu *favorite = g_menu_new ();
		channel_action_add (result, "autojoin",
			FABULOR_CHANNEL_CONTEXT_AUTOJOIN, TRUE, autojoin);
		g_menu_append (favorite, autojoin_label,
			FABULOR_CONTEXT_ACTION_NAMESPACE ".autojoin");
		g_menu_append_section (menu, NULL, G_MENU_MODEL (favorite));
		g_object_unref (favorite);
	}
	if (plugin_model && g_menu_model_get_n_items (plugin_model) > 0)
		g_menu_append_section (menu, NULL, plugin_model);
	g_object_unref (heading);
	g_object_unref (commands);
	g_menu_freeze (menu);
	result->menu = G_MENU_MODEL (menu);
	return result;
}

void
fabulor_channel_context_menu_model_free (FabulorChannelContextMenuModel *model)
{
	if (!model)
		return;
	g_clear_object (&model->menu);
	g_clear_object (&model->actions);
	g_free (model->channel);
	g_free (model);
}

GMenuModel *
fabulor_channel_context_menu_model_get_menu (FabulorChannelContextMenuModel *model)
{
	return model ? model->menu : NULL;
}

GActionGroup *
fabulor_channel_context_menu_model_get_actions (FabulorChannelContextMenuModel *model)
{
	return model ? G_ACTION_GROUP (model->actions) : NULL;
}
