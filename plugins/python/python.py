from __future__ import print_function

import importlib
import base64
import json
import os
import pydoc
import signal
import sys
import traceback
import weakref
from contextlib import contextmanager

try:
    import concurrent.interpreters as interpreters
except ImportError:
    interpreters = None

from _zoitechat_embedded import ffi, lib

if sys.version_info < (3, 0):
    from io import BytesIO as HelpEater
else:
    from io import StringIO as HelpEater

if not hasattr(sys, 'argv'):
    sys.argv = ['<zoitechat>']

VERSION = b'2.18.3'
PLUGIN_NAME = ffi.new('char[]', b'Python')
PLUGIN_DESC = ffi.new('char[]', b'Python %d.%d scripting interface' % (sys.version_info[0], sys.version_info[1]))
PLUGIN_VERSION = ffi.new('char[]', VERSION)

zoitechat = None
zoitechat_internal = None
local_interp = None
zoitechat_stdout = None
plugins = set()
python_plugin_libdir = None
python_manifest_runtime_path = None
manifest_load_token = None

MANIFEST_CALLBACKS_PER_PLUGIN_MAX = 64
MANIFEST_RESPONSE_MAX = 1024 * 1024


@contextmanager
def redirected_stdout():
    sys.stdout = sys.__stdout__
    sys.stderr = sys.__stderr__
    yield
    sys.stdout = zoitechat_stdout
    sys.stderr = zoitechat_stdout


if os.getenv('ZOITECHAT_LOG_PYTHON'):
    def log(*args):
        with redirected_stdout():
            print(*args)

else:
    def log(*args):
        pass


class Stdout:
    def __init__(self):
        self.buffer = bytearray()

    def write(self, string):
        string = string.encode()
        idx = string.rfind(b'\n')
        if idx != -1:
            self.buffer += string[:idx]
            lib.zoitechat_print(lib.ph, bytes(self.buffer))
            self.buffer = bytearray(string[idx + 1:])
        else:
            self.buffer += string

    def flush(self):
        lib.zoitechat_print(lib.ph, bytes(self.buffer))
        self.buffer = bytearray()

    def isatty(self):
        return False


class Attribute:
    def __init__(self):
        self.time = 0

    def __repr__(self):
        return '<Attribute object at {}>'.format(id(self))


class Hook:
    def __init__(self, plugin, callback, userdata, is_unload, callback_key=None):
        self.is_unload = is_unload
        self.plugin = weakref.proxy(plugin)
        self.callback = callback
        self.userdata = userdata
        self.callback_key = callback_key
        self.zoitechat_hook = None
        self.handle = ffi.new_handle(weakref.proxy(self))

    def __del__(self):
        log('Removing hook', id(self))
        if self.is_unload is False:
            assert self.zoitechat_hook is not None
            lib.zoitechat_unhook(lib.ph, self.zoitechat_hook)


if sys.version_info[0] == 2:
    def compile_file(data, filename):
        return compile(data, filename, 'exec', dont_inherit=True)


    def compile_line(string):
        try:
            return compile(string, '<string>', 'eval', dont_inherit=True)

        except SyntaxError:
            return compile(string, '<string>', 'exec', dont_inherit=True)
else:
    def compile_file(data, filename):
        return compile(data, filename, 'exec', optimize=2, dont_inherit=True)


    def compile_line(string):
        return compile(string + '\n', '<string>', 'single', optimize=2, dont_inherit=True)


