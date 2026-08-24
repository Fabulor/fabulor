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

## Compatibility Notes

Existing scripts may use `import xchat` or `import hexchat`; both modules
re-export the trusted Fabulor API. New code should use `import fabulor`.

The old `zoitechat` module is intentionally unavailable. Scripts using it must
be migrated.

The historical XChat reference remains useful for its conceptual explanations,
but Fabulor uses Python 3, zero-based Python lists, current IRC features,
profile-root path containment, explicit capabilities, and isolated manifest
interpreters. Treat this document and the Fabulor source as authoritative.
