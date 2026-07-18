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
- [`docs/gtk4/list-model-architecture.md`](docs/gtk4/list-model-architecture.md)
- [`docs/gtk4/theme-architecture.md`](docs/gtk4/theme-architecture.md)
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
- [x] Convert actions, menus, dialogs, and file-selection flows.
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
  - [x] Replace blocking dialog runs with response-driven lifecycle handling.
    - [x] Convert theme-preference acknowledgement and import-result dialogs without changing message content or modality.
    - [x] Convert the theme colour-manager lifecycle while preserving reset, live preview, staged changes, and nested picker cleanup.
    - [x] Convert client-certificate file selection with copied import state and editor-bound asynchronous cleanup.
    - [x] Convert colors.conf and GTK3 theme import selection with owner-resolved state and parent-bound asynchronous cleanup.
    - [x] Convert manifest-plugin confirmation, quit/minimize confirmation, fatal-font handling, and modal frontend messages without nested GTK event loops.
- [ ] Convert input events, shortcuts, clipboard, drag/drop, and pointer gestures.
  - [x] Convert shared explicit-copy commands to GTK4 display clipboards while retaining primary-selection updates where supported.
  - [x] Establish typed pointer-enter and focus-controller ownership for simple interactions, and use semantic button activation where available.
  - [x] Convert channel tree, viewport, tab, and close-button scrolling to a shared delta-based GTK4 scroll-controller boundary.
  - [x] Convert detached input, top-level session, and tab-window focus entry to the shared GTK4 focus-controller boundary.
  - [x] Convert channel-tab close hover and pointer-leave cleanup to shared motion and cursor boundaries without changing click dispatch.
  - [x] Convert utility-window Escape, search-bar Escape, and raw-log copy to a shared GTK4 key-controller boundary.
  - [x] Convert topic submission and channel key/limit `Ctrl+A` selection to the shared GTK4 key-controller boundary.
  - [x] Convert the main chat-input plugin and configurable shortcut engine to normalized GTK4 key-controller input.
  - [x] Convert topic URL hover, cursor leave, and modified left-click activation to GTK4 motion and click controllers.
  - [x] Convert private-dialog and user-list external file drops to a typed GTK4 `GdkFileList` boundary.
  - [x] Convert internal channel-view, user-list, scrollbar, and pane-position drag operations to typed GTK4 source/drop controllers.
- [x] Convert tree/list models, cell renderers, channel navigation, and operational lists.
  - [x] Establish tested GTK4 flat and hierarchical model stacks with explicit sorting, selection, identity, and ownership contracts.
  - [x] Convert the Notify List to a cross-version owner with GTK4 column factories, identity-based refresh, and single-selection actions.
  - [x] Replace per-session user-list stores and external row references with an opaque cross-version typed-row model owner.
  - [x] Convert the shared user-list view, factories, multi-selection, hit-testing, menus, keyboard handling, and drag/drop.
  - [x] Replace shared channel-navigation tree storage and persistent row iterators with an identity-indexed cross-version hierarchy owner.
  - [x] Convert the visible hierarchical channel tree to a cross-version view owner with GTK4 factories, expansion, selection, hit-testing, and scrolling.
  - [x] Convert channel-tree and tab-strip close/context input to a widget, button, coordinates, and modifier boundary without raw GTK3 events.
  - [x] Move grouped-tab animation and toggle suppression into each channel view and resolve focus movement through the channel model.
  - [x] Mirror grouped-tab and family reorder operations from authoritative channel-model positions without GTK child enumeration.
  - [x] Own grouped-tab family boxes by model-root identity with deterministic creation, insertion, removal, and cleanup.
  - [x] Own grouped-tab tab, label, and close-button presentation records by channel identity without GTK widget metadata.
  - [x] Convert grouped-tab close hit-testing, hover state, cursor feedback, and whole-tab prelight suppression to cross-version helpers.
  - [x] Convert grouped-tab scroll target discovery and adjustment ownership to model-driven cross-version geometry.
  - [x] Convert grouped-tab mouse and keyboard activation and confirm channel-view drag/drop already uses the typed Stage 4 boundary.
  - [x] Convert the loaded Add-ons table to a cross-version typed owner with GTK4 column factories and single-selection actions.
  - [x] Convert URL History to a typed cross-version owner with newest-first limits, selection actions, and coordinate-based activation.
  - [x] Convert the Ignore List to a typed cross-version editable owner with mask sorting, flag toggles, and selection-safe mutations.
  - [x] Convert the Ban List to a typed cross-version multi-selection owner with mode-safe removal, crop, clear, copy, and date sorting.
  - [x] Convert the combined DCC Uploads and Downloads table to a typed cross-version multi-selection owner with identity-safe updates and actions.
  - [x] Convert the distinct DCC Chat table to a typed cross-version owner.
  - [x] Convert the Channel List to a typed cross-version sorted owner while retaining batched population, filtering, export, and multi-selection actions.
  - [x] Convert the generic two-column configuration editors to a typed cross-version editable owner with pointer and keyboard reordering.
  - [x] Convert the Print Events editor and argument-help table to one typed cross-version owner with callback-gated edits and signal-safe selection.
  - [x] Convert the configurable key-bindings editor to a typed cross-version owner with accelerator capture, custom-row controls, reset, ordering, and save snapshots.
  - [x] Convert the Preferences sound-event table to a typed cross-version owner with stable event identity, explicit selection, and live file updates.
  - [x] Convert the Preferences category hierarchy to a typed cross-version owner with non-selectable headings, stable page identity, expansion, remembered selection, and explicit page switching.
  - [x] Convert the main Server List network table to a typed cross-version owner with stable network identity, favorite state, inline rename, filtering refresh, selection, and keyboard ordering.
  - [x] Convert the Server List server, autojoin-channel, and connect-command editors to a typed cross-version owner with stable entry identity, inline editing, add/remove actions, and keyboard ordering.
