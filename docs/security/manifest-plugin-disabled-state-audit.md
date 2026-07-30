# Manifest Plugin Security Audit

Date: 2026-07-11
Branch: agent/log-scroll-installer-fixes
PR context: https://github.com/Fabulor/fabulor/pull/7

## Pass 1 Scope

This is pass 1 of the plugin loader boundary review from `To-Do.md`. It reviews whether the manifest-driven C#/Python/Tcl plugin host is inert unless explicitly enabled.

The audit covers:

- Manifest enable gate and startup call sites.
- Config, command-line, installer, and environment paths that could enable the manifest host.
- Disabled-startup interaction with user-controlled `plugins` and `addons` folders.
- Runtime and DLL search path mutations in manifest loader code.

## Result

Status: pass for the manifest host disabled boundary, with one legacy-plugin caveat.

At the time of this initial disabled-state audit, the manifest host was only reachable when `FABULOR_ENABLE_MANIFEST_PLUGINS=1` was present in the process environment and the normal plugin autoload path was not skipped by `--no-plugins`. The later confirmed profile preference is recorded in the rollout follow-up below.

No config setting, command-line flag, installer property, installer shortcut, or bootstrapper code path found in this pass sets `FABULOR_ENABLE_MANIFEST_PLUGINS` or otherwise enables the manifest host.

When the manifest host is disabled, the reviewed startup path does not read `plugin-blacklist.txt`, does not discover bundled or user manifest roots, does not parse `plugin.json`, does not create the manifest callback registry, and does not load the Python, Tcl, or C# manifest runtimes.

## Enable Gate

The manifest gate is implemented in:

- `src/common/plugin.c:158` - `fabulor_manifest_plugins_enabled()`
- `src/common/plugin.c:160` - reads `FABULOR_ENABLE_MANIFEST_PLUGINS`
- `src/common/plugin.c:899` - checks the gate during `plugin_auto_load()`
- `src/common/plugin.c:901` - calls `fabulor_plugin_host_autoload(sess)` only when the gate returns true
- `src/common/plugin.c:375` - callback dispatch also returns before using the registry unless the gate is enabled

The gate accepts only a non-empty environment value that compares equal to `1`.

## Startup Boundary

Normal startup calls plugin autoload only when plugins are not globally skipped:

- `src/common/zoitechat.c:97` - `arg_skip_plugins` defaults to false
- `src/common/zoitechat.c:564` - `plugin_auto_load(sess)` is called only when `!arg_skip_plugins`
- `src/fe-gtk/fe-gtk.c:155` - GTK frontend exposes `--no-plugins`
- `src/fe-text/fe-text.c:469` - text frontend exposes `--no-plugins`

Inside manifest autoload, the user-controlled manifest root is touched only after the enable gate has already passed:

- `src/common/plugin.c:462` - `fabulor_plugin_host_autoload(session *sess)`
- `src/common/plugin.c:481` - loads `plugin-blacklist.txt`
- `src/common/plugin.c:483` - builds bundled `Plugins` root
- `src/common/plugin.c:484` - builds user `plugins` root from `get_xdir()`
- `src/common/plugin.c:487` and `src/common/plugin.c:494` - discovers manifest roots
- `src/common/plugin.c:500` - creates callback registry after discovery

The manifest parsing and loading implementation is in `src/common/fabulor-plugin-host.c`, but those routines are not called during startup unless `fabulor_plugin_host_autoload()` is reached.

## Installer and Defaults

Repository search found `FABULOR_ENABLE_MANIFEST_PLUGINS` only in `src/common/plugin.c` and documentation. The WiX installer and bootstrapper do not set it.

The installer includes optional runtime payload features, but payload installation does not enable the manifest host:

- `installer/Product.wxs:113` - C# plugin host and .NET runtime feature
- `installer/Product.wxs:116` - Python runtime feature
- `installer/Product.wxs:119` - Tcl runtime feature

The safe-mode shortcut disables all plugin autoload:

- `installer/Components/InstalledMode.wxs:37` - safe mode passes `--no-auto --no-plugins`

## Runtime Search Paths

Manifest runtime loading behaviour is located only in manifest loader code:

- `src/common/fabulor-plugin-host.c` - Tcl loader resolves the installed `Runtime\Tcl` root and loads `tcl86t.dll` without modifying process `PATH` or `TCL_LIBRARY`
- `src/common/fabulor-plugin-host.c:1728` - C# loader loads `hostfxr.dll`
- `src/common/fabulor-plugin-host.c:1804` - Python manifest loader loads `hcpython3.dll` through the legacy plugin loader

These are reachable only through language-specific manifest loader functions after manifest discovery and load-order resolution, so they do not affect normal startup while the manifest gate is disabled.

## Legacy Plugin Caveat

Manifest-disabled startup is not the same as global plugin-disabled startup.

With `FABULOR_ENABLE_MANIFEST_PLUGINS` unset, the legacy plugin autoload path still loads bundled plugins and user DLL-style add-ons unless `--no-plugins` is passed:

- `src/common/plugin.c:885` through `src/common/plugin.c:892` - bundled Windows plugin DLL autoload
- `src/common/plugin.c:897` - user `addons` DLL autoload
- `src/common/plugin.c:799` - bare plugin names resolve under `get_xdir()/addons`
- `src/common/plugin.c:800` and `src/common/plugin.c:806` - plugin loading uses `g_module_open`

This caveat does not weaken the manifest-host disabled boundary, but it matters for the broader checklist wording about user-controlled `addons` folders. For a startup mode that avoids user-controlled add-on loading entirely, use `--no-plugins`.

Coexistence decision, 2026-07-12:

- Legacy native C plugin DLLs remain first-party compatibility components loaded by the original plugin loader.
- The user-facing Fabulor plugin model remains C#, Python, and Tcl.
- Native DLL add-ons under the profile `addons` directory remain trusted local code and are outside manifest dependency resolution, manifest capability metadata, and the shared manifest callback registry.
- The manifest schema will not grow a native C/C++ language target until there is a separate design for native dependency loading, signing/provenance, and sandboxing expectations.
- Existing first-party native plugins should migrate only through plugin-specific redesigns around the shared API, not through a blanket manifest wrapper.

## Follow-Up For Pass 2

- Review manifest root canonicalization and containment before making the enable flag user-facing.
- Continue live validation of manifest Tcl loading against the bundled runtime after installer rebuilds.
- Review C# runtime root environment overrides before enabling third-party manifests.
- Track the legacy native `addons` DLL path as trusted local code separate from the manifest-host boundary.

## Pass 2 Scope

This is pass 2 of the plugin loader boundary review from `To-Do.md`. It reviews the current manifest-host design before `FABULOR_ENABLE_MANIFEST_PLUGINS=1` becomes user-facing.

The audit covers:

- Plugin root discovery and root containment.
- Manifest parsing, field validation, type checks, malformed JSON handling, size limits, and error isolation.
- Entrypoint resolution and language-to-extension consistency.
- C#, Python, and Tcl runtime loading paths and interpreter boundaries.
- Callback registration, dispatch lifetime, main-thread behavior, cleanup, and failure isolation.
- Whether declared manifest `capabilities` are advisory or enforced.
- Minimum fixes required before enabling third-party manifest plugins.

## Pass 2 Result

Status: do not expose `FABULOR_ENABLE_MANIFEST_PLUGINS=1` to users yet.

The manifest host is useful as a development scaffold, but it is not ready to enable by default for third-party plugins. Strict JSON/schema validation, entrypoint containment, runtime-root policy, capability enforcement, callback lifetime, and per-manifest runtime isolation have since been hardened as recorded below.

## Root Discovery

Status: addressed on 2026-07-13.

Manifest autoload has two explicit production roots:

- The bundled root is `Plugins` beside `fabulor.exe`, resolved from the executable installation directory.
- The user root is `plugins` beneath the Fabulor profile directory returned by `get_xdir()`.

The bundled manifest root no longer inherits the legacy `ZOITECHAT_LIBDIR` environment override during normal Windows operation. The legacy library-relative fallback is reachable only when executable-directory resolution fails and `FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS=1` explicitly enables development roots.

