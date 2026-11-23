/* Width-aware labels retain their complete identity in the model. */
#ifndef FABULOR_CLIPPED_LABEL_H
#define FABULOR_CLIPPED_LABEL_H

#include <gtk/gtk.h>

static gboolean
fabulor_clipped_label_tip_area (GtkWidget *widget, gint x, gint y,
	gboolean keyboard, GdkRectangle *tip_area)
{
	GtkLabel *label = GTK_LABEL (widget);
	PangoLayout *layout = gtk_label_get_layout (label);
	PangoRectangle ink;
	GdkRectangle area;
	gint origin_x, origin_y;

	if (!g_object_get_data (G_OBJECT (widget), "fabulor-full-label") &&
		!pango_layout_is_ellipsized (layout))
		return FALSE;
	gtk_label_get_layout_offsets (label, &origin_x, &origin_y);
	pango_layout_get_pixel_extents (layout, NULL, &ink);
	area.x = MAX (0, origin_x + ink.x);
	area.y = MAX (0, origin_y + ink.y);
	area.width = MIN (ink.width, gtk_widget_get_width (widget) - area.x);
	area.height = MIN (ink.height, gtk_widget_get_height (widget) - area.y);
	if (area.width <= 0 || area.height <= 0 || (!keyboard &&
		(x < area.x || x >= area.x + area.width ||
		 y < area.y || y >= area.y + area.height)))
		return FALSE;
	*tip_area = area;
	return TRUE;
}

static gboolean
fabulor_clipped_label_tooltip (GtkWidget *widget, gint x, gint y,
	gboolean keyboard, GtkTooltip *tooltip, gpointer data)
{
	GdkRectangle area;
	const char *full = g_object_get_data (G_OBJECT (widget), "fabulor-full-label");
	(void) data;
	if (!fabulor_clipped_label_tip_area (widget, x, y, keyboard, &area))
		return FALSE;
	gtk_tooltip_set_text (tooltip, full ? full : gtk_label_get_text (GTK_LABEL (widget)));
	gtk_tooltip_set_tip_area (tooltip, &area);
	return TRUE;
}

static inline void
fabulor_clipped_label_set_full (GtkWidget *label, const char *full)
{
	const char *previous = g_object_get_data (G_OBJECT (label), "fabulor-full-label");

	if (g_strcmp0 (previous, full) == 0)
		return;
	g_object_set_data_full (G_OBJECT (label), "fabulor-full-label", g_strdup (full), g_free);
	if (full)
		gtk_accessible_update_property (GTK_ACCESSIBLE (label),
			GTK_ACCESSIBLE_PROPERTY_LABEL, full, -1);
	else
		gtk_accessible_reset_property (GTK_ACCESSIBLE (label),
			GTK_ACCESSIBLE_PROPERTY_LABEL);
	gtk_widget_trigger_tooltip_query (label);
}

static void
fabulor_clipped_label_changed (GObject *label, GParamSpec *pspec, gpointer data)
{
	(void) pspec;
	(void) data;
	gtk_widget_trigger_tooltip_query (GTK_WIDGET (label));
}

static void
fabulor_clipped_label_init (GtkWidget *label)
{
	gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
	gtk_widget_set_has_tooltip (label, TRUE);
	g_signal_connect (label, "query-tooltip",
		G_CALLBACK (fabulor_clipped_label_tooltip), NULL);
	g_signal_connect (label, "notify::label",
		G_CALLBACK (fabulor_clipped_label_changed), NULL);
}

#endif
