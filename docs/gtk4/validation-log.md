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

### PR: [#80 - GTK4 Stage 5 Ignore List](https://github.com/Fabulor/fabulor/pull/80)

Date: 2026-07-16

Migration stage: 5, Ignore List pass 17

Files/workflows converted: editable mask rows; seven flag columns; sorting;
add, delete, confirmed clear, snapshots, and common-engine synchronization

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles and executes the Ignore List owner
- [x] accepted/rejected renames and rename/flag callbacks pass
- [x] historical DCC bit 128 and hidden `IG_NOSAVE` bit 64 survive visible edits
- [x] mask snapshots, row counts, clear, and cleanup checks pass
- [x] `ignoregui.c` contains no direct tree-model, path, iterator, cell-renderer, list-store, or tree-selection dependency
- [x] production and strict build definitions include the new owner
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 initial masks, Mask-column sorting, and all seven toggle columns
- [ ] accepted rename, duplicate rejection, unchanged rename, and empty-edit behavior
- [ ] Add, Delete with next-row selection, confirmed/cancelled Clear, and close/reopen persistence
- [ ] ignore statistics continue updating while the window is open
- [ ] GTK4 editable labels, check buttons, keyboard navigation, sorting, and focus await frontend cutover

Scope: Ignore List model/view ownership and editing workflows. Common ignore
matching, counters, command behavior, persistence format, and confirmation text
are unchanged.

### PR: #81 - GTK4 Stage 5 Ban List

Date: 2026-07-17

Migration stage: 5, Ban List pass 18

Files/workflows converted: four-mode immutable rows; multi-selection; sorting;
copy; selected remove; inverse crop; confirmed clear; refresh population

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles and executes the Ban List owner
- [x] mixed multi-selection, selection callbacks, inversion, and select-all pass
- [x] numeric mode-filtered remove and crop snapshots pass
- [x] row counts, clear, and cleanup checks pass
- [x] `banlist.c` contains no direct tree-model, path, iterator, cell-renderer, list-store, or tree-selection dependency
- [x] production and strict build definitions include the new owner
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 Ban, Exempt, Invite, and Quiet refresh and capability toggles
- [ ] Type, Mask, From, and Date sorting with mixed mode rows
- [ ] multi-select Remove, Crop, confirmed/cancelled Clear, and post-action refresh
- [ ] Copy mask and Copy entry context actions on sorted rows
- [ ] operator/non-operator and disconnected sensitivity states
- [ ] GTK4 keyboard multi-selection, context popover placement, sorting, and focus await frontend cutover

Scope: Ban List model/view ownership and row interaction. Server capability
detection, IRC replies, mode command batching, permission checks, titles, and
confirmation text are unchanged.

### PR: #82 - GTK4 Stage 5 DCC Transfer List

Date: 2026-07-17

Migration stage: 5, DCC transfer-list pass 19

Files/workflows converted: combined Uploads and Downloads rows; mutable
progress refresh; multi-selection; view filtering; details; activation;
accept, resume, abort, and clear-completed actions

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles and executes the DCC transfer-list owner
- [x] prepend/append order, duplicate rejection, and in-place update pass
- [x] multi-selection, identity snapshots, callbacks, removal, and clear pass
- [x] file-transfer state contains no legacy store, selection, transfer-column, iterator, or cell-renderer dependency
- [x] production and strict build definitions include the new owner
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 combined Uploads and Downloads population and live progress
- [ ] Both, Uploads, and Downloads filtering with active and completed rows
- [ ] single and multi-select Accept, Resume, Abort, and Clear workflows
- [ ] transfer details, Open Folder, double-click behavior, and close/reopen
- [ ] zero-byte, queued resumable, failed, aborted, and completed presentation
- [ ] GTK4 keyboard multi-selection, column sizing, colour, direction icons, and focus await frontend cutover

Scope: combined file-transfer model/view ownership and row interaction. DCC
protocol behavior, transfer engine state, command dispatch, and the distinct
DCC Chat five-column workflow are unchanged. DCC Chat is the next contained
Stage 5 target.

### PR: #83 - GTK4 Stage 5 DCC Chat List

Date: 2026-07-17

Migration stage: 5, DCC Chat pass 20

Files/workflows converted: five-column DCC Chat rows; mutable status and
counters; prepend population; multi-selection; activation; Accept and Abort

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles and executes the DCC Chat owner
- [x] duplicate rejection, in-place update, and row counts pass
- [x] ordered multi-selection, callbacks, identity removal, and clear pass
- [x] `dccgui.c` contains no direct tree-model, path, iterator, cell-renderer, list-store, or tree-selection dependency
- [x] production and strict build definitions include the new owner
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 DCC Chat list population and live status/counter refresh
- [ ] incoming and outgoing Chat rows, including queued, active, failed, and aborted states
- [ ] single and multi-select Accept and Abort workflows
- [ ] double-click activation and close/reopen behavior
- [ ] status colours, numeric alignment, long nicks, and start-time presentation
- [ ] GTK4 keyboard multi-selection, column sizing, activation, colour, and focus await frontend cutover

Scope: DCC Chat model/view ownership and row interaction. DCC protocol,
socket lifecycle, command dispatch, and the separately converted file-transfer
workflow are unchanged.

### PR: #84 - GTK4 Stage 5 Channel List

Date: 2026-07-17

Migration stage: 5, Channel List pass 21

Files/workflows converted: Channel List sorted rows; batched population;
multi-selection; Join, Copy Channel, Copy Topic, context selection, export,
refresh, teardown, and saved column widths

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles and executes the Channel List owner with 0 warnings and 0 errors
- [x] collation ordering, duplicate rejection, and row counts pass
- [x] ordered multi-selection, selected channel/topic copies, sorted export records, clear, and cleanup pass
- [x] `chanlist.c` contains no direct tree-model, path, iterator, cell-renderer, list-store, tree-selection, or custom-list dependency
- [x] production and strict build definitions include the new owner
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 `/LIST` download, first-row display, 250 ms batching, completion, refresh, and close/reopen
- [ ] simple, wildcard, and regular-expression filtering with channel/topic toggles and min/max users
- [ ] channel/users/topic sorting, saved widths, multi-select Join, Copy Channel, Copy Topic, and favourite context submenu
- [ ] sorted file export and large-network list responsiveness
- [ ] GTK4 keyboard multi-selection, column sizing, sorting, context placement, focus, accessibility, and scale await frontend cutover

Scope: Channel List model/view ownership and row interaction. IRC `/LIST`
protocol handling, server-side list arguments, filtering semantics, command
dispatch, and authoritative downloaded-row storage are unchanged.

### PR: #85 - GTK4 Stage 5 Generic Editable Lists

Date: 2026-07-17

Migration stage: 5, generic editable-list pass 22

Files/workflows converted: Commands, Popups, User Menu, Replace, URL Handlers,
User List Buttons, Dialog Buttons, and CTCP Replies two-column editors; editing;
add/delete; single selection; pointer drag; Shift+Up/Down; ordered save snapshots

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles and executes the editable-list owner with 0 warnings and 0 errors
- [x] field edits, boundary-safe movement, and ordered snapshots pass
- [x] selection, deletion, empty-row initialization, and cleanup pass
- [x] `editlist.c` contains no direct tree-model, path, iterator, cell-renderer, list-store, tree-selection, raw key-event, or key-press signal dependency
- [x] production and strict build definitions include the new owner
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 open, edit, add, delete, pointer reorder, Shift+Up/Down, cancel, save, and reopen
- [ ] all eight editor entry points preserve labels, tooltips, ordering, and file output
- [ ] Commands, Popups, User Menu, CTCP Replies, URL Handlers, and replacements reload immediately after save
- [ ] User List and Dialog Button editors refresh every open session after save
- [ ] empty and long name/command values remain editable without clipping or crashes
- [ ] GTK4 inline editing, pointer drag placement, keyboard movement, focus, accessibility, and scale await frontend cutover

Scope: generic editor model/view ownership and row interaction. Configuration
file syntax, list parsing, menu/button construction, command dispatch, and
post-save workflow refresh behavior are unchanged.

### PR: #86 - GTK4 Stage 5 Print Events Editor

Date: 2026-07-17

Migration stage: 5, Print Events pass 23

Files/workflows converted: editable Print Events table; stable event signal
identity; selected-event argument-help table; selection and edit callbacks

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles and executes the Print Events owner with 0 warnings and 0 errors
- [x] accepted and rejected edits preserve row-local signal identity
- [x] selection callbacks, help rows, clearing, and cleanup pass
- [x] `textgui.c` contains no direct tree-model, iterator, path, cell-renderer, list-store, or tree-selection dependency
- [x] production and strict build definitions include the new owner
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 editor opens, selects each event, and displays the correct numbered argument help
- [ ] valid edits update the preview and persist after restart
- [ ] malformed text and out-of-range argument references remain unchanged and show the expected warning
- [ ] Load From, Save As, Test All, theme updates, and OK/close behavior remain unchanged
- [ ] GTK4 inline editing, selection, focus, accessibility, column sizing, and scale await frontend cutover

