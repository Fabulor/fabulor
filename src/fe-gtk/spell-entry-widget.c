/* Fabulor spell-entry widget boundary. */
#include "spell-entry-widget.h"

gint
fabulor_spell_entry_pointer_position (GtkEntry *entry, gdouble x)
{
	g_return_val_if_fail (GTK_IS_ENTRY (entry), 0);

	/* GtkEntry no longer exposes its internal layout in GTK4. Its GtkText
	 * delegate updates the editable cursor before the parent click bubbles. */
	(void) x;
	return gtk_editable_get_position (GTK_EDITABLE (entry));
}

void
fabulor_spell_entry_queue_redraw (GtkWidget *widget)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));
	gtk_widget_queue_draw (widget);
}
