/* ZoiteChat
 * Copyright (C) 1998-2010 Peter Zelezny.
 * Copyright (C) 2009-2013 Berke Viktor.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/* abstract channel view: tabs or tree or anything you like */

#include <stdlib.h>
#include <string.h>

#include "../common/fabulor.h"
#include "../common/fabulorc.h"
#include "fe-gtk.h"
#include "maingui.h"
#include "gtkutil.h"
#include "gtk-compat.h"
#include "chanview.h"
#include "channel-model.h"
#include "theme/theme-manager.h"
#include "theme/theme-access.h"

struct _chanview
{
	/* impl scratch area */
	char implscratch[sizeof (void *) * 8];

	FabulorChannelModel *model;
	int size;			/* number of channels in view */

	GtkWidget *box;	/* the box we destroy when changing implementations */
	PangoFontDescription *font_desc;	/* font used for tree */
	chan *focused;		/* currently focused channel */
	int trunc_len;

	/* callbacks */
	void (*cb_focus) (chanview *, chan *, int tag, void *userdata);
	void (*cb_xbutton) (chanview *, chan *, int tag, void *userdata);
	gboolean (*cb_contextmenu) (chanview *, chan *, int tag, void *userdata,
		GtkWidget *source, guint button, gdouble x, gdouble y,
		GdkModifierType state);
	int (*cb_compare) (void *a, void *b);

	/* impl */
	void (*func_init) (chanview *);
	void (*func_postinit) (chanview *);
	void *(*func_add) (chanview *, chan *, char *, chan *);
	void (*func_move_focus) (chanview *, gboolean, int);
	void (*func_change_orientation) (chanview *);
	void (*func_remove) (chan *);
	void (*func_move) (chan *, int delta);
	void (*func_move_family) (chan *, int delta);
	void (*func_focus) (chan *);
	void (*func_set_color) (chan *, PangoAttrList *);
	void (*func_rename) (chan *, char *);
	gboolean (*func_is_collapsed) (chan *);
	chan *(*func_get_parent) (chan *);
	void (*func_cleanup) (chanview *);

	unsigned int sorted:1;
	unsigned int vertical:1;
	unsigned int use_icons:1;
	guint theme_listener_id;
};

struct _chan
{
	chanview *cv;	/* our owner */
	void *userdata;	/* session * */
	void *family;		/* server * or null */
	void *impl;	/* togglebutton or null */
	GdkPixbuf *icon;
	char *name;
	PangoAttrList *attributes;
	PangoUnderline underline;
	short allow_closure;	/* allow it to be closed when it still has children? */
	short tag;
};

static chan *cv_find_chan_by_number (chanview *cv, int num);
static int cv_find_number_of_chan (chanview *cv, chan *find_ch);
static void cv_find_neighbors_for_removal (chanview *cv, chan *find_ch, chan **left_ch, chan **first_ch);
static void chanview_update_model_row (chan *ch);

static int
cv_scroll_direction (gdouble dx, gdouble dy)
{
	(void) dx;
	if (dy > 0.0)
		return 1;
	if (dy < 0.0)
		return -1;
	return 0;
}

static int
cv_scroll_step_count (void)
{
	int speed = prefs.hex_gui_mouse_scroll_speed;
	if (speed < 1)
		speed = 1;
	return (speed + 9) / 10;
}


/* ======= TABS ======= */

#include "chanview-tabs.c"


/* ======= TREE ======= */

#include "chanview-tree.c"


/* ==== ABSTRACT CHANVIEW ==== */

void
chanview_apply_theme (chanview *cv)
{
	GtkWidget *w;
	treeview *tv;
	const PangoFontDescription *font = NULL;

	if (cv == NULL)
		return;

	/* Only the tree implementation has a dedicated view we can explicitly style. */
	if (cv->func_init != cv_tree_init)
		return;

	tv = (treeview *) cv;
	if (tv->tree == NULL)
		return;

	w = tv->tree;
	if (input_style)
		font = input_style->font_desc;

	theme_manager_apply_channel_tree_style (w,
			theme_manager_get_channel_tree_palette_behavior (font));
}

static void
chanview_theme_changed (const ThemeChangedEvent *event, gpointer userdata)
{
	chanview *cv = userdata;

	(void) event;
	chanview_apply_theme (cv);
}

static char *
truncate_tab_name (char *name, int max)
{
	char *buf;

	if (max > 2 && g_utf8_strlen (name, -1) > max)
	{
		/* truncate long channel names */
		buf = g_malloc (strlen (name) + 4);
		g_utf8_strncpy (buf, name, max);
		strcat (buf, "..");
		return buf;
	}

	return name;
}

