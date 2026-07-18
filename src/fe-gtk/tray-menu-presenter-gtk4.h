/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_TRAY_MENU_PRESENTER_GTK4_H
#define FABULOR_TRAY_MENU_PRESENTER_GTK4_H

#include <gtk/gtk.h>

typedef struct _FabulorTrayMenuPresenterGtk4 FabulorTrayMenuPresenterGtk4;

FabulorTrayMenuPresenterGtk4 *fabulor_tray_menu_presenter_gtk4_new (
	GMenuModel *menu, GActionGroup *built_in_actions,
	GActionGroup *plugin_actions);
void fabulor_tray_menu_presenter_gtk4_free (
	FabulorTrayMenuPresenterGtk4 *presenter);
gboolean fabulor_tray_menu_presenter_gtk4_set_projection (
	FabulorTrayMenuPresenterGtk4 *presenter, GMenuModel *menu,
	GActionGroup *built_in_actions, GActionGroup *plugin_actions);
GtkPopoverMenu *fabulor_tray_menu_presenter_gtk4_get_popover (
	FabulorTrayMenuPresenterGtk4 *presenter);
GMenuModel *fabulor_tray_menu_presenter_gtk4_get_menu (
	FabulorTrayMenuPresenterGtk4 *presenter);

#endif