Scope: Print Events list/model/view ownership and row interaction. Event
parsing, argument validation, compiled event storage, transcript preview,
theme handling, and file import/export remain unchanged.

### PR: #87 - GTK4 Stage 5 Key Bindings Editor

Date: 2026-07-17

Migration stage: 5, key-bindings pass 24

Files/workflows converted: shortcut key capture; action/data editing;
built-in/custom mutation rules; selection help; add/delete; Shift+Up/Down;
reset preservation; ordered save snapshots

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles and executes the key-binding owner with 0 warnings and 0 errors
- [x] accelerator normalization and copied ordered snapshots pass
- [x] built-in mutation protection and custom edit/move/delete rules pass
- [x] selection callbacks, clear, and cleanup pass
- [x] `fkeys.c` contains no direct tree-model, iterator, path, cell-renderer, list-store, tree-selection, or raw key-event dependency
- [x] production and strict build definitions include the new owner
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 editor opens and displays every current shortcut and action help
- [ ] accelerator editing, custom Add, action selection, Data1/Data2 editing, Delete, and Shift+Up/Down work
- [ ] built-in rows reject action/data editing, deletion, and movement while accepting accelerator changes
- [ ] Reset restores defaults while retaining custom and excess duplicate rows
- [ ] Save updates `keybindings.conf`, runtime shortcuts, and the Quit menu accelerator; Cancel remains non-persistent
- [ ] GTK4 capture focus, dropdown/editing, keyboard ordering, accessibility, column sizing, and scale await frontend cutover

Scope: key-binding list/model/view ownership and row interaction. Shortcut
configuration parsing, built-in signature matching, action dispatch/help,
serialization, and menu accelerator refresh remain unchanged.

### PR: #88 - GTK4 Stage 5 Preferences Sound Events

Date: 2026-07-17

Migration stage: 5, Preferences sound-event pass 25

Files/workflows converted: sound event/file table; stable event selection;
sound-file entry synchronization; live row updates; Preferences teardown

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles and executes the sound-event owner with 0 warnings and 0 errors
- [x] explicit selection and stable event-index callbacks pass
- [x] live filename updates, missing-event rejection, clear, and cleanup pass
- [x] the `setup.c` sound workflow contains no direct tree-model, iterator, path, cell-renderer, list-store, or tree-selection dependency
- [x] production and strict build definitions include the new owner
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 Preferences Sound page opens with the remembered event selected and visible
- [ ] selecting events updates the Sound file entry without changing stored values
- [ ] typing and browsing update the correct row and preserve default-sounds filename shortening
- [ ] Play uses the current entry and Cancel/OK behavior remains unchanged
- [ ] reopening Preferences preserves the last selected event without stale callbacks
- [ ] GTK4 selection, focus, accessibility, column sizing, long filenames, and scale await frontend cutover

Scope: Preferences sound-event list/model/view ownership and row interaction.
Sound playback, file selection, path normalization, core sound-file ownership,
and preference save/cancel policy remain unchanged.

### PR: #89 - GTK4 Stage 5 Preferences Category Navigation

Date: 2026-07-17

Migration stage: 5, Preferences category-navigation pass 26

Files/workflows converted: Preferences category/page hierarchy; stable page
identity; non-selectable headings; expansion; remembered startup selection;
lazy page-switch callback; Preferences teardown

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles and executes the category owner with 0 warnings and 0 errors
- [x] hierarchy counts, stable page selection, callback de-duplication, missing-page rejection, and cleanup pass
- [x] category headings are excluded from the page-selection contract
- [x] `setup.c` contains no direct tree-model, iterator, path, cell-renderer, list-store, tree-selection, or child-reordering dependency
- [x] production and strict build definitions include the new owner
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 Preferences opens with all three categories expanded and Appearance selected on first use
- [ ] category headings expand and collapse but cannot replace the active page
- [ ] pointer and keyboard page selection lazily creates and displays the correct page
- [ ] closing and reopening Preferences restores the last selected page
- [ ] repeated open/close cycles leave no stale callbacks or navigation state
- [ ] GTK4 focus, accessibility, hierarchy styling, long translations, and scale await frontend cutover

Scope: Preferences category navigation model/view ownership and page
selection. Page registration, lazy page construction, notebook switching, and
preference save/cancel policy remain in `setup.c`.

### PR: #90 - GTK4 Stage 5 Server Network List

Date: 2026-07-17

Migration stage: 5, Server List network-table pass 27

Files/workflows converted: main network chooser; stable `ircnet` identity;
favorite presentation; inline rename; favorites-only refresh; remembered
selection; add/remove/sort; Shift+Up/Down ordering; window teardown

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles and executes the server-network owner with 0 warnings and 0 errors
- [x] prepend/append order, duplicate rejection, identity selection, movement boundaries, favorite/name updates, removal, clear, and cleanup pass
- [x] the main chooser path no longer accesses tree models, iterators, paths, renderers, or selections
- [x] production and strict build definitions include the new owner
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 Network List restores the remembered network and favorite emphasis
- [ ] Add selects the new row and immediately starts inline rename
- [ ] empty rename removes the new network and non-empty rename persists
- [ ] favorite toggling and favorites-only refresh preserve a valid selection
- [ ] Sort and Shift+Up/Down keep the visible and saved network order synchronized
- [ ] Connect, Edit, Remove, Close, and repeated reopen preserve prior behavior
- [ ] GTK4 inline editing, focus, accessibility, long names, and scale await frontend cutover

Scope: main Server List network chooser model/view ownership and interaction.
The detailed Servers, Autojoin channels, and Connect commands editor tables,
network connection workflow, and configuration persistence remain unchanged.

### PR: #91 - GTK4 Stage 5 Server Editor Lists

Date: 2026-07-17

Migration stage: 5, Server List editor-table pass 28

Files/workflows converted: Servers, Autojoin channels, and Connect commands
tables; stable core-object identity; inline editing; add/remove; empty-value
deletion; optional channel keys; Shift+Up/Down ordering; editor teardown

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles and executes the server-entry owner with 0 warnings and 0 errors
- [x] duplicate labels, stable identity, selection, primary/secondary updates, movement boundaries, removal, one-column constraints, clear, and cleanup pass
- [x] `servlistgui.c` no longer accesses tree models, iterators, paths, renderers, list stores, tree selections, or tree views
- [x] production and strict build definitions include the new owner
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 editor preserves all three tabs, row text, and remembered selection
- [ ] Add selects a new server, channel, or command and immediately starts inline editing
- [ ] primary edits persist; empty edits delete except for the protected last server
- [ ] server and command canonicalization and channel-key clearing preserve prior behavior
- [ ] Shift+Up/Down keeps visible and saved order synchronized in all three tabs
- [ ] repeated editor open/close cycles leave no stale selection, model, or callback state
- [ ] GTK4 focus, accessibility, duplicate labels, long values, and scale await frontend cutover

Scope: detailed Server List editor model/view ownership and interaction.
Network connection behavior and configuration persistence remain unchanged.

### PR: #92 - GTK4 Stage 6 Transcript Render Target

Date: 2026-07-17

Migration stage: 6, transcript render-target pass 1

Files/workflows converted: transcript Cairo destination ownership; active
context exchange; offscreen surface selection; GTK3 window fallback;
GTK4 snapshot Cairo lifecycle; strict dependency contract

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles, links, and executes with 0 warnings and 0 errors
- [x] empty, offscreen, active-context, restoration, snapshot-node, and cleanup contracts pass
- [x] `GtkXText` no longer stores raw draw-window, draw-surface, or draw-context fields
- [x] Cairo and Graphene headers, libraries, and runtime DLLs are explicit validated dependencies
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 transcript preserves text, formatting, colours, backgrounds, timestamps, markers, and search highlights
- [ ] selection redraw, URL hover, scrolling, resize, new-message display, and shutdown preserve prior behavior
- [ ] repeated session/window creation leaves no render-target warning or stale context
- [ ] GTK4 visual, high-DPI, accessibility, scrollback-load, and latency checks await widget-class integration

Scope: transcript rendering-destination ownership only. Geometry, class virtual
methods, controllers, selection, clipboard behavior, and spell-check input
remain unchanged.

### PR: #93 - GTK4 Stage 6 Transcript Geometry

Date: 2026-07-17

Migration stage: 6, transcript geometry pass 2