`src/common/fabulor-plugin-path-policy.c` now applies the discovery boundary before manifest parsing:

- Each approved root is converted to an absolute canonical lexical path and must be a real directory.
- Root paths, direct plugin directories, and `plugin.json` files are queried without following symbolic links.
- Windows additionally rejects any of those paths carrying `FILE_ATTRIBUTE_REPARSE_POINT`, covering junctions and other reparse types even when GIO does not classify them as symbolic links.
- Enumerated plugin names must be direct child names: empty names, `.`, `..`, absolute paths, separators, and Windows alternate-data-stream colons are rejected before path construction.
- Constructed plugin directories must remain strict children of the approved root, and manifests must remain strict children of their plugin directory.
- Plugin directories must be directories and manifests must be regular files. Rejected entries are isolated with diagnostics while discovery continues for unrelated siblings; an invalid root rejects only that root.
- Canonical trusted paths are retained in the parsed manifest rather than rebuilding execution paths from the original caller text.

Native regression coverage in `src/common/tests/test-fabulor-plugin-manifest-json.c` exercises valid canonical trees, traversal and absolute-name rejection, and privilege-free NTFS junction rejection for both roots and plugin directories. File symbolic-link rejection is also exercised when the Windows environment grants symbolic-link creation.

## Manifest Parsing And Validation

Status: addressed on 2026-07-13.

The regex field extractor has been replaced by a bounded, schema-aware JSON parser in `src/common/fabulor-plugin-manifest-json.c`. Manifest loading in `src/common/fabulor-plugin-host.c` now:

- Requires `plugin.json` to be a regular, non-symbolic-link file.
- Rejects files outside the 1-byte to 64-KiB range before allocation and enforces the same limit while streaming the file to handle size changes safely.
- Requires exactly one root object with all 11 documented fields and no unknown or duplicate fields.
- Requires the documented JSON types; notably, `requires_api_version` is an unsigned JSON integer from `1` through the host `guint` maximum, not a quoted string.
- Rejects malformed JSON, trailing commas/content, invalid UTF-8, invalid Unicode escapes/surrogate pairs, decoded NUL, and control characters in strings.
- Enforces documented UTF-8 byte limits for strings and count/item limits for dependency and capability arrays.
- Rejects empty or duplicate dependency and capability strings during parsing.

Discovery records a diagnostic for an unreadable or invalid manifest and continues scanning sibling plugin directories. Load-order resolution no longer fails all plugins because one manifest is invalid. It skips that plugin and skips direct or transitive dependants of invalid, incompatible, safe-mode-disabled, or blacklisted plugins. Unrelated valid plugins remain eligible to load. Dependency cycles among the remaining eligible plugins continue to be a catalog-level error because no valid load order exists for that cycle.

Coverage is provided by `src/common/tests/test-fabulor-plugin-manifest-json.c`, built and executed through `manifest-json-tests.vcxproj` as part of the Windows solution. The repository manifest lint independently checks the same strict sample schema, duplicate keys, JSON types, and limits. Authoring documentation and all C#, Python, and Tcl sample manifests now use the strict contract.

## Entrypoint Resolution

At the time of the initial audit, entrypoints were built as a simple child path and checked only for existence:

- `src/common/fabulor-plugin-host.c:384` - manifest directory is taken from the manifest path
- `src/common/fabulor-plugin-host.c:406` - `entrypoint_path` is built from `plugin_directory` plus manifest `entrypoint`
- `src/common/fabulor-plugin-host.c:570` - validation checks only that the resulting path exists

Entrypoints are later executed or loaded by language:

- `src/common/fabulor-plugin-host.c:1836` - Python uses `/LOAD "<entrypoint_path>"`
- `src/common/fabulor-plugin-host.c:1887` - Tcl calls `Tcl_EvalFile()`
- `src/common/fabulor-plugin-host.c:2021` - C# passes the assembly path to the managed bridge

Pre-enable issues:

- `entrypoint` is documented as relative, but absolute paths and `..` traversal are not rejected.
- The resolved entrypoint is not canonicalized or checked to stay inside the plugin directory.
- Symlink/reparse-point escapes are not handled.
- Extension/language consistency is not enforced. A Python manifest can point at a non-`.py` file, Tcl at a non-`.tcl` file, and C# at a non-`.dll` file.
- Readability and file type are not checked; existence alone is not enough.

Minimum fix: require a relative entrypoint, reject absolute paths and `..`, canonicalize the resolved path, verify it remains under the canonical plugin directory, reject symlink/reparse escapes or define a safe policy, require a regular readable file, and enforce `.dll`, `.py`, and `.tcl` by language.

Fix status, 2026-07-14:

- Manifest parsing no longer constructs an executable path by concatenating untrusted data.
- Catalog validation resolves the entrypoint through the canonical plugin directory and accepts only a direct child regular file. Absolute, traversal, and nested names are rejected.
- Symbolic links and Windows reparse points are rejected without following them, and read access is required.
- Language and extension are enforced as `.dll` for C#, `.py` for Python, and `.tcl` for Tcl.
- The same policy is rerun immediately before each language loader call, and only the refreshed accepted path is passed to the runtime.
- These checks contain accidental and cooperative path escapes. As with the rest of the manifest model, they are not an operating-system sandbox and cannot prevent the same user from replacing ordinary file contents after validation.
- The native suite now has 17 tests, including valid C#/Python/Tcl entrypoints and rejection of absolute, traversal, nested, missing, wrong-extension, wrong-type, symbolic-link, and directory-reparse cases.
- A full MSVC x64 Release rebuild completed with 0 warnings and 0 errors and passed all 17 manifest/path tests.
- The Python host capability and interpreter-isolation suites passed all 9 and 7 tests respectively.
- The x64 MSI and bootstrapper rebuilt with 0 errors when external ICE validation was suppressed; the existing empty GTK4 `lib\\gio` harvest warning remains. Local ICE validation could not be completed because the Windows Installer service was unavailable to the first run and the elevated validator did not terminate before the command timeout.
- Installed-upgrade follow-up found that the maintained C# sample still declared its build-tree DLL path, which conflicts with the direct-child runtime policy. The sample and authoring guide now declare the flattened deployment path `GreeterPlugin.dll`, and repository manifest lint enforces direct filenames plus the language-specific extension so future samples cannot drift from the host policy.
- The same live check confirmed that all three manifest samples load under the gate. Their `message` callbacks intentionally observe incoming IRC `PRIVMSG` events, not locally entered `command:SAY` text; sample output and authoring documentation now state that distinction explicitly.

## Runtime Loading

The Tcl runtime resolver now treats the installed executable-relative runtime as the normal root:

- `src/common/fabulor-plugin-host.c` - resolves `Runtime\Tcl` beside `fabulor.exe` before any development roots.
- `src/common/fabulor-plugin-host.c` - accepts `FABULOR_TCL_RUNTIME_ROOT`, current-working-directory `Runtime\Tcl`, and plugin-lib-relative roots only when `FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS=1`.
- `src/common/fabulor-plugin-host.c` - canonicalizes the selected Tcl runtime root and requires `bin\tcl86t.dll` to be a regular file.
- `src/common/fabulor-plugin-host.c` - loads `tcl86t.dll` with `LoadLibraryExA()` using `LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS`.
- `src/common/fabulor-plugin-host.c` - sets Tcl's `tcl_library` variable on each interpreter before `Tcl_Init()` instead of setting process-global `TCL_LIBRARY`.

The C# runtime resolver now treats the installed executable-relative `Runtime\DotNet` directory as the normal runtime and managed-bridge root:

- Environment roots (`FABULOR_DOTNET_ROOT`, `DOTNET_ROOT`, and `FABULOR_CSHARP_BRIDGE_ROOT`), current-working-directory roots, source-tree bridge outputs, and the machine-wide .NET installation are accepted only when `FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS=1`.
- Selected runtime and bridge roots are canonicalized.
- Managed bridge assemblies and runtime configuration files must be regular files.
- `hostfxr.dll` must be a regular file and is loaded by absolute path with `LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS`.

The C# bridge creates a collectible assembly load context and resolves dependencies with `AssemblyDependencyResolver`:

