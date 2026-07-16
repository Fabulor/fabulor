# GTK4 Validation Log

Status: baseline and reusable test matrix

Baseline date: 2026-07-14

## Purpose

This document is the evidence log for GTK4 conversion work. Update it in every
GTK4 PR with the exact build, automated-test, manual-test, screenshot, runtime,
and packaging results relevant to that change.

Do not mark a migration stage complete from compilation alone.

## Baseline Record

The pre-migration Windows client is GTK3-based. Recent x64 Release validation
before this documentation stage established:

- MSVC solution rebuilds with 0 warnings and 0 errors
- native manifest/path/policy tests pass 18/18
- C#, Python, and Tcl manifest samples load and dispatch incoming message events
- the manifest preference default, confirmation, persistence, restart, disable,
  and safe-mode behaviour pass in an installed upgrade
- Enchant 2.8.19 checking, suggestions, add-to-dictionary, URL paste, and personal
  dictionary persistence work in the installed client
- emoji picker and edit-box responsiveness are acceptable after the recent
  caching and spell-check fixes
- the WiX installer builds with the known empty GTK4 `lib/gio` harvest warning
  when external ICE validation is suppressed

These results are a behavioural baseline, not GTK4 validation.

### PR: [#17 Establish GTK4 Build Dependency Probe](https://github.com/Fabulor/fabulor/pull/17)

Date: 2026-07-14

Validated implementation commits: `10492d71`, `a6d38806`

Migration stage: 1, dependency root and probe only

Files/workflows converted: `tools/gtk4`, Windows installer-build workflow

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] GTK4 validator unit tests: 8/8
- [x] repository `Runtime/GTK4` root validation
- [x] isolated MSVC compile, link, and runtime probe with no warnings
- [x] isolated Meson configure, compile, link, and runtime test
- [x] GitHub Actions Windows build, including MSVC/Meson probes and installer
- [x] repository lint
- [x] unchanged production MSVC x64 Release build, including native tests 18/18

Observed probe identity: GTK 4.22.4 / GLib 2.88.0 / 64-bit.

GitHub Actions result: all five required checks passed. The Windows x64 job
completed in 7 minutes 17 seconds after explicitly selecting the x64 host and
target toolchain for Meson.

Production impact: none. The Fabulor solution and frontend remain linked to
GTK3; no GTK4 widget or compatibility code is enabled.

### PR: [#18 - GTK4 Compatibility Helpers, Pass 1](https://github.com/Fabulor/fabulor/pull/18)

Date: 2026-07-14

Commit: `90915dd0`

Migration stage: 1, compatibility helper boundary

Files/workflows converted: `src/fe-gtk/gtk-compat.h`, one scrolled-window child
assignment in `gtkutil.c`, and the isolated GTK4 probes

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] GTK4 validator unit tests: 8/8
- [x] repository `Runtime/GTK4` root validation, including GObject import library
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild, including native tests 18/18
- [x] GitHub Actions required checks: 5/5

Observed probe identity: GTK 4.22.4 / GLib 2.88.0 / 64-bit.

Scope: type-specific single-child assignment and window destruction only. No
generic ownership, layout, dialog, menu, event, model, or user-facing workflow
conversion is included.

### PR: [#19 - GTK4 Widget Ownership And Layout, Pass 1](https://github.com/Fabulor/fabulor/pull/19)

Date: 2026-07-14

Commit: `1cc45829`

Migration stage: 2, typed single-child ownership

Files/workflows converted: 38 additional child assignments across utility
windows, channel views, main-window construction, server-list editors,
preferences, raw log, event editor, theme preferences, and user-list scrollers

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with no warnings
- [x] compatibility usage inventory: 39 typed child assignments
- [x] remaining direct `gtk_container_*` inventory: 117 lines in 23 files
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC and Meson compile, link, and runtime probes
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: statically identifiable single-child windows, scrolled windows, frames,
buttons, overlays, and popovers. Ambiguous containers, packing, dialog content,
menus, list rows, event boxes, viewport-dependent code, visibility, and generic
destruction remain deferred.

### PR: [#20 - GTK4 Widget Ownership And Layout, Pass 2](https://github.com/Fabulor/fabulor/pull/20)

Date: 2026-07-14

Commit: `273f7225`

Migration stage: 2, start-ordered box layout

Files/workflows converted: 54 box additions across the character chart,
list/key editors, ignore dialog, join dialog, event editor, and theme
preferences

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with no warnings
- [x] compatibility usage inventory: 54 typed box additions
- [x] remaining direct `gtk_box_pack_*` inventory: 132 lines in 15 files
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC and Meson compile, link, and runtime probes
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: start-only modules where GTK4 append order is identical to GTK3 packing
order. Mixed start/end layouts, menus, operational list models, generic
visibility, and destruction remain deferred.

### PR: [#21 - GTK4 Widget Ownership And Layout, Pass 3](https://github.com/Fabulor/fabulor/pull/21)

Date: 2026-07-14

Commit: `e16ede91`

Migration stage: 2, reviewed mixed box ordering

Files/workflows converted: 40 box additions across channel tabs, ban/DCC/
friends/add-on/URL utility windows, raw log, and preferences

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with no warnings
- [x] compatibility usage inventory: 94 typed box additions in 15 files
- [x] remaining direct `gtk_box_pack_*` inventory: 92 lines in 7 files
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC and Meson compile, link, and runtime probes
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: mixed layouts whose existing expanding leading child preserves trailing
placement under ordered append, plus one explicitly reordered channel-family
separator. Main-window dynamics, generic dialogs, menus, model surfaces,
visibility, and destruction remain deferred.

### PR: [#22 - GTK4 Widget Visibility And Lifecycle, Pass 1](https://github.com/Fabulor/fabulor/pull/22)

Date: 2026-07-14

Commit: `27d4e3e5`

Migration stage: 2, completed-tree visibility and typed window destruction

Files/workflows converted: DCC, ban, key-binding, add-on, raw-log, notify,
event-text, join, server-list, preferences, theme-colour, and edit-list windows

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] compatibility usage inventory: 12 reviewed completed-tree reveals and 24
  typed window/dialog destroys
- [x] remaining direct `gtk_widget_show_all`: 22 lines in nine files
- [x] remaining direct `gtk_widget_destroy`: 57 lines in 16 files
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: only completed roots without intentional hidden descendants and direct
destruction of statically known `GtkWindow`/`GtkDialog` instances. Menus,
conditionally hidden main-window content, generic signal callbacks, and
non-window widgets remain explicit.

### PR: [#23 - GTK4 Widget Lifecycle, Pass 2](https://github.com/Fabulor/fabulor/pull/23)

Date: 2026-07-14

Commit: `12161b00`

Migration stage: 2, shared prompt and application-window lifecycle

Files/workflows converted: shared string/integer/boolean prompts, frontend
messages and session windows, server-list certificate responses, ignore
confirmation, main tab windows, close confirmation, quit dialog, and
attach/detach replacement windows

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] compatibility usage inventory: 15 reviewed completed-tree reveals, 39
  direct typed window/dialog destroys, and five typed response adapters
- [x] remaining direct `gtk_widget_show_all`: 19 lines in eight files
- [x] remaining direct `gtk_widget_destroy`: 37 lines in 13 files
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: completed shared prompts, exact dialog response callbacks, and concrete
application windows. Generic Escape/button destroy helpers, menu ownership,
notebook/channel-view children, dynamic controls, unparented probes,
spell-entry menu items, and GTK3-only tests remain explicit.

### PR: [#24 - GTK4 Generic Dialog Layout Pass](https://github.com/Fabulor/fabulor/pull/24)

Date: 2026-07-14

Commit: `cda2b16c`

Migration stage: 2, typed dialog content ownership and trailing-pair layout

Files/workflows converted: shared string/integer/boolean prompts and the
notify-add dialog

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] remaining direct `gtk_box_pack_*`: 88 lines in seven files
- [x] remaining direct `gtk_container_*`: 113 lines in 23 files
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: four typed dialog-content insertions and two reviewed trailing
label/control pairs. Synchronous response behavior, menu dialogs, generic
button helpers, and model-heavy server-list layouts remain unchanged.

### PR: [#25 - GTK4 Main-Window Layout Pass 1](https://github.com/Fabulor/fabulor/pull/25)

Date: 2026-07-14

Commit: `7d9bc5fe`

Migration stage: 2, reviewed main-window append order

Files/workflows converted: quit content, channel mode/topic controls,
transcript scaffolding, information frames, user-list structure, centre panes,
emoji pages, search controls, reply bar, and input row

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] compatibility usage inventory: 137 start-ordered additions in 17 files,
  one horizontal trailing child, and two trailing pairs
- [x] remaining direct `gtk_box_pack_*`: 48 lines in seven files
- [x] remaining `maingui.c` direct packing: 10 lines, including one commented
  legacy block
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: only rows where construction order is the final visual order. Dynamic
nickname/meter insertion, trailing-tab placement, explicit dialog-button
reordering, menus, and model-heavy layouts remain unchanged.

### PR: [#26 - GTK4 Dynamic Main-Window Layout And Lifecycle Pass 2](https://github.com/Fabulor/fabulor/pull/26)

Date: 2026-07-14

Commit: `0d6d76bb`

Migration stage: 2, dynamic main-window ordering and box-owned lifecycle

