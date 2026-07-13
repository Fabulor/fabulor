# Python Plugin Guide

## Scope

This guide covers Python scripting for Fabulor and links to shared schema and troubleshooting rules.

Read shared rules first:

1. [Simple Add-ons](simple-addons.md)
2. [Plugin Schema, Compatibility, and Troubleshooting](plugin-schema-and-troubleshooting.md)

## Simple Python Add-on

Preferred layout:

```text
%APPDATA%\Fabulor\addons\greeter\greeter.py
```

Optional metadata:

```python
# Fabulor-Name: Greeter
# Fabulor-Version: 1.0
# Fabulor-Description: Small Python helper
```

Minimal script:

```python
import zoitechat


def init():
    zoitechat.log("Python add-on initialised")
```

## Advanced Python plugin.json

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

## Minimal Python Plugin

```python
import zoitechat

_reported_first_message = False


def on_message(event):
    global _reported_first_message
    if _reported_first_message:
        return None

    _reported_first_message = True
    user = zoitechat.get_user_info()
    location = user.get("channel") or "the active session"
    zoitechat.log(f"Python sample observed its first message event in {location}.")
    return None


def init():
    user = zoitechat.get_user_info()
    nickname = user.get("nickname") or "unknown"
    zoitechat.log(f"Hello, {nickname}. Python sample ready.")
    zoitechat.register_callback("message", on_message)
```

## Notes

1. Keep callback handlers lightweight to avoid blocking the main thread.
2. Use the simple `addons\<name>\<name>.py` layout for personal scripts and helpers. Relative Python load requests resolve under the profile `addons` directory.
3. Declare every host operation the plugin uses. Manifest Python API calls are denied when the corresponding capability is absent; simple add-ons remain outside manifest capability policy. Manifest Python entrypoints must resolve under the bundled `Plugins\` root or the user profile `plugins\` root.
4. `zoitechat.log(...)`, `zoitechat.send_message(...)`, `zoitechat.get_user_count()`, `zoitechat.get_user_info()`, and `zoitechat.register_callback(...)` are available in the embedded host.
5. `zoitechat.register_callback(...)` currently supports `message`, `server`, `server:<name>`, `print:<event>`, and `command:<name>`.
6. Callback payloads now include richer context such as `source`, `time`, `channel`, `network`, `nick`, `server`, `word1`-`word4`, and `word_eol1`-`word_eol2` where the underlying event provides them.
7. The host validates `plugin.json`, resolves declared dependencies, and dispatches callbacks on the main thread before language-specific execution.
8. `zoitechat.get_user_info()` returns a dictionary with `nickname`, `channel`, `server_name`, and `network_name`.
9. Manifest Python entrypoints use a host-authenticated internal load path. The loader attaches the manifest id and declared capabilities to the Python plugin object; ordinary `/LOAD` and `/PY LOAD` requests remain confined to the profile `addons` directory and cannot opt themselves into manifest roots. Ordinary unload/reload commands cannot mutate manifest-host-owned Python plugins.
10. Python manifest plugins still share one embedded interpreter. The manifest boundary provides path and policy attribution, not process or interpreter sandboxing.
11. A maintained sample manifest Python plugin lives under `samples\plugins\example.python.greeter\`.
12. Callback event names are limited to 128 UTF-8 bytes and each manifest plugin to 64 hooks. Registering the same Python callback for the same named event twice is rejected. Python callbacks remain owned and cleaned up by the legacy hook-backed plugin object.
