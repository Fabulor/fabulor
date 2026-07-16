#include <gtk/gtk.h>

#include <stdio.h>
#include <string.h>

#include "../../src/fe-gtk/gtk-compat.h"
#include "../../src/fe-gtk/gtk4-list-models.h"
#include "../../src/fe-gtk/ignore-list.h"
#include "../../src/fe-gtk/ban-list.h"
#include "../../src/fe-gtk/channel-list.h"
#include "../../src/fe-gtk/dcc-chat-list.h"
#include "../../src/fe-gtk/dcc-transfer-list.h"
#include "../../src/fe-gtk/addon-list.h"
#include "../../src/fe-gtk/channel-model.h"
#include "../../src/fe-gtk/channel-tree-view.h"
#include "../../src/fe-gtk/notify-list.h"
#include "../../src/fe-gtk/user-list-model.h"
#include "../../src/fe-gtk/user-list-view.h"
#include "../../src/fe-gtk/url-list.h"

#if GTK_MAJOR_VERSION != 4
#error The GTK4 probe must compile against GTK 4 headers.
#endif

#if GLIB_SIZEOF_VOID_P != 8
#error The initial GTK4 probe requires a 64-bit GLib build.
#endif

static void
check_compatibility_helper_signatures (void)
{
	void (*volatile box_append) (GtkBox *, GtkWidget *, gboolean, gboolean, guint) =
		fabulor_gtk_box_append;
	void (*volatile horizontal_box_append_trailing) (GtkBox *, GtkWidget *) =
		fabulor_gtk_horizontal_box_append_trailing;
	void (*volatile box_insert_before_trailing) (GtkBox *, GtkWidget *, GtkWidget *) =
		fabulor_gtk_box_insert_before_trailing;
	void (*volatile box_append_trailing_pair) (GtkBox *, GtkWidget *, GtkWidget *) =
		fabulor_gtk_box_append_trailing_pair;
	void (*volatile box_remove_child) (GtkBox *, GtkWidget *) =
		fabulor_gtk_box_remove_child;
	void (*volatile copy_text_to_clipboards) (GtkWidget *, const gchar *) =
		fabulor_gtk_copy_text_to_clipboards;
	void (*volatile widget_on_pointer_enter) (GtkWidget *, FabulorGtkWidgetInteractionFunc, gpointer) =
		fabulor_gtk_widget_on_pointer_enter;
	void (*volatile widget_on_pointer_motion) (GtkWidget *, FabulorGtkPointerMotionFunc,
		FabulorGtkPointerLeaveFunc, gpointer) = fabulor_gtk_widget_on_pointer_motion;
	void (*volatile widget_set_prelight) (GtkWidget *, gboolean) =
		fabulor_gtk_widget_set_prelight;
	void (*volatile widget_suppress_pointer_prelight) (GtkWidget *) =
		fabulor_gtk_widget_suppress_pointer_prelight;
	gboolean (*volatile widget_get_descendant_origin) (GtkWidget *, GtkWidget *,
		gdouble *, gdouble *) = fabulor_gtk_widget_get_descendant_origin;
	gboolean (*volatile widget_contains_descendant_point) (GtkWidget *, GtkWidget *,
		gdouble, gdouble) = fabulor_gtk_widget_contains_descendant_point;
	void (*volatile widget_set_pointing_cursor) (GtkWidget *, gboolean) =
		fabulor_gtk_widget_set_pointing_cursor;
	void (*volatile text_view_set_pointing_cursor) (GtkTextView *, gboolean) =
		fabulor_gtk_text_view_set_pointing_cursor;
	void (*volatile widget_on_click_released) (GtkWidget *, FabulorGtkClickFunc,
		gpointer) = fabulor_gtk_widget_on_click_released;
	void (*volatile widget_on_multi_click) (GtkWidget *, FabulorGtkMultiClickFunc,
		gpointer) = fabulor_gtk_widget_on_multi_click;
	void (*volatile widget_on_file_drop) (GtkWidget *, GdkDragAction,
		FabulorGtkFileDropFunc, gpointer) = fabulor_gtk_widget_on_file_drop;
	void (*volatile widget_on_file_drop_full) (GtkWidget *, GdkDragAction,
		FabulorGtkFileDropFunc, FabulorGtkFileDropMotionFunc,
		FabulorGtkFileDropLeaveFunc, gpointer) =
		fabulor_gtk_widget_on_file_drop_full;
	void (*volatile widget_enable_internal_drag_source) (GtkWidget *,
		FabulorGtkInternalDragKind, FabulorGtkInternalDragIconFunc, gpointer) =
		fabulor_gtk_widget_enable_internal_drag_source;
	void (*volatile widget_enable_internal_drop_target) (GtkWidget *, guint,
		FabulorGtkInternalDragMotionFunc, FabulorGtkInternalDragLeaveFunc,
		FabulorGtkInternalDragDropFunc, gpointer) =
		fabulor_gtk_widget_enable_internal_drop_target;
	void (*volatile widget_on_key_pressed) (GtkWidget *, FabulorGtkKeyFunc, gpointer) =
		fabulor_gtk_widget_on_key_pressed;
	void (*volatile widget_on_focus_enter) (GtkWidget *, FabulorGtkWidgetInteractionFunc, gpointer) =
		fabulor_gtk_widget_on_focus_enter;
	void (*volatile widget_on_focus_leave) (GtkWidget *, FabulorGtkWidgetInteractionFunc, gpointer) =
		fabulor_gtk_widget_on_focus_leave;
	void (*volatile widget_on_scroll) (GtkWidget *, FabulorGtkScrollFunc, gpointer) =
		fabulor_gtk_widget_on_scroll;
	void (*volatile window_set_child) (GtkWindow *, GtkWidget *) =
		fabulor_gtk_window_set_child;
	void (*volatile scrolled_window_set_child) (GtkScrolledWindow *, GtkWidget *) =
		fabulor_gtk_scrolled_window_set_child;
	void (*volatile frame_set_child) (GtkFrame *, GtkWidget *) =
		fabulor_gtk_frame_set_child;
	void (*volatile button_set_child) (GtkButton *, GtkWidget *) =
		fabulor_gtk_button_set_child;
	void (*volatile overlay_set_child) (GtkOverlay *, GtkWidget *) =
		fabulor_gtk_overlay_set_child;
	void (*volatile popover_set_child) (GtkPopover *, GtkWidget *) =
		fabulor_gtk_popover_set_child;
	void (*volatile widget_reveal_tree) (GtkWidget *) =
		fabulor_gtk_widget_reveal_tree;
	void (*volatile window_destroy) (GtkWindow *) = fabulor_gtk_window_destroy;
	void (*volatile dialog_destroy_on_response) (GtkDialog *, gint, gpointer) =
		fabulor_gtk_dialog_destroy_on_response;

	(void) box_append;
	(void) horizontal_box_append_trailing;
	(void) box_insert_before_trailing;
	(void) box_append_trailing_pair;
	(void) box_remove_child;
	(void) copy_text_to_clipboards;
	(void) widget_on_pointer_enter;
	(void) widget_on_pointer_motion;
	(void) widget_set_prelight;
	(void) widget_suppress_pointer_prelight;
	(void) widget_get_descendant_origin;
	(void) widget_contains_descendant_point;
	(void) widget_set_pointing_cursor;
	(void) text_view_set_pointing_cursor;
	(void) widget_on_click_released;
	(void) widget_on_multi_click;
	(void) widget_on_file_drop;
	(void) widget_on_file_drop_full;
	(void) widget_enable_internal_drag_source;
	(void) widget_enable_internal_drop_target;
	(void) widget_on_key_pressed;
	(void) widget_on_focus_enter;
	(void) widget_on_focus_leave;
	(void) widget_on_scroll;
	(void) window_set_child;
	(void) scrolled_window_set_child;
	(void) frame_set_child;
	(void) button_set_child;
	(void) overlay_set_child;
	(void) popover_set_child;
	(void) widget_reveal_tree;
	(void) window_destroy;
	(void) dialog_destroy_on_response;
}

