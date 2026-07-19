/* Copyright (C) 2026 Fabulor contributors */
#include "tab-context-menu-model.h"
#include "menu-action-namespaces.h"

typedef struct
{
	FabulorTabContextMenuModel *owner;
	FabulorTabContextAction action;
	FabulorTabOption option;
	char *command;
	gboolean stateful;
} TabActionData;

struct _FabulorTabContextMenuModel
{
	GMenuModel *menu;
	GSimpleActionGroup *actions;
	FabulorTabContextDispatch dispatch;
	gpointer user_data;
};

static void
tab_action_data_free (gpointer data, GClosure *closure)
{
	TabActionData *action_data = data;
	(void)closure;
	g_free (action_data->command);
	g_free (action_data);
}

static void
tab_action_activate (GSimpleAction *action, GVariant *parameter,
	gpointer user_data)
{
	TabActionData *data = user_data;
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
		data->owner->dispatch (data->action, data->option, state,
			data->command, data->owner->user_data);
}

static void
tab_action_add (FabulorTabContextMenuModel *model, const char *name,
	FabulorTabContextAction action_id, FabulorTabOption option,
	const char *command, gboolean stateful, gboolean state)
{
	GSimpleAction *action;
	TabActionData *data = g_new0 (TabActionData, 1);
	if (stateful)
		action = g_simple_action_new_stateful (name, NULL,
			g_variant_new_boolean (state));
	else
		action = g_simple_action_new (name, NULL);
	data->owner = model;
	data->action = action_id;
	data->option = option;
	data->command = g_strdup (command);
	data->stateful = stateful;
	g_signal_connect_data (action, "activate", G_CALLBACK (tab_action_activate),
		data, tab_action_data_free, 0);
	g_action_map_add_action (G_ACTION_MAP (model->actions), G_ACTION (action));
	g_object_unref (action);
}

static void
tab_section_append (GMenu *model, GMenu **section)
{
	if (g_menu_model_get_n_items (G_MENU_MODEL (*section)) > 0)
		g_menu_append_section (model, NULL, G_MENU_MODEL (*section));
	g_object_unref (*section);
	*section = g_menu_new ();
}

static void
tab_option_append (FabulorTabContextMenuModel *model, GMenu *menu,
	FabulorTabOption option, const char *label, gboolean state)
{
	char *name = g_strdup_printf ("option-%u", (guint)option);
	char *detailed = g_strconcat (FABULOR_CONTEXT_ACTION_NAMESPACE, ".",
		name, NULL);
	tab_action_add (model, name, FABULOR_TAB_CONTEXT_OPTION, option, NULL,
		TRUE, state);
	g_menu_append (menu, label, detailed);
	g_free (detailed);
	g_free (name);
}

static GMenuModel *
tab_configured_model_new (FabulorTabContextMenuModel *owner,
	const FabulorTabConfiguredItem *items, gsize count, gsize *cursor,
	guint *action_index)
{
	GMenu *model = g_menu_new ();
	GMenu *section = g_menu_new ();
	while (*cursor < count)
	{
		const FabulorTabConfiguredItem *item = &items[(*cursor)++];
		if (item->kind == FABULOR_TAB_CONFIG_SUBMENU_END)
			break;
		if (item->kind == FABULOR_TAB_CONFIG_SEPARATOR)
		{
			tab_section_append (model, &section);
			continue;
		}
		if (item->kind == FABULOR_TAB_CONFIG_SUBMENU_BEGIN)
		{
			GMenuModel *submenu = tab_configured_model_new (owner, items, count,
				cursor, action_index);
			if (item->label && g_menu_model_get_n_items (submenu) > 0)
				g_menu_append_submenu (section, item->label, submenu);
			g_object_unref (submenu);
			continue;
		}
		if (item->label && item->command)
		{
			GMenuItem *menu_item;
			char *name = g_strdup_printf ("configured-%u", (*action_index)++);
			char *detailed = g_strconcat (FABULOR_CONTEXT_ACTION_NAMESPACE,
				".", name, NULL);
			tab_action_add (owner, name,
				item->kind == FABULOR_TAB_CONFIG_TOGGLE ?
				FABULOR_TAB_CONTEXT_TOGGLE : FABULOR_TAB_CONTEXT_COMMAND,
				FABULOR_TAB_OPTION_COUNT, item->command,
				item->kind == FABULOR_TAB_CONFIG_TOGGLE, item->active);
			menu_item = g_menu_item_new (item->label, detailed);
			if (item->icon && *item->icon)
				g_menu_item_set_attribute (menu_item, "fabulor-icon", "s",
					item->icon);
			g_menu_append_item (section, menu_item);
			g_object_unref (menu_item);
			g_free (detailed);
			g_free (name);
		}
	}
	tab_section_append (model, &section);
	g_object_unref (section);
	g_menu_freeze (model);
	return G_MENU_MODEL (model);
}

