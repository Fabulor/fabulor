/* Fabulor spell-entry widget boundary. */
#ifndef FABULOR_SPELL_ENTRY_WIDGET_H
#define FABULOR_SPELL_ENTRY_WIDGET_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

gint fabulor_spell_entry_pointer_position (GtkEntry *entry, gdouble x);
void fabulor_spell_entry_queue_redraw (GtkWidget *widget);

G_END_DECLS

#endif