static void
check_user_list_view_signatures (void)
{
	GtkWidget *(*volatile create_view) (gboolean, gboolean, gint *, gint *) =
		fabulor_user_list_view_new;
	void (*volatile set_model) (GtkWidget *, FabulorUserListModel *) =
		fabulor_user_list_view_set_model;
	GPtrArray *(*volatile selected_users) (GtkWidget *) =
		fabulor_user_list_view_dup_selected_users;
	gboolean (*volatile select_user) (GtkWidget *, gpointer, gboolean,
		gboolean, gboolean) = fabulor_user_list_view_select_user;
	gpointer (*volatile user_at_position) (GtkWidget *, gdouble, gdouble) =
		fabulor_user_list_view_get_user_at_position;

	(void) create_view;
	(void) set_model;
	(void) selected_users;
	(void) select_user;
	(void) user_at_position;
}

static gboolean
check_internal_drag_payload (void)
{
	GdkContentProvider *provider = gdk_content_provider_new_typed (
		G_TYPE_POINTER,
		GUINT_TO_POINTER (FABULOR_GTK_INTERNAL_DRAG_CHANNEL_VIEW));
	GdkContentFormats *formats = gdk_content_provider_ref_formats (provider);
	GValue value = G_VALUE_INIT;
	gboolean valid;

	g_value_init (&value, G_TYPE_POINTER);
	g_value_set_pointer (&value,
		GUINT_TO_POINTER (FABULOR_GTK_INTERNAL_DRAG_CHANNEL_VIEW));
	valid = gdk_content_formats_contain_gtype (formats, G_TYPE_POINTER) &&
		fabulor_gtk_internal_drag_kind_from_value (&value) ==
			FABULOR_GTK_INTERNAL_DRAG_CHANNEL_VIEW;

	g_value_unset (&value);
	gdk_content_formats_unref (formats);
	g_object_unref (provider);
	return valid;
}

static gint
compare_file_basename (gconstpointer left, gconstpointer right,
					   gpointer user_data)
{
	gchar *left_name = g_file_get_basename (G_FILE ((gpointer) left));
	gchar *right_name = g_file_get_basename (G_FILE ((gpointer) right));
	gint result = g_strcmp0 (left_name, right_name);

	(void) user_data;
	g_free (left_name);
	g_free (right_name);
	return result < 0 ? -1 : result > 0 ? 1 : 0;
}

static gboolean
check_flat_model_stack (void)
{
	GtkSorter *sorter = GTK_SORTER (gtk_custom_sorter_new (
		compare_file_basename, NULL, NULL));
	FabulorGtk4FlatModelStack *stack =
		fabulor_gtk4_flat_model_stack_new (G_TYPE_FILE, sorter,
			FABULOR_GTK4_SELECTION_MULTIPLE);
	GtkSortListModel *sorted = NULL;
	GtkSelectionModel *selection = NULL;
	GFile *file_zero = g_file_new_for_uri ("file:///0");
	GFile *file_a = g_file_new_for_uri ("file:///a");
	GFile *file_b = g_file_new_for_uri ("file:///b");
	gpointer item;
	gboolean valid = TRUE;

	g_object_unref (sorter);
	if (!stack)
		valid = FALSE;
	if (valid)
	{
		sorted = fabulor_gtk4_flat_model_stack_get_sorted (stack);
		selection = fabulor_gtk4_flat_model_stack_get_selection (stack);
		fabulor_gtk4_flat_model_stack_append (stack, file_b);
		fabulor_gtk4_flat_model_stack_append (stack, file_a);
		item = g_list_model_get_item (G_LIST_MODEL (sorted), 0);
		valid = g_list_model_get_n_items (G_LIST_MODEL (sorted)) == 2 &&
			item == file_a;
		g_clear_object (&item);
	}
	if (valid)
	{
		valid = gtk_selection_model_select_item (selection, 0, TRUE);
		fabulor_gtk4_flat_model_stack_append (stack, file_zero);
		item = g_list_model_get_item (G_LIST_MODEL (sorted), 1);
		valid = valid && item == file_a &&
			gtk_selection_model_is_selected (selection, 1);
		g_clear_object (&item);
	}
	if (valid)
	{
		valid = fabulor_gtk4_flat_model_stack_remove (stack, file_b) &&
			g_list_model_get_n_items (G_LIST_MODEL (sorted)) == 2;
		fabulor_gtk4_flat_model_stack_clear (stack);
		valid = valid && g_list_model_get_n_items (G_LIST_MODEL (sorted)) == 0;
	}

	fabulor_gtk4_flat_model_stack_free (stack);
	g_object_unref (file_zero);
	g_object_unref (file_a);
	g_object_unref (file_b);
	return valid;
}

