# Python Plugin Guide

## Scope

This guide covers the minimal structure for a Fabulor Python plugin and links to shared schema and troubleshooting rules.

Read shared rules first:

1. [Plugin Schema, Compatibility, and Troubleshooting](plugin-schema-and-troubleshooting.md)

## Python plugin.json

```json
{
  "id": "example.python.greeter",
  "name": "Python Greeter",
  "version": "1.0.0",
  "language": "python",
  "entrypoint": "plugin.py",
  "requires_api_version": "1",
  "dependencies": [],
  "capabilities": ["messages.write", "events.message"],
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
    zoitechat.log("Python plugin initialised")
    zoitechat.send_message("#zoitechat", "Hello from Python plugin")
    zoitechat.register_callback("message", on_message)
```

## Notes

1. Keep callback handlers lightweight to avoid blocking the main thread.
2. Keep manifest capabilities aligned with actual plugin behaviour.
3. `zoitechat.log(...)`, `zoitechat.send_message(...)`, `zoitechat.get_user_count()`, and `zoitechat.register_callback(...)` are available in the embedded host.
4. `zoitechat.register_callback(...)` currently supports `message`, `server`, `server:<name>`, `print:<event>`, and `command:<name>`.
5. The host validates `plugin.json`, resolves declared dependencies, and dispatches callbacks on the main thread before language-specific execution.
