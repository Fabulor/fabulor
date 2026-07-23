#include "file-chooser-path.h"

/* GtkFileChooser is deprecated in GTK 4.10; contain it pending GtkFileDialog. */
G_GNUC_BEGIN_IGNORE_DEPRECATIONS

gboolean
fabulor_gtk_file_chooser_set_current_folder_path (GtkFileChooser *chooser,
	const gchar *path)
{
	GFile *folder;
	gboolean changed;

	g_return_val_if_fail (GTK_IS_FILE_CHOOSER (chooser), FALSE);
	g_return_val_if_fail (path != NULL, FALSE);
	folder = g_file_new_for_path (path);
	changed = gtk_file_chooser_set_current_folder (chooser, folder, NULL);
	g_object_unref (folder);
	return changed;
}

void
fabulor_gtk_file_chooser_set_local_only (GtkFileChooser *chooser,
	gboolean local_only)
{
	g_return_if_fail (GTK_IS_FILE_CHOOSER (chooser));

	/* GTK4 returns GFile values; owned-path conversion rejects non-local URIs. */
	(void) local_only;
}

void
fabulor_gtk_file_chooser_set_overwrite_confirmation (GtkFileChooser *chooser,
	gboolean enabled)
{
	g_return_if_fail (GTK_IS_FILE_CHOOSER (chooser));

	/* GTK4's native save dialog owns overwrite confirmation policy. */
	(void) enabled;
}

gchar *
fabulor_gtk_file_chooser_dup_filename (GtkFileChooser *chooser)
{
	GFile *file;
	gchar *path;

	g_return_val_if_fail (GTK_IS_FILE_CHOOSER (chooser), NULL);
	file = gtk_file_chooser_get_file (chooser);
	if (!file)
		return NULL;
	path = g_file_get_path (file);
	g_object_unref (file);
	return path;
}

gchar *
fabulor_gtk_file_chooser_dup_current_folder_path (GtkFileChooser *chooser)
{
	GFile *folder;
	gchar *path;

	g_return_val_if_fail (GTK_IS_FILE_CHOOSER (chooser), NULL);
	folder = gtk_file_chooser_get_current_folder (chooser);
	if (!folder)
		return NULL;
	path = g_file_get_path (folder);
	g_object_unref (folder);
	return path;
}

GSList *
fabulor_gtk_file_chooser_dup_filenames (GtkFileChooser *chooser)
{
	GSList *paths = NULL;

	g_return_val_if_fail (GTK_IS_FILE_CHOOSER (chooser), NULL);
	{
		GListModel *files = gtk_file_chooser_get_files (chooser);
		guint i;

		for (i = 0; files && i < g_list_model_get_n_items (files); i++)
		{
			GFile *file = g_list_model_get_item (files, i);
			gchar *path = g_file_get_path (file);

			if (path)
				paths = g_slist_prepend (paths, path);
			g_object_unref (file);
		}
		g_clear_object (&files);
	}
	return g_slist_reverse (paths);
}

G_GNUC_END_IGNORE_DEPRECATIONS