Files/workflows converted: nickname access icon and connection progress,
permanent nickname button, user-list button grid, lag/throttle meter layout
and refresh, dialog-button refresh, and obsolete generic-tab comment cleanup

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] compatibility usage inventory: 143 start-ordered additions, two
  horizontal trailing children, two before-trailing insertions, and five
  box-owned dynamic removals
- [x] remaining direct `gtk_box_pack_*`: 38 lines in six files and zero in
  `maingui.c`
- [x] remaining direct `gtk_widget_show_all`: 18 lines in eight files
- [x] remaining direct `gtk_widget_destroy`: 32 lines in 13 files
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: dynamic children with exact box ownership and reviewed final order.
Meter `GtkEventBox` wrappers, menus, model-heavy layouts, and generic widget
destruction remain unchanged for their owning stages.

### PR: [#27 - GTK4 Menu-Dialog And Shared-Button Ownership](https://github.com/Fabulor/fabulor/pull/27)

Date: 2026-07-14

Commit: `bbd7863d`

Migration stage: 2, typed Join Channel dialog and shared button ownership

Files/workflows converted: Join Channel dialog content, ordering, reveal, and
response destruction; labelled and label-less `gtkutil_button()` construction

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] compatibility usage inventory: 145 start-ordered additions, three
  trailing pairs, 40 typed child assignments, and 17 reviewed reveals
- [x] remaining direct `gtk_box_pack_*`: 35 lines in five files
- [x] remaining direct `gtk_container_*`: 110 lines in 23 files
- [x] remaining direct `gtk_widget_show_all`: 17 lines in eight files
- [x] remaining direct `gtk_widget_destroy`: 31 lines in 13 files
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: the already response-driven Join Channel dialog and the shared button
constructor's exact child/parent ownership. Menu items, actions, accelerators,
models, file selection, and synchronous dialog flows remain unchanged.

### PR: [#28 - GTK4 Operational-List Shell Layout, Pass 1](https://github.com/Fabulor/fabulor/pull/28)

Date: 2026-07-14

Commit: `8af2bab6`

Migration stage: 2, typed operational-list scroller and channel-list shell
ownership

Files/workflows converted: shared tree-view scrollers for ban, channel, DCC,
ignore, notify, plugin, and URL lists; main user-list scroller; channel-list
status, range, filter, and search-option rows

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] compatibility usage inventory: 156 start-ordered additions in 17 files
- [x] shared tree-view constructor: nine typed `GtkBox` callers
- [x] main user-list constructor: one typed `GtkBox` caller
- [x] remaining direct `gtk_box_pack_*`: 25 lines in four files
- [x] remaining direct `gtk_container_*`: 109 lines in 23 files
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: parent ownership and exact shell construction order only. Tree models,
renderers, selection, sorting, context menus, drag/drop, and pointer/key events
remain unchanged for their Stage 3-5 conversions.

### PR: [#29 - GTK4 Menu Action-Identity Foundation, Pass 1](https://github.com/Fabulor/fabulor/pull/29)

Date: 2026-07-14

Commit: `7a19ec75`

Migration stage: 3, canonical menu command identity and dispatch

Files/workflows converted: main-menu accelerator identity lookup and
configurable keyboard-shortcut dispatch in `menu.c`

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] canonical menu action entries: 16 unique command identities
- [x] menu construction and shortcut dispatch share definitions without a
  positional action table
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: command identity and dispatch only. GTK3 menu widgets, action binding,
state, sensitivity, dynamic mutation, popup ownership, accelerators, dialogs,
and pointer/key event handling remain unchanged for later Stage 3-4 passes.

### PR: [#30 - GTK4 Stateless Menu Action Activation, Pass 2](https://github.com/Fabulor/fabulor/pull/30)

Date: 2026-07-14

Commit: `422bf0d2`

Migration stage: 3, stateless canonical menu action activation

Files/workflows converted: 12 stateless main-menu and middle-menu commands in
`menu.c`; configurable keyboard shortcuts retain canonical direct dispatch

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] stateless canonical action bindings: 12 commands
- [x] focused GTK3 activation probe: exactly one activation
- [x] focused GTK3 lifetime probe: item retains group after builder release
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: stateless action activation only. Stateful menu-bar, user-list,
fullscreen, and away identities; sensitivity; dynamic menus; popup ownership;
dialogs; and pointer/key event handling remain unchanged for later passes.

### PR: [#31 - GTK4 Window-View Menu Action State, Pass 3](https://github.com/Fabulor/fabulor/pull/31)

Date: 2026-07-14

Commit: `29d593c8`

Migration stage: 3, window-view menu action state synchronization

Files/workflows converted: menu-bar visibility, user-list visibility, and
fullscreen state in `menu.c`; configurable keyboard shortcuts retain canonical
direct dispatch

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] stateful canonical action bindings: 3 window-view commands
- [x] focused GTK3 boolean action probe: activation and external state synchronization
- [x] menu-bar and user-list preference updates synchronize shared action state
- [x] fullscreen window-state correction synchronizes action state
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: window-view boolean state only. Away state and connection sensitivity,
dynamic menus, popup ownership, dialogs, and pointer/key event handling remain
unchanged for later passes.

### PR: [#32 - GTK4 Session-Aware Away Action State, Pass 4](https://github.com/Fabulor/fabulor/pull/32)

Date: 2026-07-14

Commit: `d9e77679`

Migration stage: 3, session-aware Away action state and availability

Files/workflows converted: Away menu creation, active-tab synchronization,
server-confirmed away/back state, and connect/disconnect sensitivity in
`menu.c`, `maingui.c`, and `fe-gtk.c`

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] canonical action bindings: all 16 commands
- [x] focused GTK3 action probe: disabled activation suppression
- [x] focused GTK3 action probe: enabled state propagates to the bound menu item
- [x] focused GTK3 action probe: authoritative boolean state update
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: canonical Away action state and availability only. GTK4 menu models,
dynamic menus, popup ownership, dialogs, and pointer/key event handling remain
unchanged for later passes.

### PR: [#33 - GTK4 Search Menu Model, Pass 5](https://github.com/Fabulor/fabulor/pull/33)

Date: 2026-07-14

Commit: `26097d56`

Migration stage: 3, first canonical `GMenuModel` subtree projection

Files/workflows converted: retained Search submenu model and menu-root action
group ownership in `menu.c`; the displayed GTK3 submenu remains unchanged

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] focused GTK3/GIO probe: exactly 3 Search model entries
- [x] focused GTK3/GIO probe: canonical labels and `fabulor.*` action names
- [x] focused GTK3/GIO probe: root-plus-item action ownership activates once
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: static Search submenu model projection only. The live GTK3 menu,
remaining static commands, dynamic menus, popup ownership, dialogs, and
pointer/key event handling remain unchanged for later passes.

### PR: [#34 - GTK4 Help Menu Model, Pass 6](https://github.com/Fabulor/fabulor/pull/34)

Date: 2026-07-14

Commit: `b486f1cf`

Migration stage: 3, second canonical `GMenuModel` subtree projection

Files/workflows converted: About command identity and dispatch, generalized
static model projection, and retained Help menu model in `menu.c`; built-in
shortcut help in `fkeys.c`

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] canonical action bindings: 17 commands, including About
- [x] focused GTK3/GIO probe: exactly 2 Help model entries
- [x] focused GTK3/GIO probe: canonical Contents and About action names
- [x] focused GTK3/GIO probe: shared About action activates once
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: static Help menu action and model projection only. The displayed GTK3
menu, remaining static commands, dynamic menus, popup ownership, dialog
lifecycle, and pointer/key event handling remain unchanged for later passes.

### PR: [#35 - GTK4 New Menu Model, Pass 7](https://github.com/Fabulor/fabulor/pull/35)

Date: 2026-07-14

Commit: `0804688c`

Migration stage: 3, third canonical `GMenuModel` subtree projection

Files/workflows converted: Channel Tab and Channel Window command identities,
retained four-entry New submenu model in `menu.c`, and built-in shortcut help
in `fkeys.c`

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] canonical action bindings: 19 commands
- [x] focused GTK3/GIO probe: exactly 4 New model entries
- [x] focused GTK3/GIO probe: canonical labels and `fabulor.*` action names
- [x] focused GTK3/GIO probe: shared Channel Window action activates once
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: static New submenu action and model projection only. The displayed GTK3
menu, remaining static commands, dynamic menus, popup ownership, dialogs, and
pointer/key event handling remain unchanged for later passes.

### PR: [#36 - GTK4 Server Menu Model, Pass 8](https://github.com/Fabulor/fabulor/pull/36)

Date: 2026-07-15

Commit: `1f93cac9`

Migration stage: 3, fourth canonical `GMenuModel` subtree projection

Files/workflows converted: Disconnect, Reconnect, Join a Channel, and Channel
List identities and dispatch; retained two-section Server model; action-backed
Disconnect and Join sensitivity; built-in shortcut help

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] canonical action bindings: 23 commands
- [x] focused GTK3/GIO probe: 2 Server sections containing exactly 5 actions
- [x] focused GTK3/GIO probe: canonical labels and `fabulor.*` action names
- [x] focused GTK3/GIO probe: Disconnect sensitivity sync and one activation
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: static Server menu action, model, and sensitivity projection only. The
displayed GTK3 menu, remaining static commands, dynamic menus, popup ownership,
dialogs, and pointer/key event handling remain unchanged for later passes.

### PR: [#37 - GTK4 Channel Switcher Menu Model, Pass 9](https://github.com/Fabulor/fabulor/pull/37)

