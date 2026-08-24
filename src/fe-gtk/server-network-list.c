/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "server-network-list.h"

#include "gtk-compat.h"

#include "gtk4-list-models.h"

struct _FabulorServerNetworkList
{
	GtkWidget *view;
	FabulorServerNetworkSelectionFunc selection_func;
	FabulorServerNetworkEditFunc edit_func;
	gpointer callback_data;
	gpointer pending_edit_identity;
	FabulorGtk4FlatModelStack *models;
	gulong selection_id;
	GHashTable *bindings;
};

static void
server_network_ensure_css (void)
{
	static GtkCssProvider *provider;
	GdkDisplay *display;

	if (provider)
		return;

	display = gdk_display_get_default ();
	if (!display)
		return;

	provider = gtk_css_provider_new ();
	gtk_css_provider_load_from_string (provider,
		".fabulor-favorite-network { font-weight: 600; }");
	gtk_style_context_add_provider_for_display (display,
		GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
}


typedef struct _FabulorServerNetworkRow FabulorServerNetworkRow;
typedef struct _FabulorServerNetworkRowClass FabulorServerNetworkRowClass;

struct _FabulorServerNetworkRow
{
	GObject parent_instance;
	gpointer identity;
	gchar *name;
	gboolean favorite;
};

struct _FabulorServerNetworkRowClass
{
	GObjectClass parent_class;
};

enum
{
	PROP_SERVER_NETWORK_ROW_0,
	PROP_SERVER_NETWORK_ROW_NAME,
	PROP_SERVER_NETWORK_ROW_FAVORITE,
	N_SERVER_NETWORK_ROW_PROPERTIES
};

static GParamSpec *server_network_row_properties[
	N_SERVER_NETWORK_ROW_PROPERTIES];

#define FABULOR_TYPE_SERVER_NETWORK_ROW (fabulor_server_network_row_get_type ())
#define FABULOR_SERVER_NETWORK_ROW(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), FABULOR_TYPE_SERVER_NETWORK_ROW, \
	 FabulorServerNetworkRow))

G_DEFINE_TYPE (FabulorServerNetworkRow, fabulor_server_network_row,
	G_TYPE_OBJECT)