Files/workflows converted: transcript allocated-geometry owner; line wrapping
and recalculation; page and partial rendering; selection auto-scroll; entry
visibility; buffer-switch size tracking; strict geometry contract

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles, links, and executes with 0 warnings and 0 errors
- [x] positive dimensions are preserved and non-positive dimensions are rejected and reset
- [x] operational transcript size reads no longer depend on `GdkWindow`
- [x] native window dimensions remain only in the private GTK3 surface-capture helper
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 transcript preserves wrapping, resize, scrolling, selection, visibility, and buffer switching
- [ ] smooth scrolling preserves overlap capture without flicker or stale pixels
- [ ] GTK4 visual, high-DPI, accessibility, scrollback-load, and latency checks await widget-class integration

Scope: transcript dimension sourcing only. Native GTK3 pointer and surface
operations, class virtual methods, controllers, selection, clipboard behavior,
and spell-check input remain unchanged.

### PR: #94 - GTK4 Stage 6 Transcript Widget Class

Date: 2026-07-17

Migration stage: 6, transcript widget-class pass 3

Files/workflows converted: transcript preferred-size and measure policy;
allocation dispatch and width-change policy; realize/unrealize dispatch; GTK3
Cairo draw; GTK4 snapshot Cairo lifecycle; class callback registration

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles, links, and executes with 0 warnings and 0 errors
- [x] headless GTK4 subclass receives measure, allocation, realize, unrealize, and snapshot slots
- [x] horizontal and vertical minimum requests remain 200 and 90 pixels
- [x] unchanged-width and changed-width resize decisions remain distinct
- [x] GTK4 snapshot dispatch uses the validated geometry and render-target owners
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 transcript preserves initial display, resize, wrapping, scrolling, and redraw behavior
- [ ] repeated realize/unrealize and window creation leave no stale Pango layout, native window, or render context
- [ ] GTK4 visual, high-DPI, accessibility, scrollback-load, and latency checks await full widget integration

Scope: transcript measurement, allocation, lifecycle, and frame rendering class
methods only. Pointer/input controllers, selection, clipboard behavior,
background validation, and spell-check input remain unchanged.

### PR: #95 - GTK4 Stage 6 Transcript Input Controllers

Date: 2026-07-17

Migration stage: 6, transcript input-controller pass 4

Files/workflows converted: pointer motion and leave; modifier state; click
press/count and release; character/word/line selection policy; smooth and
discrete scrolling; focus redraw; cursor roles; word-click payload; popup-menu
coordinates

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles, links, and executes with 0 warnings and 0 errors
- [x] modifier-aware motion helper has a strict GTK4-compatible signature
- [x] non-left, single, double, triple, and repeated click policies pass
- [x] negative, zero, and positive scroll direction policies pass
- [x] transcript button, motion, scroll, and leave class event slots are removed
- [x] only GTK3 selection ownership and payload class slots remain
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 transcript preserves character, word, line, and drag selection
- [ ] Shift timestamps, Ctrl colour copy, auto-copy, and selection auto-scroll behave unchanged
- [ ] URL hover/click, tooltips, separator drag, popup placement, wheel scrolling, and focus redraw behave unchanged
- [ ] GTK4 controller interaction, accessibility, high-DPI, scrollback-load, and latency checks await full widget integration

Scope: transcript pointer, click, scroll, leave, focus, and selection-input
dispatch only. Selection ownership, clipboard payloads, background validation,
and spell-check input remain unchanged.

### PR: #96 - GTK4 Stage 6 Transcript Selection Ownership

Date: 2026-07-17

Migration stage: 6, transcript selection pass 5

Files/workflows converted: PRIMARY target registration; selection ownership
and replacement; UTF-8, text, compound-text, and locale payload delivery;
explicit copy; CLIPBOARD and PRIMARY GTK4 content publication; adapter cleanup

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles, links, and executes with 0 warnings and 0 errors
- [x] complete and bounded selection payload-copy policies pass
- [x] transcript class has no direct input or selection event slot
- [x] transcript content code has no clipboard, target, atom, event, or payload API
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 selection, auto-copy, explicit copy, and colour-copy behavior remain unchanged
- [ ] PRIMARY replacement clears selection on Unix and retains established Windows behavior
- [ ] GTK4 CLIPBOARD/PRIMARY interoperability awaits full widget integration

Scope: transcript selection ownership, publication, replacement, and payload
delivery only. Background validation and spell-check input remain unchanged.

### PR: #97 - GTK4 Stage 6 Transcript Frame Redraw

Date: 2026-07-17

Migration stage: 6, transcript frame-redraw pass 6

Files/workflows converted: full-page native-window gate; upward/downward
scroll-copy policy; exposed-region damage; GTK4 full snapshot fallback; CSS
class attachment; focused-root detection; redraw requests; GTK3 helper scope

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles, links, and executes with 0 warnings and 0 errors
- [x] upward and downward scroll-copy geometry policies pass
- [x] unavailable native capture selects complete rendering
- [x] full-height overlap rejects partial copying
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 transcript wheel, scrollbar, selection auto-scroll, and new-message redraw remain unchanged
- [ ] marker focus behavior and transcript CSS class remain unchanged
- [ ] GTK4 snapshot scrolling and latency await full widget integration

Scope: transcript frame invalidation, focus, and partial-scroll optimization
only. Background composition, highlights, hit testing, accessibility, and
spell-check input remain separate targets.

### PR: #98 - GTK4 Stage 6 Transcript Background Composition

Date: 2026-07-17

Migration stage: 6, transcript background-composition pass 7

Files/workflows converted: background source ownership; aspect-fitted image
composition; repeated non-image surfaces; frame-local viewport cache; palette
fallback; source replacement and teardown

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles, links, and executes with 0 warnings and 0 errors
- [x] absent-source palette fallback produces the expected pixels
- [x] fitted image content and black letterboxing produce the expected pixels
- [x] source presence, replacement, frame cache, and teardown contracts pass
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 transcript background colour remains unchanged
- [ ] shipping GTK3 background images preserve fitting, centring, and letterboxing
- [ ] background updates and repeated non-image surfaces remain unchanged
- [ ] GTK4 snapshot background output awaits full widget integration

Scope: transcript background source, composition, fallback, frame cache, and
teardown only. Markers, highlights, hit testing, accessibility, high DPI,
scrollback performance, and spell-check input remain separate targets.

### PR: #99 - GTK4 Stage 6 Transcript Decorations

Date: 2026-07-17

Migration stage: 6, transcript decorations pass 8

Files/workflows converted: marker-line placement; persistent search-match
boundary classification; adjacent current-match precedence; transient
URL/nickname hover ranges; targeted highlight paint/clear state; timestamp
suspension; decoration teardown

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles, links, and executes with 0 warnings and 0 errors
- [x] marker positions before and after transcript entries pass
- [x] search start, middle, end, current, and adjacent boundary contracts pass
- [x] hover range, paint/clear, suspension, and teardown contracts pass
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 marker line position, colour, and seen state remain unchanged
- [ ] Search current/all highlighting and adjacent matches remain unchanged
- [ ] URL, nickname, and channel hover underlines clear without residue
- [ ] timestamp hover and tooltip behavior remain unchanged
- [ ] GTK4 snapshot decoration output awaits full widget integration

Scope: transcript marker placement, search range classification, and transient
hover-highlight state only. URL hit testing, accessibility, high DPI,
scrollback performance, and spell-check input remain separate targets.

### PR: #100 - GTK4 Stage 6 Transcript Hit Testing

Date: 2026-07-17

Migration stage: 6, transcript hit-testing pass 9

Files/workflows converted: pointer-to-scrollback-line mapping; separator hit
tolerance; IRC-formatting match adjustment; captured word type and byte range;
URL/host/nickname/channel/email click dispatch; immutable matched-substring
ownership

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles, links, and executes with 0 warnings and 0 errors
- [x] positive and negative pointer-to-line contracts pass
- [x] separator centre and one-pixel edge tolerance contracts pass
- [x] formatted match offset and invalid-range contracts pass
- [x] matched-substring duplication preserves the source word
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] URL and host hover/click/menu behavior remains unchanged
- [ ] nickname, channel, email, plain-word, and dialog menus remain unchanged
- [ ] wrapped lines, timestamps, formatted links, and separator dragging remain unchanged
- [ ] selection and empty-click focus behavior remain unchanged
- [ ] GTK4 snapshot hit testing awaits full widget integration

Scope: transcript line, separator, formatted-match, and word-click hit testing
only. Accessibility, high DPI, scrollback performance, and spell-check input
remain separate targets.

### PR: #101 - GTK4 Stage 6 Transcript Accessibility And Display Scale

Date: 2026-07-17

Migration stage: 6, transcript accessibility and display-scale pass 10

