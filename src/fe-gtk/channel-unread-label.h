#ifndef FABULOR_CHANNEL_UNREAD_LABEL_H
#define FABULOR_CHANNEL_UNREAD_LABEL_H

#include <glib/gi18n.h>
#include <gtk/gtk.h>

static GtkWidget *
channel_unread_label_new (void)
{
	GtkWidget *label = gtk_label_new ("    ");
	PangoAttrList *attributes = pango_attr_list_new ();

	/* Reserve the same space from empty through 999+ in either switcher. */
	pango_attr_list_insert (attributes, pango_attr_family_new ("monospace"));
	gtk_label_set_attributes (GTK_LABEL (label), attributes);
	pango_attr_list_unref (attributes);
	gtk_label_set_single_line_mode (GTK_LABEL (label), TRUE);
	gtk_label_set_width_chars (GTK_LABEL (label), 4);
	gtk_label_set_max_width_chars (GTK_LABEL (label), 4);
	gtk_label_set_xalign (GTK_LABEL (label), 1.0);
	gtk_accessible_update_state (GTK_ACCESSIBLE (label),
		GTK_ACCESSIBLE_STATE_HIDDEN, TRUE, -1);
	return label;
}

static void
channel_unread_label_set (GtkWidget *label, guint64 count)
{
	/* Keep four monospace glyphs even when there is no visible count. */
	char text[5] = "    ";

	if (count > 999)
		g_strlcpy (text, "999+", sizeof text);
	else if (count)
		g_snprintf (text, sizeof text, "%4u", (guint) count);
	gtk_label_set_text (GTK_LABEL (label), text);
}

static char *
channel_unread_description (guint64 count)
{
	if (!count)
		return NULL;
	/* Translators: %s is the exact number of unread events. */
	char *number = g_strdup_printf ("%" G_GUINT64_FORMAT, count);
	char *description = g_strdup_printf (_("%s unread"), number);
	g_free (number);
	return description;
}

#endif
