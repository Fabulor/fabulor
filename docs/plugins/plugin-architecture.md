# Plugin Architecture

This document describes the plugin architecture used by Fabulor. It focuses on the implemented extension model and the boundaries between legacy native plugins, simple add-ons, and advanced manifest plugins.

## High-level Architecture

Fabulor has two plugin families:

1. **Legacy/native in-process plugins**
   - Native DLLs use the C ABI in `src/common/fabulor-plugin.h`.
   - They export `fabulor_plugin_init` and optionally `fabulor_plugin_deinit`.
   - They are used mainly for first-party compatibility plugins such as Checksum, Exec, FiSHLiM, Sysinfo, and the Python bridge.
   - They run in-process with full application privileges.

2. **User-facing C#, Python, and Tcl add-ons/plugins**
   - This is the preferred extension model for third-party and user code.
   - It has two layouts:
     - **Simple add-ons** under `addons\`
     - **Advanced manifest plugins** under `plugins\`

The user-facing plugin model is **C#, Python, and Tcl**. Legacy native C plugins remain a first-party compatibility surface rather than a recommended manifest target.

## Simple Add-ons

Simple add-ons are the preferred model for personal scripts, aliases, and small trusted helpers.

Normal installed-mode layout:

```text
%APPDATA%\Fabulor\addons\<addon-name>\<addon-name>.<ext>
```

Profile paths in this document use the normal installed-mode location. Portable
and custom profiles use `addons`, `plugins`, and `plugin-blacklist.txt` beneath
their selected profile root instead.

Supported extensions:

| Extension | Runtime |
| --- | --- |
| `.py` | Python 3.14 script |
| `.tcl` | Tcl 8.6 script |
| `.dll` | C# .NET 8 assembly implementing `IFabulorPlugin` |

Examples:

```text
addons\greeter\greeter.py
addons\aliases\aliases.tcl
addons\helper\helper.dll
```

Important behaviour:

- The folder basename and file basename must match.
- Add-ons are scanned from direct child folders, alphabetically.
- Failure in one add-on is logged but does not stop others from loading.
- Simple add-ons are trusted local code and do not use `plugin.json`.
- Simple add-ons are not sandboxed.

Language-specific notes:

- **Python simple add-ons**
  - Use the `fabulor` module.
  - `xchat` and `hexchat` compatibility imports are retained for suitable older scripts.
  - Run in the shared legacy Python interpreter.
  - Must define `__module_name__`; `__module_version__` and
    `__module_description__` are optional.
- **Tcl simple add-ons**
  - Use the `fabulor::*` namespace.
  - Each simple Tcl add-on gets its own interpreter.
  - They can register commands directly.
- **C# simple add-ons**
  - Must target .NET 8.
  - Must implement `IFabulorPlugin`.
  - Fabulor does not compile C# source at startup.
  - The installed `Fabulor.PluginAbstractions.dll` supplies the contract.

The managed C# contract is intentionally small:

```csharp
public interface IFabulorPlugin
{
    void Init(FabulorContext context);
}
```

`FabulorContext` exposes only five operations: `Log`, `SendMessage`,
`GetUserCount`, `GetUserInfo`, and `RegisterCallback`.

## Manifest Plugins

Manifest plugins are the advanced model for packaged plugins that need:

- explicit plugin identity;
- versioning;
- API compatibility checks;
- dependencies;
- declared capabilities; and
- callback/event integration.

Layout:

```text
%APPDATA%\Fabulor\plugins\<plugin-id>\plugin.json
%APPDATA%\Fabulor\plugins\<plugin-id>\<entrypoint>
```

Example manifest:

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
  "description": "Logs a greeting and observes message events.",
  "author": "Fabulor",
  "homepage": "https://github.com/Fabulor/fabulor"
}
```

Supported manifest languages:

- `csharp`
- `python`
- `tcl`

Native C/C++ is deliberately **not** a manifest language.

Manifest plugin discovery roots are:

