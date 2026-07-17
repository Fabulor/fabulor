/* Fabulor spell-entry Pango styling boundary. */
#include "spell-entry-style.h"

#include <stdlib.h>
#include <string.h>

#define IRC_ATTR_BOLD          '\002'
#define IRC_ATTR_COLOR         '\003'
#define IRC_ATTR_HIDDEN        '\010'
#define IRC_ATTR_RESET         '\017'
#define IRC_ATTR_REVERSE       '\026'
#define IRC_ATTR_ITALICS       '\035'
#define IRC_ATTR_STRIKETHROUGH '\036'
#define IRC_ATTR_UNDERLINE     '\037'

static void
insert_hidden (PangoAttrList *attributes, guint start, guint end)
{
	PangoRectangle rectangle = { 0 };
	PangoAttribute *attribute = pango_attr_shape_new (&rectangle, &rectangle);

	attribute->start_index = start;
	attribute->end_index = end;
	pango_attr_list_insert (attributes, attribute);
}

static void
insert_underline (PangoAttrList *attributes, guint start, gboolean active)
{
	PangoAttribute *attribute = pango_attr_underline_new (
		active ? PANGO_UNDERLINE_NONE : PANGO_UNDERLINE_SINGLE);

	attribute->start_index = start;
	attribute->end_index = PANGO_ATTR_INDEX_TO_TEXT_END;
	pango_attr_list_change (attributes, attribute);
}

static void
insert_bold (PangoAttrList *attributes, guint start, gboolean active)
{
	PangoAttribute *attribute = pango_attr_weight_new (
		active ? PANGO_WEIGHT_NORMAL : PANGO_WEIGHT_BOLD);

	attribute->start_index = start;
	attribute->end_index = PANGO_ATTR_INDEX_TO_TEXT_END;
	pango_attr_list_change (attributes, attribute);
}

static void
insert_italic (PangoAttrList *attributes, guint start, gboolean active)
{
	PangoAttribute *attribute = pango_attr_style_new (
		active ? PANGO_STYLE_NORMAL : PANGO_STYLE_ITALIC);

	attribute->start_index = start;
	attribute->end_index = PANGO_ATTR_INDEX_TO_TEXT_END;
	pango_attr_list_change (attributes, attribute);
}

static void
insert_strikethrough (PangoAttrList *attributes, guint start, gboolean active)
{
	PangoAttribute *attribute = pango_attr_strikethrough_new (!active);

	attribute->start_index = start;
	attribute->end_index = PANGO_ATTR_INDEX_TO_TEXT_END;
	pango_attr_list_change (attributes, attribute);
}

static void
insert_colors (PangoAttrList *attributes, guint start,
	const FabulorSpellEntryColor *foreground,
	const FabulorSpellEntryColor *background)
{
	PangoAttribute *foreground_attribute = pango_attr_foreground_new (
		foreground->red, foreground->green, foreground->blue);
	PangoAttribute *underline_attribute = pango_attr_underline_color_new (
		foreground->red, foreground->green, foreground->blue);
	PangoAttribute *background_attribute = pango_attr_background_new (
		background->red, background->green, background->blue);

	foreground_attribute->start_index = start;
	foreground_attribute->end_index = PANGO_ATTR_INDEX_TO_TEXT_END;
	pango_attr_list_change (attributes, foreground_attribute);
	underline_attribute->start_index = start;
	underline_attribute->end_index = PANGO_ATTR_INDEX_TO_TEXT_END;
	pango_attr_list_change (attributes, underline_attribute);
	background_attribute->start_index = start;
	background_attribute->end_index = PANGO_ATTR_INDEX_TO_TEXT_END;
	pango_attr_list_change (attributes, background_attribute);
}

static FabulorSpellEntryColor
resolve_mirc_color (const FabulorSpellEntryPalette *palette, gint color_index,
	const FabulorSpellEntryColor *fallback)
{
	FabulorSpellEntryColor color = *fallback;

	if (color_index >= 0 && palette->resolve_mirc_color)
	{
		FabulorSpellEntryColor candidate = color;
		if (palette->resolve_mirc_color (color_index, &candidate,
			palette->user_data))
			color = candidate;
	}
	return color;
}

static void
insert_mirc_colors (PangoAttrList *attributes, guint start, gint foreground,
	gint background, const FabulorSpellEntryPalette *palette)
{
	FabulorSpellEntryColor foreground_color = resolve_mirc_color (palette,
		foreground, &palette->text_foreground);
	FabulorSpellEntryColor background_color = resolve_mirc_color (palette,
		background, &palette->text_background);

	insert_colors (attributes, start, &foreground_color, &background_color);
}

static void
insert_reset (PangoAttrList *attributes, guint start,
	const FabulorSpellEntryPalette *palette)
{
	insert_bold (attributes, start, TRUE);
	insert_underline (attributes, start, TRUE);
	insert_italic (attributes, start, TRUE);
	insert_strikethrough (attributes, start, TRUE);
	insert_colors (attributes, start, &palette->text_foreground,
		&palette->text_background);
}

