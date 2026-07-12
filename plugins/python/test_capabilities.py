import importlib.util
import pathlib
import sys
import types
import unittest


class FakeFfi:
    NULL = object()

    @staticmethod
    def new_handle(value):
        return value


class FakeLib:
    ph = object()
    _on_server_attrs_hook = object()

    def __init__(self):
        self.commands = []

    def zoitechat_command(self, _ph, command):
        self.commands.append(command)

    @staticmethod
    def zoitechat_hook_server_attrs(*_args):
        return object()


class FakePlugin:
    def __init__(self, manifest_id, capabilities):
        self.manifest_id = manifest_id
        self.capabilities = frozenset(capabilities)
        self.hooks = []

    def add_hook(self, callback, userdata, is_unload=False):
        hook = types.SimpleNamespace(
            callback=callback,
            userdata=userdata,
            is_unload=is_unload,
            handle=object(),
            zoitechat_hook=None,
        )
        self.hooks.append(hook)
        return hook


class CapabilityTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.lib = FakeLib()
        embedded = types.SimpleNamespace(ffi=FakeFfi(), lib=cls.lib)
        sys.modules['_zoitechat_embedded'] = embedded

        module_path = pathlib.Path(__file__).with_name('_zoitechat.py')
        spec = importlib.util.spec_from_file_location('_zoitechat_capability_test', module_path)
        cls.api = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(cls.api)

    def invoke(self, plugin, expression):
        namespace = {'__plugin': plugin, 'api': self.api}
        exec('def call():\n    return ' + expression, namespace)
        return namespace['call']()

    def setUp(self):
        self.lib.commands.clear()

    def test_trusted_simple_addon_is_not_capability_gated(self):
        plugin = FakePlugin(None, ())
        self.invoke(plugin, "api.command('ECHO trusted')")
        self.assertEqual(self.lib.commands, [b'ECHO trusted'])

    def test_manifest_message_is_denied_without_capability(self):
        plugin = FakePlugin('example.denied', ())
        with self.assertRaisesRegex(PermissionError, 'messages.write'):
            self.invoke(plugin, "api.send_message('#test', 'hello')")

    def test_manifest_message_is_allowed_with_capability(self):
        plugin = FakePlugin('example.allowed', ('messages.write',))
        self.invoke(plugin, "api.send_message('#test', 'hello')")
        self.assertEqual(self.lib.commands, [b'MSG #test hello'])

    def test_message_callback_does_not_grant_raw_server_hooks(self):
        plugin = FakePlugin('example.events', ('events.message',))
        self.invoke(plugin, "api.register_callback('message', lambda event: None)")
        with self.assertRaisesRegex(PermissionError, 'events.server'):
            self.invoke(plugin, "api.hook_server('NOTICE', lambda *args: None)")


if __name__ == '__main__':
    unittest.main()
