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
#include "../../src/fe-gtk/editable-list.h"
#include "../../src/fe-gtk/print-event-list.h"
#include "../../src/fe-gtk/key-binding-list.h"
#include "../../src/fe-gtk/sound-event-list.h"
#include "../../src/fe-gtk/preferences-category-list.h"
#include "../../src/fe-gtk/server-network-list.h"
#include "../../src/fe-gtk/server-entry-list.h"
#include "../../src/fe-gtk/spell-entry-words.h"
#include "../../src/fe-gtk/xtext-background.h"
#include "../../src/fe-gtk/xtext-accessible.h"
#include "../../src/fe-gtk/xtext-decoration.h"
#include "../../src/fe-gtk/xtext-display.h"
#include "../../src/fe-gtk/xtext-geometry.h"
#include "../../src/fe-gtk/xtext-hit-test.h"
#include "../../src/fe-gtk/xtext-input.h"
#include "../../src/fe-gtk/xtext-performance.h"
#include "../../src/fe-gtk/xtext-render-target.h"
#include "../../src/fe-gtk/xtext-scroll-copy.h"
#include "../../src/fe-gtk/xtext-selection.h"
#include "../../src/fe-gtk/xtext-widget-class.h"
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

typedef struct
{
	GtkWidget parent_instance;
} FabulorProbeXTextWidget;

typedef struct
{
	GtkWidgetClass parent_class;
} FabulorProbeXTextWidgetClass;

G_DEFINE_TYPE_WITH_CODE (FabulorProbeXTextWidget, fabulor_probe_xtext_widget,
	GTK_TYPE_WIDGET,
	G_IMPLEMENT_INTERFACE (GTK_TYPE_ACCESSIBLE_TEXT,
		fabulor_xtext_accessible_text_interface_init))

static void
probe_xtext_realize (GtkWidget *widget)
{
	(void) widget;
}

static void
probe_xtext_unrealize (GtkWidget *widget)
{
	(void) widget;
}

static void
probe_xtext_allocate (GtkWidget *widget,
	const FabulorXTextAllocation *allocation)
{
	(void) widget;
	(void) allocation;
}

static void
probe_xtext_render (GtkWidget *widget, const GdkRectangle *area,
	cairo_t *context)
{
	(void) widget;
	(void) area;
	(void) context;
}

static FabulorXTextRenderTarget *
probe_xtext_render_target (GtkWidget *widget)
{
	(void) widget;
	return NULL;
}

static void
fabulor_probe_xtext_widget_class_init (FabulorProbeXTextWidgetClass *class)
{
	static const FabulorXTextWidgetCallbacks callbacks = {
		probe_xtext_realize,
		probe_xtext_unrealize,
		probe_xtext_allocate,
		probe_xtext_render,
		probe_xtext_render_target
	};
	fabulor_xtext_widget_class_install (GTK_WIDGET_CLASS (class), &callbacks);
}

static void
fabulor_probe_xtext_widget_init (FabulorProbeXTextWidget *widget)
{
	(void) widget;
}

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
	void (*volatile widget_add_css_class) (GtkWidget *, const gchar *) =
		fabulor_gtk_widget_add_css_class;
	void (*volatile widget_queue_draw_region) (GtkWidget *, gint, gint,
		gint, gint) = fabulor_gtk_widget_queue_draw_region;
	gboolean (*volatile widget_has_toplevel_focus) (GtkWidget *) =
		fabulor_gtk_widget_has_toplevel_focus;
	FabulorXTextSelection *(*volatile xtext_selection_new) (GtkWidget *,
		FabulorXTextSelectionTextFunc, FabulorXTextSelectionClearFunc,
		gpointer) = fabulor_xtext_selection_new;
	void (*volatile xtext_selection_free) (FabulorXTextSelection *) =
		fabulor_xtext_selection_free;
	void (*volatile xtext_selection_publish) (FabulorXTextSelection *,
		const gchar *, gint, guint32) = fabulor_xtext_selection_publish;
	void (*volatile widget_on_pointer_enter) (GtkWidget *, FabulorGtkWidgetInteractionFunc, gpointer) =
		fabulor_gtk_widget_on_pointer_enter;
	void (*volatile widget_on_pointer_motion) (GtkWidget *, FabulorGtkPointerMotionFunc,
		FabulorGtkPointerLeaveFunc, gpointer) = fabulor_gtk_widget_on_pointer_motion;
	void (*volatile widget_on_pointer_motion_with_state) (GtkWidget *,
		FabulorGtkPointerMotionStateFunc, FabulorGtkPointerLeaveFunc, gpointer) =
		fabulor_gtk_widget_on_pointer_motion_with_state;
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
	void (*volatile paned_set_start_child) (GtkPaned *, GtkWidget *, gboolean,
		gboolean) = fabulor_gtk_paned_set_start_child;
	void (*volatile paned_set_end_child) (GtkPaned *, GtkWidget *, gboolean,
		gboolean) = fabulor_gtk_paned_set_end_child;
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
	(void) widget_add_css_class;
	(void) widget_queue_draw_region;
	(void) widget_has_toplevel_focus;
	(void) xtext_selection_new;
	(void) xtext_selection_free;
	(void) xtext_selection_publish;
	(void) widget_on_pointer_enter;
	(void) widget_on_pointer_motion;
	(void) widget_on_pointer_motion_with_state;
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
	(void) paned_set_start_child;
	(void) paned_set_end_child;
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