class Plugin:
    def __init__(self, manifest_id=None, capabilities=None):
        self.ph = None
        self.name = ''
        self.filename = ''
        self.version = ''
        self.description = ''
        self.manifest_id = manifest_id
        self.capabilities = frozenset(capabilities or ())
        self.hooks = set()
        self.callback_registrations = set()
        self.globals = {
            '__plugin': weakref.proxy(self),
            '__name__': '__main__',
        }

    def add_hook(self, callback, userdata, is_unload=False, callback_key=None):
        if self.manifest_id is not None and len(self.hooks) >= MANIFEST_CALLBACKS_PER_PLUGIN_MAX:
            raise RuntimeError("Plugin '{}' reached the 64-callback limit.".format(self.manifest_id))
        if callback_key is not None and callback_key in self.callback_registrations:
            raise ValueError("Callback '{}' is already registered.".format(callback_key[0]))

        hook = Hook(self, callback, userdata, is_unload=is_unload, callback_key=callback_key)
        self.hooks.add(hook)
        if callback_key is not None:
            self.callback_registrations.add(callback_key)
        return hook

    def remove_hook(self, hook):
        for h in self.hooks:
            if id(h) == hook:
                ud = h.userdata
                self.hooks.remove(h)
                if h.callback_key is not None:
                    self.callback_registrations.discard(h.callback_key)
                return ud

        log('Hook not found')
        return None

    def loadfile(self, filename):
        try:
            self.filename = canonical_path(filename)
            self.globals['__file__'] = self.filename
            with change_cwd(os.path.dirname(self.filename)):
                with open(self.filename, 'rb') as f:
                    data = f.read().decode('utf-8')
                compiled = compile_file(data, self.filename)
                exec(compiled, self.globals)

            try:
                self.name = self.globals['__module_name__']

            except KeyError:
                lib.zoitechat_print(lib.ph, b'Failed to load module: __module_name__ must be set')

                return False

            self.version = self.globals.get('__module_version__', '')
            self.description = self.globals.get('__module_description__', '')
            self.ph = lib.zoitechat_plugingui_add(lib.ph, self.filename.encode(), self.name.encode(),
                                                self.description.encode(), self.version.encode(), ffi.NULL)

        except Exception as e:
            lib.zoitechat_print(lib.ph, 'Failed to load module: {}'.format(e).encode())
            traceback.print_exc()
            return False

        return True

    def __del__(self):
        log('unloading', self.filename)
        for hook in list(self.hooks):
            if hook.is_unload is True:
                try:
                    hook.callback(hook.userdata)

                except Exception as e:
                    log('Failed to run hook:', e)
                    traceback.print_exc()

        del self.hooks
        if self.ph is not None:
            lib.zoitechat_plugingui_remove(lib.ph, self.ph)


