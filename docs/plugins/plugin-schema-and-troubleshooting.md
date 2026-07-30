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
.dll
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

The manifest host is disabled by default. To opt in, open **Preferences > Advanced**, enable **Enable manifest plugins (requires restart)**, review the trusted-code warning, and restart Fabulor. Manifest plugins run with your user account's operating-system privileges; capabilities constrain cooperative Fabulor API access but do not provide a process sandbox. `FABULOR_ENABLE_MANIFEST_PLUGINS=1` remains available as a developer/testing override. Safe mode (`--no-plugins`) takes precedence over both enable paths.

## Native Plugin Coexistence

Fabulor currently has two plugin families:

1. Native plugin DLLs loaded by the in-process plugin loader.
2. C#, Python, and Tcl plugins loaded through the simple add-on and manifest plugin model.

Native C plugins are first-party compatibility components. On Windows the native autoload list includes installed DLLs such as checksum, Exec, FiSHLiM, Sysinfo, update, and language/runtime bridge plugins from the installed plugin library directory. User-selected native DLLs under the profile `addons` directory remain a local-trust compatibility path, but they are not the recommended third-party extension model.

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
| `requires_api_version` | unsigned integer | Minimum Fabulor plugin API version needed by this plugin. Must be at least `1`. |
| `dependencies` | array of strings | Plugin ids that must load first. |
| `capabilities` | array of strings | Enforced permissions required by privileged host operations. |
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
  "requires_api_version": 1,
  "dependencies": [],
  "capabilities": ["events.message", "session.read"],
  "description": "Logs a local greeting and observes message events.",
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

The manifest host exposes `FabulorAPI` in `src/common/fabulor-plugin-host.h`.
Native C plugins use `src/common/fabulor-plugin.h`, export
`fabulor_plugin_init` and optionally `fabulor_plugin_deinit`, and call the
`fabulor_*` native ABI. The former product-prefixed native ABI is not exported
or accepted; native plugins built for it must be rebuilt against the Fabulor
header.

Manifests are strict JSON. The root must be one object containing exactly the documented fields. Duplicate or unknown fields, trailing content, trailing commas, invalid UTF-8 or Unicode escapes, control characters in strings, and values of the wrong JSON type are rejected. The manifest file must be a regular file between 1 byte and 64 KiB; symbolic-link manifests are rejected.

Discovery is confined to direct plugin folders under the approved roots: `Plugins` beside `fabulor.exe` for bundled plugins and `plugins` beneath the Fabulor profile directory for user plugins. The roots, plugin folders, and `plugin.json` files must not be symbolic links, junctions, or other Windows reparse points. A rejected plugin is reported and skipped without blocking unrelated sibling plugins.

Field limits are measured after JSON string decoding, in UTF-8 bytes:

| Field | Maximum |
| --- | --- |
| `id` | 128 bytes |
| `name` | 256 bytes |
| `version` | 64 bytes |
| `language` | 16 bytes |
| `entrypoint` | 1,024 bytes |
| `description` | 2,048 bytes |
| `author` | 256 bytes |
| `homepage` | 2,048 bytes |
| `dependencies` | 64 unique, non-empty strings; 128 bytes each |
| `capabilities` | 32 unique, non-empty strings; 64 bytes each |

## Current Enforcement Notes

The current runtime already enforces these checks:

1. Required manifest fields must be present and non-empty.
2. `language` must resolve to a supported loader.
3. `entrypoint` must name one readable regular file directly inside the plugin folder. Absolute paths, `..`, nested paths, symbolic links, and Windows reparse points are rejected. The extension must match the declared language: `.dll` for `csharp`, `.py` for `python`, or `.tcl` for `tcl`.
4. `requires_api_version` must not exceed the host API version.
5. Every declared dependency must be discoverable.
6. Duplicate plugin ids are rejected.
7. Dependency cycles are detected during load-order resolution.
8. Safe mode and blacklist decisions are logged as diagnostics.
9. An invalid manifest is skipped without blocking unrelated valid plugins.
10. Plugins that depend directly or transitively on an invalid, incompatible, disabled, or blacklisted plugin are also skipped with a diagnostic.

Manifest capabilities are deny-by-default and enforced across C#, Python, and Tcl. Unknown or duplicate capability names reject the manifest during validation. A plugin that calls an operation without declaring its capability is denied at runtime and receives a plugin-specific error or diagnostic.

