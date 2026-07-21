/* Copyright (C) 2026 Fabulor contributors */
#include "channel-list-context-menu-model.h"
#include "menu-action-namespaces.h"

typedef struct
{
	FabulorChannelListContextMenuModel *owner;
	FabulorChannelListContextAction action;
	gboolean stateful;
} ChannelListActionData;

struct _FabulorChannelListContextMenuModel
{
	GMenuModel *menu;
	GSimpleActionGroup *actions;
	GPtrArray *channels;
	GPtrArray *topics;
	FabulorChannelListContextDispatch dispatch;
	gpointer user_data;
};

static GPtrArray *
channel_list_context_dup_strings (const GPtrArray *source)
{
	GPtrArray *copy = g_ptr_array_new_with_free_func (g_free);
	guint i;

	for (i = 0; source && i < source->len; i++)
		g_ptr_array_add (copy, g_strdup (g_ptr_array_index (source, i)));
	return copy;
}

static void
channel_list_context_action_activate (GSimpleAction *action,
	GVariant *parameter, gpointer user_data)
{
	ChannelListActionData *data = user_data;
	gboolean state = FALSE;

	(void) parameter;
	if (data->stateful)
	{
		GVariant *current = g_action_get_state (G_ACTION (action));

		state = !g_variant_get_boolean (current);
		g_variant_unref (current);
		g_simple_action_set_state (action, g_variant_new_boolean (state));
	}
	if (data->owner->dispatch)
		data->owner->dispatch (data->action, state,
			data->owner->channels, data->owner->topics,
			data->owner->user_data);
}

static void
channel_list_context_add_action (FabulorChannelListContextMenuModel *model,
	const char *name, FabulorChannelListContextAction action_id,
	gboolean stateful, gboolean state)
{
	ChannelListActionData *data = g_new0 (ChannelListActionData, 1);
	GSimpleAction *action;

	if (stateful)
		action = g_simple_action_new_stateful (name, NULL,
			g_variant_new_boolean (state));
	else
		action = g_simple_action_new (name, NULL);
	data->owner = model;
	data->action = action_id;
	data->stateful = stateful;
	g_signal_connect_data (action, "activate",
		G_CALLBACK (channel_list_context_action_activate), data,
		(GClosureNotify) g_free, 0);
	g_action_map_add_action (G_ACTION_MAP (model->actions), G_ACTION (action));
	g_object_unref (action);
}

static void
channel_list_context_append_item (GMenu *menu, const char *label,
	const char *detailed_action, const char *icon_name)
{
	GMenuItem *item = g_menu_item_new (label, detailed_action);

	if (icon_name && *icon_name)
	{
		GIcon *icon = g_themed_icon_new (icon_name);

		g_menu_item_set_icon (item, icon);
		g_object_unref (icon);
	}
	g_menu_append_item (menu, item);
	g_object_unref (item);
}

FabulorChannelListContextMenuModel *
fabulor_channel_list_context_menu_model_new (const GPtrArray *channels,
	const GPtrArray *topics, gboolean has_network, gboolean autojoin,
	const FabulorChannelListContextLabels *labels,
	FabulorChannelListContextDispatch dispatch, gpointer user_data)
{
	FabulorChannelListContextMenuModel *model;
	GMenu *menu;
	GMenu *commands;

	g_return_val_if_fail (channels && channels->len > 0, NULL);
	g_return_val_if_fail (topics != NULL, NULL);
	g_return_val_if_fail (labels && labels->join && labels->copy_channels &&
		labels->copy_topics && labels->autojoin, NULL);
	model = g_new0 (FabulorChannelListContextMenuModel, 1);
	model->channels = channel_list_context_dup_strings (channels);
	model->topics = channel_list_context_dup_strings (topics);
	model->dispatch = dispatch;
	model->user_data = user_data;
	model->actions = g_simple_action_group_new ();
	menu = g_menu_new ();
	commands = g_menu_new ();

	channel_list_context_add_action (model, "join",
		FABULOR_CHANNEL_LIST_CONTEXT_JOIN, FALSE, FALSE);
	channel_list_context_add_action (model, "copy-channels",
		FABULOR_CHANNEL_LIST_CONTEXT_COPY_CHANNELS, FALSE, FALSE);
	channel_list_context_add_action (model, "copy-topics",
		FABULOR_CHANNEL_LIST_CONTEXT_COPY_TOPICS, FALSE, FALSE);
	channel_list_context_append_item (commands, labels->join,
		FABULOR_CONTEXT_ACTION_NAMESPACE ".join", labels->join_icon);
	channel_list_context_append_item (commands, labels->copy_channels,
		FABULOR_CONTEXT_ACTION_NAMESPACE ".copy-channels", labels->copy_icon);
	channel_list_context_append_item (commands, labels->copy_topics,
		FABULOR_CONTEXT_ACTION_NAMESPACE ".copy-topics", labels->copy_icon);
	g_menu_append_section (menu, NULL, G_MENU_MODEL (commands));
	if (has_network)
	{
		GMenu *favorite = g_menu_new ();

		channel_list_context_add_action (model, "autojoin",
			FABULOR_CHANNEL_LIST_CONTEXT_AUTOJOIN, TRUE, autojoin);
		g_menu_append (favorite, labels->autojoin,
			FABULOR_CONTEXT_ACTION_NAMESPACE ".autojoin");
		g_menu_append_section (menu, NULL, G_MENU_MODEL (favorite));
		g_object_unref (favorite);
	}
	g_object_unref (commands);
	g_menu_freeze (menu);
	model->menu = G_MENU_MODEL (menu);
	return model;
}

void
fabulor_channel_list_context_menu_model_free (
	FabulorChannelListContextMenuModel *model)
{
	if (!model)
		return;
	g_clear_object (&model->menu);
	g_clear_object (&model->actions);
	g_ptr_array_unref (model->channels);
	g_ptr_array_unref (model->topics);
	g_free (model);
}

GMenuModel *
fabulor_channel_list_context_menu_model_get_menu (
	FabulorChannelListContextMenuModel *model)
{
	return model ? model->menu : NULL;
}

GActionGroup *
fabulor_channel_list_context_menu_model_get_actions (
	FabulorChannelListContextMenuModel *model)
{
	return model ? G_ACTION_GROUP (model->actions) : NULL;
}