1. Bundled `Plugins\` under the installation root, beside `fabulor.exe`.
2. User profile `plugins\`.

Manifest plugins are disabled by default. They require either:

- enabling **Enable manifest plugins** in preferences and restarting; or
- the developer override:

```text
FABULOR_ENABLE_MANIFEST_PLUGINS=1
```

Safe mode overrides both enable paths:

```text
--no-plugins
```

## Manifest Validation and Loading

The manifest host is implemented around the shared types in `src/common/fabulor-plugin-host.h`.

The host API version is currently:

```c
#define FABULOR_PLUGIN_API_VERSION 2U
```

The manifest loader:

1. Discovers direct plugin folders.
2. Reads `plugin.json`.
3. Rejects invalid JSON, unknown fields, duplicate fields, oversized fields, symlinks/reparse points, bad entrypoints, duplicate IDs, unsupported languages, unknown capabilities, missing dependencies, incompatible API versions, and dependency cycles.
4. Resolves dependency load order.
5. Loads each plugin through the language-specific loader.
6. Records diagnostics and normally skips invalid individual manifests without
   blocking unrelated sibling plugins. A dependency cycle causes load-order
   resolution to fail and stops the entire manifest autoload batch.

Manifests must be regular files, entrypoints must be regular files directly inside the plugin folder, and entrypoint extensions must match the declared language:

| Language | Entrypoint extension |
| --- | --- |
| `csharp` | `.dll` |
| `python` | `.py` |
| `tcl` | `.tcl` |

## Capabilities

Manifest capabilities are deny-by-default for privileged host operations.

Supported capabilities:

| Capability | Grants |
| --- | --- |
| `messages.write` | Send a message through the shared message API. |
| `session.read` | Read the user count and user/session identity through `get_user_count` and `get_user_info`. |
| `events.message` | Register incoming IRC `PRIVMSG` callbacks. |
| `events.server` | Register server callbacks. |
| `events.print` | Register print-event callbacks. |
| `events.command` | Register command callbacks. |

Logging does not require a capability.

Capabilities are enforced across C#, Python, and Tcl, but they are **not an operating-system sandbox**. Plugins still run as local code with the user's account privileges.

## Shared Manifest API

The shared manifest surface is deliberately small. In C terms, `FabulorAPI` exposes:

- `log`
- `send_message`
- `get_user_count`
- `get_user_info`

C# and Tcl callback registration uses the native host's callback registry.
Python manifest callbacks use the Python bridge's capability-checked legacy
hook path, as described below.

In language-specific form, manifest plugins get roughly:

| Operation | C# | Python | Tcl |
| --- | --- | --- | --- |
| Log | `context.Log(...)` | `fabulor.log(...)` | `fabulor::log` |
| Send message | `context.SendMessage(...)` | `fabulor.send_message(...)` | `fabulor::send_message` |
| User count | `context.GetUserCount()` | `fabulor.get_user_count()` | `fabulor::get_user_count` |
| User/session info | `context.GetUserInfo()` | `fabulor.get_user_info()` | `fabulor::get_user_info` |
| Register callback | `context.RegisterCallback(...)` | `fabulor.register_callback(...)` | `fabulor::register_callback` |

The shared API intentionally excludes broader legacy operations such as arbitrary command execution, preference access, timers, direct UI printing, and native hook surfaces.

## Callback Architecture

Manifest plugins can register callbacks for event families such as:

```text
message
server
server:<name>
print:<event>
command:<name>
```

C# and Tcl use the native `FabulorCallbackRegistry` in
`src/common/fabulor-plugin-host.c`. Its event dispatch is integrated from
`src/common/plugin.c`, where Fabulor fires generic and specific event names.
For example, a server event may dispatch both:

```text
server
server:NOTICE
```

API version 2 adds consumption semantics for C# and Tcl callbacks:

- C# can return `FabulorEventResult.Consume`.
- Tcl can return `consume` or `1`.
- These plugins must declare `requires_api_version: 2` for consumption to take
  effect; version 1 callback results are ignored by the native registry.
- Command consumption prevents normal command handling after registered
  callbacks run. Message, server, and print callbacks on this registry remain
  observational and do not suppress core IRC state handling.

Python manifest callback registrations are serialised from
`plugins/python/_fabulor_manifest.py` to `ManifestPlugin._apply_operations()` in
`plugins/python/python.py`, then registered through
`_register_callback_for_plugin()` in `plugins/python/_fabulor.py`. This path
uses legacy native hooks rather than `FabulorCallbackRegistry`.

Python callbacks retain integer consumption semantics: `0` (or `None`) continues
processing, `1` stops normal Fabulor handling, `2` stops later plugins, and `3`
stops both. These results also apply to message, server, and print hooks; they
are not restricted to commands or gated on API version 2 by this bridge.

Callback limits differ by registration path:

- The native registry limits each plugin to 64 callbacks, each event to 256
  callbacks, event names to 128 UTF-8 bytes, and handler names to 256 bytes.
- The Python manifest bridge independently limits each plugin to 64 callbacks
  and event names to 128 UTF-8 bytes. It does not apply the native registry's
  256-callback-per-event limit.

## Runtime Architecture by Language

### C#

C# manifest plugins load through the managed bridge in:

```text
src/managed/Fabulor.PluginHost/NativeExports.cs
```

The native host loads the bundled private .NET runtime under:

```text
Runtime\DotNet
```

The bridge:

1. Loads the plugin assembly into a collectible `AssemblyLoadContext`.
2. Finds a concrete `IFabulorPlugin`.
3. Constructs it.
4. Passes a `FabulorContext`.
5. Routes callbacks back through the native callback registry.

### Python

There are two Python models:

- Simple add-ons run in the trusted legacy embedded interpreter.
- Manifest Python plugins run in separate Python 3.14 subinterpreters.

The manifest Python API is implemented by:

```text
plugins/python/_fabulor_manifest.py
```

It installs a restricted `fabulor` module containing only the manifest API. Host operations are serialised back to the trusted host. This isolates Python globals, imports, module state, callbacks, and teardown, but it is not an operating-system sandbox.

### Tcl

Tcl uses bundled Tcl 8.6.

Simple Tcl add-ons:

- receive isolated interpreters;
- can register commands; and
- get a broader trusted helper surface.

Manifest Tcl plugins:

- receive the restricted shared manifest API;
- can register host callbacks; and
- are capability-checked.

The Tcl runtime is loaded with `LoadLibraryEx()` from the selected bundled runtime directory rather than by modifying global `PATH`.

## Native Plugin Compatibility Layer

Native plugins use `src/common/fabulor-plugin.h`, which exposes a broader XChat/HexChat-style ABI:

- command hooks;
- server hooks;
- print hooks;
- timers;
- file-descriptor hooks;
- contexts;
- lists;
- preferences;
- plugin GUI entries;
- emit/strip helpers.

Native plugins are loaded by the legacy loader in `src/common/plugin.c`, which looks for:

```text
fabulor_plugin_init
fabulor_plugin_deinit
```

On Windows, first-party bundled DLLs are auto-loaded from the plugin library directory. User native DLLs under profile `addons` remain a local-trust compatibility path, but new third-party extensions should use C#, Python, or Tcl instead.

## Safety Model

Fabulor's plugin model is a **trust-boundary model**, not a sandbox model.

Key points:

- Simple add-ons are trusted local code.
- Native DLLs have full in-process access.
- Manifest capabilities restrict cooperative access to Fabulor APIs, but do not stop filesystem, process, or operating-system access by plugin code.
- Manifest plugins are disabled by default.
- `--no-plugins` starts without automatic plugin loading.
- Individual manifest plugin IDs can be blacklisted in:

```text
%APPDATA%\Fabulor\plugin-blacklist.txt
```

## Summary

Fabulor uses a layered plugin architecture:

- A legacy native C ABI for installed and compatibility plugins.
- A recommended modern C#/Python/Tcl add-on model.
- A simple trusted `addons\<name>\<name>.<ext>` convention for personal add-ons.
- An advanced `plugins\<id>\plugin.json` manifest model with dependency resolution, API versioning, capability checks, and cross-language callbacks.
- A compact host API exposed through C#, isolated Python manifest
  subinterpreters, and Tcl interpreters, with native-registry callbacks for C#
  and Tcl and Python-bridge callbacks through legacy native hooks.
