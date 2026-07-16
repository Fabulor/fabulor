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

/* file included in chanview.c */

typedef struct _tab_scroll_animation tab_scroll_animation;
typedef struct _tab_family tab_family;
typedef struct _tab_item tab_item;
typedef struct _tabview_state tabview_state;

typedef struct
{
	GtkWidget *outer;	/* outer box */
	GtkWidget *inner;	/* inner box */
	tabview_state *state;
	tab_scroll_animation *backward_animation;
	tab_scroll_animation *forward_animation;
	int backward_is_moving;
	int forward_is_moving;
	int ignore_toggle;
} tabview;

G_STATIC_ASSERT (sizeof (tabview) <= sizeof (((chanview *) 0)->implscratch));

struct _tab_family
{
	GtkWidget *box;
	GtkWidget *separator;
};

struct _tab_item
{
	GtkWidget *tab;
	GtkWidget *label;
	GtkWidget *close_button;
};

struct _tabview_state
{
	GtkWidget *viewport;
	GHashTable *families;	/* root chan * -> tab_family */
	GHashTable *items;	/* chan * -> tab_item */
};

static void chanview_populate (chanview *cv);

struct _tab_scroll_animation
{
	GtkAdjustment *adj;
	gdouble current_value;
	gdouble target_value;
	gint direction;
	gdouble step_size;
	guint source_id;
	int *moving_flag;
	tab_scroll_animation **slot;
};

static inline gint
cv_tabs_get_viewport_size (GtkAdjustment *adj)
{
	return (gint) gtk_adjustment_get_page_size (adj);
}

static gdouble
tab_search_offset (chanview *cv, gdouble start_offset,
				   gboolean forward, gboolean vertical)
{
	tabview *tabs = (tabview *) cv;
	guint count = fabulor_channel_model_get_flat_count (cv->model);
	guint offset;

	for (offset = 0; offset < count; offset++)
	{
		guint position = forward ? offset : count - offset - 1;
		chan *ch = fabulor_channel_model_get_flat_at (cv->model, position);
		tab_item *item = g_hash_table_lookup (tabs->state->items, ch);
		gdouble x;
		gdouble y;
		gdouble found;

		if (!item || !fabulor_gtk_widget_get_descendant_origin (tabs->inner,
			item->tab, &x, &y))
			continue;
		found = vertical ? y : x;
		if ((forward && found > start_offset) ||
			(!forward && found < start_offset))
			return found;
	}

	return 0;
}

static gboolean
tab_scroll_animation_tick (gpointer userdata)
{
	tab_scroll_animation *animation = userdata;
	gboolean reached_target;

	animation->current_value += animation->step_size * animation->direction;

	if (animation->direction < 0)
		reached_target = animation->current_value <= animation->target_value;
	else
		reached_target = animation->current_value >= animation->target_value;

	if (reached_target)
		animation->current_value = animation->target_value;

	gtk_adjustment_set_value (animation->adj, animation->current_value);

	if (!reached_target)
		return G_SOURCE_CONTINUE;

	*animation->moving_flag = 0;
	animation->source_id = 0;

	*animation->slot = NULL;

	g_object_unref (animation->adj);
	g_free (animation);

	return G_SOURCE_REMOVE;
}

static void
tab_scroll_animation_cancel (tab_scroll_animation **animation)
{
	if (*animation == NULL)
		return;

	if ((*animation)->source_id != 0)
		g_source_remove ((*animation)->source_id);

	*(*animation)->moving_flag = 0;
	g_object_unref ((*animation)->adj);
	g_free (*animation);
	*animation = NULL;
}

static void
tab_scroll_animation_start (tab_scroll_animation **slot, GtkAdjustment *adj,
							gdouble current_value, gdouble target_value,
							gint direction, int *moving_flag)
{
	tab_scroll_animation *animation;
	gdouble distance;
	gdouble frames;

	distance = target_value - current_value;
	if (distance < 0.0)
		distance = -distance;

	if (distance <= 0.0)
	{
		gtk_adjustment_set_value (adj, target_value);
		*moving_flag = 0;
		return;
	}

	animation = g_new0 (tab_scroll_animation, 1);
	animation->adj = g_object_ref (adj);
	animation->current_value = current_value;
	animation->target_value = target_value;
	animation->direction = direction;
	frames = 12.0;
	animation->step_size = distance / MAX (1.0, frames);
	animation->moving_flag = moving_flag;
	animation->slot = slot;

	*moving_flag = 1;
	animation->source_id = g_timeout_add (16, tab_scroll_animation_tick, animation);
	*slot = animation;
}