static gboolean
check_editable_list_model (void)
{
	FabulorEditableList *list = fabulor_editable_list_new ();
	GPtrArray *rows = NULL;
	gboolean valid = list != NULL;

	if (valid)
	{
		fabulor_editable_list_append (list, "alpha", "one");
		fabulor_editable_list_append (list, "beta", "two");
		fabulor_editable_list_append (list, "gamma", "three");
		valid = fabulor_editable_list_get_n_rows (list) == 3 &&
			fabulor_editable_list_set_text_at (list, 1,
				FABULOR_EDITABLE_LIST_COMMAND, "updated") &&
			fabulor_editable_list_set_selected (list, 1) &&
			fabulor_editable_list_move_selected (list, -1) &&
			!fabulor_editable_list_move_selected (list, -1);
	}
	if (valid)
	{
		rows = fabulor_editable_list_dup_all (list);
		valid = rows->len == 3 &&
			strcmp (((FabulorEditableListRecord *) g_ptr_array_index (
				rows, 0))->name, "beta") == 0 &&
			strcmp (((FabulorEditableListRecord *) g_ptr_array_index (
				rows, 0))->command, "updated") == 0 &&
			strcmp (((FabulorEditableListRecord *) g_ptr_array_index (
				rows, 1))->name, "alpha") == 0;
		g_ptr_array_unref (rows);
	}
	if (valid)
	{
		valid = fabulor_editable_list_set_selected (list, 1) &&
			fabulor_editable_list_remove_selected (list) &&
			fabulor_editable_list_get_n_rows (list) == 2;
		fabulor_editable_list_add_empty (list);
		valid = valid && fabulor_editable_list_get_n_rows (list) == 3 &&
			fabulor_editable_list_set_text_at (list, 2,
				FABULOR_EDITABLE_LIST_NAME, "delta") &&
			fabulor_editable_list_set_text_at (list, 2,
				FABULOR_EDITABLE_LIST_COMMAND, "four");
	}
	if (valid)
	{
		rows = fabulor_editable_list_dup_all (list);
		valid = rows->len == 3 &&
			strcmp (((FabulorEditableListRecord *) g_ptr_array_index (
				rows, 1))->name, "gamma") == 0 &&
			strcmp (((FabulorEditableListRecord *) g_ptr_array_index (
				rows, 2))->name, "delta") == 0 &&
			strcmp (((FabulorEditableListRecord *) g_ptr_array_index (
				rows, 2))->command, "four") == 0;
		g_ptr_array_unref (rows);
	}
	fabulor_editable_list_free (list);
	return valid;
}

typedef struct
{
	gint selected;
	gint edited;
	guint selections;
	guint edits;
} ProbePrintEventCallbacks;

static gboolean
probe_print_event_edit (gint signal_index, const gchar *new_text,
	gpointer user_data)
{
	ProbePrintEventCallbacks *callbacks = user_data;
	callbacks->edited = signal_index;
	callbacks->edits++;
	return g_strcmp0 (new_text, "reject") != 0;
}

static void
probe_print_event_selection (gint signal_index, gpointer user_data)
{
	ProbePrintEventCallbacks *callbacks = user_data;
	callbacks->selected = signal_index;
	callbacks->selections++;
}

static gboolean
check_print_event_list_model (void)
{
	ProbePrintEventCallbacks callbacks = { -1, -1, 0, 0 };
	FabulorPrintEventList *list = fabulor_print_event_list_new (
		probe_print_event_edit, probe_print_event_selection, &callbacks);
	gchar *text = NULL;
	gboolean valid = list != NULL;

	if (valid)
	{
		fabulor_print_event_list_append_event (list, "Connected", "old", 7);
		fabulor_print_event_list_append_event (list, "Message", "message", 12);
		fabulor_print_event_list_append_help (list, 1, "Nick");
		fabulor_print_event_list_append_help (list, 2, "Message");
		valid = fabulor_print_event_list_get_n_events (list) == 2 &&
			fabulor_print_event_list_get_n_help (list) == 2 &&
			fabulor_print_event_list_get_signal_at (list, 1) == 12 &&
			fabulor_print_event_list_select_at (list, 1) &&
			callbacks.selections >= 1 && callbacks.selected == 12;
	}
	if (valid)
	{
		valid = fabulor_print_event_list_edit_at (list, 0, "updated") &&
			callbacks.edits == 1 && callbacks.edited == 7 &&
			!fabulor_print_event_list_edit_at (list, 1, "reject") &&
			callbacks.edits == 2 && callbacks.edited == 12;
		text = fabulor_print_event_list_dup_text_at (list, 0);
		valid = valid && g_strcmp0 (text, "updated") == 0;
		g_clear_pointer (&text, g_free);
		text = fabulor_print_event_list_dup_text_at (list, 1);
		valid = valid && g_strcmp0 (text, "message") == 0;
		g_clear_pointer (&text, g_free);
	}
	if (valid)
	{
		fabulor_print_event_list_clear_help (list);
		fabulor_print_event_list_clear_events (list);
		valid = fabulor_print_event_list_get_n_help (list) == 0 &&
			fabulor_print_event_list_get_n_events (list) == 0;
	}
	fabulor_print_event_list_free (list);
	return valid;
}

typedef struct
{
	gchar *action;
	gboolean custom;
	guint calls;
} ProbeKeyBindingSelection;

static void
probe_key_binding_selection (const gchar *action, gboolean custom,
	gpointer user_data)
{
	ProbeKeyBindingSelection *selection = user_data;
	g_free (selection->action);
	selection->action = g_strdup (action);
	selection->custom = custom;
	selection->calls++;
}

static GdkModifierType
probe_key_binding_normalize (GdkModifierType modifiers, gpointer user_data)
{
	(void) user_data;
	return modifiers & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_ALT_MASK);
}

static gboolean
check_key_binding_list_model (void)
{
	ProbeKeyBindingSelection selection = { 0 };
	FabulorKeyBindingRecord builtin = {
		"Ctrl+B", "<Primary>b", "Built in", "one", "two", FALSE
	};
	FabulorKeyBindingRecord custom = {
		"Ctrl+C", "<Primary>c", "Custom", "three", "four", TRUE
	};
	FabulorKeyBindingList *list = fabulor_key_binding_list_new (
		probe_key_binding_selection, probe_key_binding_normalize, &selection);
	GPtrArray *rows = NULL;
	gboolean valid = list != NULL;

	if (valid)
	{
		fabulor_key_binding_list_append (list, &builtin);
		fabulor_key_binding_list_append (list, &custom);
		fabulor_key_binding_list_add_custom (list);
		valid = fabulor_key_binding_list_get_n_rows (list) == 3 &&
			selection.calls >= 1 && selection.custom &&
			!fabulor_key_binding_list_set_text_at (list, 0,
				FABULOR_KEY_BINDING_DATA1, "blocked") &&
			fabulor_key_binding_list_set_text_at (list, 1,
				FABULOR_KEY_BINDING_ACTION, "Updated") &&
			fabulor_key_binding_list_set_text_at (list, 1,
				FABULOR_KEY_BINDING_DATA1, "changed") &&
			fabulor_key_binding_list_set_accelerator_at (list, 1,
				GDK_KEY_x, GDK_CONTROL_MASK | GDK_LOCK_MASK);
	}
	if (valid)
	{
		valid = fabulor_key_binding_list_set_selected (list, 1) &&
			!fabulor_key_binding_list_move_selected (list, -1) &&
			fabulor_key_binding_list_move_selected (list, 1);
		rows = fabulor_key_binding_list_dup_all (list);
		valid = valid && rows->len == 3 &&
			g_strcmp0 (((FabulorKeyBindingRecord *) g_ptr_array_index (
				rows, 2))->action, "Updated") == 0 &&
			g_strcmp0 (((FabulorKeyBindingRecord *) g_ptr_array_index (
				rows, 2))->data1, "changed") == 0 &&
			((FabulorKeyBindingRecord *) g_ptr_array_index (rows, 2))->custom;
		g_ptr_array_unref (rows);
	}
	if (valid)
	{
		valid = fabulor_key_binding_list_remove_selected (list) &&
			fabulor_key_binding_list_get_n_rows (list) == 2 &&
			fabulor_key_binding_list_set_selected (list, 0) &&
			!fabulor_key_binding_list_remove_selected (list);
		fabulor_key_binding_list_clear (list);
		valid = valid && fabulor_key_binding_list_get_n_rows (list) == 0;
	}
	fabulor_key_binding_list_free (list);
	g_free (selection.action);
	return valid;
}

