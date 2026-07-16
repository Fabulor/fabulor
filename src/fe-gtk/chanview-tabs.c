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

typedef struct
{
	GtkWidget *outer;	/* outer box */
	GtkWidget *inner;	/* inner box */
	tab_scroll_animation *backward_animation;
	tab_scroll_animation *forward_animation;
	int backward_is_moving;
	int forward_is_moving;
	int ignore_toggle;
} tabview;

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

/* userdata for gobjects used here:
 *
 * tab (togglebuttons inside boxes):
 *   "u" userdata passed to tab-focus callback function (sess)
 *   "c" the tab's (chan *)
 *
 * box (family box)
 *   "f" family
 *
 */

static inline gint
cv_tabs_get_viewport_size (GtkAdjustment *adj)
{
	return (gint) gtk_adjustment_get_page_size (adj);
}

static gint
tab_search_offset (GtkWidget *inner, gint start_offset,
				   gboolean forward, gboolean vertical)
{
	GList *boxes;
	GList *tabs;
	GtkWidget *box;
	GtkWidget *button;
	GtkAllocation allocation;
	gint found;

	boxes = gtk_container_get_children (GTK_CONTAINER (inner));
	if (!forward && boxes)
		boxes = g_list_last (boxes);

	while (boxes)
	{
		box = (GtkWidget *)boxes->data;
		boxes = (forward ? boxes->next : boxes->prev);

		tabs = gtk_container_get_children (GTK_CONTAINER (box));
		if (!forward && tabs)
			tabs = g_list_last (tabs);

		while (tabs)
		{
			button = (GtkWidget *)tabs->data;
			tabs = (forward ? tabs->next : tabs->prev);

			if (!GTK_IS_TOGGLE_BUTTON (button))
				continue;

			gtk_widget_get_allocation (button, &allocation);
			found = (vertical ? allocation.y : allocation.x);
			if ((forward && found > start_offset) ||
				(!forward && found < start_offset))
				return found;
		}
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
	gfloat new_value;
	GtkWidget *inner;

	inner = ((tabview *)cv)->inner;

	if (cv->vertical)
	{
		adj = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (gtk_widget_get_parent(inner)));
	} else
	{
		adj = gtk_scrollable_get_hadjustment (GTK_SCROLLABLE (gtk_widget_get_parent(inner)));
	}

	viewport_size = cv_tabs_get_viewport_size (adj);

	new_value = tab_search_offset (inner, gtk_adjustment_get_value (adj), 0, cv->vertical);

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
	gfloat new_value;
	GtkWidget *inner;

	inner = ((tabview *)cv)->inner;

	if (cv->vertical)
	{
		adj = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (gtk_widget_get_parent(inner)));
	} else
	{
		adj = gtk_scrollable_get_hadjustment (GTK_SCROLLABLE (gtk_widget_get_parent(inner)));
	}

	viewport_size = cv_tabs_get_viewport_size (adj);

	new_value = tab_search_offset (inner, gtk_adjustment_get_value (adj), 1, cv->vertical);

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
	((tabview *)cv)->outer = outer;
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
	((tabview *)cv)->inner = box;
	gtk_container_add (GTK_CONTAINER (viewport), box);
	gtk_widget_show (box);

	gtk_container_add (GTK_CONTAINER (cv->box), outer);
}

static void
cv_tabs_postinit (chanview *cv)
{
}