Date: 2026-07-15

Commit: `21cec91b`

Migration stage: 3, first targeted selection action and fifth canonical
`GMenuModel` subtree projection

Files/workflows converted: shared Channel Switcher action identity; canonical
Tabs and Tree targets; retained two-item model; preference-to-action state
synchronization; preserved GTK3 radio callback bridge

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] distinct action identities: 24, including Channel Switcher selection
- [x] focused GTK3/GIO probe: exactly 2 canonical model targets
- [x] focused GTK3/GIO probe: existing radio callback activates exactly once
- [x] focused GTK3/GIO probe: typed action activates once and state sync does not reactivate
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: Channel Switcher action, target, model, and synchronization only. The
displayed GTK3 radio controls, remaining static commands, dynamic menus, popup
ownership, dialogs, and pointer/key event handling remain unchanged.

### PR: [#38 - GTK4 Network Meters Menu Model, Pass 10](https://github.com/Fabulor/fabulor/pull/38)

Date: 2026-07-15

Commit: `5a58b8da`

Migration stage: 3, second targeted selection action and sixth canonical
`GMenuModel` subtree projection

Files/workflows converted: shared Network Meters action identity; canonical
Off, Graph, Text, and Both targets; retained four-item model; cross-window
action-state synchronization; preserved GTK3 radio callback, timer, and meter
refresh behaviour

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] distinct action identities: 25, including Network Meters selection
- [x] focused GTK3/GIO probe: exactly 4 canonical model targets
- [x] focused GTK3/GIO probe: existing radio callback activates exactly once
- [x] focused GTK3/GIO probe: typed action activates once and state sync does not reactivate
- [x] focused GTK3/GIO probe: invalid selection target is rejected without changing state
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: Network Meters action, targets, model, and synchronization only. The
displayed GTK3 radio controls, independent Preferences meter values, remaining
static commands, dynamic menus, popup ownership, dialogs, and pointer/key event
handling remain unchanged.

### PR: [#40 - GTK4 Settings Menu Model, Pass 11](https://github.com/Fabulor/fabulor/pull/40)

Date: 2026-07-15

Commit: `eb7942cf`

Migration stage: 3, ten new stateless actions and seventh canonical
`GMenuModel` subtree projection

Files/workflows converted: ten Settings command identities and dispatcher
entries; retained two-section Settings model; preserved GTK3 preference and
configuration-editor behavior

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] distinct action identities: 35, including all ten Settings commands
- [x] focused GTK3/GIO probe: exactly two model sections and ten command items
- [x] focused GTK3/GIO probe: every Settings action activates exactly once
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: Settings command identities, activation, and retained model only. The
displayed GTK3 menu, preference and editor window implementations, dialog
lifecycles, dynamic menus, popup ownership, and input event handling remain
unchanged.

### PR: [#41 - GTK4 Window Menu Model, Pass 12](https://github.com/Fabulor/fabulor/pull/41)

Date: 2026-07-15

Commit: `fa9baa48`

Migration stage: 3, eleven new stateless actions, eighth canonical
`GMenuModel` subtree projection, and retained-model boundary correction

Files/workflows converted: eleven Window command identities and dispatcher
entries; complete two-section Window model with nested Search submenu; relative
Window, Search, and Help boundaries replacing stale downstream offsets

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] distinct action identities: 46, including all Window commands
- [x] source boundary assertions: Window, Search, and Help ranges resolve to their exact table entries
- [x] focused GTK3/GIO probe: Window has two sections, 15 top-level items, and a three-item Search submenu
- [x] focused GTK3/GIO probe: every newly canonical Window action activates exactly once
- [x] focused GTK3/GIO probe: corrected Help model contains Contents and About
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: Window command identities, retained model composition, and correction of
the Search/Help model boundaries. Displayed GTK3 menus, operational windows,
transcript behavior, Save Text containment, dynamic menus, dialogs, and input
event handling remain unchanged.

### PR: [#42 - GTK4 View Menu Model, Pass 13](https://github.com/Fabulor/fabulor/pull/42)

Date: 2026-07-15

Commit: `d2e33901`

Migration stage: 3, three new boolean actions and ninth canonical `GMenuModel`
subtree projection

Files/workflows converted: Topic Bar, User List Buttons, and Mode Buttons
action state; complete three-section View model; retained Channel Switcher and
Network Meters submodels; cross-window state synchronization

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] distinct action identities: 49, including all View controls
- [x] source assertions: all six View boolean controls use the stateful action path
- [x] focused GTK3/GIO probe: View has three sections with 5, 2, and 1 items
- [x] focused GTK3/GIO probe: Channel Switcher and Network Meters retain 2 and 4 targets
- [x] focused GTK3/GIO probe: each new boolean action activates once and state synchronization does not reactivate
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: View action state and retained model composition only. Displayed GTK3
check/radio controls, visibility behavior, meter timers, dynamic menus, dialogs,
and input event handling remain unchanged.

### PR: [#43 - GTK4 Fabulor Menu Model, Pass 14](https://github.com/Fabulor/fabulor/pull/43)

Date: 2026-07-15

Commit: `a6b2ddb6`

Migration stage: 3, two new stateless actions, tenth canonical `GMenuModel`
subtree projection, and static main-menu projection completion

Files/workflows converted: Load Plugin or Script and Attach/Detach identities;
complete five-section Fabulor model; retained New submenu; per-window
Attach/Detach label capture; static model milestone closeout

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] distinct action identities: 51, including Load Plugin or Script and Attach/Detach
- [x] source assertions: both new commands use the stateless dispatcher and labels are selected before model construction
- [x] focused GTK3/GIO probe: Fabulor has five sections with 1, 1, 1, 2, and 1 items
- [x] focused GTK3/GIO probe: retained New submenu contains all four commands
- [x] focused GTK3/GIO probe: Attach and Detach labels are copied per model root
- [x] focused GTK3/GIO probe: each new action activates exactly once
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: static Fabulor command identities and retained model composition only.
Displayed GTK3 menus, plugin chooser containment, Attach/Detach behavior,
dynamic Usermenu and `/MENU` mutation, popup ownership, dialogs, and input event
handling remain unchanged.

### PR: [#44 - GTK4 Dynamic Usermenu Model, Pass 15](https://github.com/Fabulor/fabulor/pull/44)

Date: 2026-07-15

Commit: `b3c5a097`

Migration stage: 3, first complete dynamic `GMenuModel` boundary

Files/workflows converted: recursive Usermenu model projection; command and
preference-toggle actions; section, submenu, icon-hint, and editor composition;
per-window model refresh and retired-action cleanup

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compile and link with 0 warnings and 0 errors
- [x] source assertions: model refresh runs at creation and after Usermenu edits
- [x] focused GIO probe: nested submenu and separator sections retain structure
- [x] focused GIO probe: command targets activate once and toggle state changes
- [x] focused GIO probe: icon hints survive and retired actions are removed
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: dynamic Usermenu model and action ownership only. Displayed GTK3 menu
widgets, command parsing and substitutions, plugin `/MENU` mutation, contextual
popup ownership, dialogs, and input event handling remain unchanged.

### PR: [#45 - GTK4 Plugin Main-Menu Model, Pass 16](https://github.com/Fabulor/fabulor/pull/45)

Date: 2026-07-15

Commit: `e8a98200`

Migration stage: 3, finalized main-menu `/MENU` mutation overlay

Files/workflows converted: common finalized-mutation sync contract; retained
per-window plugin menu overlay; built-in and custom parent-path validation;
command, toggle, and radio actions; metadata projection; recursive-delete and
retired-action cleanup

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] common, GTK3 GUI, and text frontend compile/link with 0 warnings and 0 errors
- [x] source assertions: finalized add, update, and recursive-delete sync points
- [x] source assertions: only real built-in or prior custom parent paths are accepted
- [x] source assertions: ordinary commands retain nickname/context substitution while toggles and radios remain direct
- [x] focused GIO probe: nested mount paths and separator sections retain structure
- [x] focused GIO probe: command metadata and disabled-action suppression
- [x] focused GIO probe: toggle/radio state and complete action retirement
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: main-menu `/MENU` model and copied action ownership only. Displayed GTK3
menus, plugin command semantics, contextual popup roots, dialogs, and input
event handling remain unchanged.

### PR: [#46 - GTK4 Contextual Plugin Popup Model, Pass 17](https://github.com/Fabulor/fabulor/pull/46)

Date: 2026-07-15

Commit: `3fd205ea`

Migration stage: 3, contextual plugin popup model ownership

Files/workflows converted: allowlisted `$NICK`, `$URL`, `$CHAN`, `$TAB`, and
`$TRAY` projections; per-invocation action groups; copied root and target
ownership; weak-owner state refresh; native Windows tray model lifetime

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] source assertions: exactly five contextual roots are accepted
- [x] source assertions: each GTK popup invocation retains its model and action group
- [x] source assertions: native Windows tray invocation owns and retires its model
- [x] source assertions: targeted substitution and direct toggle/radio dispatch are preserved
- [x] focused GIO probe: copied targets survive caller-buffer release
- [x] focused GIO probe: nested sections and namespaced actions retain structure
- [x] focused GIO probe: replacement actions and old references have independent lifetimes
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: contextual plugin popup model and callback ownership only. Displayed
GTK3 popup widgets, native Windows tray items, built-in popup contents, dialogs,
and input event handling remain unchanged.

