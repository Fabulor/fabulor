# GTK4 Migration Plan

Status: planning baseline

Baseline date: 2026-07-14

## Objective

Move the Fabulor desktop frontend from GTK 3.22+ to GTK 4 while preserving the
current IRC, add-on, accessibility, spell-checking, theming, installer, and
Windows integration behaviour.

The migration is complete only when the production executable links GTK4,
legacy GTK3 runtime files are absent from clean and upgraded installations, and
the validation matrix in [validation-log.md](validation-log.md) passes.

## Current State

- `src/fe-gtk/meson.build` requires `gtk+-3.0 >= 3.22`.
- `win32/zoitechat.props` selects GTK3 headers and import libraries.
- `.github/workflows/windows-build.yml` downloads separate GTK3 build
  dependencies and a GTK4 runtime payload.
- `win32/copy/copy.vcxproj` stages GTK3 DLLs, modules, print backends, emoji
  data, themes, schemas, icons, and shared libraries beside `fabulor.exe`.
- WiX already packages a repository `Runtime/GTK4` tree. The current tree
  contains GTK 4.22.4 and GLib 2.88.0, but the executable does not yet consume
  that GTK4 build surface.
- Theme code is intentionally split into policy/runtime/application layers and
  a GTK3 adapter. This is a useful boundary for introducing a GTK4 adapter.
- The frontend is source-driven rather than GtkBuilder-driven; there are no
  `.ui` files to convert.

## Migration Rules

1. Keep the shipping GTK3 build green until a GTK4 build can exercise the core
   client workflow.
2. Use one maintained frontend source tree. Temporary compatibility helpers are
   acceptable; a permanent forked `fe-gtk3` and `fe-gtk4` implementation is not.
3. Keep production behaviour unchanged unless a PR explicitly documents and
   validates an approved UX change.
4. Migrate ownership and lifetime semantics deliberately. GTK4 child ownership,
   asynchronous dialogs, event controllers, list models, and snapshot drawing
   are architectural changes, not search-and-replace work.
5. Do not switch installer payloads before the GTK4 executable and its runtime
   discovery rules are validated together.
6. Every conversion PR updates [api-inventory.md](api-inventory.md) and records
   its checks in [validation-log.md](validation-log.md).
7. Remove compatibility code once its final caller is on GTK4. Do not accumulate
   a second private widget toolkit.

## Stages

### Stage 0: Documentation And Baseline

Deliverables:

- Record the migration order, API inventory, runtime packaging state, and
  validation matrix.
- Capture baseline x64 build results, installed layout, runtime versions, and
  high-risk frontend files.
- Keep this stage documentation-only.

Exit criteria:

- The four documents under `docs/gtk4` exist and are linked from `To-Do.md`.
- High-risk API families and ownership boundaries have named migration stages.

### Stage 1: Build And Compatibility Foundation

Progress (2026-07-14): the Windows x64 GTK4 archive is pinned by URL, size, and
SHA-256 in `tools/gtk4/dependency-contract.json`. The isolated MSVC and Meson
probes validate GTK/GLib versions, x64 architecture, required files, and GTK3
contamination before compiling, linking, and executing against the same root.
Production targets remain on GTK3. Compatibility helpers have not started.

Deliverables:

- Define one authoritative GTK4 build root for MSVC and Meson.
- Add GTK4 include/library discovery without silently falling back to GTK3.
- Add narrowly scoped compatibility helpers for child assignment, widget
  visibility, window destruction, CSS provider installation, and other repeated
  mechanical transitions.
- Add CI compile probes for converted helpers and modules before enabling the
  full GTK4 target.
- Pin and record the GTK4/GLib build inputs used by CI and local builds.

Exit criteria:

- GTK3 production builds remain clean.
- The GTK4 dependency root is deterministic and version-reportable.
- Converted modules cannot accidentally link a mixed GTK3/GTK4 process.

### Stage 2: Widget Ownership And Layout

Primary API families:

- `gtk_container_*`
- `gtk_box_pack_*`
- `gtk_widget_show_all`
- `gtk_widget_destroy`
- `GtkEventBox`, `GtkTable`, and legacy child enumeration

Deliverables:

- Move windows, boxes, panes, notebooks, scrollers, and utility constructors to
  GTK4 child APIs and explicit ownership.
- Convert low-risk dialogs and utility windows first.
- Preserve geometry persistence, tab/window modes, and close behaviour.

Exit criteria:

- No converted surface depends on GTK3 container ownership.
- Window creation/destruction and repeated open/close tests are leak- and
  crash-free.

### Stage 3: Actions, Menus, Dialogs, And File Selection

Primary surfaces:

- `menu.c`, context menus, tray menus, and user-defined menus
- synchronous `gtk_dialog_run()` call sites
- message, file chooser, colour, font, and confirmation dialogs

Deliverables:

- Represent commands through `GAction`/`GMenuModel` where appropriate.
- Replace blocking dialog loops with response-driven or asynchronous flows.
- Preserve command sensitivity, accelerators, context, and cancellation paths.

Exit criteria:

- No GTK4 path depends on removed menu widgets or nested dialog event loops.
- File/add-on/theme selection retains the existing containment checks.

### Stage 4: Input, Events, Clipboard, Drag/Drop, And Shortcuts