typedef struct
{
	GObject *parent;
	GListStore *children;
} ProbeTreeData;

static GListModel *
create_probe_child_model (gpointer item, gpointer user_data)
{
	ProbeTreeData *data = user_data;

	if (item == data->parent)
		return G_LIST_MODEL (g_object_ref (data->children));
	return NULL;
}

static gboolean
check_tree_model_stack (void)
{
	GObject *parent = g_object_new (G_TYPE_OBJECT, NULL);
	GObject *child = g_object_new (G_TYPE_OBJECT, NULL);
	GObject *leaf = g_object_new (G_TYPE_OBJECT, NULL);
	GListStore *children = g_list_store_new (G_TYPE_OBJECT);
	ProbeTreeData data = { parent, children };
	FabulorGtk4TreeModelStack *stack;
	GtkTreeListModel *tree;
	GtkSingleSelection *selection;
	GtkTreeListRow *parent_row;
	GtkTreeListRow *child_row;
	gboolean valid;

	g_list_store_append (children, child);
	stack = fabulor_gtk4_tree_model_stack_new (G_TYPE_OBJECT, FALSE,
		create_probe_child_model, &data, NULL);
	if (!stack)
	{
		g_object_unref (children);
		g_object_unref (parent);
		g_object_unref (child);
		g_object_unref (leaf);
		return FALSE;
	}

	tree = fabulor_gtk4_tree_model_stack_get_tree (stack);
	selection = fabulor_gtk4_tree_model_stack_get_selection (stack);
	fabulor_gtk4_tree_model_stack_append_root (stack, parent);
	fabulor_gtk4_tree_model_stack_append_root (stack, leaf);
	parent_row = gtk_tree_list_model_get_row (tree, 0);
	valid = g_list_model_get_n_items (G_LIST_MODEL (tree)) == 2 &&
		parent_row != NULL && gtk_tree_list_row_get_item (parent_row) == parent;
	if (valid)
	{
		gtk_tree_list_row_set_expanded (parent_row, TRUE);
		child_row = gtk_tree_list_model_get_row (tree, 1);
		valid = g_list_model_get_n_items (G_LIST_MODEL (tree)) == 3 &&
			child_row != NULL && gtk_tree_list_row_get_depth (child_row) == 1 &&
			gtk_tree_list_row_get_item (child_row) == child;
		g_clear_object (&child_row);
	}
	if (valid)
	{
		gtk_single_selection_set_selected (selection, 1);
		valid = gtk_single_selection_get_selected_item (selection) != NULL &&
			fabulor_gtk4_tree_model_stack_remove_root (stack, leaf) &&
			g_list_model_get_n_items (G_LIST_MODEL (tree)) == 2 &&
			gtk_single_selection_get_selected (selection) == 1;
	}
	fabulor_gtk4_tree_model_stack_clear (stack);
	valid = valid && g_list_model_get_n_items (G_LIST_MODEL (tree)) == 0;

	g_clear_object (&parent_row);
	fabulor_gtk4_tree_model_stack_free (stack);
	g_object_unref (children);
	g_object_unref (parent);
	g_object_unref (child);
	g_object_unref (leaf);
	return valid;
}

static gboolean
check_channel_model (void)
{
	gint server;
	gint alpha;
	gint beta;
	gint utility;
	FabulorChannelModel *model = fabulor_channel_model_new ();
	FabulorChannelModelRow server_row = {
		&server, "Server", NULL, NULL, PANGO_UNDERLINE_NONE
	};
	FabulorChannelModelRow alpha_row = {
		&alpha, "#alpha", NULL, NULL, PANGO_UNDERLINE_NONE
	};
	FabulorChannelModelRow beta_row = {
		&beta, "#beta", NULL, NULL, PANGO_UNDERLINE_NONE
	};
	FabulorChannelModelRow utility_row = {
		&utility, "Utility", NULL, NULL, PANGO_UNDERLINE_NONE
	};
	gboolean valid = model != NULL;

	if (valid)
	{
		valid = fabulor_channel_model_insert (model, &server_row, NULL, 0) &&
			fabulor_channel_model_insert (model, &beta_row, &server, 0) &&
			fabulor_channel_model_insert (model, &alpha_row, &server, 0) &&
			fabulor_channel_model_insert (model, &utility_row, NULL, 1) &&
			!fabulor_channel_model_insert (model, &utility_row, NULL, 0) &&
			fabulor_channel_model_get_root_count (model) == 2 &&
			fabulor_channel_model_get_child_count (model, &server) == 2 &&
			fabulor_channel_model_get_flat_count (model) == 4 &&
			fabulor_channel_model_get_flat_at (model, 1) == &alpha &&
			fabulor_channel_model_get_parent (model, &beta) == &server;
	}
	if (valid)
	{
		alpha_row.name = "#alpha-renamed";
		valid = fabulor_channel_model_select_identity (model, &alpha) &&
			fabulor_channel_model_get_selected_identity (model) == &alpha &&
			fabulor_channel_model_update (model, &alpha_row) &&
			g_strcmp0 (fabulor_channel_model_get_name (model, &alpha),
				"#alpha-renamed") == 0 &&
			fabulor_channel_model_move_cyclic (model, &alpha, -1) &&
			fabulor_channel_model_get_child_at (model, &server, 1) == &alpha &&
			fabulor_channel_model_get_selected_identity (model) == &alpha;
	}
	if (valid)
	{
		valid = fabulor_channel_model_reparent (model, &beta, NULL, 1) &&
			fabulor_channel_model_get_parent (model, &beta) == NULL &&
			fabulor_channel_model_get_root_count (model) == 3 &&
			fabulor_channel_model_remove (model, &alpha) &&
			fabulor_channel_model_remove (model, &server) &&
			!fabulor_channel_model_remove (model, &server) &&
			fabulor_channel_model_get_flat_count (model) == 2;
	}
	if (valid)
	{
		GtkTreeListModel *tree = fabulor_channel_model_get_tree (model);
		GtkSingleSelection *selection = fabulor_channel_model_get_selection (
			model);
		valid = GTK_IS_TREE_LIST_MODEL (tree) &&
			GTK_IS_SINGLE_SELECTION (selection);
	}
	fabulor_channel_model_free (model);
	return valid;
}