class ManifestPlugin(Plugin):
    def __init__(self, manifest_id, capabilities):
        super().__init__(manifest_id=manifest_id, capabilities=capabilities)
        self.name = manifest_id
        self.interpreter = None
        self.closed = False
        self.globals['zoitechat'] = zoitechat
        self.context_reader = eval(
            "lambda: {'user_count': zoitechat.get_user_count(), "
            "'user_info': zoitechat.get_user_info()}", self.globals)
        self.host_log = eval('lambda text: zoitechat.log(text)', self.globals)
        self.host_send_message = eval(
            'lambda target, text: zoitechat.send_message(target, text)', self.globals)

    def _call(self, request):
        request_json = json.dumps(request, ensure_ascii=False, separators=(',', ':'))
        if len(request_json.encode('utf-8')) > MANIFEST_RESPONSE_MAX:
            raise RuntimeError('Isolated Python runtime request exceeds the 1 MiB limit')
        response_json = self.interpreter.call(
            lambda payload: __import__('_fabulor_manifest').handle(payload), request_json)
        if not isinstance(response_json, str) or len(response_json.encode('utf-8')) > MANIFEST_RESPONSE_MAX:
            raise RuntimeError('Isolated Python runtime returned an invalid or oversized response')
        response = json.loads(response_json)
        if not isinstance(response, dict) or not isinstance(response.get('operations'), list):
            raise RuntimeError('Isolated Python runtime returned a malformed response')
        return response

    def _apply_operations(self, operations, allow_callbacks=True):
        if len(operations) > 256:
            raise RuntimeError('Isolated Python runtime returned too many host operations')

        for operation in operations:
            if not isinstance(operation, dict):
                raise RuntimeError('Isolated Python runtime returned a malformed host operation')
            operation_name = operation.get('op')
            if operation_name == 'log':
                self.host_log(operation.get('text', ''))
            elif operation_name == 'send_message':
                self.host_send_message(operation.get('target'), operation.get('text'))
            elif operation_name == 'register_callback' and allow_callbacks:
                event_name = operation.get('event')
                handler_name = operation.get('handler')
                if not isinstance(event_name, str) or not isinstance(handler_name, str):
                    raise RuntimeError('Isolated Python callback registration is malformed')
                plugin = weakref.proxy(self)

                def isolated_callback(event, handler=handler_name, plugin_ref=plugin):
                    return plugin_ref.dispatch(handler, event)

                zoitechat_internal._register_callback_for_plugin(
                    self, event_name, isolated_callback)
            else:
                raise RuntimeError("Unsupported isolated Python host operation '{}'".format(
                    operation_name))

    def _context(self):
        context = self.context_reader()
        if not isinstance(context, dict):
            raise RuntimeError('Python host context snapshot failed')
        return context

    def loadfile(self, filename):
        try:
            if interpreters is None:
                raise RuntimeError('Python 3.14 concurrent interpreters are unavailable')
            if not python_manifest_runtime_path:
                raise RuntimeError('Trusted Python manifest runtime is unavailable')

            self.filename = canonical_path(filename)
            self.interpreter = interpreters.create()
            self.interpreter.prepare_main(
                _fabulor_runtime_path=python_manifest_runtime_path,
                _fabulor_plugin_path=os.path.dirname(self.filename),
            )
            self.interpreter.exec(
                "import importlib.util\n"
                "import sys\n"
                "_fabulor_spec = importlib.util.spec_from_file_location(\n"
                "    '_fabulor_manifest', _fabulor_runtime_path)\n"
                "if _fabulor_spec is None or _fabulor_spec.loader is None:\n"
                "    raise ImportError('Unable to load the trusted manifest runtime')\n"
                "_fabulor_runtime = importlib.util.module_from_spec(_fabulor_spec)\n"
                "sys.modules['_fabulor_manifest'] = _fabulor_runtime\n"
                "_fabulor_spec.loader.exec_module(_fabulor_runtime)\n"
                "if _fabulor_plugin_path not in sys.path:\n"
                "    sys.path.insert(0, _fabulor_plugin_path)\n"
            )
            response = self._call({
                'action': 'load',
                'plugin_id': self.manifest_id,
                'entrypoint': self.filename,
                'capabilities': sorted(self.capabilities),
                'context': self._context(),
            })
            if not response.get('ok'):
                raise RuntimeError(response.get('error') or 'isolated plugin initialisation failed')
            self._apply_operations(response['operations'])
            self.ph = lib.zoitechat_plugingui_add(
                lib.ph, self.filename.encode(), self.name.encode(),
                b'Isolated Python manifest plugin', b'', ffi.NULL)
            return True
        except Exception as error:
            print_error("Failed to load isolated Python manifest '{}': {}".format(
                self.manifest_id, error))
            self.close()
            return False

    def dispatch(self, handler_name, event):
        if self.closed or self.interpreter is None:
            return 0
        try:
            response = self._call({
                'action': 'dispatch',
                'handler': handler_name,
                'event': event,
                'context': self._context(),
            })
            if not response.get('ok'):
                raise RuntimeError(response.get('error') or 'isolated callback failed')
            self._apply_operations(response['operations'])
            return int(response.get('result') or 0)
        except Exception as error:
            print_error("Isolated Python callback failed for '{}': {}".format(
                self.manifest_id, error))
            return 0

    def close(self):
        if self.closed:
            return
        self.closed = True

        if self.interpreter is not None:
            try:
                response = self._call({'action': 'shutdown'})
                if response.get('ok'):
                    self._apply_operations(response['operations'], allow_callbacks=False)
            except Exception as error:
                log('Isolated Python shutdown failed', self.manifest_id, error)

        self.hooks.clear()
        self.callback_registrations.clear()
        if self.ph is not None:
            lib.zoitechat_plugingui_remove(lib.ph, self.ph)
            self.ph = None
        if self.interpreter is not None:
            try:
                self.interpreter.close()
            except Exception as error:
                log('Isolated Python interpreter close failed', self.manifest_id, error)
            self.interpreter = None

    def __del__(self):
        try:
            self.close()
        except Exception:
            pass


if sys.version_info[0] == 2:
    def __decode(string):
        return string

else:
    def __decode(string):
        return string.decode()


