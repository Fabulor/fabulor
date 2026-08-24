# Python Plugin Guide

## Scope

Fabulor provides two Python extension models:

1. **Simple add-ons** are trusted local scripts in the shared Python interpreter.
   They provide the complete compatibility API documented in the
   [Python API Reference](python-api-reference.md).
2. **Manifest plugins** are capability-declared plugins in isolated Python 3.14
   subinterpreters. They intentionally receive a smaller API.

The supported module name is `fabulor`. The inherited `zoitechat` module is
retired. The `xchat` and `hexchat` compatibility modules remain available to
trusted simple add-ons and forward to the same Fabulor API.

Read these shared rules first:

1. [Simple Add-ons](simple-addons.md)
2. [Plugin Schema, Compatibility, and Troubleshooting](plugin-schema-and-troubleshooting.md)

## Simple Python Add-on

Preferred layout:

```text
%APPDATA%\Fabulor\addons\greeter\greeter.py
```

The folder and file basenames must match. A trusted Python add-on must also set
`__module_name__`. Version and description are optional:

```python
__module_name__ = "Greeter"
__module_version__ = "1.0.0"
__module_description__ = "Small Python helper"

import fabulor


def init():
    user = fabulor.get_user_info()
    nickname = user.get("nickname") or "unknown"
    fabulor.prnt(f"Hello, {nickname}.")


init()
```

Simple add-ons execute when loaded. `init()` is a useful convention, but the
trusted loader does not call it automatically. Call it from the script as shown
above. Hooks and UI registrations are released when the script is unloaded.
Plugin preferences persist until the add-on explicitly deletes them.

### Loading and inspecting scripts

Fabulor auto-loads matching add-on folders at startup. The following commands
operate on trusted Python add-ons:

```text
/PY LOAD greeter
/PY UNLOAD greeter.py
/PY RELOAD greeter.py
/PY LIST
/PY ABOUT
```

`/LOAD`, `/UNLOAD`, and `/RELOAD` also recognise `.py` files. Relative loads
resolve beneath `%APPDATA%\Fabulor\addons`; ordinary scripts cannot select files
outside that trust boundary.

`/PY EXEC <statement>` and `/PY CONSOLE` provide interactive development tools.
Do not place secrets in commands because command history and diagnostics may
retain the entered text.

## Advanced Manifest Plugin

Manifest plugins belong under one of the enabled manifest roots and require a
`plugin.json` file:

```json
{
  "id": "example.python.greeter",
  "name": "Python Greeter",
  "version": "1.0.0",
  "language": "python",
  "entrypoint": "plugin.py",
  "requires_api_version": 1,
  "dependencies": [],
  "capabilities": ["events.message", "session.read"],
  "description": "Minimal Python event-observer plugin.",
  "author": "Fabulor",
  "homepage": "https://github.com/Fabulor/fabulor"
}
```

Minimal entrypoint:

```python
import fabulor

_reported_first_message = False


def on_message(event):
    global _reported_first_message
    if _reported_first_message:
        return 0

    _reported_first_message = True
    user = fabulor.get_user_info()
    location = user.get("channel") or "the active session"
    fabulor.log(f"First incoming message observed in {location}.")
    return 0


def init():
    user = fabulor.get_user_info()
    nickname = user.get("nickname") or "unknown"
    fabulor.log(f"Hello, {nickname}. Python sample ready.")
    fabulor.register_callback("message", on_message)
```

The isolated host calls `init()` after loading and `deinit()` before shutdown
when those functions exist.

### Manifest API

The isolated `fabulor` module exposes only:

| Function | Required capability | Purpose |
| --- | --- | --- |
| `log(text)` | none | Write a prefixed diagnostic line. |
| `send_message(target, text)` | `messages.write` | Send a message through the event-bound session. |
| `get_user_count()` | `session.read` | Return the current channel user count. |
| `get_user_info()` | `session.read` | Return nickname, channel, server, and network context. |
| `register_callback(event, callback, userdata=None)` | matching `events.*` | Register a bounded host callback. |

Supported callback names are `message`, `server`, `server:<name>`,
`print:<event>`, and `command:<name>`. Callback event names are limited to 128
UTF-8 bytes, each plugin may register at most 64 callbacks, and duplicate
registrations are rejected.

Callbacks receive a dictionary containing `event`, `source`, `words`,
`word_eol`, `time`, and `userdata`. Call `get_user_info()` inside the callback
when you need its event-bound nickname, channel, server, or network context.

Return `0` (or `None`) to continue normal processing, `1` to stop Fabulor's
normal handling, `2` to stop later plugins, or `3` to stop both normal handling
and later plugins.

## Choosing a Model

Use a simple add-on when you need the established scripting surface, including
native hooks, contexts, lists, preferences, print events, or compatibility with
an existing XChat or HexChat script.

Use a manifest plugin when you need explicit dependency/version metadata,
declared host capabilities, isolated Python globals and imports, or packaged
deployment. Interpreter isolation is not an operating-system sandbox: Python
code can still use the standard library and operating-system facilities
available to the Fabulor process.

## Performance and Safety

1. Keep callbacks short. They run as part of Fabulor's event processing.
2. Do not perform slow network, process, or filesystem work synchronously in a
   callback.
3. Use contexts deliberately when an add-on works across multiple networks.
4. Treat all simple add-ons as trusted local code.
5. Declare every host operation used by a manifest plugin. Missing capabilities
   are denied at runtime.
6. Manifest entrypoints are limited to 1 MiB and host messages are bounded.
7. Ordinary unload and reload commands cannot mutate manifest-host-owned
   plugins.

## Samples

Maintained samples live under:

```text
samples\plugins\simple-python-greeter\
samples\plugins\example.python.greeter\
```

The first demonstrates the trusted full Python interface. The second
demonstrates the isolated manifest interface.

## Historical Source

The [original XChat Python reference](https://xchat.org/docs/xchatpython.html),
written by Gustavo Niemeyer, preserved valuable explanations of contexts,
hooks, word lists, and event-consumption values. This Fabulor documentation is
an independently written and source-verified successor. It does not imply that
all historical XChat behaviour remains supported.
