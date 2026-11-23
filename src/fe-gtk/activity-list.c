#include "activity-list.h"
#include "context-menu-presenter-gtk4.h"

#include <glib/gi18n.h>
#include <string.h>

typedef struct _FabulorActivityRow FabulorActivityRow;
typedef struct _FabulorActivityRowClass FabulorActivityRowClass;

struct _FabulorActivityRow
{
	GObject parent_instance;
	gpointer identity;
	guint position;
	char *context;
	char *network;
	char *highlight_preview;
	ActivityState activity;
};

struct _FabulorActivityRowClass
{
	GObjectClass parent_class;
};

struct _FabulorActivityList
{
	GListStore *store;
	GHashTable *rows;
	GtkCustomFilter *filter;
	GtkSortListModel *sorted;
	GtkSingleSelection *selection;
	GtkWidget *box;
	GtkWidget *stack;
	GtkWidget *view;
	FabulorActivityActivate activate;
	FabulorActivityActivate mark_read;
	gpointer activate_data;
	FabulorContextMenuPresenterGtk4 *context_menu;
	FabulorActivityRow *context_row;
	GtkEventController *context_click;
	GtkEventController *context_keys;
	gboolean highlights_only;
};

static guint row_changed_signal;

G_DEFINE_TYPE (FabulorActivityRow, fabulor_activity_row, G_TYPE_OBJECT)

static void
fabulor_activity_row_finalize (GObject *object)
{
	FabulorActivityRow *row = (FabulorActivityRow *) object;

	g_free (row->context);
	g_free (row->network);
	g_free (row->highlight_preview);
	G_OBJECT_CLASS (fabulor_activity_row_parent_class)->finalize (object);
}