- `src/managed/Fabulor.PluginHost/NativeExports.cs:26` - per-plugin `PluginLoadContext`
- `src/managed/Fabulor.PluginHost/NativeExports.cs:33` - `AssemblyDependencyResolver`
- `src/managed/Fabulor.PluginHost/NativeExports.cs:44` and `src/managed/Fabulor.PluginHost/NativeExports.cs:157` - `LoadFromAssemblyPath()`
- `src/managed/Fabulor.PluginHost/NativeExports.cs:187` - plugin `Init()` is called
- `src/managed/Fabulor.PluginHost/NativeExports.cs:190` - load exceptions are caught

The Python path loads the legacy Python plugin and asks it to load the manifest entrypoint:

- `src/common/fabulor-plugin-host.c:1803` - `hcpython3.dll` path comes from `manifest_plugin_get_libdir()`
- `src/common/fabulor-plugin-host.c:1804` - loaded through the legacy plugin loader
- `src/common/fabulor-plugin-host.c:1836` - manifest file is loaded through the legacy `LOAD` command
- `plugins/python/python.py:146` and `plugins/python/python.py:149` - Python plugin opens and executes the file

Runtime-root policy status, 2026-07-12:

- Addressed for both Tcl and C#. Production manifest loading now anchors both runtimes beside `fabulor.exe`, while developer roots require the explicit `FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS=1` gate.
- Python manifest entrypoints no longer execute in the legacy plugin's global interpreter. The native manifest host establishes a per-process token and sends base64-encoded manifest identity/capability/path metadata through a private internal command. The trusted main Python interpreter then creates one Python 3.14 `concurrent.interpreters.Interpreter` for that manifest and loads a pure-Python proxy API there. Ordinary `/LOAD` and `/PY LOAD` requests remain confined to the profile `addons` directory and cannot select manifest roots, and ordinary unload/reload commands cannot mutate manifest-host-owned plugins.
- Python callback integration is not the shared manifest registry. `plugins/python/_zoitechat.py:304` implements callback helpers by mapping to legacy hooks.
- C# dependency resolution is per assembly-load context, but native dependency policy and allowed dependency locations are not constrained by manifest capabilities.

Minimum fix: restrict remaining C# runtime roots to installed/bundled roots by default, move developer overrides behind a separate development flag or diagnostic build path, define Python manifest interpreter isolation separately from legacy scripting, and define native dependency loading policy for C#.

Fix status, 2026-07-12:

- The Tcl runtime no longer prepends its `bin` directory to process `PATH`.
- The Tcl runtime no longer sets process-global `TCL_LIBRARY`; it sets `tcl_library` on each manifest interpreter before `Tcl_Init()`.
- The Tcl DLL is loaded from the selected absolute runtime root using `LoadLibraryExA()` with DLL-load-dir/default-dir search flags.
- Normal Tcl runtime resolution is limited to the installed `Runtime\Tcl` beside the executable. `FABULOR_TCL_RUNTIME_ROOT`, current-working-directory `Runtime\Tcl`, and plugin-lib-relative roots require `FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS=1`.
- The Python runtime now canonicalizes requested script paths before loading.
- Relative Python load requests are treated as simple add-ons and resolve under the profile `addons` directory, preferring the documented `addons\<name>\<name>.py` layout while keeping legacy flat `addons\*.py` as a compatibility fallback.
- Absolute Python load requests are rejected unless the resolved `.py` file is under the profile `addons` directory, the profile `plugins` manifest root, or the bundled `Plugins` manifest root.
- Manifest Python entrypoints now receive per-plugin interpreter isolation. The CFFI extension remains confined to the trusted main scripting interpreter, avoiding unsupported cross-interpreter extension state. Manifest callbacks still use bounded legacy proxy hooks in that main interpreter rather than the shared native manifest registry.

## Callback And Event System

The registry is per manifest host and records callback entries by event name. The relevant boundaries are `fabulor_callback_registry_new()`, `fabulor_callback_registry_register()`, `fabulor_callback_registry_fire_event()`, `fabulor_callback_registry_remove_plugin()`, `fabulor_callback_registry_shutdown()`, and `fabulor_plugin_host_shutdown()` in `src/common/fabulor-plugin-host.c`.

Status: callback lifetime hardening completed on 2026-07-13.

- C# and Tcl registrations in the shared registry accept only `message`, `server`, `server:<name>`, `print`, `print:<event>`, `command`, and `command:<name>` event forms. Event names are valid UTF-8 without control characters and are limited to 128 bytes; handler names use the same character policy and are limited to 256 bytes.
- The registry permits at most 64 callbacks per plugin and 256 callbacks for one event. An identical plugin/language/event/handler tuple is rejected as a duplicate.
- Registration is restricted to the plugin host thread. Dispatch copies the selected callback entries while holding the registry mutex and invokes the snapshot without holding that mutex, so callback code cannot invalidate the active iteration.
- Cross-thread dispatch is capped at 256 queued events and 1 MiB per payload. Each queued item holds a registry reference and releases it through the main-context destroy notification. Caller-owned loader data is not retained across threads.
- Shutdown marks the registry closed and removes all entries before C# or Tcl runtime teardown. Already queued items observe the closed state and return without entering a language runtime.
- `fabulor_callback_registry_remove_plugin()` removes all entries owned by one plugin. Tcl, C#, simple C#, generic catalog, and application autoload failure paths call it so partially initialized plugins cannot leave native callbacks behind. The API also supplies the cleanup primitive for a future live manifest-unload operation.
- Python manifest callbacks remain backed by small legacy hook proxies in the trusted main interpreter. Callback functions and userdata live only in the owning subinterpreter; events and host operations cross the boundary as bounded JSON-compatible data. The boundary applies its documented event-family allowlist and 128-byte event-name limit, caps every manifest plugin at 64 hooks, rejects identical event/callback registrations, and removes proxy hooks before closing the interpreter. Python callbacks therefore do not hold or depend on the native shared registry.
- Focused Python boundary tests cover capability isolation, event allowlisting and length, duplicate rejection, and callback count limits. The repository lint workflow executes these tests. The native host passed the x64 Release build and MSVC `/analyze`; no callback-host-specific analyzer findings were reported.

## Capabilities

Status: enforced deny-by-default policy implemented on 2026-07-12.

- Manifest validation rejects unknown and duplicate capability names.
- The allowlist covers message sending, session reads, UI output, command execution/management, preference reads/writes, and message/server/print/command/timer/unload callback families.
- C# passes plugin identity on every privileged native callback; the native boundary checks the manifest before invoking the shared API.
- Tcl stores a private capability set in each interpreter state and checks it before privileged commands.
- Python manifest loading attaches identity and capabilities to each plugin object. API wrappers derive the calling plugin and deny undeclared operations; trusted simple add-ons are unaffected.
- The shared callback registry independently verifies event-family capabilities for C# and Tcl registrations. Python's legacy hook-backed callbacks enforce the same mapping in the Python API layer.
- Logging and pure text stripping remain unprivileged.

Capabilities constrain access to cooperative Fabulor host APIs. They do not provide operating-system sandboxing or contain native code. Per-manifest runtime isolation prevents language-level plugin state from being shared accidentally, but does not change those operating-system limits.

## Minimum Pre-Enable Fix List

Before `FABULOR_ENABLE_MANIFEST_PLUGINS=1` becomes user-facing, require at least:

- Canonical root containment for discovered roots, plugin directories, manifest paths, and entrypoints. Status: addressed on 2026-07-14.
- Rejection of absolute entrypoints, `..`, symlink/reparse escapes, unreadable files, and language/extension mismatches. Status: addressed on 2026-07-14.
- Strict JSON parsing with schema/type validation and manifest size limits. Status: addressed on 2026-07-13.
- Per-plugin error isolation policy that does not let one bad manifest unexpectedly block unrelated plugins unless that is deliberately documented. Status: addressed on 2026-07-13; dependency cycles remain a deliberate catalog-level error.
- Runtime-root policy that removes normal-user reliance on environment variables and current working directory. Status: addressed for Tcl and C# on 2026-07-12.
- Removal or containment of Tcl process-wide `PATH` mutation. Status: addressed for manifest Tcl loading.
- A Python manifest host design that is separate from the legacy global Python plugin, or a clear decision that Python manifests remain disabled. Status: addressed on 2026-07-14 with one Python 3.14 subinterpreter per manifest plugin and a pure-Python proxy API; trusted simple add-ons remain on the legacy shared interpreter.
- Callback event allowlists, length/count limits, duplicate policy, safe queued-dispatch lifetime, and per-plugin callback cleanup. Status: addressed on 2026-07-13 and completed for isolated Python callback ownership on 2026-07-14. Python uses bounded main-interpreter proxy hooks while callback functions and userdata remain in the owning subinterpreter.
- A capability policy decision: explicitly advisory-only with no security claims, or enforced gates for every exposed API and event surface. Status: enforced deny-by-default policy implemented across C#, Python, and Tcl on 2026-07-12.

