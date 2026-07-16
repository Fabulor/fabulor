/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_CHANNEL_TREE_VIEW_H
#define FABULOR_CHANNEL_TREE_VIEW_H

#include <gtk/gtk.h>

#include "channel-model.h"

G_BEGIN_DECLS

typedef void (*FabulorChannelTreeSelectionFunc) (GtkWidget *view,
	gpointer identity, gpointer user_data);

GtkWidget *fabulor_channel_tree_view_new (FabulorChannelModel *model,
	gboolean use_icons, gboolean compact, gboolean show_tree_lines,
	gboolean unindent_children);
void fabulor_channel_tree_view_set_selection_callback (GtkWidget *view,
	FabulorChannelTreeSelectionFunc callback, gpointer user_data);

gpointer fabulor_channel_tree_view_get_identity_at_position (GtkWidget *view,
	gdouble x, gdouble y);
gboolean fabulor_channel_tree_view_focus_identity (GtkWidget *view,
	gpointer identity);
gboolean fabulor_channel_tree_view_expand_parent (GtkWidget *view,
	gpointer identity);
void fabulor_channel_tree_view_expand_all (GtkWidget *view);
gboolean fabulor_channel_tree_view_is_expanded (GtkWidget *view,
	gpointer identity);

G_END_DECLS

#endif