def _cstr(ptr):
    """Safely convert a C char* (possibly NULL) to bytes."""
    if ptr == ffi.NULL:
        return b''
    try:
        return ffi.string(ptr)
    except Exception:
        return b''

def wordlist_len(words):
    for i in range(31, 0, -1):
        if _cstr(words[i]):
            return i
    return 0


def create_wordlist(words):
    size = wordlist_len(words)
    return [__decode(_cstr(words[i])) for i in range(1, size + 1)]


def create_wordeollist(words):
    words = reversed(words)
    accum = None
    ret = []
    for word in words:
        if accum is None:
            accum = word

        elif word:
            last = accum
            accum = ' '.join((word, last))

        ret.insert(0, accum)

    return ret


def to_cb_ret(value):
    if value is None:
        return 0

    return int(value)


@ffi.def_extern()
def _on_command_hook(word, word_eol, userdata):
    hook = ffi.from_handle(userdata)
    word = create_wordlist(word)
    word_eol = create_wordlist(word_eol)
    return to_cb_ret(hook.callback(word, word_eol, hook.userdata))


@ffi.def_extern()
def _on_print_hook(word, userdata):
    hook = ffi.from_handle(userdata)
    word = create_wordlist(word)
    word_eol = create_wordeollist(word)
    return to_cb_ret(hook.callback(word, word_eol, hook.userdata))


@ffi.def_extern()
def _on_print_attrs_hook(word, attrs, userdata):
    hook = ffi.from_handle(userdata)
    word = create_wordlist(word)
    word_eol = create_wordeollist(word)
    attr = Attribute()
    attr.time = attrs.server_time_utc
    return to_cb_ret(hook.callback(word, word_eol, hook.userdata, attr))


@ffi.def_extern()
def _on_server_hook(word, word_eol, userdata):
    hook = ffi.from_handle(userdata)
    word = create_wordlist(word)
    word_eol = create_wordlist(word_eol)
    return to_cb_ret(hook.callback(word, word_eol, hook.userdata))


@ffi.def_extern()
def _on_server_attrs_hook(word, word_eol, attrs, userdata):
    hook = ffi.from_handle(userdata)
    word = create_wordlist(word)
    word_eol = create_wordlist(word_eol)
    attr = Attribute()
    attr.time = attrs.server_time_utc
    return to_cb_ret(hook.callback(word, word_eol, hook.userdata, attr))


@ffi.def_extern()
def _on_timer_hook(userdata):
    hook = ffi.from_handle(userdata)
    if hook.callback(hook.userdata) == True:
        return 1

    try:
        hook.is_unload = True
    except ReferenceError:
        return 0

    for h in hook.plugin.hooks:
        if h == hook:
            hook.plugin.hooks.remove(h)
            break

    return 0


@ffi.def_extern()
def _on_say_command(word, word_eol, userdata):
    """Handle input in the special >>python<< tab.

    This callback is wired via hook_command(b''), so it may be invoked for a wide range
    of internal commands. It must never throw, and must default to EAT_NONE.
    """
    try:
        channel = _cstr(lib.zoitechat_get_info(lib.ph, b'channel'))
    except Exception:
        return 0

    if channel != b'>>python<<':
        return 0

    try:
        python = __decode(_cstr(word_eol[1]))
    except Exception:
        python = ''

    if not python:
        return 1

    try:
        exec_in_interp(python)
    except Exception:
        exc = traceback.format_exc().encode('utf-8', errors='replace')
        lib.zoitechat_print(lib.ph, exc)
    return 1


def print_error(message):
    lib.zoitechat_print(lib.ph, message.encode('utf-8', 'replace'))


def config_dir():
    return __decode(_cstr(lib.zoitechat_get_info(lib.ph, b'configdir')))


def addons_dir():
    return os.path.join(config_dir(), 'addons')


def user_manifest_plugins_dir():
    return os.path.join(config_dir(), 'plugins')


def bundled_manifest_plugins_dir():
    executable = getattr(sys, 'executable', None)
    if executable:
        return os.path.join(os.path.dirname(canonical_path(executable)), 'Plugins')

    if os.getenv('FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS') == '1' and python_plugin_libdir:
        return os.path.join(python_plugin_libdir, 'Plugins')
    return None


