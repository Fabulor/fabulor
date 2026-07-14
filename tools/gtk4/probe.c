#include <gtk/gtk.h>

#include <stdio.h>

#include "../../src/fe-gtk/gtk-compat.h"

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

int
main (void)
{
	check_compatibility_helper_signatures ();

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
