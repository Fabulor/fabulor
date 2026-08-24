import inspect
import sys
import unicodedata
from contextlib import contextmanager

from _fabulor_embedded import ffi, lib

__all__ = [
    'EAT_ALL', 'EAT_FABULOR', 'EAT_NONE', 'EAT_PLUGIN', 'EAT_XCHAT', 'EAT_HEXCHAT',
    'PRI_HIGH', 'PRI_HIGHEST', 'PRI_LOW', 'PRI_LOWEST', 'PRI_NORM',
    '__doc__', '__version__', 'command', 'del_pluginpref', 'emit_print',
    'find_context', 'get_context', 'get_info', 'get_user_count', 'get_user_info', 'log',
    'get_list', 'get_lists', 'get_pluginpref', 'get_prefs', 'hook_command',
    'hook_print', 'hook_print_attrs', 'hook_server', 'hook_server_attrs',
    'hook_timer', 'hook_unload', 'list_pluginpref', 'nickcmp', 'prnt',
    'register_callback', 'send_message', 'set_pluginpref', 'strip', 'unhook',
]

__doc__ = 'Fabulor Python Scripting Interface'
__version__ = (2, 0)
__license__ = 'GPL-2.0+'

EAT_NONE = 0
EAT_FABULOR = 1
EAT_XCHAT = EAT_FABULOR
EAT_HEXCHAT = EAT_FABULOR
EAT_PLUGIN = 2
EAT_ALL = EAT_FABULOR | EAT_PLUGIN

PRI_LOWEST = -128
PRI_LOW = -64
PRI_NORM = 0
PRI_HIGH = 64
PRI_HIGHEST = 127


# We need each module to be able to reference their parent plugin
# which is a bit tricky since they all share the exact same module.
# Simply navigating up to what module called it seems to actually
# be a fairly reliable and simple method of doing so if ugly.
def __get_current_plugin():
    frame = inspect.stack()[1][0]
    while '__plugin' not in frame.f_globals:
        frame = frame.f_back
        assert frame is not None

    return frame.f_globals['__plugin']


def __require_capability(capability):
    plugin = __get_current_plugin()
    return _require_plugin_capability(plugin, capability)


# Class methods cannot call a module-level double-underscore name directly
# because Python applies class name mangling at compile time.
_require_capability = __require_capability


def _require_plugin_capability(plugin, capability):
    if plugin.manifest_id is not None and capability not in plugin.capabilities:
        raise PermissionError("Plugin '{}' lacks required capability '{}'.".format(
            plugin.manifest_id, capability))
    return plugin


# Keeping API compat
if sys.version_info[0] == 2:
    def __decode(string):
        return string

else:
    def __decode(string):
        return string.decode()


# ------------ API ------------
def prnt(string):
    __require_capability('ui.write')
    lib.fabulor_print(lib.ph, string.encode())


def emit_print(event_name, *args, **kwargs):
    __require_capability('ui.write')
    time = kwargs.pop('time', 0)  # For py2 compat
    cargs = []
    for i in range(4):
        arg = args[i].encode() if len(args) > i else b''
        cstring = ffi.new('char[]', arg)
        cargs.append(cstring)

    if time == 0:
        return lib.fabulor_emit_print(lib.ph, event_name.encode(), *cargs)

    attrs = lib.fabulor_event_attrs_create(lib.ph)
    attrs.server_time_utc = time
    ret = lib.fabulor_emit_print_attrs(lib.ph, attrs, event_name.encode(), *cargs)
    lib.fabulor_event_attrs_free(lib.ph, attrs)
    return ret


def command(command):
    __require_capability('commands.execute')
    lib.fabulor_command(lib.ph, command.encode())


def log(text):
    plugin = __get_current_plugin()
    prefix = '[Python:{}] '.format(plugin.manifest_id) if plugin.manifest_id is not None else ''
    lib.fabulor_print(lib.ph, (prefix + text).encode())


def send_message(target, text):
    __require_capability('messages.write')
    lib.fabulor_command(lib.ph, 'MSG {} {}'.format(target, text).encode())


def nickcmp(string1, string2):
    __require_capability('session.read')
    return lib.fabulor_nickcmp(lib.ph, string1.encode(), string2.encode())


def strip(text, length=-1, flags=3):
    stripped = lib.fabulor_strip(lib.ph, text.encode(), length, flags)
    ret = __decode(ffi.string(stripped))
    lib.fabulor_free(lib.ph, stripped)
    return ret