typedef struct
{
	gint event_index;
	guint calls;
} ProbeSoundEventSelection;

static void
probe_sound_event_selection (gint event_index, gpointer user_data)
{
	ProbeSoundEventSelection *selection = user_data;
	selection->event_index = event_index;
	selection->calls++;
}

static gboolean
check_sound_event_list_model (void)
{
	ProbeSoundEventSelection selection = { -1, 0 };
	FabulorSoundEventList *list = fabulor_sound_event_list_new (
		probe_sound_event_selection, &selection);
	gchar *file = NULL;
	gboolean valid = list != NULL;

	if (valid)
	{
		fabulor_sound_event_list_append (list, "Connected", "one.wav", 7);
		fabulor_sound_event_list_append (list, "Message", NULL, 12);
		valid = fabulor_sound_event_list_get_n_rows (list) == 2 &&
			fabulor_sound_event_list_select_event (list, 12) &&
			fabulor_sound_event_list_get_selected_event (list) == 12 &&
			selection.calls >= 1 && selection.event_index == 12;
	}
	if (valid)
	{
		valid = fabulor_sound_event_list_update_file (list, 12, "two.wav") &&
			!fabulor_sound_event_list_update_file (list, 99, "missing.wav");
		file = fabulor_sound_event_list_dup_file (list, 12);
		valid = valid && g_strcmp0 (file, "two.wav") == 0 &&
			fabulor_sound_event_list_dup_file (list, 99) == NULL;
		g_free (file);
	}
	if (valid)
	{
		fabulor_sound_event_list_clear (list);
		valid = fabulor_sound_event_list_get_n_rows (list) == 0 &&
			fabulor_sound_event_list_get_selected_event (list) == -1;
	}
	fabulor_sound_event_list_free (list);
	return valid;
}

typedef struct
{
	gint page_index;
	guint calls;
} ProbePreferencesCategorySelection;

static void
probe_preferences_category_selection (gint page_index, gpointer user_data)
{
	ProbePreferencesCategorySelection *selection = user_data;
	selection->page_index = page_index;
	selection->calls++;
}

static gboolean
check_preferences_category_list_model (void)
{
	ProbePreferencesCategorySelection selection = { -1, 0 };
	FabulorPreferencesCategoryList *list =
		fabulor_preferences_category_list_new (
			probe_preferences_category_selection, &selection);
	guint interface_category;
	guint chatting_category;
	gboolean valid = list != NULL;

	if (valid)
	{
		interface_category = fabulor_preferences_category_list_append_category (
			list, "Interface");
		fabulor_preferences_category_list_append_page (list,
			interface_category, "Appearance", 0);
		fabulor_preferences_category_list_append_page (list,
			interface_category, "Input box", 1);
		chatting_category = fabulor_preferences_category_list_append_category (
			list, "Chatting");
		fabulor_preferences_category_list_append_page (list,
			chatting_category, "General", 4);
		valid = interface_category == 0 && chatting_category == 1 &&
			fabulor_preferences_category_list_get_n_categories (list) == 2 &&
			fabulor_preferences_category_list_get_n_pages (list) == 3;
	}
	if (valid)
	{
		valid = fabulor_preferences_category_list_select_page (list, 4) &&
			fabulor_preferences_category_list_get_selected_page (list) == 4 &&
			selection.calls == 1 && selection.page_index == 4 &&
			!fabulor_preferences_category_list_select_page (list, 2) &&
			fabulor_preferences_category_list_get_selected_page (list) == 4 &&
			selection.calls == 1;
	}
	if (valid)
	{
		valid = fabulor_preferences_category_list_select_page (list, 0) &&
			fabulor_preferences_category_list_get_selected_page (list) == 0 &&
			selection.calls == 2 && selection.page_index == 0;
	}
	fabulor_preferences_category_list_free (list);
	return valid;
}

typedef struct
{
	gpointer identity;
	guint calls;
} ProbeServerNetworkSelection;

static void
probe_server_network_selection (gpointer identity, gpointer user_data)
{
	ProbeServerNetworkSelection *selection = user_data;
	selection->identity = identity;
	selection->calls++;
}

static gboolean
probe_server_network_edit (gpointer identity, const gchar *new_name,
	gpointer user_data)
{
	(void) identity;
	(void) user_data;
	return new_name && new_name[0] != '\0';
}

static gboolean
check_server_network_list_model (void)
{
	ProbeServerNetworkSelection selection = { NULL, 0 };
	FabulorServerNetworkList *list = fabulor_server_network_list_new (
		probe_server_network_selection, probe_server_network_edit, &selection);
	gint identities[3];
	gboolean favorite = FALSE;
	gchar *name = NULL;
	gboolean valid = list != NULL;

	if (valid)
	{
		valid = fabulor_server_network_list_append (list, &identities[0],
			"Alpha", FALSE, FALSE) &&
			fabulor_server_network_list_append (list, &identities[1],
				"Beta", TRUE, FALSE) &&
			fabulor_server_network_list_append (list, &identities[2],
				"First", FALSE, TRUE) &&
			!fabulor_server_network_list_append (list, &identities[0],
				"Duplicate", FALSE, FALSE) &&
			fabulor_server_network_list_get_n_rows (list) == 3 &&
			fabulor_server_network_list_get_identity_at (list, 0) ==
				&identities[2] &&
			fabulor_server_network_list_get_identity_at (list, 1) ==
				&identities[0];
	}
	if (valid)
	{
		valid = fabulor_server_network_list_select (list, &identities[0]) &&
			fabulor_server_network_list_get_selected (list) == &identities[0] &&
			selection.identity == &identities[0] && selection.calls >= 1 &&
			fabulor_server_network_list_move (list, &identities[0], 1) &&
			fabulor_server_network_list_get_identity_at (list, 2) ==
				&identities[0] &&
			!fabulor_server_network_list_move (list, &identities[0], 1);
	}
	if (valid)
	{
		valid = fabulor_server_network_list_set_favorite (list,
			&identities[0], TRUE) &&
			fabulor_server_network_list_get_favorite (list, &identities[0],
				&favorite) && favorite &&
			fabulor_server_network_list_update_name (list, &identities[0],
				"Renamed");
		name = fabulor_server_network_list_dup_name (list, &identities[0]);
		valid = valid && g_strcmp0 (name, "Renamed") == 0;
		g_free (name);
	}
	if (valid)
	{
		valid = fabulor_server_network_list_remove (list, &identities[1]) &&
			!fabulor_server_network_list_remove (list, &identities[1]) &&
			fabulor_server_network_list_get_n_rows (list) == 2;
		fabulor_server_network_list_clear (list);
		valid = valid && fabulor_server_network_list_get_n_rows (list) == 0 &&
			fabulor_server_network_list_get_selected (list) == NULL;
	}
	fabulor_server_network_list_free (list);
	return valid;
}