Files/workflows converted: cross-version transcript role and label; Pango font
metric conversion; strike and underline coordinates; inline flag logical and
device sizing; scale-aware pixbuf caching and Cairo painting

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles, links, and executes with 0 warnings and 0 errors
- [x] Pango metric rounding and decoration coordinate contracts pass
- [x] inline flag bounds and 1x/2x/3x scale contracts pass
- [x] invalid scale and device-to-logical conversion contracts pass
- [x] GTK4 transcript widget exposes the log accessibility role
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] GTK3 transcript text, wrapping, underline, and strikethrough remain unchanged
- [ ] flags remain aligned and sharp at 100%, 150%, and 200% display scaling
- [ ] GTK3 and GTK4 accessibility tools report a labelled transcript log
- [ ] full GTK4 accessible scrollback text awaits its dedicated integration pass
- [ ] scrollback-load and latency checks remain a separate target

Scope: transcript role/name semantics and toolkit-neutral display calculations
only. Accessible text exposure, production GTK4 visual checks, scrollback
performance, and spell-check input remain separate targets.

### PR: #102 - GTK4 Stage 6 Transcript Accessible Text

Date: 2026-07-17

Migration stage: 6, transcript accessible-text pass 11

Files/workflows converted: GTK4 `GtkAccessibleText` registration and vtable;
bounded recent plain-text snapshot; Unicode character/word/sentence/line ranges;
append, trim, clear, timestamp, and buffer-switch invalidation; coalesced idle
refresh; minimal content-change notification; teardown cancellation

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles, links, and executes with 0 warnings and 0 errors
- [x] character-indexed complete and bounded content queries pass
- [x] Pango word and sentence plus entry line-boundary contracts pass
- [x] minimal insertion replacement range contract passes
- [x] accessible snapshots remain within the 1 MiB safety bound
- [x] unobserved accessible text remains lazy until its first query
- [x] GTK4 transcript probe type implements `GtkAccessibleText`
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] GTK4 screen readers report the labelled transcript log and read recent text
- [ ] incoming messages and channel switches produce timely, non-duplicated updates
- [ ] displayed timestamps and hidden IRC runs match the accessible snapshot
- [ ] large scrollback remains responsive while accessibility is active
- [ ] selection mutation and text geometry remain unsupported for the read-only log

Scope: bounded read-only GTK4 transcript text exposure and update lifetime only.
Production screen-reader validation, accessible selection/geometry, scrollback
performance, and spell-check input remain separate targets.

### PR: #103 - GTK4 Stage 6 Transcript Performance Policy

Date: 2026-07-17

Migration stage: 6, transcript performance and latency pass 12

Files/workflows converted: visible-buffer append refresh planning; immediate
bottom redraw; historical-view idle coalescing; wrapped-display-line trimming;
complete newest-entry retention; visible-trim repaint promotion; informational
policy timing diagnostic

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles, links, and executes with 0 warnings and 0 errors
- [x] visible bottom appends select immediate redraw
- [x] historical-view appends select one coalesced idle refresh
- [x] hidden-buffer and already-pending historical-view appends schedule no additional refresh
- [x] multi-line append trimming reaches the configured wrapped-line bound
- [x] a sole oversized newest entry is retained
- [x] one million policy decisions complete as a non-gating diagnostic

Performance observations:

- strict-probe policy diagnostic: 1,000,000 decisions in 1,540 microseconds
- elapsed time is informational; deterministic policy outcomes and operation
  counts are the CI pass/fail contract

Manual checks:

- [ ] shipping GTK3 local echo remains immediate at the scrollback limit
- [ ] shipping GTK3 historical scroll position remains stable during append bursts
- [ ] production GTK4 large-scrollback wheel, Page Up, busy-channel, and local-echo latency awaits full widget integration
- [ ] production measurements record scrollback size, plugin set, spell-check state, network, and lag-meter value

Scope: transcript append refresh and wrapped-line retention policy only. Full
renderer throughput, input latency, visual smoothness, screen-reader behavior,
and spell-check input remain production validation or separate Stage 6 targets.

### PR: #104 - GTK4 Stage 6 Spell-Input Word Boundary

Date: 2026-07-17

Migration stage: 6, spell-input word-boundary pass 1

Files/workflows converted: Pango word segmentation ownership; paired UTF-8 byte
and character ranges; popup cursor lookup; add-to-dictionary and session-ignore
selection; replacement mutation; underline checks; language/preference refresh;
single-owner cleanup

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles, links, and executes with 0 warnings and 0 errors
- [x] ASCII and multibyte word order remains deterministic
- [x] `café` byte range is 6-11 and character range is 6-10
- [x] cursor lookup inside and at the word end returns the same range
- [x] empty owners reject invalid access without exposing storage
- [x] UTF-8 word duplication returns the exact source word
- [x] GTK4 dependency validator tests remain 8/8
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 misspelling underlines and suggestions remain unchanged
- [ ] add to personal dictionary remains persistent after restart
- [ ] Ignore All remains session-scoped
- [ ] accented-word replacement targets only the selected word
- [ ] URL exclusion, IRC formatting, emoji insertion, and edit-box latency remain unchanged

Scope: spell-input word segmentation, coordinate ownership, and refresh lifetime
only. GTK4 subclass lifecycle, editable delegation, formatting attributes,
dynamic menus, and complete production input validation remain separate passes.

### PR: #105 - GTK4 Stage 6 Spell-Input Widget Lifecycle

Date: 2026-07-17

Migration stage: 6, spell-input widget lifecycle and event pass 2

Files/workflows converted: inherited `GtkEditable` semantics; normalized click
dispatch; GTK3 exact pointer lookup; GTK4 editable-cursor fallback; queued
redraw; caret/style refresh; theme-listener ownership; class virtual cleanup

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles, links, and executes with 0 warnings and 0 errors
- [x] a strict GTK4 `GtkEntry` subclass inherits `GtkEditable`
- [x] pointer and redraw adapter source compiles against GTK4 4.22.4
- [x] production spell entry has no draw, button-press, or style-update class virtual
- [x] redraw scheduling has no `GdkWindow` dependency
- [x] theme listener is unregistered during idempotent disposal
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 pointer and keyboard spelling menus target the expected word
- [ ] native selection, focus, input methods, and caret remain unchanged
- [ ] theme changes refresh caret contrast and misspelling underline colours
- [ ] repeated input creation and destruction produces no stale theme callback
- [ ] GTK4 right-click and keyboard menu targeting awaits the dynamic-menu pass

Scope: spell-entry class lifetime, editing inheritance, pointer dispatch,
redraw, and theme refresh only. Formatting ownership, GTK4 dynamic menus, and
complete production input validation remain separate passes.

### PR: #106 - GTK4 Stage 6 Spell-Input Styling Boundary

Date: 2026-07-17

Migration stage: 6, spell-input Pango styling pass 3

Files/workflows converted: IRC control shaping; bold, italic, strikethrough,
underline, reset, and reverse ranges; mIRC colour resolution; semantic default
and spell colours; misspelling attributes; Pango attribute-list ownership

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles, links, and executes with 0 warnings and 0 errors
- [x] formatting-disabled text produces no Pango attributes
- [x] IRC controls are hidden and all four formatting toggles have stable ranges
- [x] reset restores normal weight and the semantic foreground colour
- [x] mIRC foreground and background indexes resolve through the supplied palette
- [x] colour parameters are hidden when a sequence ends the input
- [x] reverse formatting swaps semantic foreground and background colours
- [x] misspelling byte ranges carry error underline and semantic spell colour
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 edit-box bold, italic, underline, and strikethrough previews remain unchanged
- [ ] reset, reverse, foreground-only, and foreground/background colour previews remain correct
- [ ] control bytes and colour parameters remain hidden while editing
- [ ] misspelling underlines coexist with IRC formatting and theme changes
- [ ] formatting-disabled preference shows literal input without Pango styling

Scope: edit-box Pango attribute construction and palette roles only. GTK4
dynamic spelling/colour menus and complete production input validation remain
separate passes.

### PR: #107 - GTK4 Stage 6 Spell-Input Dynamic Menus

Date: 2026-07-17

Migration stage: 6, spell-input dynamic menu and action pass 4

Files/workflows converted: immutable spelling and formatting menu projection;
per-entry GTK4 action ownership; lazy Enchant suggestions; language-targeted
replace/add actions; session ignore; spell toggle; IRC attributes; colours
0-15; pointer and keyboard context-menu refresh; GTK3 popup containment

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 probe compiles, links, and executes with 0 warnings and 0 errors
- [x] independent Meson/Ninja GTK4 probe compiles and executes with warnings treated as errors
- [x] complete `sexy-spell-entry.c` source passes an MSVC GTK4 syntax compile
- [x] disabled spell state omits spelling suggestions but retains formatting and toggle actions
- [x] two dictionaries expose language-specific replacement and add targets
- [x] empty suggestion lists remain represented without an executable action
- [x] eleven suggestions preserve the ten-item nested `More...` boundary
- [x] all five IRC attribute and sixteen colour insertion actions are present
- [x] action targets carry language strings rather than Enchant pointers
- [x] GTK3 `populate-popup` and markup-menu code is explicitly version-contained
- [x] GTK4 dependency contract includes the translated-menu `intl` header, import library, and runtime
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 suggestions, replacement, add, Ignore All, and persistence remain unchanged
- [ ] shipping GTK3 formatting and colour swatch menu remains unchanged
- [ ] GTK4 right-click targets the pointer word and Shift+F10/Menu targets the cursor word
- [ ] GTK4 spell toggle updates underline and menu state
- [ ] GTK4 formatting insertion replaces selection and preserves cursor behavior
- [ ] production GTK4 typing and context-menu latency awaits full client integration

