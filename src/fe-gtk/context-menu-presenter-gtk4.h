/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_CONTEXT_MENU_PRESENTER_GTK4_H
#define FABULOR_CONTEXT_MENU_PRESENTER_GTK4_H

#include <gtk/gtk.h>

typedef struct _FabulorContextMenuPresenterGtk4 FabulorContextMenuPresenterGtk4;

FabulorContextMenuPresenterGtk4 *fabulor_context_menu_presenter_gtk4_new (
	GMenuModel *menu, GActionGroup *built_in_actions,
	GActionGroup *plugin_actions);
FabulorContextMenuPresenterGtk4 *
fabulor_context_menu_presenter_gtk4_new_with_namespaces (
	GMenuModel *menu, const char *built_in_namespace,
	GActionGroup *built_in_actions, const char *plugin_namespace,
	GActionGroup *plugin_actions);
void fabulor_context_menu_presenter_gtk4_free (
	FabulorContextMenuPresenterGtk4 *presenter);
gboolean fabulor_context_menu_presenter_gtk4_set_projection (
	FabulorContextMenuPresenterGtk4 *presenter, GMenuModel *menu,
	GActionGroup *built_in_actions, GActionGroup *plugin_actions);
gboolean fabulor_context_menu_presenter_gtk4_popup_at (
	FabulorContextMenuPresenterGtk4 *presenter, GtkWidget *origin,
	gdouble x, gdouble y);
GtkPopoverMenu *fabulor_context_menu_presenter_gtk4_get_popover (
	FabulorContextMenuPresenterGtk4 *presenter);

#endif