### PR: [#47 - GTK4 Response-Driven Acknowledgement Dialogs, Pass 18](https://github.com/Fabulor/fabulor/pull/47)

Date: 2026-07-15

Commit: `d4a78140`

Migration stage: 3, acknowledgement-dialog lifecycle

Files/workflows converted: theme-manager apply errors; colors.conf import
errors and success messages; modal parent ownership; response-driven cleanup

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] source assertions: three acknowledgement helpers contain no nested dialog run
- [x] source assertions: each dialog uses the exact-signature response destroy callback
- [x] source assertions: each asynchronous modal dialog is destroyed with its parent
- [x] source inventory: blocking dialog runs reduced from 9 to 6
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: acknowledgement-only theme dialogs and parent-bound response cleanup.
The color manager, certificate chooser, preference confirmation, quit dialog,
startup messages, and synchronous `FE_MSG_WAIT` behavior remain unchanged.

### PR: [#48 - GTK4 Theme Colour-Manager Lifecycle, Pass 19](https://github.com/Fabulor/fabulor/pull/48)

Date: 2026-07-15

Commit: `8d8b6d56`

Migration stage: 3, stateful theme-dialog lifecycle

Files/workflows converted: colour-manager Close and window dismissal; in-place
Reset response; staged-change finalization; parent destruction; nested live
picker data ownership and cleanup

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] source assertions: colour manager contains no nested dialog run
- [x] source assertions: Reset refreshes state and stops response emission
- [x] source assertions: all other responses finalize state and destroy the manager
- [x] source assertions: manager and nested picker are destroyed with their parent
- [x] source assertions: nested picker callback data is object-owned
- [x] source inventory: blocking dialog runs reduced from 6 to 5
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: stateful theme colour-manager responses and nested picker ownership.
The certificate chooser, preference confirmation, quit dialog, startup messages,
and synchronous `FE_MSG_WAIT` behavior remain unchanged.

### PR: [#49 - GTK4 Client-Certificate Chooser, Pass 20](https://github.com/Fabulor/fabulor/pull/49)

Date: 2026-07-15

Commit: `87576942`

Migration stage: 3, asynchronous file selection

Files/workflows converted: server-list client-certificate import chooser;
copied network and destination state; weak editor ownership; parent-destroy
cleanup; guarded button refresh; response-driven import result messages

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] source assertions: certificate import contains no nested dialog run
- [x] source assertions: local single-file selection and certificate filters are preserved
- [x] source assertions: network name, certificate root, and destination are copied before display
- [x] source assertions: editor destruction hides and releases the chooser
- [x] source assertions: accepted response retains private directory, copy, permissions, and result message
- [x] source assertions: cancel, missing filename, and retired parent remain silent
- [x] source inventory: legacy `GtkFileChooserDialog` reduced from 1 to 0
- [x] source inventory: 4 ordinary and 2 native blocking dialog runs remain
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: client-certificate file selection and editor-bound callback ownership.
Two native theme-import runs, preference confirmation, quit dialog, startup
messages, and synchronous `FE_MSG_WAIT` behavior remain unchanged.

### PR: [#50 - GTK4 Theme Import Choosers, Pass 21](https://github.com/Fabulor/fabulor/pull/50)

Date: 2026-07-15

Commit: `3c5d65f1`

Migration stage: 3, asynchronous native file selection

Files/workflows converted: colors.conf/HCT chooser; GTK3 theme archive chooser;
weak button and preferences-parent ownership; parent-destroy cleanup; live
callback-context resolution; response-driven import dispatch

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] source assertions: frontend contains no blocking native-dialog run
- [x] source assertions: both choosers retain local single-file selection and existing filters
- [x] source assertions: chooser requests weakly own launch button and preferences parent
- [x] source assertions: preferences destruction hides and releases each chooser
- [x] source assertions: accepted responses resolve live callback context before importing
- [x] source assertions: archive containment, staged colours, pevents, refresh, and messages are preserved
- [x] source assertions: cancel, missing filename, and retired owner remain silent
- [x] source inventory: 4 ordinary and 0 native blocking dialog runs remain
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] GTK4 validator unit tests: 8/8, plus repository root validation
- [x] isolated GTK4 MSVC compile, link, and runtime probe with no warnings
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] production GTK3 MSVC x64 Release rebuild and native tests: 18/18
- [x] GitHub Actions required checks: 5/5

Scope: native theme file selection and preferences-bound callback ownership.
Preference confirmation, quit dialog, startup messages, and synchronous
`FE_MSG_WAIT` behavior remain unchanged.

### PR: [#51 - GTK4 Theme Policy And Legacy Packaging Cleanup](https://github.com/Fabulor/fabulor/pull/51)

Date: 2026-07-15

Commit: `cfffaca4`

Migration stage: 7 planning and legacy packaging cleanup

Files/workflows converted: active WiX `.hct`-only association; retired `.zct`
association; legacy Inno mock-theme choices/downloads/extraction; MS-Windows
staging; incremental stale-output cleanup; final GTK4 theme policy

Automated checks:

- [x] source assertions: active installers register `.hct` and do not register `.zct`
- [x] source assertions: `.zct` remains only in explicit stale-install cleanup
- [x] source assertions: no mock Windows theme component, download, extraction, or payload rule remains
- [x] source assertions: GTK runtime and icon assets remain separate from optional theme payloads
- [x] WiX and MSBuild project XML parsing
- [x] MSVC x64 Release staging build with 0 warnings and 0 errors
- [x] staged `share\themes\MS-Windows` output is absent after an incremental build
- [x] WiX x64 Release MSI/bootstrapper build with 0 errors
- [x] GitHub Actions required checks: 5/5

Scope: policy, registration, legacy installer rules, and staging output. The
shipping GTK3 theme importer remains until the GTK4 adapter can replace it;
existing user `gtk3-themes` content is not deleted during upgrade.

### PR: [#52 - GTK4 Stage 3 Dialog Lifecycle Closure](https://github.com/Fabulor/fabulor/pull/52)

Date: 2026-07-15

Commit: `2c4a727f`

Migration stage: 3, final blocking-dialog closure

Files/workflows converted: manifest-plugin enable confirmation; quit and
minimize-to-tray confirmation; fatal font error; frontend modal-message
semantics; pre-event-loop Windows command-line information dialogs

Automated checks:

- [x] source inventory: 0 ordinary and 0 native blocking dialog runs remain
- [x] source assertions: accepted manifest confirmation alone resumes preference persistence
- [x] source assertions: quit, minimize, cancellation, and singleton cleanup remain response-owned
- [x] source assertions: fatal font failure suppresses unusable transcript rendering before acknowledgement
- [x] source assertions: Windows command-line information preserves synchronous pre-event-loop display through UTF-16 native UI
- [x] common MSVC x64 Release build with 0 warnings and 0 errors
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors

Scope: final nested-loop removal and Stage 3 closure. GTK message-dialog
presentation remains available during the staged production cutover, but no
converted path depends on a nested GTK event loop.

### PR: [#53 - GTK4 Stage 4 Shared Clipboard Boundary](https://github.com/Fabulor/fabulor/pull/53)

Date: 2026-07-15

Commit: `0ec70605`

Migration stage: 4, shared explicit-copy pass 1

Files/workflows converted: ban-list copy; channel name/topic copy; URL context
copy; URL-history copy; GTK3/GTK4 standard and primary clipboard helper

Automated checks:

- [x] source assertions: all five shared-copy callers use the typed two-argument API
- [x] source assertions: direct GTK3 clipboard calls remain only in transcript selection ownership
- [x] source assertions: GTK4 standard and primary clipboards are resolved from the widget display
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] isolated GTK4 MSVC compile, link, and runtime probe with 0 warnings and 0 errors
- [x] GTK4 validator unit tests: 8/8 under WSL
- [ ] Meson probe: local run blocked by stale administrator-owned build output and unavailable Ninja; clean CI pending

Scope: explicit copy commands only. Transcript selection, paste, input,
spell-check, emoji, drag/drop, and direct event handling remain unchanged.

### PR: [#54 - GTK4 Stage 4 Simple Controller Foundation](https://github.com/Fabulor/fabulor/pull/54)

Date: 2026-07-15

Commit: `db4699ed`

Migration stage: 4, simple controller pass 2

Files/workflows converted: Character Chart pointer hover; Join dialog entry
focus; theme-colour entry focus-loss commit; scroll-to-bottom button activation;
typed GTK3/GTK4 pointer and focus interaction ownership

Automated checks:

- [x] source assertions: converted workflow callbacks expose no `GdkEvent` parameters
- [x] source assertions: GTK4 pointer and focus paths use motion/focus controllers
- [x] source assertions: GTK3 branches retain enter/focus event behavior behind the typed boundary
- [x] source assertions: scroll-to-bottom uses semantic button activation
- [x] source inventory: direct `GdkEvent` references reduced to 118 lines across 23 files
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] isolated GTK4 MSVC compile, link, and runtime probe with 0 warnings and 0 errors
- [x] GTK4 validator unit tests: 8/8 under WSL

Scope: simple pointer-enter, focus, and button interactions only. Message
entry, transcript, spell-check, emoji, menu, and drag/drop events remain
unchanged.

### PR: [#55 - GTK4 Stage 4 Channel Scroll Controller](https://github.com/Fabulor/fabulor/pull/55)

Date: 2026-07-15

Commit: `5160d299`

