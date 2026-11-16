"""Source integration contracts; native widget tests live in test-spell-autocorrect.c."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]


def read(path):
    return (ROOT / path).read_text(encoding='utf-8')


class AutoCorrectIntegration(unittest.TestCase):
    def test_preference_is_persistent_and_off_by_default(self):
        cfg = read('src/common/cfgfiles.c')
        self.assertIn('{"gui_input_autocorrect", P_OFFINT (hex_gui_input_autocorrect), TYPE_BOOL}', cfg)
        self.assertIn('prefs.hex_gui_input_autocorrect = 0;', cfg)
        self.assertIn('hex_gui_input_autocorrect', read('src/common/fabulor.h'))

    def test_settings_apply_and_entry_creation(self):
        for path in ['src/fe-gtk/maingui.c', 'src/fe-gtk/setup.c']:
            self.assertIn('sexy_spell_entry_set_autocorrect', read(path))
        self.assertIn('Auto-correct while typing', read('src/fe-gtk/setup.c'))

    def test_dictionary_and_command_guards(self):
        source = read('src/fe-gtk/sexy-spell-entry.c')
        approve = source.split('autocorrect_original_allowed (', 1)[1].split('static GtkEntryClass', 1)[0]
        self.assertIn('signals[WORD_CHECK]', approve)
        self.assertIn('if (!misspelled || !entry->priv->dict_list) return FALSE;', approve)
        self.assertIn('enchant_dict_check (li->data, original, -1) != 1', approve)
        self.assertIn('g_ascii_strncasecmp (lang, "en", 2)', approve)
        self.assertIn('enchant_dict_check (dictionary, replacement, -1) == 0', approve)
        self.assertIn('prefs.hex_input_command_char[0]', approve)
        self.assertIn('!entry->priv->checked', approve)
        self.assertNotIn('store_replacement', approve)

    def test_nickname_veto_uses_the_entry_origin(self):
        source = read('src/fe-gtk/maingui.c')
        check = source.split('mg_spellcheck_cb (', 1)[1].split('#if 0', 1)[0]
        self.assertIn('current_tab->gui == data', check)
        self.assertIn('candidate->gui == data', check)
        self.assertIn('userlist_find_global (origin->server, word)', check)
        self.assertIn('origin->server->nick, word', check)
        self.assertIn('origin->channel, word', check)
        self.assertNotIn('current_sess', check)
        self.assertIn('G_CALLBACK (mg_spellcheck_cb), gui)', source)

    def test_tab_and_dispose_cancel_pending_edits(self):
        main = read('src/fe-gtk/maingui.c')
        restore = main.split('mg_restore_speller (', 1)[1].split('void\nmg_set_topic_tip', 1)[0]
        self.assertIn('fabulor_spell_autocorrect_reset', restore)
        spell = read('src/fe-gtk/sexy-spell-entry.c')
        self.assertIn('fabulor_spell_autocorrect_reset (GTK_ENTRY (entry))', spell)
        self.assertIn('checked && entry->priv->autocorrect', spell)

    def test_windows_preferred_suggestion_without_persistent_training(self):
        source = read('src/fe-gtk/spell-entry-autocorrect.c')
        owner = read('src/fe-gtk/sexy-spell-entry.c')
        suggest = owner.split('suggest_autocorrect (', 1)[1].split('static gboolean\napprove_autocorrect', 1)[0]
        self.assertIn('enchant_dict_suggest (dictionary, original, -1, &count)', suggest)
        self.assertIn('g_strdup (suggestions[0])', suggest)
        self.assertIn('enchant_dict_free_suggestions', suggest)
        self.assertIn('strcmp (provider, "winspell")', owner)
        self.assertNotIn('typos[]', source)
        self.assertNotIn('store_replacement', suggest)
        self.assertIn('g_utf8_strlen (candidate, -1)', source)
        self.assertNotIn('enchant_dict_store_replacement', source)
        self.assertNotIn('g_file_set_contents', source)
        self.assertIn('preedit-changed', source)
        self.assertIn('ac->enabled = enabled && ac->ime_guard', source)
        self.assertIn('keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter', source)


if __name__ == '__main__':
    unittest.main()
