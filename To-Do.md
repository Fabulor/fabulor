<!-- Fabulor Plugin & Installer Roadmap -->
# Fabulor Plugin & Installer Roadmap

## Current Direction

- [x] Confirm Inno Setup is retired.
- [x] Confirm the long-term installer surface is `installer\` only.
- [x] Confirm the release end state is exactly two installer artefacts from WiX v4:
  - [x] an MSI
  - [x] a bootstrapper `.exe`.
- [x] Treat the WiX v4 installer as complete enough to shift the main effort to plugin/API modernisation.

## 1. WiX v4 Installer Completion

- [ ] Finalise installed layout under `ProgramFiles\Fabulor\`:
  - [ ] `fabulor.exe`
  - [ ] `Plugins\`
  - [ ] `Runtime\GTK4\`
  - [ ] `Runtime\Python314\`
  - [ ] `Runtime\Tcl\`
  - [ ] `Runtime\DotNet\`
  - [ ] `Config\`.
- [ ] Update WiX directory/component authoring to match that runtime layout exactly.
- [ ] After the repo security review, remaining UI tweaks, and GTK4 migration are complete, audit and trim the installed `Runtime\` payload:
  - [ ] Keep the GTK4 runtime broad enough for the GTK4-compliant Fabulor target.
  - [ ] Remove build-time/development-only GTK data from the installed payload where it is not needed at runtime.
  - [ ] Split the Tcl payload into core scripting support and optional extra packages.
  - [ ] Keep Python314 as the bundled scripting runtime unless the plugin contract changes.
- [ ] Implement WiX v4 components for:
  - [ ] core executable
  - [ ] plugin folders
  - [ ] GTK runtime DLLs and data
  - [ ] Python embedded runtime
  - [ ] Tcl runtime
  - [ ] .NET plugin host.
- [ ] Define and implement feature behaviour for:
  - [ ] Installed mode
  - [ ] Portable mode.
- [ ] Keep registry entries scoped to Installed mode only.
- [ ] Integrate Fabulor's own updater (no Windows Update / MSIX).
- [ ] Define cutover criteria for replacing the legacy `win32\` packaging path in CI and releases.
- [ ] Switch CI/release packaging from the retired Inno path to WiX-generated artefacts once the cutover criteria are met.

## 2. Core ABI & Binding Layers

- [x] Define `ZoiteChatAPI`/`FabulorAPI` C struct and `UserInfo` in a header.
- [x] Implement C# binding:
  - [x] Map `ZoiteChatAPI` to delegates.
  - [x] Implement `ZoiteChatContext`.
  - [x] Define `IZoiteChatPlugin` with `Init(ZoiteChatContext ctx)`.
- [x] Implement Python 3.14 binding:
  - [x] Embedded Python initialisation.
  - [x] `zoitechat` module exposing core functions.
  - [x] Maintain an embedded binding path fed from the shared host/runtime surface.
- [x] Implement Tcl 8.6 binding:
  - [x] Embedded Tcl initialisation.
  - [x] `zoitechat::*` commands in a namespace.
  - [x] Feed the Tcl command surface from the shared host/runtime API.

## 3. Manifest & Plugin Layout

- [x] Treat the documented `plugin.json` schema as the contract unless a deliberate revision is approved:
  - [x] Fields: id, name, version, language, entrypoint,
        requires_api_version, dependencies, capabilities,
        description, author, homepage.
- [x] Enforce per-plugin folder layout under discovered plugin roots:
  - [x] `plugins/<plugin-id>/plugin.json`
  - [x] `plugins/<plugin-id>/<entrypoint>`.
- [ ] Decide and document how legacy built-in C plugins coexist with or migrate to the new manifest-driven model.

## 4. Unified Loader & Dependency Resolver

- [x] Implement manifest discovery under `plugins\*\plugin.json`.
- [x] Implement JSON parsing into `PluginManifest`.
- [x] Implement validation:
  - [x] required fields
  - [x] supported language
  - [x] entrypoint exists
  - [x] API version compatibility
  - [x] dependency existence.
- [x] Implement Kahn-based dependency resolver:
  - [x] build graph
  - [x] compute in-degrees
  - [x] produce sorted load order
  - [x] detect and log cycles.
- [x] Implement `IPluginLoader` interface and factory:
  - [x] CSharpLoader
  - [x] PythonLoader
  - [x] TclLoader.

## 5. Callback/Event System

- [x] Define `CallbackEntry` and global registry.
- [x] Implement registration APIs:
  - [x] C# `RegisterCallback`
  - [x] Python `zoitechat.register_callback`
  - [x] Tcl `zoitechat::register_callback`.
- [x] Implement `fire_event(event, data)` and per-language dispatch:
  - [x] `dispatch_csharp`
  - [x] `dispatch_python`
  - [x] `dispatch_tcl`.
- [x] Ensure all callbacks run on the main thread.
- [x] Add robust error logging and isolation.

## 6. Security Model

- [x] Implement manifest validation and skip invalid plugins.
- [ ] Enforce per-plugin interpreter isolation across every language runtime path.
- [ ] Restrict core access to `ZoiteChatAPI` only.
- [x] Add safe mode flag to disable third-party plugins.
- [x] Implement optional plugin blacklist.
- [x] Add logging for all plugin lifecycle events.

## Current Runtime Gaps To Close Next

- [ ] Decide and document the coexistence/migration plan for legacy built-in C plugins versus manifest-driven plugins.
- [ ] Decide whether declared `capabilities` remain advisory/diagnostic metadata or become an enforced policy surface.
- [ ] Tighten the Python runtime path so the modern manifest contract and the legacy scripting surface are clearly separated where required.
- [ ] Add broader shared API helpers beyond the current message/log/user-count/user-info surface.

## 7. Documentation & Developer Guides

- [x] Write C# plugin authoring guide.
- [x] Write Python plugin authoring guide.
- [x] Write Tcl plugin authoring guide.
- [x] Document `plugin.json` schema and compatibility rules.
- [x] Document safe mode and plugin troubleshooting.

## 8. Foundry Local Prompt Library

- [x] Create a `prompts/` folder in the repo.
- [x] Add prompts for:
  - [x] C ABI + C# binding.
  - [x] Python binding.
  - [x] Tcl binding.
  - [x] unified loader + Kahn resolver.
  - [x] callback/event system.
  - [x] WiX v4 installer skeleton.
- [x] Keep prompts small, focused, and grounded with:
  - [x] `Fabulor is currently located at C:\zoitechat-master and should move to C:\fabulor-master when the workspace folder is renamed.` at the top.
