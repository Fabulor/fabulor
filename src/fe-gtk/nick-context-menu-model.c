/* Copyright (C) 2026 Fabulor contributors */
#include "nick-context-menu-model.h"
#include "menu-action-namespaces.h"

struct _FabulorNickContextMenuModel
{
	GMenuModel *menu;
	GSimpleActionGroup *actions;
	char *nick;
	FabulorNickContextDispatch dispatch;
	gpointer user_data;
};

static void
nick_reply_activate (GSimpleAction *action, GVariant *parameter,
	gpointer user_data)
{
	FabulorNickContextMenuModel *model = user_data;
	(void)action;
	(void)parameter;
	if (model->dispatch)
		model->dispatch (FABULOR_NICK_CONTEXT_REPLY, model->nick,
			model->user_data);
}

FabulorNickContextMenuModel *
fabulor_nick_context_menu_model_new (const char *nick, const char *heading,
	gboolean show_reply, const char *reply_label, GMenuModel *plugin_model,
	FabulorNickContextDispatch dispatch, gpointer user_data)
{
	FabulorNickContextMenuModel *result;
	GMenu *menu = g_menu_new ();

	g_return_val_if_fail (nick && *nick, NULL);
	g_return_val_if_fail (!show_reply || reply_label, NULL);
	g_return_val_if_fail (!plugin_model || G_IS_MENU_MODEL (plugin_model), NULL);
	result = g_new0 (FabulorNickContextMenuModel, 1);
	result->nick = g_strdup (nick);
	result->dispatch = dispatch;
	result->user_data = user_data;
	result->actions = g_simple_action_group_new ();
	if (heading && *heading)
	{
		GMenu *heading_section = g_menu_new ();
		g_menu_append (heading_section, heading, NULL);
		g_menu_append_section (menu, NULL, G_MENU_MODEL (heading_section));
		g_object_unref (heading_section);
	}
	if (show_reply)
	{
		GMenu *reply_section = g_menu_new ();
		GSimpleAction *reply = g_simple_action_new ("reply", NULL);
		g_signal_connect (reply, "activate", G_CALLBACK (nick_reply_activate),
			result);
		g_action_map_add_action (G_ACTION_MAP (result->actions), G_ACTION (reply));
		g_object_unref (reply);
		g_menu_append (reply_section, reply_label,
			FABULOR_CONTEXT_ACTION_NAMESPACE ".reply");
		g_menu_append_section (menu, NULL, G_MENU_MODEL (reply_section));
		g_object_unref (reply_section);
	}
	if (plugin_model && g_menu_model_get_n_items (plugin_model) > 0)
		g_menu_append_section (menu, NULL, plugin_model);
	g_menu_freeze (menu);
	result->menu = G_MENU_MODEL (menu);
	return result;
}

void
fabulor_nick_context_menu_model_free (FabulorNickContextMenuModel *model)
{
	if (!model)
		return;
	g_clear_object (&model->menu);
	g_clear_object (&model->actions);
	g_free (model->nick);
	g_free (model);
}

GMenuModel *
fabulor_nick_context_menu_model_get_menu (FabulorNickContextMenuModel *model)
{
	return model ? model->menu : NULL;
}

GActionGroup *
fabulor_nick_context_menu_model_get_actions (FabulorNickContextMenuModel *model)
{
	return model ? G_ACTION_GROUP (model->actions) : NULL;
}