static void
fabulor_activity_row_class_init (FabulorActivityRowClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = fabulor_activity_row_finalize;
	row_changed_signal = g_signal_new ("changed", G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void
fabulor_activity_row_init (FabulorActivityRow *row)
{
	(void) row;
}

static int
activity_row_compare (gconstpointer a, gconstpointer b, gpointer data)
{
	const FabulorActivityRow *left = a;
	const FabulorActivityRow *right = b;

	(void) data;
	if (left->activity.highest != right->activity.highest)
		return left->activity.highest > right->activity.highest ? -1 : 1;
	if (left->activity.latest_sequence != right->activity.latest_sequence)
		return left->activity.latest_sequence > right->activity.latest_sequence ? -1 : 1;
	if (left->identity == right->identity)
		return 0;
	return (guintptr) left->identity < (guintptr) right->identity ? -1 : 1;
}

static FabulorActivityRow *
activity_list_ref_selected (FabulorActivityList *list)
{
	FabulorActivityRow *selected = gtk_single_selection_get_selected_item (
		list->selection);
	return selected ? g_object_ref (selected) : NULL;
}

static void
activity_list_restore_selection (FabulorActivityList *list, FabulorActivityRow *row)
{
	guint first = 0;
	guint end = g_list_model_get_n_items (G_LIST_MODEL (list->sorted));

	if (!row || g_hash_table_lookup (list->rows, row->identity) != row ||
		gtk_single_selection_get_selected_item (list->selection) == row)
		return;

	/* Resolve the same row after a sort-key change or a store slot move. */
	while (first < end)
	{
		guint mid = first + (end - first) / 2;
		FabulorActivityRow *candidate = g_list_model_get_item (
			G_LIST_MODEL (list->sorted), mid);
		int order = activity_row_compare (row, candidate, NULL);
		g_object_unref (candidate);
		if (order == 0)
		{
			gtk_single_selection_set_selected (list->selection, mid);
			return;
		}
		if (order < 0)
			end = mid;
		else
			first = mid + 1;
	}
}

static void
activity_list_sync_empty (FabulorActivityList *list)
{
	gtk_stack_set_visible_child_name (GTK_STACK (list->stack),
		g_list_model_get_n_items (G_LIST_MODEL (list->sorted)) ? "unread" :
		list->highlights_only ? "no-highlights" : "empty");
}

static gboolean
activity_list_filter (gpointer item, gpointer data)
{
	FabulorActivityList *list = data;
	FabulorActivityRow *row = item;

	return !list->highlights_only || row->activity.highest == ACTIVITY_HIGHLIGHT;
}

static void
activity_list_toggle_highlights (GtkCheckButton *toggle, gpointer data)
{
	FabulorActivityList *list = data;
	FabulorActivityRow *selected = activity_list_ref_selected (list);

	list->highlights_only = gtk_check_button_get_active (toggle);
	gtk_filter_changed (GTK_FILTER (list->filter), GTK_FILTER_CHANGE_DIFFERENT);
	activity_list_restore_selection (list, selected);
	g_clear_object (&selected);
	activity_list_sync_empty (list);
}

void
fabulor_activity_list_remove (FabulorActivityList *list, gpointer identity)
{
	FabulorActivityRow *row = g_hash_table_lookup (list->rows, identity);

	if (row)
	{
		guint position = row->position;
		guint last = g_list_model_get_n_items (G_LIST_MODEL (list->store)) - 1;
		FabulorActivityRow *selected = activity_list_ref_selected (list);

		g_hash_table_remove (list->rows, identity);
		if (list->context_row == row)
		{
			g_clear_object (&list->context_row);
			gtk_popover_popdown (GTK_POPOVER (
				fabulor_context_menu_presenter_gtk4_get_popover (list->context_menu)));
		}
		if (position == last)
			g_list_store_remove (list->store, last);
		else
		{
			/* Keep store positions current with one swap. Display order is sorted. */
			FabulorActivityRow *tail = g_list_model_get_item (
				G_LIST_MODEL (list->store), last);
			gpointer replacement = tail;

			g_list_store_remove (list->store, last);
			tail->position = position;
			g_list_store_splice (list->store, position, 1, &replacement, 1);
			g_object_unref (tail);
		}
		activity_list_restore_selection (list, selected);
		g_clear_object (&selected);
		activity_list_sync_empty (list);
	}
}

void
fabulor_activity_list_update (FabulorActivityList *list, gpointer identity,
							 const char *context, const char *network,
							 const ActivityState *activity, const char *highlight_preview)
{
	FabulorActivityRow *row;
	gboolean reorder;
	gboolean undisplayed;

	if (activity->highest == ACTIVITY_NONE)
	{
		fabulor_activity_list_remove (list, identity);
		return;
	}

	row = g_hash_table_lookup (list->rows, identity);
	undisplayed = row == NULL;
	if (undisplayed)
	{
		row = g_object_new (fabulor_activity_row_get_type (), NULL);
		row->identity = identity;
		row->position = g_list_model_get_n_items (G_LIST_MODEL (list->store));
	}

	reorder = row->activity.highest != activity->highest ||
		row->activity.latest_sequence != activity->latest_sequence;
	if (!reorder && g_strcmp0 (row->context, context) == 0 &&
		g_strcmp0 (row->network, network) == 0 &&
		g_strcmp0 (row->highlight_preview, highlight_preview) == 0 &&
		memcmp (row->activity.unread, activity->unread, sizeof activity->unread) == 0)
		return;

	if (g_strcmp0 (row->context, context) != 0)
	{
		g_free (row->context);
		row->context = g_strdup (context);
	}
	if (g_strcmp0 (row->network, network) != 0)
	{
		g_free (row->network);
		row->network = g_strdup (network);
	}
	if (g_strcmp0 (row->highlight_preview, highlight_preview) != 0)
	{
		g_free (row->highlight_preview);
		row->highlight_preview = g_strdup (highlight_preview);
	}
	row->activity = *activity;

	if (undisplayed)
	{
		g_hash_table_insert (list->rows, identity, row);
		g_list_store_append (list->store, row);
		g_object_unref (row);
		activity_list_sync_empty (list);
		return;
	}

	if (reorder)
	{
		FabulorActivityRow *selected = activity_list_ref_selected (list);
		gpointer replacement = row;

		/* Reinsert only this sort key while retaining the same row object. */
		g_object_ref (row);
		g_list_store_splice (list->store, row->position, 1, &replacement, 1);
		activity_list_restore_selection (list, selected);
		g_clear_object (&selected);
		g_object_unref (row);
		activity_list_sync_empty (list);
	}
	g_signal_emit (row, row_changed_signal, 0);
}

typedef struct
{
	GtkListItem *item;
	GtkLabel *context;
	GtkLabel *network;
	GtkLabel *detail;
	GtkLabel *preview;
	FabulorActivityRow *row;
	gulong handler;
} ActivityRowBinding;

static const char *
activity_priority_name (ActivityClass priority)
{
	switch (priority)
	{
	case ACTIVITY_HIGHLIGHT: return _("Highlight");
	case ACTIVITY_PRIVATE: return _("Private message");
	case ACTIVITY_MESSAGE: return _("Message");
	default: return _("Other activity");
	}
}

static GString *
activity_row_detail (const ActivityState *activity)
{
	static const char *const singular[] = {
		NULL, N_("%u other event"), N_("%u message"), N_("%u private message"),
		N_("%u highlight")
	};
	static const char *const plural[] = {
		NULL, N_("%u other events"), N_("%u messages"), N_("%u private messages"),
		N_("%u highlights")
	};
	GString *detail = g_string_new (NULL);

	for (int i = ACTIVITY_HIGHLIGHT; i >= ACTIVITY_OTHER; --i)
	{
		guint count = activity->unread[i];

		if (count)
		{
			if (detail->len)
				g_string_append (detail, " · ");
			g_string_append_printf (detail, ngettext (singular[i], plural[i], count), count);
		}
	}
	return detail;
}

static void
activity_binding_update (FabulorActivityRow *row, gpointer data)
{
	ActivityRowBinding *binding = data;
	GString *detail = activity_row_detail (&row->activity);
	guint64 total = 0;
	char *label;
	char *count;
	char *description;

	for (int i = ACTIVITY_OTHER; i < ACTIVITY_CLASS_COUNT; ++i)
		total += row->activity.unread[i];
	label = g_strdup_printf (_("%s on %s"), row->context, row->network);
	count = g_strdup_printf ("%" G_GUINT64_FORMAT, total);
	/* Translators: priority, exact number of unread events, and breakdown. */
	description = g_strdup_printf (_("%s priority. %s unread. %s"),
		activity_priority_name (row->activity.highest), count, detail->str);
	gtk_label_set_text (binding->context, row->context);
	gtk_label_set_text (binding->network, row->network);
	gtk_label_set_text (binding->detail, detail->str);
	gtk_label_set_text (binding->preview, row->highlight_preview ? row->highlight_preview : "");
	gtk_widget_set_visible (GTK_WIDGET (binding->preview), row->highlight_preview != NULL);
	gtk_widget_set_tooltip_text (GTK_WIDGET (binding->preview), row->highlight_preview);
	if (row->highlight_preview)
	{
		char *summary = g_strdup_printf (_("%s. Latest highlight: %s"), description, row->highlight_preview);
		g_free (description);
		description = summary;
	}
	gtk_list_item_set_accessible_label (binding->item, label);
	gtk_list_item_set_accessible_description (binding->item, description);
	g_free (description);
	g_free (count);
	g_free (label);
	g_string_free (detail, TRUE);
}

static GtkLabel *
activity_label (void)
{
	GtkLabel *label = GTK_LABEL (gtk_label_new (NULL));

	gtk_label_set_xalign (label, 0);
	gtk_label_set_ellipsize (label, PANGO_ELLIPSIZE_END);
	return label;
}

static void
activity_factory_setup (GtkSignalListItemFactory *factory, GtkListItem *item,
						gpointer data)
{
	ActivityRowBinding *binding = g_new0 (ActivityRowBinding, 1);
	GtkWidget *row = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 3);
	GtkWidget *heading = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);

	(void) factory;
	(void) data;
	binding->item = item;
	binding->context = activity_label ();
	binding->network = activity_label ();
	binding->detail = activity_label ();
	binding->preview = activity_label ();
	gtk_label_set_single_line_mode (binding->preview, TRUE);
	gtk_widget_set_visible (GTK_WIDGET (binding->preview), FALSE);
	gtk_label_set_ellipsize (binding->detail, PANGO_ELLIPSIZE_NONE);
	gtk_label_set_wrap (binding->detail, TRUE);
	gtk_label_set_wrap_mode (binding->detail, PANGO_WRAP_WORD_CHAR);
	gtk_widget_set_hexpand (GTK_WIDGET (binding->context), TRUE);
	gtk_label_set_max_width_chars (binding->network, 24);
	gtk_widget_add_css_class (GTK_WIDGET (binding->context), "heading");
	gtk_widget_add_css_class (GTK_WIDGET (binding->network), "dim-label");
	gtk_widget_add_css_class (GTK_WIDGET (binding->detail), "dim-label");
	gtk_widget_set_margin_start (box, 12);
	gtk_widget_set_margin_end (box, 12);
	gtk_widget_set_margin_top (box, 6);
	gtk_widget_set_margin_bottom (box, 6);
	gtk_box_append (GTK_BOX (heading), GTK_WIDGET (binding->context));
	gtk_box_append (GTK_BOX (heading), GTK_WIDGET (binding->network));
	gtk_box_append (GTK_BOX (box), heading);
	gtk_box_append (GTK_BOX (box), GTK_WIDGET (binding->preview));
	gtk_box_append (GTK_BOX (box), GTK_WIDGET (binding->detail));
	gtk_box_append (GTK_BOX (row), box);
	gtk_list_item_set_child (item, row);
	g_object_set_data (G_OBJECT (row), "fabulor-activity-binding", binding);
	g_object_set_data_full (G_OBJECT (item), "fabulor-activity-binding", binding, g_free);
}

