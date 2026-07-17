/* Fabulor spell-entry widget boundary. */
#include "spell-entry-widget.h"

gint
fabulor_spell_entry_pointer_position (GtkEntry *entry, gdouble x)
{
	g_return_val_if_fail (GTK_IS_ENTRY (entry), 0);

#if GTK_MAJOR_VERSION >= 4
	/* GtkEntry no longer exposes its internal layout in GTK4. Its GtkText
	 * delegate updates the editable cursor before the parent click bubbles. */
	(void) x;
	return gtk_editable_get_position (GTK_EDITABLE (entry));
#else
	PangoLayout *layout;
	PangoLayoutLine *line;
	const gchar *text;
	gint index;
	gint layout_x;
	gint layout_y;
	gint pos;
	gboolean trailing;

	gtk_entry_get_layout_offsets (entry, &layout_x, &layout_y);
	layout = gtk_entry_get_layout (entry);
	line = pango_layout_get_lines_readonly (layout)->data;
	pango_layout_line_x_to_index (line,
		(gint) ((x - layout_x) * PANGO_SCALE), &index, &trailing);
	text = pango_layout_get_text (layout);
	pos = g_utf8_pointer_to_offset (text, text + index);
	return pos + trailing;
#endif
}

void
fabulor_spell_entry_queue_redraw (GtkWidget *widget)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));
	gtk_widget_queue_draw (widget);
}
