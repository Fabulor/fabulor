/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_CHANNEL_MODEL_H
#define FABULOR_CHANNEL_MODEL_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _FabulorChannelModel FabulorChannelModel;

typedef struct
{
	gpointer identity;
	const gchar *name;
	PangoAttrList *attributes;
	GdkPixbuf *icon;
	PangoUnderline underline;
} FabulorChannelModelRow;

enum
{
	FABULOR_CHANNEL_COLUMN_NAME,
	FABULOR_CHANNEL_COLUMN_IDENTITY,
	FABULOR_CHANNEL_COLUMN_ATTRIBUTES,
	FABULOR_CHANNEL_COLUMN_ICON,
	FABULOR_CHANNEL_COLUMN_UNDERLINE,
	FABULOR_CHANNEL_N_COLUMNS
};

FabulorChannelModel *fabulor_channel_model_new (void);
void fabulor_channel_model_free (FabulorChannelModel *model);

gboolean fabulor_channel_model_insert (FabulorChannelModel *model,
	const FabulorChannelModelRow *row, gpointer parent_identity,
	guint position);
gboolean fabulor_channel_model_update (FabulorChannelModel *model,
	const FabulorChannelModelRow *row);
gboolean fabulor_channel_model_remove (FabulorChannelModel *model,
	gpointer identity);
gboolean fabulor_channel_model_reparent (FabulorChannelModel *model,
	gpointer identity, gpointer parent_identity, guint position);
gboolean fabulor_channel_model_move_cyclic (FabulorChannelModel *model,
	gpointer identity, gint delta);

gboolean fabulor_channel_model_contains (FabulorChannelModel *model,
	gpointer identity);
gpointer fabulor_channel_model_get_parent (FabulorChannelModel *model,
	gpointer identity);
guint fabulor_channel_model_get_root_count (FabulorChannelModel *model);
gpointer fabulor_channel_model_get_root_at (FabulorChannelModel *model,
	guint position);
guint fabulor_channel_model_get_child_count (FabulorChannelModel *model,
	gpointer parent_identity);
gpointer fabulor_channel_model_get_child_at (FabulorChannelModel *model,
	gpointer parent_identity, guint position);
guint fabulor_channel_model_get_flat_count (FabulorChannelModel *model);
gpointer fabulor_channel_model_get_flat_at (FabulorChannelModel *model,
	guint position);
gint fabulor_channel_model_get_flat_position (FabulorChannelModel *model,
	gpointer identity);
const gchar *fabulor_channel_model_get_name (FabulorChannelModel *model,
	gpointer identity);
PangoAttrList *fabulor_channel_model_ref_attributes (
	FabulorChannelModel *model, gpointer identity);

GtkTreeListModel *fabulor_channel_model_get_tree (FabulorChannelModel *model);
GtkSingleSelection *fabulor_channel_model_get_selection (
	FabulorChannelModel *model);
gboolean fabulor_channel_model_select_identity (FabulorChannelModel *model,
	gpointer identity);
gpointer fabulor_channel_model_get_selected_identity (
	FabulorChannelModel *model);
gpointer fabulor_channel_model_get_item_identity (gpointer item);
const gchar *fabulor_channel_model_get_item_name (gpointer item);
PangoAttrList *fabulor_channel_model_get_item_attributes (gpointer item);
GdkPixbuf *fabulor_channel_model_get_item_icon (gpointer item);
PangoUnderline fabulor_channel_model_get_item_underline (gpointer item);

G_END_DECLS

#endif