typedef struct
{
	gpointer identity;
	guint calls;
} ProbeServerEntrySelection;

static void
probe_server_entry_selection (gpointer identity, gpointer user_data)
{
	ProbeServerEntrySelection *selection = user_data;
	selection->identity = identity;
	selection->calls++;
}

static gboolean
check_server_entry_list_model (void)
{
	ProbeServerEntrySelection selection = { NULL, 0 };
	FabulorServerEntryList *list = fabulor_server_entry_list_new (TRUE,
		probe_server_entry_selection, NULL, &selection);
	FabulorServerEntryList *single = fabulor_server_entry_list_new (FALSE,
		NULL, NULL, NULL);
	gint identities[3];
	gchar *text = NULL;
	gboolean valid = list != NULL && single != NULL;

	if (valid)
	{
		valid = fabulor_server_entry_list_append (list, &identities[0],
			"Alpha", "key-a") &&
			fabulor_server_entry_list_append (list, &identities[1],
				"Beta", "") &&
			fabulor_server_entry_list_append (list, &identities[2],
				"Alpha", "key-c") &&
			!fabulor_server_entry_list_append (list, &identities[0],
				"Duplicate identity", "") &&
			fabulor_server_entry_list_get_n_rows (list) == 3 &&
			fabulor_server_entry_list_get_identity_at (list, 2) ==
				&identities[2];
	}
	if (valid)
	{
		valid = fabulor_server_entry_list_select (list, &identities[1]) &&
			fabulor_server_entry_list_get_selected (list) == &identities[1] &&
			selection.identity == &identities[1] && selection.calls >= 1 &&
			fabulor_server_entry_list_move (list, &identities[1], -1) &&
			fabulor_server_entry_list_get_identity_at (list, 0) ==
				&identities[1] &&
			!fabulor_server_entry_list_move (list, &identities[1], -1);
	}
	if (valid)
	{
		valid = fabulor_server_entry_list_update (list, &identities[1],
			FABULOR_SERVER_ENTRY_PRIMARY, "Renamed") &&
			fabulor_server_entry_list_update (list, &identities[1],
				FABULOR_SERVER_ENTRY_SECONDARY, "new-key");
		text = fabulor_server_entry_list_dup_text (list, &identities[1],
			FABULOR_SERVER_ENTRY_SECONDARY);
		valid = valid && g_strcmp0 (text, "new-key") == 0;
		g_free (text);
	}
	if (valid)
	{
		valid = fabulor_server_entry_list_append (single, &identities[0],
			"Only", NULL) &&
			!fabulor_server_entry_list_update (single, &identities[0],
				FABULOR_SERVER_ENTRY_SECONDARY, "invalid") &&
			fabulor_server_entry_list_dup_text (single, &identities[0],
				FABULOR_SERVER_ENTRY_SECONDARY) == NULL &&
			fabulor_server_entry_list_remove (list, &identities[0]) &&
			!fabulor_server_entry_list_remove (list, &identities[0]);
		fabulor_server_entry_list_clear (list);
		valid = valid && fabulor_server_entry_list_get_n_rows (list) == 0 &&
			fabulor_server_entry_list_get_selected (list) == NULL;
	}
	fabulor_server_entry_list_free (single);
	fabulor_server_entry_list_free (list);
	return valid;
}

static gboolean
check_xtext_decoration_policy (void)
{
	FabulorXTextDecoration *decoration = fabulor_xtext_decoration_new ();
	offsets_t first = { 0 };
	offsets_t second = { 0 };
	GList *marks = NULL;
	GList *current;
	gint entry_a = 1;
	gint entry_b = 2;
	gint marker_y = -1;
	gboolean valid = decoration != NULL &&
		!fabulor_xtext_marker_position (FALSE, &entry_a, &entry_a, NULL,
			10, 3, 12, 2, &marker_y) && marker_y == 0 &&
		fabulor_xtext_marker_position (TRUE, &entry_a, &entry_a, &entry_b,
			10, 3, 12, 2, &marker_y) && marker_y == 13 &&
		fabulor_xtext_marker_position (TRUE, &entry_b, &entry_a, &entry_b,
			10, 3, 12, 2, &marker_y) && marker_y == 37;

	first.o.start = 2;
	first.o.end = 5;
	second.o.start = 5;
	second.o.end = 8;
	marks = g_list_append (marks, GUINT_TO_POINTER (first.u));
	marks = g_list_append (marks, GUINT_TO_POINTER (second.u));
	current = g_list_last (marks);
	valid = valid && fabulor_xtext_search_match (marks, current, 1) == 0 &&
		fabulor_xtext_search_match (marks, current, 2) ==
			(FABULOR_XTEXT_MATCH_START | FABULOR_XTEXT_MATCH_MID) &&
		fabulor_xtext_search_match (marks, current, 5) ==
			(FABULOR_XTEXT_MATCH_MID | FABULOR_XTEXT_MATCH_CURRENT) &&
		fabulor_xtext_search_match (marks, current, 6) ==
			(FABULOR_XTEXT_MATCH_MID | FABULOR_XTEXT_MATCH_CURRENT) &&
		fabulor_xtext_search_match (marks, current, 8) ==
			(FABULOR_XTEXT_MATCH_MID | FABULOR_XTEXT_MATCH_END);
	g_list_free (marks);

	valid = valid && !fabulor_xtext_decoration_has_hover (decoration) &&
		!fabulor_xtext_decoration_set_hover (decoration, NULL, 2, 5) &&
		fabulor_xtext_decoration_set_hover (decoration, &entry_a, 2, 5) &&
		fabulor_xtext_decoration_hover_equals (decoration, &entry_a, 2, 5) &&
		fabulor_xtext_decoration_hover_contains (decoration, &entry_a, 2) &&
		fabulor_xtext_decoration_hover_contains (decoration, &entry_a, 4) &&
		!fabulor_xtext_decoration_hover_contains (decoration, &entry_a, 5) &&
		fabulor_xtext_decoration_hover_starts (decoration, &entry_a, 2) &&
		fabulor_xtext_decoration_hover_ends (decoration, &entry_a, 5);
	fabulor_xtext_decoration_begin_hover_render (decoration, TRUE);
	fabulor_xtext_decoration_set_hover_inside (decoration, TRUE);
	valid = valid && fabulor_xtext_decoration_hover_render_only (decoration) &&
		fabulor_xtext_decoration_hover_clearing (decoration) &&
		fabulor_xtext_decoration_hover_inside (decoration);
	fabulor_xtext_decoration_suspend_hover (decoration);
	valid = valid && !fabulor_xtext_decoration_hover_contains (decoration,
		&entry_a, 3);
	fabulor_xtext_decoration_resume_hover (decoration);
	valid = valid && fabulor_xtext_decoration_hover_contains (decoration,
		&entry_a, 3);
	fabulor_xtext_decoration_end_hover_render (decoration);
	fabulor_xtext_decoration_clear_hover (decoration);
	valid = valid && !fabulor_xtext_decoration_has_hover (decoration) &&
		!fabulor_xtext_decoration_hover_render_only (decoration) &&
		!fabulor_xtext_decoration_hover_clearing (decoration) &&
		!fabulor_xtext_decoration_hover_inside (decoration);

	fabulor_xtext_decoration_free (decoration);
	return valid;
}

