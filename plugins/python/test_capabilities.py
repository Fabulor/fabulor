import importlib.util
import pathlib
import sys
import types
import unittest
from unittest import mock


class FakeFfi:
    NULL = object()

    @staticmethod
    def new_handle(value):
        return value

    @staticmethod
    def new(_declaration, value):
        return value

    @staticmethod
    def def_extern(*args, **_kwargs):
        if args and callable(args[0]):
            return args[0]
        return lambda function: function


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
        self.callback_registrations = set()

    def add_hook(self, callback, userdata, is_unload=False, callback_key=None):
        if callback_key is not None and callback_key in self.callback_registrations:
            raise ValueError("Callback '{}' is already registered.".format(callback_key[0]))
        hook = types.SimpleNamespace(
            callback=callback,
            userdata=userdata,
            is_unload=is_unload,
            callback_key=callback_key,
            handle=object(),
            zoitechat_hook=None,
        )
        self.hooks.append(hook)
        if callback_key is not None:
            self.callback_registrations.add(callback_key)
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

        plugin_host_path = pathlib.Path(__file__).with_name('python.py')
        plugin_host_spec = importlib.util.spec_from_file_location('_zoitechat_plugin_host_test', plugin_host_path)
        cls.plugin_host = importlib.util.module_from_spec(plugin_host_spec)
        plugin_host_spec.loader.exec_module(cls.plugin_host)

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

    def test_callback_event_names_are_bounded_and_allowlisted(self):
        plugin = FakePlugin('example.events', ('events.message',))
        with self.assertRaisesRegex(ValueError, 'Supported callback events'):
            self.invoke(plugin, "api.register_callback('timer', lambda event: None)")
        with self.assertRaisesRegex(ValueError, '128 UTF-8 bytes'):
            self.invoke(plugin, "api.register_callback('server:' + ('x' * 122), lambda event: None)")

    def test_duplicate_callback_registration_is_rejected(self):
        plugin = FakePlugin('example.events', ('events.message',))
        callback = lambda event: None
        namespace = {'__plugin': plugin, 'api': self.api, 'callback': callback}
        exec('def call():\n    return api.register_callback(\'message\', callback)', namespace)
        namespace['call']()
        with self.assertRaisesRegex(ValueError, 'already registered'):
            namespace['call']()

    def test_manifest_hook_count_is_limited(self):
        plugin = self.plugin_host.Plugin('example.events', ('events.message',))
        for index in range(64):
            plugin.add_hook(lambda _userdata: None, None, is_unload=True,
                            callback_key=('message', index))
        with self.assertRaisesRegex(RuntimeError, '64-callback limit'):
            plugin.add_hook(lambda _userdata: None, None, is_unload=True,
                            callback_key=('message', 64))
        plugin.hooks.clear()

    def test_bundled_manifest_root_is_executable_relative(self):
        executable = pathlib.Path('C:/Program Files/Fabulor/fabulor.exe')
        with mock.patch.object(sys, 'executable', str(executable)):
            root = self.plugin_host.bundled_manifest_plugins_dir()
        self.assertEqual(
            self.plugin_host.canonical_path(root),
            self.plugin_host.canonical_path(str(executable.parent / 'Plugins')),
        )


if __name__ == '__main__':
    unittest.main()
