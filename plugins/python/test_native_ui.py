"""Pure mocked bridge tests. No GTK windows, network, credentials or installation."""
import base64
import importlib.util
from pathlib import Path
import sys
import threading
from types import SimpleNamespace as NS
import unittest
from unittest.mock import patch
import xml.etree.ElementTree as ET

from test_capabilities import FakeFfi, FakePlugin


class Lib:
    ph = object()
    _on_command_hook = object()
    def __init__(self):
        self.features = b''
        self.panels = set()
        self.commands = []
        self.prints = []
    def fabulor_get_info(self, ph, key):
        if key == b'addon_ui_features':
            return self.features
        if key == b'addon_ui':
            return b'1'
        if key.startswith(b'addon_oper:'):
            return b'No OPER login attempted.' if key[11:].decode() in self.panels else FakeFfi.NULL
        if key.startswith(b'addon_ui:'):
            return b'open' if key[9:].decode() in self.panels else FakeFfi.NULL
        return FakeFfi.NULL
    def fabulor_command(self, ph, command):
        self.commands.append(command)
        words = command.decode().split()
        if words[:2] == ['ADDONUI', 'OPEN']:
            self.panels.add(words[2])
        elif words[:2] == ['ADDONUI', 'CLOSE']:
            self.panels.discard(words[2])
    def fabulor_hook_command(self, *args):
        return object()
    def fabulor_print(self, ph, text):
        self.prints.append(text)


class Owner(FakePlugin):
    def __init__(self, capabilities=None):
        super().__init__('test.ui', capabilities or {'ui.write', 'session.read', 'commands.execute', 'events.command'})
        self.ui_panels = set()
    def remove_hook(self, handle):
        self.hooks[:] = [h for h in self.hooks if id(h) != handle]