Migration stage: 4, channel switcher scroll pass 3

Files/workflows converted: channel tree wheel; tab viewport wheel; channel tab
wheel; tab close-button wheel; smooth/discrete GTK3 normalization; GTK4
capture-phase scroll ownership

Automated checks:

- [x] source assertions: channel switcher workflow exposes no `GdkEventScroll`
- [x] source assertions: all four scroll surfaces use the typed delta callback
- [x] source assertions: GTK4 uses a both-axis capture-phase scroll controller
- [x] source assertions: GTK3 preserves smooth and four-way discrete normalization
- [x] source assertions: unhandled tree movement remains available to native scrolling
- [x] source inventory: direct `GdkEvent` references reduced to 116 lines across 23 files
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] isolated GTK4 MSVC compile, link, and runtime probe with 0 warnings and 0 errors
- [x] GTK4 validator unit tests: 8/8 under WSL

Scope: channel switcher wheel input only. Transcript scrolling, message input,
spell-check, emoji, menus, and drag/drop remain unchanged.

### PR: [#56 - GTK4 Stage 4 Main Focus Controllers](https://github.com/Fabulor/fabulor/pull/56)

Date: 2026-07-15

Commit: `c099ad84`

Migration stage: 4, main focus pass 4

Files/workflows converted: detached message-entry focus; standalone session
window focus; shared tab-window focus; session selection; marker/plugin and
taskbar-flash focus effects

Automated checks:

- [x] source assertions: production source contains no direct `focus-in-event` connection
- [x] source assertions: converted focus callbacks expose no `GdkEventFocus`
- [x] source assertions: all three workflows use the typed focus-enter boundary
- [x] source inventory: direct `GdkEvent` references reduced to 113 lines across 23 files
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] isolated GTK4 MSVC compile, link, and runtime probe with 0 warnings and 0 errors
- [x] GTK4 validator unit tests: 8/8 under WSL

Scope: focus entry only. Message activation, key handling, completion, history,
spell-check, emoji, transcript, menus, and drag/drop remain unchanged.

### PR: [#57 - GTK4 Stage 4 Channel Tab Close Hover](https://github.com/Fabulor/fabulor/pull/57)

Date: 2026-07-15

Commit: `f660b8f3`

Migration stage: 4, channel-tab close hover pass 5

Files/workflows converted: channel-tab close-button pointer motion and leave;
close-area hit testing; close-button prelight and pointing-cursor cleanup

Automated checks:

- [x] source assertions: converted hover callbacks expose no `GdkEventMotion` or `GdkEventCrossing`
- [x] source assertions: tab hover uses the typed motion/leave and widget-cursor boundaries
- [x] source assertions: click, context-menu, scrolling, pressed/toggled, and outer-tab prelight handlers remain unchanged
- [x] source inventory: direct `GdkEvent` references remain centralized at 113 lines across 23 files
- [x] source inventory: `gtk_widget_get_window` reduced from 37 to 32 lines across 7 files
- [x] source inventory: `gdk_window_*` reduced from 51 to 49 lines across 9 files
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] isolated GTK4 MSVC compile, link, and runtime probe with 0 warnings and 0 errors
- [x] GTK4 validator unit tests: 8/8 under WSL

Scope: close-button hover and leave cleanup only. Left-click close dispatch,
right-click context menus, scrolling, tab activation, outer-tab prelight
suppression, message input, transcript, spell-check, emoji, menus, and
drag/drop remain unchanged.

### PR: [#58 - GTK4 Stage 4 Simple Key Handlers](https://github.com/Fabulor/fabulor/pull/58)

Date: 2026-07-15

Commit: `0e80e2ac`

Migration stage: 4, simple keyboard controller pass 6

Files/workflows converted: detached utility-window Escape; search-bar Escape;
raw-log `Ctrl+Shift+C`; shared typed key-value and modifier-state boundary

Automated checks:

- [x] source assertions: converted callbacks expose no `GdkEventKey`
- [x] source assertions: all three workflows use the typed key-controller boundary
- [x] source assertions: GTK4 uses a bubble-phase `GtkEventControllerKey`
- [x] source assertions: converted callbacks retain non-consuming propagation
- [x] source inventory: direct `GdkEvent` references reduced from 113 lines across 23 files to 111 lines across 21 files
- [x] source inventory: `gtk_widget_get_window` reduced from 32 lines across 7 files to 31 lines across 6 files
- [x] source inventory: `gdk_window_*` reduced from 49 lines across 9 files to 48 lines across 8 files
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] isolated GTK4 MSVC compile, link, and runtime probe with 0 warnings and 0 errors
- [x] GTK4 validator unit tests: 8/8 under WSL

Scope: three simple independent key workflows only. Main configurable
shortcuts, completion, history, topic editing, tree/list navigation,
transcript input, spell-check, emoji, menus, and drag/drop remain unchanged.

### PR: [#59 - GTK4 Stage 4 Semantic Key Actions](https://github.com/Fabulor/fabulor/pull/59)

Date: 2026-07-15

Commit: `b60ceb84`

Migration stage: 4, semantic key actions pass 7

Files/workflows converted: topic Return and keypad Enter submission; exact
`Ctrl+A` selection in channel key and user-limit fields

Automated checks:

- [x] source assertions: converted callbacks expose no `GdkEventKey`
- [x] source assertions: all three registrations use the typed key-controller boundary
- [x] source assertions: handled topic submission and selection remain consuming
- [x] source assertions: the main chat input shortcut connection remains unchanged
- [x] source inventory: direct `GdkEvent` references reduced from 111 to 109 lines across 21 files
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] isolated GTK4 MSVC compile, link, and runtime probe with 0 warnings and 0 errors
- [x] GTK4 validator unit tests: 8/8 under WSL

Scope: topic submission and channel-mode entry selection only. Main
configurable shortcuts, completion, history, tree/list navigation, transcript
input, spell-check, emoji, menus, and drag/drop remain unchanged.

### PR: [#60 - GTK4 Stage 4 Main Shortcut Engine](https://github.com/Fabulor/fabulor/pull/60)

Date: 2026-07-15

Commit: `73c71473`

Migration stage: 4, main shortcut engine pass 8

Files/workflows converted: main chat-input key dispatch; plugin keypress
notification; configurable binding lookup; all 17 shortcut action contracts

Automated checks:

- [x] source assertions: main input and shortcut action table expose no `GdkEventKey`
- [x] source assertions: main input uses the typed key-controller boundary
- [x] source assertions: plugin notification retains key state, key value, and Unicode derivation
- [x] source assertions: binding lookup retains exact filtered modifiers and action return behavior
- [x] source assertions: shortcut-editor Shift+Up/Down remains isolated for Stage 5
- [x] source inventory: direct `GdkEvent` references reduced from 109 lines across 21 files to 71 lines across 20 files
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] isolated GTK4 MSVC compile, link, and runtime probe with 0 warnings and 0 errors
- [x] GTK4 validator unit tests: 8/8 under WSL

Scope: main chat-input plugin and configurable shortcut dispatch only. The
shortcut editor, other tree/list navigation, topic pointer actions, transcript
selection, spell-check widget internals, emoji, menus, and drag/drop remain
unchanged.

### PR: [#61 - GTK4 Stage 4 Topic Pointer Handling](https://github.com/Fabulor/fabulor/pull/61)

Date: 2026-07-15

Commit: `716e9fb6`

Migration stage: 4, topic pointer pass 9

Files/workflows converted: topic URL hover and leave; text-view pointer/text
cursor; modified left-button release and URL activation

Automated checks:

- [x] source assertions: topic workflow exposes no `GdkEventMotion`, `GdkEventCrossing`, or `GdkEventButton`
- [x] source assertions: topic widget has no direct motion, leave, button-release, or event-mask setup
- [x] source assertions: GTK4 click delivery uses `GtkGestureClick` and claims only handled activation
- [x] source assertions: URL hit testing retains text-view buffer-coordinate conversion and modifier matching
- [x] source inventory: direct `GdkEvent` references reduced from 71 to 69 lines across 20 files
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] isolated GTK4 MSVC compile, link, and runtime probe with 0 warnings and 0 errors
- [x] GTK4 validator unit tests: 8/8 under WSL

Scope: topic URL pointer interaction only. Topic editing and Return submission,
shortcut-editor navigation, other tree/list input, transcript selection,
spell-check widget internals, emoji, menus, and drag/drop remain unchanged.

### PR: [#62 - GTK4 Stage 4 External File Drops](https://github.com/Fabulor/fabulor/pull/62)

Date: 2026-07-15

Commit: `223d37a8`

Migration stage: 4, external file-drop pass 10

Files/workflows converted: private-dialog transcript file drops; user-list
nickname file drops; URI-list parsing and DCC-send result reporting

Automated checks:

- [x] source assertions: converted consumers expose no `GtkSelectionData` or `GdkDragContext`
- [x] source assertions: GTK4 file delivery uses `GdkFileList` and `GtkDropTarget`
- [x] source assertions: GTK3 bridge validates `text/uri-list` and copies its explicit byte length
- [x] source assertions: common file sender uses `g_uri_list_extract_uris`
- [x] source assertions: private-dialog and pointer-resolved nickname targeting remain unchanged
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] isolated GTK4 MSVC compile, link, and runtime probe with 0 warnings and 0 errors
- [x] isolated GTK4 Meson configure, compile, link, and runtime test with explicit GIO linkage
- [x] GTK4 validator unit tests: 8/8 under WSL