static gboolean
check_xtext_background_policy (void)
{
	FabulorXTextBackground *background = fabulor_xtext_background_new ();
	FabulorXTextGeometry geometry = { 4, 4 };
	XTextColor fallback = { 1.0, 0.0, 0.0, 1.0 };
	cairo_surface_t *output = cairo_image_surface_create (
		CAIRO_FORMAT_ARGB32, 4, 4);
	cairo_surface_t *source = cairo_image_surface_create (
		CAIRO_FORMAT_ARGB32, 4, 2);
	cairo_t *context = cairo_create (output);
	cairo_t *source_context = cairo_create (source);
	guint32 pixel = 0;
	gboolean valid = background != NULL &&
		cairo_surface_status (output) == CAIRO_STATUS_SUCCESS &&
		cairo_surface_status (source) == CAIRO_STATUS_SUCCESS &&
		cairo_status (context) == CAIRO_STATUS_SUCCESS &&
		cairo_status (source_context) == CAIRO_STATUS_SUCCESS &&
		!fabulor_xtext_background_has_surface (background);

	if (valid)
	{
		fabulor_xtext_background_begin_frame (background);
		fabulor_xtext_background_paint (background, context, &fallback,
			&geometry, 0, 0, 4, 4, 0, 0);
		fabulor_xtext_background_end_frame (background);
		cairo_surface_flush (output);
		memcpy (&pixel, cairo_image_surface_get_data (output), sizeof pixel);
		valid = pixel == 0xffff0000U;
		if (!valid)
			fprintf (stderr, "fallback pixel: 0x%08x\n", pixel);
	}
	if (valid)
	{
		cairo_set_source_rgb (source_context, 0.0, 1.0, 0.0);
		cairo_paint (source_context);
		cairo_surface_flush (source);
		cairo_set_operator (context, CAIRO_OPERATOR_CLEAR);
		cairo_paint (context);
		cairo_set_operator (context, CAIRO_OPERATOR_OVER);
		fabulor_xtext_background_set_surface (background, source);
		valid = fabulor_xtext_background_has_surface (background);
		fabulor_xtext_background_begin_frame (background);
		fabulor_xtext_background_paint (background, context, &fallback,
			&geometry, 0, 0, 4, 4, 0, 0);
		fabulor_xtext_background_end_frame (background);
		cairo_surface_flush (output);
		memcpy (&pixel, cairo_image_surface_get_data (output), sizeof pixel);
		valid = valid && pixel == 0xff000000U;
		if (pixel != 0xff000000U)
			fprintf (stderr, "letterbox pixel: 0x%08x\n", pixel);
		memcpy (&pixel, cairo_image_surface_get_data (output) +
			(2 * cairo_image_surface_get_stride (output)), sizeof pixel);
		valid = valid && pixel == 0xff00ff00U;
		if (pixel != 0xff00ff00U)
			fprintf (stderr, "fitted image pixel: 0x%08x\n", pixel);
		fabulor_xtext_background_set_surface (background, NULL);
		valid = valid && !fabulor_xtext_background_has_surface (background);
	}

	cairo_destroy (source_context);
	cairo_destroy (context);
	cairo_surface_destroy (source);
	cairo_surface_destroy (output);
	fabulor_xtext_background_free (background);
	return valid;
}

static gboolean
check_xtext_geometry (void)
{
	FabulorXTextGeometry geometry = { 99, 99 };
	gboolean valid = fabulor_xtext_geometry_init (&geometry, 640, 480) &&
		geometry.width == 640 && geometry.height == 480;

	valid = valid && !fabulor_xtext_geometry_init (&geometry, 0, 480) &&
		geometry.width == 0 && geometry.height == 0;
	valid = valid && !fabulor_xtext_geometry_init (&geometry, 640, -1) &&
		geometry.width == 0 && geometry.height == 0;
	return valid;
}

static gboolean
check_xtext_hit_test_policy (void)
{
	offlen_t first = { 0, 2, 0, 0 };
	offlen_t second = { 3, 4, 0, 0 };
	GSList *runs = NULL;
	FabulorXTextHit hit;
	gchar word[] = "xx#fabulor,";
	gchar *match;
	gint line = -1;
	gint offset = 10;
	gint length = 11;
	gboolean valid = fabulor_xtext_hit_test_line (0, 0, 10, 3, &line) &&
		line == 3 &&
		fabulor_xtext_hit_test_line (-1, 0, 10, 3, &line) && line == 2 &&
		!fabulor_xtext_hit_test_line (0, 0, 0, 3, &line) && line == 0 &&
		fabulor_xtext_hit_test_separator (TRUE, 100, 9, 94) &&
		fabulor_xtext_hit_test_separator (TRUE, 100, 9, 95) &&
		fabulor_xtext_hit_test_separator (TRUE, 100, 9, 96) &&
		!fabulor_xtext_hit_test_separator (TRUE, 100, 9, 97) &&
		!fabulor_xtext_hit_test_separator (FALSE, 100, 9, 95);

	runs = g_slist_append (runs, &first);
	runs = g_slist_append (runs, &second);
	valid = valid && fabulor_xtext_hit_test_adjust_match (runs, 2, 6,
		&offset, &length) && offset == 13 && length == 4;
	g_slist_free (runs);

	fabulor_xtext_hit_init (&hit, word, 2, 2, 10);
	valid = valid && fabulor_xtext_hit_has_match (&hit);
	match = fabulor_xtext_hit_dup_match (&hit);
	valid = valid && g_strcmp0 (match, "#fabulor") == 0 &&
		g_strcmp0 (word, "xx#fabulor,") == 0;
	g_free (match);
	fabulor_xtext_hit_init (&hit, word, 2, 2, 99);
	valid = valid && hit.type == 0 && !fabulor_xtext_hit_has_match (&hit) &&
		fabulor_xtext_hit_dup_match (&hit) == NULL;
	fabulor_xtext_hit_init (&hit, word, -1, 0, 0);
	valid = valid && hit.type == -1 && !fabulor_xtext_hit_has_match (&hit);
	return valid;
}