static void
fabulor_server_network_row_get_property (GObject *object, guint property_id,
	GValue *value, GParamSpec *pspec)
{
	FabulorServerNetworkRow *row = FABULOR_SERVER_NETWORK_ROW (object);

	if (property_id == PROP_SERVER_NETWORK_ROW_NAME)
		g_value_set_string (value, row->name);
	else if (property_id == PROP_SERVER_NETWORK_ROW_FAVORITE)
		g_value_set_boolean (value, row->favorite);
	else
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
fabulor_server_network_row_finalize (GObject *object)
{
	FabulorServerNetworkRow *row = FABULOR_SERVER_NETWORK_ROW (object);

	g_free (row->name);
	G_OBJECT_CLASS (fabulor_server_network_row_parent_class)->finalize (object);
}

static void
fabulor_server_network_row_class_init (FabulorServerNetworkRowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = fabulor_server_network_row_get_property;
	object_class->finalize = fabulor_server_network_row_finalize;
	server_network_row_properties[PROP_SERVER_NETWORK_ROW_NAME] =
		g_param_spec_string ("name", "Name", "Network name", NULL,
			G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	server_network_row_properties[PROP_SERVER_NETWORK_ROW_FAVORITE] =
		g_param_spec_boolean ("favorite", "Favorite", "Favorite network",
			FALSE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties (object_class,
		N_SERVER_NETWORK_ROW_PROPERTIES, server_network_row_properties);
}

static void
fabulor_server_network_row_init (FabulorServerNetworkRow *row)
{
	(void) row;
}

static FabulorServerNetworkRow *
server_network_row_new (gpointer identity, const gchar *name,
	gboolean favorite)
{
	FabulorServerNetworkRow *row = g_object_new (
		FABULOR_TYPE_SERVER_NETWORK_ROW, NULL);

	row->identity = identity;
	row->name = g_strdup (name ? name : "");
	row->favorite = favorite;
	return row;
}

static GListStore *
server_network_store (FabulorServerNetworkList *list)
{
	return fabulor_gtk4_flat_model_stack_get_store (list->models);
}

static GtkSingleSelection *
server_network_selection (FabulorServerNetworkList *list)
{
	return GTK_SINGLE_SELECTION (
		fabulor_gtk4_flat_model_stack_get_selection (list->models));
}

static FabulorServerNetworkRow *
server_network_row_at (FabulorServerNetworkList *list, guint position)
{
	return g_list_model_get_item (G_LIST_MODEL (server_network_store (list)),
		position);
}

static void
server_network_selection_changed (GtkSingleSelection *selection,
	GParamSpec *pspec, gpointer user_data)
{
	FabulorServerNetworkList *list = user_data;
	FabulorServerNetworkRow *row = FABULOR_SERVER_NETWORK_ROW (
		gtk_single_selection_get_selected_item (selection));

	(void) pspec;
	if (list->selection_func)
		list->selection_func (row ? row->identity : NULL,
			list->callback_data);
}

typedef struct
{
	FabulorServerNetworkList *owner;
	GtkEditableLabel *label;
	GtkWidget *favorite_icon;
	FabulorServerNetworkRow *row;
	gulong notify_id;
	gboolean editing;
	gboolean blocked;
} ServerNetworkBinding;

static void
server_network_binding_start_editing (ServerNetworkBinding *binding)
{
	if (!binding || !binding->row)
		return;
	gtk_widget_set_can_target (GTK_WIDGET (binding->label), TRUE);
	gtk_editable_label_start_editing (binding->label);
	if (!gtk_editable_label_get_editing (binding->label))
		gtk_widget_set_can_target (GTK_WIDGET (binding->label), FALSE);
}

static void
server_network_binding_refresh (ServerNetworkBinding *binding)
{
	if (!binding->row)
		return;
	binding->blocked = TRUE;
	gtk_editable_set_text (GTK_EDITABLE (binding->label), binding->row->name);
	gtk_widget_set_visible (binding->favorite_icon, binding->row->favorite);
	if (binding->row->favorite)
		gtk_widget_add_css_class (GTK_WIDGET (binding->label),
			"fabulor-favorite-network");
	else
		gtk_widget_remove_css_class (GTK_WIDGET (binding->label),
			"fabulor-favorite-network");
	binding->blocked = FALSE;
}

static void
server_network_row_changed (GObject *object, GParamSpec *pspec,
	gpointer user_data)
{
	ServerNetworkBinding *binding = user_data;
	(void) object;
	(void) pspec;
	if (!binding->blocked)
		server_network_binding_refresh (binding);
}

static void
server_network_editing_changed (GtkEditableLabel *label, GParamSpec *pspec,
	gpointer user_data)
{
	ServerNetworkBinding *binding = user_data;
	gboolean editing = gtk_editable_label_get_editing (label);

	(void) pspec;
	if (!binding->blocked && binding->row && binding->editing && !editing)
	{
		const gchar *text = gtk_editable_get_text (GTK_EDITABLE (label));
		if (binding->owner->edit_func &&
			binding->owner->edit_func (binding->row->identity, text,
				binding->owner->callback_data))
			fabulor_server_network_list_update_name (binding->owner,
				binding->row->identity, text);
		else
			server_network_binding_refresh (binding);
	}
	binding->editing = editing;
	if (!editing)
		gtk_widget_set_can_target (GTK_WIDGET (label), FALSE);
}

static void
server_network_binding_free (gpointer data)
{
	ServerNetworkBinding *binding = data;

	if (binding->row)
	{
		g_hash_table_remove (binding->owner->bindings,
			binding->row->identity);
		if (binding->notify_id)
			g_signal_handler_disconnect (binding->row, binding->notify_id);
	}
	g_clear_object (&binding->row);
	g_free (binding);
}

static void
server_network_factory_setup (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	ServerNetworkBinding *binding = g_new0 (ServerNetworkBinding, 1);
	GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
	GtkWidget *favorite_icon = fabulor_gtk_image_new_from_icon_name (
		"starred-symbolic", FABULOR_GTK_ICON_SIZE_MENU);
	GtkWidget *label = gtk_editable_label_new (NULL);

	(void) factory;
	binding->owner = user_data;
	binding->label = GTK_EDITABLE_LABEL (label);
	binding->favorite_icon = favorite_icon;
	gtk_widget_set_valign (favorite_icon, GTK_ALIGN_CENTER);
	gtk_widget_set_visible (favorite_icon, FALSE);
	gtk_widget_set_hexpand (label, TRUE);
	gtk_widget_set_halign (label, GTK_ALIGN_FILL);
	gtk_widget_set_can_target (label, FALSE);
	g_signal_connect (label, "notify::editing",
		G_CALLBACK (server_network_editing_changed), binding);
	server_network_ensure_css ();
	gtk_box_append (GTK_BOX (box), favorite_icon);
	gtk_box_append (GTK_BOX (box), label);
	gtk_list_item_set_child (item, box);
	g_object_set_data_full (G_OBJECT (item), "fabulor-server-network-binding",
		binding, server_network_binding_free);
}

static void
server_network_factory_bind (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	ServerNetworkBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-server-network-binding");

	(void) factory;
	(void) user_data;
	binding->row = g_object_ref (FABULOR_SERVER_NETWORK_ROW (
		gtk_list_item_get_item (item)));
	binding->notify_id = g_signal_connect (binding->row, "notify",
		G_CALLBACK (server_network_row_changed), binding);
	g_hash_table_insert (binding->owner->bindings, binding->row->identity,
		binding);
	server_network_binding_refresh (binding);
	binding->editing = gtk_editable_label_get_editing (binding->label);
	if (binding->row->identity == binding->owner->pending_edit_identity)
	{
		binding->owner->pending_edit_identity = NULL;
		server_network_binding_start_editing (binding);
	}
}

static void
server_network_factory_unbind (GtkSignalListItemFactory *factory,
	GtkListItem *item, gpointer user_data)
{
	ServerNetworkBinding *binding = g_object_get_data (G_OBJECT (item),
		"fabulor-server-network-binding");

	(void) factory;
	(void) user_data;
	if (binding->row)
	{
		g_hash_table_remove (binding->owner->bindings,
			binding->row->identity);
		if (binding->notify_id)
			g_signal_handler_disconnect (binding->row, binding->notify_id);
	}
	binding->notify_id = 0;
	g_clear_object (&binding->row);
}


static gboolean
server_network_find (FabulorServerNetworkList *list, gpointer identity,
	guint *position)
{
	guint count = fabulor_server_network_list_get_n_rows (list);
	guint i;

	for (i = 0; i < count; i++)
	{
		FabulorServerNetworkRow *row = server_network_row_at (list, i);
		gboolean found = row->identity == identity;
		g_object_unref (row);
		if (found)
		{
			if (position)
				*position = i;
			return TRUE;
		}
	}
	return FALSE;
}

static void
server_network_activate (GtkListView *view, guint position,
	gpointer user_data)
{
	FabulorServerNetworkList *list = user_data;
	gpointer identity;

	(void) view;
	identity = fabulor_server_network_list_get_identity_at (list, position);
	if (!identity)
		return;
	fabulor_server_network_list_select (list, identity);
	fabulor_server_network_list_start_editing_selected (list);
}

FabulorServerNetworkList *
fabulor_server_network_list_new (
	FabulorServerNetworkSelectionFunc selection_func,
	FabulorServerNetworkEditFunc edit_func, gpointer user_data)
{
	FabulorServerNetworkList *list = g_new0 (FabulorServerNetworkList, 1);

	list->selection_func = selection_func;
	list->edit_func = edit_func;
	list->callback_data = user_data;
	list->models = fabulor_gtk4_flat_model_stack_new (
		FABULOR_TYPE_SERVER_NETWORK_ROW, NULL,
		FABULOR_GTK4_SELECTION_SINGLE);
	list->bindings = g_hash_table_new (g_direct_hash, g_direct_equal);
	list->selection_id = g_signal_connect (server_network_selection (list),
		"notify::selected", G_CALLBACK (server_network_selection_changed),
		list);
	return list;
}

void
fabulor_server_network_list_free (FabulorServerNetworkList *list)
{
	if (!list)
		return;
	if (list->selection_id)
		g_signal_handler_disconnect (server_network_selection (list),
			list->selection_id);
	g_hash_table_unref (list->bindings);
	fabulor_gtk4_flat_model_stack_free (list->models);
	g_free (list);
}

GtkWidget *
fabulor_server_network_list_create_view (FabulorServerNetworkList *list,
	GtkScrolledWindow *scroller)
{
	g_return_val_if_fail (list && GTK_IS_SCROLLED_WINDOW (scroller) &&
		!list->view, NULL);
	{
		GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
		g_signal_connect (factory, "setup",
			G_CALLBACK (server_network_factory_setup), list);
		g_signal_connect (factory, "bind",
			G_CALLBACK (server_network_factory_bind), list);
		g_signal_connect (factory, "unbind",
			G_CALLBACK (server_network_factory_unbind), list);
		list->view = gtk_list_view_new (GTK_SELECTION_MODEL (g_object_ref (
			server_network_selection (list))), factory);
		gtk_list_view_set_single_click_activate (GTK_LIST_VIEW (list->view),
			FALSE);
		g_signal_connect (list->view, "activate",
			G_CALLBACK (server_network_activate), list);
	}
	fabulor_gtk_scrolled_window_set_child (scroller, list->view);
	return list->view;
}

gboolean
fabulor_server_network_list_append (FabulorServerNetworkList *list,
	gpointer identity, const gchar *name, gboolean favorite, gboolean prepend)
{
	g_return_val_if_fail (list && identity, FALSE);
	if (server_network_find (list, identity, NULL))
		return FALSE;
	{
		FabulorServerNetworkRow *row = server_network_row_new (identity, name,
			favorite);
		if (prepend)
			g_list_store_insert (server_network_store (list), 0, row);
		else
			g_list_store_append (server_network_store (list), row);
		g_object_unref (row);
	}
	return TRUE;
}

void
fabulor_server_network_list_clear (FabulorServerNetworkList *list)
{
	g_return_if_fail (list != NULL);
	g_list_store_remove_all (server_network_store (list));
}

guint
fabulor_server_network_list_get_n_rows (FabulorServerNetworkList *list)
{
	g_return_val_if_fail (list != NULL, 0);
	return g_list_model_get_n_items (G_LIST_MODEL (server_network_store (list)));
}

gpointer
fabulor_server_network_list_get_identity_at (FabulorServerNetworkList *list,
	guint position)
{
	g_return_val_if_fail (list != NULL, NULL);
	if (position >= fabulor_server_network_list_get_n_rows (list))
		return NULL;
	{
		FabulorServerNetworkRow *row = server_network_row_at (list, position);
		gpointer identity = row->identity;
		g_object_unref (row);
		return identity;
	}
}

gchar *
fabulor_server_network_list_dup_name (FabulorServerNetworkList *list,
	gpointer identity)
{
	guint position;
	gchar *name = NULL;
	g_return_val_if_fail (list != NULL, NULL);
	if (!server_network_find (list, identity, &position))
		return NULL;
	{
		FabulorServerNetworkRow *row = server_network_row_at (list, position);
		name = g_strdup (row->name);
		g_object_unref (row);
	}
	return name;
}

gboolean
fabulor_server_network_list_get_favorite (FabulorServerNetworkList *list,
	gpointer identity, gboolean *favorite)
{
	guint position;
	g_return_val_if_fail (list && favorite, FALSE);
	if (!server_network_find (list, identity, &position))
		return FALSE;
	{
		FabulorServerNetworkRow *row = server_network_row_at (list, position);
		*favorite = row->favorite;
		g_object_unref (row);
	}
	return TRUE;
}

gpointer
fabulor_server_network_list_get_selected (FabulorServerNetworkList *list)
{
	g_return_val_if_fail (list != NULL, NULL);
	{
		FabulorServerNetworkRow *row = FABULOR_SERVER_NETWORK_ROW (
			gtk_single_selection_get_selected_item (
				server_network_selection (list)));
		return row ? row->identity : NULL;
	}
}

gboolean
fabulor_server_network_list_select (FabulorServerNetworkList *list,
	gpointer identity)
{
	guint position;
	g_return_val_if_fail (list != NULL, FALSE);
	if (!server_network_find (list, identity, &position))
		return FALSE;
	gtk_single_selection_set_selected (server_network_selection (list),
		position);
	if (list->view)
		gtk_list_view_scroll_to (GTK_LIST_VIEW (list->view), position,
			GTK_LIST_SCROLL_FOCUS, NULL);
	return TRUE;
}

gboolean
fabulor_server_network_list_select_first (FabulorServerNetworkList *list)
{
	g_return_val_if_fail (list != NULL, FALSE);
	if (fabulor_server_network_list_get_n_rows (list) == 0)
		return FALSE;
	{
		FabulorServerNetworkRow *row = server_network_row_at (list, 0);
		gpointer identity = row->identity;
		g_object_unref (row);
		return fabulor_server_network_list_select (list, identity);
	}
}

gboolean
fabulor_server_network_list_remove (FabulorServerNetworkList *list,
	gpointer identity)
{
	guint position;
	g_return_val_if_fail (list != NULL, FALSE);
	if (!server_network_find (list, identity, &position))
		return FALSE;
	g_list_store_remove (server_network_store (list), position);
	return TRUE;
}

gboolean
fabulor_server_network_list_move (FabulorServerNetworkList *list,
	gpointer identity, gint delta)
{
	guint position;
	gint target;
	g_return_val_if_fail (list != NULL, FALSE);
	if (!server_network_find (list, identity, &position) ||
		(delta != -1 && delta != 1))
		return FALSE;
	target = (gint) position + delta;
	if (target < 0 || target >= (gint) fabulor_server_network_list_get_n_rows (
		list))
		return FALSE;
	{
		FabulorServerNetworkRow *row = server_network_row_at (list, position);
		g_list_store_remove (server_network_store (list), position);
		g_list_store_insert (server_network_store (list), (guint) target, row);
		g_object_unref (row);
	}
	fabulor_server_network_list_select (list, identity);
	return TRUE;
}

gboolean
fabulor_server_network_list_set_favorite (FabulorServerNetworkList *list,
	gpointer identity, gboolean favorite)
{
	guint position;
	g_return_val_if_fail (list != NULL, FALSE);
	if (!server_network_find (list, identity, &position))
		return FALSE;
	{
		FabulorServerNetworkRow *row = server_network_row_at (list, position);
		if (row->favorite != favorite)
		{
			row->favorite = favorite;
			g_object_notify_by_pspec (G_OBJECT (row),
				server_network_row_properties[PROP_SERVER_NETWORK_ROW_FAVORITE]);
		}
		g_object_unref (row);
	}
	return TRUE;
}

gboolean
fabulor_server_network_list_update_name (FabulorServerNetworkList *list,
	gpointer identity, const gchar *name)
{
	guint position;
	g_return_val_if_fail (list != NULL, FALSE);
	if (!server_network_find (list, identity, &position))
		return FALSE;
	{
		FabulorServerNetworkRow *row = server_network_row_at (list, position);
		if (g_strcmp0 (row->name, name ? name : "") != 0)
		{
			g_free (row->name);
			row->name = g_strdup (name ? name : "");
			g_object_notify_by_pspec (G_OBJECT (row),
				server_network_row_properties[PROP_SERVER_NETWORK_ROW_NAME]);
		}
		g_object_unref (row);
	}
	return TRUE;
}

void
fabulor_server_network_list_start_editing_selected (
	FabulorServerNetworkList *list)
{
	gpointer identity;
	g_return_if_fail (list != NULL);
	identity = fabulor_server_network_list_get_selected (list);
	if (!identity || !list->view)
		return;
	list->pending_edit_identity = identity;
	{
		ServerNetworkBinding *binding = g_hash_table_lookup (list->bindings,
			identity);
		if (binding)
		{
			list->pending_edit_identity = NULL;
			server_network_binding_start_editing (binding);
		}
	}
}