Scope: external file drops only. Internal channel-view, user-list, scrollbar,
and pane-position drag operations, tree/list conversion, transcript selection,
spell-check widget internals, emoji, menus, and other input remain unchanged.

CI follow-up: the first Windows workflow run exposed that the Meson probe did
not yet mirror the MSVC probe's `gio-2.0` import library. The Meson dependency
list was corrected and the validator suite plus both Windows probe paths passed
locally before the workflow was rerun.

### PR: [#63 - GTK4 Stage 4 Internal Layout Drag And Drop](https://github.com/Fabulor/fabulor/pull/63)

Date: 2026-07-15

Commit: `8f07296c`

Migration stage: 4, internal layout drag/drop pass 11

Files/workflows converted: channel-view and user-list drag sources; reciprocal
drop targets; transcript scrollbar target; user-list file/internal hover;
four-position channel-view/user-list pane placement

Automated checks:

- [x] source assertions: consumers expose no `GdkDragContext` or drag `GtkSelectionData`
- [x] source assertions: GTK4 sources and targets use typed controllers and a process-local pointer payload
- [x] source assertions: GTK3 target-name decoding is exact and retains `GTK_TARGET_SAME_APP`
- [x] source assertions: source identity is independent of selected MOVE/COPY action codes
- [x] source assertions: user-list hover selection is released on leave and completed drop
- [x] source inventory: raw context/selection references reduced from 14 lines across 5 files to 11 lines across 2 files
- [x] source inventory: the sole consumer reference is transcript clipboard ownership in `xtext.c`
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] isolated GTK4 MSVC compile, link, payload-format assertion, and runtime probe with 0 warnings and 0 errors
- [x] isolated GTK4 Meson compile, link, payload-format assertion, and runtime test
- [x] GTK4 validator unit tests: 8/8

Scope: internal layout drag/drop and user-list hover lifecycle only. Channel and
user tree/list model conversion, transcript clipboard/selection ownership,
spell-check widget internals, emoji, menus, and other input remain unchanged.

### PR: [#64 - GTK4 Stage 5 List Model Architecture](https://github.com/Fabulor/fabulor/pull/64)

Date: 2026-07-15

Implementation commit: `55e914f8`

Migration stage: 5, model architecture pass 1

Files/workflows converted: GTK4-only flat sorted multi-selection stack;
hierarchical single-selection stack; isolated model contract probes

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] flat model contract: sorted insertion, identity removal, selection persistence, and cleanup
- [x] tree model contract: expansion, child depth, identity, selection persistence, and cleanup
- [x] isolated GTK4 MSVC compile, link, and runtime probe with 0 warnings and 0 errors
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] GTK4 validator unit tests: 8/8
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] source inventory: 80 `GtkTreeView` type lines across 18 production files

Scope: architecture and executable model contracts only. Production GTK3
models, views, cell renderers, event handling, and visible workflows remain
unchanged until each owning Stage 5 surface is converted as one unit.

### PR: [#65 - GTK4 Stage 5 Notify List](https://github.com/Fabulor/fabulor/pull/65)

Date: 2026-07-16

Implementation commit: `93088543`

Migration stage: 5, Notify List pass 2

Files/workflows converted: Notify row snapshots; refresh reconciliation;
single selection; Open Dialog and Remove identity; row colours; GTK4
`GtkColumnView` factories; model and factory cleanup

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] Notify model contract: duplicate identity rejection and copied snapshots
- [x] Notify refresh contract: reorder, row reuse, and unchanged-order fast path
- [x] selection contract: exact persistence and notify-owner fallback across offline/online transitions
- [x] action contract: selected owner name and per-server data resolve without backward row traversal
- [x] GTK4 factories: four columns compile and link with explicit Pango colour dependency and binding cleanup
- [x] isolated GTK4 MSVC compile, link, and runtime probe with 0 warnings and 0 errors
- [x] isolated GTK4 Meson compile, link, and runtime test
- [x] GTK4 validator unit tests: 8/8
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] source boundary: zero direct legacy tree/list APIs remain in `notifygui.c`
- [x] source inventory: `GtkTreeView` type references reduced from 80 to 76 lines across 18 files

Manual checks:

- [ ] production GTK4 Notify List visual, keyboard, activation, and accessibility checks await frontend cutover

Scope: the Notify List owner, refresh, selection, actions, and GTK4 factories.
The production executable continues to render the owner's GTK3 branch until
the frontend build target changes to GTK4; other operational lists are
unchanged.

### PR: [#66 - GTK4 Stage 5 User List Model Ownership](https://github.com/Fabulor/fabulor/pull/66)

Date: 2026-07-16

Implementation commit: `e6d30e26`

Migration stage: 5, user-list model pass 3

Files/workflows converted: per-session model lifetime; typed row snapshots;
stable user identity; insert, update, remove, clear, typing refresh; privilege
and alphabetic ordering; GTK4 sorted multi-selection model access

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] user model contract: duplicate insert and missing update/remove rejection
- [x] ordering contract: ascending, descending, and unsorted insertion modes
- [x] update contract: external sort-key change reorders stable row identity
- [x] lifetime contract: identity removal, clear, and owner cleanup
- [x] GTK4 model contract: typed rows, sorted list, and `GtkMultiSelection` access
- [x] isolated GTK4 MSVC compile, link, and runtime probe with 0 warnings and 0 errors
- [x] isolated GTK4 Meson compile, link, and runtime test
- [x] GTK4 validator unit tests: 8/8
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] source boundary: no direct per-session toolkit model or row-reference ownership remains
- [x] source inventory: only five GTK3 row-reference lines remain inside the owner
- [x] source inventory: `GtkTreeView` type references reduced from 76 to 75 lines across 18 files

Manual checks:

- [ ] production user-list insert/update/remove, all sort modes, typing, and tab switching
- [ ] GTK4 factories, selection, menus, keyboard, drag/drop, accessibility, and load checks await pass 4 and frontend cutover

Scope: per-session user-list model data, sorting, identity, and lifetime only.
The shared GTK3 view and its visible interaction paths remain unchanged for the
next user-list pass.

### PR: [#67 - GTK4 Stage 5 User List View Ownership](https://github.com/Fabulor/fabulor/pull/67)

Date: 2026-07-16

Migration stage: 5, user-list view pass 4

Files/workflows converted: shared view ownership; GTK4 list-item factory;
active model attachment; multi-selection snapshots; select/toggle/clear;
scrolling; live-position hit-testing; double-click and keyboard controllers;
nick-menu positioning; internal and external drag/drop row targeting

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] strict GTK4 MSVC compile, link, and runtime probe with 0 warnings and 0 errors
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] GTK4 dependency validator unit tests: 8/8
- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] GTK4 factory uses current list-item positions and disconnects row notifications on unbind
- [x] GTK4 icon path uses owned, non-deprecated `GdkMemoryTexture` data
- [x] source boundary: `userlistgui.c` has no live tree-model, iterator, path, column, or selection operations
- [x] source inventory: five GTK3 row-reference lines remain contained in the model owner
- [x] source inventory: `GtkTreeView` references reduced from 75 to 65 lines across 17 files

Manual checks:

- [ ] shipping GTK3 user-list selection, nick menu, double-click command, keyboard forwarding, file drop, and tab switching
- [ ] GTK4 row layout, selection, menus, keyboard, drag/drop, accessibility, and large-channel load await frontend cutover

Scope: the shared user-list view and interaction boundary. The shipping GTK3
frontend uses the retained branch; production GTK4 visual and behavioural
validation remains a cutover requirement.

### PR: [#68 - GTK4 Stage 5 Channel Navigation Model Ownership](https://github.com/Fabulor/fabulor/pull/68)

Date: 2026-07-16

Migration stage: 5, channel-navigation model pass 5

Files/workflows converted: channel hierarchy ownership; stable identity;
root/child traversal; presentation updates; cyclic sibling movement;
reparenting; removal; GTK4 tree-list and selection models; contained GTK3 tree
storage and on-demand row references

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] full production GTK3 frontend rebuild with 0 warnings and 0 errors
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] GTK4 dependency validator unit tests: 8/8
- [x] channel-model probe covers duplicate rejection, hierarchy order, rename, cyclic movement, reparenting, removal, and selection persistence
- [x] source boundary: shared `chanview.c` has no tree-store, iterator, or row-reference operations
- [x] source boundary: channel records no longer retain `GtkTreeIter`
- [x] GTK3 compatibility row references are contained in the model owner and resolved only on demand
- [x] tab switcher no longer creates a hidden `GtkTreeView` for shared storage

Manual checks:

- [ ] shipping GTK3 tree/tab switching, rename, colours, close/reparent, movement, context menus, and drag/drop
- [ ] GTK4 factories, expansion, selection, menus, keyboard, drag/drop, accessibility, and load checks await the visible-view pass and frontend cutover

Scope: channel hierarchy model ownership and stable mutation semantics. The
shipping GTK3 tree and tab widgets remain visible; the GTK4 channel switcher is
the next contained Stage 5 pass.

### PR: [#69 - GTK4 Stage 5 Channel Tree View Ownership](https://github.com/Fabulor/fabulor/pull/69)

Date: 2026-07-16

Migration stage: 5, channel-tree view pass 6

