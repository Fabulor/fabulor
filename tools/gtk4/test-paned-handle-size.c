/* Pane geometry must not mix pending positions with old allocations. */
#include "../../src/fe-gtk/gtk-compat.h"

static void
allocate_paned (GtkWidget *paned)
{
	gint minimum, natural;
	gtk_widget_measure (paned, GTK_ORIENTATION_HORIZONTAL, -1,
		&minimum, &natural, NULL, NULL);
	gtk_widget_measure (paned, GTK_ORIENTATION_VERTICAL, 1200,
		&minimum, &natural, NULL, NULL);
	gtk_widget_allocate (paned, 1200, 1200, -1, NULL);
}

static void
test_handle_during_position_change (gconstpointer data)
{
	gint mode = GPOINTER_TO_INT (data);
	GtkOrientation orientation = mode & 1;
	GtkWidget *paned = gtk_paned_new (orientation);
	GtkWidget *start = gtk_label_new ("Transcript");
	GtkWidget *end = gtk_label_new ("Users");
	gint handle, pending;

	g_object_ref_sink (paned);
	gtk_paned_set_wide_handle (GTK_PANED (paned), (mode & 2) != 0);
	fabulor_gtk_paned_set_start_child (GTK_PANED (paned), start, TRUE, TRUE);
	fabulor_gtk_paned_set_end_child (GTK_PANED (paned), end, FALSE, TRUE);
	gtk_paned_set_position (GTK_PANED (paned), 800);
	allocate_paned (paned);
	handle = fabulor_gtk_paned_get_handle_size (GTK_PANED (paned));
	g_test_message ("Initial divider thickness: %d", handle);
	g_assert_cmpint (handle, >, 0);
	g_assert_cmpint (handle, <, 100);
	g_assert_cmpint (1200 -
		(orientation == GTK_ORIENTATION_HORIZONTAL ?
			gtk_widget_get_width (start) + gtk_widget_get_width (end) :
			gtk_widget_get_height (start) + gtk_widget_get_height (end)), ==, handle);

	/* notify::position runs before the children receive their new allocation. */
	gtk_paned_set_position (GTK_PANED (paned), 650);
	pending = fabulor_gtk_paned_get_handle_size (GTK_PANED (paned));
	g_test_message ("Pending position divider thickness: %d", pending);
	g_assert_cmpint (pending, ==, handle);
	allocate_paned (paned);
	g_assert_cmpint (fabulor_gtk_paned_get_handle_size (GTK_PANED (paned)), ==, handle);

	gtk_widget_set_visible (end, FALSE);
	allocate_paned (paned);
	gtk_widget_set_visible (end, TRUE);
	gtk_paned_set_position (GTK_PANED (paned), 1050 - handle);
	g_assert_cmpint (fabulor_gtk_paned_get_handle_size (GTK_PANED (paned)), ==, handle);
	allocate_paned (paned);
	g_assert_cmpint (fabulor_gtk_paned_get_handle_size (GTK_PANED (paned)), ==, handle);
	g_assert_cmpint (orientation == GTK_ORIENTATION_HORIZONTAL ?
		gtk_widget_get_width (end) : gtk_widget_get_height (end), ==, 150);
	g_object_unref (paned);
}

int
main (int argc, char **argv)
{
	g_test_init (&argc, &argv, NULL);
	gtk_init ();
	g_test_add_data_func ("/paned/handle/horizontal", GINT_TO_POINTER (GTK_ORIENTATION_HORIZONTAL),
		test_handle_during_position_change);
	g_test_add_data_func ("/paned/handle/vertical", GINT_TO_POINTER (GTK_ORIENTATION_VERTICAL),
		test_handle_during_position_change);
	g_test_add_data_func ("/paned/handle/horizontal-wide", GINT_TO_POINTER (2),
		test_handle_during_position_change);
	g_test_add_data_func ("/paned/handle/vertical-wide", GINT_TO_POINTER (3),
		test_handle_during_position_change);
	return g_test_run ();
}
