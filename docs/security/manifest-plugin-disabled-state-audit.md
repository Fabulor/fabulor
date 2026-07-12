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

The manifest host is only reachable when `FABULOR_ENABLE_MANIFEST_PLUGINS=1` is present in the process environment and the normal plugin autoload path is not skipped by `--no-plugins`.

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

Manifest runtime search-path mutations are located only in manifest loader code:

- `src/common/fabulor-plugin-host.c:782` - Tcl loader prepends runtime `bin` to `PATH`
- `src/common/fabulor-plugin-host.c:831` - Tcl loader sets `TCL_LIBRARY`
- `src/common/fabulor-plugin-host.c:834` - Tcl loader loads `tcl86t.dll`
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
- Review Tcl `PATH` mutation and absolute DLL loading strategy.
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

The manifest host is useful as a development scaffold, but it is not ready for third-party plugins. The largest blockers are unchecked path traversal in manifest entrypoints, regex-based manifest parsing with no strict schema or size limits, process-global runtime search-path mutations, environment/cwd runtime root overrides, no enforced capability policy, and uneven isolation across languages.

## Root Discovery

Manifest autoload discovers two roots:

- `src/common/plugin.c:483` - bundled root is `plugin_get_libdir()/Plugins`
- `src/common/plugin.c:484` - user root is `get_xdir()/plugins`
- `src/common/plugin.c:487` and `src/common/plugin.c:494` - both roots are passed to `fabulor_plugin_catalog_discover_root()`

The bundled root is indirectly environment-influenced because `plugin_get_libdir()` honors `ZOITECHAT_LIBDIR`:

- `src/common/plugin.c:861` - `plugin_get_libdir()`
- `src/common/plugin.c:865` - reads `ZOITECHAT_LIBDIR`

Discovery itself enumerates direct child folders and looks for `plugin.json`:

- `src/common/fabulor-plugin-host.c:425` - opens the root with `g_dir_open()`
- `src/common/fabulor-plugin-host.c:442` - appends `plugin.json` under each child
- `src/common/fabulor-plugin-host.c:444` - accepts a child directory with an existing manifest

Pre-enable issue: discovered roots and plugin child directories are not canonicalized, not checked against approved canonical roots, and not protected from symlink/reparse-point escapes. The environment-influenced bundled root is acceptable for developer testing but should not be a user-facing trust boundary.

Minimum fix: canonicalize approved roots and child plugin directories, reject paths outside those roots after canonicalization, decide whether `ZOITECHAT_LIBDIR` remains honored when manifest plugins are user-facing, and explicitly handle symlinks/reparse points on Windows.

## Manifest Parsing And Validation

The current parser is regex-based:

- `src/common/fabulor-plugin-host.c:220` - string field extraction
- `src/common/fabulor-plugin-host.c:258` - unsigned integer extraction
- `src/common/fabulor-plugin-host.c:305` - string-array extraction
- `src/common/fabulor-plugin-host.c:370` - reads the full manifest into memory
- `src/common/fabulor-plugin-host.c:375` - only rejects empty manifests
- `src/common/fabulor-plugin-host.c:386` through `src/common/fabulor-plugin-host.c:396` - extracts known fields

Validation checks required fields, supported language, entrypoint existence, API version, and dependency discovery:

- `src/common/fabulor-plugin-host.c:541` through `src/common/fabulor-plugin-host.c:598` - required field checks
- `src/common/fabulor-plugin-host.c:559` - language must be known
- `src/common/fabulor-plugin-host.c:570` - entrypoint path must exist
- `src/common/fabulor-plugin-host.c:576` - required API version must not exceed host version
- `src/common/fabulor-plugin-host.c:602` through `src/common/fabulor-plugin-host.c:607` - dependencies must be discovered

Pre-enable issues:

- No strict JSON parser is used, so malformed JSON can be accepted if the regexes find matching field fragments.
- No schema rejection exists for wrong field types beyond what the regexes happen to ignore.
- No manifest size limit exists before `g_file_get_contents()`.
- Arrays are not type-strict; the parser extracts quoted strings from the matched array body and ignores other values.
- Unknown fields are ignored, which is acceptable only if explicitly documented.
- If any enabled manifest has validation errors, `fabulor_plugin_catalog_resolve_load_order()` aborts all manifest loading rather than isolating the bad plugin.

Minimum fix: replace regex parsing with a real JSON parser, enforce a documented schema and maximum manifest size, reject malformed JSON and wrong field types deterministically, and decide whether one invalid plugin should block all manifest plugins or only itself and dependents.

## Entrypoint Resolution

Entrypoints are built as a simple child path and checked only for existence:

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

## Runtime Loading

The Tcl runtime resolver accepts several roots and mutates process-global environment:

- `src/common/fabulor-plugin-host.c:725` - accepts `FABULOR_TCL_RUNTIME_ROOT`
- `src/common/fabulor-plugin-host.c:731` and `src/common/fabulor-plugin-host.c:732` - accepts `Runtime/Tcl` under current working directory
- `src/common/fabulor-plugin-host.c:740` - accepts a path relative to `manifest_plugin_get_libdir()`
- `src/common/fabulor-plugin-host.c:750` - accepts `Runtime/Tcl` under the executable directory
- `src/common/fabulor-plugin-host.c:782` - prepends Tcl `bin` to `PATH`
- `src/common/fabulor-plugin-host.c:831` - sets `TCL_LIBRARY`
- `src/common/fabulor-plugin-host.c:834` - loads `tcl86t.dll`