Scope: spelling and formatting context-menu projection, activation ownership,
and GTK3 containment only. Complete production GTK4 input validation remains
the final Stage 6 spell-input pass.

### PR: #108 - GTK4 Stage 6 Spell-Input URL And Initialization

Date: 2026-07-18

Migration stage: 6, spell-input URL and initialization pass 5

Files/workflows converted: URI-token classification from immutable word
snapshots; live-check and suggestion exclusion for URLs; Enchant-safe private
state initialization; defensive dictionary-language description

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 MSVC probe compiles, links, and executes with 0 warnings and 0 errors
- [x] complete `sexy-spell-entry.c` source passes an MSVC GTK4 syntax compile
- [x] fresh Meson/Ninja 1.13.2 probe compiles 38 objects with warnings treated as errors
- [x] independent Meson runtime test passes 1/1
- [x] wrapped HTTPS URLs with multibyte query text are identified from inner word ranges
- [x] adjacent ordinary words remain eligible for Enchant checking
- [x] `www.` links are identified without requiring a URI scheme
- [x] emoji adjacent to a URL preserves valid UTF-8 segmentation
- [x] entry owners and preference state are initialized before Enchant activation
- [x] dictionary language description fails to null rather than an indeterminate pointer
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 URL paste remains stable with spell checking enabled
- [ ] shipping GTK3 suggestions and personal-dictionary persistence remain unchanged
- [ ] GTK4 URL paste, emoji insertion, clipboard, and shortcut behavior await the production frontend
- [ ] GTK4 accessibility, high-DPI, typing, Enter-to-echo, and context-menu latency await the production frontend

Scope: URL exclusion and Enchant initialization safety only. The remaining
interactive production GTK4 checks stay open as the final Stage 6 validation
target.

### PR: #109 - GTK4 Stage 6 Emoji-Picker Ownership

Date: 2026-07-18

Migration stage: 6, spell-input emoji-picker ownership and lifecycle pass 6

Files/workflows converted: per-entry popover ownership; GTK3/GTK4 parent and
teardown policy; one-time lazy category loading; notebook replacement; GTK4
flag paintables; validated regional-indicator and codepoint insertion text

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] production GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 MSVC probe compiles, links, and executes with 0 warnings and 0 errors
- [x] exact GTK3/GTK4 popover parent, teardown, and reuse boundary compiles
- [x] fresh MSVC Meson/Ninja probe compiles 39 objects with warnings treated as errors
- [x] independent Meson runtime test passes 1/1
- [x] lazy category pages accept exactly one load claim
- [x] static category arrays remain borrowed and flags pages are explicit
- [x] upper- and lower-case two-letter flag codes produce matching regional indicators
- [x] malformed flag codes, zero codepoints, and invalid Unicode scalars are rejected
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping GTK3 picker opens, changes categories, inserts emoji and flags, and reopens cleanly
- [ ] GTK4 mouse and keyboard opening, focus return, and popover dismissal await the production frontend
- [ ] GTK4 flag assets, accessibility, high-DPI rendering, and picker latency await the production frontend
- [ ] GTK4 typing, Enter-to-echo, clipboard, shortcut, and Enchant latency await the production frontend

Scope: emoji-picker ownership and lifecycle containment only. Complete
production GTK4 input validation remains open until the GTK4 frontend is
runnable.

### PR: #110 - GTK4 Stage 7 Theme Discovery

Date: 2026-07-18

Migration stage: 7, GTK4 desktop/profile theme-discovery pass 1

Files/workflows converted: exact GTK4 CSS layout recognition; profile and
desktop root ownership; source-qualified metadata; dark variant and preview
discovery; canonical duplicate suppression; deterministic result ordering

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] shipping common library compiles with 0 warnings and 0 errors
- [x] strict GTK4 MSVC probe compiles, links, and executes with 0 warnings and 0 errors
- [x] fresh MSVC Meson/Ninja probe compiles 40 objects with warnings treated as errors
- [x] independent Meson runtime test passes 1/1
- [x] Linux GCC C11 syntax checks pass with all warnings treated as errors
- [x] profile themes resolve beneath the `themes` directory
- [x] exact `gtk-4.0/gtk.css` layouts are accepted and GTK3-only layouts are excluded
- [x] profile/desktop identity, localized names, and dark variant metadata are retained
- [x] duplicate desktop roots produce one result and sorting is deterministic
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] profile theme discovery in the production GTK4 preferences awaits adapter integration
- [ ] system GTK4 desktop theme discovery awaits production GTK4 runtime integration
- [ ] CSS parsing, provider application, variant switching, and diagnostics are a later pass
- [ ] Windows light, dark, high-contrast, and custom-theme behavior await the production frontend

Scope: GTK4 theme discovery metadata only. GTK3 selection and application
remain unchanged, and no discovered GTK4 CSS is loaded by this pass.

### PR: [#111 - GTK4 Stage 7 CSS Provider Adapter](https://github.com/Fabulor/fabulor/pull/111)

Date: 2026-07-18

Migration stage: 7, GTK4 CSS-provider ownership pass 2

Files/workflows converted: display-scoped GTK4 provider ownership; base and
dark variant priority; follow-system/light/dark policy; parser diagnostics;
transactional replacement; disable and final teardown

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] shipping GTK3 frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 MSVC probe compiles, links, and executes with 0 warnings and 0 errors
- [x] fresh MSVC Meson/Ninja probe compiles 41 objects with warnings treated as errors
- [x] independent Meson runtime test passes 1/1
- [x] light and dark policy install one and two providers respectively
- [x] invalid and missing candidates preserve the active providers and identity
- [x] disable removes providers and clears active identity and variant state
- [x] Linux GCC C11 syntax check passes against GTK 4.8.3 with all warnings treated as errors

Manual checks:

- [ ] production GTK4 preferences and persistence await the next theme pass
- [ ] Windows light, dark, high-contrast, and custom-theme behavior await the production frontend

Scope: GTK4 CSS-provider ownership and failure containment only. Discovery is
not yet projected into production preferences, and shipping GTK3 theme behavior
remains unchanged.

### PR: [#112 - GTK4 Stage 7 Theme Preferences](https://github.com/Fabulor/fabulor/pull/112)

Date: 2026-07-18

Migration stage: 7, GTK4 preference-model and persistence pass 3

Files/workflows converted: owned GTK4 theme-choice projection; explicit system
default; exact persisted-selection resolution; unavailable-selection fallback;
variant validation; independent GTK4 configuration keys

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] shipping common library and full GTK3 frontend compile/link with 0 warnings and 0 errors
- [x] strict GTK4 MSVC probe compiles, links, and executes with 0 warnings and 0 errors
- [x] fresh MSVC Meson/Ninja probe compiles 42 objects with warnings treated as errors
- [x] independent Meson runtime test passes 1/1
- [x] Linux GCC C11 syntax checks pass with all warnings treated as errors
- [x] projected choices remain valid after discovery metadata is released
- [x] exact, missing, and intentional-default selections resolve deterministically
- [x] invalid stored variants normalize to follow-system
- [x] GTK4 configuration keys remain separate from GTK3 selection and variant keys

Manual checks:

- [ ] production GTK4 preference interaction and restart persistence await frontend cutover
- [ ] Windows light, dark, high-contrast, and custom-theme behavior await the production frontend

Scope: GTK4 preference ownership and persistence schema only. The shipping
GTK3 preference page remains unchanged and does not expose or apply GTK4 themes.

### PR: [#113 - GTK4 Stage 7 Windows Appearance Policy](https://github.com/Fabulor/fabulor/pull/113)

Date: 2026-07-18

Migration stage: 7, Windows light/dark/high-contrast policy pass 4

Files/workflows converted: resolved custom-theme eligibility; system-default
Windows following; explicit and automatic variant decisions; high-contrast CSS
suppression; unavailable-selection containment

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] shipping common library compiles with 0 warnings and 0 errors
- [x] strict GTK4 MSVC probe compiles, links, and executes with 0 warnings and 0 errors
- [x] Meson/Ninja GTK4 probe rebuilds and passes runtime test 1/1
- [x] Linux GCC C11 syntax check passes with all warnings treated as errors
- [x] custom follow-system, explicit light, and explicit dark decisions are deterministic
- [x] system-default mode follows Windows without enabling a custom provider
- [x] high contrast suppresses custom providers and dark requests
- [x] provider decision application removes an already-active custom theme
- [x] invalid variants normalize before appearance resolution

