import json
import os
import sys
import traceback
import types
import unicodedata


CALLBACKS_PER_PLUGIN_MAX = 64
OPERATIONS_PER_CALL_MAX = 256
EVENT_NAME_MAX = 128
LOG_TEXT_MAX = 65536
MESSAGE_TARGET_MAX = 512
MESSAGE_TEXT_MAX = 4096
ENTRYPOINT_MAX = 1024 * 1024
MESSAGE_MAX = 1024 * 1024

_state = None


class _PluginState:
    def __init__(self, plugin_id, capabilities, context):
        self.plugin_id = plugin_id
        self.capabilities = frozenset(capabilities)
        self.context = context
        self.callbacks = {}
        self.callback_keys = set()
        self.callback_counter = 0
        self.operations = []
        self.namespace = None

    def require(self, capability):
        if capability not in self.capabilities:
            raise PermissionError("Plugin '{}' lacks required capability '{}'.".format(
                self.plugin_id, capability))

    def record(self, operation):
        if len(self.operations) >= OPERATIONS_PER_CALL_MAX:
            raise RuntimeError('Manifest plugin produced too many host operations in one call')
        self.operations.append(operation)


def _require_state():
    if _state is None:
        raise RuntimeError('Manifest plugin runtime is not initialised')
    return _state


def _bounded_string(value, name, maximum):
    if not isinstance(value, str):
        raise TypeError('{} must be a string'.format(name))
    if len(value.encode('utf-8')) > maximum:
        raise ValueError('{} exceeds {} UTF-8 bytes'.format(name, maximum))
    return value


def _validate_event_name(event_name):
    event_name = _bounded_string(event_name, 'Callback event name', EVENT_NAME_MAX).lower()
    if not event_name:
        raise ValueError('Callback event name cannot be empty')
    if any(unicodedata.category(character).startswith('C') for character in event_name):
        raise ValueError('Callback event names cannot contain control characters')
    if event_name in ('message', 'server'):
        return event_name
    if any(event_name.startswith(prefix) and len(event_name) > len(prefix)
           for prefix in ('server:', 'print:', 'command:')):
        return event_name
    raise ValueError('Unsupported callback event {}'.format(event_name))


def log(text):
    state = _require_state()
    text = _bounded_string(str(text), 'Log text', LOG_TEXT_MAX)
    state.record({'op': 'log', 'text': text})


def send_message(target, text):
    state = _require_state()
    state.require('messages.write')
    target = _bounded_string(target, 'Message target', MESSAGE_TARGET_MAX)
    text = _bounded_string(text, 'Message text', MESSAGE_TEXT_MAX)
    if not target:
        raise ValueError('Message target cannot be empty')
    state.record({'op': 'send_message', 'target': target, 'text': text})


def get_user_count():
    state = _require_state()
    state.require('session.read')
    return int(state.context.get('user_count', 0))


def get_user_info():
    state = _require_state()
    state.require('session.read')
    user_info = state.context.get('user_info') or {}
    return dict(user_info)


def register_callback(event_name, callback, userdata=None):
    state = _require_state()
    if not callable(callback):
        raise TypeError('Callback must be callable')

    event_name = _validate_event_name(event_name)
    capability = 'events.' + event_name.split(':', 1)[0]
    state.require(capability)
    callback_key = (event_name, id(callback))
    if callback_key in state.callback_keys:
        raise ValueError("Callback '{}' is already registered.".format(event_name))
    if len(state.callbacks) >= CALLBACKS_PER_PLUGIN_MAX:
        raise RuntimeError("Plugin '{}' reached the 64-callback limit.".format(state.plugin_id))

    state.callback_counter += 1
    handler_name = 'python_callback_{}'.format(state.callback_counter)
    state.callbacks[handler_name] = (callback, userdata)
    state.callback_keys.add(callback_key)
    state.record({'op': 'register_callback', 'event': event_name, 'handler': handler_name})
    return state.callback_counter