static void
check_channel_tree_view_signatures (void)
{
	GtkWidget *(*new_view) (FabulorChannelModel *, gboolean, gboolean,
		gboolean, gboolean) = fabulor_channel_tree_view_new;
	gpointer (*identity_at_position) (GtkWidget *, gdouble, gdouble) =
		fabulor_channel_tree_view_get_identity_at_position;
	gboolean (*focus_identity) (GtkWidget *, gpointer) =
		fabulor_channel_tree_view_focus_identity;
	gboolean (*expand_parent) (GtkWidget *, gpointer) =
		fabulor_channel_tree_view_expand_parent;
	gboolean (*is_expanded) (GtkWidget *, gpointer) =
		fabulor_channel_tree_view_is_expanded;

	(void) new_view;
	(void) identity_at_position;
	(void) focus_identity;
	(void) expand_parent;
	(void) is_expanded;
}

typedef struct
{
	gpointer identity;
	guint changes;
} ProbeChannelSelection;

static void
probe_channel_selection_changed (GtkWidget *view, gpointer identity,
	gpointer user_data)
{
	ProbeChannelSelection *selection = user_data;

	(void) view;
	selection->identity = identity;
	selection->changes++;
}

static gboolean
check_channel_tree_view (void)
{
	gint server;
	gint alpha;
	gint beta;
	FabulorChannelModel *model = fabulor_channel_model_new ();
	FabulorChannelModelRow server_row = {
		&server, "Server", NULL, NULL, PANGO_UNDERLINE_NONE
	};
	FabulorChannelModelRow alpha_row = {
		&alpha, "#alpha", NULL, NULL, PANGO_UNDERLINE_NONE
	};
	FabulorChannelModelRow beta_row = {
		&beta, "#beta", NULL, NULL, PANGO_UNDERLINE_NONE
	};
	ProbeChannelSelection selection = { NULL, 0 };
	GtkWidget *view;
	gboolean valid = model != NULL;

	if (valid)
		valid = fabulor_channel_model_insert (model, &server_row, NULL, 0) &&
			fabulor_channel_model_insert (model, &alpha_row, &server, 0) &&
			fabulor_channel_model_insert (model, &beta_row, &server, 1);
	view = valid ? fabulor_channel_tree_view_new (model, TRUE, FALSE, TRUE,
		FALSE) : NULL;
	if (valid)
	{
		valid = GTK_IS_LIST_VIEW (view);
		g_object_ref_sink (view);
		fabulor_channel_tree_view_set_selection_callback (view,
			probe_channel_selection_changed, &selection);
		fabulor_channel_tree_view_expand_all (view);
		valid = valid && fabulor_channel_tree_view_is_expanded (view, &server) &&
			fabulor_channel_tree_view_focus_identity (view, &alpha) &&
			fabulor_channel_model_get_selected_identity (model) == &alpha &&
			selection.identity == &alpha && selection.changes == 1;
		g_object_unref (view);
		valid = valid && fabulor_channel_model_select_identity (model, &beta);
	}
	fabulor_channel_model_free (model);
	return valid;
}

static gboolean
check_notify_list_model (void)
{
	gint owner_alpha;
	gint owner_beta;
	gint server_alpha;
	FabulorNotifyList *list = fabulor_notify_list_new (NULL, NULL);
	FabulorNotifyListRow alpha = {
		&owner_alpha, NULL, "Alpha", "Alpha", "Offline", "", "Never", NULL
	};
	FabulorNotifyListRow beta = {
		&owner_beta, NULL, "Beta", "Beta", "Offline", "", "Never", NULL
	};
	gchar *selected_name = NULL;
	gboolean valid = list != NULL;

	if (valid)
	{
		fabulor_notify_list_begin_update (list);
		valid = fabulor_notify_list_append (list, &alpha) &&
			fabulor_notify_list_append (list, &beta) &&
			!fabulor_notify_list_append (list, &alpha);
		fabulor_notify_list_end_update (list);
		valid = valid && fabulor_notify_list_get_n_rows (list) == 2 &&
			fabulor_notify_list_select_identity (list, &owner_alpha, NULL);
	}
	if (valid)
	{
		selected_name = fabulor_notify_list_dup_selected_name (list);
		valid = g_strcmp0 (selected_name, "Alpha") == 0 &&
			fabulor_notify_list_get_selected_server_data (list) == NULL;
		g_clear_pointer (&selected_name, g_free);
	}
	if (valid)
	{
		alpha.server_data = &server_alpha;
		alpha.status = "Online";
		alpha.network = "ExampleNet";
		alpha.last_seen = "0 minutes ago";
		fabulor_notify_list_begin_update (list);
		valid = fabulor_notify_list_append (list, &beta) &&
			fabulor_notify_list_append (list, &alpha);
		fabulor_notify_list_end_update (list);
		selected_name = fabulor_notify_list_dup_selected_name (list);
		valid = valid && g_strcmp0 (selected_name, "Alpha") == 0 &&
			fabulor_notify_list_get_selected_server_data (list) == &server_alpha;
		g_clear_pointer (&selected_name, g_free);
	}
	if (valid)
	{
		fabulor_notify_list_begin_update (list);
		fabulor_notify_list_end_update (list);
		valid = fabulor_notify_list_get_n_rows (list) == 0 &&
			!fabulor_notify_list_has_selection (list);
	}

	fabulor_notify_list_free (list);
	return valid;
}