static void
activity_factory_bind (GtkSignalListItemFactory *factory, GtkListItem *item,
					   gpointer data)
{
	ActivityRowBinding *binding = g_object_get_data (
		G_OBJECT (item), "fabulor-activity-binding");

	(void) factory;
	(void) data;
	binding->row = g_object_ref (gtk_list_item_get_item (item));
	binding->handler = g_signal_connect (binding->row, "changed",
		G_CALLBACK (activity_binding_update), binding);
	activity_binding_update (binding->row, binding);
}

static void
activity_factory_unbind (GtkSignalListItemFactory *factory, GtkListItem *item,
						 gpointer data)
{
	ActivityRowBinding *binding = g_object_get_data (
		G_OBJECT (item), "fabulor-activity-binding");

	(void) factory;
	(void) data;
	g_clear_signal_handler (&binding->handler, binding->row);
	g_clear_object (&binding->row);
	gtk_label_set_text (binding->preview, "");
	gtk_widget_set_visible (GTK_WIDGET (binding->preview), FALSE);
	gtk_widget_set_tooltip_text (GTK_WIDGET (binding->preview), NULL);
	gtk_list_item_set_accessible_label (item, NULL);
	gtk_list_item_set_accessible_description (item, NULL);
}

static void
activity_list_activate (GtkListView *view, guint position, gpointer data)
{
	FabulorActivityList *list = data;
	FabulorActivityRow *row = g_list_model_get_item (
		G_LIST_MODEL (gtk_list_view_get_model (view)), position);

	if (row)
	{
		if (list->activate && g_hash_table_lookup (list->rows, row->identity) == row)
			list->activate (row->identity, list->activate_data);
		g_object_unref (row);
	}
}