Manual checks:

- [ ] packaged Windows light/dark switching awaits the production GTK4 frontend
- [ ] packaged Windows high-contrast rendering awaits the production GTK4 frontend

Scope: GTK4 appearance decision policy only. Existing Win32 signal acquisition
is unchanged, and the shipping GTK3 frontend does not consume this decision.

### PR: [#114 - GTK4 Stage 7 Theme Controller](https://github.com/Fabulor/fabulor/pull/114)

Date: 2026-07-18

Migration stage: 7, GTK4 theme composition and lifecycle pass 5

Files/workflows converted: discovery-to-choice composition; persisted selection
resolution; appearance-to-provider application; transactional refresh;
unavailable-selection fallback; controller teardown and diagnostics

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: MSVC x64 Release and strict GTK4 probe boundary

Automated checks:

- [x] strict GTK4 MSVC probe compiles, links, and executes with 0 warnings and 0 errors
- [x] fresh MSVC Meson/Ninja probe compiles 43 objects with warnings treated as errors
- [x] independent Meson runtime test passes 1/1
- [x] Linux GCC C11 syntax check passes with all warnings treated as errors
- [x] choices remain valid after borrowed discovery metadata is released
- [x] invalid CSS preserves active providers and the committed selection
- [x] unavailable persisted selection commits an observable system-default fallback
- [x] high contrast removes active providers while retaining preference identity
- [x] controller destruction owns final provider and choice cleanup
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] production GTK4 preference interaction and restart persistence await frontend cutover
- [ ] packaged Windows appearance switching awaits the production GTK4 frontend

Scope: pre-production GTK4 theme composition and lifecycle only. The shipping
GTK3 frontend remains unchanged and does not instantiate this controller.

### PR: [#115 - GTK4 Stage 7 Notification Boundary](https://github.com/Fabulor/fabulor/pull/115)

Date: 2026-07-18

Migration stage: 7, notification backend loading and lifecycle pass 6

Files/workflows converted: executable-relative WinRT helper discovery; strict
export validation; owned module and call-target lifetime; partial-failure
rollback; WinRT initialization ordering; managed backend errors; freedesktop
and fallback lifecycle normalization

GTK version: shipping GTK3 frontend boundary; GTK4 production binding deferred

GLib version: shipping Windows runtime and Linux system GLib

Build configuration: MSVC x64 Release helper/frontend and strict Linux GCC C11
backend checks

Automated checks:

- [x] WinRT notification helper compiles and links with 0 warnings and 0 errors
- [x] shipping frontend compiles and links with 0 warnings and 0 errors
- [x] helper DLL exports all four required notification entry points
- [x] Windows helper path is derived from the executable installation root
- [x] missing exports and initialization failure unload the retained module
- [x] normal teardown clears call targets before module unload
- [x] WinRT initialization precedes notifier construction and is balanced on failure and teardown
- [x] freedesktop and dummy backends pass strict Linux GCC C11 syntax checks
- [x] backend initialization diagnostics use managed `GError` ownership

Manual checks:

- [ ] packaged Windows toast delivery and shutdown/restart behavior
- [ ] launch from an unrelated working directory with notifications enabled
- [ ] production GTK4 notification preferences and tray interaction await frontend cutover

Scope: platform notification loading, failure containment, and lifecycle only.
The shipping GTK3 presentation remains in use and no GTK4 notification UI is
claimed by this pass.

### PR: [#116 - GTK4 Stage 7 Tray Action Model](https://github.com/Fabulor/fabulor/pull/116)

Date: 2026-07-18

Migration stage: 7, toolkit-neutral tray action and state pass 7

Files/workflows converted: owned tray labels; stable menu/action namespace;
hide/restore projection; away/back sensitivity; stateful blink preferences;
typed activation dispatch; malformed-state normalization; final teardown

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: strict GTK4 MSVC probe, fresh MSVC Meson/Ninja probe, and
Linux GCC C11 syntax validation

Automated checks:

- [x] strict GTK4 MSVC probe compiles, links, and executes with 0 warnings and 0 errors
- [x] fresh MSVC Meson/Ninja probe compiles 44 objects with warnings treated as errors
- [x] independent Meson runtime test passes 1/1
- [x] Linux GCC C11 syntax check passes with all warnings treated as errors
- [x] caller-owned labels can be released or changed after model construction
- [x] visibility updates switch deterministically between hide and restore labels
- [x] away/back sensitivity follows all-away, all-back, and mixed snapshots
- [x] disabled actions do not dispatch and blink actions update state before dispatch
- [x] malformed away state normalizes to mixed
- [x] callback data receives exactly one final destroy notification
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] production GTK4 tray menu presentation awaits a presenter binding
- [ ] native Windows shell icon and menu behavior remain on the shipping GTK3 path
- [ ] dynamic `$TRAY` plugin entries await model composition

Scope: tray action and state ownership only. This pass does not replace the
shipping status icon, AppIndicator, Win32 popup, or tray timers.

### PR: [#117 - GTK4 Stage 7 Tray Live Binding](https://github.com/Fabulor/fabulor/pull/117)

Date: 2026-07-18

Migration stage: 7, production tray action-model binding pass 8

Files/workflows converted: production model lifetime; translated label input;
live visibility/away/blink snapshots; typed command routing; frontend away
refresh; on-demand presenter access; unchanged-state notification suppression;
production MSVC and Meson source registration

GTK version: shipping GTK3 frontend plus GTK4 4.22.4 probe

GLib version: shipping Windows runtime plus GTK4 probe 2.88.0

Build configuration: shipping MSVC x64 Release, strict GTK4 MSVC probe, fresh
MSVC Meson/Ninja probe, and Linux GCC C11 syntax validation

Automated checks:

- [x] shipping frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 MSVC probe compiles, links, and executes with 0 warnings and 0 errors
- [x] fresh MSVC Meson/Ninja probe compiles 44 objects with warnings treated as errors
- [x] independent Meson runtime test passes 1/1
- [x] Linux GCC C11 syntax check passes with all warnings treated as errors
- [x] window, settings, and away transitions feed live state into the model
- [x] every typed action routes to its existing application command or preference
- [x] repeated identical snapshots emit no menu item changes
- [x] model and plugin context are released during plugin deinitialization
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping tray menu remains on its unchanged GTK3/Win32 presentation path
- [ ] production GTK4 presenter interaction awaits the presentation pass
- [ ] dynamic `$TRAY` plugin entries await model composition

Scope: live action-model binding only. This pass does not replace tray icon,
popup presentation, flashing timers, or platform backend selection.

### PR: [#118 - GTK4 Stage 7 Tray Plugin Composition](https://github.com/Fabulor/fabulor/pull/118)

Date: 2026-07-18

Migration stage: 7, dynamic tray plugin-composition pass 9

Files/workflows converted: retained `$TRAY` plugin model ownership; immutable
built-in/plugin menu composition; stable action namespaces; caller-owned
projection references; inert retained-action teardown; production and probe
source registration

GTK version: shipping GTK3 frontend plus GTK4 4.22.4 probe

GLib version: shipping Windows runtime plus GTK4 probe 2.88.0

Build configuration: shipping MSVC x64 Release, strict GTK4 MSVC probe, fresh
MSVC Meson/Ninja probe, and Linux GCC C11 syntax validation

Automated checks:

- [x] shipping frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 MSVC probe compiles, links, and executes with 0 warnings and 0 errors
- [x] fresh MSVC Meson/Ninja probe compiles 45 objects with warnings treated as errors
- [x] independent Meson runtime test passes 1/1
- [x] Linux GCC C11 syntax check passes with all warnings treated as errors
- [x] plugin section insertion preserves links, actions, and command metadata
- [x] oversized and zero insertion indices remain within menu bounds
- [x] composed menus survive source plugin-model release
- [x] retained built-in actions are disabled and disconnected during teardown
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping tray presentation remains on its unchanged GTK3/Win32 path
- [ ] production GTK4 presenter interaction awaits the presentation pass
- [ ] dynamic plugin command activation awaits a presenter consuming both action groups

Scope: dynamic menu and action-group ownership only. This pass does not replace
the tray icon, popup renderer, flashing timers, or platform backend selection.

### PR: [#119 - GTK4 Stage 7 Tray Popover Presenter](https://github.com/Fabulor/fabulor/pull/119)

Date: 2026-07-18

Migration stage: 7, GTK4 popover-presenter ownership pass 10

Files/workflows converted: candidate `GtkPopoverMenu` ownership; composed-model
binding; built-in and plugin action-group attachment; projection replacement;
popover close/unparent teardown; shared action namespace constants