The C# runtime resolver accepts environment and current-working-directory roots:

- `src/common/fabulor-plugin-host.c:1476` - accepts `FABULOR_DOTNET_ROOT`
- `src/common/fabulor-plugin-host.c:1482` - accepts `DOTNET_ROOT`
- `src/common/fabulor-plugin-host.c:1488` and `src/common/fabulor-plugin-host.c:1489` - accepts `Runtime/DotNet` under current working directory
- `src/common/fabulor-plugin-host.c:1500` - accepts `Runtime/DotNet` under executable directory
- `src/common/fabulor-plugin-host.c:1526` - accepts `FABULOR_CSHARP_BRIDGE_ROOT`
- `src/common/fabulor-plugin-host.c:1534` through `src/common/fabulor-plugin-host.c:1552` - accepts current-working-directory bridge candidates
- `src/common/fabulor-plugin-host.c:1728` - loads `hostfxr.dll` from the selected root

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

Pre-enable issues:

- Tcl changes process-wide `PATH` and `TCL_LIBRARY`; this can affect later DLL/runtime resolution outside the plugin host.
- Tcl and C# both trust developer-style environment and current-working-directory runtime roots.
- Python does not have manifest-specific interpreter isolation; it uses the legacy Python plugin's global runtime and command surface. The path boundary is now narrower: simple add-ons resolve under the profile `addons` directory, and manifest entrypoints are accepted only from manifest plugin roots.
- Python callback integration is not the shared manifest registry. `plugins/python/_zoitechat.py:304` implements callback helpers by mapping to legacy hooks.
- C# dependency resolution is per assembly-load context, but native dependency policy and allowed dependency locations are not constrained by manifest capabilities.

Minimum fix: restrict runtime roots to installed/bundled roots by default, move developer overrides behind a separate development flag or diagnostic build path, avoid global `PATH` mutation for Tcl if possible, define Python manifest interpreter isolation separately from legacy scripting, and define native dependency loading policy for C#.

Fix status, 2026-07-12:

- The Python runtime now canonicalizes requested script paths before loading.
- Relative Python load requests are treated as simple add-ons and resolve under the profile `addons` directory, preferring the documented `addons\<name>\<name>.py` layout while keeping legacy flat `addons\*.py` as a compatibility fallback.
- Absolute Python load requests are rejected unless the resolved `.py` file is under the profile `addons` directory, the profile `plugins` manifest root, or the bundled `Plugins` manifest root.
- This reduces accidental crossing between the legacy scripting surface and manifest plugin roots, but it does not provide per-manifest Python interpreter isolation or move Python callbacks into the shared manifest callback registry.

## Callback And Event System

The registry is per manifest host and records callback entries by event name:

- `src/common/fabulor-plugin-host.c:2490` - creates the callback registry
- `src/common/fabulor-plugin-host.c:2495` - stores event entries in a hash table
- `src/common/fabulor-plugin-host.c:2524` - callback registration entry point
- `src/common/fabulor-plugin-host.c:2541` - rejects unknown plugin ids
- `src/common/fabulor-plugin-host.c:2548` - rejects language mismatches
- `src/common/fabulor-plugin-host.c:2587` - dispatches callbacks and logs per-callback failures
- `src/common/fabulor-plugin-host.c:2646` - fire-event entry point
- `src/common/fabulor-plugin-host.c:2673` - non-main-thread dispatch is queued with `g_main_context_invoke_full()`
- `src/common/fabulor-plugin-host.c:2684` - shutdown resets C# and Tcl runtimes

Pre-enable issues:

- Event names and handler names are not allowlisted or length-limited.
- Duplicate callback registrations are not deduplicated or capped.
- Queued dispatch stores a raw registry pointer and does not refcount or otherwise pin the registry until the main-context callback runs.
- Callback cleanup is mostly whole-host cleanup. There is no per-plugin unload/removal path for manifest callback entries.
- Python manifest callbacks do not use this registry, so callback semantics are inconsistent across languages.

Minimum fix: define allowed event names and maximum lengths/counts, add duplicate policy, make queued dispatch lifetime safe, add per-plugin callback cleanup, and either integrate Python with the shared registry or document Python as legacy-hook based until the manifest Python host is redesigned.

## Capabilities

Capabilities are currently advisory metadata only:

- `src/common/fabulor-plugin-host.c:393` - parses `capabilities`
- `src/common/fabulor-plugin-host.c:2112` - allocates the capabilities array
- `src/common/fabulor-plugin-host.c:2130` - frees the capabilities array
- `docs/plugins/plugin-schema-and-troubleshooting.md:136` - documents that capabilities are recorded but not enforced

Pre-enable issue: no runtime API call, callback registration, file/runtime access, or language loader decision is gated by manifest `capabilities`.

