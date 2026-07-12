# Plugin Schema, Compatibility, and Troubleshooting

## Scope

This document covers shared plugin documentation for Fabulor:

1. Simple add-on folder layout
2. Advanced manifest plugin folder layout
3. `plugin.json` schema
4. Compatibility rules
5. Safe mode behaviour
6. Troubleshooting workflow

Current project path used in examples:

`C:\fabulor-master`

## Simple Add-on Folder Layout

For personal scripts, aliases, and small helper tools, prefer the simple add-on layout:

```text
addons/
  <addon-name>/
    <addon-name>.tcl
```

Supported simple add-on extensions:

```text
.tcl
.py
.cs
```

The extension selects the runtime. Metadata can be supplied as optional `Fabulor-*` comments at the top of the script.

See [Simple Add-ons](simple-addons.md) for the full convention.

## Manifest Plugin Folder Layout

Advanced manifest plugins should live in their own folder:

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

For repository examples and validation assets, see `samples\plugins\`. Those samples are intentionally outside the runtime discovery roots used by the live application.

The manifest host is currently disabled by default while the native API path is being hardened. Enable it only for development with `FABULOR_ENABLE_MANIFEST_PLUGINS=1`.

## plugin.json Schema

Required and supported fields:

| Field | Type | Description |
| --- | --- | --- |
| `id` | string | Unique plugin identifier. Use a stable namespaced id. |
| `name` | string | Human-readable plugin name. |
| `version` | string | Plugin version. Semantic versioning is recommended. |
| `language` | string | One of `csharp`, `python`, or `tcl`. |
| `entrypoint` | string | Relative entry file path inside the plugin folder. |
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

## Current Enforcement Notes

The current runtime already enforces these checks:

1. Required manifest fields must be present and non-empty.
2. `language` must resolve to a supported loader.
3. `entrypoint` must exist relative to the plugin folder.
4. `requires_api_version` must not exceed the host API version.
5. Every declared dependency must be discoverable.
6. Duplicate plugin ids are rejected.
7. Dependency cycles are detected during load-order resolution.
8. Safe mode and blacklist decisions are logged as diagnostics.

The current runtime records but does not yet actively enforce `capabilities`. For now they are best treated as accurate declarative metadata for documentation, diagnostics, and future policy work.

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

1. For simple add-ons, confirm the folder and file names match: `addons\<name>\<name>.<ext>`.
2. Confirm the extension is supported: `.tcl`, `.py`, or `.cs`.
3. For manifest plugins, confirm folder layout under `plugins/<plugin-id>/`.
4. Validate `plugin.json` and check `language` and `entrypoint`.
5. Confirm the entrypoint file exists and is readable.
6. Check `requires_api_version` against host API version.
7. Verify every declared dependency exists.
8. Review logs for validation failures, dependency cycles, callback dispatch failures, and blacklist decisions.
9. Start Fabulor in safe mode to isolate plugin-related faults.
10. Re-enable plugins one at a time to identify the failing plugin.

## Current Runtime Notes

The manifest host is staged in progressively:

1. Simple add-ons are the intended user-facing scripting path: `addons\<name>\<name>.tcl`, `addons\<name>\<name>.py`, or `addons\<name>\<name>.cs`.
2. Python manifest plugins can be auto-loaded through the existing embedded Python runtime when `FABULOR_ENABLE_MANIFEST_PLUGINS=1` is set. The Python runtime accepts manifest entrypoints only when the resolved `.py` path is under the bundled `Plugins\` root or the user profile `plugins\` root.
3. Tcl manifest plugins can be auto-loaded through the bundled Tcl runtime with a minimal `zoitechat::*` command surface when `FABULOR_ENABLE_MANIFEST_PLUGINS=1` is set.
4. The shared host validates manifests, resolves dependencies, applies blacklist decisions, and queues callback dispatch on the main thread.
5. The managed C# contract assembly is scaffolded under `src\managed\Fabulor.PluginAbstractions` so plugin and host types are concrete.
6. C# manifests load through the `src\managed\Fabulor.PluginHost` bridge, which is staged into `Runtime\DotNet` by the installer build.
7. The installer now bundles a private `.NET` runtime root under `Runtime\DotNet`, including `host\fxr\` and `shared\Microsoft.NETCore.App\`.
8. Sample cross-language manifest plugins live under `samples\plugins\` and exercise the documented schema, dependency ordering, callback registration, and current user/session-info access.

Python simple add-ons and Python manifest plugins still share the legacy embedded interpreter. The current boundary is path-based: simple add-ons resolve through the profile `addons` directory, while manifest entrypoints resolve through manifest plugin roots after the manifest host is explicitly enabled.

## Related Guides

1. [C# Plugin Guide](csharp-plugin-guide.md)
2. [Python Plugin Guide](python-plugin-guide.md)
3. [Tcl Plugin Guide](tcl-plugin-guide.md)