class NativeUiTests(unittest.TestCase):
    def setUp(self):
        self.lib = Lib()
        ffi = FakeFfi()
        ffi.string = lambda value: value
        with patch.dict(sys.modules, {'_fabulor_embedded': NS(ffi=ffi, lib=self.lib)}):
            spec = importlib.util.spec_from_file_location('_ui_test_api', Path(__file__).with_name('_fabulor.py'))
            self.api = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(self.api)
        self.owner = Owner()
        self.events = []
        self.schema = {'title': 'Test', 'context': 'Net / #test', 'pages': [{'id': 'overview', 'title': 'Overview', 'fields': [
            {'id': 'health', 'type': 'status', 'label': 'Health', 'value': 'Waiting', 'state': 'waiting'},
            {'id': 'enabled', 'type': 'check', 'label': 'Enabled', 'value': False},
            {'id': 'seconds', 'type': 'int', 'label': 'Seconds', 'value': 30, 'min': 1, 'max': 3600},
        ]}], 'buttons': [{'id': 'apply', 'label': 'Apply'}]}
        self.ns = {'__plugin': self.owner, 'api': self.api, 'schema': self.schema, 'events': self.events}
        exec('def callback(panel, event, values, userdata):\n    api.prnt("owned callback")\n    events.append((event, values))\ndef open_panel():\n    return api.ui_open(schema, callback)', self.ns)
        self.addCleanup(self.cleanup)

    def cleanup(self):
        for panel in list(self.owner.ui_panels):
            panel._dispose()

    def invoke(self, expression, owner=None):
        ns = dict(self.ns, __plugin=owner or self.owner)
        exec('def call():\n    return ' + expression, ns)
        return ns['call']()

    def event(self, panel, event, values):
        root = ET.Element('values')
        for name, value in values.items():
            ET.SubElement(root, 'field', id=name, value=value)
        wire = base64.b64encode(ET.tostring(root)).decode()
        return panel._event(['_FABULOR_UI_EVENT', panel.token, event, wire], [], None)

    def test_discovery_is_inactive_and_requires_complete_host(self):
        self.assertTrue(self.invoke('api.ui_available()'))
        self.assertFalse(self.lib.commands)
        self.assertFalse(self.owner.hooks)
        del self.owner.ui_panels
        self.assertFalse(self.invoke('api.ui_available()'))
        self.owner.ui_panels = set()

    def test_page_features_discovery_and_mixed_frontend_fallback(self):
        self.assertEqual(self.invoke('api.ui_features()'), frozenset())
        self.schema['pages'][0]['buttons'] = [{'id': 'local', 'label': 'Local'}]
        with self.assertRaises(RuntimeError):
            self.ns['open_panel']()
        self.assertFalse(self.lib.commands)
        self.lib.features = b'page_buttons unknown_future_feature'
        self.assertEqual(self.invoke('api.ui_features()'), frozenset({'page_buttons'}))
        self.assertFalse(self.lib.commands)
        panel = self.ns['open_panel']()
        self.event(panel, 'local', {'enabled': 'false', 'seconds': '30'})
        self.assertEqual(self.events[-1], ('local', {'enabled': False, 'seconds': 30}))

    def test_oper_login_is_optional_owner_bound_and_carries_no_secret(self):
        panel = self.ns['open_panel']()
        self.ns['panel'] = panel
        with self.assertRaises(RuntimeError):
            self.invoke('panel.oper_login()')
        self.lib.features = b'page_buttons oper_login unknown'
        self.assertIn('oper_login', self.invoke('api.ui_features()'))
        self.lib.commands.clear()
        self.invoke('panel.oper_login()')
        self.assertEqual(self.lib.commands, [('ADDONUI OPERLOGIN ' + panel.token).encode()])
        self.assertEqual(self.invoke('panel.oper_login_status()'), 'No OPER login attempted.')
        with self.assertRaises(PermissionError):
            self.invoke('panel.oper_login()', owner=Owner())
        with self.assertRaises(TypeError):
            self.invoke('panel.oper_login("not accepted")')
        self.assertEqual(len(self.lib.commands), 1)

    def test_page_buttons_share_identifiers_and_have_independent_limits(self):
        self.schema['pages'][0]['buttons'] = [{'id': 'apply', 'label': 'Duplicate'}]
        with self.assertRaises(ValueError):
            self.api._ui_document(self.schema)
        self.schema['pages'][0]['buttons'] = [{'id': 'b' + str(i), 'label': 'Action'} for i in range(7)]
        with self.assertRaises(ValueError):
            self.api._ui_document(self.schema)
        self.schema['pages'][0]['buttons'] = [{'id': 'b' + str(i), 'label': 'Action'} for i in range(6)]
        self.api._ui_document(self.schema)
        self.schema['pages'][0]['buttons'][0]['command'] = 'PRIVMSG'
        with self.assertRaises(ValueError):
            self.api._ui_document(self.schema)

    def test_page_button_callbacks_still_require_complete_values_and_owner(self):
        self.lib.features = b'page_buttons'
        self.schema['pages'][0]['buttons'] = [{'id': 'local', 'label': 'Local'}]
        panel = self.ns['open_panel']()
        self.event(panel, 'local', {'enabled': 'false'})
        self.assertFalse(self.events)
        self.owner.capabilities = frozenset()
        self.event(panel, 'local', {'enabled': 'true', 'seconds': '10'})
        self.assertTrue(panel.closed)
        self.assertFalse(self.events)

    def test_callback_enters_owner_frame_from_native_dispatch(self):
        panel = self.ns['open_panel']()
        self.assertEqual(self.event(panel, 'apply', {'enabled': 'true', 'seconds': '1800'}), self.api.EAT_ALL)
        self.assertEqual(self.events, [('apply', {'enabled': True, 'seconds': 1800})])
        self.assertEqual(self.lib.prints, [b'owned callback'])

    def test_other_tokens_incomplete_values_and_invalid_ranges_ignored(self):
        panel = self.ns['open_panel']()
        self.assertEqual(panel._event(['event', 'wrong', 'apply', ''], [], None), self.api.EAT_NONE)
        for values in ({'enabled': 'true'}, {'enabled': 'true', 'seconds': '4000'},
                       {'enabled': 'yes', 'seconds': '10'}, {'enabled': 'true', 'seconds': '10', 'health': 'bad'}):
            self.event(panel, 'apply', values)
        self.assertFalse(self.events)

    def test_update_readonly_only_and_foreign_owner_refused(self):
        panel = self.ns['open_panel']()
        self.ns['panel'] = panel
        self.invoke("panel.update({'health': {'value': 'Working', 'state': 'running'}})")
        self.assertTrue(self.lib.commands[-1].startswith(b'ADDONUI UPDATE '))
        with self.assertRaises(ValueError):
            self.invoke("panel.update({'enabled': True})")
        foreign = Owner()
        ns = {'__plugin': foreign, 'panel': panel}
        exec('def call():\n    panel.close()', ns)
        with self.assertRaises(PermissionError):
            ns['call']()

    def test_closed_and_owner_cleanup_release_hooks_and_native_panel(self):
        panel = self.ns['open_panel']()
        self.event(panel, 'closed', {})
        self.assertTrue(panel.closed)
        self.assertFalse(self.owner.hooks)
        self.assertFalse(self.owner.ui_panels)
        self.assertEqual(self.events[-1], ('closed', {}))
        panel = self.ns['open_panel']()
        panel._dispose()
        self.assertNotIn(panel.token, self.lib.panels)
        self.assertFalse(self.owner.hooks)
        self.event(panel, 'apply', {'enabled': 'true', 'seconds': '10'})
        self.assertEqual(len(self.events), 1)

    def test_limits_schema_validation_and_worker_thread_rejection(self):
        invalid = dict(self.schema, arbitrary='not allowed')
        with self.assertRaises(ValueError):
            self.api._ui_document(invalid)
        for _ in range(4):
            self.ns['open_panel']()
        with self.assertRaises(RuntimeError):
            self.ns['open_panel']()
        caught = []
        def worker():
            try:
                self.ns['open_panel']()
            except RuntimeError:
                caught.append(True)
        thread = threading.Thread(target=worker)
        thread.start()
        thread.join(2)
        self.assertEqual(caught, [True])

    def test_vanished_native_window_reclaims_handle_on_operation(self):
        panel = self.ns['open_panel']()
        self.ns['panel'] = panel
        self.lib.panels.discard(panel.token)
        with self.assertRaises(RuntimeError):
            self.invoke('panel.present()')
        self.assertTrue(panel.closed)
        self.assertFalse(self.owner.ui_panels)
        self.assertFalse(self.owner.hooks)

    def test_vanished_windows_do_not_consume_owner_quota_forever(self):
        old = [self.ns['open_panel']() for _ in range(4)]
        self.lib.panels.clear()
        new = self.ns['open_panel']()
        self.assertTrue(all(panel.closed for panel in old))
        self.assertEqual(self.owner.ui_panels, {new})
        self.assertEqual(len(self.owner.hooks), 1)

    def test_capability_revocation_closes_panel_without_callback(self):
        panel = self.ns['open_panel']()
        self.owner.capabilities = frozenset()
        self.event(panel, 'apply', {'enabled': 'true', 'seconds': '30'})
        self.assertTrue(panel.closed)
        self.assertFalse(self.events)


if __name__ == '__main__':
    unittest.main()