- [ ] Port the transcript and spell-check input widgets to GTK4 rendering and event semantics.
  - [x] Establish a tested transcript render-target owner for active Cairo contexts, offscreen surfaces, the contained GTK3 window fallback, and GTK4 snapshot output.
  - [x] Route transcript wrapping, rendering, selection scrolling, visibility, and buffer-switch dimensions through validated widget geometry instead of `GdkWindow` size reads.
  - [x] Install transcript measurement, allocation, realize/unrealize, and GTK3 draw or GTK4 snapshot class methods through a tested cross-version widget-class adapter.
  - [x] Replace transcript pointer, click, scroll, leave, and focus event virtual methods with normalized GTK3 signals or GTK4 controllers while preserving selection and word-click behavior.
  - [x] Replace transcript selection ownership and payload virtual methods with a GTK3 signal or GTK4 content-provider adapter while preserving primary selection and explicit copy behavior.
  - [x] Contain transcript native-window scroll copying to GTK3 and route GTK4 frame damage, focus, CSS class, and scrolling through full snapshot redraw semantics.
  - [x] Move transcript background source, fitted or tiled composition, frame cache, palette fallback, and teardown into a tested Cairo-only owner.
  - [x] Move transcript marker placement, search-match boundary classification, and transient hover-highlight state into a tested toolkit-neutral decoration owner.
  - [x] Contain transcript line, separator, formatted-match, and word-click hit testing behind a validated result boundary without scratch-buffer mutation.
  - [x] Assign transcript log semantics and a stable accessible label, and validate logical font/decorations plus scale-aware inline flag rendering.
  - [x] Expose a bounded read-only GTK4 accessible-text snapshot with Unicode boundaries and coalesced append, trim, clear, timestamp, and buffer-switch updates.
  - [x] Validate deterministic append redraw coalescing and wrapped-line scrollback bounds, preserving immediate local echo and the newest complete entry.
  - [x] Separate spell-input UTF-8 word ownership into tested byte and character ranges so Unicode dictionary and replacement actions target the correct text.
  - [x] Retain the spell-entry subclass with inherited editable semantics and replace its draw, pointer, redraw, and theme-lifetime virtual boundaries.
  - [x] Move IRC formatting, semantic colours, and misspelling Pango attributes into a tested widget-independent owner.
  - [x] Replace GTK4 spell and formatting popup mutation with an owned dynamic menu model and per-entry actions.
- [ ] Add the GTK4 theme adapter and validate tray, notifications, icons, fonts, and platform integration.
  - [x] Retire `.zct` registration and mock Windows GTK theme staging, downloads, and installer choices.
  - [x] Establish deterministic GTK4 desktop/profile discovery metadata without loading CSS.
  - [x] Establish a transactional GTK4 CSS-provider adapter with explicit diagnostics and teardown.
  - [x] Establish independent GTK4 preference choices and persisted selection/variant keys.
  - [x] Establish Windows light/dark and high-contrast decisions for GTK4 theme application.
  - [x] Compose GTK4 discovery, preferences, appearance, and providers behind one lifecycle controller.
  - [x] Contain notification backend loading, managed errors, WinRT initialization, and module teardown.
  - [x] Establish a toolkit-neutral tray action model with owned labels, state, dispatch, and teardown.
  - [x] Bind the tray action model to live visibility, away, blink, command, and plugin lifecycle state.
  - [x] Compose dynamic `$TRAY` plugin entries into an owned tray projection with separate action namespaces and inert teardown.
  - [x] Bind tray projections to an owned GTK4 popover presenter with replaceable action groups and deterministic unparenting.
  - [x] Centralize tray backend selection and prohibit legacy status-icon fallback for GTK4 or unknown toolkit versions.
  - [x] Discover system GTK4 desktop themes and imported themes under `%APPDATA%\Fabulor\themes`.
  - [x] Bind discovered GTK4 themes and variant policy to owned preference controls with transactional persistence callbacks.
  - [x] Refresh GTK4 theme appearance from queued Windows light/dark and high-contrast signals without GTK3 window filters.
  - [x] Retain `.hct` and `colors.conf` without packaging an optional default Fabulor theme.
  - [ ] Remove GTK3 theme discovery and `%APPDATA%\Fabulor\gtk3-themes` use only after the GTK4 adapter is ready.
- [ ] Cut production builds, CI, staging, and WiX packaging over to an allowlisted GTK4 runtime.
  - [x] Establish deterministic candidate staging with a pinned file/tree contract and source-bound SHA-256 manifest.
  - [ ] Validate the candidate payload, then replace transitional broad WiX harvesting with the allowlist.
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