static void
activity_context_mark_read (GSimpleAction *action, GVariant *parameter, gpointer data)
{
	FabulorActivityList *list = data;
	FabulorActivityRow *row = g_steal_pointer (&list->context_row);

	(void) action;
	(void) parameter;
	gtk_popover_popdown (GTK_POPOVER (
		fabulor_context_menu_presenter_gtk4_get_popover (list->context_menu)));
	if (row && g_hash_table_lookup (list->rows, row->identity) == row)
		list->mark_read (row->identity, list->activate_data);
	g_clear_object (&row);
}

static gboolean
activity_context_popup (FabulorActivityList *list, FabulorActivityRow *row,
						double x, double y)
{
	if (!row)
		return FALSE;

	/* GTK can dismiss the menu before activating its action. Retain the
	 * target until activation, removal, or the next menu invocation. */
	g_set_object (&list->context_row, row);
	return fabulor_context_menu_presenter_gtk4_popup_at (
		list->context_menu, list->view, x, y);
}

static void
activity_context_pressed (GtkGestureClick *gesture, int presses,
						  double x, double y, gpointer data)
{
	FabulorActivityList *list = data;
	GtkWidget *widget = gtk_widget_pick (list->view, x, y, GTK_PICK_DEFAULT);

	(void) presses;
	for (; widget && widget != list->view; widget = gtk_widget_get_parent (widget))
	{
		ActivityRowBinding *binding = g_object_get_data (
			G_OBJECT (widget), "fabulor-activity-binding");

		if (binding && activity_context_popup (list, binding->row, x, y))
		{
			gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_CLAIMED);
			return;
		}
	}
}