Decision for now: treat `capabilities` as advisory only and do not expose third-party manifest plugins as policy-enforced. Before user-facing enablement, either remove policy language from user documentation or implement enforcement for at least `messages.write`, `session.read`, and `events.*`.

## Minimum Pre-Enable Fix List

Before `FABULOR_ENABLE_MANIFEST_PLUGINS=1` becomes user-facing, require at least:

- Canonical root containment for discovered roots, plugin directories, manifest paths, and entrypoints.
- Rejection of absolute entrypoints, `..`, symlink/reparse escapes, unreadable files, and language/extension mismatches.
- Strict JSON parsing with schema/type validation and manifest size limits.
- Per-plugin error isolation policy that does not let one bad manifest unexpectedly block unrelated plugins unless that is deliberately documented.
- Runtime-root policy that removes normal-user reliance on environment variables and current working directory.
- Removal or containment of Tcl process-wide `PATH` mutation.
- A Python manifest host design that is separate from the legacy global Python plugin, or a clear decision that Python manifests remain disabled.
- Callback event allowlists, length/count limits, duplicate policy, safe queued-dispatch lifetime, and per-plugin callback cleanup.
- A capability policy decision: explicitly advisory-only with no security claims, or enforced gates for every exposed API and event surface.

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
- `installer/Components/GTK4.wxs` packages GTK4 DLLs and broad `etc`, `lib`, and `share` trees.
- `installer/Components/Core.wxs` explicitly lists the core native payload DLLs and executables.
- `installer/Components/Plugins.wxs` explicitly lists built-in plugin DLLs.

Provenance issue:

- The runtime payload is not backed by a checked-in bill of materials with source URL, version, expected hash, and license/provenance status for every harvested binary.
- The workflows download several third-party payloads by URL, but the reviewed workflow snippets do not verify expected hashes after download.
- Broad wildcard harvesting means unintended files can enter the installer if runtime directories contain extra files.

Minimum follow-up:

- Create a runtime payload bill of materials covering GTK, Python, Tcl, .NET, WinSparkle, Perl, MSYS2 packages, and other bundled binaries.
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
- Windows builds now use the same libarchive-contained extractor when `HAVE_LIBARCHIVE` is set. If libarchive is unavailable on Windows, the existing external `powershell`/`tar.exe` fallback remains a residual medium-risk path because it cannot apply the in-process entry policy before extraction.
- Added non-Windows regression coverage for `..` traversal with partial-extraction cleanup, absolute archive paths, symlink entries, and hardlink entries in `src/common/tests/test-gtk3-theme-service.c`.
- Verification: `git diff --check` passed; the focused WSL GTK3 theme-service test binary compiled with GLib/GIO/libarchive and passed all 18 tests; `src\common\common.vcxproj` built successfully with 15 pre-existing conversion warnings and 0 errors; `src\fe-gtk\fe-gtk.vcxproj` built and linked successfully with 1 pre-existing const-qualifier warning and 0 errors.

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
- `plugins/perl/perl.c:1440` - Perl is loaded with `LoadLibraryA(PERL_DLL)`.
- `plugins/perl/perl.c:1452` - fallback probes `LoadLibraryA("perl56.dll")`.

Risk:

Some of these paths are deliberate extension points, but bare DLL loading relies on process DLL search behavior and PATH contents. That is a weak boundary for installed applications, especially near plugin and scripting surfaces.

Recommended fix:

- Prefer absolute paths rooted in the installed application directory or a trusted runtime directory.
- On Windows, use `SetDefaultDllDirectories()` and `AddDllDirectory()`/`LoadLibraryEx()` with constrained search flags where compatible.
- Keep user add-on loading under the explicit `addons` trust model, but avoid using general DLL search order for dependencies such as Enchant and Perl.

Fix status, 2026-07-12:

- Enchant loading in `src/fe-gtk/sexy-spell-entry.c` now prefers an absolute module path under the application installation directory for `libenchant-2-2.dll`, `libenchant-2.dll`, and the temporary legacy `libenchant.dll` fallback. This matches the Enchant 2.8.19 rollout while keeping the old Enchant payload as an app-local fallback only.
- The legacy Perl plugin remains a legacy source/build surface and is not part of the documented Fabulor plugin model, which is C#, Python, and Tcl. The current WiX plugin payload does not package `hcperl.dll`.
- Modern manifest Tcl and .NET runtime loading already uses absolute runtime paths under the executable/runtime root. Python manifest loading now rejects command-unsafe entrypoint paths before invoking the existing script runtime hook.

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
- The GUI file filter now matches the documented Fabulor add-on model: native plugin DLLs plus `.py`, `.tcl`, and `.cs` add-ons.

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

1. Add archive extraction containment and tests for GTK3 theme import. Status: addressed for libarchive extraction on 2026-07-12; Windows external-helper fallback remains a documented residual if libarchive is unavailable at build time.
2. Fix or disable-by-default the Exec plugin command construction.
3. Constrain bare-name DLL loading for Enchant and Perl. Status: Enchant now uses app-local absolute loading first; Perl is documented as legacy/not packaged in the Fabulor C#/Python/Tcl plugin model.
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