Pass-2 decision, 2026-07-14:

- The path, parser, runtime-root, interpreter-lifetime, callback-lifetime, and cooperative capability prerequisites above are addressed. The manifest host may proceed to an explicitly opt-in user preference; it must not become enabled by default.
- The preference must state that manifest plugins are trusted local code running with the user's operating-system privileges. Capabilities restrict cooperative Fabulor API access; they are not a process sandbox and do not prevent Python, Tcl, or managed code from using their language and operating-system facilities.
- Enabling the preference must require an explicit confirmation before the value is persisted. The confirmation must identify the profile plugin root and advise loading only trusted code.
- The persisted preference must default to disabled for new and existing profiles, require a client restart to take effect, and be ignored when Fabulor safe mode disables third-party plugins.
- `FABULOR_ENABLE_MANIFEST_PLUGINS=1` may remain as a documented developer/testing override while the preference is introduced, but installer defaults and migrations must not set either mechanism silently.
- Startup must continue reporting successful lifecycle operations and per-plugin failures. Invalid or failed plugins remain isolated from unrelated valid plugins under the documented dependency policy.

This decision closes the pre-enable design audit. Implementing and validating the preference, confirmation flow, restart behavior, safe-mode precedence, and disabled-profile migration tests is a separate product stage before the environment gate can be retired from normal use.

## Manifest Plugin Preference Rollout

Date: 2026-07-14

The first user-facing implementation remains explicitly opt-in:

- `gui_manifest_plugins` is a Boolean profile preference. New profiles and existing profiles without the key receive the zero-initialized disabled default; installers and migrations do not enable it.
- **Preferences > Advanced** exposes **Enable manifest plugins (requires restart)**. Changing the preference from disabled to enabled requires a modal confirmation before preferences are saved.
- The confirmation identifies the profile `plugins` root and states that manifest plugins are trusted code running with the user's operating-system privileges. Cancelling leaves the saved preference disabled.
- Startup enables the manifest host when either the saved preference is enabled or the developer override is exactly `FABULOR_ENABLE_MANIFEST_PLUGINS=1`.
- Safe mode (`--no-plugins`) takes precedence over both enable paths and prevents manifest discovery and runtime initialization.
- Disabling the preference also requires restart so the active host can complete its normal process-lifetime teardown rather than attempting partial live unload.

Validation so far:

- A focused policy test covers default-disabled profiles, valid and invalid environment values, preference enabling, and safe-mode precedence.
- The native manifest/path/policy suite passes all 18 tests.
- A full MSVC x64 Release rebuild completed with 0 warnings and 0 errors.

Installed-upgrade validation completed successfully on 2026-07-14. The disabled default did not load manifest plugins; cancelling the trusted-code confirmation did not enable them; accepting and saving the preference persisted it and loaded all three maintained manifest samples after a normal restart; disabling it again persisted across restart; and safe mode suppressed manifest loading even while the preference was enabled. No environment override was present during these checks, so the result exercised the saved preference directly. The environment override remains available for developer and recovery testing only.

## Repository Security Tool Pass Scope

This stage covers the repository security tool pass from `To-Do.md`.

The audit covers:

- Local security tool inventory.
- Secret scanning across tracked files.
- Static analysis available in the local environment.
- Dependency and vulnerability checks for .NET, Python, Node, and bundled runtime payloads where applicable.
- Installer and runtime payload provenance, hash, and packaging boundary review.
- Current GitHub Actions check state for PR 7.

## Tool Inventory

Available locally:

- `dotnet` 10.0.926.27113
- `git` 2.55.0.1
- `gh`
- `node` 24.18.0.0 and `npm`
- Python and pip
- `rg`

Not found on `PATH` in this pass:

- MSVC `cl`
- `MSBuild`
- CodeQL CLI
- Semgrep
- gitleaks
- trufflehog
- pip-audit
- Safety
- Bandit

GitHub Actions currently provides CodeQL-style checks even though CodeQL CLI is not locally available.

## Secret Scan

High-confidence tracked-file patterns checked:

- Private key blocks.
- AWS access key ids.
- GitHub classic and fine-grained token prefixes.
- Slack token prefixes.
- Stripe live secret keys.
- Google API key prefix.

Result: no high-confidence secret matches were found.

Generic assignment scan result:

- `src/fe-gtk/servlistgui.c:2614` matched a UI label string for `Password:`. This is not a secret.

Broad keyword review result:

- The broad keyword scan produced expected references to password/token handling code, OIDC workflow permissions, license text, and parser variable names.
- Reviewed examples included `plugins/fishlim/*`, `src/common/secretstore.*`, SASL/SCRAM handling in `src/common/inbound.c` and `src/common/scram.c`, and the GitHub workflow `id-token: write` permission.
- No committed credential was identified in this pass.

Limit: gitleaks/trufflehog were not installed, so this was a regex-based local scan rather than a dedicated entropy/history-aware secret scan.

## Static Analysis

Dedicated local scanners were not available:

- MSVC `/analyze` was not run because `cl` was not on `PATH`.
- CodeQL CLI was not installed locally.
- Semgrep was not installed locally.

Fallback static pattern scans were run for risky C/C++ APIs and process/library loading surfaces.

Notable results:

- Legacy string APIs are widespread, including `strcpy`, `strcat`, `sprintf`, `sscanf`, `memcpy`, and related calls across `src/` and `plugins/`.
- Process/library loading surfaces were found in expected areas:
  - `plugins/exec/exec.c:84` - `CreateProcess()`
  - `src/common/plugin.c:800` and `src/common/plugin.c:806` - `g_module_open()`
  - `src/common/fabulor-plugin-host.c:834` - Tcl `LoadLibraryA()`
  - `src/common/fabulor-plugin-host.c:1728` - C# `hostfxr.dll` `LoadLibraryA()`
  - `src/common/outbound.c:1904` - Unix shell execution path
  - `src/fe-gtk/fe-gtk.c:1700` and `src/fe-gtk/fe-gtk.c:1709` - URL opening / spawn paths
  - `src/common/gtk3-theme-service.c:1057`, `src/common/gtk3-theme-service.c:1089`, and `src/common/gtk3-theme-service.c:1103` - external helper execution
- TLS override surfaces were found:
  - `src/common/outbound.c:3396`, `src/common/outbound.c:3615`, and `src/common/outbound.c:3744` - `-ssl-noverify`
  - `src/fe-gtk/servlistgui.c:2581` - UI option to accept invalid SSL certificates

Highest-priority manual finding from this pass:

- `plugins/exec/exec.c:62` through `plugins/exec/exec.c:84` builds a 1024-byte command line using `strcpy()` and `strcat()` from user-controlled command text before calling `CreateProcess()`. This is a legacy built-in plugin risk. It should be bounds-checked with `g_snprintf()`/`GString` or equivalent, and the plugin should clearly remain opt-in.

Other results are too broad to triage fully in this tool pass and belong in the later targeted high-risk review.

## Dependency Checks

.NET projects checked with `dotnet list package --vulnerable --include-transitive`:

- `src/managed/Fabulor.PluginHost/Fabulor.PluginHost.csproj` - no vulnerable packages reported.
- `src/managed/Fabulor.PluginAbstractions/Fabulor.PluginAbstractions.csproj` - no vulnerable packages reported.
- `samples/plugins/example.csharp.greeter/GreeterPlugin.csproj` - no vulnerable packages reported.
- `installer/UX/Fabulor.BA.csproj` - no vulnerable packages reported after allowing NuGet vulnerability metadata access.