def canonical_path(path):
    return os.path.realpath(os.path.abspath(os.path.expanduser(path)))


def path_is_under(path, root):
    if not path or not root:
        return False

    try:
        path_cmp = os.path.normcase(canonical_path(path))
        root_cmp = os.path.normcase(canonical_path(root))
        return os.path.commonpath([path_cmp, root_cmp]) == root_cmp
    except (AttributeError, ValueError):
        root_cmp = os.path.normcase(canonical_path(root))
        path_cmp = os.path.normcase(canonical_path(path))
        return path_cmp == root_cmp or path_cmp.startswith(root_cmp + os.sep)


def has_path_separator(path):
    return os.sep in path or (os.altsep is not None and os.altsep in path)


def trusted_python_roots(allow_manifest_roots):
    roots = [addons_dir()]
    if allow_manifest_roots:
        roots.append(user_manifest_plugins_dir())
        bundled_plugins_dir = bundled_manifest_plugins_dir()
        if bundled_plugins_dir:
            roots.append(bundled_plugins_dir)
    return roots


def relative_addon_candidates(filename):
    addondir = addons_dir()
    basename = os.path.basename(filename)
    stem, ext = os.path.splitext(basename)

    if not has_path_separator(filename):
        if ext.lower() == '.py':
            return [
                os.path.join(addondir, stem, basename),
                os.path.join(addondir, basename),
            ]
        if ext:
            return [os.path.join(addondir, basename)]
        return [
            os.path.join(addondir, filename, filename + '.py'),
            os.path.join(addondir, filename + '.py'),
        ]

    return [os.path.join(addondir, filename)]


def resolve_load_filename(filename, allow_manifest_roots=False):
    if not filename:
        return None

    filename = os.path.expanduser(filename)
    absolute_request = os.path.isabs(filename)
    candidates = [filename] if absolute_request else relative_addon_candidates(filename)
    roots = trusted_python_roots(allow_manifest_roots=allow_manifest_roots)

    for candidate in candidates:
        resolved = canonical_path(candidate)
        if not resolved.lower().endswith('.py'):
            continue
        if not os.path.isfile(resolved):
            continue
        if any(path_is_under(resolved, root) for root in roots):
            return resolved

    return None


def load_filename(filename, manifest_id=None, capabilities=None):
    is_manifest = manifest_id is not None
    filename = resolve_load_filename(filename, allow_manifest_roots=is_manifest)

    if not filename:
        print_error('Python load rejected: ordinary scripts must be .py files under the profile addons directory')
        return False

    if filename and not any(plugin.filename == filename for plugin in plugins):
        if is_manifest:
            plugin = ManifestPlugin(manifest_id=manifest_id, capabilities=capabilities)
        else:
            plugin = Plugin(manifest_id=manifest_id, capabilities=capabilities)
        if plugin.loadfile(filename):
            plugins.add(plugin)
            return True
        if isinstance(plugin, ManifestPlugin):
            plugin.close()

    return False


def unload_name(name):
    if name:
        for plugin in plugins:
            if name in (plugin.name, plugin.filename, os.path.basename(plugin.filename)):
                if plugin.manifest_id is not None:
                    print_error('Python unload rejected: manifest plugins are owned by the manifest host')
                    return False
                plugins.remove(plugin)
                return True

    return False


def reload_name(name):
    if name:
        for plugin in plugins:
            if name in (plugin.name, plugin.filename, os.path.basename(plugin.filename)):
                if plugin.manifest_id is not None:
                    print_error('Python reload rejected: manifest plugins are owned by the manifest host')
                    return False
                filename = plugin.filename
                plugins.remove(plugin)
                return load_filename(filename)

    return False


@contextmanager
def change_cwd(path):
    old_cwd = os.getcwd()
    os.chdir(path)
    try:
        yield
    finally:
        os.chdir(old_cwd)


def autoload():
    addondir = addons_dir()
    try:
        with change_cwd(addondir):
            for f in sorted(os.listdir(addondir)):
                candidate = os.path.join(addondir, f, f + '.py')
                if os.path.isfile(candidate):
                    log('Autoloading', candidate)
                    load_filename(candidate)

            for f in sorted(os.listdir(addondir)):
                candidate = os.path.join(addondir, f)
                if os.path.isfile(candidate) and f.endswith('.py'):
                    log('Autoloading legacy flat addon', candidate)
                    load_filename(candidate)

    except FileNotFoundError as e:
        log('Autoload failed', e)


