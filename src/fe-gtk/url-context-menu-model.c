/* Copyright (C) 2026 Fabulor contributors */
#include "url-context-menu-model.h"
#include "menu-action-namespaces.h"

typedef struct
{
	FabulorUrlContextMenuModel *owner;
	FabulorUrlContextAction action;
	char *command;
	gboolean toggle;
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
	const char *command = data->command;
	char *toggle_command = NULL;
	(void)action;
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
		data->owner->dispatch (data->action, data->owner->url,
			command, data->owner->user_data);
	g_free (toggle_command);
}

static void
url_action_data_free (gpointer user_data, GClosure *closure)
{
	UrlActionData *data = user_data;
	(void)closure;
	g_free (data->command);
	g_free (data);
}

static void
url_action_add (FabulorUrlContextMenuModel *model, const char *name,
	FabulorUrlContextAction action_id, const char *command, gboolean enabled)
{
	GSimpleAction *action = g_simple_action_new (name, NULL);
	UrlActionData *data = g_new0 (UrlActionData, 1);
	data->owner = model;
	data->action = action_id;
	data->command = g_strdup (command);
	g_simple_action_set_enabled (action, enabled);
	g_signal_connect_data (action, "activate", G_CALLBACK (url_action_activate),
		data, url_action_data_free, 0);
	g_action_map_add_action (G_ACTION_MAP (model->actions), G_ACTION (action));
	g_object_unref (action);
}

static void
url_toggle_action_add (FabulorUrlContextMenuModel *model, const char *name,
	const char *preference, gboolean enabled, gboolean active)
{
	GSimpleAction *action = g_simple_action_new_stateful (name, NULL,
		g_variant_new_boolean (active));
	UrlActionData *data = g_new0 (UrlActionData, 1);
	data->owner = model;
	data->action = FABULOR_URL_CONTEXT_HANDLER;
	data->command = g_strdup (preference);
	data->toggle = TRUE;
	g_simple_action_set_enabled (action, enabled);
	g_signal_connect_data (action, "activate", G_CALLBACK (url_action_activate),
		data, url_action_data_free, 0);
	g_action_map_add_action (G_ACTION_MAP (model->actions), G_ACTION (action));
	g_object_unref (action);
}

static void
url_model_append_section (GMenu *model, GMenu **section)
{
	if (g_menu_model_get_n_items (G_MENU_MODEL (*section)) > 0)
		g_menu_append_section (model, NULL, G_MENU_MODEL (*section));
	g_object_unref (*section);
	*section = g_menu_new ();
}

static GMenuModel *
url_handler_model_new (FabulorUrlContextMenuModel *owner,
	const FabulorUrlHandler *handlers, gsize handler_count, gsize *cursor,
	guint *action_index)
{
	GMenu *model = g_menu_new ();
	GMenu *section = g_menu_new ();

	while (*cursor < handler_count)
	{
		const FabulorUrlHandler *handler = &handlers[(*cursor)++];

		if (handler->kind == FABULOR_URL_HANDLER_SUBMENU_END)
			break;
		if (handler->kind == FABULOR_URL_HANDLER_SEPARATOR)
		{
			url_model_append_section (model, &section);
			continue;
		}
		if (handler->kind == FABULOR_URL_HANDLER_SUBMENU_BEGIN)
		{
			GMenuModel *submenu = url_handler_model_new (owner, handlers,
				handler_count, cursor, action_index);
			if (handler->label && g_menu_model_get_n_items (submenu) > 0)
				g_menu_append_submenu (section, handler->label, submenu);
			g_object_unref (submenu);
			continue;
		}
		if ((handler->kind == FABULOR_URL_HANDLER_COMMAND ||
			handler->kind == FABULOR_URL_HANDLER_TOGGLE) && handler->label &&
			handler->command)
		{
			GMenuItem *item;
			char *name = g_strdup_printf ("handler-%u", (*action_index)++);
			char *detailed = g_strconcat (FABULOR_CONTEXT_ACTION_NAMESPACE,
				".", name, NULL);
			if (handler->kind == FABULOR_URL_HANDLER_TOGGLE)
				url_toggle_action_add (owner, name, handler->command,
					handler->enabled, handler->active);
			else
				url_action_add (owner, name, FABULOR_URL_CONTEXT_HANDLER,
					handler->command, handler->enabled);
			item = g_menu_item_new (handler->label, detailed);
			if (handler->icon && *handler->icon)
				g_menu_item_set_attribute (item, "fabulor-icon", "s", handler->icon);
			g_menu_append_item (section, item);
			g_object_unref (item);
			g_free (detailed);
			g_free (name);
		}
	}
	url_model_append_section (model, &section);
	g_object_unref (section);
	g_menu_freeze (model);
	return G_MENU_MODEL (model);
}

static void
url_model_append_items (GMenu *destination, GMenuModel *source)
{
	gint i;
	for (i = 0; source && i < g_menu_model_get_n_items (source); i++)
	{
		GMenuItem *item = g_menu_item_new_from_model (source, i);
		g_menu_append_item (destination, item);
		g_object_unref (item);
	}
}

FabulorUrlContextMenuModel *
fabulor_url_context_menu_model_new (const char *url, const char *open_label,
	const char *connect_label, const char *copy_label, GMenuModel *plugin_model,
	FabulorUrlContextDispatch dispatch, gpointer user_data)
{
	return fabulor_url_context_menu_model_new_with_handlers (url, open_label,
		connect_label, copy_label, NULL, 0, plugin_model, dispatch, user_data);
}

FabulorUrlContextMenuModel *
fabulor_url_context_menu_model_new_with_handlers (const char *url,
	const char *open_label, const char *connect_label, const char *copy_label,
	const FabulorUrlHandler *handlers, gsize handler_count,
	GMenuModel *plugin_model, FabulorUrlContextDispatch dispatch,
	gpointer user_data)
{
	FabulorUrlContextMenuModel *result;
	GMenu *menu;
	GMenu *commands;
	GMenu *heading;
	GMenuModel *handler_model = NULL;
	gsize handler_cursor = 0;
	guint action_index = 0;
	gboolean is_irc;

	g_return_val_if_fail (url && *url, NULL);
	g_return_val_if_fail (open_label && connect_label && copy_label, NULL);
	g_return_val_if_fail (handler_count == 0 || handlers != NULL, NULL);
	g_return_val_if_fail (!plugin_model || G_IS_MENU_MODEL (plugin_model), NULL);
	result = g_new0 (FabulorUrlContextMenuModel, 1);
	result->url = g_strdup (url);
	result->dispatch = dispatch;
	result->user_data = user_data;
	result->actions = g_simple_action_group_new ();
	url_action_add (result, "open", FABULOR_URL_CONTEXT_OPEN, NULL, TRUE);
	url_action_add (result, "copy", FABULOR_URL_CONTEXT_COPY, NULL, TRUE);
	is_irc = g_str_has_prefix (url, "irc://") || g_str_has_prefix (url, "ircs://");
	heading = g_menu_new ();
	g_menu_append (heading, url, NULL);
	commands = g_menu_new ();
	g_menu_append (commands, is_irc ? connect_label : open_label, "context.open");
	g_menu_append (commands, copy_label, "context.copy");
	menu = g_menu_new ();
	g_menu_append_section (menu, NULL, G_MENU_MODEL (heading));
	g_menu_append_section (menu, NULL, G_MENU_MODEL (commands));
	if (handler_count > 0)
	{
		handler_model = url_handler_model_new (result, handlers, handler_count,
			&handler_cursor, &action_index);
		url_model_append_items (menu, handler_model);
		g_object_unref (handler_model);
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
