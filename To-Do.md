<!-- Fabulor Plugin & Installer Roadmap -->
# Fabulor Plugin & Installer Roadmap

## Current Direction

- [x] Confirm Inno Setup is retired.
- [x] Confirm the long-term installer surface is `installer\` only.
- [x] Confirm the release end state is exactly two installer artefacts from WiX v4:
  - [x] an MSI
  - [x] a bootstrapper `.exe`.
- [ ] Finish the WiX v4 installer before starting the deeper plugin ABI and loader rework.

## 1. WiX v4 Installer Completion

- [ ] Finalise installed layout under `ProgramFiles\Fabulor\`:
  - [ ] `fabulor.exe`
  - [ ] `Plugins\`
  - [ ] `Runtime\GTK4\`
  - [ ] `Runtime\Python312\`
  - [ ] `Runtime\Tcl\`
  - [ ] `Runtime\DotNet\`
  - [ ] `Config\`.
- [ ] Update WiX directory/component authoring to match that runtime layout exactly.
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

- [ ] Define `ZoiteChatAPI` C struct and `UserInfo` in a header.
- [ ] Implement C# binding:
  - [ ] Map `ZoiteChatAPI` to delegates.
  - [ ] Implement `ZoiteChatContext`.
  - [ ] Define `IZoiteChatPlugin` with `Init(ZoiteChatContext ctx)`.
- [ ] Implement Python 3.12 binding:
  - [ ] Embedded Python initialisation.
  - [ ] `zoitechat` module exposing core functions.
  - [ ] `init_python_binding(ZoiteChatAPI* api)`.
- [ ] Implement Tcl 8.6 binding:
  - [ ] Embedded Tcl initialisation.
  - [ ] `zoitechat::*` commands in a namespace.
  - [ ] `init_tcl_binding(ZoiteChatAPI* api, Tcl_Interp* interp)`.

## 3. Manifest & Plugin Layout

- [ ] Treat the documented `plugin.json` schema as the contract unless a deliberate revision is approved:
  - [ ] Fields: id, name, version, language, entrypoint,
        requires_api_version, dependencies, capabilities,
        description, author, homepage.
- [ ] Enforce per-plugin folder layout:
  - [ ] `plugins/<plugin-id>/plugin.json`
  - [ ] `plugins/<plugin-id>/<entrypoint>`.
- [ ] Decide and document how legacy built-in C plugins coexist with or migrate to the new manifest-driven model.

## 4. Unified Loader & Dependency Resolver

- [ ] Implement manifest discovery under `plugins\*\plugin.json`.
- [ ] Implement JSON parsing into `PluginManifest`.
- [ ] Implement validation:
  - [ ] required fields
  - [ ] supported language
  - [ ] entrypoint exists
  - [ ] API version compatibility
  - [ ] dependency existence.
- [ ] Implement Kahn-based dependency resolver:
  - [ ] build graph
  - [ ] compute in-degrees
  - [ ] produce sorted load order
  - [ ] detect and log cycles.
- [ ] Implement `IPluginLoader` interface and factory:
  - [ ] CSharpLoader
  - [ ] PythonLoader
  - [ ] TclLoader.

## 5. Callback/Event System

- [ ] Define `CallbackEntry` and global registry.
- [ ] Implement registration APIs:
  - [ ] C# `RegisterCallback`
  - [ ] Python `zoitechat.register_callback`
  - [ ] Tcl `zoitechat::register_callback`.
- [ ] Implement `fire_event(event, data)` and per-language dispatch:
  - [ ] `dispatch_csharp`
  - [ ] `dispatch_python`
  - [ ] `dispatch_tcl`.
- [ ] Ensure all callbacks run on the main thread.
- [ ] Add robust error logging and isolation.

## 6. Security Model

- [ ] Implement manifest validation and skip invalid plugins.
- [ ] Enforce per-plugin interpreter isolation.
- [ ] Restrict core access to `ZoiteChatAPI` only.
- [ ] Add safe mode flag to disable third-party plugins.
- [ ] Implement optional plugin blacklist.
- [ ] Add logging for all plugin lifecycle events.

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
