/* Copyright (C) 2026 Fabulor contributors */
#include "nick-context-menu-model.h"
#include "menu-action-namespaces.h"

typedef struct
{
	FabulorNickContextMenuModel *owner;
	FabulorNickContextAction action;
	char *command;
	gboolean toggle;
	gboolean selection_dispatch;
} NickActionData;

struct _FabulorNickContextMenuModel
{
	GMenuModel *menu;
	GSimpleActionGroup *actions;
	char *nick;
	FabulorNickContextDispatch dispatch;
	gpointer user_data;
	gboolean info_needs_refresh;
};

static void
nick_action_activate (GSimpleAction *action, GVariant *parameter,
	gpointer user_data)
{
	NickActionData *data = user_data;
	const char *command = data->command;
	char *toggle_command = NULL;
	(void)parameter;
	if (data->toggle)
	{
		GVariant *state = g_action_get_state (G_ACTION (action));
		gboolean active = !g_variant_get_boolean (state);
		g_variant_unref (state);
		g_simple_action_set_state (action, g_variant_new_boolean (active));
		toggle_command = g_strdup_printf ("set %s %d", data->command, active);
		command = toggle_command;
	}
	if (data->owner->dispatch)
		data->owner->dispatch (data->action, data->owner->nick, command,
			data->selection_dispatch, data->owner->user_data);
	g_free (toggle_command);
}

static void
nick_action_data_free (gpointer user_data, GClosure *closure)
{
	NickActionData *data = user_data;
	(void)closure;
	g_free (data->command);
	g_free (data);
}

static void
nick_action_add (FabulorNickContextMenuModel *model, const char *name,
	FabulorNickContextAction action_id, const char *command, gboolean enabled,
	gboolean toggle, gboolean active, gboolean selection_dispatch)
{
	GSimpleAction *action;
	NickActionData *data = g_new0 (NickActionData, 1);
	if (toggle)
		action = g_simple_action_new_stateful (name, NULL,
			g_variant_new_boolean (active));
	else
		action = g_simple_action_new (name, NULL);
	data->owner = model;
	data->action = action_id;
	data->command = g_strdup (command);
	data->toggle = toggle;
	data->selection_dispatch = selection_dispatch;
	g_simple_action_set_enabled (action, enabled);
	g_signal_connect_data (action, "activate", G_CALLBACK (nick_action_activate),
		data, nick_action_data_free, 0);
	g_action_map_add_action (G_ACTION_MAP (model->actions), G_ACTION (action));
	g_object_unref (action);
}

static void
nick_model_append_section (GMenu *model, GMenu **section)
{
	if (g_menu_model_get_n_items (G_MENU_MODEL (*section)) > 0)
		g_menu_append_section (model, NULL, G_MENU_MODEL (*section));
	g_object_unref (*section);
	*section = g_menu_new ();
}

static GMenuModel *
nick_handler_model_new (FabulorNickContextMenuModel *owner,
	const FabulorNickHandler *handlers, gsize handler_count, gsize *cursor,
	guint *action_index, gboolean selection_dispatch)
{
	GMenu *model = g_menu_new ();
	GMenu *section = g_menu_new ();

	while (*cursor < handler_count)
	{
		const FabulorNickHandler *handler = &handlers[(*cursor)++];
		if (handler->kind == FABULOR_NICK_HANDLER_SUBMENU_END)
			break;
		if (handler->kind == FABULOR_NICK_HANDLER_SEPARATOR)
		{
			nick_model_append_section (model, &section);
			continue;
		}
		if (handler->kind == FABULOR_NICK_HANDLER_SUBMENU_BEGIN)
		{
			GMenuModel *submenu = nick_handler_model_new (owner, handlers,
				handler_count, cursor, action_index, selection_dispatch);
			if (handler->label && g_menu_model_get_n_items (submenu) > 0)
				g_menu_append_submenu (section, handler->label, submenu);
			g_object_unref (submenu);
			continue;
		}
		if ((handler->kind == FABULOR_NICK_HANDLER_COMMAND ||
			handler->kind == FABULOR_NICK_HANDLER_TOGGLE) && handler->label &&
			handler->command)
		{
			GMenuItem *item;
			char *name = g_strdup_printf ("command-%u", (*action_index)++);
			char *detailed = g_strconcat (FABULOR_CONTEXT_ACTION_NAMESPACE,
				".", name, NULL);
			nick_action_add (owner, name, FABULOR_NICK_CONTEXT_COMMAND,
				handler->command, handler->enabled,
				handler->kind == FABULOR_NICK_HANDLER_TOGGLE, handler->active,
				handler->kind == FABULOR_NICK_HANDLER_COMMAND &&
					selection_dispatch);
			item = g_menu_item_new (handler->label, detailed);
			if (handler->icon && *handler->icon)
				g_menu_item_set_attribute (item, "fabulor-icon", "s",
					handler->icon);
			g_menu_append_item (section, item);
			g_object_unref (item);
			g_free (detailed);
			g_free (name);
		}
	}
	nick_model_append_section (model, &section);
	g_object_unref (section);
	g_menu_freeze (model);
	return G_MENU_MODEL (model);
}