def _install_api_module():
    for module_name in ('_fabulor', '_fabulor_embedded', '_zoitechat',
                        '_zoitechat_embedded', 'zoitechat', 'hexchat', 'xchat'):
        sys.modules[module_name] = None

    module = types.ModuleType('fabulor')
    module.__doc__ = 'Isolated Fabulor manifest plugin API'
    module.__all__ = [
        'get_user_count', 'get_user_info', 'log', 'register_callback', 'send_message'
    ]
    module.get_user_count = get_user_count
    module.get_user_info = get_user_info
    module.log = log
    module.register_callback = register_callback
    module.send_message = send_message
    sys.modules['fabulor'] = module


def _response(ok, result=None, error=None):
    state = _state
    operations = list(state.operations) if state is not None else []
    if state is not None:
        state.operations.clear()
    response = json.dumps({
        'ok': bool(ok),
        'result': result,
        'operations': operations,
        'error': error,
    }, ensure_ascii=False, separators=(',', ':'))
    if len(response.encode('utf-8')) > MESSAGE_MAX:
        return json.dumps({
            'ok': False,
            'result': None,
            'operations': [],
            'error': 'Manifest plugin response exceeded the 1 MiB limit',
        }, separators=(',', ':'))
    return response


def _load(request):
    global _state

    if _state is not None:
        raise RuntimeError('Manifest interpreter is already assigned to a plugin')

    plugin_id = _bounded_string(request.get('plugin_id'), 'Plugin id', 128)
    entrypoint = request.get('entrypoint')
    capabilities = request.get('capabilities')
    context = request.get('context') or {}
    if not isinstance(capabilities, list) or not all(isinstance(item, str) for item in capabilities):
        raise TypeError('Capabilities must be a list of strings')
    if not isinstance(entrypoint, str) or not os.path.isabs(entrypoint) or not os.path.isfile(entrypoint):
        raise ValueError('Manifest entrypoint must be an absolute regular file')
    if os.path.getsize(entrypoint) > ENTRYPOINT_MAX:
        raise ValueError('Manifest entrypoint exceeds the 1 MiB limit')

    _state = _PluginState(plugin_id, capabilities, context)
    _install_api_module()
    namespace = {
        '__file__': entrypoint,
        '__name__': '__fabulor_manifest_plugin__',
        '__package__': None,
    }
    _state.namespace = namespace
    with open(entrypoint, 'rb') as handle:
        source = handle.read().decode('utf-8')
    exec(compile(source, entrypoint, 'exec', dont_inherit=True, optimize=2), namespace)
    init = namespace.get('init')
    if callable(init):
        init()
    return _response(True)


def _dispatch(request):
    state = _require_state()
    handler_name = request.get('handler')
    if handler_name not in state.callbacks:
        raise KeyError("Unknown Python callback '{}'".format(handler_name))
    event = request.get('event')
    context = request.get('context') or {}
    if not isinstance(event, dict) or not isinstance(context, dict):
        raise TypeError('Callback event and context must be objects')

    state.context = context
    callback, userdata = state.callbacks[handler_name]
    event = dict(event)
    event['userdata'] = userdata
    result = callback(event)
    if result is None:
        result = 0
    result = int(result)
    if result < -2147483648 or result > 2147483647:
        raise ValueError('Callback result is outside the supported integer range')
    return _response(True, result=result)


def _shutdown():
    global _state

    state = _state
    if state is not None and state.namespace is not None:
        deinit = state.namespace.get('deinit')
        if callable(deinit):
            deinit()
    response = _response(True)
    _state = None
    sys.modules.pop('fabulor', None)
    return response


def handle(request_json):
    try:
        if not isinstance(request_json, str) or len(request_json.encode('utf-8')) > MESSAGE_MAX:
            raise ValueError('Manifest runtime request exceeds the 1 MiB limit')
        request = json.loads(request_json)
        if not isinstance(request, dict):
            raise TypeError('Manifest runtime request must be an object')
        action = request.get('action')
        if action == 'load':
            return _load(request)
        if action == 'dispatch':
            return _dispatch(request)
        if action == 'shutdown':
            return _shutdown()
        raise ValueError("Unknown manifest runtime action '{}'".format(action))
    except Exception:
        error = traceback.format_exc(limit=20)
        if len(error) > LOG_TEXT_MAX:
            error = error[-LOG_TEXT_MAX:]
        return _response(False, error=error)
