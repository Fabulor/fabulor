#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include <stdio.h>
#include <string.h>

#include "../../src/fe-gtk/gtk-compat.h"
#include "../../src/fe-gtk/file-chooser-path.h"
#include "../../src/fe-gtk/gtk4-list-models.h"
#include "../../src/common/gtk4-theme-discovery.h"
#include "../../src/common/gtk4-theme-preferences.h"
#include "../../src/fe-gtk/emoji-picker.h"
#include "../../src/fe-gtk/theme/theme-gtk3.h"
#include "../../src/fe-gtk/theme/theme-gtk4.h"
#include "../../src/fe-gtk/theme/theme-gtk4-controller.h"
#include "../../src/fe-gtk/theme/theme-preferences-gtk4.h"
#include "../../src/fe-gtk/theme/theme-appearance-monitor-gtk4.h"
#include "../../src/fe-gtk/tray-action-model.h"
#include "../../src/fe-gtk/tray-backend-policy.h"
#include "../../src/fe-gtk/tray-menu-composition.h"
#include "../../src/fe-gtk/tray-menu-presenter-gtk4.h"
#include "../../src/fe-gtk/context-menu-presenter-gtk4.h"
#include "../../src/fe-gtk/url-context-menu-model.h"
#include "../../src/fe-gtk/channel-context-menu-model.h"
#include "../../src/fe-gtk/channel-list-context-menu-model.h"
#include "../../src/fe-gtk/nick-context-menu-model.h"
#include "../../src/fe-gtk/middle-context-menu-model.h"
#include "../../src/fe-gtk/tab-context-menu-model.h"
#include "../../src/fe-gtk/window-state.h"
#include "../../src/fe-gtk/window-geometry.h"
#include "../../src/fe-gtk/application-main-loop.h"
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
#include "../../src/fe-gtk/spell-entry-menu.h"
#include "../../src/fe-gtk/spell-entry-style.h"
#include "../../src/fe-gtk/spell-entry-widget.h"
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
	GtkIconTheme *(*volatile icon_theme_get_default) (void) =
		fabulor_gtk_icon_theme_get_default;
	void (*volatile icon_theme_add_search_path) (GtkIconTheme *, const gchar *) =
		fabulor_gtk_icon_theme_add_search_path;
	void (*volatile icon_theme_set_name) (GtkIconTheme *, const gchar *) =
		fabulor_gtk_icon_theme_set_name;
	GtkWidget *(*volatile dialog_icon_new) (const gchar *) =
		fabulor_gtk_dialog_icon_new;
	gint (*volatile icon_size_get_pixels) (FabulorGtkIconSize) =
		fabulor_gtk_icon_size_get_pixels;
	GtkWidget *(*volatile image_new_from_icon_name) (const gchar *,
		FabulorGtkIconSize) = fabulor_gtk_image_new_from_icon_name;
	GtkWidget *(*volatile button_new_with_icon_and_mnemonic) (const gchar *,
		const gchar *, FabulorGtkIconSize) =
		fabulor_gtk_button_new_with_icon_and_mnemonic;
	GtkWidget *(*volatile radio_button_new_with_mnemonic) (GtkWidget *,
		const gchar *) = fabulor_gtk_radio_button_new_with_mnemonic;
	gboolean (*volatile check_button_get_active) (GtkWidget *) =
		fabulor_gtk_check_button_get_active;
	void (*volatile check_button_set_active) (GtkWidget *, gboolean) =
		fabulor_gtk_check_button_set_active;
	void (*volatile combo_box_set_single_column) (GtkComboBox *) =
		fabulor_gtk_combo_box_set_single_column;
	void (*volatile label_set_wrap) (GtkLabel *, gboolean) =
		fabulor_gtk_label_set_wrap;
	const gchar *(*volatile entry_get_text) (GtkEntry *) =
		fabulor_gtk_entry_get_text;
	void (*volatile entry_set_text) (GtkEntry *, const gchar *) =
		fabulor_gtk_entry_set_text;
	void (*volatile entry_set_width_chars) (GtkEntry *, gint) =
		fabulor_gtk_entry_set_width_chars;
	GtkWidget *(*volatile window_new) (void) = fabulor_gtk_window_new;
	gboolean (*volatile file_chooser_set_current_folder_path) (GtkFileChooser *,
		const gchar *) = fabulor_gtk_file_chooser_set_current_folder_path;
	void (*volatile file_chooser_set_overwrite_confirmation) (GtkFileChooser *,
		gboolean) = fabulor_gtk_file_chooser_set_overwrite_confirmation;
	gchar *(*volatile file_chooser_dup_filename) (GtkFileChooser *) =
		fabulor_gtk_file_chooser_dup_filename;
	gchar *(*volatile file_chooser_dup_current_folder_path) (GtkFileChooser *) =
		fabulor_gtk_file_chooser_dup_current_folder_path;
	GSList *(*volatile file_chooser_dup_filenames) (GtkFileChooser *) =
		fabulor_gtk_file_chooser_dup_filenames;
	GtkWidget *(*volatile button_box_new) (GtkOrientation,
		FabulorGtkButtonBoxLayout, gint) = fabulor_gtk_button_box_new;
	void (*volatile box_append) (GtkBox *, GtkWidget *, gboolean, gboolean, guint) =
		fabulor_gtk_box_append;
	void (*volatile horizontal_box_append_trailing) (GtkBox *, GtkWidget *) =
		fabulor_gtk_horizontal_box_append_trailing;
	void (*volatile box_insert_before_trailing) (GtkBox *, GtkWidget *, GtkWidget *) =
		fabulor_gtk_box_insert_before_trailing;
	void (*volatile box_append_trailing_pair) (GtkBox *, GtkWidget *, GtkWidget *) =
		fabulor_gtk_box_append_trailing_pair;
	void (*volatile box_reorder_child) (GtkBox *, GtkWidget *, gint) =
		fabulor_gtk_box_reorder_child;
	void (*volatile box_remove_child) (GtkBox *, GtkWidget *) =
		fabulor_gtk_box_remove_child;
	GtkWidget *(*volatile content_surface_new) (gboolean) =
		fabulor_gtk_content_surface_new;
	void (*volatile content_surface_set_child) (GtkWidget *, GtkWidget *) =
		fabulor_gtk_content_surface_set_child;
	void (*volatile list_box_row_set_child) (GtkListBoxRow *, GtkWidget *) =
		fabulor_gtk_list_box_row_set_child;
	void (*volatile list_box_append) (GtkListBox *, GtkWidget *) =
		fabulor_gtk_list_box_append;
	void (*volatile container_set_uniform_inset) (GtkWidget *, guint) =
		fabulor_gtk_container_set_uniform_inset;
	void (*volatile copy_text_to_clipboards) (GtkWidget *, const gchar *) =
		fabulor_gtk_copy_text_to_clipboards;
	void (*volatile widget_add_css_class) (GtkWidget *, const gchar *) =
		fabulor_gtk_widget_add_css_class;
	PangoFontDescription *(*volatile widget_dup_font_description) (GtkWidget *) =
		fabulor_gtk_widget_dup_font_description;
	void (*volatile button_set_flat) (GtkButton *) =
		fabulor_gtk_button_set_flat;
	void (*volatile button_set_always_show_image) (GtkButton *, gboolean) =
		fabulor_gtk_button_set_always_show_image;
	GtkWidget *(*volatile icon_button_new) (const gchar *) =
		fabulor_gtk_icon_button_new;
	void (*volatile widget_set_accessible_label) (GtkWidget *, const gchar *) =
		fabulor_gtk_widget_set_accessible_label;
	void (*volatile widget_queue_draw_region) (GtkWidget *, gint, gint,
		gint, gint) = fabulor_gtk_widget_queue_draw_region;
	gboolean (*volatile widget_has_toplevel_focus) (GtkWidget *) =
		fabulor_gtk_widget_has_toplevel_focus;
	GtkWindow *(*volatile widget_get_root_window) (GtkWidget *) =
		fabulor_gtk_widget_get_root_window;
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
	gboolean (*volatile window_minimize) (GtkWindow *) =
		fabulor_gtk_window_minimize;
	void (*volatile window_set_urgent) (GtkWindow *, gboolean) =
		fabulor_gtk_window_set_urgent;
	void (*volatile window_set_wm_class) (GtkWindow *, const gchar *,
		const gchar *) = fabulor_gtk_window_set_wm_class;
	void (*volatile window_resize) (GtkWindow *, gint, gint) =
		fabulor_gtk_window_resize;
	void (*volatile window_position_at_pointer) (GtkWindow *) =
		fabulor_gtk_window_position_at_pointer;
	void (*volatile window_position_center) (GtkWindow *) =
		fabulor_gtk_window_position_center;
	void (*volatile window_position_center_on_parent) (GtkWindow *) =
		fabulor_gtk_window_position_center_on_parent;
	void (*volatile window_move) (GtkWindow *, gint, gint) =
		fabulor_gtk_window_move;
	gboolean (*volatile window_get_position) (GtkWindow *, gint *, gint *) =
		fabulor_gtk_window_get_position;
	void (*volatile window_set_dialog_hint) (GtkWindow *) =
		fabulor_gtk_window_set_dialog_hint;
	void (*volatile window_placement_capture) (GtkWindow *,
		FabulorGtkWindowPlacement *) = fabulor_gtk_window_placement_capture;
	void (*volatile window_placement_restore) (GtkWindow *,
		const FabulorGtkWindowPlacement *) = fabulor_gtk_window_placement_restore;
	void (*volatile scrolled_window_set_child) (GtkScrolledWindow *, GtkWidget *) =
		fabulor_gtk_scrolled_window_set_child;
	void (*volatile scrolled_window_set_framed) (GtkScrolledWindow *, gboolean) =
		fabulor_gtk_scrolled_window_set_framed;
	void (*volatile paned_set_start_child) (GtkPaned *, GtkWidget *, gboolean,
		gboolean) = fabulor_gtk_paned_set_start_child;
	void (*volatile paned_set_end_child) (GtkPaned *, GtkWidget *, gboolean,
		gboolean) = fabulor_gtk_paned_set_end_child;
	gboolean (*volatile layout_retain_and_detach_child) (GtkWidget *) =
		fabulor_gtk_layout_retain_and_detach_child;
	void (*volatile frame_set_child) (GtkFrame *, GtkWidget *) =
		fabulor_gtk_frame_set_child;
	void (*volatile frame_set_outlined) (GtkFrame *) =
		fabulor_gtk_frame_set_outlined;
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

	(void) icon_theme_get_default;
	(void) icon_theme_add_search_path;
	(void) icon_theme_set_name;
	(void) dialog_icon_new;
	(void) icon_size_get_pixels;
	(void) image_new_from_icon_name;
	(void) button_new_with_icon_and_mnemonic;
	(void) radio_button_new_with_mnemonic;
	(void) check_button_get_active;
	(void) check_button_set_active;
	(void) combo_box_set_single_column;
	(void) label_set_wrap;
	(void) entry_get_text;
	(void) entry_set_text;
	(void) entry_set_width_chars;
	(void) window_new;
	(void) file_chooser_set_current_folder_path;
	(void) file_chooser_set_overwrite_confirmation;
	(void) file_chooser_dup_filename;
	(void) file_chooser_dup_current_folder_path;
	(void) file_chooser_dup_filenames;
	(void) button_box_new;
	(void) box_append;
	(void) horizontal_box_append_trailing;
	(void) box_insert_before_trailing;
	(void) box_append_trailing_pair;
	(void) box_reorder_child;
	(void) box_remove_child;
	(void) content_surface_new;
	(void) content_surface_set_child;
	(void) list_box_row_set_child;
	(void) list_box_append;
	(void) container_set_uniform_inset;
	(void) copy_text_to_clipboards;
	(void) widget_add_css_class;
	(void) widget_dup_font_description;
	(void) button_set_flat;
	(void) button_set_always_show_image;
	(void) icon_button_new;
	(void) widget_set_accessible_label;
	(void) widget_queue_draw_region;
	(void) widget_has_toplevel_focus;
	(void) widget_get_root_window;
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
	(void) window_minimize;
	(void) window_set_urgent;
	(void) window_set_wm_class;
	(void) window_resize;
	(void) window_position_at_pointer;
	(void) window_position_center;
	(void) window_position_center_on_parent;
	(void) window_move;
	(void) window_get_position;
	(void) window_set_dialog_hint;
	(void) window_placement_capture;
	(void) window_placement_restore;
	(void) scrolled_window_set_child;
	(void) scrolled_window_set_framed;
	(void) paned_set_start_child;
	(void) paned_set_end_child;
	(void) layout_retain_and_detach_child;
	(void) frame_set_child;
	(void) frame_set_outlined;
	(void) button_set_child;
	(void) overlay_set_child;
	(void) popover_set_child;
	(void) widget_reveal_tree;
	(void) window_destroy;
	(void) dialog_destroy_on_response;
}

static gboolean
check_dialog_icon (gboolean gtk_ready)
{
	GtkWidget *image;
	gboolean valid;

	if (!gtk_ready)
		return TRUE;

	image = fabulor_gtk_dialog_icon_new ("dialog-warning");
	g_object_ref_sink (image);
	valid = GTK_IS_IMAGE (image) &&
		gtk_image_get_pixel_size (GTK_IMAGE (image)) ==
			FABULOR_GTK_DIALOG_ICON_PIXEL_SIZE &&
		g_strcmp0 (gtk_image_get_icon_name (GTK_IMAGE (image)),
			"dialog-warning") == 0;
	g_object_unref (image);
	return valid;
}

static gboolean
check_icon_sizes (gboolean gtk_ready)
{
	GtkWidget *menu_image;
	GtkWidget *toolbar_image;
	gboolean valid;

	if (!gtk_ready)
		return TRUE;

	menu_image = fabulor_gtk_image_new_from_icon_name (
		"window-close-symbolic", FABULOR_GTK_ICON_SIZE_MENU);
	toolbar_image = fabulor_gtk_image_new_from_icon_name (
		"network-workgroup", FABULOR_GTK_ICON_SIZE_LARGE_TOOLBAR);
	g_object_ref_sink (menu_image);
	g_object_ref_sink (toolbar_image);
	valid = gtk_image_get_pixel_size (GTK_IMAGE (menu_image)) == 16 &&
		gtk_image_get_pixel_size (GTK_IMAGE (toolbar_image)) == 24 &&
		fabulor_gtk_icon_size_get_pixels (FABULOR_GTK_ICON_SIZE_MENU) == 16 &&
		fabulor_gtk_icon_size_get_pixels (
			FABULOR_GTK_ICON_SIZE_LARGE_TOOLBAR) == 24;
	g_object_unref (toolbar_image);
	g_object_unref (menu_image);
	return valid;
}

static gboolean
check_icon_theme_compatibility (gboolean gtk_ready)
{
	GtkIconTheme *theme = gtk_icon_theme_new ();
	char **search_path;
	char *theme_name;
	gboolean found_path = FALSE;
	gboolean passed;
	guint i;

	fabulor_gtk_icon_theme_add_search_path (theme, g_get_tmp_dir ());
	fabulor_gtk_icon_theme_set_name (theme, "Adwaita");
	search_path = gtk_icon_theme_get_search_path (theme);
	theme_name = gtk_icon_theme_get_theme_name (theme);
	for (i = 0; search_path && search_path[i]; i++)
	{
		if (g_strcmp0 (search_path[i], g_get_tmp_dir ()) == 0)
			found_path = TRUE;
	}
	passed = found_path && g_strcmp0 (theme_name, "Adwaita") == 0 &&
		(!gtk_ready || fabulor_gtk_icon_theme_get_default () != NULL);
	g_strfreev (search_path);
	g_free (theme_name);
	g_object_unref (theme);
	return passed;
}

static gboolean
check_button_box_layouts (gboolean gtk_ready)
{
	GtkWidget *spread;
	GtkWidget *end;
	GtkWidget *start;
	gboolean valid;

	if (!gtk_ready)
		return TRUE;

	spread = fabulor_gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL,
		FABULOR_GTK_BUTTON_BOX_SPREAD, 4);
	end = fabulor_gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL,
		FABULOR_GTK_BUTTON_BOX_END, 3);
	start = fabulor_gtk_button_box_new (GTK_ORIENTATION_VERTICAL,
		FABULOR_GTK_BUTTON_BOX_START, 2);
	g_object_ref_sink (spread);
	g_object_ref_sink (end);
	g_object_ref_sink (start);

	valid = GTK_IS_BOX (spread) && GTK_IS_BOX (end) && GTK_IS_BOX (start) &&
		gtk_orientable_get_orientation (GTK_ORIENTABLE (spread)) ==
			GTK_ORIENTATION_HORIZONTAL &&
		gtk_box_get_homogeneous (GTK_BOX (spread)) &&
		gtk_box_get_spacing (GTK_BOX (spread)) == 4 &&
		gtk_widget_get_halign (end) == GTK_ALIGN_END &&
		gtk_box_get_spacing (GTK_BOX (end)) == 3 &&
		gtk_orientable_get_orientation (GTK_ORIENTABLE (start)) ==
			GTK_ORIENTATION_VERTICAL &&
		gtk_widget_get_valign (start) == GTK_ALIGN_START &&
		gtk_box_get_spacing (GTK_BOX (start)) == 2;

	g_object_unref (start);
	g_object_unref (end);
	g_object_unref (spread);
	return valid;
}