def get_info(name):
    __require_capability('session.read')
    ret = lib.fabulor_get_info(lib.ph, name.encode())
    if ret == ffi.NULL:
        return None
    if name in ('gtkwin_ptr', 'win_ptr'):
        # Surely there is a less dumb way?
        ptr = repr(ret).rsplit(' ', 1)[1][:-1]
        return ptr

    return __decode(ffi.string(ret))


def get_prefs(name):
    __require_capability('preferences.read')
    string_out = ffi.new('char**')
    int_out = ffi.new('int*')
    _type = lib.fabulor_get_prefs(lib.ph, name.encode(), string_out, int_out)
    if _type == 0:
        return None

    if _type == 1:
        return __decode(ffi.string(string_out[0]))

    if _type in (2, 3):  # XXX: 3 should be a bool, but keeps API
        return int_out[0]

    raise AssertionError('Out of bounds pref storage')


def __cstrarray_to_list(arr):
    i = 0
    ret = []
    while arr[i] != ffi.NULL:
        ret.append(ffi.string(arr[i]))
        i += 1

    return ret


__FIELD_CACHE = {}


def __get_fields(name):
    return __FIELD_CACHE.setdefault(name, __cstrarray_to_list(lib.fabulor_list_fields(lib.ph, name)))


__FIELD_PROPERTY_CACHE = {}


def __cached_decoded_str(string):
    return __FIELD_PROPERTY_CACHE.setdefault(string, __decode(string))


def get_lists():
    __require_capability('session.read')
    return [__cached_decoded_str(field) for field in __get_fields(b'lists')]


def get_user_count():
    __require_capability('session.read')
    users = get_list('users')
    if users is None:
        return 0

    return len(users)


def get_user_info():
    __require_capability('session.read')
    return {
        'nickname': get_info('nick'),
        'channel': get_info('channel'),
        'server_name': get_info('server'),
        'network_name': get_info('network'),
    }


class ListItem:
    def __init__(self, name):
        self._listname = name

    def __repr__(self):
        return '<{} list item at {}>'.format(self._listname, id(self))


# done this way for speed
if sys.version_info[0] == 2:
    def get_getter(name):
        return ord(name[0])

else:
    def get_getter(name):
        return name[0]


def get_list(name):
    __require_capability('session.read')
    # XXX: This function is extremely inefficient and could be interators and
    # lazily loaded properties, but for API compat we stay slow
    orig_name = name
    name = name.encode()

    if name not in __get_fields(b'lists'):
        raise KeyError('list not available')

    list_ = lib.fabulor_list_get(lib.ph, name)
    if list_ == ffi.NULL:
        return None

    ret = []
    fields = __get_fields(name)

    def string_getter(field):
        string = lib.fabulor_list_str(lib.ph, list_, field)
        if string != ffi.NULL:
            return __decode(ffi.string(string))

        return ''

    def ptr_getter(field):
        if field == b'context':
            ptr = lib.fabulor_list_str(lib.ph, list_, field)
            ctx = ffi.cast('fabulor_context*', ptr)
            return Context(ctx)

        return None

    getters = {
        ord('s'): string_getter,
        ord('i'): lambda field: lib.fabulor_list_int(lib.ph, list_, field),
        ord('t'): lambda field: lib.fabulor_list_time(lib.ph, list_, field),
        ord('p'): ptr_getter,
    }

    while lib.fabulor_list_next(lib.ph, list_) == 1:
        item = ListItem(orig_name)
        for _field in fields:
            getter = getters.get(get_getter(_field))
            if getter is not None:
                field_name = _field[1:]
                setattr(item, __cached_decoded_str(field_name), getter(field_name))

        ret.append(item)

    lib.fabulor_list_free(lib.ph, list_)
    return ret


def hook_command(command, callback, userdata=None, priority=PRI_NORM, help=None):
    __require_capability('events.command')
    plugin = __get_current_plugin()
    hook = plugin.add_hook(callback, userdata)
    handle = lib.fabulor_hook_command(lib.ph, command.encode(), priority, lib._on_command_hook,
                                      help.encode() if help is not None else ffi.NULL, hook.handle)

    hook.fabulor_hook = handle
    return id(hook)


def hook_print(name, callback, userdata=None, priority=PRI_NORM):
    __require_capability('events.print')
    plugin = __get_current_plugin()
    hook = plugin.add_hook(callback, userdata)
    handle = lib.fabulor_hook_print(lib.ph, name.encode(), priority, lib._on_print_hook, hook.handle)
    hook.fabulor_hook = handle
    return id(hook)


