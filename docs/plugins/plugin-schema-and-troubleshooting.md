# Plugin Schema, Compatibility, and Troubleshooting

## Scope

This document covers shared plugin documentation for Fabulor:

1. Plugin folder layout
2. `plugin.json` schema
3. Compatibility rules
4. Safe mode behaviour
5. Troubleshooting workflow

Current project path used in examples:

`C:\zoitechat-master`

Planned workspace rename:

`C:\fabulor-master`

## Plugin Folder Layout

Each plugin should live in its own folder:

```text
plugins/
  <plugin-id>/
    plugin.json
    <entrypoint>
```

Examples:

```text
plugins/example.csharp.greeter/plugin.json
plugins/example.csharp.greeter/GreeterPlugin.dll

plugins/example.python.greeter/plugin.json
plugins/example.python.greeter/plugin.py

plugins/example.tcl.greeter/plugin.json
plugins/example.tcl.greeter/plugin.tcl
```

## plugin.json Schema

Required and supported fields:

| Field | Type | Description |
| --- | --- | --- |
| `id` | string | Unique plugin identifier. Use a stable namespaced id. |
| `name` | string | Human-readable plugin name. |
| `version` | string | Plugin version. Semantic versioning is recommended. |
| `language` | string | One of `csharp`, `python`, or `tcl`. |
| `entrypoint` | string | Entry file name inside the plugin folder. |
| `requires_api_version` | string or number | Minimum ZoiteChat plugin API version needed by this plugin. |
| `dependencies` | array of strings | Plugin ids that must load first. |
| `capabilities` | array of strings | Declared capabilities for policy checks and diagnostics. |
| `description` | string | Short summary of plugin behaviour. |
| `author` | string | Plugin author or organisation. |
| `homepage` | string | URL for source, docs, or support. |

Minimal example:

```json
{
  "id": "example.python.greeter",
  "name": "Greeter",
  "version": "1.0.0",
  "language": "python",
  "entrypoint": "plugin.py",
  "requires_api_version": "1",
  "dependencies": [],
  "capabilities": ["messages.write", "events.message"],
  "description": "Sends a greeting when initialised.",
  "author": "ZoiteChat Team",
  "homepage": "https://zoitechat.org"
}
```

## Compatibility Rules

Use these rules to keep plugins loadable across upgrades:

1. Keep `id` stable for the plugin lifetime.
2. Increase `version` for every release.
3. Set `requires_api_version` to the minimum API level the plugin needs.
4. Prefer additive API changes to preserve backward compatibility.
5. Declare all load-order requirements in `dependencies`.
6. Ensure `entrypoint` exists and is shipped with the plugin.
7. Keep `capabilities` accurate and up to date.

## Safe Mode

Safe mode is intended to disable third-party plugins so ZoiteChat can start with core behaviour only.

Operational expectations:

1. Third-party plugin discovery is skipped while safe mode is enabled.
2. Core startup continues even when plugins are disabled.
3. Logs clearly state that safe mode is active.
4. Users can restart in normal mode to re-enable plugins.

## Plugin Troubleshooting

Use this sequence when a plugin does not load or behaves incorrectly:

1. Confirm folder layout under `plugins/<plugin-id>/`.
2. Validate `plugin.json` and check `language` and `entrypoint`.
3. Confirm the entrypoint file exists and is readable.
4. Check `requires_api_version` against host API version.
5. Verify every declared dependency exists.
6. Review logs for validation failures, dependency cycles, and callback errors.
7. Start ZoiteChat in safe mode to isolate plugin-related faults.
8. Re-enable plugins one at a time to identify the failing plugin.

## Related Guides

1. [C# Plugin Guide](csharp-plugin-guide.md)
2. [Python Plugin Guide](python-plugin-guide.md)
3. [Tcl Plugin Guide](tcl-plugin-guide.md)