PangoAttrList *
fabulor_spell_entry_style_build (const gchar *text, gboolean parse_irc,
	const FabulorSpellEntryPalette *palette)
{
	PangoAttrList *attributes;
	gboolean bold = FALSE;
	gboolean italic = FALSE;
	gboolean underline = FALSE;
	gboolean strikethrough = FALSE;
	gint parsing_color = 0;
	gchar foreground[3] = { 0 };
	gchar background[3] = { 0 };
	gsize length;
	gsize i;
	gsize offset = 0;

	g_return_val_if_fail (palette != NULL, NULL);
	attributes = pango_attr_list_new ();
	if (!parse_irc || !text)
		return attributes;
	length = strlen (text);
	if (length > G_MAXUINT)
		return attributes;

	for (i = 0; i < length; i++)
	{
		switch (text[i])
		{
		case IRC_ATTR_BOLD:
			insert_hidden (attributes, (guint) i, (guint) i + 1);
			insert_bold (attributes, (guint) i, bold);
			bold = !bold;
			goto check_color;
		case IRC_ATTR_ITALICS:
			insert_hidden (attributes, (guint) i, (guint) i + 1);
			insert_italic (attributes, (guint) i, italic);
			italic = !italic;
			goto check_color;
		case IRC_ATTR_STRIKETHROUGH:
			insert_hidden (attributes, (guint) i, (guint) i + 1);
			insert_strikethrough (attributes, (guint) i, strikethrough);
			strikethrough = !strikethrough;
			goto check_color;
		case IRC_ATTR_UNDERLINE:
			insert_hidden (attributes, (guint) i, (guint) i + 1);
			insert_underline (attributes, (guint) i, underline);
			underline = !underline;
			goto check_color;
		case IRC_ATTR_RESET:
			insert_hidden (attributes, (guint) i, (guint) i + 1);
			insert_reset (attributes, (guint) i, palette);
			bold = italic = underline = strikethrough = FALSE;
			goto check_color;
		case IRC_ATTR_HIDDEN:
			insert_hidden (attributes, (guint) i, (guint) i + 1);
			goto check_color;
		case IRC_ATTR_REVERSE:
			insert_hidden (attributes, (guint) i, (guint) i + 1);
			insert_colors (attributes, (guint) i,
				&palette->text_background, &palette->text_foreground);
			goto check_color;
		case '\n':
			insert_reset (attributes, (guint) i, palette);
			parsing_color = 0;
			break;
		case IRC_ATTR_COLOR:
			insert_hidden (attributes, (guint) i, (guint) i + 1);
			parsing_color = 1;
			offset = 1;
			break;
		default:
check_color:
			if (!parsing_color)
				continue;
			if (!g_ascii_isdigit (text[i]))
			{
				if (text[i] == ',' && parsing_color <= 3)
				{
					parsing_color = 3;
					offset++;
					continue;
				}
				parsing_color = 5;
			}
			else if (parsing_color == 3 && text[i - 1] != ',')
				parsing_color = 5;

			switch (parsing_color)
			{
			case 1:
				foreground[0] = text[i];
				parsing_color++;
				offset++;
				continue;
			case 2:
				foreground[1] = text[i];
				parsing_color++;
				offset++;
				continue;
			case 3:
				background[0] = text[i];
				parsing_color++;
				offset++;
				continue;
			case 4:
				background[1] = text[i];
				parsing_color++;
				offset++;
				continue;
			case 5:
				if (background[0])
				{
					insert_hidden (attributes, (guint) (i - offset), (guint) i);
					insert_mirc_colors (attributes, (guint) i,
						atoi (foreground), atoi (background), palette);
				}
				else if (foreground[0])
				{
					insert_hidden (attributes, (guint) (i - offset), (guint) i);
					insert_mirc_colors (attributes, (guint) i,
						atoi (foreground), -1, palette);
				}
				else
				{
					insert_hidden (attributes, (guint) (i - offset),
						(guint) (i - offset + 1));
					insert_mirc_colors (attributes, (guint) i, -1, -1,
						palette);
				}
				memset (background, 0, sizeof (background));
				memset (foreground, 0, sizeof (foreground));
				parsing_color = 0;
				offset = 0;
				continue;
			}
		}
	}

	if (parsing_color)
	{
		if (foreground[0] || background[0])
			insert_hidden (attributes, (guint) (length - offset),
				(guint) length);
		if (background[0])
			insert_mirc_colors (attributes, (guint) length,
				atoi (foreground), atoi (background), palette);
		else if (foreground[0])
			insert_mirc_colors (attributes, (guint) length,
				atoi (foreground), -1, palette);
		else
			insert_mirc_colors (attributes, (guint) length, -1, -1, palette);
	}

	return attributes;
}

void
fabulor_spell_entry_style_add_misspelling (PangoAttrList *attributes,
	guint byte_start, guint byte_end,
	const FabulorSpellEntryPalette *palette)
{
	PangoAttribute *color;
	PangoAttribute *underline;

	g_return_if_fail (attributes != NULL);
	g_return_if_fail (palette != NULL);
	if (byte_start >= byte_end)
		return;
	color = pango_attr_underline_color_new (palette->spell_error.red,
		palette->spell_error.green, palette->spell_error.blue);
	underline = pango_attr_underline_new (PANGO_UNDERLINE_ERROR);
	color->start_index = underline->start_index = byte_start;
	color->end_index = underline->end_index = byte_end;
	pango_attr_list_insert (attributes, color);
	pango_attr_list_insert (attributes, underline);
}

gboolean
fabulor_spell_entry_style_has_attributes (PangoAttrList *attributes)
{
	PangoAttrIterator *iterator;
	gboolean has_attributes = FALSE;

	if (!attributes)
		return FALSE;
	iterator = pango_attr_list_get_iterator (attributes);
	if (!iterator)
		return FALSE;
	do
	{
		GSList *items = pango_attr_iterator_get_attrs (iterator);
		has_attributes = items != NULL;
		g_slist_free_full (items, (GDestroyNotify) pango_attribute_destroy);
	}
	while (!has_attributes && pango_attr_iterator_next (iterator));
	pango_attr_iterator_destroy (iterator);
	return has_attributes;
}
