# Plugin Schema, Compatibility, and Troubleshooting

## Scope

This document covers shared plugin documentation for Fabulor:

1. Plugin folder layout
2. `plugin.json` schema
3. Compatibility rules
4. Safe mode behaviour
5. Troubleshooting workflow

Current project path used in examples:

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

The current runtime discovers manifest plugins from these roots when they exist:

1. Bundled `Plugins\` under the installed plugin library directory.
2. User `plugins\` under the profile/config directory.

## plugin.json Schema

Required and supported fields:

| Field | Type | Description |
| --- | --- | --- |
| `id` | string | Unique plugin identifier. Use a stable namespaced id. |
| `name` | string | Human-readable plugin name. |
| `version` | string | Plugin version. Semantic versioning is recommended. |
| `language` | string | One of `csharp`, `python`, or `tcl`. |
| `entrypoint` | string | Entry file name inside the plugin folder. |
| `requires_api_version` | string or number | Minimum Fabulor plugin API version needed by this plugin. |
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
  "author": "Fabulor",
  "homepage": "https://github.com/Fabulor/fabulor"
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

The current host scaffold exposes the public contract in `src/common/fabulor-plugin-host.h`. It keeps `ZoiteChatAPI` as a compatibility alias for `FabulorAPI` while the plugin-facing host names are modernised.

## Safe Mode

Safe mode is intended to disable third-party plugins so Fabulor can start with core behaviour only.

At present, this is controlled by the existing `--no-plugins` startup switch.

Operational expectations:

1. Third-party plugin discovery is skipped while safe mode is enabled.
2. Core startup continues even when plugins are disabled.
3. Logs clearly state that safe mode is active.
4. Users can restart in normal mode to re-enable plugins.
5. Blacklisted plugin ids stay disabled even when safe mode is off.

Blacklisted plugin ids can be listed one per line in `plugin-blacklist.txt` under the user config directory. Empty lines and lines starting with `#` are ignored.

## Plugin Troubleshooting

Use this sequence when a plugin does not load or behaves incorrectly:

1. Confirm folder layout under `plugins/<plugin-id>/`.
2. Validate `plugin.json` and check `language` and `entrypoint`.
3. Confirm the entrypoint file exists and is readable.
4. Check `requires_api_version` against host API version.
5. Verify every declared dependency exists.
6. Review logs for validation failures, dependency cycles, callback dispatch failures, and blacklist decisions.
7. Start Fabulor in safe mode to isolate plugin-related faults.
8. Re-enable plugins one at a time to identify the failing plugin.

## Current Runtime Notes

The manifest host is staged in progressively:

1. Python manifest plugins can be auto-loaded through the existing embedded Python runtime.
2. Tcl manifest plugins can be auto-loaded through the bundled Tcl runtime with a minimal `zoitechat::*` command surface.
3. The shared host validates manifests, resolves dependencies, applies blacklist decisions, and queues callback dispatch on the main thread.
4. The managed C# contract assembly is scaffolded under `src\managed\Fabulor.PluginAbstractions` so plugin and host types are concrete.
5. C# manifests load through the `src\managed\Fabulor.PluginHost` bridge, which is staged into `Runtime\DotNet` by the installer build.
6. The installer now bundles a private `.NET` runtime root under `Runtime\DotNet`, including `host\fxr\` and `shared\Microsoft.NETCore.App\`.

## Related Guides

1. [C# Plugin Guide](csharp-plugin-guide.md)
2. [Python Plugin Guide](python-plugin-guide.md)
3. [Tcl Plugin Guide](tcl-plugin-guide.md)