def hook_print_attrs(name, callback, userdata=None, priority=PRI_NORM):
    __require_capability('events.print')
    plugin = __get_current_plugin()
    hook = plugin.add_hook(callback, userdata)
    handle = lib.fabulor_hook_print_attrs(lib.ph, name.encode(), priority, lib._on_print_attrs_hook, hook.handle)
    hook.fabulor_hook = handle
    return id(hook)


def hook_server(name, callback, userdata=None, priority=PRI_NORM):
    __require_capability('events.server')
    plugin = __get_current_plugin()
    hook = plugin.add_hook(callback, userdata)
    handle = lib.fabulor_hook_server(lib.ph, name.encode(), priority, lib._on_server_hook, hook.handle)
    hook.fabulor_hook = handle
    return id(hook)


def hook_server_attrs(name, callback, userdata=None, priority=PRI_NORM):
    __require_capability('events.server')
    plugin = __get_current_plugin()
    hook = plugin.add_hook(callback, userdata)
    handle = lib.fabulor_hook_server_attrs(lib.ph, name.encode(), priority, lib._on_server_attrs_hook, hook.handle)
    hook.fabulor_hook = handle
    return id(hook)


def hook_timer(timeout, callback, userdata=None):
    __require_capability('events.timer')
    plugin = __get_current_plugin()
    hook = plugin.add_hook(callback, userdata)
    handle = lib.fabulor_hook_timer(lib.ph, timeout, lib._on_timer_hook, hook.handle)
    hook.fabulor_hook = handle
    return id(hook)


def hook_unload(callback, userdata=None):
    __require_capability('events.unload')
    plugin = __get_current_plugin()
    hook = plugin.add_hook(callback, userdata, is_unload=True)
    return id(hook)


def __validate_callback_event(event_name):
    if not isinstance(event_name, str):
        raise TypeError('Callback event names must be strings')

    event_name = event_name.lower()
    if not event_name or len(event_name.encode('utf-8')) > 128:
        raise ValueError('Callback event names must be between 1 and 128 UTF-8 bytes')
    if any(unicodedata.category(character).startswith('C') for character in event_name):
        raise ValueError('Callback event names cannot contain control characters')

    if event_name in ('message', 'server'):
        return event_name
    if any(event_name.startswith(prefix) and len(event_name) > len(prefix)
           for prefix in ('server:', 'print:', 'command:')):
        return event_name

    raise ValueError('Supported callback events are "message", "server", "server:<name>", "print:<event>", and "command:<name>"')


def register_callback(event_name, callback, userdata=None):
    return _register_callback_for_plugin(__get_current_plugin(), event_name, callback, userdata)


def _register_callback_for_plugin(plugin, event_name, callback, userdata=None):
    event_name = __validate_callback_event(event_name)
    callback_key = (event_name, id(callback))

    def build_event(words, word_eol, local_userdata, attrs, source_name):
        event = {
            'event': event_name,
            'source': source_name,
            'words': words,
            'word_eol': word_eol,
            'time': getattr(attrs, 'time', 0),
            'userdata': local_userdata,
        }
        return callback(event)

    if event_name == 'message':
        _require_plugin_capability(plugin, 'events.message')

        def on_message(words, word_eol, local_userdata, attrs):
            return build_event(words, word_eol, local_userdata, attrs, 'PRIVMSG')

        hook = plugin.add_hook(on_message, userdata, callback_key=callback_key)
        handle = lib.fabulor_hook_server_attrs(lib.ph, b'PRIVMSG', PRI_NORM,
                                                lib._on_server_attrs_hook, hook.handle)
        hook.fabulor_hook = handle
        return id(hook)

    if event_name == 'server':
        _require_plugin_capability(plugin, 'events.server')

        def on_server(words, word_eol, local_userdata, attrs):
            source_name = words[0] if words else 'RAW LINE'
            return build_event(words, word_eol, local_userdata, attrs, source_name)

        hook = plugin.add_hook(on_server, userdata, callback_key=callback_key)
        handle = lib.fabulor_hook_server_attrs(lib.ph, b'RAW LINE', PRI_NORM,
                                                lib._on_server_attrs_hook, hook.handle)
        hook.fabulor_hook = handle
        return id(hook)

    if event_name.startswith('server:'):
        _require_plugin_capability(plugin, 'events.server')
        server_name = event_name.split(':', 1)[1].upper()

        def on_named_server(words, word_eol, local_userdata, attrs):
            return build_event(words, word_eol, local_userdata, attrs, server_name)

        hook = plugin.add_hook(on_named_server, userdata, callback_key=callback_key)
        handle = lib.fabulor_hook_server_attrs(lib.ph, server_name.encode(), PRI_NORM,
                                                lib._on_server_attrs_hook, hook.handle)
        hook.fabulor_hook = handle
        return id(hook)

    if event_name.startswith('print:'):
        _require_plugin_capability(plugin, 'events.print')
        print_name = event_name.split(':', 1)[1]

        def on_print(words, word_eol, local_userdata, attrs):
            source_name = words[0] if words else print_name
            return build_event(words, word_eol, local_userdata, attrs, source_name)

        hook = plugin.add_hook(on_print, userdata, callback_key=callback_key)
        handle = lib.fabulor_hook_print_attrs(lib.ph, print_name.encode(), PRI_NORM,
                                               lib._on_print_attrs_hook, hook.handle)
        hook.fabulor_hook = handle
        return id(hook)

    if event_name.startswith('command:'):
        _require_plugin_capability(plugin, 'events.command')
        command_name = event_name.split(':', 1)[1].upper()

        def on_command(words, word_eol, local_userdata):
            event = {
                'event': event_name,
                'source': command_name,
                'words': words,
                'word_eol': word_eol,
                'time': 0,
                'userdata': local_userdata,
            }
            return callback(event)

        hook = plugin.add_hook(on_command, userdata, callback_key=callback_key)
        handle = lib.fabulor_hook_command(lib.ph, command_name.encode(), PRI_NORM,
                                          lib._on_command_hook, ffi.NULL, hook.handle)
        hook.fabulor_hook = handle
        return id(hook)