static void
tab_model_append_items (GMenu *destination, GMenuModel *source)
{
	gint i;
	for (i = 0; source && i < g_menu_model_get_n_items (source); i++)
	{
		GMenuItem *item = g_menu_item_new_from_model (source, i);
		g_menu_append_item (destination, item);
		g_object_unref (item);
	}
}

static gboolean
tab_labels_valid (const FabulorTabContextLabels *labels)
{
	return labels && labels->alerts && labels->settings &&
		labels->notification && labels->beep && labels->tray &&
		labels->taskbar && labels->logging && labels->scrollback &&
		labels->strip_colors && labels->hide_join_part && labels->autojoin &&
		labels->autoconnect && labels->detach && labels->close;
}

FabulorTabContextMenuModel *
fabulor_tab_context_menu_model_new (const char *heading,
	const FabulorTabContextState *state, const FabulorTabContextLabels *labels,
	const FabulorTabConfiguredItem *configured, gsize configured_count,
	GMenuModel *plugin_model, FabulorTabContextDispatch dispatch,
	gpointer user_data)
{
	FabulorTabContextMenuModel *result;
	GMenu *menu = g_menu_new ();
	GMenu *section;
	GMenuModel *configured_model = NULL;
	gsize cursor = 0;
	guint action_index = 0;

	g_return_val_if_fail (state && tab_labels_valid (labels), NULL);
	g_return_val_if_fail (configured_count == 0 || configured, NULL);
	g_return_val_if_fail (!plugin_model || G_IS_MENU_MODEL (plugin_model), NULL);
	result = g_new0 (FabulorTabContextMenuModel, 1);
	result->actions = g_simple_action_group_new ();
	result->dispatch = dispatch;
	result->user_data = user_data;
	if (heading && *heading)
	{
		section = g_menu_new ();
		g_menu_append (section, heading, NULL);
		g_menu_append_section (menu, NULL, G_MENU_MODEL (section));
		g_object_unref (section);
	}
	if (state->has_session)
	{
		GMenu *alerts = g_menu_new ();
		GMenu *settings = g_menu_new ();
		section = g_menu_new ();
		tab_option_append (result, alerts, FABULOR_TAB_OPTION_NOTIFICATION,
			labels->notification, state->options[FABULOR_TAB_OPTION_NOTIFICATION]);
		tab_option_append (result, alerts, FABULOR_TAB_OPTION_BEEP,
			labels->beep, state->options[FABULOR_TAB_OPTION_BEEP]);
		tab_option_append (result, alerts, FABULOR_TAB_OPTION_TRAY,
			labels->tray, state->options[FABULOR_TAB_OPTION_TRAY]);
		tab_option_append (result, alerts, FABULOR_TAB_OPTION_TASKBAR,
			labels->taskbar, state->options[FABULOR_TAB_OPTION_TASKBAR]);
		tab_option_append (result, settings, FABULOR_TAB_OPTION_LOGGING,
			labels->logging, state->options[FABULOR_TAB_OPTION_LOGGING]);
		tab_option_append (result, settings, FABULOR_TAB_OPTION_SCROLLBACK,
			labels->scrollback, state->options[FABULOR_TAB_OPTION_SCROLLBACK]);
		if (state->is_channel)
		{
			tab_option_append (result, settings, FABULOR_TAB_OPTION_STRIP_COLORS,
				labels->strip_colors,
				state->options[FABULOR_TAB_OPTION_STRIP_COLORS]);
			tab_option_append (result, settings, FABULOR_TAB_OPTION_HIDE_JOIN_PART,
				labels->hide_join_part,
				state->options[FABULOR_TAB_OPTION_HIDE_JOIN_PART]);
		}
		g_menu_append_submenu (section, labels->alerts, G_MENU_MODEL (alerts));
		g_menu_append_submenu (section, labels->settings, G_MENU_MODEL (settings));
		g_menu_append_section (menu, NULL, G_MENU_MODEL (section));
		g_object_unref (alerts);
		g_object_unref (settings);
		g_object_unref (section);
		if (state->has_network && (state->is_channel || state->is_server))
		{
			const char *name = state->is_channel ? "autojoin" : "autoconnect";
			const char *label = state->is_channel ? labels->autojoin :
				labels->autoconnect;
			FabulorTabContextAction action = state->is_channel ?
				FABULOR_TAB_CONTEXT_AUTOJOIN : FABULOR_TAB_CONTEXT_AUTOCONNECT;
			gboolean active = state->is_channel ? state->autojoin :
				state->autoconnect;
			char *detailed = g_strconcat (FABULOR_CONTEXT_ACTION_NAMESPACE,
				".", name, NULL);
			section = g_menu_new ();
			tab_action_add (result, name, action, FABULOR_TAB_OPTION_COUNT,
				NULL, TRUE, active);
			g_menu_append (section, label, detailed);
			g_menu_append_section (menu, NULL, G_MENU_MODEL (section));
			g_object_unref (section);
			g_free (detailed);
		}
	}
	section = g_menu_new ();
	tab_action_add (result, "detach", FABULOR_TAB_CONTEXT_DETACH,
		FABULOR_TAB_OPTION_COUNT, NULL, FALSE, FALSE);
	tab_action_add (result, "close", FABULOR_TAB_CONTEXT_CLOSE,
		FABULOR_TAB_OPTION_COUNT, NULL, FALSE, FALSE);
	g_menu_append (section, labels->detach, FABULOR_CONTEXT_ACTION_NAMESPACE ".detach");
	g_menu_append (section, labels->close, FABULOR_CONTEXT_ACTION_NAMESPACE ".close");
	g_menu_append_section (menu, NULL, G_MENU_MODEL (section));
	g_object_unref (section);
	if (configured_count > 0)
	{
		configured_model = tab_configured_model_new (result, configured,
			configured_count, &cursor, &action_index);
		tab_model_append_items (menu, configured_model);
		g_object_unref (configured_model);
	}
	if (plugin_model && g_menu_model_get_n_items (plugin_model) > 0)
		g_menu_append_section (menu, NULL, plugin_model);
	g_menu_freeze (menu);
	result->menu = G_MENU_MODEL (menu);
	return result;
}

void
fabulor_tab_context_menu_model_free (FabulorTabContextMenuModel *model)
{
	if (!model)
		return;
	g_clear_object (&model->menu);
	g_clear_object (&model->actions);
	g_free (model);
}

GMenuModel *
fabulor_tab_context_menu_model_get_menu (FabulorTabContextMenuModel *model)
{
	return model ? model->menu : NULL;
}

GActionGroup *
fabulor_tab_context_menu_model_get_actions (FabulorTabContextMenuModel *model)
{
	return model ? G_ACTION_GROUP (model->actions) : NULL;
}
