/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "tray-menu-composition.h"

static void
tray_menu_append_range (GMenu *destination, GMenuModel *source,
	gint first, gint end)
{
	gint i;

	if (!source)
		return;

	for (i = first; i < end; i++)
	{
		GMenuItem *item = g_menu_item_new_from_model (source, i);

		g_menu_append_item (destination, item);
		g_object_unref (item);
	}
}

GMenuModel *
fabulor_tray_menu_compose (GMenuModel *built_in, GMenuModel *plugin,
	guint plugin_index)
{
	GMenu *result;
	gint built_in_count;
	gint insert_at;

	g_return_val_if_fail (G_IS_MENU_MODEL (built_in), NULL);
	g_return_val_if_fail (!plugin || G_IS_MENU_MODEL (plugin), NULL);

	result = g_menu_new ();
	built_in_count = g_menu_model_get_n_items (built_in);
	insert_at = plugin_index > (guint)built_in_count
		? built_in_count
		: (gint)plugin_index;
	tray_menu_append_range (result, built_in, 0, insert_at);
	if (plugin)
		tray_menu_append_range (result, plugin, 0,
			g_menu_model_get_n_items (plugin));
	tray_menu_append_range (result, built_in, insert_at, built_in_count);
	g_menu_freeze (result);

	return G_MENU_MODEL (result);
}
