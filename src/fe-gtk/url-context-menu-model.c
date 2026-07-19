/* Copyright (C) 2026 Fabulor contributors */
#include "url-context-menu-model.h"
#include "menu-action-namespaces.h"

typedef struct
{
	FabulorUrlContextMenuModel *owner;
	FabulorUrlContextAction action;
} UrlActionData;

struct _FabulorUrlContextMenuModel
{
	GMenuModel *menu;
	GSimpleActionGroup *actions;
	char *url;
	FabulorUrlContextDispatch dispatch;
	gpointer user_data;
};

static void
url_action_activate (GSimpleAction *action, GVariant *parameter,
	gpointer user_data)
{
	UrlActionData *data = user_data;
	(void)action;
	(void)parameter;
	if (data->owner->dispatch)
		data->owner->dispatch (data->action, data->owner->url,
			data->owner->user_data);
}

static void
url_action_add (FabulorUrlContextMenuModel *model, const char *name,
	FabulorUrlContextAction action_id)
{
	GSimpleAction *action = g_simple_action_new (name, NULL);
	UrlActionData *data = g_new (UrlActionData, 1);
	data->owner = model;
	data->action = action_id;
	g_signal_connect_data (action, "activate", G_CALLBACK (url_action_activate),
		data, (GClosureNotify)g_free, 0);
	g_action_map_add_action (G_ACTION_MAP (model->actions), G_ACTION (action));
	g_object_unref (action);
}

FabulorUrlContextMenuModel *
fabulor_url_context_menu_model_new (const char *url, const char *open_label,
	const char *connect_label, const char *copy_label, GMenuModel *plugin_model,
	FabulorUrlContextDispatch dispatch, gpointer user_data)
{
	FabulorUrlContextMenuModel *result;
	GMenu *menu;
	GMenu *commands;
	GMenu *heading;
	gboolean is_irc;

	g_return_val_if_fail (url && *url, NULL);
	g_return_val_if_fail (open_label && connect_label && copy_label, NULL);
	g_return_val_if_fail (!plugin_model || G_IS_MENU_MODEL (plugin_model), NULL);
	result = g_new0 (FabulorUrlContextMenuModel, 1);
	result->url = g_strdup (url);
	result->dispatch = dispatch;
	result->user_data = user_data;
	result->actions = g_simple_action_group_new ();
	url_action_add (result, "open", FABULOR_URL_CONTEXT_OPEN);
	url_action_add (result, "copy", FABULOR_URL_CONTEXT_COPY);
	is_irc = g_str_has_prefix (url, "irc://") || g_str_has_prefix (url, "ircs://");
	heading = g_menu_new ();
	g_menu_append (heading, url, NULL);
	commands = g_menu_new ();
	g_menu_append (commands, is_irc ? connect_label : open_label, "context.open");
	g_menu_append (commands, copy_label, "context.copy");
	menu = g_menu_new ();
	g_menu_append_section (menu, NULL, G_MENU_MODEL (heading));
	g_menu_append_section (menu, NULL, G_MENU_MODEL (commands));
	if (plugin_model && g_menu_model_get_n_items (plugin_model) > 0)
		g_menu_append_section (menu, NULL, plugin_model);
	g_object_unref (heading);
	g_object_unref (commands);
	g_menu_freeze (menu);
	result->menu = G_MENU_MODEL (menu);
	return result;
}

void
fabulor_url_context_menu_model_free (FabulorUrlContextMenuModel *model)
{
	if (!model)
		return;
	g_clear_object (&model->menu);
	g_clear_object (&model->actions);
	g_free (model->url);
	g_free (model);
}

GMenuModel *
fabulor_url_context_menu_model_get_menu (FabulorUrlContextMenuModel *model)
{
	return model ? model->menu : NULL;
}

GActionGroup *
fabulor_url_context_menu_model_get_actions (FabulorUrlContextMenuModel *model)
{
	return model ? G_ACTION_GROUP (model->actions) : NULL;
}