static gboolean
activity_context_key (GtkEventControllerKey *controller, guint keyval,
					  guint keycode, GdkModifierType state, gpointer data)
{
	FabulorActivityList *list = data;

	(void) controller;
	(void) keycode;
	if (keyval != GDK_KEY_Menu &&
		(keyval != GDK_KEY_F10 ||
		 (state & gtk_accelerator_get_default_mod_mask ()) != GDK_SHIFT_MASK))
		return FALSE;

	return activity_context_popup (list,
		gtk_single_selection_get_selected_item (list->selection),
		gtk_widget_get_width (list->view) / 2.0,
		gtk_widget_get_height (list->view) / 2.0);
}

static void
activity_context_menu_setup (FabulorActivityList *list)
{
	GMenu *menu = g_menu_new ();
	GSimpleActionGroup *actions = g_simple_action_group_new ();
	GSimpleAction *mark = g_simple_action_new ("mark-read", NULL);
	GtkGesture *click = gtk_gesture_click_new ();
	GtkEventController *keys = gtk_event_controller_key_new ();

	g_signal_connect (mark, "activate", G_CALLBACK (activity_context_mark_read), list);
	g_action_map_add_action (G_ACTION_MAP (actions), G_ACTION (mark));
	g_menu_append (menu, _("Mark as Read"), "activity.mark-read");
	list->context_menu = fabulor_context_menu_presenter_gtk4_new_with_namespaces (
		G_MENU_MODEL (menu), "activity", G_ACTION_GROUP (actions), NULL, NULL);
	g_object_unref (mark);
	g_object_unref (actions);
	g_object_unref (menu);
	gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (click), GDK_BUTTON_SECONDARY);
	g_signal_connect (click, "pressed", G_CALLBACK (activity_context_pressed), list);
	list->context_click = GTK_EVENT_CONTROLLER (click);
	gtk_event_controller_set_propagation_phase (list->context_click, GTK_PHASE_CAPTURE);
	gtk_widget_add_controller (list->view, list->context_click);
	g_signal_connect (keys, "key-pressed", G_CALLBACK (activity_context_key), list);
	list->context_keys = keys;
	gtk_event_controller_set_propagation_phase (keys, GTK_PHASE_CAPTURE);
	gtk_widget_add_controller (list->view, keys);
}

