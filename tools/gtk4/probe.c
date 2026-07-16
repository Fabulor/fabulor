#include <gtk/gtk.h>

#include <stdio.h>

#include "../../src/fe-gtk/gtk-compat.h"
#include "../../src/fe-gtk/gtk4-list-models.h"
#include "../../src/fe-gtk/notify-list.h"
#include "../../src/fe-gtk/user-list-model.h"

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
	void (*volatile widget_set_pointing_cursor) (GtkWidget *, gboolean) =
		fabulor_gtk_widget_set_pointing_cursor;
	void (*volatile text_view_set_pointing_cursor) (GtkTextView *, gboolean) =
		fabulor_gtk_text_view_set_pointing_cursor;
	void (*volatile widget_on_click_released) (GtkWidget *, FabulorGtkClickFunc,
		gpointer) = fabulor_gtk_widget_on_click_released;
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
	(void) widget_set_pointing_cursor;
	(void) text_view_set_pointing_cursor;
	(void) widget_on_click_released;
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

int
main (void)
{
	check_compatibility_helper_signatures ();
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
