/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_TRAY_MENU_COMPOSITION_H
#define FABULOR_TRAY_MENU_COMPOSITION_H

#include <gio/gio.h>

GMenuModel *fabulor_tray_menu_compose (GMenuModel *built_in,
	GMenuModel *plugin, guint plugin_index);

#endif
