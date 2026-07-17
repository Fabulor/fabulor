/* Copyright (C) 2026 Fabulor contributors */
#ifndef FABULOR_SPELL_ENTRY_MENU_H
#define FABULOR_SPELL_ENTRY_MENU_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define FABULOR_SPELL_MENU_ACTION_ADD "spell.add"
#define FABULOR_SPELL_MENU_ACTION_ENABLED "spell.enabled"
#define FABULOR_SPELL_MENU_ACTION_IGNORE "spell.ignore"
#define FABULOR_SPELL_MENU_ACTION_INSERT "spell.insert"
#define FABULOR_SPELL_MENU_ACTION_REPLACE "spell.replace"

typedef struct
{
	const gchar *language;
	const gchar *label;
	const gchar * const *suggestions;
	gsize suggestion_count;
} FabulorSpellMenuDictionary;

GMenuModel *fabulor_spell_entry_menu_new (
	const gchar *word,
	gboolean misspelled,
	gboolean checked,
	const FabulorSpellMenuDictionary *dictionaries,
	gsize dictionary_count);

G_END_DECLS

#endif