def list_plugins():
    if not plugins:
        lib.zoitechat_print(lib.ph, b'No python modules loaded')
        return

    tbl_headers = [b'Name', b'Version', b'Filename', b'Description']
    tbl = [
        tbl_headers,
        [(b'-' * len(s)) for s in tbl_headers]
    ]

    for plugin in plugins:
        basename = os.path.basename(plugin.filename).encode()
        name = plugin.name.encode()
        version = plugin.version.encode() if plugin.version else b'<none>'
        description = plugin.description.encode() if plugin.description else b'<none>'
        tbl.append((name, version, basename, description))

    column_sizes = [
        max(len(item) for item in column)
        for column in zip(*tbl)
    ]

    for row in tbl:
        lib.zoitechat_print(lib.ph, b' '.join(item.ljust(column_sizes[i])
                                            for i, item in enumerate(row)))
    lib.zoitechat_print(lib.ph, b'')


def exec_in_interp(python):
    global local_interp

    if not python:
        return

    if local_interp is None:
        local_interp = Plugin()
        local_interp.locals = {}
        local_interp.globals['zoitechat'] = zoitechat

    code = compile_line(python)
    try:
        ret = eval(code, local_interp.globals, local_interp.locals)
        if ret is not None:
            lib.zoitechat_print(lib.ph, '{}'.format(ret).encode())

    except Exception as e:
        traceback.print_exc(file=zoitechat_stdout)


@ffi.def_extern()
def _on_load_command(word, word_eol, userdata):
    filename = _cstr(word[2])
    if filename.endswith(b'.py'):
        load_filename(__decode(filename))
        return 3

    return 0


@ffi.def_extern()
def _on_unload_command(word, word_eol, userdata):
    filename = _cstr(word[2])
    if filename.endswith(b'.py'):
        unload_name(__decode(filename))
        return 3

    return 0


@ffi.def_extern()
def _on_reload_command(word, word_eol, userdata):
    filename = _cstr(word[2])
    if filename.endswith(b'.py'):
        reload_name(__decode(filename))
        return 3

    return 0


@ffi.def_extern(error=3)
def _on_py_command(word, word_eol, userdata):
    global manifest_load_token

    subcmd = __decode(ffi.string(word[2])).lower()

    if subcmd == 'manifest_init':
        token = __decode(_cstr(word[3]))
        if token and manifest_load_token is None:
            manifest_load_token = token

    elif subcmd == 'manifest_load':
        token = __decode(_cstr(word[3]))
        if not manifest_load_token or token != manifest_load_token:
            print_error('Python manifest load rejected: invalid host token')
        else:
            try:
                manifest_id = base64.b64decode(_cstr(word[4]), validate=True).decode('utf-8')
                capability_text = base64.b64decode(_cstr(word[5]), validate=True).decode('utf-8')
                filename = base64.b64decode(_cstr(word[6]), validate=True).decode('utf-8')
                if not capability_text.startswith('capabilities:'):
                    raise ValueError('invalid capability metadata')
                capability_text = capability_text[len('capabilities:'):]
                capabilities = capability_text.split(',') if capability_text else []
                load_filename(filename, manifest_id=manifest_id, capabilities=capabilities)
            except (ValueError, UnicodeDecodeError):
                print_error('Python manifest load rejected: malformed host metadata')

    elif subcmd == 'exec':
        python = __decode(ffi.string(word_eol[3]))
        exec_in_interp(python)

    elif subcmd == 'load':
        filename = __decode(ffi.string(word[3]))
        load_filename(filename)

    elif subcmd == 'unload':
        name = __decode(ffi.string(word[3]))
        if not unload_name(name):
            lib.zoitechat_print(lib.ph, b'Can\'t find a python plugin with that name')

    elif subcmd == 'reload':
        name = __decode(ffi.string(word[3]))
        if not reload_name(name):
            lib.zoitechat_print(lib.ph, b'Can\'t find a python plugin with that name')

    elif subcmd == 'console':
        lib.zoitechat_command(lib.ph, b'QUERY >>python<<')

    elif subcmd == 'list':
        list_plugins()

    elif subcmd == 'about':
        lib.zoitechat_print(lib.ph, b'ZoiteChat Python interface version ' + VERSION)

    else:
        lib.zoitechat_command(lib.ph, b'HELP PY')

    return 3