static gboolean
check_xtext_input_policy (void)
{
	return fabulor_xtext_selection_press (2, 1) ==
			FABULOR_XTEXT_SELECTION_PRESS_NONE &&
		fabulor_xtext_selection_press (1, 1) ==
			FABULOR_XTEXT_SELECTION_PRESS_SINGLE &&
		fabulor_xtext_selection_press (1, 2) ==
			FABULOR_XTEXT_SELECTION_PRESS_WORD &&
		fabulor_xtext_selection_press (1, 3) ==
			FABULOR_XTEXT_SELECTION_PRESS_LINE &&
		fabulor_xtext_selection_press (1, 4) ==
			FABULOR_XTEXT_SELECTION_PRESS_LINE &&
		fabulor_xtext_scroll_direction (-0.25) == -1 &&
		fabulor_xtext_scroll_direction (0.0) == 0 &&
		fabulor_xtext_scroll_direction (0.25) == 1;
}

static gboolean
check_xtext_selection_policy (void)
{
	gchar *full = fabulor_xtext_selection_copy_text ("Fabulor", -1);
	gchar *bounded = fabulor_xtext_selection_copy_text ("Fabulor", 3);
	gboolean valid = g_strcmp0 (full, "Fabulor") == 0 &&
		g_strcmp0 (bounded, "Fab") == 0;

	g_free (full);
	g_free (bounded);
	return valid;
}

static gboolean
check_xtext_scroll_copy_policy (void)
{
	FabulorXTextScrollCopy copy;
	gboolean valid;

	valid = fabulor_xtext_scroll_copy_plan (-20, 100, 10, 2, TRUE,
		&copy) && copy.source_y == 20 && copy.destination_y == 0 &&
		copy.copy_height == 80 && copy.damage_y == 70 &&
		copy.damage_height == 30;
	valid = valid && fabulor_xtext_scroll_copy_plan (20, 100, 10, 2,
		TRUE, &copy) && copy.source_y == 0 && copy.destination_y == 20 &&
		copy.copy_height == 80 && copy.damage_y == 0 &&
		copy.damage_height == 20;
	valid = valid && !fabulor_xtext_scroll_copy_plan (-20, 100, 10, 2,
		FALSE, &copy) && copy.copy_height == 0 && copy.damage_height == 0;
	valid = valid && !fabulor_xtext_scroll_copy_plan (100, 100, 10, 2,
		TRUE, &copy);
	return valid;
}

static gboolean
check_spell_entry_word_policy (void)
{
	FabulorSpellWords *words = fabulor_spell_words_new (
		"alpha caf\303\251 omega", pango_language_from_string ("en"));
	FabulorSpellWordRange range;
	gchar *word;
	gboolean valid;

	valid = fabulor_spell_words_count (words) == 3 &&
		fabulor_spell_words_get (words, 1, &range) &&
		range.byte_start == 6 && range.byte_end == 11 &&
		range.character_start == 6 && range.character_end == 10 &&
		fabulor_spell_words_find_character (words, 8, &range) &&
		range.byte_start == 6 && range.byte_end == 11 &&
		fabulor_spell_words_find_character (words, 10, &range) &&
		range.character_start == 6 && range.character_end == 10;
	word = fabulor_spell_words_dup_word (words, 1);
	valid = valid && g_strcmp0 (word, "caf\303\251") == 0;
	g_free (word);
	fabulor_spell_words_free (words);

	words = fabulor_spell_words_new (NULL, NULL);
	valid = valid && fabulor_spell_words_count (words) == 0 &&
		!fabulor_spell_words_get (words, 0, &range) &&
		!fabulor_spell_words_find_character (words, 0, &range) &&
		fabulor_spell_words_dup_word (words, 0) == NULL;
	fabulor_spell_words_free (words);
	return valid;
}

static gboolean
check_xtext_performance_policy (void)
{
	const guint iterations = 1000000;
	volatile guint checksum = 0;
	gint lines = 100000;
	guint removals = 0;
	gint64 started;
	gint64 elapsed;
	guint i;

	if (fabulor_xtext_append_refresh_plan (FALSE, FALSE, TRUE) !=
		FABULOR_XTEXT_APPEND_REFRESH_NONE ||
		fabulor_xtext_append_refresh_plan (TRUE, TRUE, FALSE) !=
		FABULOR_XTEXT_APPEND_REFRESH_NONE ||
		fabulor_xtext_append_refresh_plan (TRUE, TRUE, TRUE) !=
		FABULOR_XTEXT_APPEND_REFRESH_IMMEDIATE ||
		fabulor_xtext_append_refresh_plan (TRUE, FALSE, TRUE) !=
		FABULOR_XTEXT_APPEND_REFRESH_IMMEDIATE ||
		fabulor_xtext_append_refresh_plan (TRUE, FALSE, FALSE) !=
		FABULOR_XTEXT_APPEND_REFRESH_IDLE ||
		fabulor_xtext_should_trim_oldest (0, 5001, TRUE) ||
		fabulor_xtext_should_trim_oldest (2, 5001, TRUE) ||
		fabulor_xtext_should_trim_oldest (5000, 5000, TRUE) ||
		fabulor_xtext_should_trim_oldest (5000, 5001, FALSE) ||
		!fabulor_xtext_should_trim_oldest (5000, 5001, TRUE))
		return FALSE;

	while (fabulor_xtext_should_trim_oldest (5000, lines, lines > 1))
	{
		lines -= (gint) ((removals % 4) + 1);
		removals++;
	}
	if (lines > 5000 || lines < 4997 || removals != 38000)
		return FALSE;

	started = g_get_monotonic_time ();
	for (i = 0; i < iterations; i++)
	{
		checksum += (guint) fabulor_xtext_append_refresh_plan (TRUE,
			(i & 3u) != 0, (i & 4u) != 0);
	}
	elapsed = g_get_monotonic_time () - started;
	printf ("Transcript policy diagnostic: %u decisions in %" G_GINT64_FORMAT
		" us\n", iterations, elapsed);
	return checksum == 750000;
}