Only `installer/UX/Fabulor.BA.csproj` has an external package reference in the reviewed `.csproj` files:

- `WixToolset.BootstrapperApplicationApi` 7.0.0

Python/Node dependency files:

- No `package.json`, lockfile, `requirements*.txt`, `pyproject.toml`, `Pipfile`, or Poetry lockfile was found in the project surface scan.
- Python scripts are present, but there is no repository-level Python dependency manifest to audit with a dependency scanner.

Bundled runtime payloads:

- `Runtime/Python314` includes Python binaries plus OpenSSL DLLs (`libcrypto-3.dll`, `libssl-3.dll`) and other embedded modules.
- `Runtime/Tcl` includes Tcl/Tk executables and DLLs.
- `Runtime/GTK4` is a large bundled runtime tree.

Limit: vulnerability status of bundled binary/runtime payloads was not verified against a binary vulnerability database in this local pass. That requires either a curated payload bill of materials with versions and hashes or a scanner capable of binary/package identification.

## GitHub Actions Checks

Current PR 7 checks were queried with `gh pr checks 7 --repo Fabulor/fabulor` after allowing GitHub API access.

All current checks were passing:

- `Analyze (csharp)` - pass
- `Analyze (python)` - pass
- `Build validation` - pass
- `repository_lint` - pass
- `windows_build (x64, x64)` - pass

Relevant workflow coverage found locally:

- `.github/workflows/lint.yml` validates workflow files, sample manifest shape, sample manifest dependency references, and Python syntax.
- `.github/workflows/tests.yml` builds native and managed validation targets.
- `.github/workflows/windows-build.yml` builds the WiX MSI and bootstrapper and uploads both artifacts.

## Installer And Runtime Payload Review

Local runtime payload sizes:

- `Runtime/GTK4` - 6112 files, 1,153,962,024 bytes.
- `Runtime/Python312` - 35 files, 22,500,655 bytes.
- `Runtime/Python314` - 36 files, 24,456,647 bytes.
- `Runtime/Tcl` - 5633 files, 101,988,301 bytes.

Representative local SHA-256 hashes:

- `Runtime/Tcl/bin/tcl86t.dll` - `89C325C72B3C1B39B3F116BEE7CA230348F49042E87D9BF130524C502528F92D`
- `Runtime/Python314/python.exe` - `03168C01B7B7491423350E82C26FEE71F35B43694D1319D3C668BDA6903A0C38`
- `Runtime/Python314/python314.dll` - `FDE89CDB5C2D08AE65DE7EF4ABCA1876C93BA3796002F5AC0BF7E4D4F5A94DA0`
- `Runtime/GTK4/bin/glib-2.0-0.dll` - `77DE94F0A94B7ADF7B1EA669FF815FED36D3B6D3C282739DFC1A88460900A2D2`
- `Runtime/GTK4/bin/gobject-2.0-0.dll` - `73DFF8E56382B91A384BB1FA812107FBA6F1B850A70CF9E2EEF531FD3B688FDB`
- `Runtime/GTK4/bin/gtk-4-1.dll` - `68CD82FD708BB22364B2F00E7DB8DE8173E2C5F996F48CE1478006ABF53CD0B9`

Existing local release artifact hashes:

- `installer/bin/x64/Release/Fabulor.msi` - `DE770A26689BCAAF9FD153CAA481E6D6A8238F3309BBEF4860D9D36519F4C3A4`
- `installer/bin/x64/Release/FabulorSetup.exe` - `BE15342A7A0E8D92B0A0E01D3B75211C0B31C359A8F2C9BD31812768B5054E6C`

Packaging boundaries:

- `installer/Fabulor.wixproj` defines payload roots for the staged native payload, GTK4 runtime, managed plugin host, .NET runtime, Python 3.14 runtime, and Tcl runtime.
- `installer/Components/PythonRuntime.wxs` packages `$(var.PythonRuntimeRoot)\**`.
- `installer/Components/Tcl.wxs` packages Tcl `bin\**` and `lib\**`.
- `installer/Components/DotNet.wxs` packages the managed plugin host plus `host\fxr\<version>\**` and `shared\Microsoft.NETCore.App\<version>\**`.
- `installer/Components/GTK4Allowlist.wxs` packages only the deterministic,
  manifest-backed GTK4 runtime allowlist with explicit directory ownership.
- `installer/Components/Core.wxs` explicitly lists the core native payload DLLs and executables.
- `installer/Components/Plugins.wxs` explicitly lists built-in plugin DLLs.

Provenance issue:

- The runtime payload is not backed by a checked-in bill of materials with source URL, version, expected hash, and license/provenance status for every harvested binary.
- The workflows download several third-party payloads by URL, but the reviewed workflow snippets do not verify expected hashes after download.
- Broad wildcard harvesting remains for other runtime surfaces such as Python,
  Tcl, and .NET; the GTK4 surface now rejects files outside its staged allowlist
  and validates installed paths, sizes, and SHA-256 hashes in the shipping MSI.

Minimum follow-up:

- Create a runtime payload bill of materials covering GTK, Python, Tcl, .NET,
  MSYS2 packages, and other bundled binaries. Any future updater runtime must
  be added only with a Fabulor-owned authenticated feed.
- Add expected SHA-256 checks to CI download/extraction steps.
- Replace broad wildcard harvesting with allowlists or generated locked manifests where practical.
- Add an installer payload audit step that fails on unexpected files.
- Record release artifact hashes as part of release notes or provenance metadata.

## Targeted High-Risk Code Review Scope

This stage covers the targeted high-risk code review from `To-Do.md`.

The audit prioritized:

- Remotely influenced file paths and file writes.
- Archive import and extraction code.
- Process execution and dynamic library loading.
- TLS verification bypasses and insecure connection switches.
- User-controlled rendering and markup paths.
- Legacy plugin loading boundaries adjacent to the manifest-plugin work.

## Targeted Review Result

Status: completed, with several legacy high-risk findings outside the manifest-disabled boundary.

No evidence was found that the disabled manifest host is reachable through these reviewed surfaces. The highest-priority issues are in adjacent legacy features: GTK3 theme archive extraction, the built-in Exec plugin, and bare-name dynamic library loading.

## Finding: GTK3 Theme Archive Extraction Lacks Containment Checks

Severity: high for Unix-like builds; medium for Windows helper execution and extraction policy.

Evidence:

- `src/common/gtk3-theme-service.c:1025` - `extract_archive()`.
- `src/common/gtk3-theme-service.c:1057`, `src/common/gtk3-theme-service.c:1089`, and `src/common/gtk3-theme-service.c:1103` - Windows extraction shells out through `g_spawn_sync()`.
- `src/common/gtk3-theme-service.c:1062` - searches `PATH` for `tar.exe` before falling back to `SystemRoot\System32\tar.exe`.
- `src/common/gtk3-theme-service.c:1147` - libarchive extraction options omit `ARCHIVE_EXTRACT_SECURE_NODOTDOT` and `ARCHIVE_EXTRACT_SECURE_NOABSOLUTEPATHS`.
- `src/common/gtk3-theme-service.c:1163` through `src/common/gtk3-theme-service.c:1169` - archive entry paths are joined under the temp directory and written without rejecting absolute paths, `..`, or link escapes.
- `src/common/tests/test-gtk3-theme-service.c:338`, `src/common/tests/test-gtk3-theme-service.c:365`, `src/common/tests/test-gtk3-theme-service.c:607`, and `src/common/tests/test-gtk3-theme-service.c:648` - archive tests cover invalid archives and nested roots, but no path traversal, absolute path, symlink, hardlink, or extraction-containment cases were found.

Risk:

An imported theme archive is user-selected, but theme archives are likely to be downloaded from third parties. On Unix-like builds, a malicious archive can plausibly write outside the extraction temp directory unless libarchive containment flags and explicit path validation are added. On Windows, the code delegates extraction to `powershell` or `tar.exe` with `G_SPAWN_SEARCH_PATH`; this increases reliance on PATH search order and external tool behavior instead of an application-controlled extraction policy.