GTK version: shipping GTK3 frontend plus GTK4 4.22.4 probe

GLib version: shipping Windows runtime plus GTK4 probe 2.88.0

Build configuration: shipping MSVC x64 Release, strict GTK4 MSVC probe, fresh
MSVC Meson/Ninja probe, and Linux GCC C11 syntax validation

Automated checks:

- [x] shipping frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 MSVC probe compiles, links, and executes with 0 warnings and 0 errors
- [x] fresh MSVC Meson/Ninja probe compiles 46 objects with warnings treated as errors
- [x] independent Meson runtime test passes 1/1
- [x] Linux GCC C11 syntax check passes with all warnings treated as errors
- [x] both action namespaces activate through the GTK4 popover
- [x] projection replacement dispatches only through replacement groups
- [x] caller menu and action references can be released after presenter binding
- [x] retained popovers expose no menu or actions after presenter teardown
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] production GTK4 popover anchoring awaits the frontend cutover boundary
- [ ] native Windows tray icon and popup behavior remain on the shipping path
- [ ] GTK4 tray keyboard, high-DPI, and repeated-popup checks await integration

Scope: GTK4 popover ownership and action routing only. This pass does not select
a platform tray backend, own a shell icon, or replace the shipping popup.

### PR: [#120](https://github.com/Fabulor/fabulor/pull/120) - GTK4 Stage 7 Tray Backend Selection

Date: 2026-07-18

Migration stage: 7, tray backend-selection policy pass 11

Files/workflows converted: explicit backend capability environment; disabled,
Windows shell, StatusNotifier, GTK3 legacy, and unavailable outcomes; shipping
startup and preference-restart selection; stable backend diagnostics

GTK version: shipping GTK3 frontend plus GTK4 4.22.4 probe

GLib version: shipping Windows runtime plus GTK4 probe 2.88.0

Build configuration: shipping MSVC x64 Release, strict GTK4 MSVC probe, fresh
MSVC Meson/Ninja probe, and Linux GCC C11 syntax validation

Automated checks:

- [x] shipping frontend compiles and links with 0 warnings and 0 errors
- [x] strict GTK4 MSVC probe compiles, links, and executes with 0 warnings and 0 errors
- [x] fresh MSVC Meson/Ninja probe compiles 47 objects with warnings treated as errors
- [x] independent Meson runtime test passes 1/1
- [x] Linux GCC C11 syntax check passes with all warnings treated as errors
- [x] disabled preference overrides every available backend
- [x] Windows never falls through to a Unix backend
- [x] available StatusNotifier takes precedence on Unix-like builds
- [x] GTK3 may use the legacy status-icon fallback when available
- [x] GTK4 and unknown toolkit versions reject the legacy fallback
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] shipping Windows tray behavior awaits packaged regression testing
- [ ] GTK4 StatusNotifier integration awaits a production backend
- [ ] unavailable-backend diagnostics await a user-facing integration point

Scope: backend selection and shipping decision integration only. This pass does
not implement a new shell icon, StatusNotifier service, or popup renderer.

### PR: [#121](https://github.com/Fabulor/fabulor/pull/121) - GTK4 Stage 7 Theme Preference Binding

Date: 2026-07-18

Migration stage: 7, GTK4 theme-preference binding pass 12

Files/workflows converted: owned GTK4 theme and variant controls; lifecycle
controller binding; transactional selection and persistence callback; missing
theme, invalid CSS, and high-contrast status; parented-widget teardown

GTK version: GTK4 4.22.4 probe

GLib version: GTK4 probe 2.88.0

Build configuration: strict GTK4 MSVC probe, fresh MSVC Meson/Ninja probe, and
Linux GCC C11 syntax validation

Automated checks:

- [x] strict GTK4 MSVC probe compiles, links, and executes with 0 warnings and 0 errors
- [x] fresh MSVC Meson/Ninja probe compiles 48 objects with warnings treated as errors
- [x] independent Meson runtime test passes 1/1
- [x] Linux GCC C11 syntax check passes with all warnings treated as errors
- [x] successful theme and variant changes emit committed stable values
- [x] invalid CSS restores the prior selection and does not emit persistence
- [x] system-default selection removes the active custom provider
- [x] high-contrast refresh suppresses custom CSS without persistence writes
- [x] unavailable saved themes expose system-default fallback status
- [x] teardown unparents an attached surface and disconnects its controls
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] production GTK4 preferences-window insertion awaits frontend cutover
- [ ] packaged Windows light, dark, and high-contrast testing remains pending
- [ ] imported theme selection awaits the production GTK4 build

Scope: candidate GTK4 preference ownership and persistence boundary only. This
pass does not alter the shipping GTK3 preference page or GTK3 theme service.

### PR: [#122](https://github.com/Fabulor/fabulor/pull/122) - GTK4 Stage 7 Windows Appearance Monitor

Date: 2026-07-18

Migration stage: 7, Windows appearance-monitor pass 13

Files/workflows converted: GTK4 display-scoped Win32 filter; queued and
coalesced theme signals; Windows registry and high-contrast query; unchanged
state suppression; controller refresh; failure diagnostics; filter/source
teardown

GTK version: GTK4 4.22.4 probe

GLib version: GTK4 probe 2.88.0

Build configuration: strict GTK4 MSVC probe, fresh MSVC Meson/Ninja probe, and
Linux GCC C11 syntax validation

Automated checks:

- [x] strict GTK4 MSVC probe compiles, links, and executes with 0 warnings and 0 errors
- [x] fresh MSVC Meson/Ninja probe compiles 49 objects with warnings treated as errors
- [x] independent Meson runtime test passes 1/1
- [x] Linux GCC C11 syntax check passes with all warnings treated as errors
- [x] repeated appearance messages coalesce into one queued query
- [x] unchanged state does not reapply providers
- [x] follow-system dark state activates the dark provider
- [x] high contrast removes custom providers without a persistence write
- [x] query failure retains committed monitor and controller state
- [x] teardown cancels queued work before releasing callback data
- [x] repository diff whitespace validation passes

Manual checks:

- [ ] production GTK4 startup ownership awaits frontend cutover
- [ ] packaged Windows light/dark switching remains pending
- [ ] packaged Windows high-contrast rendering remains pending

Scope: candidate GTK4 Windows appearance monitoring only. This pass does not
replace the shipping GTK3 window filter or alter current packaged behavior.

### PR: [#123](https://github.com/Fabulor/fabulor/pull/123) - GTK4 Stage 7 Theme Format And Payload Contract

Date: 2026-07-18

Migration stage: 7, theme-format and payload-contract pass 14

Files/workflows converted: active `.hct` association enforcement;
`colors.conf` import and atomic persistence assertions; `.zct` exclusion with
stale-install cleanup; repository payload and WiX harvest exclusions; PR lint

Build configuration: Python 3.14 contract validator and isolated unittest
fixtures on the Windows repository-lint path

Automated checks:

- [x] current repository theme-format and payload contract passes
- [x] active WiX associations are exactly `.hct`
- [x] legacy installer template registers `.hct` and not `.zct`
- [x] stale `.zct` upgrade cleanup remains present
- [x] active import filters retain `.hct`, `colors.conf`, and `pevents.conf`
- [x] runtime retains `colors.conf` loading and atomic replacement
- [x] tracked repository payload roots contain no optional default theme
- [x] WiX harvest rules contain no default-theme formats or `share/themes`
- [x] negative test rejects a reintroduced `.zct` association
- [x] negative tests reject tracked and WiX-harvested default themes
- [x] Python syntax and repository diff whitespace validation pass

Manual checks:

- [ ] final staged GTK4 payload inspection remains part of Stage 8 cutover
- [ ] clean-install `.hct` shell-open behavior remains a packaged validation

Scope: source and CI enforcement of supported theme formats and package inputs.
This pass does not remove the shipping GTK3 theme service or inspect a final
Stage 8 release payload.

### PR: [#124](https://github.com/Fabulor/fabulor/pull/124) - GTK4 Stage 8 Runtime Staging Contract

Date: 2026-07-18

Migration stage: 8, deterministic runtime-candidate staging pass 1

Files/workflows converted: Windows x64 runtime payload contract; contained
candidate staging and loader-cache normalization; source-bound SHA-256 output
manifest; repository lint and Windows build validation

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: pinned `Runtime/GTK4` source root, Python 3.14 staging and
negative-path tests, Windows CI candidate materialization before GTK4 probes

Automated checks:

- [x] seven staging contract, failure cleanup, and path-safety tests pass
- [x] eight pinned GTK4 dependency-root tests pass
- [x] four theme-format and payload tests pass
- [x] nine Python capability and seven manifest-isolation tests pass
- [x] stale Python greeter callback assertion matches the current sample event
- [x] actual candidate contains 1,431 files and 102,726,736 payload bytes
- [x] candidate manifest is bound to source archive SHA-256
  `3910a612083c2a155c5a4a2026990701841c0d7f7de28756b2f0865decb161be`