static gboolean
probe_bytes_equal (GBytes *bytes, const gchar *expected)
{
	gsize length;
	const gchar *data = g_bytes_get_data (bytes, &length);

	return length == strlen (expected) &&
		memcmp (data, expected, length) == 0;
}

static gboolean
check_xtext_accessible_policy (void)
{
	FabulorXTextAccessible *accessible = fabulor_xtext_accessible_new (NULL,
		NULL);
	FabulorXTextAccessibleChange change;
	GBytes *contents;
	gchar *large;
	gsize content_bytes;
	guint start;
	guint end;
	gboolean valid;

	valid = !fabulor_xtext_accessible_is_observed (accessible) &&
		fabulor_xtext_accessible_replace (accessible,
		"Hello world.\nSecond line", &change) && change.remove_start == 0 &&
		change.remove_end == 0 && change.insert_start == 0 &&
		change.insert_end == 24 &&
		fabulor_xtext_accessible_length (accessible) == 24 &&
		fabulor_xtext_accessible_is_observed (accessible);
	contents = fabulor_xtext_accessible_contents (accessible, 6, 11);
	valid = valid && probe_bytes_equal (contents, "world");
	g_bytes_unref (contents);
	contents = fabulor_xtext_accessible_contents_at (accessible, 1,
		FABULOR_XTEXT_ACCESSIBLE_CHARACTER, &start, &end);
	valid = valid && start == 1 && end == 2 &&
		probe_bytes_equal (contents, "e");
	g_bytes_unref (contents);
	contents = fabulor_xtext_accessible_contents_at (accessible, 7,
		FABULOR_XTEXT_ACCESSIBLE_WORD, &start, &end);
	valid = valid && start == 6 && end == 13 &&
		probe_bytes_equal (contents, "world.\n");
	g_bytes_unref (contents);
	contents = fabulor_xtext_accessible_contents_at (accessible, 7,
		FABULOR_XTEXT_ACCESSIBLE_SENTENCE, &start, &end);
	valid = valid && start == 0 && end == 13 &&
		probe_bytes_equal (contents, "Hello world.\n");
	g_bytes_unref (contents);
	contents = fabulor_xtext_accessible_contents_at (accessible, 7,
		FABULOR_XTEXT_ACCESSIBLE_LINE, &start, &end);
	valid = valid && start == 0 && end == 13 &&
		probe_bytes_equal (contents, "Hello world.\n");
	g_bytes_unref (contents);
	valid = valid && fabulor_xtext_accessible_replace (accessible,
		"Hello brave world.\nSecond line", &change) &&
		change.remove_start == 6 && change.remove_end == 6 &&
		change.insert_start == 6 && change.insert_end == 12;
	large = g_malloc (FABULOR_XTEXT_ACCESSIBLE_MAX_BYTES + 32);
	memset (large, 'a', FABULOR_XTEXT_ACCESSIBLE_MAX_BYTES + 31);
	large[FABULOR_XTEXT_ACCESSIBLE_MAX_BYTES + 31] = '\0';
	valid = valid && fabulor_xtext_accessible_replace (accessible, large,
		&change);
	contents = fabulor_xtext_accessible_contents (accessible, 0, G_MAXUINT);
	g_bytes_get_data (contents, &content_bytes);
	valid = valid && content_bytes <= FABULOR_XTEXT_ACCESSIBLE_MAX_BYTES &&
		fabulor_xtext_accessible_length (accessible) == content_bytes;
	g_bytes_unref (contents);
	g_free (large);
	fabulor_xtext_accessible_free (accessible);
	return valid;
}

static gboolean
check_xtext_display_policy (void)
{
	FabulorXTextFontMetrics metrics;
	gint logical_width;
	gint logical_height;
	gint device_width;
	gint device_height;
	gint logical_pixels;
	gint strike_y;
	gint underline_y;

	if (!fabulor_xtext_font_metrics_init (10 * 1024 + 511,
		3 * 1024 + 999, 14 * 1024 + 500, 1024, FALSE, &metrics) ||
		metrics.ascent != 10 || metrics.descent != 3 ||
		metrics.line_height != 15)
		return FALSE;
	if (!fabulor_xtext_font_metrics_init (10 * 1024 + 511,
		3 * 1024 + 999, 14 * 1024 + 500, 1024, TRUE, &metrics) ||
		metrics.line_height != 13 ||
		fabulor_xtext_font_metrics_init (-1, 0, 0, 1024, FALSE, &metrics))
		return FALSE;
	if (!fabulor_xtext_inline_image_size (20, 2, &logical_width,
		&logical_height, &device_width, &device_height) ||
		logical_width != 24 || logical_height != 18 ||
		device_width != 48 || device_height != 36)
		return FALSE;
	if (!fabulor_xtext_inline_image_size (10, 1, &logical_width,
		&logical_height, NULL, NULL) || logical_width != 18 ||
		logical_height != 14)
		return FALSE;
	if (!fabulor_xtext_inline_image_size (100, 3, &logical_width,
		&logical_height, &device_width, &device_height) ||
		logical_width != 85 || logical_height != 64 ||
		device_width != 255 || device_height != 192)
		return FALSE;
	if (!fabulor_xtext_device_to_logical (49, 2, &logical_pixels) ||
		logical_pixels != 25 ||
		fabulor_xtext_device_to_logical (-1, 2, &logical_pixels) ||
		fabulor_xtext_scale_factor (0) != 1 ||
		fabulor_xtext_scale_factor (3) != 3)
		return FALSE;
	fabulor_xtext_decoration_positions (20, 12, 16, &strike_y,
		&underline_y);
	return strike_y == 16 && underline_y == 21;
}

