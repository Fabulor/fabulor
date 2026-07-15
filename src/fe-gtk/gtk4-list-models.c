/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "gtk4-list-models.h"

struct _FabulorGtk4FlatModelStack
{
	GListStore *store;
	GtkSortListModel *sorted;
	GtkMultiSelection *selection;
};

struct _FabulorGtk4TreeModelStack
{
	GListStore *roots;
	GtkTreeListModel *tree;
	GtkSingleSelection *selection;
};

static gboolean
fabulor_gtk4_list_item_type_is_valid (GType item_type)
{
	return item_type != G_TYPE_INVALID && g_type_is_a (item_type, G_TYPE_OBJECT);
}

FabulorGtk4FlatModelStack *
fabulor_gtk4_flat_model_stack_new (GType item_type, GtkSorter *sorter)
{
	FabulorGtk4FlatModelStack *stack;

	g_return_val_if_fail (fabulor_gtk4_list_item_type_is_valid (item_type), NULL);
	g_return_val_if_fail (sorter == NULL || GTK_IS_SORTER (sorter), NULL);

	stack = g_new0 (FabulorGtk4FlatModelStack, 1);
	stack->store = g_list_store_new (item_type);
	stack->sorted = gtk_sort_list_model_new (
		G_LIST_MODEL (g_object_ref (stack->store)),
		sorter ? g_object_ref (sorter) : NULL);
	stack->selection = gtk_multi_selection_new (
		G_LIST_MODEL (g_object_ref (stack->sorted)));
	return stack;
}

void
fabulor_gtk4_flat_model_stack_free (FabulorGtk4FlatModelStack *stack)
{
	if (!stack)
		return;

	g_clear_object (&stack->selection);
	g_clear_object (&stack->sorted);
	g_clear_object (&stack->store);
	g_free (stack);
}

GListStore *
fabulor_gtk4_flat_model_stack_get_store (FabulorGtk4FlatModelStack *stack)
{
	g_return_val_if_fail (stack != NULL, NULL);
	return stack->store;
}

GtkSortListModel *
fabulor_gtk4_flat_model_stack_get_sorted (FabulorGtk4FlatModelStack *stack)
{
	g_return_val_if_fail (stack != NULL, NULL);
	return stack->sorted;
}

GtkMultiSelection *
fabulor_gtk4_flat_model_stack_get_selection (FabulorGtk4FlatModelStack *stack)
{
	g_return_val_if_fail (stack != NULL, NULL);
	return stack->selection;
}

void
fabulor_gtk4_flat_model_stack_append (FabulorGtk4FlatModelStack *stack,
									  gpointer item)
{
	g_return_if_fail (stack != NULL);
	g_return_if_fail (G_IS_OBJECT (item));
	g_list_store_append (stack->store, item);
}

gboolean
fabulor_gtk4_flat_model_stack_remove (FabulorGtk4FlatModelStack *stack,
									  gpointer item)
{
	guint position;

	g_return_val_if_fail (stack != NULL, FALSE);
	g_return_val_if_fail (G_IS_OBJECT (item), FALSE);
	if (!g_list_store_find (stack->store, item, &position))
		return FALSE;

	g_list_store_remove (stack->store, position);
	return TRUE;
}

void
fabulor_gtk4_flat_model_stack_clear (FabulorGtk4FlatModelStack *stack)
{
	g_return_if_fail (stack != NULL);
	g_list_store_remove_all (stack->store);
}

FabulorGtk4TreeModelStack *
fabulor_gtk4_tree_model_stack_new (GType item_type, gboolean autoexpand,
									GtkTreeListModelCreateModelFunc create_func,
									gpointer user_data,
									GDestroyNotify user_destroy)
{
	FabulorGtk4TreeModelStack *stack;

	g_return_val_if_fail (fabulor_gtk4_list_item_type_is_valid (item_type), NULL);
	g_return_val_if_fail (create_func != NULL, NULL);

	stack = g_new0 (FabulorGtk4TreeModelStack, 1);
	stack->roots = g_list_store_new (item_type);
	stack->tree = gtk_tree_list_model_new (
		G_LIST_MODEL (g_object_ref (stack->roots)), FALSE, autoexpand,
		create_func, user_data, user_destroy);
	stack->selection = gtk_single_selection_new (
		G_LIST_MODEL (g_object_ref (stack->tree)));
	gtk_single_selection_set_autoselect (stack->selection, FALSE);
	gtk_single_selection_set_can_unselect (stack->selection, TRUE);
	return stack;
}

void
fabulor_gtk4_tree_model_stack_free (FabulorGtk4TreeModelStack *stack)
{
	if (!stack)
		return;

	g_clear_object (&stack->selection);
	g_clear_object (&stack->tree);
	g_clear_object (&stack->roots);
	g_free (stack);
}

GListStore *
fabulor_gtk4_tree_model_stack_get_roots (FabulorGtk4TreeModelStack *stack)
{
	g_return_val_if_fail (stack != NULL, NULL);
	return stack->roots;
}

GtkTreeListModel *
fabulor_gtk4_tree_model_stack_get_tree (FabulorGtk4TreeModelStack *stack)
{
	g_return_val_if_fail (stack != NULL, NULL);
	return stack->tree;
}

GtkSingleSelection *
fabulor_gtk4_tree_model_stack_get_selection (FabulorGtk4TreeModelStack *stack)
{
	g_return_val_if_fail (stack != NULL, NULL);
	return stack->selection;
}

void
fabulor_gtk4_tree_model_stack_append_root (FabulorGtk4TreeModelStack *stack,
									   gpointer item)
{
	g_return_if_fail (stack != NULL);
	g_return_if_fail (G_IS_OBJECT (item));
	g_list_store_append (stack->roots, item);
}

gboolean
fabulor_gtk4_tree_model_stack_remove_root (FabulorGtk4TreeModelStack *stack,
									   gpointer item)
{
	guint position;

	g_return_val_if_fail (stack != NULL, FALSE);
	g_return_val_if_fail (G_IS_OBJECT (item), FALSE);
	if (!g_list_store_find (stack->roots, item, &position))
		return FALSE;

	g_list_store_remove (stack->roots, position);
	return TRUE;
}

void
fabulor_gtk4_tree_model_stack_clear (FabulorGtk4TreeModelStack *stack)
{
	g_return_if_fail (stack != NULL);
	g_list_store_remove_all (stack->roots);
}