typedef struct
{
	gint rank;
} ProbeUser;

static gint
compare_probe_users (gconstpointer left, gconstpointer right,
					 gpointer user_data)
{
	const ProbeUser *left_user = left;
	const ProbeUser *right_user = right;

	(void) user_data;
	return left_user->rank < right_user->rank ? -1 :
		left_user->rank > right_user->rank ? 1 : 0;
}

static gboolean
check_user_list_model (void)
{
	ProbeUser first = { 1 };
	ProbeUser second = { 2 };
	ProbeUser missing = { 3 };
	FabulorUserListRow first_row = {
		&first, NULL, "@", "First", "first.example", NULL
	};
	FabulorUserListRow second_row = {
		&second, NULL, "+", "Second", "second.example", NULL
	};
	FabulorUserListRow missing_row = {
		&missing, NULL, "", "Missing", "missing.example", NULL
	};
	FabulorUserListModel *model = fabulor_user_list_model_new (
		compare_probe_users, NULL, FALSE);
	gboolean valid = model != NULL;

	if (valid)
	{
		valid = fabulor_user_list_model_insert (model, &second_row) &&
			fabulor_user_list_model_insert (model, &first_row) &&
			!fabulor_user_list_model_insert (model, &first_row) &&
			GTK_IS_MULTI_SELECTION (
				fabulor_user_list_model_get_selection (model)) &&
			g_list_model_get_n_items (
				fabulor_user_list_model_get_list_model (model)) == 2 &&
			fabulor_user_list_model_get_n_rows (model) == 2 &&
			fabulor_user_list_model_get_user_at (model, 0) == &first;
	}
	if (valid)
	{
		second.rank = 0;
		second_row.nick_markup = "Second updated";
		valid = fabulor_user_list_model_update (model, &second_row, TRUE) &&
			!fabulor_user_list_model_update (model, &missing_row, TRUE) &&
			fabulor_user_list_model_get_user_at (model, 0) == &second &&
			fabulor_user_list_model_remove (model, &first) &&
			!fabulor_user_list_model_remove (model, &first) &&
			fabulor_user_list_model_get_n_rows (model) == 1;
	}
	if (valid)
	{
		fabulor_user_list_model_clear (model);
		valid = fabulor_user_list_model_get_n_rows (model) == 0;
	}

	fabulor_user_list_model_free (model);
	if (valid)
	{
		model = fabulor_user_list_model_new (compare_probe_users, NULL, TRUE);
		valid = model != NULL &&
			fabulor_user_list_model_insert (model, &second_row) &&
			fabulor_user_list_model_insert (model, &first_row) &&
			fabulor_user_list_model_get_user_at (model, 0) == &first;
		fabulor_user_list_model_free (model);
	}
	if (valid)
	{
		model = fabulor_user_list_model_new (NULL, NULL, FALSE);
		valid = model != NULL &&
			fabulor_user_list_model_insert (model, &first_row) &&
			fabulor_user_list_model_insert (model, &second_row) &&
			fabulor_user_list_model_get_user_at (model, 0) == &second;
		fabulor_user_list_model_free (model);
	}
	return valid;
}

static gboolean
check_addon_list_model (void)
{
	FabulorAddonList *list = fabulor_addon_list_new ();
	gboolean valid = list != NULL;

	if (valid)
	{
		fabulor_addon_list_append (list, "Alpha", "1.0", "alpha.dll",
			"Alpha addon", "C:/addons/alpha.dll");
		fabulor_addon_list_append (list, "Beta", "2.0", "beta.py",
			"Beta addon", "C:/addons/beta.py");
		valid = fabulor_addon_list_get_n_rows (list) == 2;
	}
	if (valid)
	{
		fabulor_addon_list_clear (list);
		valid = fabulor_addon_list_get_n_rows (list) == 0;
	}

	fabulor_addon_list_free (list);
	return valid;
}

static gboolean
check_url_list_model (void)
{
	FabulorUrlList *list = fabulor_url_list_new ();
	gchar *first = NULL;
	gchar *second = NULL;
	gchar *third = NULL;
	gboolean valid = list != NULL;

	if (valid)
	{
		fabulor_url_list_prepend (list, "https://one.example/", 2);
		fabulor_url_list_prepend (list, "https://two.example/", 2);
		fabulor_url_list_prepend (list, "https://three.example/", 2);
		first = fabulor_url_list_dup_at (list, 0);
		second = fabulor_url_list_dup_at (list, 1);
		third = fabulor_url_list_dup_at (list, 2);
		valid = fabulor_url_list_get_n_rows (list) == 2 &&
			g_strcmp0 (first, "https://three.example/") == 0 &&
			g_strcmp0 (second, "https://two.example/") == 0 &&
			third == NULL;
		g_clear_pointer (&first, g_free);
		g_clear_pointer (&second, g_free);
		g_clear_pointer (&third, g_free);
	}
	if (valid)
	{
		fabulor_url_list_clear (list);
		valid = fabulor_url_list_get_n_rows (list) == 0;
	}

	fabulor_url_list_free (list);
	return valid;
}