@ffi.def_extern()
def _on_plugin_init(plugin_name, plugin_desc, plugin_version, arg, libdir):
    global zoitechat
    global zoitechat_internal
    global zoitechat_stdout
    global python_plugin_libdir
    global python_manifest_runtime_path

    signal.signal(signal.SIGINT, signal.SIG_DFL)

    plugin_name[0] = PLUGIN_NAME
    plugin_desc[0] = PLUGIN_DESC
    plugin_version[0] = PLUGIN_VERSION

    try:
        libdir = __decode(_cstr(libdir))
        python_plugin_libdir = canonical_path(libdir)
        modpaths = [
            os.path.abspath(os.path.join(libdir, '..', 'python')),
            os.path.abspath(os.path.join(libdir, 'python')),
        ]

        appdir = os.getenv('APPDIR')
        if appdir:
            modpaths.extend([
                os.path.join(appdir, 'usr', 'lib', 'zoitechat', 'python'),
            ])

        for modpath in modpaths:
            if os.path.isdir(modpath) and modpath not in sys.path:
                sys.path.append(modpath)

        zoitechat = importlib.import_module('zoitechat')
        zoitechat_internal = importlib.import_module('_zoitechat')
        python_manifest_runtime_path = next((canonical_path(os.path.join(
            path, '_fabulor_manifest.py')) for path in modpaths
            if os.path.isfile(os.path.join(path, '_fabulor_manifest.py'))), None)

    except (UnicodeDecodeError, ImportError) as e:
        lib.zoitechat_print(lib.ph, b'Failed to import module: ' + repr(e).encode())

        return 0

    zoitechat_stdout = Stdout()
    sys.stdout = zoitechat_stdout
    sys.stderr = zoitechat_stdout
    pydoc.help = pydoc.Helper(HelpEater(), HelpEater())

    lib.zoitechat_hook_command(lib.ph, b'', 0, lib._on_say_command, ffi.NULL, ffi.NULL)
    lib.zoitechat_hook_command(lib.ph, b'LOAD', 0, lib._on_load_command, ffi.NULL, ffi.NULL)
    lib.zoitechat_hook_command(lib.ph, b'UNLOAD', 0, lib._on_unload_command, ffi.NULL, ffi.NULL)
    lib.zoitechat_hook_command(lib.ph, b'RELOAD', 0, lib._on_reload_command, ffi.NULL, ffi.NULL)
    lib.zoitechat_hook_command(lib.ph, b'PY', 0, lib._on_py_command, b'''Usage: /PY LOAD   <filename>
           UNLOAD <filename|name>
           RELOAD <filename|name>
           LIST
           EXEC <command>
           CONSOLE
           ABOUT''', ffi.NULL)

    lib.zoitechat_print(lib.ph, b'Python interface loaded')
    autoload()
    return 1


@ffi.def_extern()
def _on_plugin_deinit():
    global local_interp
    global zoitechat
    global zoitechat_internal
    global zoitechat_stdout
    global plugins
    global python_plugin_libdir
    global python_manifest_runtime_path
    global manifest_load_token

    for plugin in list(plugins):
        if isinstance(plugin, ManifestPlugin):
            plugin.close()
    plugins = set()
    local_interp = None
    zoitechat = None
    zoitechat_internal = None
    zoitechat_stdout = None
    python_plugin_libdir = None
    python_manifest_runtime_path = None
    manifest_load_token = None
    sys.stdout = sys.__stdout__
    sys.stderr = sys.__stderr__
    pydoc.help = pydoc.Helper()

    for mod in ('_zoitechat', 'zoitechat', 'xchat', '_zoitechat_embedded'):
        try:
            del sys.modules[mod]

        except KeyError:
            pass

    return 1