static void
tab_add_sorted (chanview *cv, GtkWidget *box, GtkWidget *tab, chan *ch)
{
	GList *list;
	GtkWidget *child;
	int i = 0;
	void *b;

	if (!cv->sorted)
	{
		fabulor_gtk_box_append (GTK_BOX (box), tab, FALSE, FALSE, 0);
		gtk_widget_show (tab);
		return;
	}

	/* sorting TODO:
    *   - move tab if renamed (dialogs) */

	/* userdata, passed to mg_tabs_compare() */
	b = ch->userdata;

	list = gtk_container_get_children (GTK_CONTAINER (box));
	while (list)
	{
		child = list->data;
		if (!GTK_IS_SEPARATOR (child))
		{
			void *a = g_object_get_data (G_OBJECT (child), "u");

			if (ch->tag == 0 && cv->cb_compare (a, b) > 0)
			{
				fabulor_gtk_box_append (GTK_BOX (box), tab, FALSE, FALSE, 0);
				gtk_box_reorder_child (GTK_BOX (box), tab, ++i);
				gtk_widget_show (tab);
				return;
			}
		}
		i++;
		list = list->next;
	}

	/* append */
	fabulor_gtk_box_append (GTK_BOX (box), tab, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (box), tab, i);
	gtk_widget_show (tab);
}

/* remove empty boxes and separators */

static void
cv_tabs_prune (chanview *cv)
{
	GList *boxes, *children;
	GtkWidget *box, *inner;
	GtkWidget *child;
	int empty;

	inner = ((tabview *)cv)->inner;
	boxes = gtk_container_get_children (GTK_CONTAINER (inner));
	while (boxes)
	{
		child = boxes->data;
		box = child;
		boxes = boxes->next;

		/* check if the box is empty (except a vseperator) */
		empty = TRUE;
		children = gtk_container_get_children (GTK_CONTAINER (box));
		while (children)
		{
			if (!GTK_IS_SEPARATOR ((GtkWidget *)children->data))
			{
				empty = FALSE;
				break;
			}
			children = children->next;
		}

		if (empty)
			gtk_widget_destroy (box);
	}
}

static void
tab_add_real (chanview *cv, GtkWidget *tab, chan *ch)
{
	GList *boxes, *children;
	GtkWidget *sep, *box, *inner;
	GtkWidget *child;
	int empty;

	inner = ((tabview *)cv)->inner;
	/* see if a family for this tab already exists */
	boxes = gtk_container_get_children (GTK_CONTAINER (inner));
	while (boxes)
	{
		child = boxes->data;
		box = child;

		if (g_object_get_data (G_OBJECT (box), "f") == ch->family)
		{
			tab_add_sorted (cv, box, tab, ch);
			gtk_widget_queue_resize (gtk_widget_get_parent(inner));
			return;
		}

		boxes = boxes->next;

		/* check if the box is empty (except a vseperator) */
		empty = TRUE;
		children = gtk_container_get_children (GTK_CONTAINER (box));
		while (children)
		{
			if (!GTK_IS_SEPARATOR ((GtkWidget *)children->data))
			{
				empty = FALSE;
				break;
			}
			children = children->next;
		}

		if (empty)
			gtk_widget_destroy (box);
	}

	/* create a new family box */
	if (cv->vertical)
	{
		/* vertical */
		box = gtkutil_box_new (GTK_ORIENTATION_VERTICAL, FALSE, 0);
		sep = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	} else
	{
		/* horiz */
		box = gtkutil_box_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 0);
		sep = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
	}

	gtk_widget_show (sep);
	fabulor_gtk_box_append (GTK_BOX (inner), box, FALSE, FALSE, 0);
	g_object_set_data (G_OBJECT (box), "f", ch->family);
	fabulor_gtk_box_append (GTK_BOX (box), tab, FALSE, FALSE, 0);
	fabulor_gtk_box_append (GTK_BOX (box), sep, FALSE, FALSE, 4);
	gtk_widget_show (tab);
	gtk_widget_show (box);
	gtk_widget_queue_resize (gtk_widget_get_parent(inner));
}

static gboolean
tab_ignore_cb (GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
	return TRUE;
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
	GtkWidget *close_button;
	gint close_x;
	gint close_y;
	GtkAllocation close_alloc;

	(void) n_press;
	if (button == 1)
	{
		close_button = g_object_get_data (G_OBJECT (wid), "tab-close-button");
		if (prefs.hex_gui_tab_closebuttons && close_button &&
			gtk_widget_translate_coordinates (close_button, wid, 0, 0, &close_x, &close_y))
		{
			gtk_widget_get_allocation (close_button, &close_alloc);
			if (x >= close_x && x < close_x + close_alloc.width &&
				y >= close_y && y < close_y + close_alloc.height)
			{
				ch->cv->cb_xbutton (ch->cv, ch, ch->tag, ch->userdata);
				return TRUE;
			}
		}
	}

	return ch->cv->cb_contextmenu (ch->cv, ch, ch->tag, ch->userdata, wid,
		button, x, y, state);
}

