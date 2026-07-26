/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "context-menu-presenter-gtk4.h"
#include "menu-action-namespaces.h"

struct _FabulorContextMenuPresenterGtk4
{
	GtkPopoverMenu *popover;
	GMenuModel *menu;
	char *built_in_namespace;
	char *plugin_namespace;
	GActionGroup *built_in_actions;
	GActionGroup *plugin_actions;
	gint label_max_width_chars;
};

static void
context_menu_apply_label_width_limit (GtkWidget *widget, gint max_width_chars)
{
	GtkWidget *child;

	if (max_width_chars <= 0)
		return;
	if (GTK_IS_LABEL (widget))
	{
		gtk_label_set_ellipsize (GTK_LABEL (widget), PANGO_ELLIPSIZE_END);
		gtk_label_set_max_width_chars (GTK_LABEL (widget), max_width_chars);
	}
	for (child = gtk_widget_get_first_child (widget); child;
		child = gtk_widget_get_next_sibling (child))
		context_menu_apply_label_width_limit (child, max_width_chars);
}

FabulorContextMenuPresenterGtk4 *
fabulor_context_menu_presenter_gtk4_new (GMenuModel *menu,
	GActionGroup *built_in_actions, GActionGroup *plugin_actions)
{
	return fabulor_context_menu_presenter_gtk4_new_with_namespaces (menu,
		FABULOR_CONTEXT_ACTION_NAMESPACE, built_in_actions,
		FABULOR_PLUGIN_CONTEXT_ACTION_NAMESPACE, plugin_actions);
}

FabulorContextMenuPresenterGtk4 *
fabulor_context_menu_presenter_gtk4_new_with_namespaces (GMenuModel *menu,
	const char *built_in_namespace, GActionGroup *built_in_actions,
	const char *plugin_namespace, GActionGroup *plugin_actions)
{
	FabulorContextMenuPresenterGtk4 *presenter;
	GtkWidget *popover;

	g_return_val_if_fail (G_IS_MENU_MODEL (menu), NULL);
	g_return_val_if_fail (built_in_namespace && *built_in_namespace, NULL);
	g_return_val_if_fail (G_IS_ACTION_GROUP (built_in_actions), NULL);
	g_return_val_if_fail ((!plugin_namespace && !plugin_actions) ||
		(plugin_namespace && *plugin_namespace &&
		 G_IS_ACTION_GROUP (plugin_actions)), NULL);
	presenter = g_new0 (FabulorContextMenuPresenterGtk4, 1);
	presenter->built_in_namespace = g_strdup (built_in_namespace);
	presenter->plugin_namespace = g_strdup (plugin_namespace);
	popover = gtk_popover_menu_new_from_model (NULL);
	presenter->popover = GTK_POPOVER_MENU (g_object_ref_sink (popover));
	if (!fabulor_context_menu_presenter_gtk4_set_projection (presenter, menu,
		built_in_actions, plugin_actions))
	{
		fabulor_context_menu_presenter_gtk4_free (presenter);
		return NULL;
	}
	return presenter;
}

void
fabulor_context_menu_presenter_gtk4_free (
	FabulorContextMenuPresenterGtk4 *presenter)
{
	GtkWidget *widget;

	if (!presenter)
		return;
	widget = GTK_WIDGET (presenter->popover);
	gtk_popover_popdown (GTK_POPOVER (presenter->popover));
	gtk_popover_menu_set_menu_model (presenter->popover, NULL);
	gtk_widget_insert_action_group (widget, presenter->built_in_namespace, NULL);
	if (presenter->plugin_namespace)
		gtk_widget_insert_action_group (widget, presenter->plugin_namespace, NULL);
	if (gtk_widget_get_parent (widget))
		gtk_widget_unparent (widget);
	g_clear_object (&presenter->menu);
	g_clear_object (&presenter->built_in_actions);
	g_clear_object (&presenter->plugin_actions);
	g_clear_object (&presenter->popover);
	g_free (presenter->built_in_namespace);
	g_free (presenter->plugin_namespace);
	g_free (presenter);
}

gboolean
fabulor_context_menu_presenter_gtk4_set_projection (
	FabulorContextMenuPresenterGtk4 *presenter, GMenuModel *menu,
	GActionGroup *built_in_actions, GActionGroup *plugin_actions)
{
	GtkWidget *widget;

	g_return_val_if_fail (presenter != NULL, FALSE);
	g_return_val_if_fail (G_IS_MENU_MODEL (menu), FALSE);
	g_return_val_if_fail (G_IS_ACTION_GROUP (built_in_actions), FALSE);
	g_return_val_if_fail ((!presenter->plugin_namespace && !plugin_actions) ||
		(presenter->plugin_namespace && G_IS_ACTION_GROUP (plugin_actions)), FALSE);
	widget = GTK_WIDGET (presenter->popover);
	g_set_object (&presenter->menu, menu);
	g_set_object (&presenter->built_in_actions, built_in_actions);
	g_set_object (&presenter->plugin_actions, plugin_actions);
	gtk_popover_menu_set_menu_model (presenter->popover, presenter->menu);
	gtk_widget_insert_action_group (widget, presenter->built_in_namespace,
		presenter->built_in_actions);
	if (presenter->plugin_namespace)
		gtk_widget_insert_action_group (widget, presenter->plugin_namespace,
			presenter->plugin_actions);
	context_menu_apply_label_width_limit (widget,
		presenter->label_max_width_chars);
	return TRUE;
}

void
fabulor_context_menu_presenter_gtk4_set_label_width_limit (
	FabulorContextMenuPresenterGtk4 *presenter, gint max_width_chars)
{
	g_return_if_fail (presenter != NULL);
	g_return_if_fail (max_width_chars > 0);
	presenter->label_max_width_chars = max_width_chars;
	context_menu_apply_label_width_limit (GTK_WIDGET (presenter->popover),
		max_width_chars);
}

gboolean
fabulor_context_menu_presenter_gtk4_popup_at (
	FabulorContextMenuPresenterGtk4 *presenter, GtkWidget *origin,
	gdouble x, gdouble y)
{
	GdkRectangle point = { (gint)x, (gint)y, 1, 1 };
	GtkWidget *widget;

	g_return_val_if_fail (presenter != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_WIDGET (origin), FALSE);
	widget = GTK_WIDGET (presenter->popover);
	if (gtk_widget_get_parent (widget) != origin)
	{
		gtk_popover_popdown (GTK_POPOVER (presenter->popover));
		if (gtk_widget_get_parent (widget))
			gtk_widget_unparent (widget);
		gtk_widget_set_parent (widget, origin);
	}
	context_menu_apply_label_width_limit (widget,
		presenter->label_max_width_chars);
	gtk_popover_set_pointing_to (GTK_POPOVER (presenter->popover), &point);
	gtk_popover_popup (GTK_POPOVER (presenter->popover));
	return TRUE;
}

GtkPopoverMenu *
fabulor_context_menu_presenter_gtk4_get_popover (
	FabulorContextMenuPresenterGtk4 *presenter)
{
	return presenter ? presenter->popover : NULL;
}