def unhook(handle):
    plugin = __get_current_plugin()
    return plugin.remove_hook(handle)


def set_pluginpref(name, value):
    __require_capability('preferences.write')
    if isinstance(value, str):
        return bool(lib.fabulor_pluginpref_set_str(lib.ph, name.encode(), value.encode()))

    if isinstance(value, int):
        return bool(lib.fabulor_pluginpref_set_int(lib.ph, name.encode(), value))

    # XXX: This should probably raise but this keeps API
    return False


def get_pluginpref(name):
    __require_capability('preferences.read')
    name = name.encode()
    string_out = ffi.new('char[512]')
    if lib.fabulor_pluginpref_get_str(lib.ph, name, string_out) != 1:
        return None

    string = ffi.string(string_out)
    # This API stores everything as a string so we have to figure out what
    # its actual type was supposed to be.
    if len(string) > 12:  # Can't be a number
        return __decode(string)

    number = lib.fabulor_pluginpref_get_int(lib.ph, name)
    if number == -1 and string != b'-1':
        return __decode(string)

    return number


def del_pluginpref(name):
    __require_capability('preferences.write')
    return bool(lib.fabulor_pluginpref_delete(lib.ph, name.encode()))


def list_pluginpref():
    __require_capability('preferences.read')
    prefs_str = ffi.new('char[4096]')
    if lib.fabulor_pluginpref_list(lib.ph, prefs_str) == 1:
        preference_names = __decode(ffi.string(prefs_str)).rstrip(',')
        return preference_names.split(',') if preference_names else []

    return []


class Context:
    def __init__(self, ctx):
        self._ctx = ctx

    def __eq__(self, value):
        if not isinstance(value, Context):
            return False

        return self._ctx == value._ctx

    @contextmanager
    def __change_context(self):
        old_ctx = lib.fabulor_get_context(lib.ph)
        if not self.set():
            # XXX: Behavior change, previously used wrong context
            lib.fabulor_print(lib.ph, b'Context object refers to closed context, ignoring call')
            return

        try:
            yield
        finally:
            lib.fabulor_set_context(lib.ph, old_ctx)

    def set(self):
        _require_capability('session.read')
        # XXX: API addition, C plugin silently ignored failure
        return bool(lib.fabulor_set_context(lib.ph, self._ctx))

    def prnt(self, string):
        with self.__change_context():
            prnt(string)

    def emit_print(self, event_name, *args, **kwargs):
        time = kwargs.pop('time', 0)  # For py2 compat
        with self.__change_context():
            return emit_print(event_name, *args, time=time)

    def command(self, string):
        with self.__change_context():
            command(string)

    def get_info(self, name):
        with self.__change_context():
            return get_info(name)

    def get_list(self, name):
        with self.__change_context():
            return get_list(name)


def get_context():
    __require_capability('session.read')
    ctx = lib.fabulor_get_context(lib.ph)
    return Context(ctx)


def find_context(server=None, channel=None):
    __require_capability('session.read')
    server = server.encode() if server is not None else ffi.NULL
    channel = channel.encode() if channel is not None else ffi.NULL
    ctx = lib.fabulor_find_context(lib.ph, server, channel)
    if ctx == ffi.NULL:
        return None

    return Context(ctx)
