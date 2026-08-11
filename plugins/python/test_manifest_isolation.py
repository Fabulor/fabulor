import json
import pathlib
import tempfile
import textwrap
import unittest

try:
    import concurrent.interpreters as interpreters
except ImportError:
    interpreters = None


PLUGIN_SOURCE = '''
import fabulor

counter = 0


def on_message(event):
    global counter
    counter += 1
    fabulor.log('{}:{}'.format(fabulor.get_user_info()['nickname'], counter))
    return counter


def init():
    fabulor.register_callback('message', on_message)
'''


@unittest.skipIf(interpreters is None, 'Python 3.14 concurrent interpreters are unavailable')
class ManifestIsolationTests(unittest.TestCase):
    def setUp(self):
        self.temporary_directory = tempfile.TemporaryDirectory(dir=pathlib.Path.cwd())
        self.root = pathlib.Path(self.temporary_directory.name)
        self.runtime_path = pathlib.Path(__file__).with_name('_fabulor_manifest.py').resolve()
        self.interpreters = []

    def tearDown(self):
        for interpreter in self.interpreters:
            try:
                self.call(interpreter, {'action': 'shutdown'})
            except Exception:
                # Continue closing the remaining isolated interpreters.
                pass
            interpreter.close()
        self.temporary_directory.cleanup()

    def create_interpreter(self, name, source=PLUGIN_SOURCE):
        plugin_directory = self.root / name
        plugin_directory.mkdir()
        entrypoint = plugin_directory / 'plugin.py'
        entrypoint.write_text(textwrap.dedent(source), encoding='utf-8')

        interpreter = interpreters.create()
        interpreter.prepare_main(
            _fabulor_runtime_path=str(self.runtime_path),
            _fabulor_plugin_path=str(plugin_directory),
        )
        interpreter.exec(
            "import importlib.util\n"
            "import sys\n"
            "_fabulor_spec = importlib.util.spec_from_file_location(\n"
            "    '_fabulor_manifest', _fabulor_runtime_path)\n"
            "_fabulor_runtime = importlib.util.module_from_spec(_fabulor_spec)\n"
            "sys.modules['_fabulor_manifest'] = _fabulor_runtime\n"
            "_fabulor_spec.loader.exec_module(_fabulor_runtime)\n"
            "sys.path.insert(0, _fabulor_plugin_path)\n"
        )
        self.interpreters.append(interpreter)
        return interpreter, entrypoint

    @staticmethod
    def call(interpreter, request):
        request_json = json.dumps(request, separators=(',', ':'))
        return ManifestIsolationTests.call_json(interpreter, request_json)

    @staticmethod
    def call_json(interpreter, request_json):
        response_json = interpreter.call(
            lambda payload: __import__('_fabulor_manifest').handle(payload), request_json)
        return json.loads(response_json)

    @staticmethod
    def context(nickname):
        return {
            'user_count': 3,
            'user_info': {
                'nickname': nickname,
                'channel': '#fabulor',
                'server_name': 'irc.example',
                'network_name': 'Example',
            },
        }

    def load(self, interpreter, entrypoint, plugin_id, capabilities):
        return self.call(interpreter, {
            'action': 'load',
            'plugin_id': plugin_id,
            'entrypoint': str(entrypoint.resolve()),
            'capabilities': capabilities,
            'context': self.context(plugin_id),
        })

    def dispatch(self, interpreter, handler, nickname):
        return self.call(interpreter, {
            'action': 'dispatch',
            'handler': handler,
            'event': {'event': 'message', 'source': 'PRIVMSG'},
            'context': self.context(nickname),
        })

    def test_plugins_have_independent_module_and_callback_state(self):
        first, first_entrypoint = self.create_interpreter('first')
        second, second_entrypoint = self.create_interpreter('second')
        capabilities = ['events.message', 'session.read']

        first_load = self.load(first, first_entrypoint, 'first', capabilities)
        second_load = self.load(second, second_entrypoint, 'second', capabilities)
        self.assertTrue(first_load['ok'], first_load['error'])
        self.assertTrue(second_load['ok'], second_load['error'])
        first_handler = first_load['operations'][0]['handler']
        second_handler = second_load['operations'][0]['handler']

        first_result = self.dispatch(first, first_handler, 'Alice')
        first_again = self.dispatch(first, first_handler, 'Alice')
        second_result = self.dispatch(second, second_handler, 'Bob')

        self.assertEqual(first_result['result'], 1)
        self.assertEqual(first_again['result'], 2)
        self.assertEqual(second_result['result'], 1)
        self.assertEqual(first_again['operations'][0]['text'], 'Alice:2')
        self.assertEqual(second_result['operations'][0]['text'], 'Bob:1')

    def test_capability_denial_is_confined_to_failed_interpreter(self):
        denied_source = '''
            import fabulor

            def init():
                fabulor.send_message('#test', 'denied')
        '''
        denied, denied_entrypoint = self.create_interpreter('denied', denied_source)
        allowed, allowed_entrypoint = self.create_interpreter('allowed')

        denied_result = self.load(denied, denied_entrypoint, 'denied', [])
        allowed_result = self.load(
            allowed, allowed_entrypoint, 'allowed', ['events.message', 'session.read'])

        self.assertFalse(denied_result['ok'])
        self.assertIn('messages.write', denied_result['error'])
        self.assertTrue(allowed_result['ok'], allowed_result['error'])

    def test_legacy_cffi_modules_are_not_importable(self):
        source = '''
            def init():
                for module_name in ('_fabulor', '_fabulor_embedded',
                                    '_zoitechat', '_zoitechat_embedded',
                                    'zoitechat', 'hexchat', 'xchat'):
                    try:
                        __import__(module_name)
                    except ModuleNotFoundError:
                        continue
                    raise AssertionError('{} was importable'.format(module_name))
        '''
        interpreter, entrypoint = self.create_interpreter('blocked-cffi', source)
        result = self.load(interpreter, entrypoint, 'blocked-cffi', [])
        self.assertTrue(result['ok'], result['error'])

    def test_duplicate_and_callback_limits_are_per_interpreter(self):
        duplicate_source = '''
            import fabulor

            def callback(event):
                return None

            def init():
                fabulor.register_callback('message', callback)
                fabulor.register_callback('message', callback)
        '''
        interpreter, entrypoint = self.create_interpreter('duplicate', duplicate_source)
        result = self.load(interpreter, entrypoint, 'duplicate', ['events.message'])
        self.assertFalse(result['ok'])
        self.assertIn('already registered', result['error'])

    def test_oversized_runtime_request_is_rejected(self):
        interpreter, _ = self.create_interpreter('oversized-request')
        result = self.call_json(interpreter, ' ' * (1024 * 1024 + 1))
        self.assertFalse(result['ok'])
        self.assertIn('request exceeds the 1 MiB limit', result['error'])

    def test_oversized_entrypoint_is_rejected(self):
        interpreter, entrypoint = self.create_interpreter('oversized-entrypoint')
        with entrypoint.open('ab') as handle:
            handle.write(b'#' * (1024 * 1024))

        result = self.load(interpreter, entrypoint, 'oversized-entrypoint', [])
        self.assertFalse(result['ok'])
        self.assertIn('entrypoint exceeds the 1 MiB limit', result['error'])

    def test_oversized_response_is_replaced_with_bounded_error(self):
        oversized_source = '''
            import fabulor

            def init():
                for _ in range(17):
                    fabulor.log('x' * 65536)
        '''
        interpreter, entrypoint = self.create_interpreter(
            'oversized-response', oversized_source)
        result = self.load(interpreter, entrypoint, 'oversized-response', [])
        self.assertFalse(result['ok'])
        self.assertEqual(result['operations'], [])
        self.assertEqual(result['error'], 'Manifest plugin response exceeded the 1 MiB limit')


if __name__ == '__main__':
    unittest.main()