static gboolean
check_entry_text (gboolean gtk_ready)
{
	GtkWidget *entry;
	gboolean valid;

	if (!gtk_ready)
		return TRUE;

	entry = gtk_entry_new ();
	g_object_ref_sink (entry);
	fabulor_gtk_entry_set_text (GTK_ENTRY (entry), "channel search");
	fabulor_gtk_entry_set_width_chars (GTK_ENTRY (entry), 12);
	valid = g_strcmp0 (fabulor_gtk_entry_get_text (GTK_ENTRY (entry)),
		"channel search") == 0 &&
		gtk_editable_get_width_chars (GTK_EDITABLE (entry)) == 12;
	g_object_unref (entry);
	return valid;
}

static gboolean
check_container_uniform_insets (gboolean gtk_ready)
{
	GtkWidget *window_child;
	GtkWidget *window;
	GtkWidget *box;
	gboolean valid;

	if (!gtk_ready)
		return TRUE;

	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	g_object_ref_sink (box);
	fabulor_gtk_container_set_uniform_inset (box, 4);
	valid = gtk_widget_get_margin_start (box) == 4 &&
		gtk_widget_get_margin_end (box) == 4 &&
		gtk_widget_get_margin_top (box) == 4 &&
		gtk_widget_get_margin_bottom (box) == 4;
	g_object_unref (box);

	window = fabulor_gtk_window_new ();
	window_child = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	g_object_ref_sink (window);
	fabulor_gtk_container_set_uniform_inset (window, 7);
	fabulor_gtk_window_set_child (GTK_WINDOW (window), window_child);
	valid = valid && gtk_widget_get_margin_start (window_child) == 7 &&
		gtk_widget_get_margin_end (window_child) == 7 &&
		gtk_widget_get_margin_top (window_child) == 7 &&
		gtk_widget_get_margin_bottom (window_child) == 7;
	fabulor_gtk_container_set_uniform_inset (window, 0);
	valid = valid && gtk_widget_get_margin_start (window_child) == 0 &&
		gtk_widget_get_margin_end (window_child) == 0 &&
		gtk_widget_get_margin_top (window_child) == 0 &&
		gtk_widget_get_margin_bottom (window_child) == 0;
	gtk_window_destroy (GTK_WINDOW (window));
	g_object_unref (window);
	return valid;
}

static gboolean
check_content_and_list_ownership (gboolean gtk_ready)
{
	GtkWidget *transparent_surface;
	GtkWidget *visible_surface;
	GtkWidget *transparent_child;
	GtkWidget *visible_child;
	GtkWidget *list;
	GtkWidget *row;
	GtkWidget *row_child;
	gboolean valid;

	if (!gtk_ready)
		return TRUE;

	transparent_surface = fabulor_gtk_content_surface_new (FALSE);
	visible_surface = fabulor_gtk_content_surface_new (TRUE);
	transparent_child = gtk_label_new ("meter");
	visible_child = gtk_label_new ("preview");
	g_object_ref_sink (transparent_surface);
	g_object_ref_sink (visible_surface);
	fabulor_gtk_content_surface_set_child (transparent_surface,
		transparent_child);
	fabulor_gtk_content_surface_set_child (visible_surface, visible_child);
	valid = GTK_IS_BOX (transparent_surface) && GTK_IS_BOX (visible_surface) &&
		gtk_widget_get_parent (transparent_child) == transparent_surface &&
		gtk_widget_get_parent (visible_child) == visible_surface;
	g_object_unref (visible_surface);
	g_object_unref (transparent_surface);

	list = gtk_list_box_new ();
	row = gtk_list_box_row_new ();
	row_child = gtk_label_new ("row");
	g_object_ref_sink (list);
	fabulor_gtk_list_box_row_set_child (GTK_LIST_BOX_ROW (row), row_child);
	fabulor_gtk_list_box_append (GTK_LIST_BOX (list), row);
	valid = valid && gtk_list_box_row_get_child (GTK_LIST_BOX_ROW (row)) ==
		row_child && gtk_widget_get_parent (row) == list &&
		gtk_widget_get_first_child (list) == row;
	g_object_unref (list);
	return valid;
}

static gboolean
check_box_reorder_ownership (gboolean gtk_ready)
{
	GtkWidget *box;
	GtkWidget *first;
	GtkWidget *second;
	GtkWidget *third;
	gboolean valid;

	if (!gtk_ready)
		return TRUE;

	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	first = gtk_label_new ("first");
	second = gtk_label_new ("second");
	third = gtk_label_new ("third");
	g_object_ref_sink (box);
	gtk_box_append (GTK_BOX (box), first);
	gtk_box_append (GTK_BOX (box), second);
	gtk_box_append (GTK_BOX (box), third);

	fabulor_gtk_box_reorder_child (GTK_BOX (box), third, 0);
	valid = gtk_widget_get_first_child (box) == third &&
		gtk_widget_get_next_sibling (third) == first &&
		gtk_widget_get_next_sibling (first) == second;
	fabulor_gtk_box_reorder_child (GTK_BOX (box), third, 1);
	valid = valid && gtk_widget_get_first_child (box) == first &&
		gtk_widget_get_next_sibling (first) == third &&
		gtk_widget_get_next_sibling (third) == second;
	fabulor_gtk_box_reorder_child (GTK_BOX (box), third, 2);
	valid = valid && gtk_widget_get_first_child (box) == first &&
		gtk_widget_get_next_sibling (first) == second &&
		gtk_widget_get_next_sibling (second) == third;

	g_object_unref (box);
	return valid;
}

static gboolean
check_layout_reparent_ownership (gboolean gtk_ready)
{
	GtkWidget *paned;
	GtkWidget *paned_child;
	GtkWidget *grid;
	GtkWidget *grid_child;
	GtkWidget *unparented;
	gboolean valid;

	if (!gtk_ready)
		return TRUE;

	paned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	paned_child = gtk_label_new ("paned child");
	g_object_ref_sink (paned);
	fabulor_gtk_paned_set_start_child (GTK_PANED (paned), paned_child,
		FALSE, TRUE);
	valid = fabulor_gtk_layout_retain_and_detach_child (paned_child) &&
		gtk_widget_get_parent (paned_child) == NULL;
	fabulor_gtk_paned_set_end_child (GTK_PANED (paned), paned_child,
		FALSE, TRUE);
	valid = valid && gtk_widget_get_parent (paned_child) == paned;
	g_object_unref (paned_child);
	g_object_unref (paned);

	grid = gtk_grid_new ();
	grid_child = gtk_label_new ("grid child");
	g_object_ref_sink (grid);
	gtk_grid_attach (GTK_GRID (grid), grid_child, 0, 0, 1, 1);
	valid = valid &&
		fabulor_gtk_layout_retain_and_detach_child (grid_child) &&
		gtk_widget_get_parent (grid_child) == NULL;
	gtk_grid_attach (GTK_GRID (grid), grid_child, 0, 0, 1, 1);
	valid = valid && gtk_widget_get_parent (grid_child) == grid;
	g_object_unref (grid_child);
	g_object_unref (grid);

	unparented = gtk_label_new ("unparented");
	g_object_ref_sink (unparented);
	valid = valid &&
		!fabulor_gtk_layout_retain_and_detach_child (unparented);
	g_object_unref (unparented);
	return valid;
}

static gboolean
check_flat_button (gboolean gtk_ready)
{
	GtkWidget *button;
	gboolean valid;

	if (!gtk_ready)
		return TRUE;

	button = gtk_button_new ();
	g_object_ref_sink (button);
	fabulor_gtk_button_set_flat (GTK_BUTTON (button));
	valid = gtk_widget_has_css_class (button, "flat");
	g_object_unref (button);
	return valid;
}

static gboolean
check_icon_button (gboolean gtk_ready)
{
	GtkWidget *button;
	GtkWidget *child;
	gboolean valid;

	if (!gtk_ready)
		return TRUE;

	button = fabulor_gtk_icon_button_new ("go-bottom-symbolic");
	g_object_ref_sink (button);
	fabulor_gtk_widget_set_accessible_label (button, "Scroll to bottom");
	child = gtk_button_get_child (GTK_BUTTON (button));
	valid = GTK_IS_IMAGE (child) &&
		g_strcmp0 (gtk_image_get_icon_name (GTK_IMAGE (child)),
			"go-bottom-symbolic") == 0;
	g_object_unref (button);
	return valid;
}

static gboolean
check_icon_mnemonic_button (gboolean gtk_ready)
{
	GtkWidget *button;
	GtkWidget *box;
	GtkWidget *image;
	GtkWidget *label;
	gboolean valid;

	if (!gtk_ready)
		return TRUE;

	button = fabulor_gtk_button_new_with_icon_and_mnemonic ("Go _Now",
		"go-bottom-symbolic", FABULOR_GTK_ICON_SIZE_MENU);
	g_object_ref_sink (button);
	box = gtk_button_get_child (GTK_BUTTON (button));
	image = GTK_IS_BOX (box) ? gtk_widget_get_first_child (box) : NULL;
	label = image ? gtk_widget_get_next_sibling (image) : NULL;
	valid = GTK_IS_IMAGE (image) && GTK_IS_LABEL (label) &&
		gtk_widget_get_next_sibling (label) == NULL &&
		g_strcmp0 (gtk_image_get_icon_name (GTK_IMAGE (image)),
			"go-bottom-symbolic") == 0 &&
		gtk_image_get_pixel_size (GTK_IMAGE (image)) ==
			FABULOR_GTK_ICON_SIZE_MENU &&
		g_strcmp0 (gtk_label_get_text (GTK_LABEL (label)), "Go Now") == 0 &&
		gtk_label_get_mnemonic_widget (GTK_LABEL (label)) == button;
	g_object_unref (button);
	return valid;
}

static gboolean
check_choice_buttons_and_root_window (gboolean gtk_ready)
{
	GtkWidget *window;
	GtkWidget *box;
	GtkWidget *first;
	GtkWidget *second;
	GtkWidget *third;
	GtkWidget *check;
	GtkWidget *label;
	GtkWidget *unparented;
	gboolean valid;

	if (!gtk_ready)
		return TRUE;

	window = gtk_window_new ();
	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	first = fabulor_gtk_radio_button_new_with_mnemonic (NULL, "_First");
	second = fabulor_gtk_radio_button_new_with_mnemonic (first, "_Second");
	third = fabulor_gtk_radio_button_new_with_mnemonic (first, "_Third");
	check = gtk_check_button_new_with_mnemonic ("_Remember");
	label = gtk_label_new ("A wrapped label");
	unparented = gtk_label_new ("Unparented");
	g_object_ref_sink (window);
	g_object_ref_sink (unparented);
	gtk_box_append (GTK_BOX (box), first);
	gtk_box_append (GTK_BOX (box), second);
	gtk_box_append (GTK_BOX (box), third);
	gtk_box_append (GTK_BOX (box), check);
	gtk_box_append (GTK_BOX (box), label);
	fabulor_gtk_window_set_child (GTK_WINDOW (window), box);

	valid = fabulor_gtk_check_button_get_active (first) &&
		!fabulor_gtk_check_button_get_active (second) &&
		!fabulor_gtk_check_button_get_active (third);
	fabulor_gtk_check_button_set_active (second, TRUE);
	valid = valid && !fabulor_gtk_check_button_get_active (first) &&
		fabulor_gtk_check_button_get_active (second) &&
		!fabulor_gtk_check_button_get_active (third);
	fabulor_gtk_check_button_set_active (check, TRUE);
	valid = valid && fabulor_gtk_check_button_get_active (check);
	fabulor_gtk_label_set_wrap (GTK_LABEL (label), TRUE);
	valid = valid && gtk_label_get_wrap (GTK_LABEL (label)) &&
		fabulor_gtk_widget_get_root_window (second) == GTK_WINDOW (window) &&
		fabulor_gtk_widget_get_root_window (unparented) == NULL;

	g_object_unref (unparented);
	gtk_window_destroy (GTK_WINDOW (window));
	g_object_unref (window);
	return valid;
}