static void
tab_scroll_left_up (chanview *cv)
{
	tabview *tabs = (tabview *) cv;
	GtkAdjustment *adj;
	gint viewport_size;
	gdouble new_value;

	if (cv->vertical)
		adj = gtk_scrolled_window_get_vadjustment (
			GTK_SCROLLED_WINDOW (tabs->state->viewport));
	else
		adj = gtk_scrolled_window_get_hadjustment (
			GTK_SCROLLED_WINDOW (tabs->state->viewport));

	viewport_size = cv_tabs_get_viewport_size (adj);

	new_value = tab_search_offset (cv, gtk_adjustment_get_value (adj),
		FALSE, cv->vertical);

	if (new_value + viewport_size > gtk_adjustment_get_upper (adj))
		new_value = gtk_adjustment_get_upper (adj) - viewport_size;

	if (tabs->backward_is_moving)
	{
		tab_scroll_animation_cancel (&tabs->backward_animation);
		return;
	}

	tab_scroll_animation_start (&tabs->backward_animation, adj,
							gtk_adjustment_get_value (adj), new_value,
							-1, &tabs->backward_is_moving);
}

static void
tab_scroll_right_down (chanview *cv)
{
	tabview *tabs = (tabview *) cv;
	GtkAdjustment *adj;
	gint viewport_size;
	gdouble new_value;

	if (cv->vertical)
		adj = gtk_scrolled_window_get_vadjustment (
			GTK_SCROLLED_WINDOW (tabs->state->viewport));
	else
		adj = gtk_scrolled_window_get_hadjustment (
			GTK_SCROLLED_WINDOW (tabs->state->viewport));

	viewport_size = cv_tabs_get_viewport_size (adj);

	new_value = tab_search_offset (cv, gtk_adjustment_get_value (adj),
		TRUE, cv->vertical);

	if (new_value == 0 || new_value + viewport_size > gtk_adjustment_get_upper (adj))
		new_value = gtk_adjustment_get_upper (adj) - viewport_size;

	if (tabs->forward_is_moving)
	{
		tab_scroll_animation_cancel (&tabs->forward_animation);
		return;
	}

	tab_scroll_animation_start (&tabs->forward_animation, adj,
							gtk_adjustment_get_value (adj), new_value,
							1, &tabs->forward_is_moving);
}

static gboolean
tab_scroll_cb (GtkWidget *widget, gdouble dx, gdouble dy, gpointer cv)
{
	int direction = cv_scroll_direction (dx, dy);
	int i;

	(void) widget;

	if (prefs.hex_gui_tab_scrollchans)
	{
		if (direction != 0)
		{
			for (i = 0; i < cv_scroll_step_count (); i++)
				mg_switch_page (1, direction);
			return TRUE;
		}
	}
	else
	{
		if (direction < 0)
		{
			for (i = 0; i < cv_scroll_step_count (); i++)
				tab_scroll_left_up (cv);
			return TRUE;
		}
		else if (direction > 0)
		{
			for (i = 0; i < cv_scroll_step_count (); i++)
				tab_scroll_right_down (cv);
			return TRUE;
		}
	}

	return FALSE;
}

static void
cv_tabs_init (chanview *cv)
{
	tabview *tabs = (tabview *) cv;
	GtkWidget *box;
	GtkWidget *viewport;
	GtkWidget *outer;

	if (cv->vertical)
	{
		outer = gtkutil_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 0);
	}
	else
	{
		outer = gtkutil_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
	}
	tabs->outer = outer;
	gtk_widget_show (outer);

	viewport = gtk_scrolled_window_new (0, 0);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (viewport), GTK_SHADOW_NONE);
	if (cv->vertical)
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (viewport),
												  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	else
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (viewport),
												  GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
	gtk_scrolled_window_set_min_content_width (GTK_SCROLLED_WINDOW (viewport), 1);
	gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (viewport), 1);
	gtk_widget_set_hexpand (viewport, TRUE);
	gtk_widget_set_vexpand (viewport, TRUE);
	fabulor_gtk_widget_on_scroll (viewport, tab_scroll_cb, cv);
	fabulor_gtk_box_append (GTK_BOX (outer), viewport, TRUE, TRUE, 0);
	gtk_widget_show (viewport);

	if (cv->vertical)
	{
		box = gtkutil_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 0);
	}
	else
	{
		box = gtkutil_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
	}
	tabs->inner = box;
	tabs->state = g_new0 (tabview_state, 1);
	tabs->state->viewport = viewport;
	tabs->state->families = g_hash_table_new_full (g_direct_hash, g_direct_equal,
		NULL, g_free);
	tabs->state->items = g_hash_table_new_full (g_direct_hash, g_direct_equal,
		NULL, g_free);
	gtk_container_add (GTK_CONTAINER (viewport), box);
	gtk_widget_show (box);

	gtk_container_add (GTK_CONTAINER (cv->box), outer);
}