static gboolean
check_xtext_widget_class_policy (void)
{
	GtkWidgetClass *widget_class = g_type_class_ref (
		fabulor_probe_xtext_widget_get_type ());
	gint minimum = 0;
	gint natural = 0;
	gint minimum_baseline = 0;
	gint natural_baseline = 0;

	if (!g_type_is_a (fabulor_probe_xtext_widget_get_type (),
		GTK_TYPE_ACCESSIBLE_TEXT) || !widget_class->realize ||
		!widget_class->unrealize ||
		!widget_class->size_allocate || !widget_class->measure ||
		!widget_class->snapshot ||
		gtk_widget_class_get_accessible_role (widget_class) !=
		GTK_ACCESSIBLE_ROLE_LOG)
	{
		g_type_class_unref (widget_class);
		return FALSE;
	}
	widget_class->measure (NULL, GTK_ORIENTATION_HORIZONTAL, -1, &minimum,
		&natural, &minimum_baseline, &natural_baseline);
	if (minimum != 200 || natural != 200 || minimum_baseline != -1 ||
		natural_baseline != -1)
	{
		g_type_class_unref (widget_class);
		return FALSE;
	}
	fabulor_xtext_widget_measure (GTK_ORIENTATION_HORIZONTAL, &minimum,
		&natural, &minimum_baseline, &natural_baseline);
	if (minimum != 200 || natural != 200 || minimum_baseline != -1 ||
		natural_baseline != -1)
	{
		g_type_class_unref (widget_class);
		return FALSE;
	}
	fabulor_xtext_widget_measure (GTK_ORIENTATION_VERTICAL, &minimum,
		&natural, &minimum_baseline, &natural_baseline);
	g_type_class_unref (widget_class);
	return minimum == 90 && natural == 90 && minimum_baseline == -1 &&
		natural_baseline == -1 &&
		!fabulor_xtext_widget_width_changed (640, 640) &&
		fabulor_xtext_widget_width_changed (640, 800);
}

static gboolean
check_xtext_render_target (void)
{
	FabulorXTextRenderTarget *target = fabulor_xtext_render_target_new ();
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
		16, 16);
	cairo_t *surface_context = NULL;
	cairo_t *copy = NULL;
	GtkSnapshot *snapshot = NULL;
	GskRenderNode *node = NULL;
	gboolean valid = target != NULL &&
		cairo_surface_status (surface) == CAIRO_STATUS_SUCCESS &&
		fabulor_xtext_render_target_create_context (target) == NULL &&
		!fabulor_xtext_render_target_has_active_context (target);

	if (valid)
	{
		fabulor_xtext_render_target_set_surface (target, surface);
		copy = fabulor_xtext_render_target_create_context (target);
		valid = copy != NULL && cairo_status (copy) == CAIRO_STATUS_SUCCESS;
		if (copy)
		{
			cairo_set_source_rgb (copy, 1.0, 0.0, 0.0);
			cairo_paint (copy);
			cairo_destroy (copy);
		}
		fabulor_xtext_render_target_set_surface (target, NULL);
	}
	if (valid)
	{
		surface_context = cairo_create (surface);
		valid = fabulor_xtext_render_target_exchange_context (target,
			surface_context) == NULL &&
			fabulor_xtext_render_target_has_active_context (target);
		copy = fabulor_xtext_render_target_create_context (target);
		valid = valid && copy != NULL &&
			cairo_get_target (copy) == cairo_get_target (surface_context) &&
			fabulor_xtext_render_target_exchange_context (target, NULL) ==
				surface_context &&
			!fabulor_xtext_render_target_has_active_context (target);
		if (copy)
			cairo_destroy (copy);
		cairo_destroy (surface_context);
	}
	if (valid)
	{
		snapshot = gtk_snapshot_new ();
		surface_context = fabulor_xtext_render_target_begin_snapshot (target,
			snapshot, 32, 16);
		valid = surface_context != NULL &&
			fabulor_xtext_render_target_has_active_context (target);
		if (surface_context)
		{
			cairo_set_source_rgb (surface_context, 0.0, 1.0, 0.0);
			cairo_paint (surface_context);
			fabulor_xtext_render_target_end_snapshot (target, surface_context);
		}
		valid = valid &&
			!fabulor_xtext_render_target_has_active_context (target);
		node = gtk_snapshot_free_to_node (snapshot);
		valid = valid && node != NULL;
		if (node)
			gsk_render_node_unref (node);
	}
	cairo_surface_destroy (surface);
	fabulor_xtext_render_target_free (target);
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
	if (!check_editable_list_model ())
	{
		fprintf (stderr, "GTK4 editable list model contract mismatch\n");
		return 1;
	}
	if (!check_print_event_list_model ())
	{
		fprintf (stderr, "GTK4 Print Events list contract mismatch\n");
		return 1;
	}
	if (!check_key_binding_list_model ())
	{
		fprintf (stderr, "GTK4 key-binding list contract mismatch\n");
		return 1;
	}
	if (!check_sound_event_list_model ())
	{
		fprintf (stderr, "GTK4 sound-event list contract mismatch\n");
		return 1;
	}
	if (!check_preferences_category_list_model ())
	{
		fprintf (stderr, "GTK4 Preferences category list contract mismatch\n");
		return 1;
	}
	if (!check_server_network_list_model ())
	{
		fprintf (stderr, "GTK4 server-network list contract mismatch\n");
		return 1;
	}
	if (!check_server_entry_list_model ())
	{
		fprintf (stderr, "GTK4 server-entry list contract mismatch\n");
		return 1;
	}
	if (!check_xtext_render_target ())
	{
		fprintf (stderr, "GTK4 transcript render-target contract mismatch\n");
		return 1;
	}
	if (!check_xtext_background_policy ())
	{
		fprintf (stderr, "GTK4 transcript background contract mismatch\n");
		return 1;
	}
	if (!check_xtext_decoration_policy ())
	{
		fprintf (stderr, "GTK4 transcript decoration contract mismatch\n");
		return 1;
	}
	if (!check_xtext_geometry ())
	{
		fprintf (stderr, "GTK4 transcript geometry contract mismatch\n");
		return 1;
	}
	if (!check_xtext_hit_test_policy ())
	{
		fprintf (stderr, "GTK4 transcript hit-test contract mismatch\n");
		return 1;
	}
	if (!check_xtext_input_policy ())
	{
		fprintf (stderr, "GTK4 transcript input policy mismatch\n");
		return 1;
	}
	if (!check_xtext_selection_policy ())
	{
		fprintf (stderr, "GTK4 transcript selection policy mismatch\n");
		return 1;
	}
	if (!check_xtext_scroll_copy_policy ())
	{
		fprintf (stderr, "GTK4 transcript scroll-copy policy mismatch\n");
		return 1;
	}
	if (!check_spell_entry_word_policy ())
	{
		fprintf (stderr, "GTK4 spell-entry word policy mismatch\n");
		return 1;
	}
	if (!check_xtext_performance_policy ())
	{
		fprintf (stderr, "GTK4 transcript performance policy mismatch\n");
		return 1;
	}
	if (!check_xtext_accessible_policy ())
	{
		fprintf (stderr, "GTK4 transcript accessible-text policy mismatch\n");
		return 1;
	}
	if (!check_xtext_display_policy ())
	{
		fprintf (stderr, "GTK4 transcript display policy mismatch\n");
		return 1;
	}
	if (!check_xtext_widget_class_policy ())
	{
		fprintf (stderr, "GTK4 transcript widget-class contract mismatch\n");
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