static gboolean
check_frame_presentation (gboolean gtk_ready)
{
	GtkWidget *scroller;
	GtkWidget *frame;
	gboolean valid;

	if (!gtk_ready)
		return TRUE;

	scroller = gtk_scrolled_window_new ();
	frame = gtk_frame_new (NULL);
	g_object_ref_sink (scroller);
	g_object_ref_sink (frame);
	fabulor_gtk_scrolled_window_set_framed (
		GTK_SCROLLED_WINDOW (scroller), TRUE);
	valid = gtk_widget_has_css_class (scroller, "frame");
	fabulor_gtk_scrolled_window_set_framed (
		GTK_SCROLLED_WINDOW (scroller), FALSE);
	valid = valid && !gtk_widget_has_css_class (scroller, "frame");
	fabulor_gtk_frame_set_outlined (GTK_FRAME (frame));
	g_object_unref (frame);
	g_object_unref (scroller);
	return valid;
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
	offsets_t first;
	offsets_t second;
	GList *marks = NULL;
	GList *current;
	gint entry_a = 1;
	gint entry_b = 2;
	gint marker_y = -1;
	gboolean valid;

	first.u = 0;
	second.u = 0;
	valid = decoration != NULL &&
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
check_emoji_picker_policy (void)
{
	static const gunichar items[] = { 0x1F600, 0 };
	GtkPopover *(*volatile popover_get) (GtkEntry *) =
		fabulor_emoji_picker_popover_get;
	GtkPopover *(*volatile popover_ensure) (GtkEntry *) =
		fabulor_emoji_picker_popover_ensure;
	FabulorEmojiPickerPage *page;
	FabulorEmojiPickerPage *flags_page;
	gchar *eu;
	gchar *lowercase_eu;
	gchar *grinning;
	gboolean valid;

	page = fabulor_emoji_picker_page_new (items, FALSE);
	flags_page = fabulor_emoji_picker_page_new (NULL, TRUE);
	eu = fabulor_emoji_picker_flag_sequence ("EU");
	lowercase_eu = fabulor_emoji_picker_flag_sequence ("eu");
	grinning = fabulor_emoji_picker_codepoint_sequence (0x1F600);
	valid = popover_get != NULL && popover_ensure != NULL &&
		page != NULL && flags_page != NULL &&
		fabulor_emoji_picker_page_items (page) == items &&
		!fabulor_emoji_picker_page_has_flags (page) &&
		fabulor_emoji_picker_page_items (flags_page) == NULL &&
		fabulor_emoji_picker_page_has_flags (flags_page) &&
		fabulor_emoji_picker_page_claim_load (page) &&
		!fabulor_emoji_picker_page_claim_load (page) &&
		g_strcmp0 (eu, "\xF0\x9F\x87\xAA\xF0\x9F\x87\xBA") == 0 &&
		g_strcmp0 (lowercase_eu, eu) == 0 &&
		g_strcmp0 (grinning, "\xF0\x9F\x98\x80") == 0 &&
		fabulor_emoji_picker_flag_sequence (NULL) == NULL &&
		fabulor_emoji_picker_flag_sequence ("E") == NULL &&
		fabulor_emoji_picker_flag_sequence ("EUU") == NULL &&
		fabulor_emoji_picker_flag_sequence ("E1") == NULL &&
		fabulor_emoji_picker_codepoint_sequence (0) == NULL &&
		fabulor_emoji_picker_codepoint_sequence (0xD800) == NULL;

	g_free (eu);
	g_free (lowercase_eu);
	g_free (grinning);
	fabulor_emoji_picker_page_free (page);
	fabulor_emoji_picker_page_free (flags_page);
	return valid;
}

static void
probe_remove_tree (const char *path)
{
	GDir *dir;
	const char *name;

	if (!path || !g_file_test (path, G_FILE_TEST_EXISTS))
		return;
	if (!g_file_test (path, G_FILE_TEST_IS_DIR))
	{
		g_remove (path);
		return;
	}
	dir = g_dir_open (path, 0, NULL);
	if (dir)
	{
		while ((name = g_dir_read_name (dir)) != NULL)
		{
			char *child = g_build_filename (path, name, NULL);
			probe_remove_tree (child);
			g_free (child);
		}
		g_dir_close (dir);
	}
	g_rmdir (path);
}

static char *
probe_make_gtk4_theme (const char *base, const char *directory_name,
	const char *display_name, gboolean dark)
{
	char *root = g_build_filename (base, directory_name, NULL);
	char *gtk_dir = g_build_filename (root, "gtk-4.0", NULL);
	char *css = g_build_filename (gtk_dir, "gtk.css", NULL);
	char *index = g_build_filename (root, "index.theme", NULL);
	char *index_contents = g_strdup_printf (
		"[Desktop Entry]\nName=%s\n", display_name);

	g_mkdir_with_parents (gtk_dir, 0700);
	g_file_set_contents (css, "window { color: #123456; }\n", -1, NULL);
	g_file_set_contents (index, index_contents, -1, NULL);
	if (dark)
	{
		char *dark_css = g_build_filename (gtk_dir, "gtk-dark.css", NULL);
		g_file_set_contents (dark_css, "window { color: #eeeeee; }\n", -1, NULL);
		g_free (dark_css);
	}
	g_free (index_contents);
	g_free (index);
	g_free (css);
	g_free (gtk_dir);
	return root;
}

static gboolean
check_gtk3_theme_adapter_containment (void)
{
	GError *error = NULL;
	gboolean valid;

	theme_gtk3_init ();
	valid = !theme_gtk3_is_active () &&
		theme_gtk3_apply_current (&error) && error == NULL &&
		theme_gtk3_apply ("legacy-theme", THEME_GTK3_VARIANT_PREFER_DARK,
			&error) && error == NULL &&
		theme_gtk3_refresh ("legacy-theme", THEME_GTK3_VARIANT_PREFER_LIGHT,
			&error) && error == NULL &&
		theme_gtk3_variant_for_theme ("legacy-theme") ==
			THEME_GTK3_VARIANT_PREFER_LIGHT &&
		!theme_gtk3_is_active ();
	theme_gtk3_invalidate_provider_cache ();
	theme_gtk3_disable ();
	valid = valid && !theme_gtk3_is_active ();
	g_clear_error (&error);
	return valid;
}

static gboolean
check_gtk4_theme_discovery_policy (void)
{
	GError *error = NULL;
	char *temporary = g_dir_make_tmp ("fabulor-gtk4-theme-probe-XXXXXX", &error);
	char *desktop_root;
	char *profile_root;
	char *config_root;
	char *profile_from_config;
	char *desktop_theme;
	char *profile_theme;
	char *gtk3_only;
	char *gtk3_dir;
	char *gtk3_css;
	const char *desktop_roots[3];
	GPtrArray *themes;
	FabulorGtk4Theme *first;
	FabulorGtk4Theme *second;
	gboolean valid;

	if (!temporary)
	{
		g_clear_error (&error);
		return FALSE;
	}
	desktop_root = g_build_filename (temporary, "desktop", NULL);
	profile_root = g_build_filename (temporary, "profile", NULL);
	config_root = g_build_filename (temporary, "config", NULL);
	g_mkdir_with_parents (desktop_root, 0700);
	g_mkdir_with_parents (profile_root, 0700);
	desktop_theme = probe_make_gtk4_theme (desktop_root, "zulu", "Zulu", TRUE);
	profile_theme = probe_make_gtk4_theme (profile_root, "alpha", "Alpha", FALSE);
	gtk3_only = g_build_filename (desktop_root, "legacy", NULL);
	gtk3_dir = g_build_filename (gtk3_only, "gtk-3.0", NULL);
	gtk3_css = g_build_filename (gtk3_dir, "gtk.css", NULL);
	g_mkdir_with_parents (gtk3_dir, 0700);
	g_file_set_contents (gtk3_css, "window { color: #000000; }\n", -1, NULL);

	desktop_roots[0] = desktop_root;
	desktop_roots[1] = desktop_root;
	desktop_roots[2] = NULL;
	themes = fabulor_gtk4_theme_discover_roots (profile_root, desktop_roots);
	profile_from_config = fabulor_gtk4_theme_profile_dir (config_root);
	first = themes->len > 0 ? g_ptr_array_index (themes, 0) : NULL;
	second = themes->len > 1 ? g_ptr_array_index (themes, 1) : NULL;
	valid = themes->len == 2 && first && second &&
		g_strcmp0 (first->display_name, "Alpha") == 0 &&
		first->source == FABULOR_GTK4_THEME_SOURCE_PROFILE &&
		g_str_has_prefix (first->id, "profile:") &&
		g_strcmp0 (first->path, profile_theme) == 0 &&
		first->dark_css_path == NULL &&
		g_strcmp0 (second->display_name, "Zulu") == 0 &&
		second->source == FABULOR_GTK4_THEME_SOURCE_DESKTOP &&
		g_str_has_prefix (second->id, "desktop:") &&
		g_strcmp0 (second->path, desktop_theme) == 0 &&
		second->dark_css_path != NULL &&
		g_str_has_suffix (second->css_path, "gtk-4.0" G_DIR_SEPARATOR_S "gtk.css") &&
		g_str_has_suffix (profile_from_config, "config" G_DIR_SEPARATOR_S "themes") &&
		fabulor_gtk4_theme_profile_dir (NULL) == NULL;

	g_ptr_array_unref (themes);
	g_free (profile_from_config);
	g_free (gtk3_css);
	g_free (gtk3_dir);
	g_free (gtk3_only);
	g_free (profile_theme);
	g_free (desktop_theme);
	g_free (config_root);
	g_free (profile_root);
	g_free (desktop_root);
	probe_remove_tree (temporary);
	g_free (temporary);
	return valid;
}

static gboolean
check_gtk4_theme_preferences_policy (void)
{
	GPtrArray *themes = g_ptr_array_new_with_free_func (
		(GDestroyNotify) fabulor_gtk4_theme_free);
	FabulorGtk4Theme *profile = g_new0 (FabulorGtk4Theme, 1);
	FabulorGtk4Theme *desktop = g_new0 (FabulorGtk4Theme, 1);
	GPtrArray *choices;
	FabulorGtk4ThemeChoice *system_choice;
	FabulorGtk4ThemeChoice *profile_choice;
	FabulorGtk4ThemeChoice *desktop_choice;
	FabulorGtk4ThemeAppearanceDecision appearance;
	gboolean available = FALSE;
	gboolean valid;

	profile->id = g_strdup ("profile:alpha");
	profile->display_name = g_strdup ("Alpha");
	profile->source = FABULOR_GTK4_THEME_SOURCE_PROFILE;
	desktop->id = g_strdup ("desktop:zulu");
	desktop->display_name = g_strdup ("Zulu");
	desktop->dark_css_path = g_strdup ("gtk-dark.css");
	desktop->source = FABULOR_GTK4_THEME_SOURCE_DESKTOP;
	g_ptr_array_add (themes, profile);
	g_ptr_array_add (themes, desktop);

	choices = fabulor_gtk4_theme_preferences_project (themes);
	g_ptr_array_unref (themes);
	system_choice = choices->len > 0 ? g_ptr_array_index (choices, 0) : NULL;
	profile_choice = choices->len > 1 ? g_ptr_array_index (choices, 1) : NULL;
	desktop_choice = choices->len > 2 ? g_ptr_array_index (choices, 2) : NULL;
	valid = choices->len == 3 && system_choice && profile_choice && desktop_choice &&
		system_choice->system_default && system_choice->id[0] == '\0' &&
		system_choice->display_name == NULL &&
		!profile_choice->system_default &&
		g_strcmp0 (profile_choice->id, "profile:alpha") == 0 &&
		g_strcmp0 (profile_choice->display_name, "Alpha") == 0 &&
		profile_choice->source == FABULOR_GTK4_THEME_SOURCE_PROFILE &&
		!profile_choice->has_dark_variant &&
		desktop_choice->source == FABULOR_GTK4_THEME_SOURCE_DESKTOP &&
		desktop_choice->has_dark_variant &&
		fabulor_gtk4_theme_preferences_resolve_index (choices,
			"desktop:zulu", &available) == 2 && available &&
		fabulor_gtk4_theme_preferences_resolve_index (choices,
			"desktop:missing", &available) == 0 && !available &&
		fabulor_gtk4_theme_preferences_resolve_index (choices,
			"", &available) == 0 && available &&
		fabulor_gtk4_theme_preferences_normalize_variant (0) ==
			FABULOR_GTK4_THEME_VARIANT_FOLLOW_SYSTEM &&
		fabulor_gtk4_theme_preferences_normalize_variant (2) ==
			FABULOR_GTK4_THEME_VARIANT_PREFER_DARK &&
		fabulor_gtk4_theme_preferences_normalize_variant (99) ==
			FABULOR_GTK4_THEME_VARIANT_FOLLOW_SYSTEM;

	fabulor_gtk4_theme_preferences_resolve_appearance (
		TRUE, FABULOR_GTK4_THEME_VARIANT_FOLLOW_SYSTEM,
		TRUE, FALSE, &appearance);
	valid = valid && appearance.use_custom_theme && appearance.prefer_dark &&
		!appearance.high_contrast;
	fabulor_gtk4_theme_preferences_resolve_appearance (
		TRUE, FABULOR_GTK4_THEME_VARIANT_PREFER_LIGHT,
		TRUE, FALSE, &appearance);
	valid = valid && appearance.use_custom_theme && !appearance.prefer_dark;
	fabulor_gtk4_theme_preferences_resolve_appearance (
		TRUE, FABULOR_GTK4_THEME_VARIANT_PREFER_DARK,
		FALSE, FALSE, &appearance);
	valid = valid && appearance.use_custom_theme && appearance.prefer_dark;
	fabulor_gtk4_theme_preferences_resolve_appearance (
		FALSE, FABULOR_GTK4_THEME_VARIANT_PREFER_LIGHT,
		TRUE, FALSE, &appearance);
	valid = valid && !appearance.use_custom_theme && appearance.prefer_dark;
	fabulor_gtk4_theme_preferences_resolve_appearance (
		TRUE, FABULOR_GTK4_THEME_VARIANT_PREFER_DARK,
		TRUE, TRUE, &appearance);
	valid = valid && appearance.high_contrast &&
		!appearance.use_custom_theme && !appearance.prefer_dark;
	fabulor_gtk4_theme_preferences_resolve_appearance (
		TRUE, 99, TRUE, FALSE, &appearance);
	valid = valid && appearance.variant ==
		FABULOR_GTK4_THEME_VARIANT_FOLLOW_SYSTEM &&
		appearance.use_custom_theme && appearance.prefer_dark;
	g_ptr_array_unref (choices);
	return valid;
}

static gboolean
check_gtk4_theme_adapter_policy (void)
{
	GError *error = NULL;
	char *temporary = g_dir_make_tmp ("fabulor-gtk4-adapter-probe-XXXXXX", &error);
	char *valid_root;
	char *invalid_root;
	char *invalid_gtk_dir;
	char *invalid_css;
	FabulorGtk4Theme valid_theme = { 0 };
	FabulorGtk4Theme invalid_theme = { 0 };
	FabulorGtk4Theme missing_theme = { 0 };
	FabulorGtk4ThemeAppearanceDecision appearance;
	ThemeGtk4Adapter *adapter;
	gboolean valid;

	if (!temporary)
	{
		g_clear_error (&error);
		return FALSE;
	}
	valid_root = probe_make_gtk4_theme (temporary, "valid", "Valid", TRUE);
	invalid_root = g_build_filename (temporary, "invalid", NULL);
	invalid_gtk_dir = g_build_filename (invalid_root, "gtk-4.0", NULL);
	invalid_css = g_build_filename (invalid_gtk_dir, "gtk.css", NULL);
	g_mkdir_with_parents (invalid_gtk_dir, 0700);
	g_file_set_contents (invalid_css,
		"window { color: definitely-not-a-colour; }\n", -1, NULL);

	valid_theme.id = "profile:valid";
	valid_theme.path = valid_root;
	valid_theme.css_path = g_build_filename (valid_root, "gtk-4.0", "gtk.css", NULL);
	valid_theme.dark_css_path = g_build_filename (
		valid_root, "gtk-4.0", "gtk-dark.css", NULL);
	invalid_theme.id = "profile:invalid";
	invalid_theme.path = invalid_root;
	invalid_theme.css_path = invalid_css;
	missing_theme.id = "profile:missing";
	missing_theme.css_path = g_build_filename (
		temporary, "missing", "gtk-4.0", "gtk.css", NULL);

	adapter = theme_gtk4_adapter_new (NULL);
	valid = adapter != NULL &&
		!theme_gtk4_variant_uses_dark (THEME_GTK4_VARIANT_FOLLOW_SYSTEM, FALSE) &&
		theme_gtk4_variant_uses_dark (THEME_GTK4_VARIANT_FOLLOW_SYSTEM, TRUE) &&
		!theme_gtk4_variant_uses_dark (THEME_GTK4_VARIANT_PREFER_LIGHT, TRUE) &&
		theme_gtk4_variant_uses_dark (THEME_GTK4_VARIANT_PREFER_DARK, FALSE) &&
		theme_gtk4_adapter_apply (adapter, &valid_theme,
			THEME_GTK4_VARIANT_PREFER_LIGHT, TRUE, &error) &&
		error == NULL && theme_gtk4_adapter_is_active (adapter) &&
		g_strcmp0 (theme_gtk4_adapter_active_id (adapter), "profile:valid") == 0 &&
		theme_gtk4_adapter_active_variant (adapter) ==
			THEME_GTK4_VARIANT_PREFER_LIGHT &&
		theme_gtk4_adapter_active_provider_count (adapter) == 1 &&
		theme_gtk4_adapter_error_count (adapter) == 0 &&
		theme_gtk4_adapter_apply (adapter, &valid_theme,
			THEME_GTK4_VARIANT_PREFER_DARK, FALSE, &error) &&
		error == NULL && theme_gtk4_adapter_active_provider_count (adapter) == 2;

	valid = valid && !theme_gtk4_adapter_apply (adapter, &invalid_theme,
		THEME_GTK4_VARIANT_PREFER_LIGHT, FALSE, &error) &&
		error != NULL && theme_gtk4_adapter_error_count (adapter) > 0 &&
		theme_gtk4_adapter_last_diagnostic (adapter) != NULL &&
		theme_gtk4_adapter_is_active (adapter) &&
		g_strcmp0 (theme_gtk4_adapter_active_id (adapter), "profile:valid") == 0 &&
		theme_gtk4_adapter_active_provider_count (adapter) == 2;
	g_clear_error (&error);
	valid = valid && !theme_gtk4_adapter_apply (adapter, &missing_theme,
		THEME_GTK4_VARIANT_PREFER_LIGHT, FALSE, &error) &&
		g_error_matches (error, G_FILE_ERROR, G_FILE_ERROR_NOENT) &&
		theme_gtk4_adapter_is_active (adapter) &&
		theme_gtk4_adapter_active_provider_count (adapter) == 2;
	g_clear_error (&error);
	fabulor_gtk4_theme_preferences_resolve_appearance (
		TRUE, FABULOR_GTK4_THEME_VARIANT_PREFER_DARK,
		FALSE, FALSE, &appearance);
	valid = valid && theme_gtk4_adapter_apply_decision (adapter, &valid_theme,
		&appearance, &error) && error == NULL &&
		theme_gtk4_adapter_active_provider_count (adapter) == 2;
	fabulor_gtk4_theme_preferences_resolve_appearance (
		TRUE, FABULOR_GTK4_THEME_VARIANT_PREFER_DARK,
		TRUE, TRUE, &appearance);
	valid = valid && theme_gtk4_adapter_apply_decision (adapter, &valid_theme,
		&appearance, &error) && error == NULL &&
		!theme_gtk4_adapter_is_active (adapter);
	fabulor_gtk4_theme_preferences_resolve_appearance (
		FALSE, FABULOR_GTK4_THEME_VARIANT_FOLLOW_SYSTEM,
		TRUE, FALSE, &appearance);
	valid = valid && theme_gtk4_adapter_apply_decision (adapter, NULL,
		&appearance, &error) && error == NULL &&
		!theme_gtk4_adapter_is_active (adapter);

	theme_gtk4_adapter_disable (adapter);
	valid = valid && !theme_gtk4_adapter_is_active (adapter) &&
		theme_gtk4_adapter_active_id (adapter) == NULL &&
		theme_gtk4_adapter_active_variant (adapter) ==
			THEME_GTK4_VARIANT_FOLLOW_SYSTEM &&
		theme_gtk4_adapter_active_provider_count (adapter) == 0;
	theme_gtk4_adapter_free (adapter);

	g_free (missing_theme.css_path);
	g_free (valid_theme.dark_css_path);
	g_free (valid_theme.css_path);
	g_free (invalid_css);
	g_free (invalid_gtk_dir);
	g_free (invalid_root);
	g_free (valid_root);
	probe_remove_tree (temporary);
	g_free (temporary);
	return valid;
}

static gboolean
check_gtk4_theme_controller_policy (void)
{
	GError *error = NULL;
	char *temporary = g_dir_make_tmp (
		"fabulor-gtk4-controller-probe-XXXXXX", &error);
	char *valid_root;
	char *invalid_root;
	char *invalid_gtk_dir;
	char *invalid_css;
	FabulorGtk4Theme *valid_theme;
	FabulorGtk4Theme *invalid_theme;
	GPtrArray *themes;
	ThemeGtk4Controller *controller;
	const FabulorGtk4ThemeChoice *choice;
	const FabulorGtk4ThemeAppearanceDecision *appearance;
	gboolean valid;

	if (!temporary)
	{
		g_clear_error (&error);
		return FALSE;
	}
	valid_root = probe_make_gtk4_theme (temporary, "valid", "Valid", TRUE);
	invalid_root = g_build_filename (temporary, "invalid", NULL);
	invalid_gtk_dir = g_build_filename (invalid_root, "gtk-4.0", NULL);
	invalid_css = g_build_filename (invalid_gtk_dir, "gtk.css", NULL);
	g_mkdir_with_parents (invalid_gtk_dir, 0700);
	g_file_set_contents (invalid_css,
		"window { color: definitely-not-a-colour; }\n", -1, NULL);

	valid_theme = g_new0 (FabulorGtk4Theme, 1);
	valid_theme->id = g_strdup ("profile:valid");
	valid_theme->display_name = g_strdup ("Valid");
	valid_theme->path = g_strdup (valid_root);
	valid_theme->css_path = g_build_filename (
		valid_root, "gtk-4.0", "gtk.css", NULL);
	valid_theme->dark_css_path = g_build_filename (
		valid_root, "gtk-4.0", "gtk-dark.css", NULL);
	valid_theme->source = FABULOR_GTK4_THEME_SOURCE_PROFILE;
	invalid_theme = g_new0 (FabulorGtk4Theme, 1);
	invalid_theme->id = g_strdup ("profile:invalid");
	invalid_theme->display_name = g_strdup ("Invalid");
	invalid_theme->path = g_strdup (invalid_root);
	invalid_theme->css_path = g_build_filename (
		invalid_root, "gtk-4.0", "gtk.css", NULL);
	invalid_theme->source = FABULOR_GTK4_THEME_SOURCE_PROFILE;
	themes = g_ptr_array_new_with_free_func (
		(GDestroyNotify) fabulor_gtk4_theme_free);
	g_ptr_array_add (themes, valid_theme);
	g_ptr_array_add (themes, invalid_theme);

	controller = theme_gtk4_controller_new (NULL);
	valid = theme_gtk4_controller_refresh_from_themes (controller, themes,
		"profile:valid", FABULOR_GTK4_THEME_VARIANT_PREFER_DARK,
		FALSE, FALSE, &error) && error == NULL &&
		theme_gtk4_controller_theme_is_active (controller) &&
		g_strcmp0 (theme_gtk4_controller_active_id (controller),
			"profile:valid") == 0 &&
		theme_gtk4_controller_active_provider_count (controller) == 2 &&
		theme_gtk4_controller_stored_selection_available (controller);
	g_ptr_array_unref (themes);
	choice = theme_gtk4_controller_selected_choice (controller);
	valid = valid && choice &&
		g_strcmp0 (choice->display_name, "Valid") == 0;

	themes = g_ptr_array_new_with_free_func (
		(GDestroyNotify) fabulor_gtk4_theme_free);
	valid_theme = g_new0 (FabulorGtk4Theme, 1);
	valid_theme->id = g_strdup ("profile:valid");
	valid_theme->display_name = g_strdup ("Valid");
	valid_theme->css_path = g_build_filename (
		valid_root, "gtk-4.0", "gtk.css", NULL);
	valid_theme->dark_css_path = g_build_filename (
		valid_root, "gtk-4.0", "gtk-dark.css", NULL);
	valid_theme->source = FABULOR_GTK4_THEME_SOURCE_PROFILE;
	invalid_theme = g_new0 (FabulorGtk4Theme, 1);
	invalid_theme->id = g_strdup ("profile:invalid");
	invalid_theme->display_name = g_strdup ("Invalid");
	invalid_theme->css_path = g_build_filename (
		invalid_root, "gtk-4.0", "gtk.css", NULL);
	invalid_theme->source = FABULOR_GTK4_THEME_SOURCE_PROFILE;
	g_ptr_array_add (themes, valid_theme);
	g_ptr_array_add (themes, invalid_theme);
	valid = valid && !theme_gtk4_controller_refresh_from_themes (controller,
		themes, "profile:invalid", FABULOR_GTK4_THEME_VARIANT_PREFER_LIGHT,
		FALSE, FALSE, &error) && error != NULL &&
		g_strcmp0 (theme_gtk4_controller_active_id (controller),
			"profile:valid") == 0;
	g_clear_error (&error);
	choice = theme_gtk4_controller_selected_choice (controller);
	valid = valid && choice && g_strcmp0 (choice->id, "profile:valid") == 0;

	valid = valid && theme_gtk4_controller_refresh_from_themes (controller,
		themes, "profile:valid", FABULOR_GTK4_THEME_VARIANT_PREFER_DARK,
		TRUE, TRUE, &error) && error == NULL &&
		!theme_gtk4_controller_theme_is_active (controller);
	appearance = theme_gtk4_controller_appearance (controller);
	choice = theme_gtk4_controller_selected_choice (controller);
	valid = valid && appearance && appearance->high_contrast &&
		!appearance->use_custom_theme && choice &&
		g_strcmp0 (choice->id, "profile:valid") == 0;
	valid = valid && theme_gtk4_controller_refresh_from_themes (controller,
		themes, "profile:missing", FABULOR_GTK4_THEME_VARIANT_PREFER_DARK,
		TRUE, FALSE, &error) && error == NULL &&
		!theme_gtk4_controller_stored_selection_available (controller) &&
		!theme_gtk4_controller_theme_is_active (controller) &&
		theme_gtk4_controller_selected_index (controller) == 0;
	valid = valid && theme_gtk4_controller_refresh_from_themes (controller,
		themes, "profile:valid", FABULOR_GTK4_THEME_VARIANT_PREFER_LIGHT,
		FALSE, FALSE, &error) && error == NULL &&
		theme_gtk4_controller_theme_is_active (controller);

	g_ptr_array_unref (themes);
	theme_gtk4_controller_free (controller);
	g_free (invalid_css);
	g_free (invalid_gtk_dir);
	g_free (invalid_root);
	g_free (valid_root);
	probe_remove_tree (temporary);
	g_free (temporary);
	return valid;
}

typedef struct
{
	char *theme_id;
	guint variant;
	guint count;
} ProbeThemePreferencesCommit;

static void
probe_theme_preferences_commit (const char *theme_id, guint variant,
	gpointer user_data)
{
	ProbeThemePreferencesCommit *commit = user_data;

	g_free (commit->theme_id);
	commit->theme_id = g_strdup (theme_id);
	commit->variant = variant;
	commit->count++;
}

static guint
probe_theme_choice_index (const GPtrArray *choices, const char *display_name)
{
	guint i;

	for (i = 0; choices && i < choices->len; i++)
	{
		const FabulorGtk4ThemeChoice *choice = g_ptr_array_index (choices, i);

		if (g_strcmp0 (choice->display_name, display_name) == 0)
			return i;
	}
	return GTK_INVALID_LIST_POSITION;
}

static gboolean
check_gtk4_theme_preferences_binding (void)
{
	GError *error = NULL;
	char *temporary = g_dir_make_tmp (
		"fabulor-gtk4-theme-preferences-XXXXXX", &error);
	char *profile_root;
	char *valid_root;
	char *invalid_root;
	char *invalid_gtk_dir;
	char *invalid_css;
	GPtrArray *discovered;
	char *valid_id = NULL;
	ThemePreferencesGtk4 *preferences;
	ThemePreferencesGtk4 *missing_preferences;
	ThemeGtk4Controller *controller;
	const GPtrArray *choices;
	ProbeThemePreferencesCommit commit = { 0 };
	GtkWidget *container;
	GtkWidget *retained;
	guint valid_index;
	guint invalid_index;
	gboolean valid;
	guint i;

	if (!temporary)
	{
		g_clear_error (&error);
		return FALSE;
	}
	profile_root = fabulor_gtk4_theme_profile_dir (temporary);
	g_mkdir_with_parents (profile_root, 0700);
	valid_root = probe_make_gtk4_theme (profile_root, "valid", "Valid", TRUE);
	invalid_root = g_build_filename (profile_root, "invalid", NULL);
	invalid_gtk_dir = g_build_filename (invalid_root, "gtk-4.0", NULL);
	invalid_css = g_build_filename (invalid_gtk_dir, "gtk.css", NULL);
	g_mkdir_with_parents (invalid_gtk_dir, 0700);
	g_file_set_contents (invalid_css,
		"window { color: definitely-not-a-colour; }\n", -1, NULL);
	discovered = fabulor_gtk4_theme_discover (temporary);
	for (i = 0; i < discovered->len; i++)
	{
		const FabulorGtk4Theme *theme = g_ptr_array_index (discovered, i);

		if (g_strcmp0 (theme->display_name, "Valid") == 0)
			valid_id = g_strdup (theme->id);
	}
	g_ptr_array_unref (discovered);

	preferences = theme_preferences_gtk4_new (NULL, temporary, valid_id,
		FABULOR_GTK4_THEME_VARIANT_PREFER_LIGHT, FALSE, FALSE,
		probe_theme_preferences_commit, &commit, NULL, &error);
	controller = theme_preferences_gtk4_controller (preferences);
	choices = controller ? theme_gtk4_controller_choices (controller) : NULL;
	valid_index = probe_theme_choice_index (choices, "Valid");
	invalid_index = probe_theme_choice_index (choices, "invalid");
	valid = preferences != NULL && error == NULL && choices &&
		choices->len == 3 && valid_index != GTK_INVALID_LIST_POSITION &&
		invalid_index != GTK_INVALID_LIST_POSITION &&
		GTK_IS_BOX (theme_preferences_gtk4_widget (preferences)) &&
		g_strcmp0 (theme_preferences_gtk4_stored_id (preferences), valid_id) == 0 &&
		theme_preferences_gtk4_stored_variant (preferences) ==
			FABULOR_GTK4_THEME_VARIANT_PREFER_LIGHT &&
		theme_gtk4_controller_theme_is_active (controller) &&
		theme_preferences_gtk4_status (preferences) == NULL;
	valid = valid && theme_preferences_gtk4_select_variant (preferences,
		FABULOR_GTK4_THEME_VARIANT_PREFER_DARK, &error) && error == NULL &&
		commit.count == 1 && commit.variant ==
			FABULOR_GTK4_THEME_VARIANT_PREFER_DARK &&
		g_strcmp0 (commit.theme_id, valid_id) == 0 &&
		theme_gtk4_controller_active_provider_count (controller) == 2;
	valid = valid && !theme_preferences_gtk4_select_theme (preferences,
		invalid_index, &error) && error != NULL && commit.count == 1 &&
		g_strcmp0 (theme_preferences_gtk4_stored_id (preferences), valid_id) == 0 &&
		g_strcmp0 (theme_gtk4_controller_active_id (controller), valid_id) == 0 &&
		theme_preferences_gtk4_status (preferences) != NULL;
	g_clear_error (&error);
	valid = valid && theme_preferences_gtk4_select_theme (preferences, 0,
		&error) && error == NULL && commit.count == 2 &&
		commit.theme_id[0] == '\0' &&
		!theme_gtk4_controller_theme_is_active (controller);
	valid = valid && theme_preferences_gtk4_select_theme (preferences,
		valid_index, &error) && error == NULL && commit.count == 3 &&
		theme_preferences_gtk4_refresh (preferences, TRUE, TRUE, &error) &&
		error == NULL && !theme_gtk4_controller_theme_is_active (controller) &&
		theme_preferences_gtk4_status (preferences) != NULL;

	container = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	g_object_ref_sink (container);
	retained = g_object_ref (theme_preferences_gtk4_widget (preferences));
	gtk_box_append (GTK_BOX (container), retained);
	theme_preferences_gtk4_free (preferences);
	valid = valid && gtk_widget_get_parent (retained) == NULL;
	g_object_unref (retained);
	g_object_unref (container);

	missing_preferences = theme_preferences_gtk4_new (NULL, temporary,
		"profile:missing", FABULOR_GTK4_THEME_VARIANT_FOLLOW_SYSTEM,
		FALSE, FALSE, NULL, NULL, NULL, &error);
	valid = valid && missing_preferences != NULL && error == NULL &&
		theme_preferences_gtk4_status (missing_preferences) != NULL &&
		theme_gtk4_controller_selected_index (
			theme_preferences_gtk4_controller (missing_preferences)) == 0;
	theme_preferences_gtk4_free (missing_preferences);

	g_free (commit.theme_id);
	g_free (valid_id);
	g_free (invalid_css);
	g_free (invalid_gtk_dir);
	g_free (invalid_root);
	g_free (valid_root);
	g_free (profile_root);
	probe_remove_tree (temporary);
	g_free (temporary);
	return valid;
}

typedef struct
{
	gboolean prefer_dark;
	gboolean high_contrast;
	gboolean succeed;
	guint count;
} ProbeThemeAppearanceQuery;

static gboolean
probe_theme_appearance_query (gboolean *prefer_dark, gboolean *high_contrast,
	gpointer user_data)
{
	ProbeThemeAppearanceQuery *query = user_data;

	query->count++;
	if (!query->succeed)
		return FALSE;
	*prefer_dark = query->prefer_dark;
	*high_contrast = query->high_contrast;
	return TRUE;
}

static void
probe_run_pending_main_context (void)
{
	while (g_main_context_iteration (NULL, FALSE))
		;
}

static gboolean
check_gtk4_theme_appearance_monitor (void)
{
	GError *error = NULL;
	char *temporary = g_dir_make_tmp (
		"fabulor-gtk4-theme-appearance-XXXXXX", &error);
	char *profile_root;
	char *valid_root;
	GPtrArray *discovered;
	char *valid_id = NULL;
	ThemePreferencesGtk4 *preferences;
	ThemeAppearanceMonitorGtk4 *monitor;
	ThemeGtk4Controller *controller;
	ProbeThemePreferencesCommit commit = { 0 };
	ProbeThemeAppearanceQuery query = { FALSE, FALSE, TRUE, 0 };
	gboolean valid;
	guint query_count_before_free;
	guint i;

	if (!temporary)
	{
		g_clear_error (&error);
		return FALSE;
	}
	profile_root = fabulor_gtk4_theme_profile_dir (temporary);
	g_mkdir_with_parents (profile_root, 0700);
	valid_root = probe_make_gtk4_theme (profile_root, "valid", "Valid", TRUE);
	discovered = fabulor_gtk4_theme_discover (temporary);
	for (i = 0; i < discovered->len; i++)
	{
		const FabulorGtk4Theme *theme = g_ptr_array_index (discovered, i);

		if (g_strcmp0 (theme->display_name, "Valid") == 0)
			valid_id = g_strdup (theme->id);
	}
	g_ptr_array_unref (discovered);
	preferences = theme_preferences_gtk4_new (NULL, temporary, valid_id,
		FABULOR_GTK4_THEME_VARIANT_FOLLOW_SYSTEM, FALSE, FALSE,
		probe_theme_preferences_commit, &commit, NULL, &error);
	monitor = preferences ? theme_appearance_monitor_gtk4_new_with_query (
		gdk_display_get_default (), preferences, probe_theme_appearance_query,
		&query, NULL, &error) : NULL;
	controller = theme_preferences_gtk4_controller (preferences);
	valid = preferences && monitor && error == NULL && query.count == 1 &&
		theme_appearance_monitor_gtk4_refresh_count (monitor) == 1 &&
		!theme_appearance_monitor_gtk4_prefers_dark (monitor) &&
		!theme_appearance_monitor_gtk4_high_contrast (monitor) &&
		!theme_appearance_monitor_gtk4_is_pending (monitor) &&
		commit.count == 0 && theme_gtk4_controller_theme_is_active (controller) &&
		theme_gtk4_controller_active_provider_count (controller) == 1;
#ifdef G_OS_WIN32
	valid = valid && (!gdk_display_get_default () ||
		theme_appearance_monitor_gtk4_filter_is_installed (monitor));
#else
	valid = valid &&
		!theme_appearance_monitor_gtk4_filter_is_installed (monitor);
#endif

	query.prefer_dark = TRUE;
	valid = valid && theme_appearance_monitor_gtk4_queue_refresh (monitor) &&
		!theme_appearance_monitor_gtk4_queue_refresh (monitor) &&
		theme_appearance_monitor_gtk4_is_pending (monitor);
	probe_run_pending_main_context ();
	valid = valid && query.count == 2 &&
		theme_appearance_monitor_gtk4_refresh_count (monitor) == 2 &&
		theme_appearance_monitor_gtk4_prefers_dark (monitor) &&
		theme_gtk4_controller_active_provider_count (controller) == 2 &&
		commit.count == 0;

	valid = valid && theme_appearance_monitor_gtk4_queue_refresh (monitor);
	probe_run_pending_main_context ();
	valid = valid && query.count == 3 &&
		theme_appearance_monitor_gtk4_refresh_count (monitor) == 2 &&
		theme_appearance_monitor_gtk4_last_diagnostic (monitor) == NULL;

	query.high_contrast = TRUE;
	valid = valid && theme_appearance_monitor_gtk4_queue_refresh (monitor);
	probe_run_pending_main_context ();
	valid = valid && query.count == 4 &&
		theme_appearance_monitor_gtk4_refresh_count (monitor) == 3 &&
		theme_appearance_monitor_gtk4_high_contrast (monitor) &&
		!theme_gtk4_controller_theme_is_active (controller) &&
		commit.count == 0;

	query.succeed = FALSE;
	valid = valid && theme_appearance_monitor_gtk4_queue_refresh (monitor);
	probe_run_pending_main_context ();
	valid = valid && query.count == 5 &&
		theme_appearance_monitor_gtk4_refresh_count (monitor) == 3 &&
		theme_appearance_monitor_gtk4_high_contrast (monitor) &&
		theme_appearance_monitor_gtk4_last_diagnostic (monitor) != NULL;

	query.succeed = TRUE;
	query.prefer_dark = FALSE;
	query.high_contrast = FALSE;
	valid = valid && theme_appearance_monitor_gtk4_queue_refresh (monitor);
	query_count_before_free = query.count;
	theme_appearance_monitor_gtk4_free (monitor);
	probe_run_pending_main_context ();
	valid = valid && query.count == query_count_before_free;
	theme_preferences_gtk4_free (preferences);

	g_free (commit.theme_id);
	g_free (valid_id);
	g_free (valid_root);
	g_free (profile_root);
	probe_remove_tree (temporary);
	g_free (temporary);
	return valid;
}

static gboolean
probe_spell_mirc_color (gint color_index, FabulorSpellEntryColor *color,
	gpointer user_data)
{
	(void) user_data;
	color->red = (guint16) (1000 + color_index);
	color->green = (guint16) (2000 + color_index);
	color->blue = (guint16) (3000 + color_index);
	return TRUE;
}

static gboolean
spell_style_has_type (PangoAttrList *attributes, PangoAttrType type,
	guint index)
{
	PangoAttrIterator *iterator = pango_attr_list_get_iterator (attributes);
	gboolean found = FALSE;

	do
	{
		gint start;
		gint end;
		GSList *items;
		GSList *item;

		pango_attr_iterator_range (iterator, &start, &end);
		if ((gint) index < start || (gint) index >= end)
			continue;
		items = pango_attr_iterator_get_attrs (iterator);
		for (item = items; item; item = item->next)
		{
			PangoAttribute *attribute = item->data;
			if (attribute->klass->type == type)
			{
				found = TRUE;
				break;
			}
		}
		g_slist_free_full (items, (GDestroyNotify) pango_attribute_destroy);
	}
	while (!found && pango_attr_iterator_next (iterator));
	pango_attr_iterator_destroy (iterator);
	return found;
}

static gboolean
spell_style_has_int (PangoAttrList *attributes, PangoAttrType type,
	guint index, gint expected)
{
	PangoAttrIterator *iterator = pango_attr_list_get_iterator (attributes);
	gboolean found = FALSE;

	do
	{
		gint start;
		gint end;
		GSList *items;
		GSList *item;

		pango_attr_iterator_range (iterator, &start, &end);
		if ((gint) index < start || (gint) index >= end)
			continue;
		items = pango_attr_iterator_get_attrs (iterator);
		for (item = items; item; item = item->next)
		{
			PangoAttribute *attribute = item->data;
			if (attribute->klass->type == type &&
				((PangoAttrInt *) attribute)->value == expected)
			{
				found = TRUE;
				break;
			}
		}
		g_slist_free_full (items, (GDestroyNotify) pango_attribute_destroy);
	}
	while (!found && pango_attr_iterator_next (iterator));
	pango_attr_iterator_destroy (iterator);
	return found;
}

static gboolean
spell_style_has_color (PangoAttrList *attributes, PangoAttrType type,
	guint index, guint16 red, guint16 green, guint16 blue)
{
	PangoAttrIterator *iterator = pango_attr_list_get_iterator (attributes);
	gboolean found = FALSE;

	do
	{
		gint start;
		gint end;
		GSList *items;
		GSList *item;

		pango_attr_iterator_range (iterator, &start, &end);
		if ((gint) index < start || (gint) index >= end)
			continue;
		items = pango_attr_iterator_get_attrs (iterator);
		for (item = items; item; item = item->next)
		{
			PangoAttribute *attribute = item->data;
			PangoAttrColor *color = (PangoAttrColor *) attribute;

			if (attribute->klass->type == type && color->color.red == red &&
				color->color.green == green && color->color.blue == blue)
			{
				found = TRUE;
				break;
			}
		}
		g_slist_free_full (items, (GDestroyNotify) pango_attribute_destroy);
	}
	while (!found && pango_attr_iterator_next (iterator));
	pango_attr_iterator_destroy (iterator);
	return found;
}

typedef struct
{
	FabulorTrayAction action;
	gboolean value;
	guint count;
	gboolean destroyed;
} TrayActionRecord;

static void
tray_action_record (FabulorTrayAction action, gboolean value,
	gpointer user_data)
{
	TrayActionRecord *target = user_data;
	target->action = action;
	target->value = value;
	target->count++;
}

static void
tray_action_destroy (gpointer user_data)
{
	TrayActionRecord *target = user_data;
	target->destroyed = TRUE;
}

static void
tray_menu_items_changed (GMenuModel *model, gint position,
	gint removed, gint added, gpointer user_data)
{
	guint *changes = user_data;

	(void)model;
	(void)position;
	(void)removed;
	(void)added;
	(*changes)++;
}

static gboolean
check_tray_action_model (void)
{
	char hide_window[] = "_Hide Window";
	FabulorTrayActionLabels labels = {
		hide_window,
		"_Restore Window",
		"_Blink on",
		"Channel Message",
		"Private Message",
		"Highlighted Message",
		"_Change status",
		"_Away",
		"_Back",
		"_Preferences",
		"_Quit"
	};
	FabulorTrayActionState state = {
		FALSE,
		FABULOR_TRAY_AWAY_MIXED,
		FALSE,
		TRUE,
		FALSE
	};
	TrayActionRecord record = { 0 };
	FabulorTrayActionModel *model;
	GMenuModel *menu;
	GMenuModel *section;
	GActionGroup *actions;
	GActionGroup *retained_actions;
	GVariant *action_state;
	char *label = NULL;
	guint menu_changes = 0;
	gboolean passed = TRUE;

	model = fabulor_tray_action_model_new (&labels, &state,
		tray_action_record, &record, tray_action_destroy);
	if (!model)
		return FALSE;

	hide_window[0] = 'X';
	menu = fabulor_tray_action_model_get_menu (model);
	actions = fabulor_tray_action_model_get_actions (model);
	retained_actions = g_object_ref (actions);
	g_signal_connect (menu, "items-changed",
		G_CALLBACK (tray_menu_items_changed), &menu_changes);
	fabulor_tray_action_model_update (model, &state);
	passed = passed && menu_changes == 0;
	section = g_menu_model_get_item_link (menu, 0, G_MENU_LINK_SECTION);
	passed = passed && section != NULL;
	if (section)
	{
		g_menu_model_get_item_attribute (section, 0,
			G_MENU_ATTRIBUTE_LABEL, "s", &label);
		passed = passed && g_strcmp0 (label, "_Hide Window") == 0;
		g_free (label);
		g_object_unref (section);
	}

	action_state = g_action_group_get_action_state (actions, "blink-private");
	passed = passed && action_state && g_variant_get_boolean (action_state);
	g_clear_pointer (&action_state, g_variant_unref);

	state.window_hidden = TRUE;
	state.away_state = FABULOR_TRAY_AWAY_ALL_AWAY;
	state.blink_private = FALSE;
	fabulor_tray_action_model_update (model, &state);
	passed = passed && menu_changes > 0;
	menu_changes = 0;
	fabulor_tray_action_model_update (model, &state);
	passed = passed && menu_changes == 0;
	passed = passed && !g_action_group_get_action_enabled (actions, "set-away");
	passed = passed && g_action_group_get_action_enabled (actions, "set-back");
	section = g_menu_model_get_item_link (menu, 0, G_MENU_LINK_SECTION);
	if (section)
	{
		g_menu_model_get_item_attribute (section, 0,
			G_MENU_ATTRIBUTE_LABEL, "s", &label);
		passed = passed && g_strcmp0 (label, "_Restore Window") == 0;
		g_free (label);
		g_object_unref (section);
	}
	else
		passed = FALSE;

	g_action_group_activate_action (actions, "set-away", NULL);
	passed = passed && record.count == 0;
	g_action_group_activate_action (actions, "set-back", NULL);
	passed = passed && record.count == 1 &&
		record.action == FABULOR_TRAY_ACTION_SET_BACK && !record.value;
	g_action_group_activate_action (actions, "blink-channel", NULL);
	passed = passed && record.count == 2 &&
		record.action == FABULOR_TRAY_ACTION_BLINK_CHANNEL && record.value;
	action_state = g_action_group_get_action_state (actions, "blink-channel");
	passed = passed && action_state && g_variant_get_boolean (action_state);
	g_clear_pointer (&action_state, g_variant_unref);

	state.away_state = (FabulorTrayAwayState)99;
	fabulor_tray_action_model_update (model, &state);
	passed = passed && g_action_group_get_action_enabled (actions, "set-away") &&
		g_action_group_get_action_enabled (actions, "set-back");

	fabulor_tray_action_model_free (model);
	passed = passed && record.destroyed &&
		!g_action_group_get_action_enabled (retained_actions, "quit");
	g_action_group_activate_action (retained_actions, "quit", NULL);
	passed = passed && record.count == 2;
	g_object_unref (retained_actions);
	return passed;
}

static gboolean
tray_backend_policy_matches (const FabulorTrayBackendEnvironment *environment,
	FabulorTrayBackendKind expected, const char *expected_name,
	gboolean expected_usable)
{
	FabulorTrayBackendKind selected =
		fabulor_tray_backend_select (environment);

	return selected == expected &&
		g_strcmp0 (fabulor_tray_backend_name (selected), expected_name) == 0 &&
		fabulor_tray_backend_is_usable (selected) == expected_usable;
}

static gboolean
check_tray_backend_policy (void)
{
	FabulorTrayBackendEnvironment environment = { 0 };
	gboolean passed;

	environment.windows = TRUE;
	environment.windows_shell_available = TRUE;
	environment.status_notifier_compiled = TRUE;
	environment.status_notifier_available = TRUE;
	passed = tray_backend_policy_matches (&environment,
		FABULOR_TRAY_BACKEND_DISABLED, "disabled", FALSE);

	environment.enabled = TRUE;
	passed = passed && tray_backend_policy_matches (&environment,
		FABULOR_TRAY_BACKEND_WINDOWS_SHELL, "windows-shell", TRUE);
	environment.windows_shell_available = FALSE;
	passed = passed && tray_backend_policy_matches (&environment,
		FABULOR_TRAY_BACKEND_UNAVAILABLE, "unavailable", FALSE);

	environment.windows = FALSE;
	environment.toolkit_major = 4;
	passed = passed && tray_backend_policy_matches (&environment,
		FABULOR_TRAY_BACKEND_STATUS_NOTIFIER, "status-notifier", TRUE);
	environment.status_notifier_available = FALSE;
	environment.legacy_status_icon_available = TRUE;
	passed = passed && tray_backend_policy_matches (&environment,
		FABULOR_TRAY_BACKEND_UNAVAILABLE, "unavailable", FALSE);

	environment.toolkit_major = 0;
	passed = passed && tray_backend_policy_matches (&environment,
		FABULOR_TRAY_BACKEND_UNAVAILABLE, "unavailable", FALSE);
	environment.toolkit_major = 3;
	passed = passed && tray_backend_policy_matches (&environment,
		FABULOR_TRAY_BACKEND_LEGACY_STATUS_ICON,
		"legacy-status-icon", TRUE);
	environment.status_notifier_available = TRUE;
	passed = passed && tray_backend_policy_matches (&environment,
		FABULOR_TRAY_BACKEND_STATUS_NOTIFIER, "status-notifier", TRUE);
	return passed;
}

static gboolean
check_tray_menu_composition (void)
{
	GMenu *built_in = g_menu_new ();
	GMenu *plugin = g_menu_new ();
	GMenu *plugin_section = g_menu_new ();
	GMenuItem *plugin_item;
	GMenuModel *composed;
	GMenuModel *section;
	GMenuModel *without_plugin;
	GMenuModel *at_start;
	char *action = NULL;
	char *kind = NULL;
	gboolean passed;

	g_menu_append (built_in, "First", "tray.first");
	g_menu_append (built_in, "Second", "tray.second");
	g_menu_append (built_in, "Last", "tray.last");
	plugin_item = g_menu_item_new ("Plugin command",
		"fabulor-context.plugin-0");
	g_menu_item_set_attribute (plugin_item, "fabulor-menu-kind", "s", "command");
	g_menu_append_item (plugin_section, plugin_item);
	g_object_unref (plugin_item);
	g_menu_append_section (plugin, NULL, G_MENU_MODEL (plugin_section));
	g_object_unref (plugin_section);

	composed = fabulor_tray_menu_compose (G_MENU_MODEL (built_in),
		G_MENU_MODEL (plugin), 2);
	at_start = fabulor_tray_menu_compose (G_MENU_MODEL (built_in),
		G_MENU_MODEL (plugin), 0);
	g_object_unref (plugin);
	passed = composed && g_menu_model_get_n_items (composed) == 4;
	passed = passed && at_start &&
		g_menu_model_get_n_items (at_start) == 4;
	section = composed ? g_menu_model_get_item_link (
		composed, 2, G_MENU_LINK_SECTION) : NULL;
	passed = passed && section != NULL;
	if (section)
	{
		g_menu_model_get_item_attribute (section, 0,
			G_MENU_ATTRIBUTE_ACTION, "s", &action);
		g_menu_model_get_item_attribute (section, 0,
			"fabulor-menu-kind", "s", &kind);
		passed = passed &&
			g_strcmp0 (action, "fabulor-context.plugin-0") == 0 &&
			g_strcmp0 (kind, "command") == 0;
		g_free (action);
		g_free (kind);
		g_object_unref (section);
	}

	without_plugin = fabulor_tray_menu_compose (G_MENU_MODEL (built_in),
		NULL, G_MAXUINT);
	passed = passed && without_plugin &&
		g_menu_model_get_n_items (without_plugin) == 3;
	g_clear_object (&at_start);
	g_clear_object (&without_plugin);
	g_clear_object (&composed);
	g_object_unref (built_in);
	return passed;
}

static void
tray_presenter_action_activated (GSimpleAction *action, GVariant *parameter,
	gpointer user_data)
{
	guint *count = user_data;

	(void)action;
	(void)parameter;
	(*count)++;
}

static GActionGroup *
tray_presenter_action_group_new (const char *name, guint *count)
{
	GSimpleActionGroup *group = g_simple_action_group_new ();
	GSimpleAction *action = g_simple_action_new (name, NULL);

	g_signal_connect (action, "activate",
		G_CALLBACK (tray_presenter_action_activated), count);
	g_action_map_add_action (G_ACTION_MAP (group), G_ACTION (action));
	g_object_unref (action);
	return G_ACTION_GROUP (group);
}

static gboolean
check_tray_menu_presenter_gtk4 (void)
{
	GMenu *first_menu = g_menu_new ();
	GActionGroup *first_built_in;
	GActionGroup *first_plugin;
	GMenu *second_menu;
	GActionGroup *second_built_in;
	GActionGroup *second_plugin;
	FabulorTrayMenuPresenterGtk4 *presenter;
	GtkPopoverMenu *popover;
	GtkPopoverMenu *retained_popover;
	guint first_built_in_count = 0;
	guint first_plugin_count = 0;
	guint second_built_in_count = 0;
	guint second_plugin_count = 0;
	gboolean passed;

	g_menu_append (first_menu, "Built in", "tray.run");
	g_menu_append (first_menu, "Plugin", "fabulor-context.run");
	first_built_in = tray_presenter_action_group_new (
		"run", &first_built_in_count);
	first_plugin = tray_presenter_action_group_new (
		"run", &first_plugin_count);
	presenter = fabulor_tray_menu_presenter_gtk4_new (
		G_MENU_MODEL (first_menu), first_built_in, first_plugin);
	g_object_unref (first_menu);
	g_object_unref (first_built_in);
	g_object_unref (first_plugin);
	if (!presenter)
		return FALSE;

	popover = fabulor_tray_menu_presenter_gtk4_get_popover (presenter);
	retained_popover = g_object_ref (popover);
	passed = gtk_popover_menu_get_menu_model (popover) ==
		fabulor_tray_menu_presenter_gtk4_get_menu (presenter);
	passed = passed && gtk_widget_activate_action (
		GTK_WIDGET (popover), "tray.run", NULL);
	passed = passed && gtk_widget_activate_action (
		GTK_WIDGET (popover), "fabulor-context.run", NULL);
	passed = passed && first_built_in_count == 1 && first_plugin_count == 1;

	second_menu = g_menu_new ();
	g_menu_append (second_menu, "Replacement built in", "tray.run");
	g_menu_append (second_menu, "Replacement plugin", "fabulor-context.run");
	second_built_in = tray_presenter_action_group_new (
		"run", &second_built_in_count);
	second_plugin = tray_presenter_action_group_new (
		"run", &second_plugin_count);
	passed = passed && fabulor_tray_menu_presenter_gtk4_set_projection (
		presenter, G_MENU_MODEL (second_menu), second_built_in, second_plugin);
	g_object_unref (second_menu);
	g_object_unref (second_built_in);
	g_object_unref (second_plugin);
	passed = passed && gtk_widget_activate_action (
		GTK_WIDGET (popover), "tray.run", NULL);
	passed = passed && gtk_widget_activate_action (
		GTK_WIDGET (popover), "fabulor-context.run", NULL);
	passed = passed && first_built_in_count == 1 && first_plugin_count == 1 &&
		second_built_in_count == 1 && second_plugin_count == 1;

	fabulor_tray_menu_presenter_gtk4_free (presenter);
	passed = passed &&
		gtk_popover_menu_get_menu_model (retained_popover) == NULL &&
		!gtk_widget_activate_action (GTK_WIDGET (retained_popover),
			"tray.run", NULL) &&
		!gtk_widget_activate_action (GTK_WIDGET (retained_popover),
			"fabulor-context.run", NULL);
	g_object_unref (retained_popover);
	return passed;
}

static gboolean
check_context_menu_presenter_gtk4 (void)
{
	GMenu *custom_menu;
	GActionGroup *custom_actions;
	GMenu *menu = g_menu_new ();
	GActionGroup *built_in;
	GActionGroup *plugin;
	GtkWidget *origin = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget *window = gtk_window_new ();
	FabulorContextMenuPresenterGtk4 *presenter;
	FabulorContextMenuPresenterGtk4 *custom_presenter;
	GtkPopoverMenu *popover;
	GdkRectangle point = { 0 };
	guint built_in_count = 0;
	guint plugin_count = 0;
	guint custom_count = 0;
	gboolean passed;

	g_object_ref_sink (origin);
	g_object_ref_sink (window);
	gtk_window_set_child (GTK_WINDOW (window), origin);
	gtk_window_present (GTK_WINDOW (window));
	g_menu_append (menu, "Built in", "context.run");
	g_menu_append (menu, "Plugin", "fabulor-context.run");
	built_in = tray_presenter_action_group_new ("run", &built_in_count);
	plugin = tray_presenter_action_group_new ("run", &plugin_count);
	presenter = fabulor_context_menu_presenter_gtk4_new (
		G_MENU_MODEL (menu), built_in, plugin);
	g_object_unref (menu);
	g_object_unref (built_in);
	g_object_unref (plugin);
	if (!presenter)
	{
		gtk_window_set_child (GTK_WINDOW (window), NULL);
		g_object_unref (origin);
		g_object_unref (window);
		return FALSE;
	}
	popover = fabulor_context_menu_presenter_gtk4_get_popover (presenter);
	passed = fabulor_context_menu_presenter_gtk4_popup_at (
		presenter, origin, 17.0, 23.0);
	passed = passed && gtk_widget_get_parent (GTK_WIDGET (popover)) == origin;
	passed = passed && gtk_popover_get_pointing_to (
		GTK_POPOVER (popover), &point) && point.x == 17 && point.y == 23;
	passed = passed && gtk_widget_activate_action (
		GTK_WIDGET (popover), "context.run", NULL);
	passed = passed && gtk_widget_activate_action (
		GTK_WIDGET (popover), "fabulor-context.run", NULL);
	passed = passed && built_in_count == 1 && plugin_count == 1;
	fabulor_context_menu_presenter_gtk4_free (presenter);
	passed = passed && gtk_widget_get_first_child (origin) == NULL;

	custom_menu = g_menu_new ();
	g_menu_append (custom_menu, "Main action", "fabulor.run");
	custom_actions = tray_presenter_action_group_new ("run", &custom_count);
	custom_presenter =
		fabulor_context_menu_presenter_gtk4_new_with_namespaces (
			G_MENU_MODEL (custom_menu), "fabulor", custom_actions, NULL, NULL);
	g_object_unref (custom_menu);
	g_object_unref (custom_actions);
	passed = passed && custom_presenter != NULL;
	if (custom_presenter)
	{
		passed = passed && fabulor_context_menu_presenter_gtk4_popup_at (
			custom_presenter, origin, 7.0, 11.0);
		popover = fabulor_context_menu_presenter_gtk4_get_popover (
			custom_presenter);
		passed = passed && gtk_widget_activate_action (GTK_WIDGET (popover),
			"fabulor.run", NULL) && custom_count == 1;
		fabulor_context_menu_presenter_gtk4_free (custom_presenter);
		passed = passed && gtk_widget_get_first_child (origin) == NULL;
	}
	gtk_window_set_child (GTK_WINDOW (window), NULL);
	g_object_unref (origin);
	g_object_unref (window);
	return passed;
}

typedef struct
{
	guint open_count;
	guint copy_count;
	guint handler_count;
	const char *last_url;
	const char *last_command;
} UrlContextProbe;

static void
url_context_probe_dispatch (FabulorUrlContextAction action, const char *url,
	const char *command, gpointer user_data)
{
	UrlContextProbe *probe = user_data;
	if (action == FABULOR_URL_CONTEXT_OPEN)
		probe->open_count++;
	else if (action == FABULOR_URL_CONTEXT_COPY)
		probe->copy_count++;
	else if (action == FABULOR_URL_CONTEXT_HANDLER)
		probe->handler_count++;
	probe->last_url = url;
	probe->last_command = command;
}

static gboolean
check_url_context_menu_model (void)
{
	GMenu *plugin = g_menu_new ();
	FabulorUrlContextMenuModel *model;
	GMenuModel *menu;
	GMenuModel *commands;
	GMenuModel *handler_section;
	GMenuModel *handler_submenu;
	GActionGroup *actions;
	FabulorUrlHandler handlers[] = {
		{ FABULOR_URL_HANDLER_SUBMENU_BEGIN, "Tools", NULL, NULL, TRUE, FALSE },
		{ FABULOR_URL_HANDLER_COMMAND, "Browser", "exec browser %s", "web", TRUE, FALSE },
		{ FABULOR_URL_HANDLER_SEPARATOR, NULL, NULL, NULL, TRUE, FALSE },
		{ FABULOR_URL_HANDLER_COMMAND, "Unavailable", "exec missing %s", NULL, FALSE, FALSE },
		{ FABULOR_URL_HANDLER_SUBMENU_END, NULL, NULL, NULL, TRUE, FALSE },
		{ FABULOR_URL_HANDLER_COMMAND, "Inspect", "inspect %s", NULL, TRUE, FALSE },
		{ FABULOR_URL_HANDLER_TOGGLE, "Toggle", "gui_test", NULL, TRUE, FALSE }
	};
	char *reloadable_command = g_strdup ("inspect %s");
	UrlContextProbe probe = { 0 };
	char *label = NULL;
	gboolean passed;

	g_menu_append (plugin, "Plugin", "fabulor-context.run");
	handlers[5].command = reloadable_command;
	model = fabulor_url_context_menu_model_new_with_handlers ("ircs://irc.example",
		"Open", "Connect", "Copy", handlers, G_N_ELEMENTS (handlers),
		G_MENU_MODEL (plugin),
		url_context_probe_dispatch, &probe);
	g_free (reloadable_command);
	g_object_unref (plugin);
	if (!model)
		return FALSE;
	menu = fabulor_url_context_menu_model_get_menu (model);
	actions = fabulor_url_context_menu_model_get_actions (model);
	passed = g_menu_model_get_n_items (menu) == 4;
	commands = g_menu_model_get_item_link (menu, 1, G_MENU_LINK_SECTION);
	passed = passed && commands && g_menu_model_get_item_attribute (commands, 0,
		G_MENU_ATTRIBUTE_LABEL, "s", &label) && g_strcmp0 (label, "Connect") == 0;
	g_free (label);
	g_clear_object (&commands);
	handler_section = g_menu_model_get_item_link (menu, 2, G_MENU_LINK_SECTION);
	handler_submenu = handler_section ? g_menu_model_get_item_link (
		handler_section, 0, G_MENU_LINK_SUBMENU) : NULL;
	passed = passed && handler_submenu &&
		g_menu_model_get_n_items (handler_submenu) == 2;
	g_clear_object (&handler_submenu);
	g_clear_object (&handler_section);
	g_action_group_activate_action (actions, "open", NULL);
	g_action_group_activate_action (actions, "copy", NULL);
	g_action_group_activate_action (actions, "handler-0", NULL);
	g_action_group_activate_action (actions, "handler-1", NULL);
	g_action_group_activate_action (actions, "handler-2", NULL);
	g_action_group_activate_action (actions, "handler-3", NULL);
	passed = passed && probe.open_count == 1 && probe.copy_count == 1 &&
		probe.handler_count == 3 &&
		g_strcmp0 (probe.last_url, "ircs://irc.example") == 0 &&
		g_strcmp0 (probe.last_command, "set gui_test 1") == 0;
	fabulor_url_context_menu_model_free (model);
	return passed;
}

typedef struct
{
	guint counts[5];
	gboolean autojoin_state;
	const char *last_channel;
} ChannelContextProbe;

static void
channel_context_probe_dispatch (FabulorChannelContextAction action,
	const char *channel, gboolean state, gpointer user_data)
{
	ChannelContextProbe *probe = user_data;
	if ((guint)action < G_N_ELEMENTS (probe->counts))
		probe->counts[action]++;
	if (action == FABULOR_CHANNEL_CONTEXT_AUTOJOIN)
		probe->autojoin_state = state;
	probe->last_channel = channel;
}

static gboolean
check_channel_context_menu_model (void)
{
	GMenu *plugin = g_menu_new ();
	FabulorChannelContextMenuModel *model;
	GMenuModel *menu;
	GMenuModel *commands;
	GActionGroup *actions;
	GVariant *state;
	ChannelContextProbe probe = { { 0 }, FALSE, NULL };
	char *channel = g_strdup ("#retained");
	char *label = NULL;
	gboolean passed;

	g_menu_append (plugin, "Plugin", "fabulor-context.run");
	model = fabulor_channel_context_menu_model_new (channel, TRUE, FALSE,
		TRUE, FALSE, "Join", "Focus", "Part", "Cycle", "Autojoin",
		G_MENU_MODEL (plugin), channel_context_probe_dispatch, &probe);
	g_free (channel);
	g_object_unref (plugin);
	if (!model)
		return FALSE;
	menu = fabulor_channel_context_menu_model_get_menu (model);
	actions = fabulor_channel_context_menu_model_get_actions (model);
	passed = g_menu_model_get_n_items (menu) == 4;
	commands = g_menu_model_get_item_link (menu, 1, G_MENU_LINK_SECTION);
	passed = passed && commands && g_menu_model_get_n_items (commands) == 3 &&
		g_menu_model_get_item_attribute (commands, 0,
			G_MENU_ATTRIBUTE_LABEL, "s", &label) &&
		g_strcmp0 (label, "Focus") == 0;
	g_free (label);
	g_clear_object (&commands);
	g_action_group_activate_action (actions, "focus", NULL);
	g_action_group_activate_action (actions, "part", NULL);
	g_action_group_activate_action (actions, "cycle", NULL);
	g_action_group_activate_action (actions, "autojoin", NULL);
	state = g_action_group_get_action_state (actions, "autojoin");
	passed = passed && state && g_variant_get_boolean (state) &&
		probe.counts[FABULOR_CHANNEL_CONTEXT_FOCUS] == 1 &&
		probe.counts[FABULOR_CHANNEL_CONTEXT_PART] == 1 &&
		probe.counts[FABULOR_CHANNEL_CONTEXT_CYCLE] == 1 &&
		probe.counts[FABULOR_CHANNEL_CONTEXT_AUTOJOIN] == 1 &&
		probe.autojoin_state &&
		g_strcmp0 (probe.last_channel, "#retained") == 0;
	g_clear_pointer (&state, g_variant_unref);
	fabulor_channel_context_menu_model_free (model);

	memset (&probe, 0, sizeof (probe));
	model = fabulor_channel_context_menu_model_new ("#new", FALSE, FALSE,
		FALSE, FALSE, "Join", "Focus", "Part", "Cycle", "Autojoin",
		NULL, channel_context_probe_dispatch, &probe);
	if (!model)
		return FALSE;
	menu = fabulor_channel_context_menu_model_get_menu (model);
	actions = fabulor_channel_context_menu_model_get_actions (model);
	commands = g_menu_model_get_item_link (menu, 1, G_MENU_LINK_SECTION);
	passed = passed && g_menu_model_get_n_items (menu) == 2 && commands &&
		g_menu_model_get_n_items (commands) == 1 &&
		g_action_group_has_action (actions, "join") &&
		!g_action_group_has_action (actions, "autojoin");
	g_clear_object (&commands);
	g_action_group_activate_action (actions, "join", NULL);
	passed = passed && probe.counts[FABULOR_CHANNEL_CONTEXT_JOIN] == 1 &&
		g_strcmp0 (probe.last_channel, "#new") == 0;
	fabulor_channel_context_menu_model_free (model);
	return passed;
}

typedef struct
{
	guint counts[4];
	gboolean autojoin_state;
	const char *first_channel;
	const char *second_channel;
	const char *first_topic;
} ChannelListContextProbe;

static void
channel_list_context_probe_dispatch (FabulorChannelListContextAction action,
	gboolean state, const GPtrArray *channels, const GPtrArray *topics,
	gpointer user_data)
{
	ChannelListContextProbe *probe = user_data;

	if ((guint) action < G_N_ELEMENTS (probe->counts))
		probe->counts[action]++;
	if (action == FABULOR_CHANNEL_LIST_CONTEXT_AUTOJOIN)
		probe->autojoin_state = state;
	probe->first_channel = channels->len > 0 ?
		g_ptr_array_index (channels, 0) : NULL;
	probe->second_channel = channels->len > 1 ?
		g_ptr_array_index (channels, 1) : NULL;
	probe->first_topic = topics->len > 0 ?
		g_ptr_array_index (topics, 0) : NULL;
}

static gboolean
check_channel_list_context_menu_model (void)
{
	FabulorChannelListContextLabels labels = {
		"Join", "Copy channels", "Copy topics", "Autojoin",
		"list-add", "edit-copy"
	};
	FabulorChannelListContextMenuModel *model;
	ChannelListContextProbe probe = { { 0 }, FALSE, NULL, NULL, NULL };
	GPtrArray *channels = g_ptr_array_new_with_free_func (g_free);
	GPtrArray *topics = g_ptr_array_new_with_free_func (g_free);
	GMenuModel *menu;
	GMenuModel *commands;
	GActionGroup *actions;
	GVariant *state;
	char *label = NULL;
	gboolean passed;

	g_ptr_array_add (channels, g_strdup ("#owned-one"));
	g_ptr_array_add (channels, g_strdup ("#owned-two"));
	g_ptr_array_add (topics, g_strdup ("First topic"));
	g_ptr_array_add (topics, g_strdup ("Second topic"));
	model = fabulor_channel_list_context_menu_model_new (channels, topics,
		TRUE, FALSE, &labels, channel_list_context_probe_dispatch, &probe);
	g_ptr_array_unref (topics);
	g_ptr_array_unref (channels);
	if (!model)
		return FALSE;
	menu = fabulor_channel_list_context_menu_model_get_menu (model);
	actions = fabulor_channel_list_context_menu_model_get_actions (model);
	commands = g_menu_model_get_item_link (menu, 0, G_MENU_LINK_SECTION);
	passed = g_menu_model_get_n_items (menu) == 2 && commands &&
		g_menu_model_get_n_items (commands) == 3 &&
		g_menu_model_get_item_attribute (commands, 1,
			G_MENU_ATTRIBUTE_LABEL, "s", &label) &&
		g_strcmp0 (label, "Copy channels") == 0;
	g_free (label);
	g_clear_object (&commands);
	g_action_group_activate_action (actions, "join", NULL);
	g_action_group_activate_action (actions, "copy-channels", NULL);
	g_action_group_activate_action (actions, "copy-topics", NULL);
	g_action_group_activate_action (actions, "autojoin", NULL);
	state = g_action_group_get_action_state (actions, "autojoin");
	passed = passed && state && g_variant_get_boolean (state) &&
		probe.autojoin_state &&
		probe.counts[FABULOR_CHANNEL_LIST_CONTEXT_JOIN] == 1 &&
		probe.counts[FABULOR_CHANNEL_LIST_CONTEXT_COPY_CHANNELS] == 1 &&
		probe.counts[FABULOR_CHANNEL_LIST_CONTEXT_COPY_TOPICS] == 1 &&
		probe.counts[FABULOR_CHANNEL_LIST_CONTEXT_AUTOJOIN] == 1 &&
		g_strcmp0 (probe.first_channel, "#owned-one") == 0 &&
		g_strcmp0 (probe.second_channel, "#owned-two") == 0 &&
		g_strcmp0 (probe.first_topic, "First topic") == 0;
	g_clear_pointer (&state, g_variant_unref);
	fabulor_channel_list_context_menu_model_free (model);
	return passed;
}

typedef struct
{
	guint reply_count;
	guint command_count;
	guint copy_info_count;
	gboolean selection_dispatch;
	const char *last_nick;
	const char *last_command;
} NickContextProbe;

static void
nick_context_probe_dispatch (FabulorNickContextAction action,
	const char *nick, const char *command, gboolean selection_dispatch,
	gpointer user_data)
{
	NickContextProbe *probe = user_data;
	if (action == FABULOR_NICK_CONTEXT_REPLY)
		probe->reply_count++;
	else if (action == FABULOR_NICK_CONTEXT_COMMAND)
		probe->command_count++;
	else if (action == FABULOR_NICK_CONTEXT_COPY_INFO)
		probe->copy_info_count++;
	probe->selection_dispatch = selection_dispatch;
	probe->last_nick = nick;
	probe->last_command = command;
}

static gboolean
check_nick_context_menu_model (void)
{
	GMenu *plugin = g_menu_new ();
	FabulorNickContextMenuModel *model;
	GMenuModel *menu;
	GMenuModel *heading;
	GMenuModel *info_submenu;
	GMenuModel *handler_section;
	GMenuModel *handler_submenu;
	GActionGroup *actions;
	FabulorNickHandler handlers[] = {
		{ FABULOR_NICK_HANDLER_SUBMENU_BEGIN, "Tools", NULL, NULL, TRUE, FALSE },
		{ FABULOR_NICK_HANDLER_COMMAND, "Operate", "operate %s", "system-run", TRUE, FALSE },
		{ FABULOR_NICK_HANDLER_SEPARATOR, NULL, NULL, NULL, TRUE, FALSE },
		{ FABULOR_NICK_HANDLER_COMMAND, "Unavailable", "missing %s", NULL, FALSE, FALSE },
		{ FABULOR_NICK_HANDLER_TOGGLE, "Toggle", "gui_test", NULL, TRUE, FALSE },
		{ FABULOR_NICK_HANDLER_SUBMENU_END, NULL, NULL, NULL, TRUE, FALSE },
		{ FABULOR_NICK_HANDLER_COMMAND, "Inspect", "inspect %s", NULL, TRUE, FALSE }
	};
	NickContextProbe probe = { 0, 0, 0, FALSE, NULL, NULL };
	char *nick = g_strdup ("RetainedNick");
	char *reloadable_command = g_strdup ("inspect %s");
	char *label = NULL;
	char *action_name = NULL;
	GVariant *state;
	gboolean passed;

	g_menu_append (plugin, "Plugin", "fabulor-context.run");
	handlers[6].command = reloadable_command;
	model = fabulor_nick_context_menu_model_new_with_handlers (nick,
		"RetainedNick", TRUE, "Reply", TRUE, handlers,
		G_N_ELEMENTS (handlers), G_MENU_MODEL (plugin),
		nick_context_probe_dispatch, &probe);
	g_free (nick);
	g_free (reloadable_command);
	g_object_unref (plugin);
	if (!model)
		return FALSE;
	menu = fabulor_nick_context_menu_model_get_menu (model);
	actions = fabulor_nick_context_menu_model_get_actions (model);
	heading = g_menu_model_get_item_link (menu, 0, G_MENU_LINK_SECTION);
	passed = g_menu_model_get_n_items (menu) == 4 && heading &&
		g_menu_model_get_item_attribute (heading, 0, G_MENU_ATTRIBUTE_LABEL,
			"s", &label) && g_strcmp0 (label, "RetainedNick") == 0;
	g_free (label);
	g_clear_object (&heading);
	handler_section = g_menu_model_get_item_link (menu, 1, G_MENU_LINK_SECTION);
	handler_submenu = handler_section ? g_menu_model_get_item_link (
		handler_section, 0, G_MENU_LINK_SUBMENU) : NULL;
	passed = passed && handler_submenu &&
		g_menu_model_get_n_items (handler_submenu) == 2;
	g_clear_object (&handler_submenu);
	g_clear_object (&handler_section);
	g_action_group_activate_action (actions, "command-0", NULL);
	g_action_group_activate_action (actions, "command-1", NULL);
	g_action_group_activate_action (actions, "command-2", NULL);
	state = g_action_group_get_action_state (actions, "command-2");
	passed = passed && state && g_variant_get_boolean (state) &&
		!probe.selection_dispatch &&
		g_strcmp0 (probe.last_command, "set gui_test 1") == 0;
	g_clear_pointer (&state, g_variant_unref);
	g_action_group_activate_action (actions, "command-3", NULL);
	passed = passed && probe.command_count == 3 && probe.selection_dispatch &&
		g_strcmp0 (probe.last_command, "inspect %s") == 0;
	g_action_group_activate_action (actions, "reply", NULL);
	passed = passed && probe.reply_count == 1 && probe.command_count == 3 &&
		g_strcmp0 (probe.last_nick, "RetainedNick") == 0 &&
		probe.last_command == NULL;
	fabulor_nick_context_menu_model_free (model);

	{
		char *real_name_label = g_strdup ("Real Name: Retained Person");
		char *real_name_value = g_strdup ("Retained Person");
		char *account_label = g_strdup ("Account: retained-account");
		char *account_value = g_strdup ("retained-account");
		FabulorNickInfoItem info_items[] = {
			{ real_name_label, real_name_value },
			{ account_label, account_value },
			{ "Last Msg: 12 seconds ago", NULL }
		};

		model = fabulor_nick_context_menu_model_new_with_details (
			"DetailNick", "DetailNick", TRUE, "Reply", FALSE, NULL, 0,
			info_items, G_N_ELEMENTS (info_items), TRUE, NULL,
			nick_context_probe_dispatch, &probe);
		g_free (real_name_label);
		g_free (real_name_value);
		g_free (account_label);
		g_free (account_value);
	}
	if (!model)
		return FALSE;
	menu = fabulor_nick_context_menu_model_get_menu (model);
	actions = fabulor_nick_context_menu_model_get_actions (model);
	heading = g_menu_model_get_item_link (menu, 0, G_MENU_LINK_SECTION);
	info_submenu = heading ? g_menu_model_get_item_link (heading, 0,
		G_MENU_LINK_SUBMENU) : NULL;
	label = NULL;
	passed = passed && fabulor_nick_context_menu_model_needs_info_refresh (
		model) && g_menu_model_get_n_items (menu) == 2 && info_submenu &&
		g_menu_model_get_n_items (info_submenu) == 3 &&
		g_menu_model_get_item_attribute (info_submenu, 0,
			G_MENU_ATTRIBUTE_LABEL, "s", &label) &&
		g_strcmp0 (label, "Real Name: Retained Person") == 0 &&
		!g_menu_model_get_item_attribute (info_submenu, 2,
			G_MENU_ATTRIBUTE_ACTION, "s", &action_name);
	g_free (label);
	g_free (action_name);
	g_clear_object (&info_submenu);
	g_clear_object (&heading);
	g_action_group_activate_action (actions, "info-0", NULL);
	passed = passed && probe.copy_info_count == 1 &&
		g_strcmp0 (probe.last_nick, "DetailNick") == 0 &&
		g_strcmp0 (probe.last_command, "Retained Person") == 0 &&
		!probe.selection_dispatch;
	g_action_group_activate_action (actions, "info-1", NULL);
	passed = passed && probe.copy_info_count == 2 &&
		g_strcmp0 (probe.last_command, "retained-account") == 0;
	fabulor_nick_context_menu_model_free (model);

	model = fabulor_nick_context_menu_model_new ("FirstNick",
		"3 nicks selected.", FALSE, NULL, NULL, nick_context_probe_dispatch,
		&probe);
	if (!model)
		return FALSE;
	menu = fabulor_nick_context_menu_model_get_menu (model);
	actions = fabulor_nick_context_menu_model_get_actions (model);
	passed = passed && g_menu_model_get_n_items (menu) == 1 &&
		!g_action_group_has_action (actions, "reply") &&
		!fabulor_nick_context_menu_model_needs_info_refresh (model);
	fabulor_nick_context_menu_model_free (model);
	return passed;
}

typedef struct
{
	guint counts[7];
	FabulorTabOption last_option;
	gboolean last_state;
	const char *last_command;
} TabContextProbe;

static void
tab_context_probe_dispatch (FabulorTabContextAction action,
	FabulorTabOption option, gboolean state, const char *command,
	gpointer user_data)
{
	TabContextProbe *probe = user_data;
	if ((guint)action < G_N_ELEMENTS (probe->counts))
		probe->counts[action]++;
	probe->last_option = option;
	probe->last_state = state;
	probe->last_command = command;
}

static gboolean
check_tab_context_menu_model (void)
{
	FabulorTabContextLabels labels = {
		"Alerts", "Settings", "Notify", "Beep", "Tray", "Taskbar",
		"Logging", "Scrollback", "Strip colors", "Hide joins",
		"Autojoin", "Auto-connect", "Detach", "Close"
	};
	FabulorTabContextState state = { 0 };
	FabulorTabConfiguredItem configured[] = {
		{ FABULOR_TAB_CONFIG_COMMAND, "Who", "WHO $NICK", "system-search", FALSE },
		{ FABULOR_TAB_CONFIG_SUBMENU_BEGIN, "Tools", NULL, NULL, FALSE },
		{ FABULOR_TAB_CONFIG_TOGGLE, "Muted", "quiet", NULL, TRUE },
		{ FABULOR_TAB_CONFIG_SUBMENU_END, NULL, NULL, NULL, FALSE },
		{ FABULOR_TAB_CONFIG_SEPARATOR, NULL, NULL, NULL, FALSE },
		{ FABULOR_TAB_CONFIG_COMMAND, "Cycle", "CYCLE", NULL, FALSE }
	};
	GMenu *plugin = g_menu_new ();
	FabulorTabContextMenuModel *model;
	GMenuModel *menu;
	GMenuModel *option_section;
	GMenuModel *alerts;
	GActionGroup *actions;
	GVariant *value;
	TabContextProbe probe = { { 0 }, FABULOR_TAB_OPTION_COUNT, FALSE, NULL };
	char *label = NULL;
	gboolean passed;

	state.has_session = TRUE;
	state.is_channel = TRUE;
	state.has_network = TRUE;
	state.autojoin = FALSE;
	state.options[FABULOR_TAB_OPTION_NOTIFICATION] = TRUE;
	state.options[FABULOR_TAB_OPTION_LOGGING] = TRUE;
	g_menu_append (plugin, "Plugin", "fabulor-context.run");
	model = fabulor_tab_context_menu_model_new ("#retained", &state, &labels,
		configured, G_N_ELEMENTS (configured), G_MENU_MODEL (plugin),
		tab_context_probe_dispatch, &probe);
	g_object_unref (plugin);
	if (!model)
		return FALSE;
	menu = fabulor_tab_context_menu_model_get_menu (model);
	actions = fabulor_tab_context_menu_model_get_actions (model);
	option_section = g_menu_model_get_item_link (menu, 1, G_MENU_LINK_SECTION);
	alerts = option_section ? g_menu_model_get_item_link (option_section, 0,
		G_MENU_LINK_SUBMENU) : NULL;
	passed = g_menu_model_get_n_items (menu) == 7 && option_section && alerts &&
		g_menu_model_get_n_items (option_section) == 2 &&
		g_menu_model_get_n_items (alerts) == 4 &&
		g_menu_model_get_item_attribute (alerts, 0, G_MENU_ATTRIBUTE_LABEL,
			"s", &label) && g_strcmp0 (label, "Notify") == 0 &&
		g_action_group_has_action (actions, "option-0") &&
		g_action_group_has_action (actions, "autojoin") &&
		g_action_group_has_action (actions, "detach") &&
		g_action_group_has_action (actions, "close") &&
		g_action_group_has_action (actions, "configured-0") &&
		g_action_group_has_action (actions, "configured-1");
	g_free (label);
	g_clear_object (&alerts);
	g_clear_object (&option_section);
	g_action_group_activate_action (actions, "option-0", NULL);
	value = g_action_group_get_action_state (actions, "option-0");
	passed = passed && value && !g_variant_get_boolean (value) &&
		probe.counts[FABULOR_TAB_CONTEXT_OPTION] == 1 &&
		probe.last_option == FABULOR_TAB_OPTION_NOTIFICATION &&
		!probe.last_state;
	g_clear_pointer (&value, g_variant_unref);
	g_action_group_activate_action (actions, "autojoin", NULL);
	g_action_group_activate_action (actions, "detach", NULL);
	g_action_group_activate_action (actions, "close", NULL);
	g_action_group_activate_action (actions, "configured-0", NULL);
	passed = passed && probe.counts[FABULOR_TAB_CONTEXT_AUTOJOIN] == 1 &&
		probe.counts[FABULOR_TAB_CONTEXT_DETACH] == 1 &&
		probe.counts[FABULOR_TAB_CONTEXT_CLOSE] == 1 &&
		probe.counts[FABULOR_TAB_CONTEXT_COMMAND] == 1 &&
		g_strcmp0 (probe.last_command, "WHO $NICK") == 0;
	g_action_group_activate_action (actions, "configured-1", NULL);
	passed = passed && probe.counts[FABULOR_TAB_CONTEXT_TOGGLE] == 1 &&
		!probe.last_state && g_strcmp0 (probe.last_command, "quiet") == 0;
	fabulor_tab_context_menu_model_free (model);

	memset (&state, 0, sizeof (state));
	memset (&probe, 0, sizeof (probe));
	model = fabulor_tab_context_menu_model_new (NULL, &state, &labels,
		NULL, 0, NULL, tab_context_probe_dispatch, &probe);
	if (!model)
		return FALSE;
	menu = fabulor_tab_context_menu_model_get_menu (model);
	actions = fabulor_tab_context_menu_model_get_actions (model);
	passed = passed && g_menu_model_get_n_items (menu) == 1 &&
		g_action_group_has_action (actions, "detach") &&
		g_action_group_has_action (actions, "close") &&
		!g_action_group_has_action (actions, "option-0");
	g_action_group_activate_action (actions, "detach", NULL);
	passed = passed && probe.counts[FABULOR_TAB_CONTEXT_DETACH] == 1;
	fabulor_tab_context_menu_model_free (model);
	return passed;
}

static gboolean
check_middle_context_menu_model (void)
{
	GMenu *fabulor = g_menu_new ();
	GMenu *view = g_menu_new ();
	GMenu *plugin_root = g_menu_new ();
	GMenu *plugin_fabulor = g_menu_new ();
	GMenu *plugin_tools = g_menu_new ();
	FabulorMiddleContextMenuModel *model;
	FabulorMiddleContextSection sections[2];
	GMenuModel *menu;
	GMenuModel *fabulor_submenu;
	GMenuModel *plugin_section;
	char *fabulor_label = g_strdup ("Fabulor");
	char *view_label = g_strdup ("View");
	char *label = NULL;
	gboolean passed;

	g_menu_append (fabulor, "Networks", "fabulor.networks");
	g_menu_append (view, "Menu Bar", "fabulor.menu-bar");
	g_menu_append (plugin_fabulor, "Plugin Command", "fabulor.plugin-0");
	g_menu_append (plugin_tools, "Tool Command", "fabulor.plugin-1");
	g_menu_append_submenu (plugin_root, "_Fabulor",
		G_MENU_MODEL (plugin_fabulor));
	g_menu_append_submenu (plugin_root, "Tools", G_MENU_MODEL (plugin_tools));
	sections[0].label = fabulor_label;
	sections[0].plugin_path = "Fabulor";
	sections[0].model = G_MENU_MODEL (fabulor);
	sections[1].label = view_label;
	sections[1].plugin_path = "View";
	sections[1].model = G_MENU_MODEL (view);
	model = fabulor_middle_context_menu_model_new (sections,
		G_N_ELEMENTS (sections), G_MENU_MODEL (plugin_root));
	g_free (fabulor_label);
	g_free (view_label);
	g_object_unref (fabulor);
	g_object_unref (view);
	g_object_unref (plugin_fabulor);
	g_object_unref (plugin_tools);
	g_object_unref (plugin_root);
	if (!model)
		return FALSE;
	menu = fabulor_middle_context_menu_model_get_menu (model);
	fabulor_submenu = g_menu_model_get_item_link (menu, 0,
		G_MENU_LINK_SUBMENU);
	plugin_section = fabulor_submenu ? g_menu_model_get_item_link (
		fabulor_submenu, 1, G_MENU_LINK_SECTION) : NULL;
	passed = g_menu_model_get_n_items (menu) == 3 && fabulor_submenu &&
		g_menu_model_get_n_items (fabulor_submenu) == 2 && plugin_section &&
		g_menu_model_get_n_items (plugin_section) == 1 &&
		g_menu_model_get_item_attribute (plugin_section, 0,
			G_MENU_ATTRIBUTE_LABEL, "s", &label) &&
		g_strcmp0 (label, "Plugin Command") == 0;
	g_free (label);
	label = NULL;
	passed = passed && g_menu_model_get_item_attribute (menu, 2,
		G_MENU_ATTRIBUTE_LABEL, "s", &label) &&
		g_strcmp0 (label, "Tools") == 0;
	g_free (label);
	g_clear_object (&plugin_section);
	g_clear_object (&fabulor_submenu);
	fabulor_middle_context_menu_model_free (model);
	return passed;
}

static gboolean
check_main_menu_bar_projection_gtk4 (void)
{
	GMenu *fabulor = g_menu_new ();
	GMenu *replacement = g_menu_new ();
	GMenu *replacement_content = g_menu_new ();
	GActionGroup *actions;
	FabulorMiddleContextMenuModel *model;
	FabulorMiddleContextSection section;
	GtkWidget *menu_bar;
	guint activation_count = 0;
	gboolean passed;

	g_menu_append (fabulor, "Run", "fabulor.run");
	section.label = "Fabulor";
	section.plugin_path = "Fabulor";
	section.model = G_MENU_MODEL (fabulor);
	model = fabulor_middle_context_menu_model_new (&section, 1, NULL);
	g_object_unref (fabulor);
	if (!model)
		return FALSE;

	menu_bar = gtk_popover_menu_bar_new_from_model (
		fabulor_middle_context_menu_model_get_menu (model));
	g_object_ref_sink (menu_bar);
	actions = tray_presenter_action_group_new ("run", &activation_count);
	gtk_widget_insert_action_group (menu_bar, "fabulor", actions);
	g_object_unref (actions);
	passed = gtk_popover_menu_bar_get_menu_model (
		GTK_POPOVER_MENU_BAR (menu_bar)) ==
		fabulor_middle_context_menu_model_get_menu (model) &&
		gtk_widget_activate_action (menu_bar, "fabulor.run", NULL) &&
		activation_count == 1;

	g_menu_append (replacement_content, "Run again", "fabulor.run");
	g_menu_append_submenu (replacement, "Replacement",
		G_MENU_MODEL (replacement_content));
	gtk_popover_menu_bar_set_menu_model (GTK_POPOVER_MENU_BAR (menu_bar),
		G_MENU_MODEL (replacement));
	passed = passed && gtk_popover_menu_bar_get_menu_model (
		GTK_POPOVER_MENU_BAR (menu_bar)) == G_MENU_MODEL (replacement) &&
		gtk_widget_activate_action (menu_bar, "fabulor.run", NULL) &&
		activation_count == 2;
	g_object_unref (replacement_content);
	g_object_unref (replacement);
	gtk_popover_menu_bar_set_menu_model (GTK_POPOVER_MENU_BAR (menu_bar), NULL);
	gtk_widget_insert_action_group (menu_bar, "fabulor", NULL);
	g_object_unref (menu_bar);
	fabulor_middle_context_menu_model_free (model);
	return passed;
}

static gboolean
check_spell_entry_style_policy (void)
{
	FabulorSpellEntryPalette palette = {
		{ 100, 200, 300 }, { 400, 500, 600 }, { 700, 800, 900 },
		probe_spell_mirc_color, NULL
	};
	PangoAttrList *attributes;
	gboolean valid;

	attributes = fabulor_spell_entry_style_build ("plain", FALSE, &palette);
	valid = !fabulor_spell_entry_style_has_attributes (attributes);
	pango_attr_list_unref (attributes);

	attributes = fabulor_spell_entry_style_build ("a\002b\002c", TRUE,
		&palette);
	valid = valid && fabulor_spell_entry_style_has_attributes (attributes) &&
		spell_style_has_type (attributes, PANGO_ATTR_SHAPE, 1) &&
		spell_style_has_int (attributes, PANGO_ATTR_WEIGHT, 2,
			PANGO_WEIGHT_BOLD) &&
		spell_style_has_int (attributes, PANGO_ATTR_WEIGHT, 4,
			PANGO_WEIGHT_NORMAL);
	pango_attr_list_unref (attributes);
	attributes = fabulor_spell_entry_style_build ("\002b\017n", TRUE,
		&palette);
	valid = valid && spell_style_has_int (attributes, PANGO_ATTR_WEIGHT, 3,
		PANGO_WEIGHT_NORMAL) && spell_style_has_color (attributes,
		PANGO_ATTR_FOREGROUND, 3, 100, 200, 300);
	pango_attr_list_unref (attributes);

	attributes = fabulor_spell_entry_style_build ("\035i\036s\037u", TRUE,
		&palette);
	valid = valid && spell_style_has_int (attributes, PANGO_ATTR_STYLE, 5,
		PANGO_STYLE_ITALIC) && spell_style_has_int (attributes,
		PANGO_ATTR_STRIKETHROUGH, 5, TRUE) && spell_style_has_int (
		attributes, PANGO_ATTR_UNDERLINE, 5, PANGO_UNDERLINE_SINGLE);
	pango_attr_list_unref (attributes);

	attributes = fabulor_spell_entry_style_build ("x\003" "04,12y", TRUE,
		&palette);
	valid = valid && spell_style_has_type (attributes, PANGO_ATTR_SHAPE, 2) &&
		spell_style_has_color (attributes,
		PANGO_ATTR_FOREGROUND, 7, 1004, 2004, 3004) &&
		spell_style_has_color (attributes, PANGO_ATTR_BACKGROUND, 7,
			1012, 2012, 3012);
	pango_attr_list_unref (attributes);
	attributes = fabulor_spell_entry_style_build ("x\003" "04", TRUE,
		&palette);
	valid = valid && spell_style_has_type (attributes, PANGO_ATTR_SHAPE, 2);
	pango_attr_list_unref (attributes);

	attributes = fabulor_spell_entry_style_build ("\026x", TRUE, &palette);
	valid = valid && spell_style_has_color (attributes,
		PANGO_ATTR_FOREGROUND, 1, 400, 500, 600) &&
		spell_style_has_color (attributes, PANGO_ATTR_BACKGROUND, 1,
			100, 200, 300);
	fabulor_spell_entry_style_add_misspelling (attributes, 1, 2, &palette);
	valid = valid && spell_style_has_int (attributes, PANGO_ATTR_UNDERLINE, 1,
		PANGO_UNDERLINE_ERROR) && spell_style_has_color (attributes,
		PANGO_ATTR_UNDERLINE_COLOR, 1, 700, 800, 900);
	pango_attr_list_unref (attributes);
	return valid;
}

static gboolean
check_spell_entry_widget_policy (void)
{
	GType entry_type = g_type_from_name ("FabulorGtk4ProbeSpellEntry");

	if (entry_type == G_TYPE_INVALID)
	{
		entry_type = g_type_register_static_simple (GTK_TYPE_ENTRY,
			"FabulorGtk4ProbeSpellEntry", sizeof (GtkEntryClass), NULL,
			sizeof (GtkEntry), NULL, 0);
	}

	return g_type_is_a (entry_type, GTK_TYPE_ENTRY) &&
		g_type_is_a (entry_type, GTK_TYPE_EDITABLE);
}

static guint
spell_menu_count_action (GMenuModel *model, const gchar *expected_action)
{
	guint count = 0;
	gint i;

	for (i = 0; i < g_menu_model_get_n_items (model); i++)
	{
		GVariant *action = g_menu_model_get_item_attribute_value (model, i,
			G_MENU_ATTRIBUTE_ACTION, G_VARIANT_TYPE_STRING);
		GMenuModel *submenu;
		GMenuModel *section;

		if (action && g_strcmp0 (g_variant_get_string (action, NULL),
			expected_action) == 0)
			count++;
		g_clear_pointer (&action, g_variant_unref);

		submenu = g_menu_model_get_item_link (model, i, G_MENU_LINK_SUBMENU);
		if (submenu)
		{
			count += spell_menu_count_action (submenu, expected_action);
			g_object_unref (submenu);
		}
		section = g_menu_model_get_item_link (model, i, G_MENU_LINK_SECTION);
		if (section)
		{
			count += spell_menu_count_action (section, expected_action);
			g_object_unref (section);
		}
	}

	return count;
}

static gboolean
check_spell_entry_menu_policy (void)
{
	static const gchar *english_suggestions[] = {
		"one", "two", "three", "four", "five", "six", "seven", "eight",
		"nine", "ten", "eleven"
	};
	FabulorSpellMenuDictionary dictionaries[] = {
		{ "en_AU", "English (Australia)", english_suggestions,
			G_N_ELEMENTS (english_suggestions) },
		{ "fr_FR", "French (France)", NULL, 0 }
	};
	GMenuModel *model = fabulor_spell_entry_menu_new ("wrod", TRUE, TRUE,
		dictionaries, G_N_ELEMENTS (dictionaries));
	gboolean valid;

	valid = g_menu_model_get_n_items (model) == 3 &&
		spell_menu_count_action (model,
			FABULOR_SPELL_MENU_ACTION_REPLACE) == 11 &&
		spell_menu_count_action (model,
			FABULOR_SPELL_MENU_ACTION_ADD) == 2 &&
		spell_menu_count_action (model,
			FABULOR_SPELL_MENU_ACTION_IGNORE) == 1 &&
		spell_menu_count_action (model,
			FABULOR_SPELL_MENU_ACTION_INSERT) == 21 &&
		spell_menu_count_action (model,
			FABULOR_SPELL_MENU_ACTION_ENABLED) == 1;
	g_object_unref (model);

	model = fabulor_spell_entry_menu_new (NULL, FALSE, FALSE, NULL, 0);
	valid = valid && g_menu_model_get_n_items (model) == 2 &&
		spell_menu_count_action (model,
			FABULOR_SPELL_MENU_ACTION_REPLACE) == 0 &&
		spell_menu_count_action (model,
			FABULOR_SPELL_MENU_ACTION_INSERT) == 21 &&
		spell_menu_count_action (model,
			FABULOR_SPELL_MENU_ACTION_ENABLED) == 1;
	g_object_unref (model);

	return valid;
}

static gboolean
check_spell_entry_word_policy (void)
{
	FabulorSpellWords *words = fabulor_spell_words_new (
		"alpha caf\303\251 omega", pango_language_from_string ("en"));
	FabulorSpellWordRange range;
	const gchar *mixed = "hello <https://example.com/path?q=caf\303\251> \360\237\230\200 world";
	const gchar *example;
	guint example_position;
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

	words = fabulor_spell_words_new (mixed,
		pango_language_from_string ("en"));
	example = strstr (mixed, "example");
	example_position = example ?
		(guint) g_utf8_pointer_to_offset (mixed, example) : G_MAXUINT;
	valid = valid && example != NULL &&
		fabulor_spell_words_find_character (words, example_position, &range) &&
		fabulor_spell_words_range_is_uri (words, &range) &&
		fabulor_spell_words_find_character (words, 1, &range) &&
		!fabulor_spell_words_range_is_uri (words, &range);
	fabulor_spell_words_free (words);

	words = fabulor_spell_words_new ("www.example.test/path",
		pango_language_from_string ("en"));
	valid = valid &&
		fabulor_spell_words_find_character (words, 5, &range) &&
		fabulor_spell_words_range_is_uri (words, &range);
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
	printf ("Transcript policy diagnostic: %u decisions in %.0f us\n",
		iterations, (double) elapsed);
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

static void
probe_window_state_cb (GtkWindow *window, const FabulorWindowState *state,
	gpointer user_data)
{
	guint *count = user_data;
	(void)window;
	(void)state;
	(*count)++;
}

static gboolean
check_window_state_boundary (gboolean gtk_ready)
{
	FabulorWindowState previous = { 0 };
	FabulorWindowState current = { 0 };
	guint changed;

	current.maximized = TRUE;
	current.fullscreen = TRUE;
	current.focused = TRUE;
	changed = fabulor_window_state_changes (&previous, &current);
	if (changed != (FABULOR_WINDOW_STATE_MAXIMIZED |
		FABULOR_WINDOW_STATE_FULLSCREEN | FABULOR_WINDOW_STATE_FOCUSED))
		return FALSE;
	previous = current;
	current.maximized = FALSE;
	current.fullscreen = FALSE;
	current.minimized = TRUE;
	current.focused = FALSE;
	changed = fabulor_window_state_changes (&previous, &current);
	if (changed != (FABULOR_WINDOW_STATE_MINIMIZED |
		FABULOR_WINDOW_STATE_MAXIMIZED |
		FABULOR_WINDOW_STATE_FULLSCREEN | FABULOR_WINDOW_STATE_FOCUSED))
		return FALSE;
	if (gtk_ready)
	{
		GtkWindow *window = GTK_WINDOW (gtk_window_new ());
		FabulorWindowState state;
		guint callback_count = 0;
		fabulor_window_state_get (window, &state);
		if (state.changed || state.minimized || state.maximized ||
			state.fullscreen || state.focused)
		{
			gtk_window_destroy (window);
			return FALSE;
		}
		fabulor_window_state_watch (window, probe_window_state_cb,
			&callback_count);
		fabulor_window_state_watch (window, probe_window_state_cb,
			&callback_count);
		fabulor_window_state_allow_autohide_taskbar (window, &state);
		gtk_window_destroy (window);
		if (callback_count != 0)
			return FALSE;
	}
	return TRUE;
}

typedef struct
{
	guint count;
	FabulorWindowGeometry geometry;
} ProbeWindowGeometry;

static void
probe_window_geometry_cb (GtkWindow *window,
	const FabulorWindowGeometry *geometry, gpointer user_data)
{
	ProbeWindowGeometry *probe = user_data;
	(void)window;
	probe->count++;
	probe->geometry = *geometry;
}

static gboolean
check_window_geometry_boundary (gboolean gtk_ready)
{
	GtkWindow *window;
	FabulorWindowGeometry geometry;
	ProbeWindowGeometry probe = { 0 };

	if (!gtk_ready)
		return TRUE;
	window = GTK_WINDOW (fabulor_gtk_window_new ());
	g_object_ref_sink (window);
	fabulor_window_geometry_get (window, &geometry);
	if (geometry.width != 0 || geometry.height != 0 || geometry.has_position)
	{
		g_object_unref (window);
		return FALSE;
	}
	fabulor_window_geometry_watch (window, probe_window_geometry_cb, &probe);
	gtk_window_set_default_size (window, 320, 180);
	gtk_window_present (window);
	probe_run_pending_main_context ();
	fabulor_window_geometry_get (window, &geometry);
	gtk_window_destroy (window);
	g_object_unref (window);
	probe_run_pending_main_context ();
	return probe.count > 0 && probe.geometry.width > 0 &&
		probe.geometry.height > 0 && !probe.geometry.has_position &&
		geometry.width > 0 && geometry.height > 0 && !geometry.has_position;
}

typedef struct
{
	FabulorApplicationMainLoop *owner;
	guint quit_count;
} ApplicationMainLoopProbe;

static gboolean
application_main_loop_quit_cb (gpointer user_data)
{
	ApplicationMainLoopProbe *probe = user_data;

	probe->quit_count++;
	fabulor_application_main_loop_request_quit (probe->owner);
	return G_SOURCE_REMOVE;
}

static gboolean
check_application_main_loop (void)
{
	ApplicationMainLoopProbe probe = { 0 };
	FabulorApplicationMainLoop *prequit;
	gboolean passed;

	prequit = fabulor_application_main_loop_new ();
	fabulor_application_main_loop_request_quit (prequit);
	fabulor_application_main_loop_run (prequit);
	passed = !fabulor_application_main_loop_is_running (prequit);
	fabulor_application_main_loop_free (prequit);

	probe.owner = fabulor_application_main_loop_new ();
	g_idle_add (application_main_loop_quit_cb, &probe);
	fabulor_application_main_loop_run (probe.owner);
	passed = passed && probe.quit_count == 1 &&
		!fabulor_application_main_loop_is_running (probe.owner);
	fabulor_application_main_loop_free (probe.owner);
	return passed;
}

int
main (void)
{
	gboolean gtk_ready = gtk_init_check ();
	g_log_set_always_fatal (G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL);

	check_compatibility_helper_signatures ();
	check_user_list_view_signatures ();
	check_channel_tree_view_signatures ();
	if (!check_application_main_loop ())
	{
		fprintf (stderr, "GTK4 application main-loop contract mismatch\n");
		return 1;
	}
	if (!check_dialog_icon (gtk_ready))
	{
		fprintf (stderr, "GTK4 dialog icon contract mismatch\n");
		return 1;
	}
	if (!check_icon_sizes (gtk_ready))
	{
		fprintf (stderr, "GTK4 icon size contract mismatch\n");
		return 1;
	}
	if (!check_icon_theme_compatibility (gtk_ready))
	{
		fprintf (stderr, "GTK4 icon-theme compatibility mismatch\n");
		return 1;
	}
	if (!check_button_box_layouts (gtk_ready))
	{
		fprintf (stderr, "GTK4 button box layout contract mismatch\n");
		return 1;
	}
	if (!check_container_uniform_insets (gtk_ready))
	{
		fprintf (stderr, "GTK4 container inset ownership mismatch\n");
		return 1;
	}
	if (!check_content_and_list_ownership (gtk_ready))
	{
		fprintf (stderr, "GTK4 content or list ownership mismatch\n");
		return 1;
	}
	if (!check_box_reorder_ownership (gtk_ready))
	{
		fprintf (stderr, "GTK4 box reorder ownership mismatch\n");
		return 1;
	}
	if (!check_layout_reparent_ownership (gtk_ready))
	{
		fprintf (stderr, "GTK4 layout reparent ownership mismatch\n");
		return 1;
	}
	if (!check_entry_text (gtk_ready))
	{
		fprintf (stderr, "GTK4 entry text contract mismatch\n");
		return 1;
	}
	if (!check_flat_button (gtk_ready))
	{
		fprintf (stderr, "GTK4 flat button contract mismatch\n");
		return 1;
	}
	if (!check_icon_button (gtk_ready))
	{
		fprintf (stderr, "GTK4 icon button contract mismatch\n");
		return 1;
	}
	if (!check_icon_mnemonic_button (gtk_ready))
	{
		fprintf (stderr, "GTK4 icon mnemonic button contract mismatch\n");
		return 1;
	}
	if (!check_choice_buttons_and_root_window (gtk_ready))
	{
		fprintf (stderr, "GTK4 choice or root-window contract mismatch\n");
		return 1;
	}
	if (!check_frame_presentation (gtk_ready))
	{
		fprintf (stderr, "GTK4 frame presentation contract mismatch\n");
		return 1;
	}
	if (!check_window_state_boundary (gtk_ready))
	{
		fprintf (stderr, "GTK4 window state boundary mismatch\n");
		return 1;
	}
	if (!check_window_geometry_boundary (gtk_ready))
	{
		fprintf (stderr, "GTK4 window geometry boundary mismatch\n");
		return 1;
	}
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
	if (!check_emoji_picker_policy ())
	{
		fprintf (stderr, "GTK4 emoji-picker ownership policy mismatch\n");
		return 1;
	}
	if (!check_gtk3_theme_adapter_containment ())
	{
		fprintf (stderr, "GTK3 theme adapter containment mismatch\n");
		return 1;
	}
	if (!check_gtk4_theme_discovery_policy ())
	{
		fprintf (stderr, "GTK4 theme discovery policy mismatch\n");
		return 1;
	}
	if (!check_gtk4_theme_preferences_policy ())
	{
		fprintf (stderr, "GTK4 theme preferences policy mismatch\n");
		return 1;
	}
	if (!check_gtk4_theme_adapter_policy ())
	{
		fprintf (stderr, "GTK4 theme adapter policy mismatch\n");
		return 1;
	}
	if (!check_gtk4_theme_controller_policy ())
	{
		fprintf (stderr, "GTK4 theme controller policy mismatch\n");
		return 1;
	}
	if (!check_gtk4_theme_preferences_binding ())
	{
		fprintf (stderr, "GTK4 theme preferences binding mismatch\n");
		return 1;
	}
	if (!check_gtk4_theme_appearance_monitor ())
	{
		fprintf (stderr, "GTK4 theme appearance monitor mismatch\n");
		return 1;
	}
	if (!check_tray_action_model ())
	{
		fprintf (stderr, "GTK4 tray action model contract mismatch\n");
		return 1;
	}
	if (!check_tray_backend_policy ())
	{
		fprintf (stderr, "GTK4 tray backend policy mismatch\n");
		return 1;
	}
	if (!check_tray_menu_composition ())
	{
		fprintf (stderr, "GTK4 tray menu composition contract mismatch\n");
		return 1;
	}
	if (gtk_ready && !check_tray_menu_presenter_gtk4 ())
	{
		fprintf (stderr, "GTK4 tray menu presenter contract mismatch\n");
		return 1;
	}
	if (gtk_ready && !check_context_menu_presenter_gtk4 ())
	{
		fprintf (stderr, "GTK4 context menu presenter contract mismatch\n");
		return 1;
	}
	if (!check_url_context_menu_model ())
	{
		fprintf (stderr, "GTK4 URL context menu model mismatch\n");
		return 1;
	}
	if (!check_channel_context_menu_model ())
	{
		fprintf (stderr, "GTK4 channel context menu model mismatch\n");
		return 1;
	}
	if (!check_channel_list_context_menu_model ())
	{
		fprintf (stderr, "GTK4 channel-list context menu model mismatch\n");
		return 1;
	}
	if (!check_tab_context_menu_model ())
	{
		fprintf (stderr, "GTK4 tab context menu model mismatch\n");
		return 1;
	}
	if (!check_nick_context_menu_model ())
	{
		fprintf (stderr, "GTK4 nick context menu model mismatch\n");
		return 1;
	}
	if (!check_middle_context_menu_model ())
	{
		fprintf (stderr, "GTK4 middle context menu model mismatch\n");
		return 1;
	}
	if (gtk_ready && !check_main_menu_bar_projection_gtk4 ())
	{
		fprintf (stderr, "GTK4 main menu bar projection mismatch\n");
		return 1;
	}
	if (!check_spell_entry_word_policy ())
	{
		fprintf (stderr, "GTK4 spell-entry word policy mismatch\n");
		return 1;
	}
	if (!check_spell_entry_widget_policy ())
	{
		fprintf (stderr, "GTK4 spell-entry widget policy mismatch\n");
		return 1;
	}
	if (!check_spell_entry_menu_policy ())
	{
		fprintf (stderr, "GTK4 spell-entry menu policy mismatch\n");
		return 1;
	}
	if (!check_spell_entry_style_policy ())
	{
		fprintf (stderr, "GTK4 spell-entry style policy mismatch\n");
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