Recommended fix:

- Reject archive entries with absolute paths, drive-qualified paths, `..` segments, empty names, alternate data stream syntax on Windows, and unsafe link targets before writing.
- Use libarchive secure extraction flags where available, including no-dot-dot and no-absolute-path protections.
- After resolving each destination, verify it remains under the canonical extraction root.
- Avoid `G_SPAWN_SEARCH_PATH` for extraction helpers on Windows. Prefer an in-process archive reader or absolute helper paths from trusted system locations.
- Add regression tests for traversal, absolute paths, symlinks, hardlinks, nested archive roots, and cleanup after partial extraction.

Fix status, 2026-07-12:

- Addressed for the in-process libarchive extractor in `src/common/gtk3-theme-service.c`.
- Archive entry paths are now rejected before write when they are empty, absolute, drive-qualified, contain `..`, contain colon syntax, contain backslashes, contain control characters, or include empty components.
- Symlink and hardlink entries are rejected outright for theme archives.
- Libarchive secure extraction flags are enabled when compatible with the absolute temp-root destination rewrite: `ARCHIVE_EXTRACT_SECURE_NODOTDOT` and `ARCHIVE_EXTRACT_SECURE_SYMLINKS`. Absolute archive paths are rejected by explicit entry validation before the destination is rewritten.
- Each destination is built under the canonical extraction root and checked for a root-prefix match before it is passed to libarchive.
- Build files now define `HAVE_LIBARCHIVE` when libarchive is configured: Meson does this when `dependency('libarchive')` is found, and the MSBuild props do this only when both `ArchiveInclude` and `ArchiveLib` are present.
- Windows GTK3 builds used the same libarchive-contained extractor when `HAVE_LIBARCHIVE` was set. Stage 9 first removed libarchive from the supported GTK4 Windows dependency graph, then deleted the GTK3 theme service, adapter, preference workflow, saved keys, and dedicated tests entirely.
- Stage 9 also removed retained `.hct` reading from this GTK3 extraction path. `src/common/theme-archive-reader.c` now reads only `colors.conf` or `pevents.conf` through the absolute Windows system `tar.exe`, resolving the system directory through the Windows API rather than the environment and avoiding PATH search, command-string interpolation, or filesystem-tree extraction. It limits archive, listing, and output sizes; validates entry components and depth; and rejects duplicate matching files. Four native ZIP-fixture tests cover successful reads, duplicate rejection, decompression-size rejection, and the filename allowlist.
- Historical non-Windows regression coverage verified `..` traversal cleanup plus absolute-path, symlink, and hardlink rejection before the GTK3 importer and its dedicated tests were retired in Stage 9.
- Verification: `git diff --check` passed; the focused WSL GTK3 theme-service test binary compiled with GLib/GIO/libarchive and passed all 18 tests; `src\common\common.vcxproj` built successfully with 15 pre-existing conversion warnings and 0 errors; `src\fe-gtk\fe-gtk.vcxproj` built and linked successfully with 1 pre-existing const-qualifier warning and 0 errors.

GTK4 follow-up, 2026-07-28:

- The supported GTK4 Appearance page now owns a separate desktop-theme archive
  importer. The selected archive is copied with a compressed-size bound into a
  private profile staging directory before inventory, closing replacement
  races between validation and extraction.
- Inventory rejects unsafe Windows paths, duplicate names, excessive depth or
  counts, links, special entries, and excessive per-file or total expanded
  sizes. Extraction uses the absolute system `tar.exe` and an argument vector.
- Only immediate theme roots containing `gtk-4.0/gtk.css` are materialized.
  Unrelated GTK2/GTK3, shell, window-manager, and dock components are ignored.
  The staged filesystem tree is checked again for ordinary files/directories
  and reparse points before collision-free installation.
- Controlled archive tests cover contained extraction and overwrite refusal.
  The six-theme `Orchis-Grey.tar.xz` package was also imported successfully
  through the same test boundary. The UI dispatches import on a worker task.

## Finding: Exec Plugin Uses Unbounded Command Construction

Severity: high if the legacy Exec plugin is installed and loaded.

Evidence:

- `plugins/exec/exec.c:62` - initializes a fixed 1024-byte `commandLine` buffer with `strcpy()`.
- `plugins/exec/exec.c:66` and `plugins/exec/exec.c:71` - appends user-controlled command text with `strcat()`.
- `plugins/exec/exec.c:84` - passes the resulting command line to `CreateProcess()`.
- `installer/Components/Plugins.wxs:20` - packages the Exec plugin DLL.

Risk:

This is an intentional local command-execution plugin, but the fixed buffer and unbounded appends create a memory-corruption risk before process creation. It also means any script, alias, or plugin path that can issue `/EXEC` can run through `cmd.exe /c`.

Recommended fix:

- Replace the fixed buffer with `g_strdup_printf()`, `GString`, or bounded formatting.
- Enforce a command length limit with a clear error path.
- Check `CreatePipe()` and `CreateProcess()` return values before using handles.
- Keep the plugin explicitly opt-in, and consider excluding it from default plugin autoload if the product security posture should not include a shell plugin.

Fix status:

- Addressed in `plugins/exec/exec.c` by replacing the fixed command buffer with a dynamically allocated command line, enforcing an 8192-character command limit, validating pipe/process creation, and preserving the plugin as optional/selectable.

## Finding: Bare-Name Dynamic Library Loading Remains In Legacy Paths

Severity: medium.

Evidence:

- `src/common/plugin.c:800` - bare legacy plugin names resolve under `get_xdir()/addons`.
- `src/common/plugin.c:806` - path-bearing plugin names are passed directly to `g_module_open()`.
- `src/fe-gtk/sexy-spell-entry.c:209` - Enchant is loaded by bare DLL names such as `libenchant-2-2.dll`.
- Historical evidence: the former `plugins/perl/perl.c:1440` loaded Perl with
  `LoadLibraryA(PERL_DLL)`.
- Historical evidence: the former `plugins/perl/perl.c:1452` fallback probed
  `LoadLibraryA("perl56.dll")`.

Risk:

Some of these paths are deliberate extension points, but bare DLL loading relies on process DLL search behavior and PATH contents. That is a weak boundary for installed applications, especially near plugin and scripting surfaces.

Recommended fix:

- Prefer absolute paths rooted in the installed application directory or a trusted runtime directory.
- On Windows, use `SetDefaultDllDirectories()` and `AddDllDirectory()`/`LoadLibraryEx()` with constrained search flags where compatible.
- Keep user add-on loading under the explicit `addons` trust model, but avoid using general DLL search order for dependencies such as Enchant and Perl.

Fix status, 2026-07-12:

- Enchant loading in `src/fe-gtk/sexy-spell-entry.c` is restricted to the absolute application-local `libenchant-2-2.dll` path on Windows. After installed-client validation and soak testing, the Enchant 1.6.1 core/provider fallback and the in-tree legacy Win8 provider were retired; packaging now requires the MSVC/UCRT Enchant 2.8.19 core, upstream WinSpell provider, and ordering file.
- A later URL-paste crash was traced with CDB to cross-CRT heap corruption in the MinGW/MSVCRT Enchant 2.8.19 personal-word-list path. Enchant core and WinSpell are now rebuilt with MSVC/UCRT against Fabulor's GTK/GLib libraries. The analysis and reproducible build are documented in `docs/security/enchant-windows-crash-analysis.md`.
- The legacy Perl project and source tree are retired. Its project and
  `hcperl.dll` startup probe were already absent from the Windows solution and
  WiX payload; repository cleanup Stage 2 removed the remaining source,
  `perl_warnings` configuration surface, obsolete build macro, and stale
  user-facing guidance.