static void
cv_tabs_postinit (chanview *cv)
{
}

static guint
tab_model_position (chanview *cv, chan *parent, chan *ch)
{
	guint count = fabulor_channel_model_get_child_count (cv->model, parent);
	guint position;

	g_return_val_if_fail (parent != NULL, 0);
	for (position = 0; position < count; position++)
		if (fabulor_channel_model_get_child_at (cv->model, parent,
			position) == ch)
			return position;
	g_warn_if_reached ();
	return count > 0 ? count - 1 : 0;
}

static void
tab_add_real (chanview *cv, GtkWidget *tab, chan *ch, chan *parent)
{
	tabview *tabs = (tabview *) cv;
	chan *root = parent ? parent : ch;
	tab_family *family = g_hash_table_lookup (tabs->state->families, root);

	if (!family)
	{
		g_return_if_fail (parent == NULL);
		family = g_new0 (tab_family, 1);
		if (cv->vertical)
		{
			family->box = gtkutil_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 0);
			family->separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
		}
		else
		{
			family->box = gtkutil_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
			family->separator = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
		}
		g_hash_table_insert (tabs->state->families, root, family);
		fabulor_gtk_box_append (GTK_BOX (tabs->inner), family->box,
			FALSE, FALSE, 0);
		fabulor_gtk_box_append (GTK_BOX (family->box), tab,
			FALSE, FALSE, 0);
		fabulor_gtk_box_append (GTK_BOX (family->box), family->separator,
			FALSE, FALSE, 4);
		gtk_widget_show (family->separator);
		gtk_widget_show (family->box);
	}
	else
	{
		guint position = tab_model_position (cv, parent, ch);

		fabulor_gtk_box_append (GTK_BOX (family->box), tab,
			FALSE, FALSE, 0);
		/* The server tab occupies position zero in each family box. */
		gtk_box_reorder_child (GTK_BOX (family->box), tab,
			(gint) position + 1);
	}
	gtk_widget_show (tab);
	gtk_widget_queue_resize (gtk_widget_get_parent (tabs->inner));
}

/* called when a tab is clicked (button down) */

static void
tab_pressed_cb (GtkToggleButton *tab, chan *ch)
{
	chan *old_tab;
	int is_switching = TRUE;
	chanview *cv = ch->cv;
	tabview *tabs = (tabview *) cv;

	tabs->ignore_toggle = TRUE;
	/* de-activate the old tab */
	old_tab = cv->focused;
	if (old_tab && old_tab->impl)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (old_tab->impl), FALSE);
		if (old_tab == ch)
			is_switching = FALSE;
	}
	gtk_toggle_button_set_active (tab, TRUE);
	tabs->ignore_toggle = FALSE;
	cv->focused = ch;

	if (is_switching)
		/* call the focus callback */
		cv->cb_focus (cv, ch, ch->tag, ch->userdata);
}

/* called for keyboard tab toggles only */
static void
tab_toggled_cb (GtkToggleButton *tab, chan *ch)
{
	if (((tabview *) ch->cv)->ignore_toggle)
		return;

	/* activated a tab via keyboard */
	tab_pressed_cb (tab, ch);
}

static gboolean
tab_click_cb (GtkWidget *wid, guint button, guint n_press, gdouble x,
	gdouble y, GdkModifierType state, gpointer user_data)
{
	chan *ch = user_data;
	tabview *tabs = (tabview *) ch->cv;
	tab_item *item = g_hash_table_lookup (tabs->state->items, ch);

	(void) n_press;
	if (button == 1)
	{
		if (prefs.hex_gui_tab_closebuttons && item &&
			fabulor_gtk_widget_contains_descendant_point (wid,
				item->close_button, x, y))
		{
			ch->cv->cb_xbutton (ch->cv, ch, ch->tag, ch->userdata);
			return TRUE;
		}
	}

	return ch->cv->cb_contextmenu (ch->cv, ch, ch->tag, ch->userdata, wid,
		button, x, y, state);
}