typedef struct
{
	gchar *old_mask;
	gchar *new_mask;
	gchar *flags_mask;
	guint rename_flags;
	guint changed_flags;
	guint rename_count;
	guint flags_count;
} ProbeIgnoreCallbacks;

static gboolean
probe_ignore_rename (const gchar *old_mask, const gchar *new_mask, guint flags,
	gpointer user_data)
{
	ProbeIgnoreCallbacks *callbacks = user_data;

	callbacks->rename_count++;
	callbacks->rename_flags = flags;
	g_free (callbacks->old_mask);
	g_free (callbacks->new_mask);
	callbacks->old_mask = g_strdup (old_mask);
	callbacks->new_mask = g_strdup (new_mask);
	return g_strcmp0 (new_mask, "reject!*@*") != 0;
}

static void
probe_ignore_flags (const gchar *mask, guint flags, gpointer user_data)
{
	ProbeIgnoreCallbacks *callbacks = user_data;

	callbacks->flags_count++;
	callbacks->changed_flags = flags;
	g_free (callbacks->flags_mask);
	callbacks->flags_mask = g_strdup (mask);
}

static gboolean
check_ignore_list_model (void)
{
	ProbeIgnoreCallbacks callbacks = { 0 };
	FabulorIgnoreList *list = fabulor_ignore_list_new (probe_ignore_rename,
		probe_ignore_flags, &callbacks);
	GPtrArray *masks = NULL;
	gchar *first = NULL;
	gboolean valid = list != NULL;

	if (valid)
	{
		fabulor_ignore_list_append (list, "alpha!*@*", 1u | 4u | 64u, FALSE);
		fabulor_ignore_list_append (list, "beta!*@*", 2u, FALSE);
		first = fabulor_ignore_list_dup_mask_at (list, 0);
		valid = fabulor_ignore_list_get_n_rows (list) == 2 &&
			g_strcmp0 (first, "alpha!*@*") == 0 &&
			fabulor_ignore_list_get_flags_at (list, 0) == (1u | 4u | 64u);
		g_clear_pointer (&first, g_free);
	}
	if (valid)
	{
		valid = fabulor_ignore_list_rename_at (list, 0, "gamma!*@*") &&
			callbacks.rename_count == 1 &&
			callbacks.rename_flags == (1u | 4u | 64u) &&
			g_strcmp0 (callbacks.old_mask, "alpha!*@*") == 0 &&
			g_strcmp0 (callbacks.new_mask, "gamma!*@*") == 0 &&
			!fabulor_ignore_list_rename_at (list, 0, "reject!*@*") &&
			callbacks.rename_count == 2;
	}
	if (valid)
	{
		first = fabulor_ignore_list_dup_mask_at (list, 0);
		valid = g_strcmp0 (first, "gamma!*@*") == 0 &&
			fabulor_ignore_list_set_flag_at (list, 0, 128u, TRUE) &&
			callbacks.flags_count == 1 &&
			callbacks.changed_flags == (1u | 4u | 64u | 128u) &&
			g_strcmp0 (callbacks.flags_mask, "gamma!*@*") == 0;
		g_clear_pointer (&first, g_free);
	}
	if (valid)
	{
		masks = fabulor_ignore_list_dup_masks (list);
		valid = masks->len == 2 &&
			g_strcmp0 (g_ptr_array_index (masks, 0), "gamma!*@*") == 0 &&
			g_strcmp0 (g_ptr_array_index (masks, 1), "beta!*@*") == 0;
		g_ptr_array_unref (masks);
		fabulor_ignore_list_clear (list);
		valid = valid && fabulor_ignore_list_get_n_rows (list) == 0;
	}

	fabulor_ignore_list_free (list);
	g_free (callbacks.old_mask);
	g_free (callbacks.new_mask);
	g_free (callbacks.flags_mask);
	return valid;
}

typedef struct
{
	guint selected;
	guint calls;
} ProbeBanSelection;

static void
probe_ban_selection (guint selected, gpointer user_data)
{
	ProbeBanSelection *selection = user_data;
	selection->selected = selected;
	selection->calls++;
}

static gboolean
check_channel_list_model (void)
{
	gint beta_identity = 1;
	gint alpha_identity = 2;
	FabulorChannelListSnapshot beta = {
		&beta_identity, "#beta", 20, "Beta topic", "beta"
	};
	FabulorChannelListSnapshot alpha = {
		&alpha_identity, "#alpha", 10, "Alpha topic", "alpha"
	};
	FabulorChannelList *list = fabulor_channel_list_new (NULL, NULL);
	GPtrArray *rows = NULL;
	GPtrArray *selected = NULL;
	gchar *first = NULL;
	gboolean valid = list != NULL;

	if (valid)
	{
		valid = fabulor_channel_list_append (list, &beta) &&
			fabulor_channel_list_append (list, &alpha) &&
			!fabulor_channel_list_append (list, &beta) &&
			fabulor_channel_list_get_n_rows (list) == 2;
	}
	if (valid)
	{
		rows = fabulor_channel_list_dup_all (list);
		valid = rows->len == 2 &&
			strcmp (((FabulorChannelListRecord *) g_ptr_array_index (
				rows, 0))->channel, "#alpha") == 0 &&
			((FabulorChannelListRecord *) g_ptr_array_index (rows, 0))->users == 10 &&
			strcmp (((FabulorChannelListRecord *) g_ptr_array_index (
				rows, 1))->topic, "Beta topic") == 0;
		g_ptr_array_unref (rows);
	}
	if (valid)
	{
		valid = fabulor_channel_list_set_selected (list, 0, TRUE) &&
			fabulor_channel_list_set_selected (list, 1, TRUE);
		selected = fabulor_channel_list_dup_selected_text (list,
			FABULOR_CHANNEL_LIST_TOPIC);
		first = fabulor_channel_list_dup_first_selected_channel (list);
		valid = valid && selected->len == 2 &&
			strcmp (g_ptr_array_index (selected, 0), "Alpha topic") == 0 &&
			strcmp (first, "#alpha") == 0;
		g_ptr_array_unref (selected);
		g_free (first);
	}
	if (valid)
	{
		fabulor_channel_list_clear (list);
		valid = fabulor_channel_list_get_n_rows (list) == 0;
	}
	fabulor_channel_list_free (list);
	return valid;
}