- Modern manifest Tcl loading now uses the installed executable-relative runtime root by default, avoids process `PATH` and `TCL_LIBRARY` mutation, and loads `tcl86t.dll` with constrained `LoadLibraryExA()` flags. The installer preserves the matching `Runtime\Tcl\bin` and `Runtime\Tcl\lib` directories; an earlier flattened layout was detected during installed-package testing and corrected. Simple Tcl add-ons now load independently of the manifest gate from exact `addons\<name>\<name>.tcl` paths, receive separate interpreters, and reject reparse-point roots, directories, and scripts. Their direct command registry rejects duplicate command names and is cleaned up with interpreter state. C# now uses the installed executable-relative `Runtime\DotNet` root by default, gates developer roots explicitly, canonicalizes selected roots, and loads `hostfxr.dll` with constrained `LoadLibraryExA()` flags. The installer preserves the private runtime's `host\fxr` and `shared\Microsoft.NETCore.App` hierarchy and stages the managed bridge in `Runtime\DotNet`; installed-package inspection found and corrected an earlier flattened layout. Trusted simple C# add-ons load only from exact `addons\<name>\<name>.dll` paths after rejecting reparse-point roots, directories, and entry assemblies. They receive collectible managed load contexts, participate in callback dispatch while the manifest host is disabled, and are removed from managed/native registries when initialisation fails. Python manifest loading rejects command-unsafe entrypoint paths before invoking the authenticated runtime hook. The shared legacy/simple Python host preloads `Runtime\Python314\python314.dll` from the executable-relative trusted root with constrained Windows search flags; ambient development lookup is available only behind `FABULOR_ENABLE_DEVELOPMENT_RUNTIME_ROOTS=1`. Manifest Python code now runs in one subinterpreter per plugin without importing CFFI there. The Python installer feature includes `hcpython3.dll`, the API compatibility modules, the isolated manifest proxy, and `_cffi_backend`, with build-time payload validation.

## Finding: Theme Import And Legacy Add-On Loading Need Stronger Canonicalization

Severity: medium.

Evidence:

- `src/common/gtk3-theme-service.c:858` through `src/common/gtk3-theme-service.c:904` - `copy_tree()` recursively copies imported theme content and overwrites files at the destination.
- `src/common/gtk3-theme-service.c:1376` - imported theme tree is copied into the user theme directory after archive extraction and root selection.
- `src/fe-gtk/plugingui.c:212` through `src/fe-gtk/plugingui.c:215` - the add-on GUI uses a string prefix check to decide whether a file is already inside `addons`.
- `src/fe-gtk/plugingui.c:223` through `src/fe-gtk/plugingui.c:227` - external add-ons are copied into `addons` by basename.
- `src/fe-gtk/plugingui.c:244` through `src/fe-gtk/plugingui.c:247` - the load path is interpolated into a `/LOAD` command string.

Risk:

The add-on path is user-driven and does not weaken the manifest-disabled boundary, but string-prefix containment and command-string construction are brittle. The theme copy path depends on extraction safety and does not independently document or enforce a symlink/reparse-point policy.

Recommended fix:

- Canonicalize both the selected file and `addons` directory before containment checks.
- Escape or reject quotes and control characters before constructing `/LOAD`, or route the GUI through a direct plugin-load API instead of the command parser.
- For theme imports, define whether symlinks/reparse points are allowed; if not, reject them during extraction and copy.

Fix status, 2026-07-12:

- Theme archive symlinks and hardlinks are rejected during extraction before the theme tree is copied.
- The Add-ons GUI now canonicalizes the selected file and profile `addons` directory before containment checks.
- External GUI-selected add-on files must copy into the profile `addons` directory before loading; failure to copy stops the load.
- Native plugin DLLs selected through the Add-ons GUI load via `plugin_load()` directly instead of a `/LOAD` command string.
- Script add-ons still use the existing language runtime command hooks, but `LOAD`, `UNLOAD`, and `RELOAD` GUI commands reject paths containing quotes or control characters before constructing the command.
- The GUI file filter no longer advertises unsupported runtime `.cs` compilation. Native plugin DLLs remain selectable through the legacy loader; simple managed C# DLLs auto-load from their exact profile folder layout.
- Startup now prints a consolidated active-plugin report. Native and Python entries come from the live legacy plugin list, while simple Tcl and C# entries come from their successful runtime registries; merely discovered or failed add-ons are not reported as loaded.
- The report is emitted after built-in notification and tray initialization and retains the original startup session as its output destination while auto-connect creates further tabs. Notifications and Tray publish meaningful metadata instead of appearing unnamed or being omitted because they initialized later. The obsolete built-in Identd service was retired on 2026-07-26.
- The maintained C#, Python, and Tcl manifest samples now use independent manifests with only `events.message` and `session.read`. They log locally at startup and demonstrate a one-time message callback without transmitting IRC messages during initialization.

## Finding: Log Mask Can Write Outside The Config Directory By Design

Severity: low to medium, depending on whether config files are considered trusted.

Evidence:

- `src/common/text.c:605` - log paths are built from `prefs.hex_irc_logmask`.
- `src/common/text.c:617` - absolute log masks are accepted.
- `src/common/text.c:627` - Windows path sanitization runs after variable expansion, but it only strips invalid/control characters rather than enforcing a root.

Risk:

This appears to be an intentional advanced preference. It is not remotely reachable by normal chat traffic because server/channel substitutions are sanitized, but imported or attacker-modified configuration can redirect logs to arbitrary writable locations.

Recommended fix:

- Keep the feature if needed, but document it as a trusted-config capability.
- Consider UI warnings for absolute paths.
- If configuration import is added later, either reject absolute log masks during import or require explicit confirmation.

Fix status, 2026-07-12:

- Documented absolute log masks and invalid TLS certificate acceptance as trusted local configuration in `docs/security/trusted-config.md`.
- The documented policy is that normal chat traffic must not mutate these settings, defaults/imports/migrations must not silently enable them, and future config import code should reject or explicitly confirm these trusted-config capabilities.

## Reviewed And Downgraded Surfaces

DCC receive paths were reviewed because they are remotely influenced:

- `src/common/dcc.c:2597` - incoming `DCC SEND` uses `file_part(word[6])` before calling `dcc_add_file()`.
- `src/common/dcc.c:2419` through `src/common/dcc.c:2441` - the destination path appends the already-normalized offered filename under `prefs.hex_dcc_dir`.
- `src/common/dcc.c:680` through `src/common/dcc.c:698` - existing files are renamed with a numeric suffix before create.

No direct DCC path traversal finding is recorded in this stage because the offered filename is reduced to its basename before destination construction. Residual hardening would still be useful: reject control characters and platform-reserved names in received filenames, and keep DCC auto-receive disabled by default unless explicitly chosen.

TLS bypass controls were reviewed:

- `src/common/outbound.c:3396`, `src/common/outbound.c:3615`, and `src/common/outbound.c:3744` - `-ssl-noverify` is an explicit command option.
- `src/common/servlist.c:529` - server-list setting can persist accepting invalid certificates.
- `src/common/server.c:657` through `src/common/server.c:675` - certificate and hostname verification failures are only bypassed when `accept_invalid_cert` is set.
- `src/common/ssl.c:324` through `src/common/ssl.c:337` - OpenSSL verify paths and peer verification are enabled.
- `src/common/ssl.c:566` - custom hostname verification is called after chain verification.

No silent TLS verification bypass was found. The residual risk is product-policy based: keep invalid-certificate acceptance visibly labeled as insecure and avoid enabling it through defaults, imports, or migration code.

User-controlled markup paths were reviewed:

- `src/common/outbound.c:2181` - `/GUI MSGBOX` passes text with `FE_MSG_MARKUP`.
- `src/fe-gtk/fe-gtk.c:1090` and `src/fe-gtk/fe-gtk.c:1091` - marked messages use `gtk_message_dialog_set_markup()`.
- `src/fe-gtk/setup.c:2344` - another markup message uses a fixed translated string.

No remotely triggered markup injection path was identified in this pass. `/GUI MSGBOX` is a local command/plugin/script surface, so it should be treated as trusted UI markup rather than network-rendered text.

## Targeted Fix Priority

Suggested ordering:

1. Add archive extraction containment and tests for GTK3 theme import. Status: closed. The legacy GTK3 theme-package importer and its external-helper fallback were deleted in Stage 9; the supported `.hct` reader does not extract an archive tree and retains bounded ZIP-fixture coverage.
2. Fix or disable-by-default the Exec plugin command construction.
3. Constrain bare-name DLL loading for Enchant and Perl. Status: Enchant now
   uses app-local absolute loading first; the unbuilt Perl integration and its
   bare-name DLL loading code were retired in repository cleanup Stage 2.
