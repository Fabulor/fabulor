/* Fabulor spell-entry Pango styling boundary. */
#ifndef FABULOR_SPELL_ENTRY_STYLE_H
#define FABULOR_SPELL_ENTRY_STYLE_H

#include <pango/pango.h>

G_BEGIN_DECLS

typedef struct
{
	guint16 red;
	guint16 green;
	guint16 blue;
} FabulorSpellEntryColor;

typedef gboolean (*FabulorSpellEntryMircColorFunc) (gint color_index,
	FabulorSpellEntryColor *color, gpointer user_data);

typedef struct
{
	FabulorSpellEntryColor text_foreground;
	FabulorSpellEntryColor text_background;
	FabulorSpellEntryColor spell_error;
	FabulorSpellEntryMircColorFunc resolve_mirc_color;
	gpointer user_data;
} FabulorSpellEntryPalette;

PangoAttrList *fabulor_spell_entry_style_build (const gchar *text,
	gboolean parse_irc, const FabulorSpellEntryPalette *palette);
void fabulor_spell_entry_style_add_misspelling (PangoAttrList *attributes,
	guint byte_start, guint byte_end,
	const FabulorSpellEntryPalette *palette);
gboolean fabulor_spell_entry_style_has_attributes (PangoAttrList *attributes);

G_END_DECLS

#endif