static void
nick_model_append_items (GMenu *destination, GMenuModel *source)
{
	gint i;
	for (i = 0; source && i < g_menu_model_get_n_items (source); i++)
	{
		GMenuItem *item = g_menu_item_new_from_model (source, i);
		g_menu_append_item (destination, item);
		g_object_unref (item);
	}
}

static void
nick_model_append_heading (FabulorNickContextMenuModel *owner, GMenu *menu,
	const char *heading, const FabulorNickInfoItem *info_items,
	gsize info_count)
{
	GMenu *heading_section;
	gsize i;

	if (!heading || !*heading)
		return;
	heading_section = g_menu_new ();
	if (info_count == 0)
		g_menu_append (heading_section, heading, NULL);
	else
	{
		GMenu *info_menu = g_menu_new ();
		for (i = 0; i < info_count; i++)
		{
			const FabulorNickInfoItem *info = &info_items[i];
			GMenuItem *item;
			char *name = NULL;
			char *detailed = NULL;
			if (!info->label || !*info->label)
				continue;
			if (info->value)
			{
				name = g_strdup_printf ("info-%" G_GSIZE_FORMAT, i);
				detailed = g_strconcat (FABULOR_CONTEXT_ACTION_NAMESPACE,
					".", name, NULL);
				nick_action_add (owner, name, FABULOR_NICK_CONTEXT_COPY_INFO,
					info->value, TRUE, FALSE, FALSE, FALSE);
			}
			item = g_menu_item_new (info->label, detailed);
			g_menu_append_item (info_menu, item);
			g_object_unref (item);
			g_free (detailed);
			g_free (name);
		}
		g_menu_append_submenu (heading_section, heading,
			G_MENU_MODEL (info_menu));
		g_object_unref (info_menu);
	}
	g_menu_append_section (menu, NULL, G_MENU_MODEL (heading_section));
	g_object_unref (heading_section);
}

FabulorNickContextMenuModel *
fabulor_nick_context_menu_model_new (const char *nick, const char *heading,
	gboolean show_reply, const char *reply_label, GMenuModel *plugin_model,
	FabulorNickContextDispatch dispatch, gpointer user_data)
{
	return fabulor_nick_context_menu_model_new_with_handlers (nick, heading,
		show_reply, reply_label, FALSE, NULL, 0, plugin_model, dispatch,
		user_data);
}

FabulorNickContextMenuModel *
fabulor_nick_context_menu_model_new_with_handlers (const char *nick,
	const char *heading, gboolean show_reply, const char *reply_label,
	gboolean selection_dispatch, const FabulorNickHandler *handlers,
	gsize handler_count, GMenuModel *plugin_model,
	FabulorNickContextDispatch dispatch, gpointer user_data)
{
	return fabulor_nick_context_menu_model_new_with_details (nick, heading,
		show_reply, reply_label, selection_dispatch, handlers, handler_count,
		NULL, 0, FALSE, plugin_model, dispatch, user_data);
}

FabulorNickContextMenuModel *
fabulor_nick_context_menu_model_new_with_details (const char *nick,
	const char *heading, gboolean show_reply, const char *reply_label,
	gboolean selection_dispatch, const FabulorNickHandler *handlers,
	gsize handler_count, const FabulorNickInfoItem *info_items,
	gsize info_count, gboolean info_needs_refresh, GMenuModel *plugin_model,
	FabulorNickContextDispatch dispatch, gpointer user_data)
{
	FabulorNickContextMenuModel *result;
	GMenu *menu = g_menu_new ();
	GMenuModel *handler_model = NULL;
	gsize handler_cursor = 0;
	guint action_index = 0;

	g_return_val_if_fail (nick && *nick, NULL);
	g_return_val_if_fail (!show_reply || reply_label, NULL);
	g_return_val_if_fail (handler_count == 0 || handlers, NULL);
	g_return_val_if_fail (info_count == 0 || info_items, NULL);
	g_return_val_if_fail (!plugin_model || G_IS_MENU_MODEL (plugin_model), NULL);
	result = g_new0 (FabulorNickContextMenuModel, 1);
	result->nick = g_strdup (nick);
	result->dispatch = dispatch;
	result->user_data = user_data;
	result->info_needs_refresh = info_needs_refresh;
	result->actions = g_simple_action_group_new ();
	nick_model_append_heading (result, menu, heading, info_items, info_count);
	if (handler_count > 0)
	{
		handler_model = nick_handler_model_new (result, handlers, handler_count,
			&handler_cursor, &action_index, selection_dispatch);
		nick_model_append_items (menu, handler_model);
		g_object_unref (handler_model);
	}
	if (show_reply)
	{
		GMenu *reply_section = g_menu_new ();
		nick_action_add (result, "reply", FABULOR_NICK_CONTEXT_REPLY, NULL,
			TRUE, FALSE, FALSE, FALSE);
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

gboolean
fabulor_nick_context_menu_model_needs_info_refresh (
	FabulorNickContextMenuModel *model)
{
	return model && model->info_needs_refresh;
}