Primary API families:

- direct `GdkEvent` handlers and event masks
- `gtk_widget_get_window()` and `gdk_window_*`
- GTK3 clipboard, selection, drag/drop, and accelerator handling

Deliverables:

- Use GTK4 event controllers and gestures for keys, pointers, scrolling, focus,
  and clicks.
- Use `GdkClipboard` and GTK4 content providers.
- Migrate drag/drop and keyboard shortcuts without changing IRC input semantics.
- Measure edit-box latency and emoji-picker responsiveness throughout this
  stage.

Exit criteria:

- Typing, Enter-to-send, completion, history, paste, URLs, spell-check popup,
  emoji insertion, scrolling, selection, and drag/drop pass the interaction
  matrix.

### Stage 5: Lists, Trees, Models, And Channel Navigation

Primary surfaces:

- server list and editor
- channel/user/notify/ignore/ban/DCC lists
- preferences trees, key bindings, event text, and custom list models
- channel tree and tab switcher

Deliverables:

- Choose GTK4 list/model widgets per workflow rather than mechanically retaining
  deprecated tree abstractions.
- Preserve sorting, editing, selection, keyboard navigation, context menus,
  badges/icons, and large-list performance.
- Retire GTK3 cell-renderer assumptions as each surface moves.

Exit criteria:

- All operational lists pass keyboard, mouse, sorting, editing, and scale tests.
- Channel switching and user-list updates remain responsive under IRC load.

### Stage 6: Custom Text And Input Widgets

Primary surfaces:

- `xtext.c` / `xtext.h`
- `sexy-spell-entry.c` / `sexy-spell-entry.h`

Deliverables:

- Port the transcript widget from GTK3 realize/window/draw/event virtual methods
  to GTK4 measurement, allocation, snapshot rendering, focus, selection, and
  event-controller semantics.
- Port or replace the `GtkEntry` spell-check subclass while preserving Enchant
  2.8.19 checking, suggestions, add-to-dictionary, URL paste stability, and
  personal-dictionary persistence.
- Preserve IRC formatting, timestamps, markers, search, URL hit-testing,
  selection, copy, scrolling, and scrollback performance.

Exit criteria:

- Transcript and edit-box behavioural tests pass on normal and high-DPI displays.
- No GTK3 windowing or draw virtual method remains in either widget.
- Input and sent-message display latency meet the baseline thresholds.

### Stage 7: Themes, Tray, Notifications, And Platform Integration

Primary surfaces:

- `theme/theme-gtk3.c` and GTK3 theme import/service assumptions
- CSS provider installation and display/theme change notifications
- `plugin-tray.c`, AppIndicator/StatusNotifier, Win32 tray, and notifications
- icon, fontconfig, GSettings, and runtime path setup in `fe-gtk.c`

Deliverables:

- Add a GTK4 theme adapter and define the fate of imported GTK3 themes. Do not
  claim direct compatibility for GTK3 CSS that GTK4 cannot safely consume.
- Preserve Fabulor palette/custom-CSS behaviour and dark-mode selection.
- Validate native Windows tray/notification paths independently from optional
  Unix tray backends.
- Remove GTK3-specific icon-path and theme workarounds only after replacement
  behaviour is proven.

Exit criteria:

- Theme switching, restart persistence, tray state, notifications, icons,
  fonts, spell-check UI, and emoji rendering pass on the packaged GTK4 runtime.

### Stage 8: Production Cutover And Runtime Cleanup

Deliverables:

- Make GTK4 the only production frontend dependency in MSVC, Meson, and CI.
- Rebuild native dependencies that share GLib/CRT ownership against the final
  runtime where required.
- Switch staging and WiX to an allowlisted GTK4 payload.
- Remove GTK3 DLLs, modules, themes, emoji data, build downloads, properties,
  compatibility helpers, and installer components.
- Validate clean install, in-place upgrade, repair, uninstall, and portable mode.

Exit criteria:

- `fabulor.exe` imports GTK4 and no GTK3 DLL.
- Clean and upgraded installations contain no unintended GTK3 runtime files.
- CI, CodeQL, native tests, installer builds, and the full validation matrix pass.
- Release notes identify the GTK4 cutover and any deliberately retired theme or
  platform behaviour.

## Pull Request Boundaries

Prefer PRs that complete one ownership boundary or one user workflow. Suitable
examples include:

- build-root and compatibility helpers
- utility dialogs and file selection
- menu/action model
- clipboard and shortcut controllers
- channel/user list model
- transcript rendering
- spell-check input
- theme adapter
- tray and notification integration
- packaging cutover

Do not combine transcript rendering, list models, and installer cutover in one
PR. Each PR must keep the shipping build usable and identify its rollback point.

## Open Decisions

- Minimum supported GTK4/GLib versions after evaluating the current 4.22.4 /
  2.88.0 Windows bundle against supported non-Windows distributions.
- Whether imported GTK3 themes are retired, converted into a constrained Fabulor
  palette format, or retained only for the final GTK3 release line.
- GTK4 list widget choices for each large editable model.
- Whether the spell-check entry remains a custom widget or becomes a composed
  input control.
- Which Unix tray backend remains supportable after GTK4 cutover.
- The final allowlisted Windows runtime payload and provenance mechanism.