Files/workflows converted: hierarchical channel view ownership; GTK4
`GtkListView` and `GtkTreeExpander` factory; optional icon and attributed-name
binding; expansion; identity selection; callback dispatch; hit-testing;
focus scrolling; teardown and selection-listener cleanup; contained GTK3 tree
view, renderer, iterator, and path operations

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and Meson 1.11.2 Release with MSVC 19.44

Automated checks:

- [x] strict GTK4 MSVC compile, link, and runtime probe with 0 warnings and 0 errors
- [x] full production GTK3 frontend rebuild with 0 warnings and 0 errors
- [x] isolated GTK4 Meson configure, compile, link, and runtime test
- [x] GTK4 dependency validator unit tests: 8/8
- [x] GTK4 runtime probe constructs the channel tree, expands its root, selects a child by identity, dispatches one selection callback, and destroys the view
- [x] post-destruction probe reuses the model selection with GLib critical diagnostics treated as fatal
- [x] source boundary: `chanview-tree.c` has no direct tree-view, renderer, iterator, or path operations
- [x] factory unbind disconnects row notifications and view teardown disconnects its longer-lived selection listener

Manual checks:

- [ ] shipping GTK3 tree switching, expand/collapse, context menu, rename, colours, close/reparent, movement, and drag/drop
- [ ] GTK4 row layout, icon and text alignment, pointer hit-testing, keyboard focus, accessibility, and large-channel load await frontend cutover

Scope: the hierarchical channel-tree presentation. The grouped tab strip,
close controls, animated scrolling, context actions, and family reordering are
the next contained Stage 5 pass.

### PR: [#70 - GTK4 Stage 5 Channel Context Input Boundary](https://github.com/Fabulor/fabulor/pull/70)

Date: 2026-07-16

Migration stage: 5, channel close/context input pass 7

Files/workflows converted: shared channel context callback; tree and tab
multi-click controllers; tree identity hit-testing; tab close-button hit area;
middle-click close; right-click tab menu placement and widget fallback

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 compatibility probe covers the shared multi-click controller signature
- [x] source boundary: no `GdkEventButton` or `button-press-event` remains in shared channel view, tree, or tab code
- [x] source boundary: the channel callback API carries only widget, button, coordinates, and modifier state
- [x] retained GTK3 popup has pointer-event placement and widget-anchored fallback paths

Manual checks:

- [ ] shipping GTK3 tab close button, optional middle-click close, right-click tab/tree menu, and empty-area clicks
- [ ] GTK4 gesture claiming, popup placement, and close/context behavior await frontend cutover

Scope: pointer input and callback type containment only. Grouped tab family
layout, close presentation, animated scrolling, focus, and reordering remain
for Stage 5 pass 8.

### PR: [#71 - GTK4 Stage 5 Grouped Tab State Ownership](https://github.com/Fabulor/fabulor/pull/71)

Date: 2026-07-16

Migration stage: 5, grouped-tab state pass 8

Files/workflows converted: per-view forward/backward animation slots and flags;
per-view toggle suppression; timeout cancellation during cleanup; model-based
absolute and relative focus movement; removal of unused scroll-button sizing
state

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] source boundary: no process-global tab animation pointer, movement flag, or toggle guard remains
- [x] source boundary: tab focus movement no longer enumerates GTK family-box or toggle-button children
- [x] cleanup cancels both per-view animations before destroying the tab widget tree
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 relative/absolute tab switching and horizontal/vertical wheel animation
- [ ] simultaneous detached-window scrolling and cleanup during animation
- [ ] GTK4 grouped-tab presentation awaits the remaining owner pass

Scope: grouped-tab transient state, focus lookup, and animation lifetime only.
Family-box construction, close presentation, updates, and widget reordering
remain in the legacy tab implementation for the next contained pass.

### PR: [#72 - GTK4 Stage 5 Grouped Tab Reordering](https://github.com/Fabulor/fabulor/pull/72)

Date: 2026-07-16

Migration stage: 5, grouped-tab reorder pass 9

Files/workflows converted: child-tab reorder lookup; server/root family reorder
lookup; authoritative model-to-widget position mirroring

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] child-tab movement resolves sibling order from the channel model and preserves the leading server-tab offset
- [x] server/root movement repositions the complete family from the model root order
- [x] reorder paths do not enumerate GTK children or rediscover family identity through widget data
- [x] strict GTK4 probe and channel-model validator remain green
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 child-tab and server-family movement in horizontal and vertical tab layouts
- [ ] cyclic first/last movement with multiple servers and channels
- [ ] GTK4 grouped-tab presentation awaits the remaining owner conversion

Scope: grouped-tab reorder lookup and dispatch only. Family-box construction,
close presentation, animated scrolling presentation, keyboard handling, and
drag/drop remain in the legacy tab implementation for the next contained pass.

### PR: [#73 - GTK4 Stage 5 Grouped Tab Family Ownership](https://github.com/Fabulor/fabulor/pull/73)

Date: 2026-07-16

Migration stage: 5, grouped-tab family-owner pass 10

Files/workflows converted: per-view root-to-family ownership; model-position
child insertion; deterministic root-family creation and removal; family-map
cleanup during orientation, implementation, and view teardown

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] family discovery and empty-box pruning no longer enumerate GTK children
- [x] family identity is not stored on GTK widgets
- [x] child insertion resolves authoritative model parent and sibling position
- [x] a compile-time assertion protects the channel-view implementation scratch boundary
- [x] strict GTK4 probe and dependency validator remain green
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 server/channel add, remove, reconnect, and root-child reparent workflows
- [ ] tabs/tree switching and horizontal/vertical orientation changes with several families
- [ ] sorted dialog tabs retain their expected order
- [ ] GTK4 grouped-tab presentation awaits the remaining presentation passes

Scope: grouped-tab family identity and lifetime only. Close presentation,
scroll geometry and animation presentation, keyboard handling, and drag/drop
remain in the legacy tab implementation for later contained passes.

### PR: [#74 - GTK4 Stage 5 Grouped Tab Presentation Ownership](https://github.com/Fabulor/fabulor/pull/74)

Date: 2026-07-16

Migration stage: 5, grouped-tab item-owner pass 11

Files/workflows converted: per-view channel-to-item ownership; tab, label, and
close-button identity; rename and colour updates; close hit-testing and hover
cleanup lookup; item removal and whole-view teardown ordering

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] no tab label, close-button, or channel identity remains in GTK object data
- [x] close callbacks, rename, colour, and removal resolve channel-owned item records
- [x] widgets are destroyed before whole-view item records are released
- [x] the compile-time assertion still protects the channel-view implementation scratch boundary
- [x] strict GTK4 probe and dependency validator remain green
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 tab rename and colour updates across server, channel, dialog, and utility tabs
- [ ] close-button visibility, hover, click, context menu, and cursor behavior
- [ ] tabs/tree switching and orientation changes with active and background tabs
- [ ] GTK4 close geometry and hover presentation await the next contained pass

Scope: grouped-tab item identity and presentation lifetime only. Cross-version
close geometry and hover presentation, animated scrolling presentation,
keyboard handling, and drag/drop remain for later contained passes.

### PR: [#75 - GTK4 Stage 5 Grouped Tab Close Presentation](https://github.com/Fabulor/fabulor/pull/75)

Date: 2026-07-16

Migration stage: 5, grouped-tab close-presentation pass 12

Files/workflows converted: descendant close-button hit-testing; close hover
prelight; pointer cursor feedback; whole-tab prelight suppression; strict GTK4
compatibility helper signatures

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles descendant geometry and prelight helper branches and passes its runtime identity check
- [x] grouped-tab code has no raw crossing-event type or enter/leave signal connection
- [x] grouped-tab code has no direct allocation read or coordinate translation for close hit-testing
- [x] close dispatch still consumes a visible close-button left click before context fallback
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 close hover, pointer cursor, left-click close, right-click context, and hidden-close-button behavior
- [ ] server, channel, dialog, and utility tabs under horizontal and vertical layouts
- [ ] GTK4 transformed close hit area and theme prelight await frontend cutover

Scope: grouped-tab close geometry and hover presentation only. Animated
scrolling presentation, keyboard handling, and drag/drop remain for later
contained passes.

### PR: [#76 - GTK4 Stage 5 Grouped Tab Scroll Presentation](https://github.com/Fabulor/fabulor/pull/76)

Date: 2026-07-16

Migration stage: 5, grouped-tab scroll-presentation pass 13

Files/workflows converted: model-driven forward/backward target discovery;
cross-version descendant-origin geometry; explicit scrolled-window ownership;
horizontal/vertical adjustment lookup

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles descendant-origin geometry and passes its runtime identity check
- [x] grouped-tab code contains no GTK child enumeration
- [x] scroll target discovery follows authoritative flattened model order and item identity
- [x] grouped-tab code contains no direct allocation read or coordinate translation
- [x] adjustment lookup uses the explicitly retained scrolled-window owner
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 horizontal and vertical wheel scrolling with channel switching enabled and disabled
- [ ] forward/backward animation across several server families and partial viewport positions
- [ ] rapid direction changes, orientation changes, detached windows, and teardown during animation
- [ ] GTK4 tab offset transforms and animation await frontend cutover

Scope: grouped-tab scroll target geometry and adjustment ownership. Keyboard
handling and drag/drop remain for later contained passes.

### PR: [#77 - GTK4 Stage 5 Grouped Tab Activation](https://github.com/Fabulor/fabulor/pull/77)

Date: 2026-07-16

Migration stage: 5, grouped-tab activation pass 14

