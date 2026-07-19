/* Copyright (C) 2026 Fabulor contributors */
#include "middle-context-menu-model.h"

struct _FabulorMiddleContextMenuModel
{
	GMenuModel *menu;
};

static gboolean
middle_context_path_equal (const char *left, const char *right)
{
	if (!left || !right)
		return FALSE;
	while (*left || *right)
	{
		while (*left == '_')
			left++;
		while (*right == '_')
			right++;
		if (g_ascii_tolower (*left) != g_ascii_tolower (*right))
			return FALSE;
		if (*left)
			left++;
		if (*right)
			right++;
	}
	return TRUE;
}

static void
middle_context_append_items (GMenu *destination, GMenuModel *source)
{
	gint i;
	for (i = 0; source && i < g_menu_model_get_n_items (source); i++)
	{
		GMenuItem *item = g_menu_item_new_from_model (source, i);
		g_menu_append_item (destination, item);
		g_object_unref (item);
	}
}

static gint
middle_context_plugin_find (GMenuModel *plugin_model, const char *path)
{
	gint i;
	for (i = 0; plugin_model && i < g_menu_model_get_n_items (plugin_model); i++)
	{
		char *label = NULL;
		gboolean matches;
		g_menu_model_get_item_attribute (plugin_model, i,
			G_MENU_ATTRIBUTE_LABEL, "s", &label);
		matches = middle_context_path_equal (label, path);
		g_free (label);
		if (matches)
		{
			GMenuModel *submenu = g_menu_model_get_item_link (plugin_model, i,
				G_MENU_LINK_SUBMENU);
			if (submenu)
			{
				g_object_unref (submenu);
				return i;
			}
		}
	}
	return -1;
}

FabulorMiddleContextMenuModel *
fabulor_middle_context_menu_model_new (
	const FabulorMiddleContextSection *sections, gsize section_count,
	GMenuModel *plugin_model)
{
	FabulorMiddleContextMenuModel *result;
	GMenu *menu;
	gboolean *used;
	gsize i;

	g_return_val_if_fail (section_count == 0 || sections, NULL);
	g_return_val_if_fail (!plugin_model || G_IS_MENU_MODEL (plugin_model), NULL);
	result = g_new0 (FabulorMiddleContextMenuModel, 1);
	menu = g_menu_new ();
	used = g_new0 (gboolean, plugin_model ?
		g_menu_model_get_n_items (plugin_model) : 0);
	for (i = 0; i < section_count; i++)
	{
		GMenu *submenu;
		GMenuModel *plugin_submenu = NULL;
		gint plugin_index;

		if (!sections[i].label || !*sections[i].label ||
			(sections[i].model && !G_IS_MENU_MODEL (sections[i].model)))
			continue;
		plugin_index = middle_context_plugin_find (plugin_model,
			sections[i].plugin_path);
		if (plugin_index >= 0)
		{
			plugin_submenu = g_menu_model_get_item_link (plugin_model,
				plugin_index, G_MENU_LINK_SUBMENU);
			if (plugin_submenu)
				used[plugin_index] = TRUE;
		}
		submenu = g_menu_new ();
		middle_context_append_items (submenu, sections[i].model);
		if (plugin_submenu && g_menu_model_get_n_items (plugin_submenu) > 0)
			g_menu_append_section (submenu, NULL, plugin_submenu);
		if (g_menu_model_get_n_items (G_MENU_MODEL (submenu)) > 0)
			g_menu_append_submenu (menu, sections[i].label,
				G_MENU_MODEL (submenu));
		g_clear_object (&plugin_submenu);
		g_object_unref (submenu);
	}
	for (i = 0; plugin_model && i < (gsize)g_menu_model_get_n_items (plugin_model);
		i++)
	{
		if (!used[i])
		{
			GMenuItem *item = g_menu_item_new_from_model (plugin_model, (gint)i);
			g_menu_append_item (menu, item);
			g_object_unref (item);
		}
	}
	g_free (used);
	g_menu_freeze (menu);
	result->menu = G_MENU_MODEL (menu);
	return result;
}

void
fabulor_middle_context_menu_model_free (FabulorMiddleContextMenuModel *model)
{
	if (!model)
		return;
	g_clear_object (&model->menu);
	g_free (model);
}

GMenuModel *
fabulor_middle_context_menu_model_get_menu (
	FabulorMiddleContextMenuModel *model)
{
	return model ? model->menu : NULL;
}