FabulorActivityList *
fabulor_activity_list_new (FabulorActivityActivate activate,
						  FabulorActivityActivate mark_read, gpointer data)
{
	FabulorActivityList *list = g_new0 (FabulorActivityList, 1);
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	GtkWidget *scroll = gtk_scrolled_window_new ();
	GtkWidget *empty = gtk_label_new (_("No unread activity"));
	GtkWidget *no_highlights = gtk_label_new (_("No unread highlights"));
	GtkWidget *toggle = gtk_check_button_new_with_label (_("Highlights only"));
	GtkFilterListModel *filtered;

	list->activate = activate;
	list->mark_read = mark_read;
	list->activate_data = data;
	list->rows = g_hash_table_new (g_direct_hash, g_direct_equal);
	list->store = g_list_store_new (fabulor_activity_row_get_type ());
	list->filter = gtk_custom_filter_new (activity_list_filter, list, NULL);
	filtered = gtk_filter_list_model_new (G_LIST_MODEL (g_object_ref (list->store)),
		GTK_FILTER (g_object_ref (list->filter)));
	list->sorted = gtk_sort_list_model_new (G_LIST_MODEL (filtered),
		GTK_SORTER (gtk_custom_sorter_new (activity_row_compare, NULL, NULL)));
	list->selection = gtk_single_selection_new (G_LIST_MODEL (g_object_ref (list->sorted)));
	gtk_single_selection_set_autoselect (list->selection, FALSE);
	gtk_single_selection_set_can_unselect (list->selection, TRUE);
	g_signal_connect (factory, "setup", G_CALLBACK (activity_factory_setup), NULL);
	g_signal_connect (factory, "bind", G_CALLBACK (activity_factory_bind), NULL);
	g_signal_connect (factory, "unbind", G_CALLBACK (activity_factory_unbind), NULL);
	list->view = gtk_list_view_new (GTK_SELECTION_MODEL (g_object_ref (list->selection)), factory);
	g_signal_connect (list->view, "activate", G_CALLBACK (activity_list_activate), list);
	if (mark_read)
		activity_context_menu_setup (list);
	gtk_accessible_update_property (GTK_ACCESSIBLE (list->view),
		GTK_ACCESSIBLE_PROPERTY_LABEL, _("Activity List"), -1);
	gtk_widget_add_css_class (empty, "dim-label");
	gtk_widget_add_css_class (no_highlights, "dim-label");
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroll), list->view);

	list->box = g_object_ref_sink (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));
	list->stack = gtk_stack_new ();
	gtk_widget_set_vexpand (list->stack, TRUE);
	gtk_stack_add_named (GTK_STACK (list->stack), empty, "empty");
	gtk_stack_add_named (GTK_STACK (list->stack), no_highlights, "no-highlights");
	gtk_stack_add_named (GTK_STACK (list->stack), scroll, "unread");
	gtk_widget_set_margin_start (toggle, 12);
	gtk_widget_set_margin_top (toggle, 6);
	gtk_widget_set_margin_bottom (toggle, 6);
	gtk_box_append (GTK_BOX (list->box), toggle);
	gtk_box_append (GTK_BOX (list->box), list->stack);
	g_signal_connect (toggle, "toggled", G_CALLBACK (activity_list_toggle_highlights), list);
	activity_list_sync_empty (list);
	return list;
}

void
fabulor_activity_list_free (FabulorActivityList *list)
{
	if (list->context_menu)
	{
		gtk_widget_remove_controller (list->view, list->context_click);
		gtk_widget_remove_controller (list->view, list->context_keys);
		fabulor_context_menu_presenter_gtk4_free (list->context_menu);
		g_clear_object (&list->context_row);
	}
	g_signal_handlers_disconnect_by_data (list->view, list);
	g_signal_handlers_disconnect_by_data (gtk_widget_get_first_child (list->box), list);
	gtk_custom_filter_set_filter_func (list->filter, NULL, NULL, NULL);
	g_object_unref (list->box);
	g_object_unref (list->selection);
	g_object_unref (list->sorted);
	g_object_unref (list->filter);
	g_hash_table_unref (list->rows);
	g_object_unref (list->store);
	g_free (list);
}

GtkWidget *
fabulor_activity_list_get_widget (FabulorActivityList *list)
{
	return list->box;
}

void
fabulor_activity_list_focus (FabulorActivityList *list)
{
	if (g_list_model_get_n_items (G_LIST_MODEL (list->sorted)))
	{
		if (!gtk_single_selection_get_selected_item (list->selection))
			gtk_single_selection_set_selected (list->selection, 0);
		gtk_widget_grab_focus (list->view);
	}
}