static void
chanview_pop_cb (chanview *cv, chan *ch)
{
	chan *parent = fabulor_channel_model_get_parent (cv->model, ch);

	ch->impl = cv->func_add (cv, ch, ch->name, parent);
	if (ch->attributes)
		cv->func_set_color (ch, ch->attributes);
}

static void
chanview_populate (chanview *cv)
{
	guint count = fabulor_channel_model_get_flat_count (cv->model);
	guint i;

	for (i = 0; i < count; i++)
		chanview_pop_cb (cv, fabulor_channel_model_get_flat_at (cv->model, i));
}

void
chanview_set_impl (chanview *cv, int type)
{
	/* cleanup the old one */
	if (cv->func_cleanup)
		cv->func_cleanup (cv);

	switch (type)
	{
	case 0:
		cv->func_init = cv_tabs_init;
		cv->func_postinit = cv_tabs_postinit;
		cv->func_add = cv_tabs_add;
		cv->func_move_focus = cv_tabs_move_focus;
		cv->func_change_orientation = cv_tabs_change_orientation;
		cv->func_remove = cv_tabs_remove;
		cv->func_move = cv_tabs_move;
		cv->func_move_family = cv_tabs_move_family;
		cv->func_focus = cv_tabs_focus;
		cv->func_set_color = cv_tabs_set_color;
		cv->func_rename = cv_tabs_rename;
		cv->func_is_collapsed = cv_tabs_is_collapsed;
		cv->func_get_parent = cv_tabs_get_parent;
		cv->func_cleanup = cv_tabs_cleanup;
		break;

	default:
		cv->func_init = cv_tree_init;
		cv->func_postinit = cv_tree_postinit;
		cv->func_add = cv_tree_add;
		cv->func_move_focus = cv_tree_move_focus;
		cv->func_change_orientation = cv_tree_change_orientation;
		cv->func_remove = cv_tree_remove;
		cv->func_move = cv_tree_move;
		cv->func_move_family = cv_tree_move_family;
		cv->func_focus = cv_tree_focus;
		cv->func_set_color = cv_tree_set_color;
		cv->func_rename = cv_tree_rename;
		cv->func_is_collapsed = cv_tree_is_collapsed;
		cv->func_get_parent = cv_tree_get_parent;
		cv->func_cleanup = cv_tree_cleanup;
		break;
	}

	/* now rebuild a new tabbar or tree */
	cv->func_init (cv);

	chanview_populate (cv);

	cv->func_postinit (cv);

	/* force re-focus */
	if (cv->focused)
		cv->func_focus (cv->focused);
}

static void
chanview_free_ch (chan *ch)
{
	g_free (ch->name);
	if (ch->attributes)
		pango_attr_list_unref (ch->attributes);
	g_free (ch);
}

static void
chanview_destroy_model (chanview *cv)
{
	guint count = fabulor_channel_model_get_flat_count (cv->model);
	guint i;

	for (i = 0; i < count; i++)
		chanview_free_ch (fabulor_channel_model_get_flat_at (cv->model, i));
	fabulor_channel_model_free (cv->model);
}

static void
chanview_destroy (chanview *cv)
{
	if (cv->theme_listener_id)
	{
		theme_listener_unregister (cv->theme_listener_id);
		cv->theme_listener_id = 0;
	}

	if (cv->func_cleanup)
		cv->func_cleanup (cv);

	chanview_destroy_model (cv);
	g_free (cv);
}

static void
chanview_box_finalized_cb (gpointer user_data, GObject *box)
{
	chanview *cv = user_data;

	(void) box;
	cv->box = NULL;
	chanview_destroy (cv);
}

chanview *
chanview_new (int type, int trunc_len, gboolean sort, gboolean use_icons,
				  PangoFontDescription *font_desc
)
{
	chanview *cv;

	cv = g_new0 (chanview, 1);
	cv->model = fabulor_channel_model_new ();
	cv->font_desc = font_desc;
	cv->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	cv->trunc_len = trunc_len;
	cv->sorted = sort;
	cv->use_icons = use_icons;
	gtk_widget_show (cv->box);
	chanview_set_impl (cv, type);
	cv->theme_listener_id = theme_listener_register ("chanview", chanview_theme_changed, cv);

	g_object_weak_ref (G_OBJECT (cv->box), chanview_box_finalized_cb, cv);

	return cv;
}