4. Canonicalize add-on GUI containment and avoid `/LOAD` command-string interpolation. Status: addressed for Add-ons GUI load/unload/reload paths on 2026-07-12; script runtimes still receive command-hook requests after path validation.
5. Document trusted-config behavior for absolute log masks and invalid TLS certificate acceptance. Status: documented in `docs/security/trusted-config.md` on 2026-07-12.

## Local Scanner Follow-Up

Date: 2026-07-12

This follow-up revisits scanner availability after the Visual Studio developer environment, CodeQL CLI, and Semgrep installation were checked locally.

Updated availability:

- MSVC `cl.exe` is installed and available from the Visual Studio Build Tools environment.
- MSBuild is installed at `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe`.
- CodeQL CLI is installed at `C:\codeql\codeql.exe`, version 2.26.0.
- Semgrep is installed as Python package `semgrep` 1.169.0, with entry points under `C:\Users\Barry\AppData\Local\Python\pythoncore-3.14-64\Scripts`.

MSVC `/analyze` result:

- A full-solution `/analyze` run against `win32\zoitechat.sln` did not complete.
- The default build root `C:\zoitechat-build` could not be written by the scanner shell because existing `.tlog` files returned access denied.
- Redirecting `ZoiteChatBuild` into a workspace-local `.scan-msvc` directory allowed project-level analysis to start.
- `plugins\exec\exec.vcxproj` completed with `/analyze` enabled and reported 0 warnings and 0 errors.
- `src\common\common.vcxproj` reached `/analyze` compilation after setting process-scope `PSExecutionPolicyPreference=Bypass` and `SolutionDir`, but did not complete. It produced 21 compiler warnings before failing on generated data/PDB issues:
  - `src/common/url.c:30` - missing generated `public_suffix_data.h` after `gen-public-suffix.py` reported `unable to load public suffix list`.
  - `src/common/util.c:1421` and `src/common/gtk3-theme-service.c:1413` - obsolete `common.pdb` format errors in the redirected scan output.
  - Warnings were mostly `C4244` size/socket/integer conversion warnings, plus `C4018` and `C4090`.

CodeQL result:

- CodeQL CLI 2.26.0 runs locally.
- A C/C++ `--build-mode=none` database create indexed repository files but failed finalization with `no source code seen during build`.
- A traced CodeQL database create around the successfully building `plugins\exec\exec.vcxproj` also failed finalization with `no source code seen during build`.
- No CodeQL SARIF/CSV findings were produced locally in this follow-up.

Semgrep result:

- Native `semgrep.exe` fails before scan startup with `Failed to create system store X509 authenticator: ca_certs_iter_on_anchors: CertOpenSystemStore returned NULL`.
- Python wrapper `pysemgrep.exe` reports version 1.169.0 when workspace-local config/log paths are supplied, but local scan attempts failed before producing findings. The observed failures were `semgrep-core rule validation failed`, `Failed to obtain target files from semgrep-core`, and Windows access-denied cleanup errors for Semgrep temporary directories.
- No Semgrep findings were produced locally in this follow-up.

Gitleaks result:

- Gitleaks 8.30.1 was installed and run with `gitleaks detect --source C:\fabulor-master --no-banner`.
- It scanned 63 commits and approximately 12.13 MB.
- It reported 1 finding:
  - Rule: `generic-api-key`
  - File: `src/fe-gtk/chanlist.c`
  - Line: 461
  - Commit: `746467e7d2516d6dfa25c9407c65b9d1ddf79e56`
- Manual review classified this as a false positive. The matched symbol is `collation_key`, a GLib collation/sort key for channel names, not an API key or secret.

Conclusion:

The original audit limitation should be refined rather than removed. MSVC and CodeQL are installed, Gitleaks produced a usable history-aware secret scan, and Semgrep was installed but not usable in this environment. Only a partial MSVC `/analyze` pass produced usable compiler-analysis results. The successful Exec-plugin `/analyze` run does not weaken the earlier manual finding: the command construction in `plugins/exec/exec.c` remains a security issue even though MSVC did not warn on that project.

Updated scanner follow-up:

- Run MSVC `/analyze` from a clean normal-user developer shell after resolving `C:\zoitechat-build` write access or standardizing a workspace-local build root.
- Fix or make deterministic generation of `public_suffix_data.h` for redirected/native scanner builds.
- Investigate why CodeQL's Windows C/C++ tracer is not seeing the local MSBuild/CL invocations, or rely on the existing GitHub Actions CodeQL jobs for CodeQL coverage.
- Investigate Semgrep's Windows cert-store startup failure and Python wrapper temp-directory cleanup failure before treating Semgrep as locally usable.

## Compiler Warning Remediation Follow-Up

Date: 2026-07-14

The warnings observed during the earlier MSVC analysis and release builds were
reviewed rather than suppressed. The cleanup adds checked narrowing at the Lua
API, socket, text-conversion, IRC-mode, nickname-completion, and timestamp
boundaries; uses the GLib portability API for certificate permissions; and
removes the const-qualification mismatch in the OpenSSL common-name fallback.

The SCRAM review also found and fixed two adjacent error-path issues: invalid
server nonces now release their temporary values, and PBKDF2 input sizes and
failure results are validated before authentication continues.

Validation:

- A full MSVC x64 Release rebuild of `win32\zoitechat.sln` completed with 0
  warnings and 0 errors.
- All 15 manifest JSON and path-policy tests passed during that rebuild.
- `git diff --check` passed.

This removes the concrete compiler-warning set recorded by the local scanner
follow-up. It does not replace the still-outstanding full MSVC `/analyze` or
CodeQL tracing work.

## Python Manifest Interpreter Isolation Follow-Up

Date: 2026-07-14

Python manifest plugins no longer execute beside trusted simple add-ons in the
legacy CFFI interpreter. The authenticated manifest load path creates one Python
3.14 `concurrent.interpreters.Interpreter` per plugin and imports a pure-Python
`zoitechat` proxy before executing the entrypoint. Each plugin therefore owns its
globals, imports, `sys.modules` state, callback functions, userdata, and shutdown
lifetime.

The CFFI extension and native plugin handle remain confined to the trusted main
interpreter. Manifest API calls and callback events cross the interpreter
boundary as bounded JSON-compatible values. The main interpreter rechecks
capabilities while applying log, message, and callback-registration operations,
and removes all proxy hooks before closing the subinterpreter. Simple Python
add-ons retain their existing compatibility behavior.

Validation:

- Nine Python host capability/boundary tests pass, including an integration test
  through the main-interpreter proxy and maintained manifest sample.
- Seven new Python 3.14 tests create multiple live subinterpreters and verify
  independent module/callback state, capability-failure isolation, legacy CFFI
  module blocking, duplicate callback rejection, and the request, entrypoint,
  and response size limits.
- A full MSVC x64 Release rebuild regenerated `hcpython3.dll` with 0 warnings and
  0 errors and passed all 15 native manifest/path tests.
- The staged release payload contains `_fabulor_manifest.py` under `python`, and
  the rebuilt x64 MSI binding record confirms that the helper is packaged.
- The x64 MSI and bootstrapper rebuild completed with 0 errors. The existing
  empty GTK4 `lib\\gio` harvest and same-version ICE61 warnings remain unchanged.
- Installed-client validation exposed false "untrusted plugin directory"
  diagnostics for legacy plugin DLLs stored beside manifest directories. Root
  discovery now ignores ordinary non-directory entries before applying strict
  directory containment checks; symbolic links and directory reparse points
  still reach the rejecting validator. A native regression test covers the
  legacy-DLL and manifest-directory distinction. The updated installed client
  then loaded the Python greeter in its isolated interpreter, dispatched its
  first message callback alongside C#, and closed normally without teardown
  errors; the false legacy-DLL diagnostics were absent.

This is interpreter isolation, not an operating-system sandbox. A Python plugin
can still use standard-library and operating-system facilities available to the
Fabulor process. The manifest capability model controls cooperative access to
Fabulor host operations only.