Supported capabilities:

| Capability | Grants |
| --- | --- |
| `messages.write` | Send a message through the shared message API. |
| `session.read` | Read active session, user, context, and list information. |
| `ui.write` | Print or emit output into the client UI. |
| `commands.execute` | Execute a client command. |
| `commands.manage` | Add or remove user commands. |
| `preferences.read` | Read client or plugin preference data. |
| `preferences.write` | Set or delete plugin preference data. |
| `events.message` | Register incoming IRC `PRIVMSG` callbacks. Locally entered channel text is an outgoing `command:SAY` event unless the server echoes it back. |
| `events.server` | Register generic or named server callbacks. |
| `events.print` | Register print-event callbacks. |
| `events.command` | Register command callbacks. |
| `events.timer` | Register timer callbacks. |
| `events.unload` | Register unload callbacks. |

Logging through the language-specific manifest logger and pure text stripping do not require capabilities. Capabilities constrain cooperation with the Fabulor host; they are not an operating-system sandbox and cannot contain native code or a compromised language runtime.

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
2. Confirm the extension is supported: `.tcl`, `.py`, or a compiled C# `.dll` implementing `IFabulorPlugin`.
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

1. Simple add-ons are the intended user-facing path: `addons\<name>\<name>.tcl`, `addons\<name>\<name>.py`, or a compiled C# `addons\<name>\<name>.dll`.
2. Python manifest plugins can be auto-loaded through the embedded Python runtime when the confirmed profile preference or developer override is enabled. The Python runtime accepts manifest entrypoints only when the resolved `.py` path is under the bundled `Plugins\` root or the user profile `plugins\` root.
3. Tcl manifest plugins can be auto-loaded through the bundled Tcl runtime with a minimal `fabulor::*` command surface when the confirmed profile preference or developer override is enabled. The normal Tcl runtime root is the installed `Runtime\Tcl` directory beside `fabulor.exe`; development-only runtime roots require `FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS=1`.
4. The shared host validates manifests, resolves dependencies, applies blacklist decisions, and queues callback dispatch on the main thread.
5. The managed C# contract assembly is scaffolded under `src\managed\Fabulor.PluginAbstractions` so plugin and host types are concrete.
6. C# manifests load through the `src\managed\Fabulor.PluginHost` bridge, which is staged into `Runtime\DotNet` by the installer build.
7. The installer now bundles a private `.NET` runtime root under `Runtime\DotNet`, including `host\fxr\` and `shared\Microsoft.NETCore.App\`. Normal loading uses only this executable-relative runtime and bridge root; environment, current-directory, source-tree, and machine-wide .NET roots require `FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS=1`.
8. Sample cross-language simple add-ons and manifest plugins live under `samples\plugins\`. The simple trio exercises exact profile naming and initialization; the manifest trio exercises the documented schema, dependency ordering, callback registration, and current user/session-info access.
9. Shared C#/Tcl callbacks are bounded to 64 registrations per plugin and 256 per event. Cross-thread event delivery retains the registry safely, caps queued work, and is discarded before runtime teardown. Python applies matching per-plugin and event-name limits through its legacy hook-backed plugin object.

Python simple add-ons remain in the trusted legacy embedded interpreter. Each Python manifest plugin receives a separate Python 3.14 subinterpreter after a host-authenticated load request resolves its entrypoint through an enabled manifest root. CFFI is not imported into those subinterpreters; a small pure-Python API serialises allowed host operations back through the trusted main interpreter. This isolates Python globals, imports, module state, callbacks, and teardown, but it is not an operating-system sandbox.

Tcl manifests use `LoadLibraryEx()` with the selected runtime DLL directory instead of modifying process `PATH`. The Tcl library path is configured per interpreter before `Tcl_Init`, rather than through process-global `TCL_LIBRARY`.

Legacy native plugins continue to use the original plugin loader and are deliberately outside the manifest dependency resolver. This avoids creating a false security boundary around native code that already runs in-process with full application privileges.

## Related Guides

1. [C# Plugin Guide](csharp-plugin-guide.md)
2. [Python Plugin Guide](python-plugin-guide.md)
3. [Tcl Plugin Guide](tcl-plugin-guide.md)