static void
tab_close_motion_cb (GtkWidget *wid, gdouble x, gdouble y, gpointer user_data)
{
	chan *ch = user_data;
	tabview *tabs = (tabview *) ch->cv;
	tab_item *item = g_hash_table_lookup (tabs->state->items, ch);
	gboolean hover = prefs.hex_gui_tab_closebuttons && item &&
		fabulor_gtk_widget_contains_descendant_point (wid,
			item->close_button, x, y);

	if (hover)
	{
		fabulor_gtk_widget_set_prelight (item->close_button, TRUE);
		fabulor_gtk_widget_set_pointing_cursor (wid, TRUE);
	}
	else
	{
		if (item)
			fabulor_gtk_widget_set_prelight (item->close_button, FALSE);
		fabulor_gtk_widget_set_pointing_cursor (wid, FALSE);
	}
}

static void
tab_close_leave_cb (GtkWidget *wid, gpointer user_data)
{
	chan *ch = user_data;
	tabview *tabs = (tabview *) ch->cv;
	tab_item *item = g_hash_table_lookup (tabs->state->items, ch);

	if (prefs.hex_gui_tab_closebuttons && item)
		fabulor_gtk_widget_set_prelight (item->close_button, FALSE);
	fabulor_gtk_widget_set_pointing_cursor (wid, FALSE);
}

static void *
cv_tabs_add (chanview *cv, chan *ch, char *name, chan *parent)
{
	tabview *tabs = (tabview *) cv;
	tab_item *item = g_new0 (tab_item, 1);
	GtkWidget *hbox;
	GtkWidget *close_icon;

	item->tab = gtk_toggle_button_new ();
	gtk_widget_set_name (item->tab, "zoitechat-tab");
	gtk_widget_set_size_request (item->tab, -1, 14);
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	item->label = gtk_label_new (name);
	item->close_button = gtk_button_new ();
	gtk_style_context_add_class (gtk_widget_get_style_context (item->close_button), "flat");
	close_icon = gtk_image_new_from_icon_name ("window-close-symbolic", GTK_ICON_SIZE_MENU);
	gtk_image_set_pixel_size (GTK_IMAGE (close_icon), 8);
	gtk_button_set_always_show_image (GTK_BUTTON (item->close_button), TRUE);
	gtk_widget_set_can_focus (item->close_button, FALSE);
	fabulor_gtk_button_set_child (GTK_BUTTON (item->close_button), close_icon);
	fabulor_gtk_box_append (GTK_BOX (hbox), item->label, TRUE, TRUE, 0);
	fabulor_gtk_box_append (GTK_BOX (hbox), item->close_button, FALSE, FALSE, 0);
	fabulor_gtk_button_set_child (GTK_BUTTON (item->tab), hbox);
	g_hash_table_insert (tabs->state->items, ch, item);
	/* used for close-button hit testing and context actions */
	fabulor_gtk_widget_on_multi_click (item->tab, tab_click_cb, ch);
	fabulor_gtk_widget_on_scroll (item->tab, tab_scroll_cb, cv);
	fabulor_gtk_widget_on_scroll (item->close_button, tab_scroll_cb, cv);
	fabulor_gtk_widget_on_pointer_motion (item->tab, tab_close_motion_cb,
									  tab_close_leave_cb, ch);
	/* Keep close-button hover distinct from whole-tab hover. */
	fabulor_gtk_widget_suppress_pointer_prelight (item->tab);
	g_signal_connect (G_OBJECT (item->tab), "pressed",
							G_CALLBACK (tab_pressed_cb), ch);
	/* for keyboard */
	g_signal_connect (G_OBJECT (item->tab), "toggled",
						 	G_CALLBACK (tab_toggled_cb), ch);
	gtk_widget_show_all (hbox);
	if (!prefs.hex_gui_tab_closebuttons)
		gtk_widget_hide (item->close_button);

	tab_add_real (cv, item->tab, ch, parent);

	return item->tab;
}

static void
cv_tabs_focus (chan *ch)
{
	if (ch->impl)
	/* focus the new one (tab_pressed_cb defocuses the old one) */
		tab_pressed_cb (GTK_TOGGLE_BUTTON (ch->impl), ch);
}

static void
cv_tabs_change_orientation (chanview *cv)
{
	/* cleanup the old one */
	if (cv->func_cleanup)
		cv->func_cleanup (cv);

	/* now rebuild a new tabbar or tree */
	cv->func_init (cv);
	chanview_populate (cv);
}

/* switch to the tab number specified */