static void
tab_close_motion_cb (GtkWidget *wid, gdouble x, gdouble y, gpointer user_data)
{
	GtkWidget *close_button;
	gint close_x;
	gint close_y;
	GtkAllocation close_alloc;
	gboolean hover = FALSE;

	(void) user_data;

	close_button = g_object_get_data (G_OBJECT (wid), "tab-close-button");
	if (prefs.hex_gui_tab_closebuttons && close_button &&
		gtk_widget_translate_coordinates (close_button, wid, 0, 0, &close_x, &close_y))
	{
		gtk_widget_get_allocation (close_button, &close_alloc);
		hover = x >= close_x && x < close_x + close_alloc.width &&
			y >= close_y && y < close_y + close_alloc.height;
	}

	if (hover)
	{
		gtk_widget_set_state_flags (close_button, GTK_STATE_FLAG_PRELIGHT, TRUE);
		fabulor_gtk_widget_set_pointing_cursor (wid, TRUE);
	}
	else
	{
		if (close_button)
			gtk_widget_unset_state_flags (close_button, GTK_STATE_FLAG_PRELIGHT);
		fabulor_gtk_widget_set_pointing_cursor (wid, FALSE);
	}
}

static void
tab_close_leave_cb (GtkWidget *wid, gpointer user_data)
{
	GtkWidget *close_button;

	(void) user_data;

	close_button = g_object_get_data (G_OBJECT (wid), "tab-close-button");
	if (prefs.hex_gui_tab_closebuttons && close_button)
		gtk_widget_unset_state_flags (close_button, GTK_STATE_FLAG_PRELIGHT);
	fabulor_gtk_widget_set_pointing_cursor (wid, FALSE);
}

static GtkWidget *
tab_get_label (GtkWidget *tab)
{
	GtkWidget *label;

	label = g_object_get_data (G_OBJECT (tab), "tab-label");
	if (label)
		return label;

	return gtk_bin_get_child (GTK_BIN (tab));
}

static void *
cv_tabs_add (chanview *cv, chan *ch, char *name, chan *parent)
{
	GtkWidget *but;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *close_button;
	GtkWidget *close_icon;

	(void) parent;

	but = gtk_toggle_button_new ();
	gtk_widget_set_name (but, "zoitechat-tab");
	gtk_widget_set_size_request (but, -1, 14);
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	label = gtk_label_new (name);
	close_button = gtk_button_new ();
	gtk_style_context_add_class (gtk_widget_get_style_context (close_button), "flat");
	close_icon = gtk_image_new_from_icon_name ("window-close-symbolic", GTK_ICON_SIZE_MENU);
	gtk_image_set_pixel_size (GTK_IMAGE (close_icon), 8);
	gtk_button_set_always_show_image (GTK_BUTTON (close_button), TRUE);
	gtk_widget_set_can_focus (close_button, FALSE);
	fabulor_gtk_button_set_child (GTK_BUTTON (close_button), close_icon);
	fabulor_gtk_box_append (GTK_BOX (hbox), label, TRUE, TRUE, 0);
	fabulor_gtk_box_append (GTK_BOX (hbox), close_button, FALSE, FALSE, 0);
	fabulor_gtk_button_set_child (GTK_BUTTON (but), hbox);
	g_object_set_data (G_OBJECT (but), "tab-label", label);
	g_object_set_data (G_OBJECT (but), "tab-close-button", close_button);
	g_object_set_data (G_OBJECT (but), "c", ch);
	/* used for close-button hit testing and context actions */
	fabulor_gtk_widget_on_multi_click (but, tab_click_cb, ch);
	fabulor_gtk_widget_on_scroll (but, tab_scroll_cb, cv);
	fabulor_gtk_widget_on_scroll (close_button, tab_scroll_cb, cv);
	fabulor_gtk_widget_on_pointer_motion (but, tab_close_motion_cb,
									  tab_close_leave_cb, NULL);
	/* avoid prelights */
	g_signal_connect (G_OBJECT (but), "enter-notify-event",
						 	G_CALLBACK (tab_ignore_cb), NULL);
	g_signal_connect (G_OBJECT (but), "leave-notify-event",
						 	G_CALLBACK (tab_ignore_cb), NULL);
	g_signal_connect (G_OBJECT (but), "pressed",
							G_CALLBACK (tab_pressed_cb), ch);
	/* for keyboard */
	g_signal_connect (G_OBJECT (but), "toggled",
						 	G_CALLBACK (tab_toggled_cb), ch);
	g_object_set_data (G_OBJECT (but), "u", ch->userdata);
	gtk_widget_show_all (hbox);
	if (!prefs.hex_gui_tab_closebuttons)
		gtk_widget_hide (close_button);

	tab_add_real (cv, but, ch);

	return but;
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
	gtk_widget_destroy (ch->impl);
	ch->impl = NULL;

	cv_tabs_prune (ch->cv);
}

