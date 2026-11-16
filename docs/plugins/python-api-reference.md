# Python API Reference

## Scope

This reference documents the full `fabulor` module available to trusted simple
Python add-ons. Manifest plugins receive the smaller interface documented in
the [Python Plugin Guide](python-plugin-guide.md#manifest-api).

The scripting interface reports API version `(2, 0)`. Fabulor embeds Python
3.14; write new scripts for Python 3.

Trusted simple add-ons do not declare manifest capabilities. Capability notes
in this reference describe the checks applied when the full bridge is used by
a manifest-owned plugin path; ordinary scripts under the profile `addons`
directory are trusted local code.

## Constants

### Hook priorities

Hooks with higher priorities run before hooks with lower priorities:

| Constant | Value |
| --- | ---: |
| `PRI_HIGHEST` | `127` |
| `PRI_HIGH` | `64` |
| `PRI_NORM` | `0` |
| `PRI_LOW` | `-64` |
| `PRI_LOWEST` | `-128` |

### Event-consumption values

A command, print, or server callback returns one of these values:

| Constant | Effect |
| --- | --- |
| `EAT_NONE` | Continue normal Fabulor processing and call later plugins. |
| `EAT_FABULOR` | Stop Fabulor's normal handling but continue later plugins. |
| `EAT_PLUGIN` | Stop later plugins but continue Fabulor's normal handling. |
| `EAT_ALL` | Stop both normal handling and later plugins. |

`EAT_XCHAT` and `EAT_HEXCHAT` are compatibility aliases for `EAT_FABULOR`.
Returning `None` is equivalent to `EAT_NONE`.

## Output and Commands

### `prnt(text)`

Print text in the current context. Requires `ui.write` when used by a
capability-bound plugin.

```python
fabulor.prnt("Add-on ready")
```

### `emit_print(event_name, *args, time=0)`

Emit a named Fabulor print event. Up to four positional event arguments are
accepted. `time` supplies an IRC server timestamp when non-zero. Requires
`ui.write`.

```python
fabulor.emit_print("Channel Message", "Barry", "Hello")
```

### `command(text)`

Run a Fabulor command in the current context. Do not include the leading `/`.
Requires `commands.execute`.

```python
fabulor.command("JOIN #fabulor")
```

### `log(text)`

Write a diagnostic line. Manifest diagnostics are prefixed with their plugin
identifier.

### `send_message(target, text)`

Send a private or channel message through the current session. Requires
`messages.write`.

```python
fabulor.send_message("#fabulor", "Hello")
```

## Information

### `nickcmp(first, second)`

Compare two nicknames using the current server's casemapping rules. Return `0`
when they are equivalent. Requires `session.read`.

### `strip(text, length=-1, flags=3)`

Remove IRC formatting from text. `length=-1` processes the complete string.
The default flags remove colour and text attributes.

### `get_info(name)`

Return a string describing the current context, or `None` when the value is not
available. Common names include:

| Name | Result |
| --- | --- |
| `nick` | Current nickname. |
| `id` | Stable connection identifier as a string (new bridge-capable builds; older builds expose it through `get_prefs('id')` and channel-list `id`). |
| `channel` | Current channel or dialogue name. |
| `server` | Connected server name. |
| `network` | Configured network name. |
| `host` | Current server hostname. |
| `configdir` | Fabulor profile directory. |
| `version` | Fabulor version string. |
| `away` | Away reason when marked away. |
| `topic` | Current channel topic. |
| `win_status` | Window state. |

Some names are meaningful only in a connected server or channel context.
`gtkwin_ptr` and `win_ptr` are compatibility values represented as pointer
strings; new add-ons should not depend on them. Requires `session.read`.

### `get_prefs(name)`

Read a Fabulor setting by its `/SET` name. The result is a string, integer, or
`None`. Boolean settings are returned as integers for compatibility. Requires
`preferences.read`.

```python
timestamps = fabulor.get_prefs("stamp_text")
```

### `get_user_count()`

Return the number of users in the current channel. Returns zero when no user
list is available. Requires `session.read`.

### `get_user_info()`

Return a dictionary with `nickname`, `channel`, `server_name`, and
`network_name`. Requires `session.read`.

## Lists

### `get_lists()`

Return the available list names: `channels`, `dcc`, `ignore`, `notify`, and
`users`. Requires `session.read`.

### `get_list(name)`

Return a list of objects whose attributes depend on `name`. Return `None` when
the list is unavailable in the current context. An unknown name raises
`KeyError`.

```python
for user in fabulor.get_list("users") or []:
    fabulor.prnt(f"{user.nick} {user.host}")
```

#### `channels`

| Attribute | Type |
| --- | --- |
| `channel`, `channelkey`, `chanmodes`, `chantypes` | string |
| `network`, `nickmodes`, `nickprefixes`, `server` | string |
| `context` | `Context` |
| `flags`, `id`, `lag`, `maxmodes`, `queue`, `type`, `users` | integer |

#### `dcc`

| Attribute | Type |
| --- | --- |
| `destfile`, `file`, `nick` | string |
| `address32`, `cps`, `port`, `pos`, `poshigh` | integer |
| `resume`, `resumehigh`, `size`, `sizehigh`, `status`, `type` | integer |

#### `ignore`

| Attribute | Type |
| --- | --- |
| `mask` | string |
| `flags` | integer |

#### `notify`

| Attribute | Type |
| --- | --- |
| `networks`, `nick` | string |
| `flags` | integer |
| `off`, `on`, `seen` | Unix timestamp |

#### `users`

| Attribute | Type |
| --- | --- |
| `account`, `host`, `nick`, `prefix`, `realname` | string |
| `away`, `selected` | integer |
| `lasttalk` | Unix timestamp |

Requires `session.read`.

## Hooks

Hook registration returns an opaque integer handle. Retain the handle when you
may need to remove the hook later.

### Callback arguments

Command and server callbacks receive:

```python
callback(word, word_eol, userdata)
```

Print callbacks receive the same arguments. Attribute-aware callbacks receive
an additional final `attrs` argument whose `time` member contains the IRC
server timestamp when available.

`word` splits the event into words. `word_eol[n]` contains the event text from
word `n` through the end. Both are ordinary zero-based Python lists even though
historical XChat documentation described one-based C arrays.

### `register_callback(event_name, callback, userdata=None)`

Register a callback through Fabulor's shared manifest callback registry. This
API is exported to trusted add-ons as well as the isolated manifest module, but
trusted scripts normally use the more specific native hook functions below.

Supported event names are `message`, `server`, `server:<name>`,
`print:<event>`, and `command:<name>`. The callback receives one dictionary
containing `event`, `source`, `words`, `word_eol`, `time`, and `userdata`. Use
`get_user_info()` to read the event-bound nickname, channel, server, and network
context.

Return `EAT_NONE` (or `None`) to continue normal processing,
`EAT_FABULOR` to stop Fabulor's normal handling, `EAT_PLUGIN` to stop later
plugins, or `EAT_ALL` to stop both. The corresponding numeric values in the
isolated manifest module are `0`, `1`, `2`, and `3`.

The event name is limited to 128 UTF-8 bytes. Duplicate registrations are
rejected and each manifest plugin may register at most 64 callbacks. The
required capability is selected from the event family: `events.message`,
`events.server`, `events.print`, or `events.command`.

### `hook_command(name, callback, userdata=None, priority=PRI_NORM, help=None)`

Register a slash command. `help` is displayed by `/HELP` when supplied.
Requires `events.command`.

```python
def hello(word, word_eol, userdata):
    fabulor.prnt("Hello")
    return fabulor.EAT_ALL


hello_hook = fabulor.hook_command("HELLO", hello, help="Usage: /HELLO")
```

### `hook_print(name, callback, userdata=None, priority=PRI_NORM)`

Register for a named Fabulor print event. Requires `events.print`.

### `hook_print_attrs(name, callback, userdata=None, priority=PRI_NORM)`

Register for a print event and receive timestamp attributes. The callback
signature is `callback(word, word_eol, userdata, attrs)`. Requires
`events.print`.

### `hook_server(name, callback, userdata=None, priority=PRI_NORM)`

Register for an IRC command such as `PRIVMSG`, `JOIN`, or `CAP`. Use `RAW LINE`
to observe all incoming IRC lines. Requires `events.server`.

### `hook_server_attrs(name, callback, userdata=None, priority=PRI_NORM)`

Register for an IRC command and receive timestamp attributes. The callback
signature is `callback(word, word_eol, userdata, attrs)`. Requires
`events.server`.

### `hook_timer(milliseconds, callback, userdata=None)`

Run a callback after the given interval. Return `True` to repeat the timer or
`False`/`None` to remove it. Requires `events.timer`.

```python
def tick(userdata):
    fabulor.prnt("tick")
    return True


timer_hook = fabulor.hook_timer(1000, tick)
```

### `hook_unload(callback, userdata=None)`

Register cleanup code called when the trusted add-on unloads. Its callback
receives only `userdata`. Requires `events.unload`.

### `unhook(handle)`

Remove a hook previously returned by a hook function. The hook's original
`userdata` is returned, or `None` when the handle is not owned by the current
add-on.

## Contexts

A context identifies one Fabulor server, channel, or dialogue tab. Functions
such as `command`, `prnt`, `get_info`, and `get_list` operate on the current
context. Capturing the correct context prevents an add-on from sending output
to a same-named channel on another network.

### `get_context()`

Return a `Context` for the current tab. Requires `session.read`.

### `find_context(server=None, channel=None)`

Find a matching context. Either argument may be omitted. Return `None` when no
matching tab exists. Requires `session.read`.

```python
context = fabulor.find_context(server="Libera.Chat", channel="#fabulor")
if context is not None:
    context.prnt("Hello from another context")
```

### `Context` methods

| Method | Purpose |
| --- | --- |
| `set()` | Make this context current; return whether it still exists. |
| `prnt(text)` | Print temporarily in this context. |
| `emit_print(name, *args, time=0)` | Emit an event in this context. |
| `command(text)` | Run a command in this context. |
| `get_info(name)` | Read context information. |
| `get_list(name)` | Read a context-specific list. |

The convenience methods restore the previous context after the operation. A
closed context is rejected rather than silently redirecting the operation.

## Plugin Preferences

Plugin preferences are stored under the current add-on's identity.

| Function | Result | Capability |
| --- | --- | --- |
| `set_pluginpref(name, value)` | Store a string or integer; return success. | `preferences.write` |
| `get_pluginpref(name)` | Return a string, integer, or `None`. | `preferences.read` |
| `del_pluginpref(name)` | Delete a value; return success. | `preferences.write` |
| `list_pluginpref()` | Return stored preference names. | `preferences.read` |

## Optional Native Control Panels

Bridge-capable builds expose `ui_available()` and `ui_open(schema, callback,
userdata=None)` through the **trusted full Python API**, not the isolated
manifest interface. Both the frontend and Python host must be updated. Feature
detection must use `getattr(fabulor, 'ui_available', lambda: False)()`; do not
infer support from an installed GTK DLL or a compatibility window pointer.

The facility uses Fabulor's existing GTK4 frontend. No PyGObject, Qt, Tkinter,
OpenAI dependency or public native-plugin vtable change is required. No window,
UI timer or native panel registry is created merely by loading an add-on or
querying availability; Python keeps only empty owner bookkeeping until use. The host's regular Preferences window is unchanged.

- `ui_available()` discovers protocol 1, requiring `ui.write` and `session.read`
  on capability-bound full-API paths.
- `ui_features()` returns an immutable set of additive features supported by both
  wrapper and frontend, with the same discovery permissions/thread checks. Use
  `getattr(fabulor, 'ui_features', lambda: ())()` for older wrappers. Currently
  `page_buttons` enables a page's optional `buttons` list of `{id, label}` objects.
  Unsupported page-button opens are rejected before creating a handle/window.
  Global-button schemas retain their existing behaviour.
- `ui_open(...)` additionally requires `events.command` and `commands.execute`.
  All panel operations must be on the main thread.
- The returned handle supports `present()`, `close()` and `update(values)`.
- `callback(panel, event, values, userdata)` receives a button ID or `closed`.
  Input values are typed booleans, integers or strings. Bind the callback through
  the add-on entry module, retaining loader ownership, as with native hooks.
- Controls are **drafts**. The bridge does not apply/save add-on settings.
  Explicit button callbacks implement those operations. Closing does not roll
  back changes an add-on already applied.
- `update()` accepts existing readonly `label`/`status` fields only, never
  overwriting editable drafts. A status update supplies `{value, state}`.
- User-close callbacks receive a handle already marked closed. Owner unload
  closes panels before hooks are removed. No callback values are routed to IRC.
  If the captured IRC context is gone, unsafe close-event delivery is skipped;
  stale handles are reclaimed on the next UI operation or owner unload.

Example schema (see `samples/plugins/simple-python-ui/simple-python-ui.py`):

```python
schema = {
    'title': 'Example settings',
    'context': 'A clearly identified target',
    'pages': [{'id': 'overview', 'title': 'Overview', 'fields': [
        {'id': 'health', 'type': 'status', 'label': 'State',
         'value': 'Waiting', 'state': 'waiting'},
        {'id': 'enabled', 'type': 'check', 'label': 'Enabled', 'value': False},
    ]}],
    'buttons': [{'id': 'apply', 'label': 'Apply temporarily'}],
}
```

Supported field types: `label`, `status`, `check`, `text`, `int` and `choice`.
All have `id`, `type`, `label` and `value`; `status` adds a state from `off`,
`waiting`, `running`, `error`; `int` needs inclusive integer `min`/`max`;
`choice` needs a `choices` list containing its string value. Identifiers are
unique across the whole panel, 1-64 ASCII letters/digits/underscores. Limits:
four panels per Python add-on, eight native panels globally, eight pages,
64 fields, six global custom buttons and (with `page_buttons`) six per page,
16 choices, integer bounds within +/-1,000,000,
and 32 KiB of decoded schema/event data. Schema string byte limits and text
input length are checked. Unknown fields/types and invalid events are rejected.
Page/global button IDs share the panel-wide identifier namespace with fields and
pages. All actions deliver the same complete editable snapshot, regardless of
which page contains the button. Readonly UPDATE cannot add/change buttons.
For a backwards-compatible example, see the demo's optional feature check.

The internal `ADDONUI` command transports bounded base64 GMarkup; it is not a
user-facing configuration file or general widget scripting language. There are
no arbitrary GTK pointers, markup, scripts, file paths, browser widgets or
command templates in schemas. Registered event hooks are dispatched directly;
no unknown-command/IRC fallback is used for button values. Python handles check
add-on ownership. Trusted add-ons remain trusted local code, not a sandbox;
raw native commands are not an isolation/security boundary between such scripts.
Do not put credentials in panel schemas or text inputs.

Development Windows/TLS hosts may advertise the additional `oper_login` feature.
Only when both wrapper and frontend support it, an owned panel handle offers
`oper_login()` and `oper_login_status()`. The first opens a separate host-owned
masked IRC OPER window; the second returns non-secret status. No password/name
arguments or secret results cross this API. The native window owns credential
input, optional purpose/profile/peer/oper-name-scoped Windows Credential Manager
storage and explicit login/forget confirmation. It sends no request merely by
opening. No automatic login/retry, no input-history/generic-hook secret exposure,
no password in schemas/events/settings. Existing full-API capability, owner and
main-thread checks apply; no isolated manifest API expansion. See the native
bridge document for TLS/bouncer, status, lifecycle and memory/trust limitations.

The opening native session/connection/channel is captured for dispatch. A panel
must not assume it is editing whichever tab later becomes active. Add-ons must
also validate their own policy snapshots and reconnect lifecycle before applying
changes. Availability/health lamps describe operation, not whether IRC content
is safe. The bridge itself does no network I/O, persistence or health polling.

Developer build and validation details: [Native add-on panel bridge](native-ui-bridge.md).

## Compatibility Notes

Existing scripts may use `import xchat` or `import hexchat`; both modules
re-export the trusted Fabulor API. New code should use `import fabulor`.

The old `zoitechat` module is intentionally unavailable. Scripts using it must
be migrated.

The historical XChat reference remains useful for its conceptual explanations,
but Fabulor uses Python 3, zero-based Python lists, current IRC features,
profile-root path containment, explicit capabilities, and isolated manifest
interpreters. Treat this document and the Fabulor source as authoritative.
