"""Build/test source contracts for session-local WinSpell behaviour."""
from pathlib import Path
import json
import unittest

ROOT = Path(__file__).resolve().parents[2]


def source(path):
    return (ROOT / path).read_text(encoding='utf-8')


class EnchantSessionIsolation(unittest.TestCase):
    def test_recipe_disables_both_persistent_provider_hooks(self):
        text = source('tools/enchant-msvc/CMakeLists.txt')
        self.assertIn('foreach(method IN ITEMS add_to_session remove_from_session)', text)
        self.assertIn('dict->${method} = nullptr;', text)
        self.assertIn('if(NOT removed_length EQUAL anchor_length)', text)
        self.assertIn('winspell-session-local.cpp', text)
        self.assertIn('file(WRITE "${winspell_patched_source}"', text)
        self.assertIn('SHARED "${winspell_patched_source}"', text)
        self.assertNotIn('file(WRITE "${ENCHANT_SOURCE_DIR}', text)
        manifest = json.loads(source('third-party/components.json'))
        component = next(c for c in manifest['components'] if c['id'] == 'enchant')
        self.assertEqual(component['version'], '2.8.19+fabulor-session-local.1')
        self.assertIn('enchant-fabulor-patch-NOTICE.txt', component['licence_files'])
        self.assertIn('core already maintains session', source('third-party/licenses/enchant-fabulor-patch-NOTICE.txt'))

    def test_smoke_preflights_provider_before_word_changes(self):
        text = source('tools/enchant-msvc/smoke.c')
        self.assertIn('!dict->add_to_session && !dict->remove_from_session', text)
        main = text.split('int main(', 1)[1]
        self.assertLess(main.index('safe_provider(argv[1])'), main.index('enchant_dict_add_to_session'))
        self.assertLess(main.index('isolated_config()'), main.index('enchant_dict_add(dict'))
        self.assertIn('Unsafe provider or missing isolated config: no word tests performed', main)

    def test_config_override_is_explicit_unique_and_restored(self):
        text = source('tools/build-enchant-msvc.ps1')
        self.assertIn('[Guid]::NewGuid()', text)
        self.assertIn('$env:ENCHANT_CONFIG_DIR = $smokeConfig', text)
        self.assertIn('$env:ENCHANT_CONFIG_DIR = $originalEnchantConfig', text)
        self.assertIn('$env:XDG_CONFIG_HOME = $originalXdgConfig', text)
        self.assertIn("lib\\enchant-2\\enchant_winspell.dll')", text)

    def test_session_tests_use_fresh_dictionaries(self):
        text = source('tools/enchant-msvc/smoke.c')
        self.assertIn('enchant_dict_remove_from_session(dict, "the", -1)', text)
        self.assertIn('enchant_dict_check(other_dict, session_word, -1) != 1', text)
        self.assertIn('enchant_dict_check(other_dict, "the", -1) != 0', text)
        self.assertIn('g_canonicalize_filename(configured, NULL)', text)
        self.assertIn('g_ascii_strcasecmp(a, b) == 0', text)

    def test_real_key_probe_checks_own_window_and_actual_widget(self):
        text = source('tools/gtk4/test-real-spell-entry.c')
        self.assertIn('GetForegroundWindow () != hwnd', text)
        self.assertIn('GetAsyncKeyState (VK_LWIN)', text)
        self.assertIn('sexy_spell_entry_new ()', text)
        self.assertIn('the sleek better T', text)
        self.assertIn('gtk_widget_activate_action (entry, "spell.ignore", NULL)', text)
        self.assertIn('!ignored && fresh_misspelled', text)
        self.assertNotIn('checker->Add', text)


if __name__ == '__main__':
    unittest.main()
