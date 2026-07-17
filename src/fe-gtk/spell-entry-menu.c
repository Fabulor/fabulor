#include "spell-entry-menu.h"

#include <glib/gi18n.h>

#define SUGGESTIONS_PER_MENU 10

static void
append_target_item (GMenu *menu, const gchar *label, const gchar *action,
	GVariant *target)
{
	GMenuItem *item = g_menu_item_new (label, NULL);

	g_menu_item_set_action_and_target_value (item, action, target);
	g_menu_append_item (menu, item);
	g_object_unref (item);
}

static void
append_suggestions (GMenu *menu,
	const FabulorSpellMenuDictionary *dictionary)
{
	GMenu *current = menu;
	gsize i;

	if (!dictionary->suggestions || dictionary->suggestion_count == 0)
	{
		g_menu_append (current, _("(no suggestions)"), NULL);
		return;
	}

	for (i = 0; i < dictionary->suggestion_count; i++)
	{
		if (i != 0 && i % SUGGESTIONS_PER_MENU == 0)
		{
			GMenu *more = g_menu_new ();

			g_menu_append_submenu (current, _("More..."),
				G_MENU_MODEL (more));
			current = more;
			g_object_unref (more);
		}

		append_target_item (current, dictionary->suggestions[i],
			FABULOR_SPELL_MENU_ACTION_REPLACE,
			g_variant_new ("(ss)", dictionary->language,
				dictionary->suggestions[i]));
	}
}

static GMenu *
build_spelling_menu (const gchar *word,
	const FabulorSpellMenuDictionary *dictionaries,
	gsize dictionary_count)
{
	GMenu *menu = g_menu_new ();
	GMenu *suggestions = g_menu_new ();
	GMenu *commands = g_menu_new ();
	gchar *label;
	gsize i;

	if (dictionary_count == 1)
	{
		append_suggestions (suggestions, &dictionaries[0]);
	}
	else
	{
		for (i = 0; i < dictionary_count; i++)
		{
			GMenu *language = g_menu_new ();

			append_suggestions (language, &dictionaries[i]);
			g_menu_append_submenu (suggestions,
				dictionaries[i].label ? dictionaries[i].label :
					dictionaries[i].language,
				G_MENU_MODEL (language));
			g_object_unref (language);
		}
	}

	g_menu_append_section (menu, NULL, G_MENU_MODEL (suggestions));
	g_object_unref (suggestions);

	label = g_strdup_printf (_("Add \"%s\" to Dictionary"), word);
	if (dictionary_count == 1)
	{
		append_target_item (commands, label,
			FABULOR_SPELL_MENU_ACTION_ADD,
			g_variant_new_string (dictionaries[0].language));
	}
	else
	{
		GMenu *languages = g_menu_new ();

		for (i = 0; i < dictionary_count; i++)
			append_target_item (languages,
				dictionaries[i].label ? dictionaries[i].label :
					dictionaries[i].language,
				FABULOR_SPELL_MENU_ACTION_ADD,
				g_variant_new_string (dictionaries[i].language));
		g_menu_append_submenu (commands, label, G_MENU_MODEL (languages));
		g_object_unref (languages);
	}
	g_free (label);

	g_menu_append (commands, _("Ignore All"),
		FABULOR_SPELL_MENU_ACTION_IGNORE);
	g_menu_append_section (menu, NULL, G_MENU_MODEL (commands));
	g_object_unref (commands);

	return menu;
}

static void
append_insert_item (GMenu *menu, const gchar *label, gint code)
{
	append_target_item (menu, label, FABULOR_SPELL_MENU_ACTION_INSERT,
		g_variant_new_int32 (code));
}

static GMenu *
build_formatting_menu (void)
{
	GMenu *menu = g_menu_new ();
	GMenu *attributes = g_menu_new ();
	GMenu *low_colors = g_menu_new ();
	GMenu *high_colors = g_menu_new ();
	gint i;

	append_insert_item (attributes, _("Bold"), 100);
	append_insert_item (attributes, _("Underline"), 101);
	append_insert_item (attributes, _("Italic"), 102);
	append_insert_item (attributes, _("Strikethrough"), 103);
	append_insert_item (attributes, _("Normal"), 999);
	g_menu_append_section (menu, NULL, G_MENU_MODEL (attributes));
	g_object_unref (attributes);

	for (i = 0; i < 8; i++)
	{
		gchar *label = g_strdup_printf (_("Color %02d"), i);
		append_insert_item (low_colors, label, i);
		g_free (label);
	}
	for (i = 8; i < 16; i++)
	{
		gchar *label = g_strdup_printf (_("Color %02d"), i);
		append_insert_item (high_colors, label, i);
		g_free (label);
	}

	g_menu_append_submenu (menu, _("Colors 0-7"),
		G_MENU_MODEL (low_colors));
	g_menu_append_submenu (menu, _("Colors 8-15"),
		G_MENU_MODEL (high_colors));
	g_object_unref (low_colors);
	g_object_unref (high_colors);

	return menu;
}

GMenuModel *
fabulor_spell_entry_menu_new (const gchar *word, gboolean misspelled,
	gboolean checked, const FabulorSpellMenuDictionary *dictionaries,
	gsize dictionary_count)
{
	GMenu *root = g_menu_new ();
	GMenu *formatting = build_formatting_menu ();
	GMenu *settings = g_menu_new ();

	if (checked && misspelled && word && *word && dictionaries &&
		dictionary_count != 0)
	{
		GMenu *spelling = build_spelling_menu (word, dictionaries,
			dictionary_count);

		g_menu_append_submenu (root, _("Spelling Suggestions"),
			G_MENU_MODEL (spelling));
		g_object_unref (spelling);
	}

	g_menu_append_submenu (root, _("Insert Attribute or Color Code"),
		G_MENU_MODEL (formatting));
	g_object_unref (formatting);

	g_menu_append (settings, _("Enable Spell Checking"),
		FABULOR_SPELL_MENU_ACTION_ENABLED);
	g_menu_append_section (root, NULL, G_MENU_MODEL (settings));
	g_object_unref (settings);

	return G_MENU_MODEL (root);
}