- [x] candidate contains no `.pdb`, import library, header, pkg-config, GIR, or
  Python build artifact selected by the contract
- [x] GDK pixbuf loader cache contains no build-machine path
- [x] workflow YAML, Python syntax, and repository diff whitespace checks pass

Manual checks:

- [ ] executable-relative startup against the candidate awaits a production
  GTK4 frontend target
- [ ] packaged icons, fonts, emoji, translations, spawn, and SVG behavior await
  the parallel-package validation pass
- [ ] WiX remains on its transitional broad harvest until candidate validation
  is complete

Scope: deterministic generation of the first allowlisted GTK4 runtime candidate.
This pass does not switch production linking, root staging, or installer harvest
rules and does not remove the shipping GTK3 payload.

### PR: [#125](https://github.com/Fabulor/fabulor/pull/125) - GTK4 Stage 8 Parallel Candidate MSI

Date: 2026-07-18

Migration stage: 8, parallel candidate-package pass 2

Files/workflows converted: opt-in WiX candidate composition; explicit GTK4
runtime directory ownership; isolated MSI output; MSI decompile, extraction,
path, size, and SHA-256 validation; candidate artifact upload

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: Windows x64 WiX 7 candidate MSI with default production
MSI/bootstrapper composition retained and built first

Automated checks:

- [x] candidate MSI builds locally with zero warnings and zero errors
- [x] default MSI and bootstrapper still build with candidate mode disabled
- [x] candidate is named `FabulorGtk4RuntimeCandidate.msi`
- [x] candidate installs all 1,431 manifest payload paths plus the manifest
- [x] decompiled MSI contains zero missing, unexpected, duplicate, or flattened
  GTK4 paths
- [x] all extracted candidate GTK4 sizes and SHA-256 hashes match the manifest
- [x] five candidate path-layout and rejection tests pass
- [x] seven staging, eight dependency-root, four theme-contract, nine Python
  capability, and seven manifest-isolation tests pass
- [x] installer XML, workflow YAML, Python syntax, and diff whitespace checks pass

Manual checks:

- [ ] local ICE validation is unavailable because this session cannot access the
  Windows Installer service; local composition used `SuppressValidation=true`
- [x] GitHub candidate build retains normal unsuppressed ICE validation
- [ ] candidate artifact inspection on a clean machine remains pending
- [ ] the candidate executable remains GTK3 and is not a GTK4 application smoke
  test

Scope: parallel packaging and exact payload integrity validation only. Default
WiX harvesting, the shipping bootstrapper, production linking, and root GTK3
payload remain unchanged.

### PR: [#126](https://github.com/Fabulor/fabulor/pull/126) - GTK4 Stage 8 Executable-Relative Runtime Startup

Date: 2026-07-18

Migration stage: 8, executable-relative runtime startup pass 3

Files/workflows converted: Win32-only GTK4 runtime bootstrap; standalone loader
probe; candidate-root staging layout; decoy, missing-root, and reparse-point
integration validation

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: Windows x64 MSVC Release bootstrap probe against the
deterministic 1,431-file candidate root

Automated checks:

- [x] bootstrap and probe compile with warning level 4 and warnings as errors
- [x] shared `common.vcxproj` rebuild succeeds with the bootstrap source
- [x] probe imports only Windows/UCRT support DLLs before runtime configuration;
  it has no GTK or GLib import
- [x] unrelated-current-directory launch succeeds with ambient GTK paths
  removed and an invalid `gtk-4-1.dll` decoy present
- [x] loaded `gtk-4-1.dll` resolves beneath the executable-relative
  `Runtime/GTK4/bin` directory and reports GTK major version 4
- [x] repeated bootstrap configuration is idempotent
- [x] missing runtime and junction-backed runtime roots fail closed
- [x] candidate staging retains 1,431 files and its source-bound manifest
- [x] project XML, workflow YAML, Python syntax, and diff whitespace checks pass

Manual checks:

- [ ] clean-machine execution awaits the GitHub Windows candidate run
- [ ] production `fabulor.exe` remains directly linked to GTK3 and is not tested
  by this probe
- [ ] final GTK4 linking must delay-load GTK-family imports or place the frontend
  behind the bootstrap before this nested runtime can start production code

Scope: executable-relative Windows loader discovery and fail-closed path policy
only. Production linking, the shipping GTK3 executable, installer selection,
and root GTK3 payload remain unchanged.

### PR: [#127](https://github.com/Fabulor/fabulor/pull/127) - GTK4 Stage 8 Native Import Closure

Date: 2026-07-18

Migration stage: 8, native import-closure pass 4

Files/workflows converted: versioned native import contract; dumpbin-backed PE
closure validator; ownership-root reachability; lint rejection tests; full
candidate graph validation in Windows CI

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: deterministic Windows x64 candidate root inspected with
MSVC 14.44 `dumpbin`

Automated checks:

- [x] all 35 staged DLL/executable files have unique case-insensitive basenames
- [x] four owned roots reach every packaged native file
- [x] all 107 packaged import edges resolve within the candidate
- [x] all 54 distinct external imports match the reviewed Windows/UCRT surface
- [x] no GTK3 or lib-prefixed duplicate GLib-family import is present
- [x] eight focused missing, forbidden, duplicate, unowned, root, inspection,
  parser, and valid-closure tests pass
- [x] CI-equivalent `dumpbin` discovery and candidate validation pass locally
- [x] contract JSON, workflow YAML, Python syntax, and diff whitespace checks
  pass

Manual checks:

- [ ] clean-run evidence awaits the GitHub Windows candidate build
- [ ] dynamically selected modules and runtime data still require packaged
  feature tests before trimming
- [ ] production `fabulor.exe` remains GTK3 and is not part of this import graph

Scope: static candidate PE dependency closure and native ownership only.
Production linking, dynamic feature coverage, payload trimming, installer
selection, and the shipping GTK3 runtime remain unchanged.

### PR: [#128](https://github.com/Fabulor/fabulor/pull/128) - GTK4 Stage 8 Shipping WiX Allowlist

Date: 2026-07-18

Migration stage: 8, shipping WiX allowlist pass 5

Files/workflows converted: normal WiX GTK4 component selection; staged-root
requirement; shipping MSI payload validation; duplicate candidate build and
artifact retirement

GTK version: 4.22.4

GLib version: 2.88.0

Build configuration: Windows x64 normal `Fabulor.msi` and `FabulorSetup.exe`
against the deterministic staged allowlist

Automated checks:

- [x] broad `GTK4.wxs` and its empty `lib/gio` harvest are removed
- [x] `GTK4Allowlist.wxs` is the sole `GTK4Components` provider
- [x] normal WiX builds fail closed without `runtime-manifest.json`
- [x] normal `Fabulor.msi` and `FabulorSetup.exe` build against the staged root
- [x] shipping MSI contains all 1,431 manifest paths plus the manifest
- [x] shipping MSI extraction reports zero missing, unexpected, duplicate,
  flattened, size-mismatched, or hash-mismatched GTK4 entries
- [x] duplicate candidate MSI build and artifact upload are removed
- [x] installer XML, workflow YAML, Python syntax, and diff whitespace checks
  pass

Manual checks:

- [ ] clean-run unsuppressed ICE and bootstrapper evidence awaits GitHub CI
- [ ] installed clean/upgrade/repair/uninstall tests remain pending
- [ ] production executable and root payload remain GTK3 in this pass

Scope: shipping WiX GTK4 component selection and exact MSI payload validation.
Production frontend linking, root GTK3 staging/removal, and installed feature
validation remain unchanged.

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

## 2026-07-19 - Stage 8 Full-Project MSVC Build Profile

Automated evidence:

- isolated `BuildCommon` against `Runtime\GTK4`: pass; full `common.lib`
  produced beneath `build\gtk4-full`
- isolated complete frontend compile against `Runtime\GTK4`: expected fail;
  compilation reaches the production frontend and reports the remaining
  GTK3-only boundaries instead of failing dependency discovery
- default x64 Release `common.vcxproj`: pass
- default x64 Release `fe-gtk.vcxproj`: pass; shipping
  `C:\zoitechat-build\x64\rel\fabulor.exe` produced
- profile XML and whitespace validation: pass

Current frontend blocker groups:

- raw `GdkEventButton` and legacy menu/widget construction
- channel-list and channel-view GTK3 container, icon-size, and child APIs
- removed window position, screen, visibility, and destruction APIs
- direct GTK3 theme backend use in application and preference integration

Packaging impact: none. CI runs only the green isolated common checkpoint. The
full frontend target is not referenced by staging, WiX, or the shipping
solution and cannot overwrite the production executable.

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