static void
cv_tabs_move (chan *ch, int delta)
{
	int i = 0;
	int pos = 0;
	GList *list;
	GtkWidget *parent = gtk_widget_get_parent(GTK_WIDGET (ch->impl));

	for (list = gtk_container_get_children (GTK_CONTAINER (parent)); list; list = list->next)
	{
		GtkWidget *child_entry;

		child_entry = list->data;
		if (child_entry == ch->impl)
			pos = i;

		/* keep separator at end to not throw off our count */
		if (GTK_IS_SEPARATOR (child_entry))
			gtk_box_reorder_child (GTK_BOX (parent), child_entry, -1);
		else
			i++;
	}

	pos = (pos - delta) % i;
	gtk_box_reorder_child (GTK_BOX (parent), ch->impl, pos);
}

static void
cv_tabs_move_family (chan *ch, int delta)
{
	int i, pos = 0;
	GList *list;
	GtkWidget *box = NULL;

	/* find position of tab's family */
	i = 0;
	for (list = gtk_container_get_children (GTK_CONTAINER (((tabview *)ch->cv)->inner)); list; list = list->next)
	{
		GtkWidget *child_entry;
		void *fam;

		child_entry = list->data;
		fam = g_object_get_data (G_OBJECT (child_entry), "f");
		if (fam == ch->family)
		{
			box = child_entry;
			pos = i;
		}
		i++;
	}

	pos = (pos - delta) % i;
	gtk_box_reorder_child (GTK_BOX (gtk_widget_get_parent(box)), box, pos);
}

static void
cv_tabs_cleanup (chanview *cv)
{
	tabview *tabs = (tabview *) cv;

	tab_scroll_animation_cancel (&tabs->backward_animation);
	tab_scroll_animation_cancel (&tabs->forward_animation);
	if (cv->box)
		gtk_widget_destroy (tabs->outer);
}

static void
cv_tabs_set_color (chan *ch, PangoAttrList *list)
{
	gtk_label_set_attributes (GTK_LABEL (tab_get_label (ch->impl)), list);
}

static void
cv_tabs_rename (chan *ch, char *name)
{
	PangoAttrList *attr;
	GtkWidget *tab = ch->impl;

	attr = gtk_label_get_attributes (GTK_LABEL (tab_get_label (tab)));
	if (attr)
		pango_attr_list_ref (attr);

	gtk_label_set_text (GTK_LABEL (tab_get_label (tab)), name);
	gtk_widget_queue_resize (gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(tab))));

	if (attr)
	{
		gtk_label_set_attributes (GTK_LABEL (tab_get_label (tab)), attr);
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
