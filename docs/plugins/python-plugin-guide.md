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
  "requires_api_version": "1",
  "dependencies": [],
  "capabilities": ["messages.write", "events.message", "session.read"],
  "description": "Minimal Python greeting plugin.",
  "author": "Fabulor",
  "homepage": "https://github.com/Fabulor/fabulor"
}
```

## Minimal Python Plugin

```python
import zoitechat


def on_message(event):
    # Keep callback handlers resilient and non-blocking.
    return None


def init():
    user = zoitechat.get_user_info()
    target = user.get("channel") or "#fabulor"
    zoitechat.log("Python plugin initialised")
    zoitechat.send_message(target, "Hello from Python plugin")
    zoitechat.register_callback("message", on_message)
```

## Notes

1. Keep callback handlers lightweight to avoid blocking the main thread.
2. Use the simple `addons\<name>\<name>.py` layout for personal scripts and helpers. Relative Python load requests resolve under the profile `addons` directory.
3. Keep manifest capabilities aligned with actual plugin behaviour when using the advanced manifest path. Manifest Python entrypoints must resolve under the bundled `Plugins\` root or the user profile `plugins\` root.
4. `zoitechat.log(...)`, `zoitechat.send_message(...)`, `zoitechat.get_user_count()`, `zoitechat.get_user_info()`, and `zoitechat.register_callback(...)` are available in the embedded host.
5. `zoitechat.register_callback(...)` currently supports `message`, `server`, `server:<name>`, `print:<event>`, and `command:<name>`.
6. Callback payloads now include richer context such as `source`, `time`, `channel`, `network`, `nick`, `server`, `word1`-`word4`, and `word_eol1`-`word_eol2` where the underlying event provides them.
7. The host validates `plugin.json`, resolves declared dependencies, and dispatches callbacks on the main thread before language-specific execution.
8. `zoitechat.get_user_info()` returns a dictionary with `nickname`, `channel`, `server_name`, and `network_name`.
9. Manifest Python entrypoints use a host-authenticated internal load path. The loader attaches the manifest id and declared capabilities to the Python plugin object; ordinary `/LOAD` and `/PY LOAD` requests remain confined to the profile `addons` directory and cannot opt themselves into manifest roots. Ordinary unload/reload commands cannot mutate manifest-host-owned Python plugins.
10. Python manifest plugins still share one embedded interpreter. The manifest boundary provides path and policy attribution, not process or interpreter sandboxing.
9. A maintained sample manifest Python plugin lives under `samples\plugins\example.python.greeter\`.
