/*
 * Copyright (C) 2026 Fabulor contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "user-list-view.h"


#define FABULOR_USER_LIST_VIEW_DATA "fabulor-user-list-view-data"

typedef struct
{
	GtkWidget *view;
	FabulorUserListModel *model;
	gboolean compact;
	gboolean show_hosts;
	gint *nick_width;
	gint *host_width;
} FabulorUserListView;

static FabulorUserListView *
user_list_view_data (GtkWidget *view)
{
	g_return_val_if_fail (GTK_IS_WIDGET (view), NULL);
	return g_object_get_data (G_OBJECT (view), FABULOR_USER_LIST_VIEW_DATA);
}


typedef struct
{
	GtkWidget *box;
	GtkWidget *icon;
	GtkWidget *prefix;
	GtkWidget *nick;
	GtkWidget *host;
	gpointer item;
	GdkPixbuf *icon_source;
	gulong notify_id;
} FabulorUserListItemBinding;

static GQuark
user_list_item_quark (void)
{
	return g_quark_from_static_string ("fabulor-user-list-item");
}

static void
user_list_item_set_icon (FabulorUserListItemBinding *binding,
	GdkPixbuf *pixbuf)
{
	GdkTexture *texture = NULL;

	if (binding->icon_source == pixbuf)
		return;
	binding->icon_source = pixbuf;
	if (pixbuf)
	{
		GBytes *bytes = g_bytes_new_with_free_func (
			gdk_pixbuf_get_pixels (pixbuf), gdk_pixbuf_get_byte_length (pixbuf),
			(GDestroyNotify) g_object_unref, g_object_ref (pixbuf));
		GdkMemoryFormat format = gdk_pixbuf_get_has_alpha (pixbuf) ?
			GDK_MEMORY_R8G8B8A8 : GDK_MEMORY_R8G8B8;

		texture = gdk_memory_texture_new (gdk_pixbuf_get_width (pixbuf),
			gdk_pixbuf_get_height (pixbuf), format, bytes,
			(gsize) gdk_pixbuf_get_rowstride (pixbuf));
		g_bytes_unref (bytes);
	}
	gtk_image_set_from_paintable (GTK_IMAGE (binding->icon),
		texture ? GDK_PAINTABLE (texture) : NULL);
	g_clear_object (&texture);
}

static void
user_list_item_update (FabulorUserListItemBinding *binding)
{
	GdkPixbuf *pixbuf = fabulor_user_list_model_get_item_icon (binding->item);
	const GdkRGBA *color = fabulor_user_list_model_get_item_foreground (
		binding->item);
	PangoAttrList *attributes = NULL;

	user_list_item_set_icon (binding, pixbuf);
	gtk_label_set_markup (GTK_LABEL (binding->prefix),
		fabulor_user_list_model_get_item_prefix (binding->item) ?
		fabulor_user_list_model_get_item_prefix (binding->item) : "");
	gtk_label_set_markup (GTK_LABEL (binding->nick),
		fabulor_user_list_model_get_item_nick (binding->item) ?
		fabulor_user_list_model_get_item_nick (binding->item) : "");
	if (binding->host)
		gtk_label_set_text (GTK_LABEL (binding->host),
			fabulor_user_list_model_get_item_host (binding->item) ?
			fabulor_user_list_model_get_item_host (binding->item) : "");
	if (color)
	{
		attributes = pango_attr_list_new ();
		pango_attr_list_insert (attributes, pango_attr_foreground_new (
			(guint16) (color->red * 65535.0),
			(guint16) (color->green * 65535.0),
			(guint16) (color->blue * 65535.0)));
	}
	gtk_label_set_attributes (GTK_LABEL (binding->nick), attributes);
	if (attributes)
		pango_attr_list_unref (attributes);
}

static void
user_list_item_notify (GObject *item, GParamSpec *pspec, gpointer user_data)
{
	(void) item;
	(void) pspec;
	user_list_item_update (user_data);
}

static void
user_list_factory_setup (GtkSignalListItemFactory *factory,
	GtkListItem *list_item, gpointer user_data)
{
	FabulorUserListView *owner = user_data;
	FabulorUserListItemBinding *binding = g_new0 (
		FabulorUserListItemBinding, 1);
	gint spacing = owner->compact ? 1 : 3;

	(void) factory;
	binding->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, spacing);
	binding->icon = gtk_image_new ();
	binding->prefix = gtk_label_new (NULL);
	binding->nick = gtk_label_new (NULL);
	gtk_label_set_xalign (GTK_LABEL (binding->prefix), 0.0f);
	gtk_label_set_xalign (GTK_LABEL (binding->nick), 0.0f);
	gtk_label_set_ellipsize (GTK_LABEL (binding->nick), PANGO_ELLIPSIZE_END);
	gtk_widget_set_hexpand (binding->nick, TRUE);
	if (owner->nick_width && *owner->nick_width > 0)
		gtk_widget_set_size_request (binding->nick, *owner->nick_width, -1);
	gtk_box_append (GTK_BOX (binding->box), binding->icon);
	gtk_box_append (GTK_BOX (binding->box), binding->prefix);
	gtk_box_append (GTK_BOX (binding->box), binding->nick);
	if (owner->show_hosts)
	{
		binding->host = gtk_label_new (NULL);
		gtk_label_set_xalign (GTK_LABEL (binding->host), 0.0f);
		gtk_label_set_ellipsize (GTK_LABEL (binding->host),
			PANGO_ELLIPSIZE_END);
		gtk_widget_set_hexpand (binding->host, TRUE);
		if (owner->host_width && *owner->host_width > 0)
			gtk_widget_set_size_request (binding->host, *owner->host_width, -1);
		gtk_box_append (GTK_BOX (binding->box), binding->host);
	}
	gtk_list_item_set_child (list_item, binding->box);
	g_object_set_data_full (G_OBJECT (list_item),
		"fabulor-user-list-item-binding", binding, g_free);
}

static void
user_list_factory_bind (GtkSignalListItemFactory *factory,
	GtkListItem *list_item, gpointer user_data)
{
	FabulorUserListItemBinding *binding = g_object_get_data (
		G_OBJECT (list_item), "fabulor-user-list-item-binding");

	(void) factory;
	(void) user_data;
	binding->item = gtk_list_item_get_item (list_item);
	binding->notify_id = g_signal_connect (binding->item, "notify",
		G_CALLBACK (user_list_item_notify), binding);
	g_object_set_qdata (G_OBJECT (binding->box), user_list_item_quark (),
		list_item);
	user_list_item_update (binding);
}

static void
user_list_factory_unbind (GtkSignalListItemFactory *factory,
	GtkListItem *list_item, gpointer user_data)
{
	FabulorUserListItemBinding *binding = g_object_get_data (
		G_OBJECT (list_item), "fabulor-user-list-item-binding");

	(void) factory;
	(void) user_data;
	if (binding->item && binding->notify_id)
		g_signal_handler_disconnect (binding->item, binding->notify_id);
	binding->item = NULL;
	binding->icon_source = NULL;
	binding->notify_id = 0;
	g_object_set_qdata (G_OBJECT (binding->box), user_list_item_quark (), NULL);
}

static gboolean
user_list_position_for_user (FabulorUserListView *owner, gpointer user,
	guint *position)
{
	guint count;
	guint i;

	if (!owner->model)
		return FALSE;
	count = fabulor_user_list_model_get_n_rows (owner->model);
	for (i = 0; i < count; i++)
	{
		if (fabulor_user_list_model_get_user_at (owner->model, i) == user)
		{
			*position = i;
			return TRUE;
		}
	}
	return FALSE;
}

static gboolean
user_list_position_at_point (FabulorUserListView *owner, gdouble x,
	gdouble y, guint *position)
{
	GtkWidget *picked = gtk_widget_pick (owner->view, x, y, GTK_PICK_DEFAULT);

	while (picked && picked != owner->view)
	{
		GtkListItem *item = g_object_get_qdata (G_OBJECT (picked),
			user_list_item_quark ());
		if (item)
		{
			*position = gtk_list_item_get_position (item);
			return TRUE;
		}
		picked = gtk_widget_get_parent (picked);
	}
	return FALSE;
}


GtkWidget *
fabulor_user_list_view_new (gboolean compact, gboolean show_hosts,
	gint *nick_width, gint *host_width)
{
	FabulorUserListView *owner = g_new0 (FabulorUserListView, 1);

	owner->compact = compact;
	owner->show_hosts = show_hosts;
	owner->nick_width = nick_width;
	owner->host_width = host_width;
	{
		GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
		owner->view = gtk_list_view_new (NULL, factory);
		g_signal_connect (factory, "setup", G_CALLBACK (user_list_factory_setup),
			owner);
		g_signal_connect (factory, "bind", G_CALLBACK (user_list_factory_bind),
			owner);
		g_signal_connect (factory, "unbind", G_CALLBACK (user_list_factory_unbind),
			owner);
		gtk_list_view_set_single_click_activate (GTK_LIST_VIEW (owner->view),
			FALSE);
	}
	gtk_widget_set_hexpand (owner->view, TRUE);
	gtk_widget_set_vexpand (owner->view, TRUE);
	gtk_widget_set_name (owner->view, "zoitechat-userlist");
	gtk_widget_set_can_focus (owner->view, TRUE);
	g_object_set_data_full (G_OBJECT (owner->view), FABULOR_USER_LIST_VIEW_DATA,
		owner, g_free);
	return owner->view;
}

void
fabulor_user_list_view_set_model (GtkWidget *view,
	FabulorUserListModel *model)
{
	FabulorUserListView *owner = user_list_view_data (view);

	if (!owner)
		return;
	owner->model = model;
	gtk_list_view_set_model (GTK_LIST_VIEW (view), model ?
		fabulor_user_list_model_get_selection (model) : NULL);
}

FabulorUserListModel *
fabulor_user_list_view_get_model (GtkWidget *view)
{
	FabulorUserListView *owner = user_list_view_data (view);
	return owner ? owner->model : NULL;
}

gfloat
fabulor_user_list_view_get_scroll_value (GtkWidget *view)
{
	GtkAdjustment *adjustment = gtk_scrollable_get_vadjustment (
		GTK_SCROLLABLE (view));
	return adjustment ? (gfloat) gtk_adjustment_get_value (adjustment) : 0.0f;
}

void
fabulor_user_list_view_set_scroll_value (GtkWidget *view, gfloat value)
{
	GtkAdjustment *adjustment = gtk_scrollable_get_vadjustment (
		GTK_SCROLLABLE (view));
	if (adjustment)
		gtk_adjustment_set_value (adjustment, value);
}

GPtrArray *
fabulor_user_list_view_dup_selected_users (GtkWidget *view)
{
	FabulorUserListView *owner = user_list_view_data (view);
	GPtrArray *users = g_ptr_array_new ();

	if (!owner || !owner->model)
		return users;
	{
		GtkSelectionModel *selection = fabulor_user_list_model_get_selection (
			owner->model);
		GtkBitset *selected = gtk_selection_model_get_selection (selection);
		GtkBitsetIter iter;
		guint position;
		if (gtk_bitset_iter_init_first (&iter, selected, &position))
		{
			do
			{
				gpointer item = g_list_model_get_item (
					fabulor_user_list_model_get_list_model (owner->model), position);
				if (item)
				{
					g_ptr_array_add (users,
						fabulor_user_list_model_get_item_user (item));
					g_object_unref (item);
				}
			} while (gtk_bitset_iter_next (&iter, &position));
		}
		gtk_bitset_unref (selected);
	}
	return users;
}

gboolean
fabulor_user_list_view_is_user_selected (GtkWidget *view, gpointer user)
{
	FabulorUserListView *owner = user_list_view_data (view);

	if (!owner || !owner->model || !user)
		return FALSE;
	{
		guint position;
		return user_list_position_for_user (owner, user, &position) &&
			gtk_selection_model_is_selected (
				fabulor_user_list_model_get_selection (owner->model), position);
	}
}

gboolean
fabulor_user_list_view_select_user (GtkWidget *view, gpointer user,
	gboolean toggle, gboolean clear_others, gboolean scroll_to)
{
	FabulorUserListView *owner = user_list_view_data (view);

	if (!owner || !owner->model || !user)
		return FALSE;
	{
		GtkSelectionModel *selection = fabulor_user_list_model_get_selection (
			owner->model);
		guint position;
		gboolean selected;
		if (!user_list_position_for_user (owner, user, &position))
			return FALSE;
		selected = gtk_selection_model_is_selected (selection, position);
		if (toggle && selected)
			gtk_selection_model_unselect_item (selection, position);
		else
			gtk_selection_model_select_item (selection, position, clear_others);
		if (scroll_to)
			gtk_list_view_scroll_to (GTK_LIST_VIEW (view), position,
				GTK_LIST_SCROLL_FOCUS, NULL);
		return TRUE;
	}
}

gpointer
fabulor_user_list_view_get_user_at_position (GtkWidget *view, gdouble x,
	gdouble y)
{
	FabulorUserListView *owner = user_list_view_data (view);

	if (!owner || !owner->model)
		return NULL;
	{
		guint position;
		return user_list_position_at_point (owner, x, y, &position) ?
			fabulor_user_list_model_get_user_at (owner->model, position) : NULL;
	}
}

gboolean
fabulor_user_list_view_select_at_position (GtkWidget *view, gdouble x,
	gdouble y, gboolean clear_others)
{
	gpointer user = fabulor_user_list_view_get_user_at_position (view, x, y);
	return user && fabulor_user_list_view_select_user (view, user, FALSE,
		clear_others, FALSE);
}

void
fabulor_user_list_view_unselect_all (GtkWidget *view)
{
	FabulorUserListView *owner = user_list_view_data (view);

	if (!owner || !owner->model)
		return;
	gtk_selection_model_unselect_all (
		fabulor_user_list_model_get_selection (owner->model));
}
