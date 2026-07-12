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

## Legacy Native Plugin Coexistence

Fabulor currently has two plugin families:

1. Legacy native plugin DLLs loaded by the original plugin loader.
2. C#, Python, and Tcl plugins loaded through the simple add-on and manifest plugin model.

Legacy native C plugins are first-party compatibility components. On Windows the native autoload list includes installed DLLs such as checksum, Exec, FiSHLiM, Sysinfo, update, and language/runtime bridge plugins from the installed plugin library directory. User-selected native DLLs under the profile `addons` directory remain a local-trust compatibility path, but they are not the recommended third-party extension model.

Manifest plugins do not introduce a C language target. The manifest schema intentionally remains limited to `csharp`, `python`, and `tcl` while the shared API, isolation model, and capability policy are being hardened. Existing native plugins should not be migrated into manifests just to make them appear in the new catalog; any migration should be a deliberate redesign of that plugin around the shared API.

Operational policy:

1. Keep bundled native plugin DLLs controlled by installer feature selection and the legacy `--no-plugins` safe-mode switch.
2. Keep third-party/user-authored extensions on C#, Python, or Tcl.
3. Treat native DLL add-ons as trusted local code with full process access, outside manifest dependency and capability policy.
4. Do not add manifest support for native C/C++ entrypoints until there is a separate native sandboxing, signing, and dependency-loading design.
5. If a first-party native plugin is replaced later, prefer a feature-specific migration plan rather than a blanket legacy-to-manifest conversion.

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
3. Tcl manifest plugins can be auto-loaded through the bundled Tcl runtime with a minimal `zoitechat::*` command surface when `FABULOR_ENABLE_MANIFEST_PLUGINS=1` is set. The normal Tcl runtime root is the installed `Runtime\Tcl` directory beside `fabulor.exe`; development-only runtime roots require `FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS=1`.
4. The shared host validates manifests, resolves dependencies, applies blacklist decisions, and queues callback dispatch on the main thread.
5. The managed C# contract assembly is scaffolded under `src\managed\Fabulor.PluginAbstractions` so plugin and host types are concrete.
6. C# manifests load through the `src\managed\Fabulor.PluginHost` bridge, which is staged into `Runtime\DotNet` by the installer build.
7. The installer now bundles a private `.NET` runtime root under `Runtime\DotNet`, including `host\fxr\` and `shared\Microsoft.NETCore.App\`. Normal loading uses only this executable-relative runtime and bridge root; environment, current-directory, source-tree, and machine-wide .NET roots require `FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS=1`.
8. Sample cross-language manifest plugins live under `samples\plugins\` and exercise the documented schema, dependency ordering, callback registration, and current user/session-info access.

Python simple add-ons and Python manifest plugins still share the embedded interpreter, but they no longer share the same load authority. Simple add-ons resolve only through the profile `addons` directory. Manifest entrypoints require a host-authenticated internal load request, resolve through enabled manifest plugin roots, and receive their manifest id and declared capabilities as per-plugin policy metadata. This is an attribution and path boundary, not an interpreter sandbox.

Tcl manifests use `LoadLibraryEx()` with the selected runtime DLL directory instead of modifying process `PATH`. The Tcl library path is configured per interpreter before `Tcl_Init`, rather than through process-global `TCL_LIBRARY`.

Legacy native plugins continue to use the original plugin loader and are deliberately outside the manifest dependency resolver. This avoids creating a false security boundary around native code that already runs in-process with full application privileges.

## Related Guides

1. [C# Plugin Guide](csharp-plugin-guide.md)
2. [Python Plugin Guide](python-plugin-guide.md)
3. [Tcl Plugin Guide](tcl-plugin-guide.md)
