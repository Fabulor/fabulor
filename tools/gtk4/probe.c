#include <gtk/gtk.h>

#include <stdio.h>

#if GTK_MAJOR_VERSION != 4
#error The GTK4 probe must compile against GTK 4 headers.
#endif

#if GLIB_SIZEOF_VOID_P != 8
#error The initial GTK4 probe requires a 64-bit GLib build.
#endif

int
main (void)
{
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