static void
cv_tabs_move_focus (chanview *cv, gboolean relative, int num)
{
	chan *ch;

	if (relative)
	{
		num += cv_find_number_of_chan (cv, cv->focused);
		num %= cv->size;
		if (num < 0)
			num = cv->size - 1;
	}
	ch = cv_find_chan_by_number (cv, num);
	if (ch)
		cv_tabs_focus (ch);
}

static void
cv_tabs_remove (chan *ch)
{
	tabview *tabs = (tabview *) ch->cv;
	tab_item *item = g_hash_table_lookup (tabs->state->items, ch);
	chan *parent = fabulor_channel_model_get_parent (ch->cv->model, ch);
	tab_family *family;

	gtk_widget_destroy (item ? item->tab : ch->impl);
	g_hash_table_remove (tabs->state->items, ch);
	ch->impl = NULL;
	if (parent)
		return;

	family = g_hash_table_lookup (tabs->state->families, ch);
	if (family)
	{
		gtk_widget_destroy (family->box);
		g_hash_table_remove (tabs->state->families, ch);
	}
}

static void cv_tabs_move_family (chan *ch, int delta);

static void
cv_tabs_move (chan *ch, int delta)
{
	tabview *tabs = (tabview *) ch->cv;
	chan *parent = fabulor_channel_model_get_parent (ch->cv->model, ch);
	tab_family *family;
	guint count;
	guint position;

	if (!parent)
	{
		cv_tabs_move_family (ch, delta);
		return;
	}
	family = g_hash_table_lookup (tabs->state->families, parent);
	if (!family)
		return;
	count = fabulor_channel_model_get_child_count (ch->cv->model, parent);
	for (position = 0; position < count; position++)
		if (fabulor_channel_model_get_child_at (ch->cv->model, parent,
			position) == ch)
		{
			/* The server tab occupies position zero in each family box. */
			gtk_box_reorder_child (GTK_BOX (family->box),
				ch->impl, (gint) position + 1);
			return;
		}
}

static void
cv_tabs_move_family (chan *ch, int delta)
{
	tabview *tabs = (tabview *) ch->cv;
	chan *root = fabulor_channel_model_get_parent (ch->cv->model, ch);
	tab_family *family;
	guint count = fabulor_channel_model_get_root_count (ch->cv->model);
	guint position;

	(void) delta;
	if (!root)
		root = ch;
	family = g_hash_table_lookup (tabs->state->families, root);
	if (!family)
		return;
	for (position = 0; position < count; position++)
		if (fabulor_channel_model_get_root_at (ch->cv->model, position) == root)
		{
			gtk_box_reorder_child (GTK_BOX (tabs->inner), family->box,
				(gint) position);
			return;
		}
}

static void
cv_tabs_cleanup (chanview *cv)
{
	tabview *tabs = (tabview *) cv;

	tab_scroll_animation_cancel (&tabs->backward_animation);
	tab_scroll_animation_cancel (&tabs->forward_animation);
	if (cv->box)
		gtk_widget_destroy (tabs->outer);
	if (tabs->state)
	{
		g_hash_table_destroy (tabs->state->items);
		g_hash_table_destroy (tabs->state->families);
		g_free (tabs->state);
		tabs->state = NULL;
	}
	tabs->outer = NULL;
	tabs->inner = NULL;
}

static void
cv_tabs_set_color (chan *ch, PangoAttrList *list)
{
	tabview *tabs = (tabview *) ch->cv;
	tab_item *item = g_hash_table_lookup (tabs->state->items, ch);

	g_return_if_fail (item != NULL);
	gtk_label_set_attributes (GTK_LABEL (item->label), list);
}

static void
cv_tabs_rename (chan *ch, char *name)
{
	tabview *tabs = (tabview *) ch->cv;
	tab_item *item = g_hash_table_lookup (tabs->state->items, ch);
	PangoAttrList *attr;

	g_return_if_fail (item != NULL);
	attr = gtk_label_get_attributes (GTK_LABEL (item->label));
	if (attr)
		pango_attr_list_ref (attr);

	gtk_label_set_text (GTK_LABEL (item->label), name);
	gtk_widget_queue_resize (gtk_widget_get_parent (gtk_widget_get_parent (
		gtk_widget_get_parent (item->tab))));

	if (attr)
	{
		gtk_label_set_attributes (GTK_LABEL (item->label), attr);
		pango_attr_list_unref (attr);
	}
}

static gboolean
cv_tabs_is_collapsed (chan *ch)
{
	return FALSE;
}

static chan *
cv_tabs_get_parent (chan *ch)
{
	return NULL;
}