/* too lazy for signals */

void
chanview_set_callbacks (chanview *cv,
	void (*cb_focus) (chanview *, chan *, int tag, void *userdata),
	void (*cb_xbutton) (chanview *, chan *, int tag, void *userdata),
	gboolean (*cb_contextmenu) (chanview *, chan *, int tag, void *userdata,
		GtkWidget *source, guint button, gdouble x, gdouble y,
		GdkModifierType state),
	int (*cb_compare) (void *a, void *b))
{
	cv->cb_focus = cb_focus;
	cv->cb_xbutton = cb_xbutton;
	cv->cb_contextmenu = cb_contextmenu;
	cv->cb_compare = cb_compare;
}

static guint
chanview_insert_position (chanview *cv, chan *parent, void *userdata)
{
	guint count;
	guint i;

	if (!parent)
		return fabulor_channel_model_get_root_count (cv->model);
	count = fabulor_channel_model_get_child_count (cv->model, parent);
	if (cv->sorted)
	{
		for (i = 0; i < count; i++)
		{
			chan *existing = fabulor_channel_model_get_child_at (
				cv->model, parent, i);
			if (existing->tag == 0 &&
				cv->cb_compare (existing->userdata, userdata) > 0)
				return i;
		}
	}
	return count;
}

/* find a parent node with the same "family" pointer (i.e. the Server tab) */

static chan *
chanview_find_parent (chanview *cv, void *family, chan *avoid)
{
	guint count = fabulor_channel_model_get_root_count (cv->model);
	guint i;

	for (i = 0; i < count; i++)
	{
		chan *candidate = fabulor_channel_model_get_root_at (cv->model, i);
		if (family == candidate->family && candidate != avoid)
			return candidate;
	}
	return NULL;
}

static chan *
chanview_add_real (chanview *cv, char *name, void *family, void *userdata,
						 gboolean allow_closure, int tag, GdkPixbuf *icon,
						 chan *ch, chan *avoid)
{
	chan *parent = chanview_find_parent (cv, family, avoid);
	guint position;
	FabulorChannelModelRow row;

	if (!ch)
	{
		ch = g_new0 (chan, 1);
		ch->userdata = userdata;
		ch->family = family;
		ch->cv = cv;
		ch->allow_closure = allow_closure;
		ch->tag = tag;
		ch->icon = icon;
		ch->underline = PANGO_UNDERLINE_NONE;
	}
	g_free (ch->name);
	ch->name = g_strdup (name);
	row.identity = ch;
	row.name = ch->name;
	row.attributes = ch->attributes;
	row.icon = ch->icon;
	row.underline = ch->underline;
	position = chanview_insert_position (cv, parent, userdata);
	g_return_val_if_fail (fabulor_channel_model_insert (cv->model, &row,
		parent, position), NULL);

	cv->size++;
	ch->impl = cv->func_add (cv, ch, name, parent);

	return ch;
}

chan *
chanview_add (chanview *cv, char *name, void *family, void *userdata, gboolean allow_closure, int tag, GdkPixbuf *icon)
{
	char *new_name;
	chan *ret;

	new_name = truncate_tab_name (name, cv->trunc_len);

	ret = chanview_add_real (cv, new_name, family, userdata, allow_closure, tag, icon, NULL, NULL);

	if (new_name != name)
		g_free (new_name);

	return ret;
}

int
chanview_get_size (chanview *cv)
{
	return cv->size;
}

GtkWidget *
chanview_get_box (chanview *cv)
{
	return cv->box;
}

void
chanview_move_focus (chanview *cv, gboolean relative, int num)
{
	cv->func_move_focus (cv, relative, num);
}

GtkOrientation
chanview_get_orientation (chanview *cv)
{
	return (cv->vertical ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL);
}

void
chanview_set_orientation (chanview *cv, gboolean vertical)
{
	if (vertical != cv->vertical)
	{
		cv->vertical = vertical;
		cv->func_change_orientation (cv);
	}
}

int
chan_get_tag (chan *ch)
{
	return ch->tag;
}

void *
chan_get_userdata (chan *ch)
{
	return ch->userdata;
}

void
chan_focus (chan *ch)
{
	if (ch->cv->focused == ch)
		return;

	ch->cv->func_focus (ch);
}

void
chan_move (chan *ch, int delta)
{
	fabulor_channel_model_move_cyclic (ch->cv->model, ch, delta);
	ch->cv->func_move (ch, delta);
}