Files/workflows converted: mouse press activation; keyboard toggle activation;
close-button dispatch priority; final grouped-tab drag/drop boundary audit

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles the shared input boundary and passes its runtime identity check
- [x] grouped-tab code contains no direct GTK3-only `pressed` signal dependency
- [x] left-click activation uses the shared multi-click press controller after close-button dispatch
- [x] keyboard activation remains on the cross-version `GtkToggleButton` `toggled` signal
- [x] source audit finds no tab-local drag/drop implementation; complete channel-view placement uses the typed Stage 4 boundary
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 left-click focus timing and Space-key activation
- [ ] close-button, middle-click close, and right-click context behavior
- [ ] horizontal/vertical layouts, detached windows, and tree/tab switching
- [ ] internal channel-view placement drag/drop
- [ ] GTK4 activation and placement behavior await frontend cutover

Scope: grouped-tab activation and final drag/drop audit. The grouped-tab owner
conversion is complete; remaining Stage 5 work covers operational lists and
editors.

### PR: [#78 - GTK4 Stage 5 Loaded Add-ons List](https://github.com/Fabulor/fabulor/pull/78)

Date: 2026-07-16

Migration stage: 5, loaded Add-ons list pass 15

Files/workflows converted: loaded add-on row ownership; GTK4 column factories;
single-selection name/path lookup; refresh, unload, and reload integration

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles and executes the Add-on List owner
- [x] GTK4 owner append, row-count, clear, and cleanup checks pass
- [x] `plugingui.c` contains no direct tree-model, iterator, list-store, or tree-selection dependency
- [x] production and strict build definitions include the new owner
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 Add-ons window row contents, column sizing, and selection
- [ ] Load, Unload, and Reload actions for native, Python, and Tcl add-ons
- [ ] refresh after load/unload and window close/reopen cleanup
- [ ] GTK4 column presentation and selection-driven actions await frontend cutover

Scope: the loaded Add-ons table and its selection-dependent actions only.
Add-on loading policy, file selection, path containment, and command dispatch
are unchanged.

### PR: [#79 - GTK4 Stage 5 URL History List](https://github.com/Fabulor/fabulor/pull/79)

Date: 2026-07-16

Migration stage: 5, URL History list pass 16

Files/workflows converted: URL row ownership; newest-first limit enforcement;
GTK4 list-item factory; selection, copy, clear, double-click, and context lookup

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles and executes the URL History owner
- [x] GTK4 prepend order, configured limit, row lookup, clear, and cleanup checks pass
- [x] `urlgrab.c` contains no direct tree-model, path, iterator, selection, list-store, or raw button-event dependency
- [x] production and strict build definitions include the new owner
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 URL History population, newest-first ordering, and configured limit
- [ ] single selection, Copy, Clear, Save As, and disabled-state message
- [ ] double-click browser opening and right-click menu placement/actions
- [ ] close/reopen cleanup and additions while the window is open
- [ ] GTK4 row presentation and selection-driven actions await frontend cutover

Scope: URL History model/view ownership and row interaction only. URL capture,
logging, browser dispatch, save-file behavior, and URL menu contents are
unchanged.

## Per-PR Record Template

Copy this section for each GTK4 PR:

```text
### PR: <number and title>

Date:
Commit:
Migration stage:
Files/workflows converted:
GTK version:
GLib version:
Build configuration:

Automated checks:
- [ ] MSVC x64 Release
- [ ] Meson build/tests
- [ ] repository lint
- [ ] CodeQL
- [ ] focused unit tests
- [ ] installer build

Manual checks:
- [ ] converted workflow
- [ ] keyboard-only operation
- [ ] high-DPI layout
- [ ] repeated open/close
- [ ] error and cancellation paths

Performance observations:
Screenshots/artifacts:
Known regressions:
Follow-up:
```

## Build Matrix

| Check | GTK3 shipping target | GTK4 candidate | Final GTK4 target |
|---|---|---|---|
| MSVC x64 Release | required until cutover | required when target exists | required |
| Meson build | required where dependencies are available | required when target exists | required |
| Native tests | required | required | required |
| Repository lint | required | required | required |
| CodeQL C/C++ | required | required | required |
| WiX MSI/bootstrapper | required for packaging changes | required before cutover | required |
| Clean install | baseline | required before cutover | required |
| In-place upgrade | baseline | required before cutover | required |

## Core Workflow Matrix

| Workflow | Required checks | Status |
|---|---|---|
| Startup and shutdown | normal, safe mode, repeated restart, clean exit | not run on GTK4 |
| Network lifecycle | connect, TLS, proxy, reconnect, disconnect, ZNC | not run on GTK4 |
| Channel use | join/part, switch, topic, modes, user list, nick menu | not run on GTK4 |
| Messaging | type, paste, Enter, actions, notices, queries, history | not run on GTK4 |
| Transcript | colours, timestamps, wrap, search, URLs, selection, copy | not run on GTK4 |
| Scrollback | replay, marker, wheel, Page Up/Down, scroll-to-bottom | not run on GTK4 |
| Completion | nick/channel/command completion and popup behaviour | not run on GTK4 |
| Spell check | underline, suggestions, replace, add, persistence, URLs | not run on GTK4 |
| Emoji and flags | picker open/search/page/insert and flag rendering | not run on GTK4 |
| Server list | create/edit/delete/reorder/connect and password storage | not run on GTK4 |
| Preferences | every page, save/cancel, restart-only settings | not run on GTK4 |
| Operational lists | channel, notify, ignore, ban, DCC, key bindings | not run on GTK4 |
| Themes | built-in, custom CSS, dark mode, import, persistence | not run on GTK4 |
| Tray/notifications | states, click actions, hide/show, notification click | not run on GTK4 |
| Add-ons/plugins | built-ins, simple add-ons, manifest C#/Python/Tcl | not run on GTK4 |
| Updater | check, UI, cancellation, installer handoff | not run on GTK4 |

## Input And Performance Gates

Measure on the same machine and representative network/session where possible.
Record raw observations rather than only "feels fast".

| Scenario | Baseline to preserve | GTK4 result |
|---|---|---|
| Continuous edit-box typing | no visible keystroke backlog | not measured |
| Enter-to-local-echo | no sustained delay after warm-up | not measured |
| URL paste with spell check | no crash or prolonged input stall | not measured |
| Emoji picker first open | responsive with cached subsequent pages | not measured |
| Large scrollback wheel/Page Up | continuous and correctly framed | not measured |
| Busy channel updates | input remains responsive | not measured |
| Large channel/user list | sorting and selection remain responsive | not measured |
| Startup to usable server window | no material regression without cause | not measured |

For rendering changes, capture CPU usage and frame behaviour where practical.
For regressions, record profile size, scrollback size, plugin set, spell-check
state, network, and lag meter value.

## Visual Matrix

Capture screenshots for affected surfaces at:

- 100%, 125%, 150%, and 200% Windows scale where available
- normal and maximized windows
- narrow and wide layouts
- tabs and tree channel-switcher modes
- light, dark, and custom theme states
- long translated labels where translation assets are available

Check:

- no clipped labels, buttons, rows, icons, menus, or dialog content
- no overlapping edit box, topic bar, transcript, user list, or status areas
- stable row heights and icon alignment
- crisp text, emoji, flags, tray icons, and high-DPI assets
- sensible focus indication, selection colours, and disabled states
- popup/menu placement on multi-monitor and mixed-DPI setups

## Keyboard And Accessibility Matrix

- traverse every converted control with keyboard only
- preserve menu accelerators and configurable key bindings
- verify focus order, default buttons, Escape/cancel, and Enter activation
- verify transcript and list selection semantics
- inspect accessible names/roles/states for custom controls
- test screen-reader exposure on Windows when custom widgets change
- preserve high-contrast/theme readability
- avoid pointer-only access to required commands

## Runtime And Packaging Matrix

| Check | Status |
|---|---|
| `fabulor.exe` imports GTK4 and not GTK3 | not run |
| no duplicate GLib/GObject/GIO families loaded | not run |
| executable-relative runtime works from unrelated CWD | not run |
| clean install has allowlisted GTK4 payload | not run |
| upgrade removes legacy GTK3 payload | not run |
| repair restores required GTK4 files | not run |
| uninstall removes GTK payload and preserves profile | not run |
| fontconfig, pixbuf loaders, schemas, icons, and locales resolve | not run |
| Enchant/WinSpell smoke test passes against final runtime | not run |
| built-in and manifest plugins load against final runtime | not run |
| MSI/bootstrapper hashes recorded | not run |
| payload manifest/SBOM matches installed files | not run |

## Crash And Diagnostic Evidence

For GTK4 crashes or hangs, record:

- exact build commit and runtime versions
- Event Viewer exception code/module/offset
- debugger stack with symbols
- loaded module list, especially GTK/GLib/CRT duplicates
- reproduction profile and steps
- whether spell check, themes, plugins, tray, or notifications were active
- relevant GDK/GTK debug environment output

Keep crash dumps and large logs out of Git. Record their local path/hash or attach
them to the corresponding issue/PR according to project policy.

## Stage Completion Rule

A stage can move to complete in `migration-plan.md` only when:

1. its API inventory rows are converted or deliberately retired;
2. automated checks are green;
3. affected manual workflows pass;
4. performance and visual results are recorded;
5. packaging impact is either validated or explicitly not applicable; and
6. known regressions have owners and follow-up issues rather than being hidden.