static gboolean
check_ban_list_model (void)
{
	ProbeBanSelection selection = { 0 };
	FabulorBanList *list = fabulor_ban_list_new (probe_ban_selection,
		&selection);
	GPtrArray *selected_bans = NULL;
	GPtrArray *cropped_quiets = NULL;
	gboolean valid = list != NULL;
	guint stage = 0;

	if (valid)
	{
		stage = 1;
		fabulor_ban_list_append (list, 0, "Ban", "alpha!*@*", "setter",
			"Sat Mar 16 21:24:27 2013");
		fabulor_ban_list_append (list, 0, "Ban", "beta!*@*", "setter",
			"Sun Mar 17 21:24:27 2013");
		fabulor_ban_list_append (list, 3, "Quiet", "quiet!*@*", "setter",
			"Mon Mar 18 21:24:27 2013");
		valid = fabulor_ban_list_get_n_rows (list) == 3 &&
			fabulor_ban_list_set_selected (list, 0, TRUE) &&
			fabulor_ban_list_set_selected (list, 2, TRUE) &&
			fabulor_ban_list_get_n_selected (list) == 2 &&
			selection.selected == 2 && selection.calls >= 2;
	}
	if (valid)
	{
		stage = 2;
		selected_bans = fabulor_ban_list_dup_masks (list, 0, TRUE);
		cropped_quiets = fabulor_ban_list_dup_masks (list, 3, FALSE);
		valid = selected_bans->len == 1 &&
			g_strcmp0 (g_ptr_array_index (selected_bans, 0), "alpha!*@*") == 0 &&
			cropped_quiets->len == 0;
		g_ptr_array_unref (selected_bans);
		g_ptr_array_unref (cropped_quiets);
	}
	if (valid)
	{
		stage = 3;
		fabulor_ban_list_invert_selection (list);
		valid = fabulor_ban_list_get_n_selected (list) == 1;
		fabulor_ban_list_select_all (list);
		valid = valid && fabulor_ban_list_get_n_selected (list) == 3;
		fabulor_ban_list_clear (list);
		valid = valid && fabulor_ban_list_get_n_rows (list) == 0 &&
			fabulor_ban_list_get_n_selected (list) == 0;
	}
	if (!valid)
		fprintf (stderr, "Ban list probe failed at stage %u: rows=%u selected=%u callback=%u calls=%u\n",
			stage, list ? fabulor_ban_list_get_n_rows (list) : 0,
			list ? fabulor_ban_list_get_n_selected (list) : 0,
			selection.selected, selection.calls);
	fabulor_ban_list_free (list);
	return valid;
}

typedef struct
{
	guint calls;
} ProbeDccSelection;

static void
probe_dcc_selection (gpointer user_data)
{
	ProbeDccSelection *selection = user_data;
	selection->calls++;
}

static gboolean
check_dcc_transfer_list_model (void)
{
	ProbeDccSelection selection = { 0 };
	gint first_identity = 1;
	gint second_identity = 2;
	FabulorDccTransferSnapshot first = { 0 };
	FabulorDccTransferSnapshot second = { 0 };
	FabulorDccTransferList *list = fabulor_dcc_transfer_list_new (
		probe_dcc_selection, NULL, &selection);
	GPtrArray *rows = NULL;
	gboolean valid = list != NULL;

	first.identity = &first_identity;
	first.status = "Queued";
	first.file = "first.bin";
	first.size = "1.0 kB";
	first.position = "0 bytes";
	first.percentage = "0%";
	first.speed = "0.0";
	first.eta = "--:--:--";
	first.nick = "alpha";
	second = first;
	second.identity = &second_identity;
	second.upload = TRUE;
	second.file = "second.bin";
	second.nick = "beta";
	if (valid)
	{
		valid = fabulor_dcc_transfer_list_append (list, &first, FALSE) &&
			fabulor_dcc_transfer_list_append (list, &second, TRUE) &&
			!fabulor_dcc_transfer_list_append (list, &first, FALSE) &&
			fabulor_dcc_transfer_list_get_n_rows (list) == 2;
	}
	if (valid)
	{
		rows = fabulor_dcc_transfer_list_dup_all (list);
		valid = rows->len == 2 &&
			g_ptr_array_index (rows, 0) == &second_identity &&
			g_ptr_array_index (rows, 1) == &first_identity;
		g_ptr_array_unref (rows);
	}
	if (valid)
	{
		first.status = "Active";
		first.position = "512 bytes";
		first.percentage = "50%";
		valid = fabulor_dcc_transfer_list_update (list, &first) &&
			fabulor_dcc_transfer_list_set_selected (list, 1, TRUE) &&
			fabulor_dcc_transfer_list_get_n_selected (list) == 1 &&
			fabulor_dcc_transfer_list_get_first_selected (list) ==
				&first_identity && selection.calls >= 1;
	}
	if (valid)
	{
		rows = fabulor_dcc_transfer_list_dup_selected (list);
		valid = rows->len == 1 &&
			g_ptr_array_index (rows, 0) == &first_identity &&
			fabulor_dcc_transfer_list_remove (list, &first_identity) &&
			!fabulor_dcc_transfer_list_remove (list, &first_identity) &&
			fabulor_dcc_transfer_list_get_n_rows (list) == 1;
		g_ptr_array_unref (rows);
		fabulor_dcc_transfer_list_clear (list);
		valid = valid && fabulor_dcc_transfer_list_get_n_rows (list) == 0;
	}
	fabulor_dcc_transfer_list_free (list);
	return valid;
}

typedef struct
{
	guint selected;
	guint calls;
} ProbeDccChatSelection;