void
chan_move_family (chan *ch, int delta)
{
	chan *root = fabulor_channel_model_get_parent (ch->cv->model, ch);
	fabulor_channel_model_move_cyclic (ch->cv->model, root ? root : ch, delta);
	ch->cv->func_move_family (ch, delta);
}

static void
chanview_update_model_row (chan *ch)
{
	FabulorChannelModelRow row = {
		ch, ch->name, ch->attributes, ch->icon, ch->underline
	};

	g_warn_if_fail (fabulor_channel_model_update (ch->cv->model, &row));
}

void
chan_set_color (chan *ch, PangoAttrList *list)
{
	if (list)
		pango_attr_list_ref (list);
	if (ch->attributes)
		pango_attr_list_unref (ch->attributes);
	ch->attributes = list;
	chanview_update_model_row (ch);
	ch->cv->func_set_color (ch, list);
}

void
chan_rename (chan *ch, char *name, int trunc_len)
{
	char *new_name;

	new_name = truncate_tab_name (name, trunc_len);

	g_free (ch->name);
	ch->name = g_strdup (new_name);
	chanview_update_model_row (ch);
	ch->cv->func_rename (ch, new_name);
	ch->cv->trunc_len = trunc_len;

	if (new_name != name)
		g_free (new_name);
}

/* this thing is overly complicated */

static int
cv_find_number_of_chan (chanview *cv, chan *find_ch)
{
	gint position = fabulor_channel_model_get_flat_position (cv->model, find_ch);
	return position >= 0 ? position : 0;
}

/* this thing is overly complicated too */

static chan *
cv_find_chan_by_number (chanview *cv, int num)
{
	return num < 0 ? NULL : fabulor_channel_model_get_flat_at (
		cv->model, (guint) num);
}

static void
cv_find_neighbors_for_removal (chanview *cv, chan *find_ch, chan **left_ch, chan **first_ch)
{
	guint count;
	guint i;
	chan *prev = NULL;

	*left_ch = NULL;
	*first_ch = NULL;

	count = fabulor_channel_model_get_flat_count (cv->model);
	for (i = 0; i < count; i++)
	{
		chan *candidate = fabulor_channel_model_get_flat_at (cv->model, i);
		if (candidate == find_ch)
			*left_ch = prev;
		else if (*first_ch == NULL)
			*first_ch = candidate;
		prev = candidate;
	}
}

static void
chan_emancipate_children (chan *ch)
{
	chan *childch;

	while ((childch = fabulor_channel_model_get_child_at (
		ch->cv->model, ch, 0)) != NULL)
	{
		chan *new_parent;
		guint position;

		ch->cv->func_remove (childch);
		new_parent = chanview_find_parent (ch->cv, childch->family, ch);
		position = chanview_insert_position (ch->cv, new_parent,
			childch->userdata);
		g_warn_if_fail (fabulor_channel_model_reparent (ch->cv->model,
			childch, new_parent, position));
		childch->impl = ch->cv->func_add (ch->cv, childch, childch->name,
			new_parent);
		if (childch->attributes)
			childch->cv->func_set_color (childch, childch->attributes);
	}
}

gboolean
chan_remove (chan *ch, gboolean force)
{
	chan *new_ch;
	chan *first_ch;
	extern int fabulor_is_quitting;

	if (fabulor_is_quitting)	/* avoid lots of looping on exit */
		return TRUE;

	/* is this ch allowed to be closed while still having children? */
	if (!force &&
		 fabulor_channel_model_get_child_count (ch->cv->model, ch) > 0 &&
		 !ch->allow_closure)
		return FALSE;

	chan_emancipate_children (ch);
	ch->cv->func_remove (ch);

	/* is it the focused one? */
	if (ch->cv->focused == ch)
	{
		ch->cv->focused = NULL;

		cv_find_neighbors_for_removal (ch->cv, ch, &new_ch, &first_ch);
		if (new_ch && new_ch != ch)
		{
			chan_focus (new_ch);
		}
		else if (first_ch && first_ch != ch)
		{
			chan_focus (first_ch);
		}
	}

	ch->cv->size--;
	g_warn_if_fail (fabulor_channel_model_remove (ch->cv->model, ch));
	chanview_free_ch (ch);
	return TRUE;
}

gboolean
chan_is_collapsed (chan *ch)
{
	return ch->cv->func_is_collapsed (ch);
}

chan *
chan_get_parent (chan *ch)
{
	return ch->cv->func_get_parent (ch);
}
