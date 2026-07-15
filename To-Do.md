<!-- Fabulor Plugin & Installer Roadmap -->
# Fabulor Plugin & Installer Roadmap

## Current Direction

- [x] Confirm Inno Setup is retired.
- [x] Confirm the long-term installer surface is `installer\` only.
- [x] Confirm the release end state is exactly two installer artefacts from WiX v4:
  - [x] an MSI
  - [x] a bootstrapper `.exe`.
- [x] Treat the WiX v4 installer as complete enough to shift the main effort to plugin/API modernisation.

## Recently Completed QoL/UI Work

- [x] Add configurable editbox history with max-history and save/restore options.
- [x] Add optional channel scroll-to-bottom overlay and fix its icon rendering.
- [x] Fix installed logging path handling so server logs open under the expected network log folders.
- [x] Upgrade spell checking to Enchant 2 + WinSpell for live Windows spelling suggestions.
- [x] Replace the GTK built-in emoji chooser path with a Fabulor-owned emoji picker.
- [x] Bundle Noto Color Emoji and generated flag PNG assets for Windows installs.
- [x] Render selected country flags as real images in channel/server text output.
- [x] Convert bundled flag images to transparent-background PNGs.
- [x] Replace the bundled country flags with the flat PNG set and align the picker list with its 253 two-letter assets.
- [x] Add visible two-letter labels to flag picker buttons to disambiguate lookalike flags such as `IE`/`CI`.
- [x] Enlarge non-flag emoji picker glyphs while leaving the flag tab compact.
- [x] Build emoji picker categories lazily and reuse the popover so opening it does not synchronously rebuild every page and decode every flag image.

## Follow-Up QoL/UI Items

- [ ] Decide whether the editbox should keep showing Windows regional-indicator labels (`FR`, `IE`) or receive a deeper custom inline-image rendering pass.
- [ ] Consider country names or search/filter support in the flag picker if two-letter labels are not sufficient.

## Security Scanning Plan

- [ ] Plugin loader boundary review, pass 1: disabled-state audit.
  - [ ] Confirm manifest plugins are inert unless the explicit enable gate is set.
  - [ ] Confirm no config, environment, command-line, or installer default can accidentally enable the manifest plugin host.
  - [ ] Confirm disabled startup does not parse, load, execute, or trust files from user-controlled plugin/addon folders.
  - [ ] Confirm disabled plugin code does not alter DLL/runtime search paths in a way that affects normal client startup.
  - [ ] Record exact files/functions that implement the enable gate.
- [x] Plugin loader boundary review, pass 2: pre-enable design audit.
  - [x] Review and harden plugin root discovery so roots, direct plugin directories, and manifests remain inside approved canonical paths and reject symlink/reparse points.
  - [x] Review and harden manifest parsing and validation for required fields, strict types, bounded size, malformed JSON handling, and per-plugin error isolation.
  - [x] Review entrypoint resolution for path traversal, absolute paths, symlinks/reparse points, and extension/language mismatches.
  - [x] Review C#, Python, and Tcl runtime loading paths, DLL search order, and interpreter initialization boundaries.
  - [x] Review and harden callback/event registration lifetime, main-thread dispatch, cleanup, and failure isolation.
  - [x] Decide which declared manifest `capabilities` are advisory versus enforced before enabling third-party plugins.
  - [x] Define the minimum fixes required before `FABULOR_ENABLE_MANIFEST_PLUGINS=1` can become user-facing.
- [x] Manifest plugin opt-in preference rollout.
  - [x] Add an off-by-default persisted preference with an explicit trusted-code confirmation and restart notice.
  - [x] Keep `FABULOR_ENABLE_MANIFEST_PLUGINS=1` as a developer override and make `--no-plugins` take precedence over both enable paths.
  - [x] Add native policy tests for disabled defaults, preference/override enabling, invalid override values, and safe-mode precedence.
  - [x] Validate the preference, cancellation, persistence, normal restart, disabled restart, and safe-mode behavior in an installed upgrade.
- [ ] Repository security tool pass.
  - [ ] Inventory available local tools: MSVC `/analyze`, CodeQL CLI, Semgrep, gitleaks/trufflehog, dependency scanners, and GitHub Actions checks.
  - [ ] Run secret scanning across tracked files and review any hits.
  - [ ] Run static analysis suited to the C/C++ codebase and triage findings by exploitability.
  - [ ] Run dependency/vulnerability checks for .NET, Python, Node, and bundled binary/runtime payloads where applicable.
  - [ ] Review installer and runtime payload provenance, hashes, and packaging boundaries.
- [ ] Targeted high-risk code review.
  - [ ] File/path handling: logs, downloads, config, addon/plugin roots, installer paths, and portable-mode paths.
  - [ ] Network/TLS handling: certificate loading, proxy settings, reconnect paths, and unsafe fallbacks.
  - [ ] Process/library loading: plugin hosts, scripting runtimes, external commands, DLL search paths, and updater behavior.
  - [ ] User-controlled text rendering: URL detection, markup escaping, emoji/flag rendering, and theme/config parsing.

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
- [x] Decide and document how legacy built-in C plugins coexist with or migrate to the new manifest-driven model.

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
- [x] Allowlist and bound callback event/handler names and registration counts.
- [x] Reject duplicate registrations and cap queued dispatch work.
- [x] Pin queued registry lifetime, dispatch from snapshots, and discard work during shutdown.
- [x] Remove per-plugin callbacks after failed loads and before runtime teardown.

## 6. Security Model

- [x] Implement manifest validation and skip invalid plugins.
- [x] Enforce per-plugin interpreter isolation across every manifest language runtime path.
- [ ] Restrict core access to `ZoiteChatAPI` only.
- [x] Add safe mode flag to disable third-party plugins.
- [x] Implement optional plugin blacklist.
- [x] Add logging for all plugin lifecycle events.

## Current Runtime Gaps To Close Next

- [x] Decide and document the coexistence/migration plan for legacy built-in C plugins versus manifest-driven plugins.
- [x] Decide whether declared `capabilities` remain advisory/diagnostic metadata or become an enforced policy surface.
- [x] Tighten the Python runtime path so the modern manifest contract and the legacy scripting surface are clearly separated where required.
- [ ] Add broader shared API helpers beyond the current message/log/user-count/user-info surface.
- [ ] Rework the manifest plugin API before enabling it by default:
  - [x] Keep the manifest host disabled by default; allow explicit opt-in through the confirmed profile preference or `FABULOR_ENABLE_MANIFEST_PLUGINS=1` developer override.
  - [x] Investigate the connect/editbox crash in the current native API path using a debugger with symbols.
  - [x] Design a simpler add-on scripting flow that works from the user `addons` folder without manifest ceremony for small Tcl/Python aliases.
- [x] Finish the Enchant 2.8.19 spell-checker rollout after live validation:
  - [x] Confirm Enchant 2 + WinSpell is live in Fabulor and catches edit-box typos, verified with `Hllo thre piples` -> `Hello there peoples`.
  - [x] Revalidate URL paste, suggestions, add-to-dictionary, and persistence after installing the MSVC rebuild; the previous MinGW/MSVCRT payload caused reproducible heap corruption in `enchant_pwl_check`.
  - [x] Soak test the upgraded spell checker for a day or two before committing the rollout.
  - [x] Keep the Enchant 2 WinSpell build reproducible, including the temporary `bcp47.h` compatibility shim currently needed by the official source tarball. The pinned MSVC recipe is under `tools\enchant-msvc` and `tools\build-enchant-msvc.ps1`.
  - [x] Rebuild Enchant 2.8.19 core and WinSpell with MSVC/UCRT to match Fabulor's GTK/GLib payload and eliminate cross-CRT `FILE*` ownership.
  - [x] Add an isolated native smoke test covering checks, suggestions, add-to-personal, broker restart, and personal-word persistence.
  - [x] Remove the Enchant 1.6.1 fallback payload (`libenchant.dll`, `lib\enchant`) after Enchant 2 verification.
  - [x] Retire the legacy `src\libenchant_win8` provider after confirming upstream WinSpell covers the required Windows spell-check behaviour.
  - [x] Verify an in-place installer update preserves full spell-check functionality and removes the legacy Enchant DLL/provider/data payload.
  - [x] Verify spell checking, suggestions, "add to dictionary", and installer packaging on a clean install.

## GTK4 Migration

Detailed planning and evidence live in:

- [`docs/gtk4/migration-plan.md`](docs/gtk4/migration-plan.md)
- [`docs/gtk4/api-inventory.md`](docs/gtk4/api-inventory.md)
- [`docs/gtk4/runtime-packaging.md`](docs/gtk4/runtime-packaging.md)
- [`docs/gtk4/validation-log.md`](docs/gtk4/validation-log.md)

- [x] Establish the documentation baseline, source/API inventory, packaging inventory, and validation matrix.
- [x] Establish deterministic GTK4 build roots and compatibility helpers while keeping the GTK3 production build green.
  - [x] Pin and validate the Windows x64 GTK4 archive, root identity, versions, architecture, and required build files.
  - [x] Compile, link, and execute isolated MSVC and Meson probes against the same validated GTK4 root in CI.
  - [x] Introduce narrowly scoped compatibility helpers before converting production frontend modules.
- [ ] Convert widget ownership, layout, visibility, and lifecycle APIs.
  - [x] Route statically typed single-child window, scroller, frame, button, overlay, and popover assignments through the GTK3/GTK4 compatibility boundary.
  - [ ] Convert box packing and legacy child enumeration with explicit GTK4 expansion, alignment, ordering, and margin semantics.
    - [x] Convert start-only utility and preferences layouts with explicit expansion, fill, and directional padding semantics.
    - [ ] Convert mixed start/end layouts through per-surface child ordering reviews.
      - [x] Convert reviewed trailing action rows and channel-tab separators where expanding predecessors preserve placement.
      - [x] Convert shared prompt and notify dialog content with explicit trailing-pair order and typed content-area ownership.
      - [x] Convert reviewed main-window topic, transcript, user-list, emoji, search, reply, and input rows with exact append order.
      - [x] Convert dynamic main-window nickname, meter, user-list-button, and refreshed dialog-button ordering.
      - [x] Convert the Join Channel dialog and shared button-helper ownership without changing menu actions.
      - [x] Convert channel-list controls and shared operational-list scroller ownership without changing models or events.
      - [ ] Convert remaining menu-item and server-list model-surface ordering with their dedicated stages.
  - [ ] Replace recursive visibility and generic widget destruction with surface-specific GTK4 lifecycle handling.
    - [x] Convert completed utility-window/dialog trees and statically typed window destruction without changing menu or conditional-child behaviour.
    - [x] Convert shared prompt dialogs, exact response callbacks, frontend session windows, and concrete main-window/dialog lifetimes.
    - [x] Convert box-owned dynamic main-window nickname, progress, meter, and dialog-button child lifecycles.
    - [x] Convert the response-driven Join Channel dialog reveal and destruction lifecycle.
    - [ ] Convert remaining menu, unparented-widget, GTK3-test, and non-window child lifecycles in their owning stages.
- [ ] Convert actions, menus, dialogs, and file-selection flows.
  - [x] Establish one canonical action-identity registry for main-menu accelerators and configurable shortcut dispatch.
  - [x] Bind canonical commands and state to GTK4 actions and menu models without changing IRC command behaviour.
    - [x] Bind all 19 stateless canonical commands through per-menu `GSimpleActionGroup` ownership.
    - [x] Convert menu-bar, user-list, and fullscreen window-view state synchronization.
    - [x] Convert away state and connection sensitivity as a session/server action boundary.
    - [x] Convert Channel Switcher radio state to one targeted string action while retaining the GTK3 callback bridge.
    - [x] Convert Network Meters radio state to one targeted string action while retaining meter timer and UI updates.
    - [x] Project complete canonical menu subtrees into retained `GMenuModel` structures.
      - [x] Project the static three-command Search submenu without changing the live GTK3 menu.
      - [x] Add the About action and project the complete two-command Help menu.
      - [x] Add Channel Tab and Channel Window actions and project the complete four-command New submenu.
      - [x] Add the four Server commands and project the complete five-command Server menu with session sensitivity.
      - [x] Project the two-choice Channel Switcher submenu with canonical `tabs` and `tree` targets.
      - [x] Project the four-choice Network Meters submenu with canonical `off`, `graph`, `text`, and `both` targets.
      - [x] Add canonical actions for all ten Settings commands and project the complete two-section Settings menu.
      - [x] Add canonical actions for the remaining Window commands, project its complete nested model, and correct the retained Search/Help boundaries.
      - [x] Convert the remaining View toggles to synchronized boolean actions and project the complete three-section View model.
      - [x] Add canonical Load and Attach/Detach actions and project the complete five-section Fabulor menu.
      - [x] Project the dynamic Usermenu into a retained nested model with command, toggle, separator, icon-hint, edit, and refresh ownership.
      - [x] Project finalized main-menu `/MENU` mutation into a retained nested overlay with copied action ownership and recursive-delete cleanup.
      - [x] Convert contextual `$NICK`, `$URL`, `$CHAN`, `$TAB`, and `$TRAY` popup model ownership.
  - [ ] Replace blocking dialog runs with response-driven lifecycle handling.
    - [x] Convert theme-preference acknowledgement and import-result dialogs without changing message content or modality.
    - [x] Convert the theme colour-manager lifecycle while preserving reset, live preview, staged changes, and nested picker cleanup.
    - [x] Convert client-certificate file selection with copied import state and editor-bound asynchronous cleanup.
    - [x] Convert colors.conf and GTK3 theme import selection with owner-resolved state and parent-bound asynchronous cleanup.
- [ ] Convert input events, shortcuts, clipboard, drag/drop, and pointer gestures.
- [ ] Convert tree/list models, cell renderers, channel navigation, and operational lists.
- [ ] Port the transcript and spell-check input widgets to GTK4 rendering and event semantics.
- [ ] Add the GTK4 theme adapter and validate tray, notifications, icons, fonts, and platform integration.
  - [x] Retire `.zct` registration and mock Windows GTK theme staging, downloads, and installer choices.
  - [ ] Discover system GTK4 desktop themes and imported themes under `%APPDATA%\Fabulor\themes`.
  - [ ] Retain `.hct` and `colors.conf` without packaging an optional default Fabulor theme.
  - [ ] Remove GTK3 theme discovery and `%APPDATA%\Fabulor\gtk3-themes` use only after the GTK4 adapter is ready.
- [ ] Cut production builds, CI, staging, and WiX packaging over to an allowlisted GTK4 runtime.
- [ ] Remove GTK3 code, build inputs, runtime files, installer components, and compatibility helpers.
- [ ] Complete clean-install, upgrade, accessibility, visual, performance, plugin, and packaging validation.

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
