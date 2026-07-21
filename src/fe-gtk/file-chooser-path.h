#ifndef FABULOR_FILE_CHOOSER_PATH_H
#define FABULOR_FILE_CHOOSER_PATH_H

#include <gtk/gtk.h>

gboolean fabulor_gtk_file_chooser_set_current_folder_path (GtkFileChooser *chooser,
	const gchar *path);
void fabulor_gtk_file_chooser_set_local_only (GtkFileChooser *chooser,
	gboolean local_only);
gchar *fabulor_gtk_file_chooser_dup_filename (GtkFileChooser *chooser);
gchar *fabulor_gtk_file_chooser_dup_current_folder_path (GtkFileChooser *chooser);
GSList *fabulor_gtk_file_chooser_dup_filenames (GtkFileChooser *chooser);

#endif
