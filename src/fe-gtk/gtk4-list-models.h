/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef FABULOR_GTK4_LIST_MODELS_H
#define FABULOR_GTK4_LIST_MODELS_H

#include <gtk/gtk.h>


G_BEGIN_DECLS

typedef struct _FabulorGtk4FlatModelStack FabulorGtk4FlatModelStack;
typedef struct _FabulorGtk4TreeModelStack FabulorGtk4TreeModelStack;

typedef enum
{
	FABULOR_GTK4_SELECTION_SINGLE,
	FABULOR_GTK4_SELECTION_MULTIPLE
} FabulorGtk4SelectionMode;

FabulorGtk4FlatModelStack *fabulor_gtk4_flat_model_stack_new (
	GType item_type, GtkSorter *sorter, FabulorGtk4SelectionMode selection_mode);
void fabulor_gtk4_flat_model_stack_free (FabulorGtk4FlatModelStack *stack);
GListStore *fabulor_gtk4_flat_model_stack_get_store (
	FabulorGtk4FlatModelStack *stack);
GtkSortListModel *fabulor_gtk4_flat_model_stack_get_sorted (
	FabulorGtk4FlatModelStack *stack);
GtkSelectionModel *fabulor_gtk4_flat_model_stack_get_selection (
	FabulorGtk4FlatModelStack *stack);
void fabulor_gtk4_flat_model_stack_append (FabulorGtk4FlatModelStack *stack,
	gpointer item);
gboolean fabulor_gtk4_flat_model_stack_remove (
	FabulorGtk4FlatModelStack *stack, gpointer item);
void fabulor_gtk4_flat_model_stack_clear (FabulorGtk4FlatModelStack *stack);

FabulorGtk4TreeModelStack *fabulor_gtk4_tree_model_stack_new (
	GType item_type, gboolean autoexpand,
	GtkTreeListModelCreateModelFunc create_func, gpointer user_data,
	GDestroyNotify user_destroy);
void fabulor_gtk4_tree_model_stack_free (FabulorGtk4TreeModelStack *stack);
GListStore *fabulor_gtk4_tree_model_stack_get_roots (
	FabulorGtk4TreeModelStack *stack);
GtkTreeListModel *fabulor_gtk4_tree_model_stack_get_tree (
	FabulorGtk4TreeModelStack *stack);
GtkSingleSelection *fabulor_gtk4_tree_model_stack_get_selection (
	FabulorGtk4TreeModelStack *stack);
void fabulor_gtk4_tree_model_stack_append_root (
	FabulorGtk4TreeModelStack *stack, gpointer item);
gboolean fabulor_gtk4_tree_model_stack_remove_root (
	FabulorGtk4TreeModelStack *stack, gpointer item);
void fabulor_gtk4_tree_model_stack_clear (FabulorGtk4TreeModelStack *stack);

G_END_DECLS

#endif