static void
probe_dcc_chat_selection (guint selected, gpointer user_data)
{
	ProbeDccChatSelection *selection = user_data;
	selection->selected = selected;
	selection->calls++;
}

static gboolean
check_dcc_chat_list_model (void)
{
	ProbeDccChatSelection selection = { 0 };
	gint first_identity = 1;
	gint second_identity = 2;
	FabulorDccChatSnapshot first = { 0 };
	FabulorDccChatSnapshot second = { 0 };
	FabulorDccChatList *list = fabulor_dcc_chat_list_new (
		probe_dcc_chat_selection, NULL, &selection);
	GPtrArray *rows = NULL;
	gboolean valid = list != NULL;

	first.identity = &first_identity;
	first.status = "Queued";
	first.nick = "alpha";
	first.received = "0 bytes";
	first.sent = "0 bytes";
	first.start_time = "Fri Jul 17 00:00:00 2026";
	second = first;
	second.identity = &second_identity;
	second.nick = "beta";
	if (valid)
	{
		valid = fabulor_dcc_chat_list_append (list, &first, FALSE) &&
			fabulor_dcc_chat_list_append (list, &second, TRUE) &&
			!fabulor_dcc_chat_list_append (list, &first, FALSE) &&
			fabulor_dcc_chat_list_get_n_rows (list) == 2;
	}
	if (valid)
	{
		first.status = "Active";
		first.received = "512 bytes";
		valid = fabulor_dcc_chat_list_update (list, &first) &&
			fabulor_dcc_chat_list_set_selected (list, 0, TRUE) &&
			fabulor_dcc_chat_list_set_selected (list, 1, TRUE) &&
			fabulor_dcc_chat_list_get_n_selected (list) == 2 &&
			selection.selected == 2 && selection.calls >= 2;
	}
	if (valid)
	{
		rows = fabulor_dcc_chat_list_dup_selected (list);
		valid = rows->len == 2 &&
			g_ptr_array_index (rows, 0) == &second_identity &&
			g_ptr_array_index (rows, 1) == &first_identity &&
			fabulor_dcc_chat_list_remove (list, &second_identity) &&
			!fabulor_dcc_chat_list_remove (list, &second_identity) &&
			fabulor_dcc_chat_list_get_n_rows (list) == 1;
		g_ptr_array_unref (rows);
		fabulor_dcc_chat_list_clear (list);
		valid = valid && fabulor_dcc_chat_list_get_n_rows (list) == 0 &&
			fabulor_dcc_chat_list_get_n_selected (list) == 0;
	}
	fabulor_dcc_chat_list_free (list);
	return valid;
}

int
main (void)
{
	gboolean gtk_ready = gtk_init_check ();
	g_log_set_always_fatal (G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL);

	check_compatibility_helper_signatures ();
	check_user_list_view_signatures ();
	check_channel_tree_view_signatures ();
	if (!check_internal_drag_payload ())
	{
		fprintf (stderr, "GTK4 internal drag payload format mismatch\n");
		return 1;
	}
	if (!check_flat_model_stack ())
	{
		fprintf (stderr, "GTK4 flat list model contract mismatch\n");
		return 1;
	}
	if (!check_tree_model_stack ())
	{
		fprintf (stderr, "GTK4 tree list model contract mismatch\n");
		return 1;
	}
	if (!check_channel_model ())
	{
		fprintf (stderr, "GTK4 channel hierarchy model contract mismatch\n");
		return 1;
	}
	if (gtk_ready && !check_channel_tree_view ())
	{
		fprintf (stderr, "GTK4 channel tree view contract mismatch\n");
		return 1;
	}
	if (!check_notify_list_model ())
	{
		fprintf (stderr, "GTK4 notify list model contract mismatch\n");
		return 1;
	}
	if (!check_user_list_model ())
	{
		fprintf (stderr, "GTK4 user list model contract mismatch\n");
		return 1;
	}
	if (!check_addon_list_model ())
	{
		fprintf (stderr, "GTK4 addon list model contract mismatch\n");
		return 1;
	}
	if (!check_url_list_model ())
	{
		fprintf (stderr, "GTK4 URL list model contract mismatch\n");
		return 1;
	}
	if (!check_ignore_list_model ())
	{
		fprintf (stderr, "GTK4 ignore list model contract mismatch\n");
		return 1;
	}
	if (!check_ban_list_model ())
	{
		fprintf (stderr, "GTK4 ban list model contract mismatch\n");
		return 1;
	}
	if (!check_channel_list_model ())
	{
		fprintf (stderr, "GTK4 channel list model contract mismatch\n");
		return 1;
	}
	if (!check_dcc_transfer_list_model ())
	{
		fprintf (stderr, "GTK4 DCC transfer list model contract mismatch\n");
		return 1;
	}
	if (!check_dcc_chat_list_model ())
	{
		fprintf (stderr, "GTK4 DCC Chat list model contract mismatch\n");
		return 1;
	}

	if (gtk_get_major_version () != GTK_MAJOR_VERSION ||
		gtk_get_minor_version () != GTK_MINOR_VERSION ||
		gtk_get_micro_version () != GTK_MICRO_VERSION)
	{
		fprintf (stderr, "GTK runtime/header version mismatch\n");
		return 1;
	}

	if (glib_major_version != GLIB_MAJOR_VERSION ||
		glib_minor_version != GLIB_MINOR_VERSION ||
		glib_micro_version != GLIB_MICRO_VERSION)
	{
		fprintf (stderr, "GLib runtime/header version mismatch\n");
		return 1;
	}

	printf ("GTK %u.%u.%u / GLib %u.%u.%u / %u-bit\n",
		gtk_get_major_version (),
		gtk_get_minor_version (),
		gtk_get_micro_version (),
		glib_major_version,
		glib_minor_version,
		glib_micro_version,
		(unsigned int) (GLIB_SIZEOF_VOID_P * 8));
	return 0;
}
