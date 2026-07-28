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
- [x] light and dark policy each install one resolved complete provider
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

## 2026-07-19 - Stage 8 Compositor-Owned Window Placement

Automated evidence:

- strict MSVC GTK4 probe: pass; helper signatures compile, link, and execute
- default x64 Release common library: pass
- default x64 Release GTK3 frontend: pass; shipping `fabulor.exe` produced
- isolated GTK4 common library: pass
- complete isolated GTK4 frontend inventory: expected fail at later menu,
  channel-view, lifecycle, and theme blockers; no direct placement API error
- Meson/GCC probe: all 49 objects compile; local link is unavailable because
  Strawberry GCC 4.8.3 cannot consume the MSVC GTK4 import-library ABI
- source audit: no shared direct `gtk_window_set_position`, `gtk_window_move`,
  `gtk_window_get_position`, or tray `GdkScreen` placement call remains
- `git diff --check`: pass

Behavior contract:

- GTK3 pointer, centered, parent-centered, saved-coordinate, and saved-screen
  behavior remains unchanged behind the compatibility owner.
- GTK4 preserves transient parents, maximize/fullscreen state, and dimensions,
  but does not request or persist native coordinates unavailable from GTK4.
- Legacy X11 status-icon probing remains GTK3-only; GTK4 tray backend selection
  continues through the existing StatusNotifier/native policy.

Manual GTK4 placement and multi-monitor checks remain deferred until the full
frontend links. Packaging impact: none.

## 2026-07-19 - Stage 8 Context-Menu Event Boundary

Automated evidence:

- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: pass
- default x64 Release GTK3 frontend: pass with zero warnings and zero errors;
  shipping `C:\zoitechat-build\x64\rel\fabulor.exe` produced
- isolated complete GTK4 frontend inventory: expected fail at later legacy
  channel-view, dialog, lifecycle, and theme boundaries
- source audit: `menu.h` exposes no `GdkEventButton`; `menu.c` retains one
  synthetic button event inside its private GTK3-only popup presenter
- `git diff --check`: pass

Behavior contract:

- GTK3 URL, channel, nick, user-list, transcript, and middle-click menus retain
  their existing pointer-relative coordinates, modifier state, and button.
- GTK4 does not claim built-in context-menu support in this pass. The remaining
  legacy menu construction must be projected into the retained action/model
  presenter before real-time GTK4 menu testing begins.
- No installer or runtime payload changes are included.

## 2026-07-19 - Stage 8 Context-Menu Presenter

Automated evidence:

- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: pass
- presenter probe uses a real toplevel origin and verifies coordinate
  placement, built-in and plugin action activation, and complete detachment
- MSVC and Meson GTK4 probe manifests include the same presenter source
- `git diff --check`: pass

Behavior contract:

- the presenter retains its model and both action groups for popup lifetime
- changing origins first pops down and unparents the existing popover
- teardown removes both namespaces, clears the model, and unparents the widget
- legacy context commands are not projected or shown by this foundation pass
- packaging impact: none

## 2026-07-19 - Stage 8 URL Context Model

Automated evidence:

- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: pass
- URL model probe: IRC Connect label, Open and Copy typed dispatch, retained URL,
  and plugin-section composition pass
- Meson 1.11.2 / Ninja 1.13.2: all 51 objects compile, including the URL model
- local Meson final link: unavailable because Strawberry GCC 4.8.3 cannot link
  the MSVC GTK4 import libraries; CI remains the supported Meson link path
- `git diff --check`: pass

Scope: fixed URL actions and plugin composition only. Configurable URL handlers
and the live GTK4 presenter connection remain open. Packaging impact: none.

## 2026-07-19 - Stage 8 URL Handler Projection

Automated evidence:

- strict MSVC GTK4 probe: pass
- nested submenu and separator-section projection: pass
- enabled handlers dispatch once; disabled handlers do not dispatch: pass
- stateful handlers toggle and dispatch `set <preference> 0|1`: pass
- action closures retain copied URL-handler commands: pass
- icon metadata and plugin-section ordering remain model-owned
- `git diff --check`: pass

Scope: configured URL-handler projection. The live GTK4 presenter adapter
remains open. The shipping GTK3 menu is unchanged.

## 2026-07-19 - Stage 8 Live URL Context Adapter

Automated evidence:

- strict MSVC GTK4 model/presenter probe: pass
- default x64 Release GTK3 frontend: pass with zero warnings and zero errors
- isolated full GTK4 frontend inventory: expected fail at older menu and other
  frontend blockers; no error in the new URL adapter range
- source audit: GTK4 snapshots and path-filters `urlhandler_list`, rebuilds the
  `$URL` plugin projection, and binds one popup owner to the origin widget
- temporary handler labels/icons are released after the retained model copies
  action commands and state
- `git diff --check`: pass

Behavior contract: GTK3 remains on its existing widget menu. GTK4 now has the
complete retained URL context path, pending a linkable full frontend for manual
Open/Connect/Copy, custom-handler, plugin, and repeated-popup testing.
Packaging impact: none.

## 2026-07-19 - Stage 8 Channel Context Model

Automated evidence:

- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: pass with zero
  warnings and zero errors
- joined, non-current model exposes Focus, Part, Cycle, and stateful Autojoin
- all joined actions dispatch once and Autojoin transitions from false to true
- model retains a copied channel name after the caller releases its input
- unjoined, networkless model exposes Join without an Autojoin action
- plugin-section composition and stable shared action namespaces: pass
- Meson 1.11.2 / Ninja 1.13.2: all 52 objects compile under `-Werror`, including
  the channel model and probe
- local Meson final link: unavailable because Strawberry GCC 4.8.3 cannot link
  the MSVC GTK4 import libraries; CI remains the supported Meson link path
- `git diff --check`: pass

Scope: retained channel actions and plugin composition only. The live GTK4
presenter adapter remains open pending safe server/network lifetime handling.
The shipping GTK3 menu and packaging are unchanged.

## 2026-07-19 - Stage 8 Live Channel Context Adapter

Automated evidence:

- strict MSVC GTK4 model/presenter probe: pass with zero warnings and zero errors
- default x64 Release GTK3 frontend: pass with zero warnings and zero errors
- isolated full GTK4 frontend inventory: expected fail at older frontend
  blockers; the channel model compiles and the complete live adapter range has
  no diagnostic
- source audit: GTK4 projects joined/current/network/Autojoin state, rebuilds
  the `$CHAN` plugin projection, and binds one popup owner to the origin widget
- Autojoin retains a copied network name and resolves the current `ircnet` on
  activation; no raw `server`, `session`, or `ircnet` pointer survives in the
  popup owner
- Join, Focus, Part, and Cycle preserve the existing current-session command
  dispatch forms
- `git diff --check`: pass

Behavior contract: GTK3 remains on its existing widget menu. GTK4 now has the
complete retained channel context path, pending a linkable full frontend for
manual joined/unjoined, current/non-current, Autojoin persistence, plugin,
network-removal, and repeated-popup testing. Packaging impact: none.

## 2026-07-19 - Stage 8 Nick Context Model

Automated evidence:

- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: pass with zero
  warnings and zero errors
- single-user model preserves heading, Reply, and plugin section ordering
- Reply dispatches once after the caller releases its nick buffer
- multi-selection model preserves its supplied heading and omits Reply
- shared retained context action namespace: pass
- Meson 1.11.2 / Ninja 1.13.2: all 53 source objects compile under `-Werror`,
  including the nick model and probe
- local Meson final link: unavailable because Strawberry GCC 4.8.3 cannot link
  the MSVC GTK4 import libraries; CI remains the supported Meson link path
- `git diff --check`: pass

Scope: fixed nick heading, Reply, and plugin composition only. Recursive
`popup.conf`, user-info/WHOIS refresh, and the live GTK4 presenter connection
remain open. The shipping GTK3 menu and packaging are unchanged.

## 2026-07-19 - Stage 8 Nick Popup Projection

Automated evidence:

- strict MSVC GTK4 probe: pass with zero warnings and zero errors
- recursive submenu and separator-section projection: pass
- enabled commands dispatch; disabled commands remain suppressed: pass
- stateful toggle transitions and dispatches `set <preference> 0|1`: pass
- command actions retain copied storage after the caller releases its buffer
- ordinary multi-nick commands carry selection dispatch; toggles do not
- icon metadata and heading/command/Reply/plugin section ordering remain
  model-owned
- Meson 1.11.2 / Ninja 1.13.2: all 53 source objects compile under `-Werror`
- local Meson final link remains unavailable at the documented Strawberry GCC
  4.8.3 versus MSVC import-library boundary
- `git diff --check`: pass

Scope: recursive `popup.conf` projection and multi-selection policy. User-info
copy actions, WHOIS refresh, and the live GTK4 presenter adapter remain open.
The shipping GTK3 menu and packaging are unchanged.

## 2026-07-19 - Stage 8 Nick Information Model

Automated evidence:

- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: pass with zero
  warnings and zero errors
- copyable information rows retain caller-released labels and clipboard values
- display-only Last Msg rows expose no action
- incomplete and complete information refresh states remain model-owned
- existing fixed-heading construction and command/Reply/plugin ordering remain
  compatible
- Meson 1.11.2 / Ninja 1.13.2: all 53 source objects compile under `-Werror`
- local Meson final link remains unavailable at the documented Strawberry GCC
  4.8.3 versus MSVC import-library boundary

Scope: retained user-information rows, typed copy dispatch, and safe refresh
intent only. Live IRC snapshots, WHOIS dispatch, and the GTK4 presenter adapter
remain open. No `User`, `session`, server, or widget pointer is retained by the
model. The shipping GTK3 menu and packaging are unchanged.

## 2026-07-19 - Stage 8 Live Nick Context Adapter

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass with zero warnings and zero
  errors
- strict MSVC GTK4 probe: pass with zero warnings and zero errors
- isolated full GTK4 frontend inventory: expected fail at older frontend
  blockers; the complete live nick-adapter range and nick model compile without
  a diagnostic
- source audit: GTK4 snapshots `popup.conf`, user details, Reply, and `$NICK`
  plugin state before presentation
- source audit: single-target commands use copied nick storage; multi-selection
  commands resolve the live user-list selection at activation
- source audit: incomplete details issue WHOIS only from the valid construction
  session, and refresh matches a weak origin against copied nick/network identity
- source audit: popup ownership retains no `User`, `session`, `server`, popup
  configuration entry, or widget pointer
- `git diff --check`: pass

Behavior contract: GTK3 remains on its existing widget menu. GTK4 now has the
complete retained nick context path, pending a linkable full frontend for manual
single/multi-selection, copy, Reply, notify, plugin, disconnect, WHOIS refresh,
and repeated-popup testing. Packaging impact: the GTK4-only frontend project
now includes the nick model; the shipping GTK3 payload is unchanged.

## 2026-07-19 - Stage 8 Middle Context Model

Automated evidence:

- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: pass with zero
  warnings and zero errors
- ordered built-in top-level section composition: pass
- caller-released section labels and source models remain retained: pass
- stable plugin paths match display labels independent of mnemonic underscores
- matching plugin content remains a separate submenu section: pass
- unmatched plugin roots remain top-level and ordered after built-in sections
- Meson 1.11.2 / Ninja 1.13.2: all 54 source objects compile under `-Werror`
- local Meson final link remains unavailable at the documented Strawberry GCC
  4.8.3 versus MSVC import-library boundary
- `git diff --check`: pass

Scope: retained middle-click application-menu composition only. Presenter
support for the main `fabulor` action namespace and the production middle-click
adapter remain open. The model owns no action group, widget, session, or server
state. The shipping GTK3 menu and packaging are unchanged.

## 2026-07-19 - Stage 8 Live Middle Context Adapter

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass with zero warnings and zero
  errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: pass with zero
  warnings and zero errors
- configurable presenter namespace and nullable plugin namespace: action
  activation and cleanup pass
- isolated full GTK4 frontend inventory: expected fail at older frontend
  blockers; the changed middle-click adapter, presenter, and model ranges add no
  diagnostic
- Meson 1.11.2 / Ninja 1.13.2: changed sources compile under `-Werror`; local
  final link remains unavailable at the documented Strawberry GCC 4.8.3 versus
  MSVC import-library boundary
- source audit: the clicked origin owns presenter, model, dynamic action data,
  and replacement cleanup without retaining a `session` or `server`
- source audit: built-in, user, and plugin actions share the main `fabulor`
  namespace and state preparation used by the shipping menu
- `git diff --check`: pass

Behavior contract: GTK3 remains on its accelerator-backed widget menu. GTK4
now presents the retained Fabulor, View, Server, optional User, Settings,
Window, Help, matched plugin, and unmatched plugin sections at the click
origin. Manual middle-click placement, repeated-popup, toggle, disabled-action,
user-command, and `/MENU` testing remains gated on a linkable full GTK4
frontend. Packaging remains unchanged apart from registering the already
probed middle model in the GTK4-only frontend source list.

## 2026-07-19 - Stage 8 Live Main Menu Bar

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass with zero warnings and zero
  errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: pass with zero
  compiler warnings, zero errors, and no GTK runtime warning
- `GtkPopoverMenuBar` model attachment, `fabulor` action activation, retained
  model replacement, namespace removal, and cleanup: pass
- isolated full GTK4 frontend inventory: expected fail at older blockers;
  reduced from 236 errors / 380 warnings to 225 errors / 365 warnings
- changed `menu_create_main()` and `mg_create_menu()` GTK4 ranges compile
  without a diagnostic
- Meson 1.11.2 / Ninja 1.13.2: updated probe compiles under `-Werror`; local
  final link remains unavailable at the documented Strawberry GCC 4.8.3 versus
  MSVC import-library boundary
- source audit: GTK3 menu construction and accelerator registration remain in
  their original branch
- source audit: GTK4 state proxies are menu-bar-owned and retain no session or
  server pointer
- source audit: user-menu and `/MENU` changes rebuild the retained root and do
  not invoke GTK3 widget mutation
- `git diff --check`: pass

Behavior contract: GTK4 creates and attaches a retained popover menu bar with
Fabulor, View, Server, optional User, Settings, Window, Help, matched plugin,
and Add-ons roots. Existing action-state updates continue through the shared
action group. GTK3 retains its widget tree, accelerators, mnemonics, and custom
font walk. Manual menu opening, every built-in action, toggles, selection
states, disabled server actions, dynamic user commands, `/MENU` add/update/
delete, menu hiding, and repeated window lifecycle testing remain gated on a
linkable full GTK4 frontend.

### GTK4 Stage 8 Main-Menu Accelerator Boundary

Date: 2026-07-20

Files/workflows converted: version-specific `menu_create_main()` API;
GTK3-only icon-menu construction, accelerator registration, key metadata, and
recursive accelerator refresh; GTK4 canonical shortcut refresh policy.

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- isolated complete GTK4 frontend inventory: expected fail at later legacy
  frontend boundaries; reduced from 225 errors / 365 warnings to 217 errors /
  363 warnings
- changed GTK4 main-menu constructor and accelerator-refresh ranges: no
  compiler diagnostic
- source audit: GTK4 no longer declares or passes `GtkAccelGroup`; GTK3 keeps
  its existing concrete owner, visible accelerators, Ctrl+Q policy, and menu
  widget traversal
- source audit: GTK4 shortcut changes require no widget rebuild because
  configurable dispatch reads the current binding table through canonical
  action identities
- `git diff --check`: pass

Local invocation notes: the sandbox environment exposed duplicate `Path`/`PATH`
entries to the VS 2022 v143 compiler task, so the strict probe was run through
the approved unsandboxed build invocation. The shipping project required
`CL_MPCount=1` after earlier interrupted builds left its shared PDB contended;
the serial rebuild then completed cleanly. Neither condition required a source
or project-file change.

Behavior contract: GTK3 retains the displayed menu hierarchy, mnemonics,
accelerators, custom Ctrl+Q preference behavior, and dynamic rebinding. GTK4
retains the live popover menu bar and canonical key-action dispatch introduced
by earlier passes. Manual GTK4 keyboard traversal and displayed shortcut-label
validation remain gated on the linkable full frontend. Packaging impact: none.

### GTK4 Stage 8 Tab Context-Menu Presentation

Date: 2026-07-20

Files/workflows converted: channel-view tab popup model and live GTK4
presentation; per-session alert/settings state; Autojoin and Auto-Connect;
recursive `tabmenu.conf` commands and toggles; `$TAB` plugin composition;
Detach/Close and popup replacement/destruction ownership.

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- retained model structure, option state transition, Autojoin, Detach, Close,
  recursive configured command/toggle ownership, plugin section, and utility
  tab surface: pass
- isolated complete GTK4 frontend inventory: expected fail at the older Win32
  window-state header before the new `maingui.c` adapter; unchanged at 217
  errors / 363 warnings
- source audit: model actions own copied commands and labels; source-owned
  teardown is deferred beyond active action dispatch; session actions validate
  live session membership before dereference
- source audit: GTK3 keeps its existing tab widget menu, option callbacks,
  pointer placement, and accelerator behavior
- Meson 1.11.2 / Ninja 1.13.2: local attempt became idle with no compiler or
  linker child and was terminated; the strict MSVC probe supplies executable
  evidence for this isolated model
- `git diff --check`: pass

Behavior contract: GTK4 presents the same tab heading, alert and settings
submenus, network toggle, Detach/Close commands, configured tab commands, and
plugin entries at the click coordinates. Utility tabs expose only Detach and
Close. Manual repeated-popup placement and live command testing remain gated
on the linkable full GTK4 frontend. Packaging impact: none.

### GTK4 Stage 8 Top-Level Window-State Boundary

Date: 2026-07-20

Files/workflows converted: minimized, maximized, fullscreen, and focused
top-level observation; main-window state persistence and relayout;
minimize-to-tray and tray action refresh; fullscreen action synchronization;
Windows auto-hide taskbar adjustment and native handle resolution.

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- state-difference flags, unrealized-window snapshot, multiple observer
  ownership, safe destruction, and unavailable-native-handle behavior: pass
- isolated complete GTK4 frontend inventory: expected fail with 261 errors /
  552 warnings; the former Win32 header and raw window-state callback no longer
  stop `maingui.c`
- first remaining `maingui.c` blockers: GTK3 window-to-pixbuf capture, then
  configure-event geometry ownership
- source audit: focus transitions continue to refresh tray state; requested
  maximize/fullscreen state is observed when the GTK4 surface realizes
- source audit: GTK3 retains its event semantics and Windows taskbar behavior;
  GTK4 resolves the `HWND` only from a live Win32 `GdkSurface`
- source audit: the legacy per-window Win32 message filter is GTK3-only and is
  tracked for conversion to GTK4 display-filter ownership
- `git diff --check`: pass

Behavior contract: GTK3 top-level and tray behavior remains unchanged. GTK4
now supplies equivalent typed state transitions without exposing removed event
or window types to either consumer. Live minimize/maximize/fullscreen, focus,
auto-hide taskbar, and tray testing remains gated on the linkable full GTK4
frontend. Packaging impact: none.

### GTK4 Stage 8 Internal Drag-Icon Capture

Date: 2026-07-20

Files/workflows converted: channel-view and user-list internal drag icon
capture; native `GdkWindow` readback; GTK4 widget-paintable ownership.

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- isolated complete GTK4 frontend inventory: expected fail; improved from 261
  errors / 552 warnings to 257 errors / 552 warnings
- first remaining `maingui.c` blocker: `GdkEventConfigure` geometry ownership
- source audit: GTK3 retains native capture, ARGB conversion, scaling, and
  `GdkPixbuf` drag icons
- source audit: GTK4 internal drag begin uses a live `GtkWidgetPaintable` and
  never invokes the legacy capture callback
- `git diff --check`: pass

Behavior contract: GTK3 keeps the existing scaled snapshot drag icon. GTK4
keeps the already-converted live widget drag icon and has no native surface
readback. Manual drag-icon appearance and scale testing remains gated on the
linkable full GTK4 frontend. Packaging impact: none.

### GTK4 Stage 8 Main-Window Geometry Observer

Date: 2026-07-20

Files/workflows converted: main-window and detached-dialog dimension
persistence; available GTK3 coordinate persistence; resize-triggered relayout;
GTK4 surface-layout attachment and teardown.

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- real GTK4 window presentation, positive surface-layout callback, readable
  application-pixel dimensions, absent coordinates, and safe teardown: pass
- isolated complete GTK4 frontend inventory: expected fail; improved from 257
  errors / 552 warnings to 248 errors / 550 warnings
- first remaining `maingui.c` blocker: retired `GTK_ICON_SIZE_DIALOG` use
- source audit: `maingui.c` contains no `GdkEventConfigure`, direct
  `configure-event` connection, or direct window-size read
- source audit: DCC and Server List configure callbacks remain explicitly
  tracked for their separate workflow conversions
- `git diff --check`: pass

Behavior contract: GTK3 continues saving main and dialog position and size.
GTK4 saves valid main/dialog dimensions and queues the same relayout work, but
does not overwrite saved coordinates it cannot observe. Manual resize,
maximize/fullscreen exclusion, restart persistence, and detached-dialog testing
remain gated on the linkable full GTK4 frontend. Packaging impact: none.

### GTK4 Stage 8 Quit-Dialog Icon Sizing

Date: 2026-07-20

Files/workflows converted: quit confirmation warning-image construction;
cross-version dialog icon-size presentation; strict GTK4 image contract.

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- real GTK4 image construction: `dialog-warning` identity and explicit
  48-pixel presentation size pass
- isolated complete GTK4 frontend inventory: expected fail; improved from 248
  errors / 550 warnings to 245 errors / 549 warnings
- first remaining `maingui.c` errors: missing explicit Server List type and
  function dependencies in the retained GTK4 tab-menu path
- source audit: `maingui.c` contains no `GTK_ICON_SIZE_DIALOG` reference
- `git diff --check`: pass

Behavior contract: GTK3 keeps its theme-defined dialog warning size. GTK4 uses
a stable 48-pixel warning image without depending on removed icon-size enums.
Manual quit-dialog appearance and high-DPI testing remain gated on the linkable
full GTK4 frontend. Packaging impact: none.

### GTK4 Stage 8 Tab-Menu Server List Dependencies

Date: 2026-07-20

Files/workflows converted: retained tab-menu Autojoin and Auto-Connect compile
dependencies; Server List model, persistence, and frontend edit declarations.

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- isolated complete GTK4 frontend inventory: expected fail; improved from 245
  errors / 549 warnings to 239 errors / 546 warnings
- all six former `ircnet` parse errors and three dependent warnings are absent
- first remaining `maingui.c` blocker: retired `GTK_RELIEF_NONE` presentation
  enum near line 3325
- `git diff --check`: pass

Behavior contract: Autojoin and Auto-Connect dispatch and persistence remain
unchanged. The retained GTK4 code now names its actual common and frontend
Server List dependencies instead of relying on GTK3 include side effects.
Packaging impact: none.

### GTK4 Stage 8 Flat-Button Presentation

Date: 2026-07-20

Files/workflows converted: channel-mode toggles; emoji choices; search
close/previous/next controls; reply cancellation; nickname command button.

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- real GTK4 button construction and standard `flat` CSS class assertion: pass
- isolated complete GTK4 frontend inventory: expected fail; improved from 239
  errors / 546 warnings to 233 errors / 545 warnings
- source audit: `maingui.c` contains no `gtk_button_set_relief` or
  `GTK_RELIEF_NONE` reference
- first remaining `maingui.c` blocker: legacy `GDK_MOD1_MASK` name near line
  3426
- `git diff --check`: pass

Behavior contract: all seven controls remain visually flat. GTK3 retains its
existing relief setting; GTK4 uses the toolkit's semantic class so desktop and
profile themes remain authoritative. Manual light/dark, high-contrast, hover,
active, and focus appearance testing remains gated on the linkable full GTK4
frontend. Packaging impact: none.

### GTK4 Stage 8 Alt-Modifier Normalization

Date: 2026-07-20

Files/workflows converted: configurable key filtering and legacy key loading;
menu shortcut dispatch; Ctrl+A selection guard; user-list type-to-input guard.

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- isolated complete GTK4 frontend inventory: expected fail; improved from 233
  errors / 545 warnings to 228 errors / 545 warnings
- source audit: all active consumers use `STATE_ALT`; `GDK_MOD1_MASK` remains
  only in the GTK3 branch of that definition
- first remaining `maingui.c` blocker: retired `GTK_SHADOW_NONE` presentation
  near line 3821
- `git diff --check`: pass

Behavior contract: Alt combinations retain the same modifier bit, configurable
key comparison, menu handling, and key-file representation. GTK4 uses its
current API name and GTK3 retains its established name. Manual Alt shortcuts,
Ctrl+A, Ctrl+Q, and type-to-input testing remains gated on the linkable full
GTK4 frontend. Packaging impact: none.

### GTK4 Stage 8 Frame Presentation

Date: 2026-07-20

Files/workflows converted: topic scroller border suppression; transcript
scroller frame; lag and throttle meter info-frame outlines.

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- real GTK4 scrolled-window `frame` class enable/disable assertions: pass
- isolated complete GTK4 frontend inventory: expected fail; improved from 228
  errors / 545 warnings to 225 errors / 543 warnings
- source audit: `maingui.c` contains no `gtk_scrolled_window_set_shadow_type`,
  `gtk_frame_set_shadow_type`, or `GTK_SHADOW_*` reference
- first remaining `maingui.c` blocker: legacy `GDK_BUTTON_PRESS_MASK` near line
  4153
- `git diff --check`: pass

Behavior contract: the topic remains unframed, the transcript remains framed,
and meter labels remain outlined. GTK4 delegates final colours and border shape
to the active desktop/profile theme. Manual theme, high-contrast, and scale
testing remains gated on the linkable full GTK4 frontend. Packaging impact:
none.

### GTK4 Stage 8 Scroll-To-Bottom Control

Date: 2026-07-20

Files/workflows converted: transcript overlay scroll-to-bottom presentation,
activation, icon ownership, tooltip, and accessible naming.

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- real GTK4 icon-button construction and `go-bottom-symbolic` child identity:
  pass
- explicit GTK4 accessible-label update: pass without runtime diagnostics
- isolated complete GTK4 frontend inventory: expected fail; improved from 225
  errors / 543 warnings to 224 errors / 541 warnings
- source audit: the control contains no drawing area, custom Cairo draw
  callback, app-paintable flag, raw event mask, or direct style-context call
- first remaining `maingui.c` blockers: retired `GTK_ICON_SIZE_MENU` uses near
  lines 5324, 5601, 5622, and 5629
- `git diff --check`: pass

Behavior contract: the control retains its preference, visibility threshold,
overlay position, compact footprint, tooltip, and adjustment-to-bottom action.
Its arrow now follows the active icon theme and exposes button semantics plus an
accessible label. Manual activation, keyboard, screen-reader, theme, and scale
testing remains gated on the linkable full GTK4 frontend. Packaging impact:
none.

### GTK4 Stage 8 Shared Icon Sizes

Date: 2026-07-20

Files/workflows converted: shared icon resolver and image utility; menu and
toolbar sizing; search controls; channel tab/list icons; Join, spell, plugin,
menu, and pixmap consumers.

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- real GTK4 named-image assertions at 16-pixel menu and 24-pixel large-toolbar
  sizes: pass
- isolated complete GTK4 frontend inventory: expected fail; improved from 224
  errors / 541 warnings to 212 errors / 537 warnings
- source audit: active callers contain no `GtkIconSize`, `GTK_ICON_SIZE_*`,
  `gtk_icon_size_lookup()`, or obsolete two-argument GTK4 named-image use;
  legacy names remain only inside GTK3 compatibility branches
- source audit: `maingui.c` has no remaining compiler errors in the isolated
  complete GTK4 inventory
- next errors: independent button-box layout, channel-view shadow, DCC geometry,
  and chooser boundaries
- `git diff --check`: pass

Behavior contract: custom/bundled/system icon resolution order is unchanged.
GTK3 named images retain native theme roles; resolved pixbufs and GTK4 images
receive deterministic logical sizes while the active theme still supplies
named system images. Manual icon clarity, scale, light/dark, high-contrast, and
missing-theme fallback testing remains gated on the linkable full GTK4
frontend. Packaging impact: none.

### GTK4 Stage 8 Button-Box Layout

Date: 2026-07-20

Files/workflows converted: Ban List, DCC transfers and chats, generic editor,
key bindings, Ignore List, Notify List, Add-ons, Raw Log, Server List,
Preferences actions, Print Events, and URL History button groups.

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- real GTK4 horizontal spread, horizontal end, and vertical start layout,
  orientation, spacing, homogeneity, and alignment assertions: pass
- isolated complete GTK4 frontend inventory: expected fail; improved from 212
  errors / 537 warnings to 196 errors / 485 warnings
- source audit: active frontend code contains no direct `gtk_button_box_new()`
  or `gtk_button_box_set_layout()` call outside the GTK3 compatibility branch
- next errors: DCC configure-event geometry, channel-view shadow presentation,
  channel-list indexing, and file-chooser boundaries
- `git diff --check`: pass

Behavior contract: button order, orientation, spacing, and start/end/spread
intent are unchanged. GTK3 retains its native button-box behavior; GTK4 uses
ordinary boxes and toolkit-supported allocation/alignment. Manual dialog width,
long-label, keyboard order, theme, and scale testing remains gated on the
linkable full GTK4 frontend. Packaging impact: none.

### GTK4 Stage 8 DCC Geometry

Date: 2026-07-20

Files/workflows converted: detached DCC transfer window resize observation and
remembered reopen dimensions.

Automated evidence:

- shipping GTK3 MSVC x64 Release frontend: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- shared real-window geometry probe: positive surface-layout dimensions and
  automatic observer cleanup pass
- isolated complete GTK4 frontend inventory: expected fail; improved from 196
  errors / 485 warnings to 188 errors / 484 warnings
- source audit: `dccgui.c` contains no raw configure-event connection,
  `GdkEventConfigure`, or direct `gtk_window_get_size()` call
- next errors: channel-view shadow presentation, channel-list indexing, Join
  window type hints, and retained menu ownership
- `git diff --check`: pass

Behavior contract: detached DCC transfers retain their latest positive size
for the next reopen. Tabbed utilities still do not overwrite that detached
size. Manual detached resize/reopen and tabbed-mode switching remain gated on
the linkable full GTK4 frontend. Packaging impact: none.

### GTK4 Stage 8 Channel-View Frame Presentation

Date: 2026-07-20

Files/workflows converted: scrollable horizontal/vertical tab strips; framed
tree channel switcher; deprecated resolved-pixbuf size lookup cleanup.

Automated evidence:

- full shipping GTK3 MSVC x64 Release rebuild: pass; zero warnings and zero
  errors, including explicit `chanview.c` recompilation
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: full compile, link,
  and execution pass under `/W4 /WX`; zero warnings and zero errors
- real GTK4 scrolled-window `frame` class enable/disable assertions: pass
- isolated complete GTK4 frontend inventory: expected fail; improved from 188
  errors / 484 warnings to 186 errors / 483 warnings
- source audit: `chanview-tabs.c` and `chanview-tree.c` contain no direct
  shadow-type call or `GTK_SHADOW_*` value
- source audit: active code contains no `gtk_icon_size_lookup()` call
- next errors: Join window type hints, chooser ownership, channel-list indexing,
  and retained menu ownership
- `git diff --check`: pass

Behavior contract: tab strips remain unframed and the tree switcher remains
visually framed. Named GTK3 images retain native icon roles; resolved pixbufs
use the established semantic pixel values. Manual tab/tree switching, themes,
high contrast, and scale testing remain gated on the linkable full GTK4
frontend. Packaging impact: none.

### GTK4 Stage 8 Dialog-Window Hints

Date: 2026-07-20

Files/workflows converted: Join dialog; shared transient utility windows;
Server Editor; Network List dialog-window classification.

Automated evidence:

- full shipping GTK3 MSVC x64 Release rebuild: pass; zero warnings and zero
  errors
- full strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link,
  and execution pass under `/W4 /WX`; zero warnings and zero errors
- compatibility helper signature and GTK4 compilation without the retired GDK
  enum: pass
- isolated complete GTK4 frontend inventory: expected fail; improved from 186
  errors / 483 warnings to 182 errors / 480 warnings
- source audit: active callers contain no direct `gtk_window_set_type_hint()`
  or `GDK_WINDOW_TYPE_HINT_DIALOG`; both remain GTK3-private in the helper
- next errors: channel-list indexing, chooser ownership, shared top-level
  construction, and retained menu ownership
- `git diff --check`: pass

Behavior contract: Join remains a modal transient dialog; shared utility and
Server List windows retain their existing parent, modal, role, and destruction
relationships. GTK3 additionally keeps its window-manager dialog hint. Manual
parent stacking, focus, modality, and close behavior remain gated on the
linkable full GTK4 frontend. Packaging impact: none.

### GTK4 Stage 8 Channel List Entry Text

Date: 2026-07-20

Files/workflows converted: Channel List empty-search detection; glob search;
plain case-insensitive search; regex compilation input.

Automated evidence:

- full shipping GTK3 MSVC x64 Release rebuild: pass; zero warnings and zero
  errors
- full strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link,
  and execution pass under `/W4 /WX`; zero warnings and zero errors
- real GTK4 entry set/read assertion through the compatibility helper: pass
- isolated complete GTK4 frontend inventory: expected fail; improved from 182
  errors / 480 warnings to 181 errors / 474 warnings
- source audit: `chanlist.c` contains no direct `gtk_entry_get_text()` call
- next errors: shared file-chooser folder ownership, shared top-level window
  construction, and retained menu ownership
- `git diff --check`: pass

Behavior contract: search pattern contents and widget-owned lifetime remain
unchanged across empty, glob, plain-text, and regex modes. Manual incremental
search, Search button, refresh, and large-list filtering remain gated on the
linkable full GTK4 frontend. Packaging impact: none.

### GTK4 Stage 8 Shared File-Chooser Paths

Date: 2026-07-20

Files/workflows converted: shared open/save/folder requests; Server List import;
Preferences sound-file selection; colors.conf and legacy GTK3-theme import.

Automated evidence:

- full shipping GTK3 MSVC x64 Release rebuild: pass; zero warnings and zero
  errors
- full strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link,
  and execution pass under `/W4 /WX`; zero warnings and zero errors
- function-signature and source-owner compilation coverage for single-file,
  multiple-file, current-folder, and initial-folder adapters: pass
- Meson source path: all 58 objects, including `file-chooser-path.c`, compile;
  local link remains unavailable because ambient Strawberry GCC 4.8 cannot
  link the MSVC-built GTK4 import libraries
- isolated complete GTK4 frontend inventory: expected fail; improved from 181
  errors / 474 warnings to 174 errors / 392 warnings
- source audit: production callers contain no direct
  `gtk_file_chooser_get_filename()`, `gtk_file_chooser_get_filenames()`,
  `gtk_file_chooser_get_current_folder()`, or path-based
  `gtk_file_chooser_set_current_folder()` call
- GTK 4.10 `GtkFileChooser` deprecation is isolated to
  `file-chooser-path.c`; the general compatibility header and callers have no
  suppression
- next errors: shared top-level window construction and retained menu ownership
- `git diff --check`: pass

Behavior contract: chooser callbacks continue receiving owned local paths;
multi-selection order, folder validation, suggested names, filters, response
handling, and cleanup remain unchanged. Non-local `GFile` selections are not
passed to path-only callbacks. Manual open/save/folder selection and import
workflows remain gated on the linkable full GTK4 frontend. Packaging impact:
none.

### GTK4 Stage 8 Top-Level Window Constructor

Date: 2026-07-20

Files/workflows converted: shared utility windows; Server Editor; Server List.

Automated evidence:

- full shipping GTK3 MSVC x64 Release rebuild: pass; zero warnings and zero
  errors
- full strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link,
  and execution pass under `/W4 /WX`; zero warnings and zero errors
- helper signature and real GTK4 window construction through the shared
  geometry probe: pass
- isolated complete GTK4 frontend inventory: expected fail; improved from 174
  errors / 392 warnings to 171 errors / 389 warnings
- source audit: active frontend callers contain no direct
  `gtk_window_new(GTK_WINDOW_TOPLEVEL)`; two remaining literals are confined to
  GTK3-only theme tests
- next errors: retained `menu.c` GTK3 item types, callbacks, and constructors
- `git diff --check`: pass

Behavior contract: utility, Server Editor, and Server List windows retain their
existing title, role, default size, theme attachment, transient parent,
modality, pointer placement, and parent-destruction behavior. Manual open,
close, focus, stacking, and repeated Server List/Editor lifecycle validation
remain gated on the linkable full GTK4 frontend. Packaging impact: none.

### GTK4 Stage 8 Legacy Popup Builder Containment

Date: 2026-07-20

Files/workflows converted: generic GTK3 check/quick menu construction; nested
submenu construction; configured popup-list expansion; legacy popup cleanup.

Automated evidence:

- full shipping GTK3 MSVC x64 Release rebuild: pass; zero warnings and zero
  errors
- full strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link,
  and execution pass under `/W4 /WX`; zero warnings and zero errors
- isolated complete GTK4 frontend inventory: expected fail; subsequent clean
  `/t:Rebuild` rebaseline is 169 errors / 435 warnings; the earlier 169 / 372
  incremental count is superseded
- source audit: the generic menu-widget builder, submenu ownership list, popup
  destroy/presentation owner, and public declarations are GTK3-private
- shared `menu_parse_icon_label()` and executable path filtering remain
  available to GTK4 model projection
- next errors: legacy Away check-menu callback and GTK3 `/MENU` widget mutation
- `git diff --check`: pass

Behavior contract: GTK3 configured popup labels, icons, toggles, separators,
submenus, commands, path filtering, sensitivity, ordering, popup placement, and
cleanup are unchanged. GTK4 continues using the previously validated typed
context models and retained popover presenter. Packaging impact: none.

### GTK4 Stage 8 Away Check-Item Callback

Date: 2026-07-20

Files/workflows converted: Away/Back action dispatch and GTK3 menu-state
synchronization boundary.

Automated evidence:

- full shipping GTK3 MSVC x64 Release rebuild: pass; zero warnings and zero
  errors
- full strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link,
  and execution pass under `/W4 /WX`; zero warnings and zero errors
- clean isolated complete GTK4 frontend comparison: expected fail; moves from
  169 errors / 435 warnings to 171 errors / 438 warnings because six invalid
  callback parse errors are removed and eight downstream `/MENU` errors become
  visible
- source audit: `GtkCheckMenuItem` and its active-state getter in `menu_away()`
  are GTK3-private; GTK4 uses `MENU_ACTION_AWAY_TOGGLE`
- next errors: GTK3 `/MENU` item lookup, mutation callbacks, and constructors
- `git diff --check`: pass

Behavior contract: GTK4 and GTK3 continue selecting `away` or `back` from the
current server state through the shared action path. GTK3 retains its existing
check-item synchronization fallback without emitting commands during blocked
programmatic state changes. Packaging impact: none.

### GTK4 Stage 8 `/MENU` Widget-Mutation Containment

Date: 2026-07-20

Files/workflows converted: dynamic `/MENU` item lookup, add/delete/update,
radio and toggle state changes, ordering, accelerators, and contextual popup
injection.

Automated evidence:

- full shipping GTK3 MSVC x64 Release rebuild: pass; zero warnings and zero
  errors
- full strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link,
  and execution pass under `/W4 /WX`; zero warnings and zero errors
- clean isolated complete GTK4 frontend comparison: expected fail; improves
  from 171 errors / 438 warnings to 118 errors / 405 warnings
- source audit: GTK3 owns menu-widget lookup and mutation; GTK4 owns retained
  plugin trees, copied action data, action groups, and `GMenuModel` projection
- source audit: core add, delete, and update paths call `fe_menu_sync()` after
  mutation, rebuilding live GTK4 main and contextual menu projections
- no `menu.c` errors remain in the complete GTK4 frontend inventory
- next errors: retired icon lookup sizing flag in `pixmaps.c`, then legacy tray
  window-state ownership
- `git diff --check`: pass

Behavior contract: GTK3 `/MENU` labels, markup normalization, icons, commands,
radio/toggle state, sensitivity, ordering, accelerators, and popup placement are
unchanged. GTK4 add, delete, update, dispatch, and synchronization continue
through the previously validated retained menu models without GTK3 widget-tree
mutation. Packaging impact: none.

### GTK4 Stage 8 System-Icon Pixbuf Boundary

Date: 2026-07-20

Files/workflows converted: system-theme fallback loading for user-list, tray,
channel-tree, book, and application pixbuf roles.

Automated evidence:

- full shipping GTK3 MSVC x64 Release rebuild: pass; zero warnings and zero
  errors after clearing a stale MSVC program-database service lock
- full strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link,
  and execution pass under `/W4 /WX`; zero warnings and zero errors
- clean isolated complete GTK4 frontend comparison: expected fail; improves
  from 118 errors / 405 warnings to 117 errors / 401 warnings
- changed `pixmaps.c` compiles in the complete GTK4 frontend inventory with no
  diagnostics
- source audit: GTK4 paintable, borrowed file, owned stream, and owned pixbuf
  lifetimes are explicit; GTK3 forced-size loading remains private
- next errors: legacy tray `GdkWindow` minimized-state reads
- `git diff --check`: pass

Behavior contract: embedded resource and configured file icons still take
precedence over system-theme fallback. The fallback result remains an owned
16-pixel menu-role pixbuf before the existing `GDK_SCALE` multiplication, and
missing icons retain the existing warning path. Packaging impact: none.

### GTK4 Stage 8 Window-Surface Ownership

Date: 2026-07-20

Files/workflows converted: tray hidden/minimized decisions; plugin window
status and native pointer queries; Win32 tray popup ownership; native titlebar
styling; autohide-taskbar adjustment; tray restore.

Automated evidence:

- full shipping GTK3 MSVC x64 Release rebuild: pass; zero warnings and zero
  errors after clearing the recurring stale MSVC program-database service lock
- full strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link,
  and execution pass under `/W4 /WX`; zero warnings and zero errors
- clean isolated complete GTK4 frontend comparison: expected fail; moves from
  117 errors / 401 warnings to 106 errors / 409 warnings as 11 direct surface
  errors are removed and eight downstream warnings become visible
- source audit: all converted GTK3 `GdkWindow` state and Win32 handle access is
  private to `window-state.c`; GTK4 uses `GdkToplevel` and `GdkWin32Surface`
- next errors: Raw Log inset framing, then Server List lifecycle callbacks
- `git diff --check`: pass

Behavior contract: invisible or minimized windows remain hidden to tray and
plugin status consumers; focused and normal results are unchanged. Win32 tray
menus, plugin native pointers, dark titlebars, and taskbar adjustment still use
the realized main-window handle. GTK3 restore still deiconifies explicitly;
GTK4 uses show and present. Packaging impact: none.

### GTK4 Stage 8 Raw Log Framing

Date: 2026-07-20

Files/workflows converted: Raw Log scroller construction and inset-frame
presentation.

Automated evidence:

- full shipping GTK3 MSVC x64 Release rebuild: pass; zero warnings and zero
  errors after proactively clearing the stale MSVC program-database service
- full strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link,
  and execution pass under `/W4 /WX`; zero warnings and zero errors
- clean isolated complete GTK4 frontend comparison: expected fail; improves
  from 106 errors / 409 warnings to 105 errors / 407 warnings
- changed `rawlog.c` compiles in the complete GTK4 frontend inventory with no
  diagnostics
- source audit: GTK4 constructor and CSS frame semantics are active; GTK3 null
  adjustments and inset shadow remain private to the compatibility helper
- next errors: Server List lifecycle and geometry callbacks
- `git diff --check`: pass

Behavior contract: Raw Log scroll policies, expansion, transcript widget,
theme-derived colours, selection copying, clear/save actions, and close
lifecycle are unchanged. GTK3 retains the inset appearance and GTK4 uses the
standard framed-scroller presentation. Packaging impact: none.

### GTK4 Stage 8 Server List Lifecycle and Geometry

Date: 2026-07-20

Files/workflows converted: main Server List and network-editor close handling,
size persistence, and close-time entry text reads.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- clean isolated complete GTK4 frontend comparison: expected fail; improves
  from 105 errors / 407 warnings to 70 errors / 384 warnings
- source audit: no active Server List callback exposes `GdkEventAny` or
  `GdkEventConfigure` to GTK4; both windows use the shared geometry observer
- source audit: editor close still saves pending network fields before explicit
  destruction; main-list close still saves global configuration and preserves
  startup-exit behavior
- source audit: Server List entry reads use the borrowed GTK3/GTK4 text helper
- next errors: Server List editor and main-list framed-scroller construction
- `git diff --check`: pass

Behavior contract: pending network edits, password handling, global identity
validation, configuration persistence, startup cancellation, and remembered
main/editor dimensions are unchanged. GTK3 retains private delete-event
handling; GTK4 uses close-request without duplicate editor destruction.
Packaging impact: none.

### GTK4 Stage 8 Server List Scrollers

Date: 2026-07-20

Files/workflows converted: three network-editor list scrollers and the main
network-list scroller.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- clean isolated complete GTK4 frontend comparison: expected fail; improves
  from 70 errors / 384 warnings to 66 errors / 379 warnings
- changed `servlistgui.c` has no hard errors in the complete GTK4 inventory
- source audit: no direct scrolled-window constructor, shadow setter, or
  `GTK_SHADOW_IN` reference remains in `servlistgui.c`
- source audit: typed server-entry and network-list owners still attach their
  views through `fabulor_gtk_scrolled_window_set_child()`
- next errors: Preferences framed-scroller state, then user-list framing
- `git diff --check`: pass

Behavior contract: Servers, Autojoin channels, Connect commands, and Networks
retain their existing scroll policies, typed list ownership, selection,
editing, notebook placement, and command tooltip. GTK3 retains inset shadows;
GTK4 uses the standard frame CSS presentation. Packaging impact: none.

### GTK4 Stage 8 Preferences Framing

Date: 2026-07-20

Files/workflows converted: Preferences page scroller construction, frame
presentation, and page-content ownership.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- clean isolated complete GTK4 frontend comparison: expected fail; improves
  from 66 errors / 379 warnings to 64 errors / 373 warnings
- changed `setup.c` has no hard errors in the complete GTK4 inventory
- source audit: GTK4 Preferences code uses the shared constructor, frame, and
  child APIs without `GtkBin`, `GtkViewport`, or shadow enums
- source audit: GTK3 privately clears its auto-created viewport shadow after
  attaching non-scrollable page content
- next errors: user-list framing, then the theme API boundary
- `git diff --check`: pass

Behavior contract: Preferences labels, page order, vertical scrolling, lazy
page creation, notebook ownership, and settings controls are unchanged. GTK3
retains one inset border without a nested viewport shadow; GTK4 uses the
standard scroller frame CSS class. Packaging impact: none.

### GTK4 Stage 8 User-List Framing

Date: 2026-07-20

Files/workflows converted: main user-list scroller construction and frame
presentation.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- clean isolated complete GTK4 frontend comparison: expected fail; improves
  from 64 errors / 373 warnings to 63 errors / 371 warnings
- changed `userlistgui.c` has no hard errors in the complete GTK4 inventory
- source audit: no direct scrolled-window constructor, shadow setter, or
  `GTK_SHADOW_IN` reference remains in `userlistgui.c`
- source audit: the typed user-list view still attaches through the explicit
  cross-version scroller child owner
- error grouping: all 63 remaining hard frontend errors are under `theme/`
- next errors: theme manager display/provider ownership, theme application and
  CSS provider ownership, style access, then the private GTK3 adapter
- `git diff --check`: pass

Behavior contract: user-list expansion, scroll policies, minimum width, model
identity, selection, drag/drop, file drops, pointer activation, and keyboard
forwarding are unchanged. GTK3 retains its inset shadow; GTK4 uses the standard
frame CSS class. Packaging impact: none.

### GTK4 Stage 8 Theme Window Ownership

Date: 2026-07-20

Files/workflows converted: top-level dark/light class presentation and the
GTK3 KDE/Wayland client-side decoration workaround boundary.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- clean isolated complete GTK4 frontend comparison: expected fail; improves
  from 63 errors / 371 warnings to 53 errors / 365 warnings
- changed `theme-manager.c` has no errors or warnings in the complete GTK4
  inventory
- source audit: `GdkScreen`, legacy header-bar sizing/title APIs, and widget
  style reset are confined to the GTK3 KDE/Wayland CSD branch
- source audit: GTK4 applies exactly one existing top-level dark/light class
  through widget CSS ownership and leaves CSD to the compositor
- next errors: theme application and CSS provider display ownership, style
  access, then the private GTK3 adapter
- `git diff --check`: pass

Behavior contract: resolved dark/light mode, top-level CSS selectors, Windows
native-titlebar updates, GTK3 KDE/Wayland decorations, attached-window
lifetime, and theme-change dispatch are unchanged. GTK4 deliberately does not
recreate a GTK3-only KDE decoration workaround. Packaging impact: none.

### GTK4 Stage 8 Theme Provider Ownership

Date: 2026-07-20

Files/workflows converted: application-wide CSS provider installation,
removal, priority selection, and CSS string loading.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- clean isolated complete GTK4 frontend comparison: expected fail; improves
  from 53 errors / 365 warnings to 38 errors / 354 warnings
- changed `theme-application.c` and `theme-css.c` have no errors or warnings in
  the complete GTK4 inventory
- source audit: application providers resolve exactly one default screen or
  display and use the matching scoped add/remove API
- source audit: top-level CSS retains application priority plus one; input and
  palette CSS retain user priority and existing provider identity
- source audit: CSS strings use one guarded loader rather than version-specific
  call signatures at each producer
- strict probe: GTK4 theme-controller provider application, variant changes,
  diagnostics, and removal remain covered
- next errors: theme style access, then the private GTK3 adapter
- `git diff --check`: pass

Behavior contract: dark-mode preference, top-level selector generation, input
style fingerprints, palette providers, provider priority, retry behavior,
removal, and object lifetime are unchanged. GTK4 scopes application providers
to the display; GTK3 retains screen scope. Packaging impact: none.

### GTK4 Stage 8 Theme Style Access

Date: 2026-07-20

Files/workflows converted: widget style-context palette sampling and GTK4
semantic runtime fallback routing.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- dedicated Meson Theme Access Routing Test: not run; WSL does not provide
  `gtk+-3.0` development metadata
- clean isolated complete GTK4 frontend comparison: expected fail; improves
  from 38 errors / 354 warnings to 37 errors / 351 warnings
- changed `theme-access.c` has no errors or warnings in the complete GTK4
  inventory
- source audit: GTK3 custom-theme palette sampling retains normal, selected,
  link, foreground, and background state queries
- source audit: GTK4 cannot enter the GTK3 sampler and resolves widget/transcript
  values through `theme_runtime_get_widget_style_values()`
- error grouping: all 37 remaining hard frontend errors are in `theme-gtk3.c`
- next errors: private GTK3 adapter containment
- `git diff --check`: pass

Behavior contract: semantic colors, default IRC colors, user overrides,
dark/light palette selection, transcript marker colors, and RGB16 conversion
are unchanged. GTK3 custom themes retain style-derived palette mapping; GTK4
uses the semantic runtime palette. Packaging impact: none.

### GTK4 Stage 8 GTK3 Theme Adapter Containment

Date: 2026-07-21

Files/workflows converted: production GTK3 theme-adapter compilation boundary
and its inert GTK4 compatibility contract.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- fresh Meson MSVC GTK4 probe: configure, compile, link, and execution pass;
  production `theme-gtk3.c` is compiled and its containment check executes
- clean isolated complete GTK4 frontend comparison: expected fail; improves
  from 37 ordinary errors / 351 warnings to zero ordinary errors / 336 warnings
- complete GTK4 frontend does not reach link: compilation stops at
  `xtext.c(65)` because GTK4 does not provide `gdk/gdkwin32.h`
- source audit: GTK3 retains the complete legacy adapter implementation
- source audit: GTK4 setup/apply/refresh calls are harmless, the adapter never
  becomes active, and variant probing returns the established light default
- next error: Xtext Win32 GDK surface/header containment
- `git diff --check`: pass

Behavior contract: GTK3 theme discovery, CSS loading, provider lifecycle,
settings monitoring, variant resolution, and refresh behavior are unchanged.
GTK4 cannot load or activate the GTK3 adapter. Packaging impact: none.

### GTK4 Stage 8 Xtext Win32 Header Containment

Date: 2026-07-21

Files/workflows converted: transcript platform-header ownership and complete
GTK4 frontend compilation through the first link inventory.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 regression probe against GTK 4.22.4 / GLib 2.88.0: compile,
  link, and execution pass under `/W4 /WX`; zero warnings and zero errors
- clean isolated complete GTK4 frontend compilation: all production sources
  compile; zero C compiler errors and 336 warnings
- complete GTK4 frontend reaches link for the first time; expected link failure
  reports 89 unique unresolved symbols over 238 diagnostic lines and ends with
  `LNK1120`
- changed `xtext.c` has no errors or warnings in the complete GTK4 inventory
- source audit: no native GDK Win32 handle or surface API is used by Xtext
- source audit: Windows transcript export retains `io.h`; flag lookup retains
  `glib/gwin32.h`; GDK/Cairo uses the public cross-platform header
- next target: production GTK4 link-input closure, beginning with
  `gtk4-list-models.c`
- `git diff --check`: pass

Behavior contract: transcript rendering, selection, scrolling, export, flag
lookup, and background rendering are unchanged. This is compile-time header
containment only. Packaging impact: none.

### GTK4 Stage 8 List-Model Link Input

Date: 2026-07-21

Files/workflows converted: production GTK4 MSVC source ownership for the shared
flat and tree list-model stacks.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  336 warnings; every production source reaches link
- expected complete GTK4 link failure improves from 89 to 76 unique unresolved
  symbols and from 238 to 166 repeated unresolved-symbol diagnostics
- all 13 `fabulor_gtk4_flat_model_stack_*` and
  `fabulor_gtk4_tree_model_stack_*` symbols are resolved
- project-condition audit: `gtk4-list-models.c` is included only when
  `FabulorGtkMajor` is `4`; the GTK3 shipping source set is unchanged
- next target: remaining legacy GTK3 compatibility-call link surface
- `git diff --check`: pass

Behavior contract: converted list construction, sorting, selection, mutation,
and cleanup are unchanged and remain covered by the strict probe. Shipping GTK3
does not consume this GTK4-only implementation. Packaging impact: none.

### GTK4 Stage 8 Entry Compatibility Link Closure

Date: 2026-07-21

Files/workflows converted: entry text reads, text replacement, width requests,
and spell-entry text macros.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- Meson MSVC GTK4 probe: regenerate, compile, link, and execution pass
- strict runtime check: helper text replacement is returned by the typed reader
  and the width request is visible through `GtkEditable`
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  278 warnings, improved from 336 warnings
- expected complete GTK4 link failure improves from 76 to 73 unique unresolved
  symbols and from 166 to 144 repeated unresolved-symbol diagnostics
- `gtk_entry_get_text`, `gtk_entry_set_text`, and
  `gtk_entry_set_width_chars` no longer appear in the GTK4 link inventory
- next target: legacy GTK3 container/child ownership link surface
- `git diff --check`: pass

Behavior contract: entry contents, borrowed-text lifetime, cursor behavior,
configured character widths, spell-entry operations, and persistence are
unchanged. GTK3 retains native entry APIs; GTK4 uses editable ownership.
Packaging impact: none.

### GTK4 Stage 8 Container Inset Link Closure

Date: 2026-07-21

Files/workflows converted: uniform container content spacing and deferred
top-level window-child inset application.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- Meson MSVC GTK4 probe: compile, link, and execution pass
- strict runtime checks: ordinary widgets receive four equal margins; a window
  configured before child attachment applies its stored inset to that child;
  setting zero clears all four margins
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  255 warnings, improved from 278 warnings
- expected complete GTK4 link failure improves from 73 to 72 unique unresolved
  symbols and from 144 to 121 repeated unresolved-symbol diagnostics
- `gtk_container_set_border_width` no longer appears in the GTK4 link inventory
- next target: typed child attachment and removal link closure
- `git diff --check`: pass

Behavior contract: all configured inset values and GTK3 container spacing are
unchanged. GTK4 maps the same values to content margins and preserves requests
made before top-level child ownership is established. Packaging impact: none.

### GTK4 Stage 8 Typed Box-Child Attachment

Date: 2026-07-21

Files/workflows converted: ordinary box and button-box children in the ASCII
palette, shared label/entry utility, plugin manager, preferences radio group,
Server List controls, theme color page, channel tree, and channel tabs; channel
tab viewport child ownership.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  245 warnings, improved from 255 warnings
- expected complete GTK4 link failure retains 72 unique unresolved symbols and
  improves from 121 to 111 repeated unresolved-symbol diagnostics
- 14 direct `gtk_container_add` calls remain in production and test sources;
  they are event-surface, list-row/list-box, legacy menu, lazy-page, spell-menu,
  and test-window ownership rather than ordinary box attachment
- next target: typed event-surface and list ownership, followed by child removal
- `git diff --check`: pass

Behavior contract: GTK3 box placement remains non-expanding and filling, and
the channel-tab viewport remains its child's sole owner. No event, menu, list,
lazy-page, or reparenting behavior is changed. Packaging impact: none.

### GTK4 Stage 8 Content-Surface And List Ownership

Date: 2026-07-21

Files/workflows converted: lag/throttle meter wrappers, theme preview and color
surfaces, and theme color-manager row/list attachment.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- Meson MSVC GTK4 probe: compile, link, and execution pass
- strict runtime checks: transparent and visible content surfaces own their
  children; a list row owns its content; the list owns and orders the row
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  233 warnings, improved from 245 warnings
- expected complete GTK4 link failure improves from 72 to 69 unique unresolved
  symbols and from 111 to 104 repeated unresolved-symbol diagnostics
- `gtk_event_box_new`, `gtk_event_box_set_visible_window`, and `GTK_EVENT_BOX`
  no longer appear in the GTK4 link inventory
- remaining active generic attachment diagnostics are channel-list menu
  construction and lazy Preferences page attachment
- next target: typed lazy-page attachment, then child reparent/removal
- `git diff --check`: pass

Behavior contract: GTK3 visible and transparent event-box behavior is retained;
GTK4 surfaces remain CSS-palette targets and own one content tree. Theme list
rows retain their order and non-selectable policy. Packaging impact: none.

### GTK4 Stage 8 Lazy Preferences Page Ownership

Date: 2026-07-21

Files/workflows converted: one-time Preferences page creation, attachment, and
reveal.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  228 warnings, improved from 233 warnings
- expected complete GTK4 link failure retains 69 unique unresolved symbols and
  improves from 104 to 100 repeated unresolved-symbol diagnostics
- `setup.c` has no direct `gtk_container_get_children`, `gtk_container_add`, or
  `gtk_widget_show_all` calls
- next target: typed child reparenting and removal
- `git diff --check`: pass

Behavior contract: Preferences pages remain lazy and each page factory runs at
most once per Preferences window registry. Page order, scrolling, visibility,
and factory selection are unchanged. Packaging impact: none.

### GTK4 Stage 8 Layout Reparent Ownership

Date: 2026-07-21

Files/workflows converted: initial pane hierarchy construction and channel-view
or user-list movement between pane and grid positions.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- Meson MSVC GTK4 probe: compile, link, and execution pass
- strict runtime checks: pane and grid children survive typed detachment, have
  no parent while retained, reattach to the requested owner, and release their
  temporary reference; an unparented child reports no detach
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  225 warnings, improved from 228 warnings
- expected complete GTK4 link failure improves from 69 to 66 unique unresolved
  symbols and from 100 to 97 repeated unresolved-symbol diagnostics
- `gtk_container_remove`, `gtk_paned_pack1`, and `gtk_paned_pack2` no longer
  appear in the GTK4 link inventory
- next target: channel-list menu item construction ownership
- `git diff --check`: pass

Behavior contract: layout positions, hidden attachment, pane-divider settings,
and temporary child lifetime are unchanged. GTK3 retains its resize and shrink
flags; GTK4 uses explicit pane slots and grid removal. Packaging impact: none.

### GTK4 Stage 8 Channel List Context Menu

Date: 2026-07-21

Files/workflows converted: Channel List row right-click menu, multi-selection
Join and copy actions, first-selected-channel Autojoin, and popup lifetime.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- Meson MSVC GTK4 probe: compile, link, and execution pass
- strict model checks: labels and actions project correctly; original channel
  and topic arrays may be released before Join, both copy actions, and Autojoin
  dispatch; Autojoin state toggles and both selected channels remain owned
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  213 warnings, improved from 225 warnings
- expected complete GTK4 link failure improves from 66 to 61 unique unresolved
  symbols and from 97 to 87 repeated unresolved-symbol diagnostics
- closed symbols: `gtk_container_add`, `GTK_MENU`, `gtk_menu_item_new`,
  `gtk_menu_new`, and `gtk_menu_popup_at_pointer`
- next target: Channel List button image and window lifecycle compatibility
- `git diff --check`: pass

Behavior contract: right-click selection, multi-channel joining, newline-
separated channel/topic copying, icons, and Autojoin state are retained. GTK4
actions use an owned selection snapshot; GTK3 behavior is unchanged. Packaging
impact: one GTK4-only model source is added to frontend and probe build inputs.

### GTK4 Stage 8 Icon/Mnemonic Button And Channel List Lifecycle

Date: 2026-07-21

Files/workflows converted: Channel List and plugin-manager icon/mnemonic button
construction plus Channel List model/view construction-failure cleanup.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- Meson MSVC GTK4 probe: compile, link, and execution pass
- strict runtime checks: the button owns an image followed by one mnemonic
  label; icon identity and 16-pixel menu sizing are retained; displayed label
  text omits the mnemonic marker and targets the containing button
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  210 warnings, improved from 213 warnings
- expected complete GTK4 link failure improves from 61 to 60 unique unresolved
  symbols and from 87 to 84 repeated unresolved-symbol diagnostics
- closed symbol: `gtk_button_set_image`; both direct Channel List
  `gtk_widget_destroy` calls are removed
- inventory log: `build/gtk4-full/channel-list-button-lifecycle-final.log`
- next target: Channel View button presentation and lifecycle compatibility
- `git diff --check`: pass

Behavior contract: button labels, icons, mnemonics, click callbacks, and GTK3
presentation are unchanged. Channel List construction failures retain their
existing close behavior through a typed GTK3/GTK4 helper. Packaging impact:
none.

### GTK4 Stage 8 Channel View Ownership And Lifecycle

Date: 2026-07-21

Files/workflows converted: tab and tree scroller construction, family/tab
ordering, close-button presentation, recursive reveal, child removal,
implementation cleanup, and Channel View root finalization.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- Meson MSVC GTK4 probe: compile, link, and execution pass
- strict runtime checks: an existing box child moves to the first and final
  positions while sibling order and ownership remain intact
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  204 warnings, improved from 210 warnings
- expected complete GTK4 link failure improves from 60 to 58 unique unresolved
  symbols and from 84 to 80 repeated unresolved-symbol diagnostics
- closed symbols: `gtk_box_reorder_child` and
  `gtk_button_set_always_show_image`
- `chanview.obj` has no remaining GTK4 compiler warning, `gtk_widget_destroy`,
  or `gtk_widget_show_all` unresolved diagnostic
- inventory log: `build/gtk4-full/chanview-lifecycle-pass65-final.log`
- next target: remaining top-level visibility and lifecycle callbacks
- `git diff --check`: pass

Behavior contract: tab/tree selection, ordering, scrolling, close-button
visibility and hover, implementation switching, and automatic main-window
ownership are retained. GTK3 presentation and destruction remain unchanged;
GTK4 owns removal and finalization explicitly. Packaging impact: none.

### GTK4 Stage 8 Join Channel Dialog Lifecycle

Date: 2026-07-21

Files/workflows converted: Join Channel grouped choices, persistence checkbox,
wrapped explanatory label, invalid-server root close, default OK response, and
dialog pointer cleanup.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- Meson MSVC GTK4 probe: compile, link, and execution pass
- strict runtime checks: activating each grouped choice clears its siblings;
  ordinary check-button state is readable and writable; label wrapping is
  enabled; a parented control resolves its owning window while an unparented
  widget resolves no window
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  191 warnings, improved from 204 warnings
- expected complete GTK4 link failure improves from 58 to 56 unique unresolved
  symbols and from 80 to 72 repeated unresolved-symbol diagnostics
- closed symbols: `gtk_radio_button_set_group` and
  `gtk_label_set_line_wrap`
- `joind.obj` has no remaining GTK4 compiler warning or unresolved diagnostic
- inventory log: `build/gtk4-full/join-dialog-lifecycle-pass66.log`
- next target: DCC grouped choice controls
- `git diff --check`: pass

Behavior contract: Nothing, Join this channel, and Open channel list remain
mutually exclusive; entry focus selects Join; Enter and OK preserve dispatch;
the persistence checkbox and dialog-close cleanup remain intact. GTK3 retains
native radio presentation. Packaging impact: none.

### GTK4 Stage 8 DCC Grouped Choice Controls

Date: 2026-07-21

Files/workflows converted: Transfers window Both, Uploads, and Downloads filter
construction, initial selection, exclusive state, and filter dispatch.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- Meson MSVC GTK4 probe: compile, link, and execution pass
- strict runtime checks: the first grouped-control constructor result is active
  by default; activating a later member clears both siblings; ordinary checkbox
  state remains independent
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  183 warnings, improved from 191 warnings
- expected complete GTK4 link failure retains 56 unique unresolved symbols and
  improves from 72 to 69 repeated unresolved-symbol diagnostics
- `dccgui.obj` has no remaining GTK4 compiler warning or unresolved diagnostic
- Preferences is the only remaining source of
  `gtk_radio_button_new_with_mnemonic`, `gtk_radio_button_get_group`, and
  `GTK_RADIO_BUTTON`
- inventory log: `build/gtk4-full/dcc-choice-controls-pass67.log`
- next target: Preferences grouped choice controls
- `git diff --check`: pass

Behavior contract: Both remains selected initially; Uploads and Downloads
remain mutually exclusive and immediately refresh the transfer list; GTK3
retains native radio presentation. The Join dialog's Nothing choice also now
retains its GTK3 default under GTK4. Packaging impact: none.

### GTK4 Stage 8 Preferences Grouped Choice Controls

Date: 2026-07-21

Files/workflows converted: Preferences Appearance page switcher-type choice
construction, initial selection, exclusive state, and preference dispatch.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: compile, link, and
  execution pass under `/W4 /WX`; zero warnings and zero errors
- Meson MSVC GTK4 probe: compile, link, and execution pass
- repository GTK4 Python validation: 28 tests pass
- source audit: no active frontend source calls
  `gtk_radio_button_new_with_mnemonic`, `gtk_radio_button_get_group`, or
  `GTK_RADIO_BUTTON`; the names remain only in the GTK3 compatibility branch
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  178 warnings, improved from 183 warnings
- expected complete GTK4 link failure improves from 56 to 53 unique unresolved
  symbols and from 69 to 66 repeated unresolved-symbol diagnostics
- `setup.obj` now contributes one unrelated combo-box warning and one matching
  unresolved diagnostic
- inventory log:
  `build/gtk4-full/preferences-choice-controls-pass68.log`
- next target: Preferences ordinary check-button state access
- `git diff --check`: pass

Behavior contract: Tabs and Tree remain mutually exclusive, their stored
indices remain 0 and 2 despite the intentionally blank list entry, and changing
the choice immediately updates the staged Preferences value. GTK3 retains
native radio presentation. Packaging impact: none.

### GTK4 Stage 8 Preferences Check-Button State

Date: 2026-07-21

Files/workflows converted: ordinary Preferences toggles, alert event matrices,
staged Boolean values, topic-bar choice sensitivity, and parent-controlled
field sensitivity.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and zero errors
- Meson MSVC GTK4 probe: build and execution pass
- repository GTK4 Python validation: 28 tests pass
- source audit: `setup.c` has no direct `GtkToggleButton`,
  `GTK_TOGGLE_BUTTON`, `gtk_toggle_button_get_active`, or
  `gtk_toggle_button_set_active` use
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  178 warnings, unchanged from pass 68
- expected complete GTK4 link failure retains 53 unique unresolved symbols and
  66 repeated unresolved-symbol diagnostics
- `setup.obj` retains only one unrelated combo-box warning and one matching
  unresolved diagnostic
- inventory log: `build/gtk4-full/preferences-check-controls-pass69.log`
- next target: Preferences combo-box wrap-width compatibility
- `git diff --check`: pass

Behavior contract: all Preferences Boolean values still initialize from staged
preferences and update immediately when toggled. Dependent controls and labels
retain their enabled state, mode-button placement still disables multi-line
topics, and all three alert-event columns remain independent. Packaging impact:
none.

### GTK4 Stage 8 Preferences Combo Wrap Compatibility

Date: 2026-07-21

Files/workflows converted: DCC upload/download speed-unit selector popup
presentation and the shared GTK3/GTK4 compatibility policy.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: helper signature
  compile, link, and execution pass under `/W4 /WX`; zero warnings and errors
- Meson MSVC GTK4 probe: compile, link, and execution pass
- repository GTK4 Python validation: 28 tests pass
- source audit: `gtk_combo_box_set_wrap_width` remains only in the GTK3 branch
  of `fabulor_gtk_combo_box_set_single_column`
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  177 warnings, improved from 178 warnings
- expected complete GTK4 link failure improves from 53 to 52 unique unresolved
  symbols and from 66 to 65 repeated unresolved-symbol diagnostics
- `setup.obj` has zero compiler warnings and zero unresolved diagnostics
- inventory log: `build/gtk4-full/preferences-combo-wrap-pass70.log`
- next target: remaining top-level visibility and lifecycle callbacks
- `git diff --check`: pass

Behavior contract: KiB/s and MiB/s remain a compact, single-column selector;
the remembered unit, speed conversion, allowed range, and staged transfer-speed
value remain unchanged. GTK4 uses its native popup layout. Packaging impact:
none.

### GTK4 Stage 8 Main-Window Visibility And Close Request

Date: 2026-07-21

Files/workflows converted: utility-window presentation, generic-tab closure,
user-list button regeneration, meter refresh, main/detached window reveal, and
tabbed main-window close dispatch.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and zero errors
- Meson MSVC GTK4 probe: build and execution pass
- repository GTK4 Python validation: 28 tests pass
- source audit: active GTK4 `maingui.c` paths use typed root lookup,
  `fabulor_gtk_widget_reveal_tree`, notebook/box removal, and `close-request`;
  GTK3-only code retains `gtk_widget_get_toplevel` and `delete-event`
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  173 warnings, improved from 177 warnings
- expected complete GTK4 link failure retains 52 unique unresolved symbols and
  improves from 65 to 62 repeated unresolved-symbol diagnostics
- `maingui.obj` improves from 23 to 20 unresolved diagnostics
- inventory log: `build/gtk4-full/main-window-visibility-close-pass71.log`
- next target: main-window finalization callbacks and cleanup ordering
- `git diff --check`: pass

Behavior contract: bringing a detached utility forward still presents its
owning window; closing utility tabs removes the matching view and closes the
main window when it was last; refreshed meters and controls remain visible;
tray-close still hides when supported; and closing the final tabbed window still
opens quit confirmation. Packaging impact: none.

### GTK4 Stage 8 Main-Window Finalization Ownership

Date: 2026-07-21

Files/workflows converted: detached IRC window finalization, shared tab-window
finalization, theme-listener cleanup ordering, detach/reattach callback
suppression, and shared theme-manager window ownership.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- Meson MSVC GTK4 probe: build and execution pass
- repository GTK4 Python validation: 28 tests pass
- source audit: active GTK4 main-window and theme-manager ownership uses
  `g_object_weak_ref`/`g_object_weak_unref`; GTK3 retains `destroy` callbacks
- source audit: detach/reattach disconnects the lifecycle owner and unregisters
  the obsolete GUI's theme listener before that GUI can be freed
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  173 warnings, unchanged from pass 71
- expected complete GTK4 link failure retains 52 unique unresolved symbols and
  62 repeated unresolved-symbol diagnostics
- `maingui.obj` retains 20 unresolved diagnostics
- inventory log: `build/gtk4-full/main-window-finalization-pass72.log`
- next target: auxiliary main-window dialog, user-list, and generic-tab
  finalization callbacks
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] close a detached IRC window and verify only its session is released
- [ ] close the shared tab window with and without detached windows remaining
- [ ] hide through tray-close and verify the live window still receives themes
- [ ] detach and reattach the first, middle, last, and only IRC tab
- [ ] switch themes after reattachment and then exit without duplicate cleanup

Behavior contract: session release still follows native window closure; theme
listeners are removed first; tray hiding does not finalize a live window; and
temporary detach/reattach destruction cannot invoke stale or duplicate session
cleanup. GTK3 callback behavior is retained. Packaging impact: none.

### GTK4 Stage 8 Auxiliary Finalization Ownership

Date: 2026-07-21

Files/workflows converted: quit and fatal-font dialog pointer clearing, main
user-list theme-listener lifetime, generic detached/embedded utility cleanup,
Channel List timer/model cleanup, and Raw Log, Keyboard Shortcuts, and Print
Events theme-listener ownership.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- Meson MSVC GTK4 probe: build and execution pass
- repository GTK4 Python validation: 28 tests pass
- source audit: `mg_create_generic_tab` accepts `GDestroyNotify`; all non-null
  callers implement the exact one-argument cleanup signature
- source audit: GTK4 generic tabs and user lists use weak finalization, while
  quit and fatal-font globals use weak pointer clearing
- source audit: Channel List, Raw Log, Keyboard Shortcuts, and Print Events no
  longer register a second widget `destroy` cleanup callback
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  173 warnings, unchanged from pass 72
- expected complete GTK4 link failure retains 52 unique unresolved symbols and
  62 repeated unresolved-symbol diagnostics
- inventory log: `build/gtk4-full/auxiliary-finalization-pass73.log`
- next target: Server List and Preferences top-level finalization callbacks
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] cancel and reopen the quit dialog without retaining a stale pointer
- [ ] close and reopen each utility as a detached window and an embedded tab
- [ ] close Channel List during refresh and verify timers stop and widths save
- [ ] close Raw Log, Keyboard Shortcuts, and Print Events after a theme change
- [ ] cancel Edit List and Keyboard Shortcuts without duplicate model cleanup
- [ ] detach/reattach sessions and switch themes after user-list reconstruction

Behavior contract: every generic utility cleanup runs once; close commands do
not recursively finalize their model; theme listeners and Channel List sources
cannot retain closed UI state; and GTK3 utility behavior remains unchanged.
Packaging impact: none.

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

### GTK4 Stage 8 Server List And Preferences Finalization Ownership

Date: 2026-07-21

Files/workflows converted: Server List and network-editor finalization,
client-certificate native chooser parent ownership, Preferences finalization,
and Preferences font-chooser pointer ownership.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- Meson MSVC GTK4 probe: clean build and execution pass
- repository GTK4 Python validation: 28 tests pass
- source audit: GTK4 Server List, editor, and Preferences cleanup use weak
  finalization; corresponding GTK3 paths retain typed `destroy` callbacks
- source audit: certificate import removes its GTK4 parent weak watch before
  response-owned unref, and parent finalization hides and releases the chooser
- source audit: Preferences font chooser clears its global pointer on response
  and on GTK4 finalization
- clean isolated complete GTK4 frontend compilation: zero C compiler errors and
  173 warnings, unchanged from pass 73
- expected complete GTK4 link failure retains 52 unique unresolved symbols and
  62 repeated unresolved-symbol diagnostics
- inventory log: `build/gtk4-full/server-preferences-finalization-pass74.log`
- next target: theme-import native chooser parent lifetime
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] close and reopen Server List through the title-bar and Close button
- [ ] connect from Server List and reopen it without stale selection/model state
- [ ] close the network editor directly and by closing its parent Server List
- [ ] accept and cancel certificate import, including while closing the editor
- [ ] cancel and reopen Preferences after opening and closing its font chooser
- [ ] apply Preferences and verify staged theme state commits exactly once

Behavior contract: close and response callbacks retain save/apply/connect
policy while object finalization owns pointer, model, theme-stage, and password
cleanup exactly once. A parent window can close before either native chooser
without retaining stale callback data. GTK3 behavior and packaging are
unchanged.

### GTK4 Stage 8 Theme-Import Chooser Lifetime

Date: 2026-07-21

Files/workflows converted: colour/HCT import parent ownership, legacy GTK3
theme-import parent ownership, theme dialog root lookup, and shared native
file-chooser local-path enforcement.

Automated evidence:

- shipping MSVC GTK3 frontend build: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- Meson MSVC GTK4 probe: clean build and execution pass
- repository GTK4 Python validation: 28 tests pass
- source audit: both theme-import response paths remove the GTK4 parent weak
  watch before releasing their native chooser
- source audit: GTK4 theme dialogs use the shared root-window boundary and no
  active theme-preference path calls `gtk_widget_get_toplevel()`
- source audit: all remaining local-only chooser requests use the shared
  adapter; GTK4 owned-path conversion returns `NULL` for non-local `GFile`
- clean isolated complete GTK4 frontend compilation: zero C compiler errors
  and 166 warnings, down from 173 in pass 74
- expected complete GTK4 link failure improves from 52 to 50 unique unresolved
  symbols and from 62 to 59 repeated unresolved-symbol diagnostics
- inventory log: `build/gtk4-full/theme-import-lifetime-pass75.log`
- next target: remaining Server List GTK3 widget and layout calls
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] accept and cancel `.hct` and `colors.conf` imports
- [ ] close Preferences while either native import chooser is open
- [ ] reject a non-local chooser result without changing staged colours
- [ ] open the colour manager and import-result dialogs from Preferences
- [ ] confirm certificate import still accepts a local file and rejects a
  non-local result

Behavior contract: chooser responses can reach their owner only while the
Preferences parent remains alive; parent closure hides and releases the native
chooser exactly once. Supported `.hct` and `colors.conf` semantics are
unchanged, and non-local selections never become filesystem paths.

### GTK4 Stage 8 Server List Widget Boundary

Date: 2026-07-21

Files/workflows converted: Server List and network-editor editable combo child
access, box attachment, window roles, and default-button ownership; shared
utility-window role containment.

Automated evidence:

- shipping MSVC GTK3 frontend build: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- Meson MSVC GTK4 probe: clean configure, build, and execution pass
- repository GTK4 Python validation: 28 tests pass
- source audit: `servlistgui.c` has no direct `GtkBin`, `gtk_box_pack_start()`,
  `gtk_window_set_role()`, `gtk_widget_set_can_default()`, or
  `gtk_widget_grab_default()` calls
- source audit: the complete GTK4 compile reports no diagnostics from
  `servlistgui.c`
- clean isolated complete GTK4 frontend compilation: zero C compiler errors
  and 156 warnings, down from 166 in pass 75
- expected complete GTK4 link failure improves from 50 to 46 unique unresolved
  symbols and from 59 to 52 repeated unresolved-symbol diagnostics
- inventory log: `build/gtk4-full/server-list-widget-boundary-pass76.log`
- next target: main-window label/icon child access and pane-layout queries
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] open the Server List and confirm Close remains the default action
- [ ] open the network editor and confirm Close remains the default action
- [ ] edit and retain a custom character set
- [ ] change login method and confirm dependent password controls update
- [ ] resize both windows and inspect button, separator, and notebook spacing

Behavior contract: Server List layout retains its existing expansion, fill,
padding, and focus behavior. GTK3 keeps its window-manager roles and default
button mechanics; GTK4 assigns the default through the owning window. The
editable character-set control continues to emit changes from its entry child.

### GTK4 Stage 8 Main-Window Child And Pane Boundary

Date: 2026-07-21

Files/workflows converted: main-window nickname label lookup, access-icon
identity and rendering, empty-pane visibility, right-pane sizing, persistence,
and initial restoration.

Automated evidence:

- shipping MSVC GTK3 frontend build: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- Meson MSVC GTK4 probe: clean configure, build, and execution pass
- repository GTK4 Python validation: 28 tests pass
- source audit: active GTK4 main-window paths no longer call `GTK_BIN`,
  `gtk_bin_get_child()`, `gtk_image_get_pixbuf()`,
  `gtk_paned_get_child1()`, `gtk_paned_get_child2()`, or
  `gtk_widget_style_get()`
- source audit: GTK4 right-pane restoration waits for allocated pane and end
  child widths through a one-shot frame callback; GTK3 retains one
  `size-allocate` callback
- clean isolated complete GTK4 frontend compilation: zero C compiler errors
  and 142 warnings, down from 156 in pass 76
- expected complete GTK4 link failure improves from 46 to 40 unique unresolved
  symbols and from 52 to 46 repeated unresolved-symbol diagnostics
- inventory log: `build/gtk4-full/main-window-child-pane-boundary-pass77.log`
- next target: main-window reply-bar child reveal and hidden-until-used
  semantics
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] change away state and confirm nickname label styling updates
- [ ] change channel privilege and confirm the access icon updates only when
  its source changes
- [ ] hide and show the user list while preserving its configured width
- [ ] restart with a saved right-pane width and confirm first layout restores
  it without a visible jump
- [ ] hide both children of either vertical pane and confirm its divider is
  not left visible

Behavior contract: nickname styling and access-icon identity remain tied to
the active session. Right-pane width is measured from the window's right edge,
includes the live divider width, and is restored only after complete layout.
GTK3 geometry behavior is unchanged.

### GTK4 Stage 8 Reply-Bar Visibility Boundary

Date: 2026-07-21

Files/workflows converted: main-window reply-bar initial visibility, explicit
child reveal, reply-state activation, cancellation, and post-send hiding.

Automated evidence:

- shipping MSVC GTK3 frontend build: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- Meson MSVC GTK4 probe: clean configure, build, and execution pass
- repository GTK4 Python validation: 28 tests pass
- source audit: `maingui.c` no longer calls `gtk_container_foreach()` or
  `gtk_widget_set_no_show_all()`
- source audit: GTK3 keeps recursive-show exclusion inside the compatibility
  helper; GTK4 uses explicit visibility and sibling traversal
- clean isolated complete GTK4 frontend compilation: zero C compiler errors
  and 140 warnings, down from 142 in pass 77
- expected complete GTK4 link failure improves from 40 to 38 unique unresolved
  symbols and from 46 to 44 repeated unresolved-symbol diagnostics
- inventory log: `build/gtk4-full/reply-bar-visibility-pass78.log`
- next target: residual main-window GTK3 popup/menu constructor containment
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] open a channel with no reply selected and confirm the reply bar is hidden
- [ ] select Reply and confirm the label and close button appear together
- [ ] cancel a reply and confirm the bar hides without changing edit-box text
- [ ] send a tagged reply and confirm the bar hides after dispatch
- [ ] switch tabs with different reply state and confirm visibility follows the
  active session

Behavior contract: an inactive reply bar is never exposed by initial window
reveal. Active reply state reveals the complete bar atomically, and clearing
reply state hides the bar without changing its children or input state.

### GTK4 Stage 8 Legacy Widget-Menu Constructor Containment

Date: 2026-07-21

Files/workflows converted: main-window icon/submenu constructor declarations,
legacy popup release, tray widget-menu item creation and population, mutable
tray item labels, and obsolete Win32 widget-menu hover tracking.

Automated evidence:

- shipping MSVC GTK3 frontend build: pass; zero warnings and zero errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- Meson MSVC GTK4 probe: clean configure, build, and execution pass
- repository GTK4 Python validation: 28 tests pass
- source audit: main-window icon/submenu widget constructors and the tray's
  legacy widget-menu population compile only for GTK3
- source audit: GTK4 retains the tab context model/presenter and native Windows
  tray popup without mutable `GtkMenuItem` state
- clean isolated complete GTK4 frontend compilation: zero C compiler errors
  and 123 warnings, down from 140 in pass 78
- expected complete GTK4 link failure improves from 38 to 33 unique unresolved
  symbols and from 44 to 37 repeated unresolved-symbol diagnostics
- inventory log:
  `build/gtk4-full/legacy-menu-constructor-containment-pass79.log`
- next target: legacy GTK status-icon backend isolation
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] right-click an IRC tab and confirm the retained GTK4 context menu opens
- [ ] trigger Detach and Close from the retained tab menu
- [ ] open the native Windows tray menu and trigger Hide/Restore, Away/Back,
  Preferences, and Quit
- [ ] confirm no GTK widget-menu or mutable tray item is created by GTK4

Behavior contract: GTK3 keeps its legacy widget menus unchanged. GTK4 uses
only retained menu models/presenters and the native Windows tray popup; no
removed GTK menu widget can be constructed or mutated in that profile.

### GTK4 Stage 8 Legacy Status-Icon Backend Isolation

Date: 2026-07-22

Files/workflows converted: tray backend compile capability, legacy status-icon
declarations and state, status-icon operations, and status-icon popup callback.

Automated evidence:

- shipping MSVC GTK3 frontend build: pass; `plugin-tray.c` rebuilt and the
  shipping executable linked
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero compiler warnings and errors
- fresh Meson/Ninja MSVC GTK4 probe: clean configure, build, and execution pass
- repository GTK4 Python validation: 28 tests pass
- source audit: every `GtkStatusIcon` declaration, object, callback, and
  operation compiles only when `GTK_MAJOR_VERSION < 4`
- source audit: GTK4 without AppIndicator has an inert operation table and
  cannot call or acquire a legacy status-icon symbol
- clean isolated complete GTK4 frontend compilation: zero C compiler errors
  and 123 warnings, unchanged from pass 79
- expected complete GTK4 link failure improves from 33 to 29 unique unresolved
  symbols and from 37 to 29 repeated unresolved-symbol diagnostics
- no `GtkStatusIcon` or `gtk_status_icon_*` text occurs in the complete GTK4
  build inventory
- inventory log:
  `build/gtk4-full/status-icon-backend-isolation-pass80.log`
- next target: GTK4 application startup and main-loop ownership
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] enable the tray in the shipping GTK3 client and confirm its icon appears
- [ ] trigger Restore, Away/Back, Preferences, and Quit from the GTK3 tray menu
- [ ] confirm a GTK4 build without a supported tray backend stays visible and
  never offers minimize-to-tray behavior
- [ ] confirm the later native GTK4 tray backend can be enabled without
  reintroducing `GtkStatusIcon`

Behavior contract: GTK3 retains its AppIndicator and legacy status-icon
backends. GTK4 cannot compile or select the legacy status-icon implementation;
when no supported backend exists, tray initialization fails closed without
hiding the application window.

### GTK4 Stage 8 Application Main-Loop Ownership

Date: 2026-07-22

Files/workflows converted: frontend option initialization, GTK initialization,
main-loop entry, shutdown requests, and main-loop lifetime ownership.

Automated evidence:

- clean shipping MSVC GTK3 frontend rebuild: pass; zero warnings and errors,
  and the GTK4 main-loop owner is excluded from the target
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- fresh Meson/Ninja MSVC GTK4 probe: clean configure, build, and execution pass
- strict lifecycle probe: quit-before-run returns without blocking; active
  idle-source shutdown runs once and returns with inactive state
- repository GTK4 Python validation: 28 tests pass
- source audit: GTK4 does not register `gtk_get_option_group()` and calls the
  correct no-argument `gtk_init()`
- source audit: `gtk_main()` and `gtk_main_quit()` compile only for GTK3
- clean isolated complete GTK4 frontend compilation: zero C compiler errors
  and 117 warnings, down from 123 in pass 80
- expected complete GTK4 link failure improves from 29 to 26 unique unresolved
  symbols and from 29 to 26 repeated unresolved-symbol diagnostics
- no `gtk_get_option_group`, `gtk_main`, or `gtk_main_quit` text occurs in the
  complete GTK4 build inventory
- inventory log: `build/gtk4-full/application-main-loop-pass81.log`
- next target: Windows GTK4 icon-theme bootstrap
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] start Fabulor normally and confirm initial windows and connections appear
- [ ] launch with supported command-line options and confirm they retain their
  existing behavior
- [ ] quit through the window, menu, tray action, and shutdown command
- [ ] enable wait-on-exit and confirm its post-loop delay remains intact
- [ ] trigger an early startup shutdown and confirm the GTK4 loop is not entered

Behavior contract: the common core still completes configuration, frontend,
plugin, and session initialization before entering the UI loop. GTK4 owns and
releases one default-context loop and honors shutdown before or during entry;
GTK3 startup and shutdown are unchanged.

### GTK4 Stage 8 Windows Icon-Theme Bootstrap

Date: 2026-07-22

Files/workflows converted: default icon-theme acquisition, startup icon search
paths, indexed-theme eligibility, Adwaita selection, and runtime-layout fallback.

Automated evidence:

- clean shipping MSVC GTK3 frontend rebuild: pass; zero warnings and errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- fresh Meson/Ninja MSVC GTK4 probe: clean configure, build, and execution pass
- strict icon-theme probe: the display default resolves, and an isolated GTK4
  theme accepts an added search root and reports the selected Adwaita name
- repository GTK4 Python validation: 28 tests pass
- source audit: GTK4 default theme acquisition is display-scoped and uses
  `gtk_icon_theme_add_search_path()` plus `gtk_icon_theme_set_theme_name()`
- source audit: GTK4 accepts indexed icon roots; GTK3 alone retains the
  fail-fast `hicolor/index.theme` safeguard
- source audit: startup recognizes candidate `Runtime/GTK4/share/icons` and
  flattened `share/icons` Adwaita layouts
- clean isolated complete GTK4 frontend compilation: zero C compiler errors
  and 112 warnings, down from 117 in pass 81
- expected complete GTK4 link failure improves from 26 to 23 unique unresolved
  symbols and from 26 to 23 repeated unresolved-symbol diagnostics
- startup no longer contributes `gtk_icon_theme_get_default`,
  `gtk_icon_theme_append_search_path`, `gtk_icon_theme_set_custom_theme`, or
  `gtk_icon_theme_rescan_if_needed`; one main-window default lookup remains
- inventory log: `build/gtk4-full/windows-icon-theme-bootstrap-pass82.log`
- next target: main-window icon-theme lookup boundary
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] start from the candidate runtime and confirm toolbar, access, dialog, and
  status icons resolve without ambient GTK paths
- [ ] place valid custom icons under `%APPDATA%\Fabulor\icons` and confirm they
  are discoverable
- [ ] set `ZOITECHAT_ICON_PATH` to a valid indexed theme root and confirm GTK4
  accepts it
- [ ] launch from a different working directory and confirm executable-relative
  candidate icons still resolve
- [ ] run the shipping GTK3 client and confirm its icon theme remains stable

Behavior contract: Windows icon lookup remains executable-relative and keeps
the established environment, user, module, working-directory, and argv search
sources. GTK4 accepts standard indexed themes and uses display ownership; GTK3
retains its defensive indexed-theme rejection unchanged.

### GTK4 Stage 8 Main-Window Icon-Theme Lookup

Date: 2026-07-22

Files/workflows converted: edit-box emoji access icon theme lookup and packaged
fallback selection.

Automated evidence:

- clean shipping MSVC GTK3 frontend rebuild: pass; zero warnings and errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- fresh Meson/Ninja MSVC GTK4 probe: clean configure, build, and execution pass
- repository GTK4 Python validation: 28 tests pass
- source audit: the main-window icon lookup uses the cross-version
  display-owned default-theme helper
- source audit: candidate icon order and packaged `icon-resolver` fallback are
  unchanged
- clean isolated complete GTK4 frontend compilation: zero C compiler errors
  and 110 warnings, down from 112 in pass 82
- expected complete GTK4 link failure improves from 23 to 22 unique unresolved
  symbols and from 23 to 22 repeated unresolved-symbol diagnostics
- no active `gtk_icon_theme_get_default` reference occurs in the complete GTK4
  build inventory
- inventory log: `build/gtk4-full/main-window-icon-theme-lookup-pass83.log`
- next target: GTK4 window-operation boundary
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] confirm the edit-box emoji access icon appears from the active GTK4 theme
- [ ] remove the themed candidate and confirm the packaged fallback still
  appears
- [ ] activate the icon and confirm the emoji picker opens at the edit box
- [ ] switch GTK4 themes and confirm icon availability follows the display
  theme without changing picker behavior

Behavior contract: the first available themed emoji icon remains preferred,
with the packaged resolver as fallback. GTK4 obtains availability from its
display-owned icon theme; GTK3 lookup and all entry interactions are unchanged.

### GTK4 Stage 8 Window Operations

Date: 2026-07-22

Files/workflows converted: startup and command-driven minimization, attention
hints, auxiliary-window identity, and Windows post-fullscreen sizing.

Automated evidence:

- clean shipping MSVC GTK3 frontend rebuild: pass; zero warnings and errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- fresh Meson/Ninja MSVC GTK4 probe: clean configure, build, and execution pass
- repository GTK4 Python validation: 28 tests pass
- probe audit: every new cross-version window helper is address-taken and
  linked by both strict probe build systems
- source audit: GTK4 minimization uses the realized window's `GdkToplevel`
- source audit: GTK4 post-fullscreen sizing uses the configured default size;
  GTK3 retains `gtk_window_resize()` and its existing position restoration
- source audit: GTK4 urgency and per-window WM-class branches are explicit
  no-ops because GTK4 removed both hints; process identity remains configured
  before GTK initialization
- clean isolated complete GTK4 frontend compilation: zero C compiler errors
  and 106 warnings, down from 110 in pass 83
- expected complete GTK4 link failure improves from 22 to 18 unique unresolved
  symbols and from 25 to 21 unresolved-symbol diagnostics
- `gtk_window_iconify`, `gtk_window_set_urgency_hint`,
  `gtk_window_set_wmclass`, and `gtk_window_resize` no longer occur in the
  active GTK4 inventory
- inventory log: `build/gtk4-full/window-operations-pass84.log`
- next target: main-window menu-font traversal boundary
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] start with the normal minimize option and confirm the main window starts
  minimized
- [ ] invoke command-driven iconify and confirm the active main window
  minimizes
- [ ] trigger an unfocused highlight and confirm GTK4 does not steal focus
- [ ] enter and leave fullscreen on Windows and confirm the saved non-maximized
  size is restored
- [ ] open auxiliary windows and confirm taskbar grouping follows Fabulor's
  established process identity

Behavior contract: GTK3 window operations remain unchanged. GTK4 minimizes
only a realized toplevel, restores the configured size after fullscreen, and
does not replace removed compositor hints with focus-stealing presentation.

### GTK4 Stage 8 Main-Menu Font And Refresh

Date: 2026-07-22

Files/workflows converted: main-menu font application and theme-driven menu
relayout.

Automated evidence:

- clean shipping MSVC GTK3 frontend rebuild: pass; zero warnings and errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- fresh Meson/Ninja MSVC GTK4 probe: clean configure, build, and execution pass
- repository GTK4 Python validation: 28 tests pass
- source audit: GTK4 applies font styling and relayout only to the
  model-owned `GtkPopoverMenuBar` root
- source audit: GTK4 does not inspect generated menu children or submenus;
  GTK3 retains its recursive menu-shell traversal unchanged
- clean isolated complete GTK4 frontend compilation: zero C compiler errors
  and 96 warnings, down from 106 in pass 84
- expected complete GTK4 link failure improves from 18 to 14 unique unresolved
  symbols and from 21 to 15 unresolved-symbol diagnostics
- `GTK_IS_MENU_SHELL`, `GTK_IS_MENU_ITEM`, `GTK_MENU_ITEM`, and
  `gtk_menu_item_get_submenu` no longer occur in the active GTK4 inventory
- inventory log: `build/gtk4-full/main-menu-font-pass85.log`
- next target: main-window emoji fallback style lookup
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] change the configured input font and confirm the GTK4 menu bar and its
  generated popovers inherit the updated font
- [ ] switch themes and confirm the menu bar and open popovers relayout without
  stale sizing or clipped labels
- [ ] rebuild and run GTK3, then confirm nested menus retain their existing
  font and preferred-size behavior

Behavior contract: GTK4 menu structure remains owned by the retained model and
receives inherited font styling from its root. GTK3 continues recursive widget
and submenu updates with no behavior change.

### GTK4 Stage 8 Emoji Fallback Font Lookup

Date: 2026-07-22

Files/workflows converted: effective widget font discovery used by emoji
fallback CSS for mode entries, picker labels, and the search entry.

Automated evidence:

- clean shipping MSVC GTK3 frontend rebuild: pass; zero warnings and errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- fresh Meson/Ninja MSVC GTK4 probe: clean configure, build, and execution pass
- repository GTK4 Python validation: 28 tests pass
- probe audit: the owned widget font-description helper is address-taken and
  linked by both strict probe build systems
- source audit: GTK4 copies the effective default description from the
  widget's `PangoContext`; GTK3 retains the style-context `"font"` query
- source audit: both branches return caller-owned descriptions and the emoji
  fallback caller frees the base and merged descriptions
- source audit: emoji-family detection, fallback order, and CSS provider keys
  are unchanged
- clean isolated complete GTK4 frontend compilation: zero C compiler errors
  and 95 warnings, down from 96 in pass 85
- expected complete GTK4 link failure improves from 14 to 13 unique unresolved
  symbols and from 15 to 14 unresolved-symbol diagnostics
- `gtk_style_context_get` no longer occurs in the active GTK4 inventory, and
  `maingui.c` contributes no unresolved symbols
- inventory log: `build/gtk4-full/emoji-font-lookup-pass86.log`
- next target: native save-dialog overwrite confirmation
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] confirm emoji render with the configured fallback family in mode key and
  limit entries
- [ ] confirm emoji picker labels retain their large size and fallback glyphs
- [ ] confirm emoji in the search entry uses the fallback without changing the
  configured text font
- [ ] switch GTK4 themes and fonts, then reopen these widgets and confirm their
  effective base font remains current

Behavior contract: the effective widget font remains the base, the emoji
family remains appended only when absent, and every temporary font description
has one clear owner. GTK3 rendering behavior is unchanged.

### GTK4 Stage 8 Native Save-Dialog Overwrite Confirmation

Date: 2026-07-22

Files/workflows converted: overwrite-confirmation policy for native save
requests, including buffer, channel-list, raw-log, event, URL-list, and DCC
save-as callers.

Automated evidence:

- clean shipping MSVC GTK3 frontend rebuild: pass; zero warnings and errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- fresh Meson/Ninja MSVC GTK4 probe: clean configure, build, and execution pass
- repository GTK4 Python validation: 28 tests pass
- probe audit: the overwrite-confirmation compatibility helper is
  address-taken and linked by both strict probe build systems
- source audit: GTK3 applies the requested boolean through
  `gtk_file_chooser_set_do_overwrite_confirmation()`; GTK4 leaves the native
  save dialog's confirmation policy unchanged
- source audit: save callers continue to express the existing
  `FRF_NOASKOVERWRITE` policy without direct toolkit calls
- clean isolated complete GTK4 frontend compilation: zero C compiler errors
  and 94 warnings, down from 95 in pass 86
- expected complete GTK4 link failure improves from 13 to 12 unique unresolved
  symbols and from 14 to 13 unresolved-symbol diagnostics
- `gtk_file_chooser_set_do_overwrite_confirmation` no longer occurs in the
  active GTK4 link inventory
- inventory log: `build/gtk4-full/save-overwrite-confirmation-pass87.log`
- next target: shared widget destruction
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] save a buffer, channel list, raw log, event file, and URL list over an
  existing file and confirm the native GTK4 dialog applies its platform policy
- [ ] use DCC Save As over an existing path and confirm the native GTK4 result
  is accepted without a second application-owned prompt
- [ ] rebuild and run GTK3, then confirm ordinary saves still prompt while DCC
  Save As retains the existing `FRF_NOASKOVERWRITE` behavior

Behavior contract: save requests retain one native chooser and one completion
callback. GTK3 preserves the explicit overwrite toggle. GTK4 does not recreate
that removed toggle with a separate path check or application dialog; the
native save dialog remains the sole owner of overwrite confirmation.

### GTK4 Stage 8 Typed Widget Destruction

Date: 2026-07-22

Files/workflows converted: Preferences Cancel closure, About dialog response
closure, and removal of the About dialog's generated action buttons.

Automated evidence:

- clean shipping MSVC GTK3 frontend rebuild: pass; zero warnings and errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- fresh Meson/Ninja MSVC GTK4 probe: clean configure, build, and execution pass
- repository GTK4 Python validation: 28 tests pass
- helper audit: both typed window destruction and typed box-child removal are
  address-taken and exercised by the strict GTK4 probe
- source audit: active top-level closures validate and destroy a `GtkWindow`;
  action-button removal validates the parent box and child relationship
- source audit: remaining raw `gtk_widget_destroy()` calls in `menu.c` are
  confined to GTK3-only menu code
- clean isolated complete GTK4 frontend compilation: zero C compiler errors
  and 93 warnings, down from 94 in pass 87
- expected complete GTK4 link failure improves from 12 to 11 unique unresolved
  symbols and from 13 to 11 unresolved-symbol diagnostics
- `gtk_widget_destroy` no longer occurs in the active GTK4 link inventory
- inventory log: `build/gtk4-full/widget-destruction-pass88.log`
- next target: legacy About-dialog layout and presentation
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] open Preferences, select Cancel, and confirm the window closes once
  without applying pending changes
- [ ] open About, follow the Website and License actions, and confirm the
  dialog remains open after each action
- [ ] close About through its Close action and window control, confirming
  theme cleanup and window finalization run once
- [ ] repeat Preferences and About closure on GTK3 and confirm unchanged
  behavior

Behavior contract: top-level windows are closed through window ownership, not
generic widget disposal. Child removal is performed by the owning container.
GTK3 retains its existing destruction semantics; GTK4 uses its explicit window
and parent-child lifecycle APIs.

### GTK4 Stage 8 Native About Dialog

Date: 2026-07-22

Files/workflows converted: About construction, logo ownership, website and
license access, presentation, Close/Escape behavior, and toolkit-specific
action layout.

Automated evidence:

- clean shipping MSVC GTK3 frontend rebuild: pass; zero warnings and errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- fresh Meson/Ninja MSVC GTK4 probe: clean configure, build, and execution pass
- repository GTK4 Python validation: 28 tests pass
- probe audit: the typed About-logo helper is address-taken and linked by both
  strict probe build systems
- source audit: GTK4 constructs `GtkAboutDialog` through its native constructor
  and does not cast it to the removed `GtkDialog` base
- source audit: GTK4 uses the native website, GPL 2.0-only license,
  Close/Escape, link activation, transient-parent, and window presentation
  contracts; GTK3 retains its custom response buttons
- ownership audit: GTK4 creates one `GdkTexture` from the retained logo pixbuf,
  assigns it as the dialog paintable, and releases the temporary reference
- clean isolated complete GTK4 frontend compilation: zero C compiler errors
  and 86 warnings, down from 93 in pass 88
- expected complete GTK4 link failure improves from 11 to 5 unique unresolved
  symbols and from 11 to 5 unresolved-symbol diagnostics
- `GTK_BUTTON_BOX`, `gtk_button_box_set_child_secondary`, `GTK_CONTAINER`,
  `gtk_container_get_children`, `gtk_dialog_get_action_area`, and
  `gtk_widget_show_all` no longer occur in the active GTK4 link inventory
- inventory log: `build/gtk4-full/about-dialog-pass89.log`
- next target: remaining check-menu and toggle-action boundary
- `git diff --check`: pass

Manual checks deferred until the full GTK4 frontend links:

- [ ] open About and confirm the logo, program name, version, copyright,
  platform details, and theme styling render correctly
- [ ] activate Website and confirm it opens the Fabulor project URL once
- [ ] open the GPL 2.0-only license page and return to the main About view
- [ ] close About through its native Close control, Escape, and window control,
  confirming each path finalizes the window once
- [ ] repeat the custom Website, License, and Close actions on GTK3 and confirm
  unchanged behavior

Behavior contract: GTK4 owns the About dialog as a native top-level window and
provides its standard accessible website, license, and closure UI. Link
activation continues through Fabulor's URL opener. GTK3 retains its established
custom action-area behavior and visual layout.

### GTK4 Stage 8 Menu-Toggle Link Closure

Date: 2026-07-22

Files/workflows converted: View-menu action-state propagation, channel-switcher
and network-meter radio callbacks, and tab-context Autojoin/Auto-Connect menu
construction.

Automated evidence:

- clean shipping MSVC GTK3 frontend rebuild: pass; zero warnings and errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- fresh Meson/Ninja MSVC GTK4 probe: clean configure, build, and execution pass
- repository GTK4 Python validation: 28 tests pass
- source audit: GTK4 View and layout synchronization uses retained boolean and
  string-target `GSimpleAction` state; the check-item widget fallback is GTK3-only
- source audit: GTK4 tab context dispatch owns Autojoin and Auto-Connect state;
  their legacy widget-menu constructors and callbacks are GTK3-only
- clean isolated complete GTK4 frontend build: pass; zero compiler errors, 81
  warnings, zero unresolved-symbol diagnostics, and zero unresolved symbols
- improvement from pass 89: warnings fall from 86 to 81 and the final five
  unresolved symbols and diagnostics are removed
- linked candidate: `build/gtk4-full/x64/rel/fabulor.exe`, 1,592,832 bytes,
  SHA-256 `EF98F010E475DE6BBA7B4C15697BDB684D3FAC9BD0D4324E3B5CCB28F14C2ADF`
- PE import audit: `gtk-4-1.dll` is present; no GTK3 DLL is imported
- inventory log: `build/gtk4-full/menu-toggle-closure-pass90.log`
- next target: isolated candidate runtime staging and startup smoke validation
- `git diff --check`: pass

Manual checks deferred to candidate runtime staging:

- [ ] toggle Menu Bar, Topic Bar, User List, user-list buttons, mode buttons,
  and fullscreen; confirm each retained action and visible state stays aligned
- [ ] switch between Tabs and Tree and confirm the selected radio target and
  saved layout stay aligned across all windows
- [ ] switch among Off, Graph, Text, and Both network-meter modes and confirm
  all windows update without duplicate dispatch
- [ ] toggle Autojoin and Auto-Connect from GTK4 tab context menus, reopen the
  menus, and confirm persisted state
- [ ] repeat the equivalent GTK3 widget-menu workflows and confirm unchanged
  behavior

Behavior contract: GTK4 menu state has one owner in retained actions and
models; no GTK4 path falls back to removed check-menu widgets. GTK3 preserves
its existing widget callbacks and signal-blocked state synchronization. The
full GTK4 source profile now compiles and links, but this isolated executable is
not yet a staged or runtime-validated release candidate.

### GTK4 Stage 8 Candidate Startup Bootstrap

Date: 2026-07-22

Files/workflows converted: executable-relative runtime bootstrap, GTK4 frontend
module entry, isolated launcher build, PE import validation, and controlled
candidate startup/shutdown.

Automated evidence:

- shipping MSVC GTK3 frontend rebuild: pass; zero warnings and errors
- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; zero warnings and errors
- fresh Meson/Ninja MSVC GTK4 probe: configure, 61-step build, and execution pass
- repository validation: 38 tests pass
- complete isolated GTK4 profile: pass; `fabulor.exe` launcher and
  `fabulor-gtk4-frontend.dll` produced with zero compiler or linker errors
- launcher build: pass under `/W4 /WX`; zero warnings and errors
- bootstrap contract tests: six pass
- staged runtime import validation: 35 native files, four roots, 107 packaged
  edges, and 54 reviewed system imports
- frontend bootstrap PE validation: nine launcher imports, all system-owned;
  31 frontend imports resolved through the staged runtime, reviewed OpenSSL
  application DLLs, or system allowlist; exported entry present; no GTK3 import
- isolated candidate launcher SHA-256:
  `66FC3472803D65C9A70C751516C45AB7EE464B5A20A59AEF37DB5A76A9DFBC19`
- isolated frontend SHA-256:
  `8C6628BF198932CE401F282DE2C0CB7B51E7AFA7A853BA808F93DA2768911F0F`

Controlled smoke evidence:

- launched from `build/gtk4-startup-smoke` with a System32-only `PATH`,
  `--no-auto`, `--no-plugins`, and a workspace-only configuration root
- responsive `Network List - Fabulor` window remained active for eight seconds
- loaded `fabulor-gtk4-frontend.dll` from the candidate root and GTK4, GLib,
  GObject, and GIO from `Runtime/GTK4/bin`
- normal window close returned exit code 0
- no matching Windows Application event was recorded during the run

Behavior contract: the process bootstrap has no GTK or GLib import and must
register the trusted nested runtime before loading the frontend module. The
shipping GTK3 build and installed/user configuration roots remain untouched.
WiX frontend-module composition and clean-machine feature workflows are the
next packaging targets.

### GTK4 Stage 8 Candidate MSI Composition

Date: 2026-07-22

Files/workflows converted: isolated candidate product identity and component
composition, CI build/artifact publication, exact extracted-content validation,
and packaged launcher/frontend PE validation.

Automated evidence:

- candidate WiX rebuild: pass; zero warnings and zero errors
- ordinary shipping MSI and bootstrapper regression rebuild: pass; candidate
  component fragments remain excluded from shipping composition; four local
  warnings reflect the absent pinned .NET 8.0.28 directories and one retained
  warning reflects shipping same-version upgrade policy
- rebuilt shipping MSI GTK4 subtree: 1,432 installed entries and all hashes
  still match the locked 1,431-file runtime manifest plus generated manifest
- candidate identity: `Fabulor GTK4 Frontend Candidate`, distinct UpgradeCode
  `B0361915-6035-48B7-B535-8E72AB7493AA`, and distinct install folder
  `Program Files\Fabulor GTK4 Candidate`
- candidate installed payload: exactly 1,437 files; five reviewed root files,
  1,431 manifest-locked runtime files, and `runtime-manifest.json`
- decompiled MSI content: every extracted size and SHA-256 hash verified
- packaged launcher PE validation: nine imports, all reviewed system modules
- packaged frontend PE validation: 31 imports, all resolved through the runtime,
  reviewed OpenSSL application DLLs, or system allowlist; exported entry present
- focused validator tests: five pass, including wrong identity, forbidden
  installer side effects, unexpected GTK3 payload, and packaged-content
  mismatch rejection
- candidate MSI SHA-256:
  `C4BCA3EF3CB5074C815D022490A40662012CCC790730E75A0AC09F8BBA6AF3E8`
- shipping MSI product, bootstrapper, shortcuts, protocol registration, plugins,
  Enchant, Python, Tcl, .NET, and user configuration remain outside this change

Behavior contract: candidate mode must remain a separate, minimal product and
must fail validation on any identity drift, unexpected installed file, content
change, legacy GTK import, unresolved native import, or missing frontend export.
Native plugin and Enchant compatibility remains required before those features
can enter the GTK4 candidate package.

### GTK4 Stage 8 Native Extension Compatibility

Date: 2026-07-22

Files/workflows converted: isolated native-extension build ownership, FiSHLiM
key-manager GTK4 UI, Enchant/WinSpell final-runtime rebuild, executable-relative
Windows plugin discovery, native import contract, candidate WiX composition,
and expanded exact-package validation.

Automated evidence:

- full shipping MSVC x64 solution rebuild: pass; production frontend and all
  native plugins build, and 18 manifest/path tests pass
- isolated GTK4 native extension build: eight projects pass; checksum, Exec,
  FiSHLiM, Lua, Python, SysInfo, updater, and WinRT notifications build against
  the final GTK4 library root
- FiSHLiM retained GTK3 branch: isolated rebuild pass
- FiSHLiM GTK4 PE audit: imports `gtk-4-1.dll`; no GTK3 import
- Enchant 2.8.19 and WinSpell final-runtime rebuild: pass; personal dictionary
  smoke pass
- Enchant core SHA-256:
  `F6B26865B1DB04ACC96F8BACA853D2CBDFA818EC0A363BCA95FC1677CD7E6EAC`
- WinSpell provider SHA-256:
  `381DFAF94A38E4D4193E4E47199DEC3F6D2E175BB6820842A3F8D2CD6C3B3380`
- native extension validator: nine modules, one data file, and fourteen owned
  import edges pass; required-import, GTK3, unresolved-import, and ownership
  failures are covered by four focused tests
- complete `tools/gtk4` Python validation: 43 tests pass
- candidate WiX rebuild with full ICE validation: pass; zero warnings and zero
  errors
- extracted candidate: exactly 1,447 files; all sizes and SHA-256 hashes pass
- packaged launcher imports: nine reviewed system modules
- packaged frontend imports: 31 resolved runtime/application/system modules
- packaged extension graph: nine modules, one data file, fourteen owned edges
- candidate MSI: 32,299,852 bytes; SHA-256
  `8A2240556091F10786860D8BEBA59AC4AD4EE2864695619A7A9D9FD5182BE8FC`

Controlled smoke evidence:

- launched the exact extracted candidate from `C:\Windows` with an isolated
  profile and local refused URL session so native autoload was exercised
- responsive `Fabulor` window remained active during module inspection
- checksum, Exec, FiSHLiM, updater, SysInfo, WinRT notifications, WinSparkle,
  Enchant core, and WinSpell provider all loaded from the candidate root
- normal window close returned exit code 0

Behavior contract: candidate native code must resolve only through its explicit
application, runtime, and reviewed Windows ownership roots. Windows autoload
must use the executable-relative plugin directory unless the explicit
development-runtime gate permits an override. Shipping GTK3 output and product
composition remain unchanged.

### GTK4 Stage 8 Plugin Host Parity

Date: 2026-07-22

Files/workflows converted: exact C#/Python/Tcl host staging, candidate WiX
composition, extracted-byte validation, CI runtime provisioning, and uniform
successful manifest-plugin startup reporting.

Automated evidence:

- plugin-host staging tests: six pass
- complete `tools/gtk4` Python validation: 50 tests pass
- plugin-host manifest: 5,821 files, 199,443,761 bytes, .NET 8.0.29; SHA-256
  `FBDC8221E3CC959A9F353E09812212B13B53A8D8808093B36B06AC836CE401DD`
- isolated GTK4 full-profile rebuild: pass; zero compiler errors and the
  previously recorded compatibility-warning inventory
- candidate WiX full ICE build: pass; zero warnings and zero errors
- extracted candidate: exactly 7,270 files; all sizes and SHA-256 hashes pass
- packaged native graph: ten modules, one data file, fifteen owned import edges
- candidate MSI: 100,229,788 bytes; SHA-256
  `6B1CF0226DD6405A73BD455F51EAE4B67E50C0CCE3C5FBED33BAD3C8B7BA2F43`
- full shipping MSVC x64 solution: pass; 18 manifest/path tests pass
- shipping MSI and bootstrapper: pass with no errors; decompiled hostfxr and
  shared-runtime directories both resolve to .NET 8.0.29
- shipping MSI: 133,330,185 bytes; SHA-256
  `1E7671B4951B4CD990F0CA0C3F399F56C9313BB9EB4BDC84859AF26665DEB76C`

Controlled smoke evidence:

- launched the exact extracted candidate from `C:\Windows` with an isolated
  profile, manifest-host developer gate, and local refused URL session
- loaded `hcpython3.dll`, `python314.dll`, `tcl86t.dll`, `hostfxr.dll`,
  `hostpolicy.dll`, `coreclr.dll`, `clrjit.dll`, and
  `Fabulor.PluginHost.dll` exclusively from the candidate root
- startup report recorded one C# Greeter, one Python Greeter, and one Tcl
  Greeter entry with `C#`, `Python`, and `Tcl` labels; the native Python host
  remained a separate single entry
- process remained responsive and normal window close returned exit code 0

Behavior contract: candidate plugin hosts must come only from the staged,
manifest-locked private runtime tree. Staging and MSI validation fail closed on
path, identity, count, hash, reparse-point, collision, or native ownership
drift. The manifest-host user preference and developer gate retain their
existing policy; this pass changes package parity, not trust defaults.

### GTK4 Stage 8 Win32 Display Filter Ownership

Date: 2026-07-22

Files/workflows converted: GTK4 native-message filter installation and
teardown, shared GTK3/GTK4 dispatch, bounded single-instance command payloads,
taskbar toggling, timezone refresh, and same-process wheel forwarding.

Automated evidence:

- complete isolated GTK4 frontend profile: pass; launcher and frontend module
  link successfully with the existing 81-warning conversion inventory
- full shipping MSVC x64 solution: pass; zero warnings and errors from this
  boundary, and all 18 manifest/path tests pass
- candidate WiX build with full ICE validation: pass; zero warnings and errors
- extracted candidate: exactly 7,270 files; all sizes and SHA-256 hashes pass
- packaged launcher imports: nine reviewed system modules
- packaged frontend imports: 31 resolved runtime/application/system modules
- packaged extension graph: ten modules, one data file, fifteen owned edges
- frontend module: 1,594,368 bytes; SHA-256
  `D0257C93335C764DC972D9B977FCC4E87BD7FF1FFFC76F1E77EF8254F2FDFB7B`
- candidate MSI: 100,229,788 bytes; SHA-256
  `BDEF6A46D1527A009FFF0600D6E7BFCA36AD5ED0C5CAD71FE3FE57B1ED9E89AD`

Controlled smoke evidence:

- launched the exact extracted candidate from `C:\Windows` with an isolated
  profile, disabled plugin autoload, and a local refused URL session
- sent the internal taskbar command directly to the candidate window through
  `WM_COPYDATA`; the window minimized and a second message restored it
- candidate remained responsive before and after dispatch
- normal window close returned exit code 0; the installed Fabulor process was
  not targeted or interrupted

Behavior contract: GTK4 owns exactly one main native-message filter on the
default Win32 display and removes it before releasing the retained display.
GTK4 appearance changes remain owned by the separate appearance monitor.
Single-instance payloads must use identifier zero, include a terminal NUL, and
remain between 2 bytes and 64 KiB. Invalid payloads continue through normal
window processing, and wheel messages are consumed only after successful
same-process forwarding. GTK3 retains its existing per-window filter.

### GTK4 Stage 8 Top-Level Visibility Closure

Date: 2026-07-23

Files/workflows converted: shared visible-state snapshots and observation,
top-level hide/present policy, frontend GUI commands, tray hide/restore,
plugin window-status reporting, and Win32 taskbar visibility decisions.

Automated evidence:

- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; visible-state changes plus hide/present snapshots are
  covered
- complete isolated GTK4 frontend profile: pass; launcher and frontend module
  link with no new warning or unresolved symbol and retain the established
  81-warning cleanup inventory
- full shipping MSVC x64 solution: pass; zero warnings and errors, and all 18
  manifest/path tests pass
- complete `tools/gtk4` Python validation: 50 tests pass
- candidate WiX build with full ICE validation: pass; zero warnings and errors
- extracted candidate: exactly 7,270 files; all sizes and SHA-256 hashes pass
- packaged launcher imports: nine reviewed system modules
- packaged frontend imports: 31 resolved runtime/application/system modules
- packaged extension graph: ten modules, one data file, fifteen owned edges
- frontend module: 1,595,392 bytes; SHA-256
  `16EB63B4CC0A85399899F363869DB3CC74162973AB861489EF6E8C6994B18398`
- candidate MSI: 100,233,884 bytes; SHA-256
  `4F980188A7619020DBDC319207D75B311A42A1F221C2EA93FCE1523FB3506EB9`

Controlled smoke evidence:

- launched the exact extracted candidate from `C:\Windows` with an isolated
  profile, disabled plugin autoload, and a local refused URL session
- delivered `GUI HIDE` through the candidate's bounded single-instance path;
  the native window became invisible
- delivered `GUI SHOW`; the same window became visible and presented again
- candidate remained responsive before and after both transitions
- normal window close returned exit code 0; the installed Fabulor process was
  not targeted or interrupted

Behavior contract: visible, minimized, maximized, fullscreen, and focused
state are read from one typed owner. Visibility watches report an explicit
`FABULOR_WINDOW_STATE_VISIBLE` change, hide uses common widget visibility, and
present preserves GTK3 deiconification before common show/present behavior.
Frontend commands, tray policy, plugin status, and Win32 dispatch must not
perform independent top-level visibility reads. Existing versioned lifecycle,
geometry, and native-message owners remain unchanged.

### GTK4 Stage 8 Theme-Controller Application Integration

Date: 2026-07-23

Files/workflows converted: application-lifetime GTK4 theme controller,
display-scoped appearance callback, borrowed Preferences binding, staged
selection rollback, and ordered frontend theme teardown.

Automated evidence:

- strict MSVC GTK4 probe against GTK 4.22.4 / GLib 2.88.0: build and execution
  pass under `/W4 /WX`; owned and borrowed controller lifetimes, invalid-CSS
  rollback, theme/variant selection, appearance refresh, and teardown are
  covered
- complete isolated GTK4 frontend profile: pass; launcher and frontend module
  link with no new warning or unresolved symbol and retain the established
  81-warning cleanup inventory
- full shipping MSVC x64 solution: pass with zero warnings and zero errors; all
  18 manifest/path tests pass
- complete `tools/gtk4` Python validation: 50 tests pass
- candidate WiX build with full ICE validation: zero warnings and zero errors
- extracted candidate: exactly 7,270 files; every size and SHA-256 hash passes
- packaged launcher imports: nine reviewed system modules
- packaged frontend imports: 31 resolved runtime/application/system modules
- packaged extension graph: ten modules, one data file, fifteen owned edges
- frontend module: 1,572,864 bytes; SHA-256
  `470C48CA8CE03F81EA0F9418B94AC36F008C9C920BAB042CA079E9E75FB0ED5E`
- candidate MSI: 100,221,596 bytes; SHA-256
  `562EA53966686F7CA8216ED9F347BFC3B37016E4CA582404DF2083B5829C1758`

Controlled smoke evidence:

- launched the exact extracted candidate from `C:\Windows` with an isolated
  profile and hidden test presentation
- application theme initialization completed, a responsive main window was
  created, and normal close returned exit code 0
- the existing GTK4 conversion diagnostics remained outside the changed theme
  boundary; no theme-controller, CSS-provider, appearance-filter, or teardown
  diagnostic was emitted

Behavior contract: GTK4 owns one application-lifetime controller. Preferences
may borrow but must not destroy it; successful previews update staged settings,
and Cancel or save failure restores the opening theme and variant. Windows
appearance changes use the same controller without requiring Preferences to be
open. Shutdown removes the appearance filter before releasing CSS providers.
The shipping GTK3 path remains unchanged.

### GTK4 Stage 9 Production Package Cutover, Pass 1

Date: 2026-07-23

Boundary converted: the normal Fabulor product identity, feature tree, MSI,
and bootstrapper now consume the validated GTK4 frontend and runtime payload.
The side-by-side candidate and an explicit GTK3 rollback profile remain
available but are not the default production artifact.

Automated evidence:

- production WiX build: pass with zero warnings and zero errors
- bootstrapper application build: pass with zero warnings and zero errors
- decompiled production MSI: `Fabulor` identity, established UpgradeCode and
  install root retained; 7,623 installed files and zero GTK3 path markers;
  repository-owned emoji flags, Noto Color Emoji font, and licenses are present
- exact nested GTK4 runtime validation: all 1,431 manifest entries and hashes
  pass; 1,432 installed entries include the generated manifest
- retained side-by-side candidate: 7,270 files; all content hashes, nine
  launcher imports, 31 frontend imports, ten extension modules, one extension
  data file, fifteen import edges, 5,821 plugin-host files, and .NET 8.0.29
  pass exact validation
- explicit `LegacyGtk3Frontend=true` rollback MSI: builds with zero warnings
  and zero errors; bootstrapper publication disabled
- complete `tools/gtk4` Python validation: 54 tests pass
- theme-contract validation: four tests pass
- production MSI: 111,815,012 bytes; SHA-256
  `AF827B31B0B32C0FA194CF5DABA768E462AFD8BEEAF328E5817119E136C95B4B`
- production bootstrapper: 112,110,362 bytes; SHA-256
  `059FCB631B76115C4F3673065007856EC8195177434F562CD293D4F1AD2DF945`

Behavior contract: the default installer profile must package the GTK4
launcher/frontend pair, mandatory allowlisted GTK4 runtime, rebuilt native
extensions and Enchant provider, and staged C#/Python/Tcl hosts. It must retain
the normal product upgrade identity and user-selectable plugin features while
excluding GTK3 binaries, data, and translation catalogs. GTK3 packaging is
permitted only when `LegacyGtk3Frontend=true` is supplied explicitly.

Manual upgrade, repair, uninstall, portable-mode, accessibility, visual, and
performance checks remain assigned to the final validation stage. Promoting
GTK4 to the sole MSVC and CI frontend build profile is the next contained
target.

### GTK4 Stage 9 Sole Build Profile, Pass 2

Date: 2026-07-23

Boundary converted: the supported Visual Studio solution, Windows CI workflow,
and WiX project now expose one GTK4 production frontend and installer path. The
temporary candidate and GTK3 rollback products are removed.

Automated evidence:

- GTK4-only Visual Studio solution: build completes; all 18 native manifest
  tests pass; an explicit `FabulorGtkMajor=3` build fails at profile validation
- production support staging: exactly 12 allowlisted files with SHA-256 hashes;
  nonempty outputs, duplicate destinations, ambiguous globs, and reparse-point
  source roots are rejected
- native extension graph: ten modules, one data file, and fifteen owned import
  edges validated
- launcher/frontend boundary: nine launcher imports, 31 frontend imports, and
  exported `fabulor_frontend_main` validated
- plugin-host staging: 5,821 files and .NET 8.0.29 validated
- production WiX and bootstrapper build: zero warnings and zero errors
- decompiled production MSI: 7,623 installed files, zero GTK3 path markers,
  and the established Fabulor product identity retained
- exact GTK4 runtime validation: all 1,431 manifest entries and hashes pass;
  1,432 installed runtime entries include the generated manifest
- repository validation: 55 GTK4 tooling tests, 16 Python plugin tests, four
  theme tests, and 18 native manifest tests pass
- production MSI: 111,794,532 bytes; SHA-256
  `90B37496A23BF7EBEB7A8AB90905065DD87B4BB4D3440723E8D3091CD39321E0`
- production bootstrapper: 112,091,878 bytes; SHA-256
  `35BCFBD7211F172A13BB562A44EBAC14FBBA2D4CFCEA9BD90B65D8CE80FCF30E`

Behavior contract: Windows CI must produce only `Fabulor.msi` and
`FabulorSetup.exe`; MSVC must reject non-GTK4 frontend selection; WiX must have
one unconditional production graph; and non-GTK support files must enter the
package only through the deterministic production-support contract. The
transitional GTK3-named dependency bundle remains solely as a source of
non-GTK compile dependencies and is assigned to the next cleanup pass.

### GTK4 Stage 9 Legacy Build Inputs, Pass 3

Date: 2026-07-23

Boundary converted: Windows builds no longer download or stage the GTK3
gvsbuild archive, MSYS2 hicolor/libarchive packages, LuaJIT, Perl, or gendef.
The retired broad copy project and Inno installer are deleted. The supported
dependency graph now uses the pinned GTK4 root for GTK/GLib/XML/image/gettext
inputs and a pinned vcpkg manifest containing only OpenSSL; the production
workflow verifies the dated Mozilla CA bundle before staging it.

Automated evidence:

- full GTK4 Visual Studio solution: build and link pass; all 18 native manifest
  tests pass and the unsupported Lua project is absent
- supported native extensions: checksum, exec, FiSHLiM, Python, sysinfo,
  updater, and WinRT notifications build and link; Lua is absent
- native manifest policy suite: all 18 tests pass during the solution build
- production WiX profile tests: eight tests pass, including pinned OpenSSL and
  absence of the retired projects/properties
- production support staging tests: five tests pass
- theme contract: active WiX association and import/persistence validation no
  longer depends on the deleted Inno template
- complete GTK4 Python tooling suite: all 57 tests pass
- repository diff whitespace validation: pass

Behaviour contract: no supported Windows target may depend on the GTK3 archive,
MSYS2 runtime augmentation, LuaJIT, Perl, gendef, the broad runtime-copy project,
or Inno Setup. Historical Lua/Perl and GTK3 theme source may remain in the tree,
but it is outside the supported build and package. GTK3 source branches and
compatibility helpers remain assigned to later Stage 9 cleanup passes.

### GTK4 Stage 9 HCT Archive Ownership, Pass 4

Date: 2026-07-23

Boundary converted: supported `.hct` palette/event import no longer calls into
the GTK3 theme service or its extract-to-directory helper. A dedicated common
reader now lists and streams only `colors.conf` or `pevents.conf` through the
absolute system archive executable. Windows resolves that directory through
the system API rather than a mutable environment variable.

Automated evidence:

- full GTK4 common/frontend/launcher build and link: pass
- native validation executable: all 22 tests pass
- theme contract suite: all 5 tests pass
- GTK4 tooling contract suite: all 57 tests pass
- real ZIP-form HCT fixture read: pass
- duplicate matching archive entries: rejected
- text expanding beyond 1 MiB: terminated and rejected
- non-allowlisted archive filename: rejected before filesystem access
- project XML parse and repository whitespace validation: pass

Behaviour contract: `.hct` input must be an absolute regular file no larger
than 16 MiB. Archive listings and selected text are independently capped at
1 MiB; matching entry paths must be relative, free of control characters,
backslashes, colons, and parent components, and no deeper than eight components.
The reader must not search PATH, interpolate a command string, or extract an
archive tree. Legacy GTK3 desktop-theme archive import remains assigned to the
next removal pass.

### GTK4 Stage 9 GTK3 Theme Retirement, Pass 5

Date: 2026-07-23

Boundary retired: the GTK3 desktop-theme discovery/import service, frontend
adapter, GTK3 preference branch, dedicated tests/stubs, and strict-probe
compatibility check are deleted. MSVC and Meson source lists no longer compile
the service or adapter. `gui_gtk3_theme` and `gui_gtk3_variant` are no longer
members of the saved preference contract; older keys are ignored on load.

Automated evidence:

- complete GTK4 common/frontend/launcher build and link: pass
- strict GTK4 MSVC probe build and execution: pass
- native manifest/path/HCT suite: all 22 tests pass
- theme retirement/import contract suite: all 7 tests pass
- GTK4 tooling contract suite: all 57 tests pass
- repository scan: no GTK3 theme service, adapter, test, call-site, or config
  symbol remains under active source/build/probe roots
- project XML parse and repository whitespace validation: pass

Behaviour contract: GTK4 desktop CSS remains owned by discovery, controller,
provider, Preferences, and appearance-monitor components. `.hct` palette/event
imports remain owned by the bounded archive reader. Neither workflow may
restore the retired `%APPDATA%\Fabulor\gtk3-themes` service, adapter, or saved
keys. Historical extraction findings remain in the security audit as evidence,
but their vulnerable implementation is no longer present.

### GTK4 Stage 9 Compatibility Header Specialization, Pass 6

Date: 2026-07-23

Boundary retired: `src/fe-gtk/gtk-compat.h` no longer supports two toolkit
majors. Its stable helper API now contains only the selected GTK4
implementations; 898 lines of GTK3 branches and version directives are
removed.

Automated evidence:

- strict GTK4 MSVC probe build and execution: pass with `/W4 /WX`, zero
  warnings and zero errors; GTK 4.22.4 / GLib 2.88.0 / x64 runtime confirmed
- complete GTK4 frontend project build and link: pass with zero errors and the
  unchanged 81-warning inventory
- production profile contract suite: all 9 tests pass, including rejection of
  restored toolkit-version switches and representative GTK3-only APIs
- source audit: no `GTK_MAJOR_VERSION`, GTK3 event type, container/bin,
  selection, synchronous-dialog, widget-destruction, or recursive-show API
  remains in the compatibility header
- repository whitespace validation: pass

The full orchestration's common-library prebuild was not rerun locally because
its public-suffix download is blocked by the execution sandbox; the unchanged
common library and complete frontend link validated the affected boundary.
GitHub's clean-environment build remains the final orchestration check.

Behaviour contract: callers retain the reviewed helper API and GTK4 behavior,
but no supported build can select a GTK3 implementation from the compatibility
header. Direct GTK3 branches in individual source files and legacy,
non-configurable Meson fragments remain separate Stage 9 cleanup targets.

### GTK4 Stage 9 Operational List Source Specialization, Pass 7

Date: 2026-07-23

Boundary retired: 22 converted operational list, model, and view sources no
longer carry parallel GTK3 implementations. A total of 3,707 inactive GTK3 and
toolkit-selection lines are removed. The frontend-wide
`GTK_MAJOR_VERSION`-conditional inventory falls from 564 to 290.

Automated evidence:

- strict GTK4 MSVC probe build and execution: pass with `/W4 /WX`, zero
  warnings and zero errors; GTK 4.22.4 / GLib 2.88.0 / x64 runtime confirmed
- complete GTK4 frontend project build and link: pass with zero errors; the
  affected incremental build reports 7 existing warnings
- production profile contract suite: all 10 tests pass, including all 22
  specialized files and rejection of restored toolkit-version switches or
  classic GTK3 tree/list APIs
- complete GTK4 tooling contract suite: all 59 tests pass
- theme retirement/import contract suite: all 7 tests pass
- native manifest, path, and archive suite: all 22 tests pass
- source audit: no `GTK_MAJOR_VERSION`, `GtkTreeView`, `GtkListStore`,
  `GtkTreeStore`, `GtkCellRenderer`, `GtkTreeSelection`, or matching classic
  function family remains in the specialized source set
- repository whitespace validation: pass

Behaviour contract: operational list behavior continues through the reviewed
GTK4 models, selection owners, factories, list/column views, tree-list rows,
and tree expanders. No supported build can select the retired GTK3
implementation from these files. The remaining frontend source branches and
legacy, non-configurable Meson fragments remain separate Stage 9 cleanup
targets.

### GTK4 Stage 9 Theme Source Specialization, Pass 8

Date: 2026-07-23

Boundary retired: nine GTK4 theme controller, CSS provider, palette,
appearance-monitor, Preferences staging, and window-lifecycle files no longer
carry parallel GTK3 implementations. A total of 205 inactive GTK3 and
toolkit-selection lines are removed. The frontend-wide
`GTK_MAJOR_VERSION`-conditional inventory falls from 290 to 253.

Automated evidence:

- strict GTK4 MSVC probe build and execution: pass with `/W4 /WX`, zero
  warnings and zero errors; GTK 4.22.4 / GLib 2.88.0 / x64 runtime confirmed
- complete GTK4 frontend project build and link: pass with zero errors; the
  affected build reports only the existing frontend warning inventory
- production profile contract suite: all 11 tests pass, including all nine
  specialized files and rejection of restored toolkit-version switches or
  representative GTK3 theme APIs
- complete GTK4 tooling contract suite: all 60 tests pass
- theme retirement/import contract suite: all 7 tests pass
- native manifest, path, and archive suite: all 22 tests pass
- source audit: no `GTK_MAJOR_VERSION`, screen-scoped CSS provider,
  style-context palette, legacy CSS loading, or widget-destruction API remains
  in the specialized source set
- repository whitespace validation: pass

Behaviour contract: theme discovery, application, Preferences staging,
appearance refresh, CSS-provider ownership, and top-level finalization continue
through the reviewed GTK4 implementations. No supported build can select the
retired GTK3 theme integration from these files. Remaining frontend source
branches and legacy, non-configurable Meson fragments remain separate Stage 9
cleanup targets.

### GTK4 Stage 9 Window/File Helper Specialization, Pass 9

Date: 2026-07-23

Boundary retired: `window-state.c`, `window-geometry.c`, and
`file-chooser-path.c` no longer carry parallel GTK3 implementations. A total of
139 inactive GTK3 and toolkit-selection lines are removed. The frontend-wide
`GTK_MAJOR_VERSION`-conditional inventory falls from 253 to 233.

Automated evidence:

- strict GTK4 MSVC probe build and execution: pass with `/W4 /WX`, zero
  warnings and zero errors; GTK 4.22.4 / GLib 2.88.0 / x64 runtime confirmed
- complete GTK4 frontend project build and link: pass with zero warnings and
  zero errors in the affected incremental build
- production profile contract suite: all 12 tests pass, including all three
  specialized files and rejection of restored toolkit-version switches or
  representative GTK3 window/file APIs
- complete GTK4 tooling contract suite: all 61 tests pass
- theme retirement/import contract suite: all 7 tests pass
- native manifest, path, and archive suite: all 22 tests pass
- source audit: no `GTK_MAJOR_VERSION`, GTK3 window/configure event, window
  position/state, native `GdkWindow`, or retired file-chooser API remains in
  the specialized source set
- repository whitespace validation: pass

Behaviour contract: state and geometry callbacks continue through GTK4
surface/toplevel observation, native Windows handles use `GdkWin32Surface`,
and file chooser paths remain owned local `GFile` projections. The GTK 4.10
chooser deprecation suppression remains local to `file-chooser-path.c` pending
the asynchronous `GtkFileDialog` replacement. Remaining frontend source
branches and legacy, non-configurable Meson fragments remain separate Stage 9
cleanup targets.

### GTK4 Stage 9 Spell-Input Source Specialization, Pass 10

Date: 2026-07-23

Boundary retired: `sexy-spell-entry.c`, `spell-entry-widget.c`, and
`emoji-picker.c` no longer carry parallel GTK3 implementations. A total of 303
inactive GTK3 and toolkit-selection lines are removed. The frontend-wide
`GTK_MAJOR_VERSION`-conditional inventory falls from 233 to 218.

Automated evidence:

- strict GTK4 MSVC probe build and execution: pass with `/W4 /WX`, zero
  warnings and zero errors; GTK 4.22.4 / GLib 2.88.0 / x64 runtime confirmed
- complete GTK4 frontend project build and link: pass with zero warnings and
  zero errors in the affected incremental build
- production profile contract suite: all 13 tests pass, including all three
  specialized files and rejection of restored toolkit-version switches or
  representative GTK3 spell/menu APIs
- complete GTK4 tooling contract suite: all 62 tests pass
- theme retirement/import contract suite: all 7 tests pass
- native manifest, path, and archive suite: all 22 tests pass
- source audit: no `GTK_MAJOR_VERSION`, GTK3 widget-menu, entry-layout,
  CSS-data loading, container attachment, recursive reveal, or widget
  destruction API remains in the specialized source set
- repository whitespace validation: pass

Behaviour contract: spell checking, personal dictionaries, language choices,
suggestion replacement, checked-state actions, caret styling, pointer
positioning, and emoji insertion continue through the reviewed GTK4
implementations. No supported build can select the retired GTK3 spell-input
path from these files. Remaining main edit-box and frontend source branches
remain separate Stage 9 cleanup targets.

### GTK4 Stage 9 Transcript-Helper Source Specialization, Pass 11

Date: 2026-07-24

Boundary retired: eight transcript selection, render-target, widget-class,
accessibility, geometry, and header files no longer carry parallel GTK3
implementations. A total of 274 inactive GTK3 and toolkit-selection lines are
removed. The frontend-wide `GTK_MAJOR_VERSION`-conditional inventory falls
from 218 to 197.

Automated evidence:

- strict GTK4 MSVC probe build and execution: pass with `/W4 /WX`, zero
  warnings and zero errors; GTK 4.22.4 / GLib 2.88.0 / x64 runtime confirmed
- complete GTK4 frontend project build and link: pass with zero errors; its
  affected rebuild reports 16 existing warnings from remaining caller branches
- production profile contract suite: all 14 tests pass, including all eight
  specialized files and rejection of restored toolkit-version switches or
  representative GTK3 transcript APIs
- complete GTK4 tooling contract suite: all 63 tests pass
- theme retirement/import contract suite: all 7 tests pass
- native manifest, path, and archive suite: all 22 tests pass
- source audit: no `GTK_MAJOR_VERSION`, GTK3 selection/clipboard, `GdkWindow`
  render context, draw/preferred-size, allocation-query, or ATK naming API
  remains in the specialized source set
- repository whitespace validation: pass

Behaviour contract: primary selection, snapshot rendering, explicit Cairo
targets, widget measurement, accessible text updates, and geometry continue
through the reviewed GTK4 implementations. No supported build can select the
retired GTK3 transcript-helper path. The main `xtext.c` renderer and remaining
frontend source branches remain separate Stage 9 cleanup targets.

### GTK4 Stage 9 Transcript Renderer Source Specialization, Pass 12

Date: 2026-07-24

Boundary retired: `xtext.c` no longer carries its parallel GTK3 renderer,
window, pointer, scrolling, and lifecycle implementation. A total of 328
inactive GTK3 and toolkit-selection lines are removed. The frontend-wide
`GTK_MAJOR_VERSION`-conditional inventory falls from 197 to 168.

Automated evidence:

- complete GTK4 frontend project build and link: pass with zero warnings and
  zero errors in the affected incremental build
- strict GTK4 MSVC transcript-policy probe build and execution: pass with
  `/W4 /WX`, zero warnings and zero errors; GTK 4.22.4 / GLib 2.88.0 / x64
  runtime confirmed
- production profile contract suite: all 15 tests pass, including rejection of
  restored renderer version switches or representative GTK3
  window/event/rendering APIs
- complete GTK4 tooling contract suite: all 64 tests pass
- theme retirement/import contract suite: all 7 tests pass
- native manifest, path, and archive suite: all 22 tests pass
- source audit: no `GTK_MAJOR_VERSION`, GTK3 window/device, direct Cairo window
  source, widget window/allocation mutation, widget grab, style-updated, or
  native scroll-copy API remains in `xtext.c`
- repository whitespace validation: pass

Behaviour contract: transcript realization, snapshot rendering, pointer and
cursor state, selection interaction, full-redraw fallback, scrolling,
accessible text, and buffer-change notifications continue through the reviewed
GTK4 implementation and extracted policy helpers. No supported build can select
the retired GTK3 transcript renderer. Remaining main UI, menu, platform, and
dialog source branches remain separate Stage 9 cleanup targets.

### GTK4 Stage 9 Tray Source Specialization, Pass 13

Date: 2026-07-24

Boundary retired: `plugin-tray.c` and the GTK4 tray presenter no longer carry
parallel GTK3 integration. A total of 755 inactive GTK3 tray-source lines and
20 obsolete AppIndicator Meson dependency lines are removed. The frontend-wide
`GTK_MAJOR_VERSION`-conditional inventory falls from 168 to 159.

Automated evidence:

- strict GTK4 MSVC tray-policy/presenter probe build and execution: pass with
  `/W4 /WX`, zero warnings and zero errors; GTK 4.22.4 / GLib 2.88.0 / x64
  runtime confirmed
- complete GTK4 frontend project build and link: pass with zero warnings and
  zero errors in the affected incremental build
- production profile contract suite: all 16 tests pass, including rejection of
  restored toolkit-version switches, AppIndicator/StatusIcon APIs, GTK3 widget
  menus, and Meson AppIndicator dependencies
- complete GTK4 tooling contract suite: all 65 tests pass
- theme retirement/import contract suite: all 7 tests pass
- native manifest, path, and archive suite: all 22 tests pass
- source audit: no `GTK_MAJOR_VERSION`, `GtkStatusIcon`, AppIndicator,
  `GtkCheckMenuItem`, GTK3 menu-shell/item, or legacy backend macro remains in
  the tray source set
- repository whitespace validation: pass

Behaviour contract: tray commands continue through the reviewed GTK4
action/menu model, window-state observer, presenter, and retained native Windows
menu projection. No GTK4 build may load or compile against the GTK3
AppIndicator or `GtkStatusIcon` ABI. Non-Windows builds report no usable tray
backend until a GTK4-native StatusNotifier implementation is added. Remaining
application, main UI, menu, and dialog source branches remain separate Stage 9
cleanup targets.

### GTK4 Stage 9 Application-Lifecycle Source Specialization, Pass 14

Date: 2026-07-24

Boundary retired: `fe-gtk.c` no longer carries parallel GTK3 initialization,
option parsing, icon validation, main-loop, or quit implementations. A total of
41 inactive GTK3 and toolkit-selection lines are removed. The frontend-wide
`GTK_MAJOR_VERSION`-conditional inventory falls from 159 to 150.

Automated evidence:

- strict GTK4 MSVC application-main-loop probe build and execution: pass with
  `/W4 /WX`, zero warnings and zero errors; GTK 4.22.4 / GLib 2.88.0 / x64
  runtime confirmed
- complete GTK4 frontend project build and link: pass with zero warnings and
  zero errors in the affected incremental build
- production profile contract suite: all 17 tests pass, including rejection of
  restored toolkit-version switches, GTK3 option groups, argument-taking
  initialization, or legacy main-loop APIs
- complete GTK4 tooling contract suite: all 66 tests pass
- theme retirement/import contract suite: all 7 tests pass
- native manifest, path, and archive suite: all 22 tests pass
- source audit: no `GTK_MAJOR_VERSION`, `gtk_get_option_group`, `gtk_main`,
  `gtk_main_quit`, or GTK3 `gtk_init` call remains in `fe-gtk.c`
- repository whitespace validation: pass

Behaviour contract: command-line handling, GTK initialization, Windows icon
theme discovery, frontend run-loop ownership, and quit dispatch continue
through the reviewed GTK4 implementation and `FabulorApplicationMainLoop`.
No supported build can select the retired GTK3 lifecycle path. Remaining main
UI, menu, server-list, and dialog source branches remain separate Stage 9
cleanup targets.

### GTK4 Stage 9 Server List Source Specialization, Pass 15

Date: 2026-07-24

Boundary retired: `servlistgui.c` no longer carries parallel GTK3
certificate-dialog parent, editor/window close, destroy, and finalization
implementations. A total of 84 inactive GTK3 and toolkit-selection lines are
removed. The frontend-wide `GTK_MAJOR_VERSION`-conditional inventory falls
from 150 to 135.

Automated evidence:

- complete GTK4 frontend project build and link: pass with zero warnings and
  zero errors in the affected incremental build
- strict GTK4 MSVC list/lifecycle probe build and execution: pass with
  `/W4 /WX`, zero warnings and zero errors; GTK 4.22.4 / GLib 2.88.0 / x64
  runtime confirmed
- production profile contract suite: all 18 tests pass, including rejection of
  restored toolkit-version switches, GTK3 delete/destroy callbacks,
  `GdkEventAny`, or parent signal-ID ownership
- complete GTK4 tooling contract suite: all 67 tests pass
- theme retirement/import contract suite: all 7 tests pass
- native manifest, path, and archive suite: all 22 tests pass
- source audit: no `GTK_MAJOR_VERSION`, `delete-event`, `GdkEventAny`, legacy
  parent destroy handler, editor destroy callback, or Server List destroy
  callback remains in `servlistgui.c`
- repository whitespace validation: pass

Behaviour contract: Server List and network-editor save/close behavior,
geometry persistence, model release, certificate chooser parenting, and
top-level pointer cleanup continue through typed GTK4 close requests and weak
finalization. No supported build can select the retired GTK3 lifecycle path.
Remaining main UI, menu, setup, and dialog source branches remain separate
Stage 9 cleanup targets.

### GTK4 Stage 9 Channel/Ban List Source Specialization, Pass 16

Date: 2026-07-24

Boundary retired: `chanlist.c` and `banlist.c` no longer carry parallel GTK3
context-menu construction, presentation, copy, and cleanup implementations. A
total of 144 inactive GTK3 and toolkit-selection lines are removed. The
frontend-wide `GTK_MAJOR_VERSION`-conditional inventory falls from 135 to 127.

Automated evidence:

- strict GTK4 MSVC list/context-menu probe build and execution: pass with
  `/W4 /WX`, zero warnings and zero errors; GTK 4.22.4 / GLib 2.88.0 / x64
  runtime confirmed
- complete GTK4 frontend project build and link: pass with zero warnings and
  zero errors in the affected incremental build
- production profile contract suite: all 19 tests pass, including rejection of
  restored toolkit-version switches, GTK3 widget menus, menu-shell/item
  ownership, box packing, recursive reveal, or widget destruction
- complete GTK4 tooling contract suite: all 68 tests pass
- theme retirement/import contract suite: all 7 tests pass
- native manifest, path, and archive suite: all 22 tests pass
- source audit: no `GTK_MAJOR_VERSION`, `gtk_menu_*`, `gtk_menu_shell_*`,
  `gtk_box_pack_start`, `gtk_container_add`, `gtk_widget_show_all`, or
  `gtk_widget_destroy` remains in the specialized dialog sources
- repository whitespace validation: pass

Behaviour contract: Channel List join/copy/autojoin commands continue through
the reviewed GTK4 context-menu action/model presenter, and Ban List copy
commands continue through explicit popover ownership and cleanup. No supported
build can select the retired GTK3 widget-menu path. Remaining main UI, menu,
setup, join-dialog, and channel-view source branches remain separate Stage 9
cleanup targets.

### GTK4 Stage 9 Preferences/Join Source Specialization, Pass 17

Date: 2026-07-24

Boundary retired: `setup.c` and `joind.c` no longer carry parallel GTK3
destroy-signal callbacks or Preferences viewport inspection. A total of 41
inactive GTK3 and toolkit-selection lines are removed. The frontend-wide
`GTK_MAJOR_VERSION`-conditional inventory falls from 127 to 119.

Automated evidence:

- strict GTK4 MSVC probe build and execution: pass with `/W4 /WX`, zero
  warnings and zero errors; GTK 4.22.4 / GLib 2.88.0 / x64 runtime confirmed
- complete GTK4 frontend project build and link: pass with zero warnings and
  zero errors in the affected incremental build
- production profile contract suite: all 20 tests pass, including rejection of
  restored toolkit-version switches, GTK3 destroy callbacks, `GtkBin` child
  inspection, or viewport shadow mutation
- complete GTK4 tooling contract suite: all 69 tests pass
- theme retirement/import contract suite: all 7 tests pass
- native manifest, path, and archive suite: all 22 tests pass
- source audit: no `GTK_MAJOR_VERSION`, `GTK_BIN`, `gtk_bin_get_child`,
  `gtk_viewport_set_shadow_type`, `joind_destroy_cb`, or `setup_close_cb`
  remains in the specialized sources
- repository whitespace validation: pass

Behaviour contract: Preferences, its font chooser, and the Join dialog retain
their reviewed GTK4 weak-finalization cleanup. Preferences pages retain
explicit scroller child ownership without depending on GTK3 internal viewport
construction. No supported build can select the retired GTK3 lifecycle path.
Remaining main UI, menu, channel-view, and small helper source branches remain
separate Stage 9 cleanup targets.

### GTK4 Stage 9 Channel View/Helper Source Specialization, Pass 18

Date: 2026-07-24

Boundary retired: Channel View finalization, system-icon loading, tray-support
policy, the Alt modifier alias, and the GTK4 context-menu presenter no longer
carry toolkit-version branches. Three unreferenced generic GTK3 tree-view
helpers and their declarations are removed. A total of 163 inactive or dead
source lines are removed, and four residual GTK4 call/initializer signatures
are corrected. The frontend-wide `GTK_MAJOR_VERSION`-conditional inventory
falls from 119 to 113.

Automated evidence:

- strict GTK4 MSVC probe build and execution: pass with `/W4 /WX`, zero
  warnings and zero errors; GTK 4.22.4 / GLib 2.88.0 / x64 runtime confirmed
- clean GTK4 frontend project rebuild and link: pass with zero errors; all 14
  remaining warnings are confined to `maingui.c` and `menu.c`
- affected source units compile and link with zero warnings and zero errors
- production profile contract suite: all 21 tests pass, including rejection of
  restored toolkit-version switches, GTK3 icon/tray APIs, tree-view helpers,
  destroy callbacks, old modifier masks, or GTK3 call signatures
- complete GTK4 tooling contract suite: all 70 tests pass
- theme retirement/import contract suite: all 7 tests pass
- native manifest, path, and archive suite: all 22 tests pass
- source audit: the remaining 113 `GTK_MAJOR_VERSION` conditionals are confined
  to `maingui.c`, `menu.c`, `maingui.h`, and `menu.h`
- repository whitespace validation: pass

Behaviour contract: Channel View retains its model, listener, and implementation
cleanup through GTK4 weak finalization. System icons retain theme lookup and
scaled stream loading through `GtkIconPaintable`. Tray availability continues
to report false until a genuine GTK4 backend exists. Palette values and editor
presentation are unchanged by their type/signature corrections. The main
window and menu sources are the only remaining source-specialization targets.

### GTK4 Stage 9 Main-Window Source Specialization, Pass 19

Date: 2026-07-24

Boundary retired: `maingui.c` and `maingui.h` no longer carry parallel GTK3
main/tab window, context-menu, layout, entry, emoji, lifecycle, drag-and-drop,
or Windows event-filter implementations. A total of 801 inactive GTK3 and
toolkit-selection lines are removed; nine retained GTK4 declarations, calls,
and Windows guards are made exact. The frontend-wide `GTK_MAJOR_VERSION`
conditional inventory falls from 113 to 56.

Automated evidence:

- strict GTK4 MSVC probe build and execution: pass with `/W4 /WX`, zero
  warnings and zero errors; GTK 4.22.4 / GLib 2.88.0 / x64 runtime confirmed
- clean GTK4 frontend project rebuild and link: pass with zero errors;
  `maingui.c` is warning-free and all seven remaining warnings are confined to
  `menu.c`
- production profile contract suite: all 22 tests pass, including rejection of
  restored toolkit-version switches, GTK3 widget menus, accelerator groups,
  GDK window/event paths, drag destinations, container inspection, or legacy
  main-window helper declarations
- complete GTK4 tooling contract suite: all 71 tests pass
- theme retirement/import contract suite: all 7 tests pass
- native manifest, path, and archive suite: all 22 tests pass
- source audit: the remaining 56 `GTK_MAJOR_VERSION` conditionals are confined
  to `menu.c` and `menu.h`
- repository whitespace validation: pass

Behaviour contract: main and tab windows retain the reviewed GTK4 construction,
layout, action-model context menus, weak finalization, close requests, pane
restoration, input/emoji behavior, typed drag-and-drop, and native Windows
display filtering. No supported build can select the retired GTK3 paths.
`menu.c` and `menu.h` are the sole remaining source-specialization target.

### GTK4 Stage 9 Menu Source Specialization, Pass 20

Date: 2026-07-24

Boundary retired: `menu.c` and `menu.h` no longer carry parallel GTK3 main,
context, plugin, Usermenu, check/radio, accelerator, popup, or lifecycle
implementations. Two orphaned GTK3 Usermenu widget helpers are also removed. A
total of 1,665 inactive or dead source lines are removed. The frontend-wide
`GTK_MAJOR_VERSION` conditional inventory falls from 56 to zero.

Automated evidence:

- strict GTK4 MSVC probe build and execution: pass with `/W4 /WX`, zero
  warnings and zero errors; GTK 4.22.4 / GLib 2.88.0 / x64 runtime confirmed
- clean GTK4 frontend project rebuild and link: pass with zero warnings and
  zero errors
- production profile contract suite: all 23 tests pass, including rejection of
  restored toolkit-version switches across every frontend C/header source and
  rejection of GTK3 widget-menu, check/radio item, accelerator, popup,
  container, and destruction APIs in the menu boundary
- complete GTK4 tooling contract suite: all 72 tests pass
- theme retirement/import contract suite: all 7 tests pass
- native manifest, path, and archive suite: all 22 tests pass
- source audit: zero `GTK_MAJOR_VERSION` conditionals remain under
  `src/fe-gtk`
- repository whitespace validation: pass

Behaviour contract: main and contextual menus retain canonical GTK4 actions,
state and sensitivity synchronization, retained `GMenuModel` composition,
owned popover presenters, plugin overlays, and recursive Usermenu projection.
No supported source path can select or expose the retired GTK3 widget-menu
implementation. GTK3 source retirement is complete; final runtime and
packaging validation remains tracked separately.

### GTK4 Stage 9 Production Artifact Validation, Pass 21

Date: 2026-07-24

Boundary hardened: the separately published production MSI and Burn
bootstrapper are now validated as one release pair. The validator extracts the
bundle and rejects changes to the established bundle or MSI upgrade identity,
project version, per-machine scope, single-package chain, embedded payload
relationship, or exact bootstrapper application file set. The extracted
`Fabulor.msi` must match the separately published MSI in size and SHA-256.

Automated evidence:

- production WiX MSI and bootstrapper rebuild: pass with zero warnings and
  zero errors
- production MSI identity/payload validation: 7,623 installed files, all
  required features and paths present, zero GTK3 path markers
- exact GTK4 runtime validation: all 1,431 manifest entries plus the packaged
  manifest pass installed-path, size, and SHA-256 comparison
- production bundle validation: version `1.0.3`, one embedded MSI chain,
  established bundle/MSI upgrade codes, exact ten-file bootstrapper
  application payload, and byte-for-byte published/embedded MSI equality
- bundle validator unit suite: all 6 tests pass, covering extraction arguments,
  valid identity, version ownership, upgrade-code drift, unexpected
  bootstrapper files, and mismatched MSI content
- complete GTK4 tooling contract suite: all 78 tests pass
- theme retirement/import contract suite: all 7 tests pass
- Python capability/isolation suites: all 16 tests pass
- native manifest, path, and archive suite: all 22 tests pass
- production MSI: 111,786,340 bytes; SHA-256
  `CADF5CCE03650AE31EF089DA7F4CD6995EB653D34FDF25E8E9C3CBCC2C243061`
- production bootstrapper: 112,084,516 bytes; SHA-256
  `49376D82009D1D902EAE35E6560F6282314BF0E95E2D78CC7E0CCEFBF3A90301`
- repository whitespace validation: pass

Behaviour contract: CI cannot publish a bootstrapper whose version, upgrade
identity, chain, application payload, or embedded MSI differs from the
reviewed production contract or the accompanying MSI artifact. This closes
the automated production packaging boundary. Clean-install, in-place upgrade,
repair, uninstall, accessibility, visual, performance, and live plugin
acceptance remain open and require controlled installed-client testing.

### GTK4 Stage 9 Clean-Install Layout Acceptance, Pass 22

Date: 2026-07-25

Defects observed: the first clean program installation reused a retained
profile whose right-pane width was `2`, below its configured `80` minimum, so
the user list was effectively absent. Pressing the transcript's draggable
nick/text separator then caused Windows Application Error 1000 with exception
`c0000005` in the packaged `cairo-2.dll`. The next connection attempt showed
that Server List checkboxes changed visually but did not persist, and password
visibility never activated.

Root cause and correction:

- right-pane restoration trusted the captured saved width after initial GTK4
  allocation; it now clamps the end-child size against the configured minimum,
  available pane allocation, and handle width
- transcript separator input called the Cairo drawing path outside a GTK4
  snapshot, where the reviewed render target correctly returned no context;
  all immediate transcript drawing now uses one guarded acquisition boundary
  that queues a widget redraw and returns when no context is active
- production source guards require the centralized transcript draw boundary
  and the pane-size clamp to remain in the main-window restore path
- Server List check controls were GTK4 `GtkCheckButton` objects accessed
  through incompatible `GtkToggleButton` casts; all network flags, keyring,
  password visibility, startup skipping, and favorites controls now use the
  typed GTK4 check-button getter/setter, with a source guard rejecting the
  retired accessors
- installed-client follow-up confirmed connection and Server List state, then
  showed that the GTK4 trailing-box adapter expanded the nickname button and
  displaced the input field; the adapter now aligns the containing box and
  leaves its trailing controls at natural width
- the visible user-list container now owns its configured minimum width, and
  its position callback ignores hidden or unallocated layout state rather than
  persisting a transient zero
- emoji pages now expand inside a viewport calculated from the main-window
  allocation, bounded from `320 x 240` to `640 x 420`; the current
  `668 x 429` window receives a `620 x 289` picker instead of a fixed
  `500 x 330` page plus unconstrained tab chrome

Automated evidence:

- strict GTK4 MSVC probe rebuild and execution: pass with `/W4 /WX`; GTK
  4.22.4 / GLib 2.88.0 / x64 confirmed
- pane clamp probe: stale `2`-pixel state normalizes to `80`; valid `120`
  remains `120`; oversized state clamps to available width; no available space
  returns zero
- emoji viewport probe: compact, current-window, and maximum allocation cases
  resolve to `320 x 240`, `620 x 289`, and `640 x 420`
- clean full GTK4 frontend rebuild and link: zero warnings and zero errors
- native extension rebuild and import validation: ten modules, one data file,
  and 15 owned import edges pass
- complete GTK4 tooling contract suite: all 78 tests pass
- theme retirement/import contract suite: all 7 tests pass
- Python capability/isolation suites: all 16 tests pass
- native manifest, path, and archive suite: all 22 tests pass
- production WiX MSI and bootstrapper rebuild: zero warnings and zero errors
- production MSI validation: 7,623 installed files, zero GTK3 path markers,
  all 1,431 runtime entries and packaged manifest hashes verified
- production bundle validation: version `1.0.3`, one embedded MSI chain, and
  byte-for-byte embedded/published MSI equality
- production MSI `1.0.4`: 111,524,196 bytes; SHA-256
  `0310177CFA972B0C719E6FF0B23AA32BBC2B14071294C2D49B185FD31D40B947`
- production bootstrapper `1.0.4`: 111,825,754 bytes; SHA-256
  `DE6C99BEAB66E97B52A81FE28590441EADA2536CDA35EA7687E715907BA3207D`
- repository whitespace validation: pass

Installed acceptance so far: ChatLounge connects and Server List changes now
take effect. The subsequent `1.0.3` same-version update replaced the frontend
but omitted newly registered root-level OpenSSL components, and the launcher
failed with Win32 error 126 because `libssl-3-x64.dll` and
`libcrypto-3-x64.dll` were absent. The MSI contained those components, but its
reused product identity did not provide a valid component-graph upgrade. The
corrected release advances the shared client/MSI/bundle version to `1.0.4`,
forcing a WiX major upgrade, and regenerates production support from the exact
OpenSSL 3.6.3 root used to link the frontend.

Acceptance pending: upgrade the installed `1.0.3` client with the corrected
`1.0.4` bootstrapper, verify the frontend loads and both OpenSSL DLLs are
installed, confirm the user list is visible with the retained profile, drag
and release the transcript separator repeatedly, confirm the nickname control
no longer displaces the edit field, inspect and use the emoji picker at normal
and maximized sizes, then restart and confirm pane, separator, SSL,
password-visibility, and input-row behavior remain usable. Add-ons remain
excluded during this retest.

### GTK4 Stage 9 Installed-Client Visual Acceptance Follow-up, Pass 23

Date: 2026-07-25

The first running `1.0.4` screenshots confirmed the earlier user-list,
input-width, connection, and emoji viewport improvements and exposed eight
remaining acceptance defects: leaf-channel carets, topic/server-row spacing,
late initial server presentation, inactive scroll-to-bottom overlay, inactive
spell checking, obsolete GTK3 theme wording, and two incorrect Window branches
in the transcript context menu.

Corrections:

- channel rows below a server no longer publish empty child models, removing
  false expanders while retaining server-root expansion and child indentation;
  focusing the initial server row also has a single-dispatch fallback when GTK
  has not emitted its first selection notification
- inline topic mode uses zero vertical text padding, and server sessions hide
  the channel-only nickname box so the edit field starts at the content edge
- scroll-to-bottom now uses a transcript-owned operation that sets both the
  adjustment and persistent bottom-follow state before queuing a redraw
- Preferences labels the retained palette/archive surface `Fabulor Theme`
- middle-context composition merges every matching add-on path into one
  canonical submenu; the built-in Window model continues to own Ban List,
  Character Chart, Direct Chat, transfers, friends, ignore, plug-ins, raw log,
  URL grabber, transcript controls, and search
- the clean MSI's spell failure was a package split: WinSpell and ordering data
  were installed, but `libenchant-2-2.dll` was omitted. WiX now installs the
  Enchant 2.8.19 core as a required root component, and a source contract
  requires all three spell payload groups

Automated evidence:

- production profile contract suite: all 24 tests pass
- strict GTK4 MSVC probe: clean `/W4 /WX` compile, link, and execution; duplicate
  add-on submenu inputs collapse into one canonical branch
- full GTK4 frontend rebuild: zero warnings and zero errors
- complete GTK4 tooling contract suite: all 79 tests pass
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production MSI: 111,548,871 bytes; SHA-256
  `C44E07582D78ABD985E9AA3D35AA308E1A42BC9E8146D9D1DCAC2853FF0FCA90`
- production bootstrapper: 111,850,450 bytes; SHA-256
  `E41B0F4A713AEC877D834A76794B40F26AAE33953231CE6A5EEBA7EDB4479A16`
- staged Enchant core SHA-256:
  `F6B26865B1DB04ACC96F8BACA853D2CBDFA818EC0A363BCA95FC1677CD7E6EAC`
- repository whitespace validation: pass

Installed acceptance remains open for startup server presentation, the compact
topic and server input rows, leaf-channel indicators, scroll-to-bottom click,
live Enchant underlines/suggestions/personal dictionary, Preferences wording,
and the complete single Window context submenu.

### GTK4 Stage 9 Transcript Interaction Acceptance, Pass 24

Date: 2026-07-26

Installed testing found that transcript drag selection could not target a
partial line, frequently highlighted an adjacent complete line, and left both
automatic clipboard publication and `Copy Selection` without the intended
range. URL hover and activation were also absent.

Root cause and correction:

- the inherited selection stored screen coordinates and repeatedly resolved
  them through character widths that no longer matched Pango-shaped GTK4 text
- horizontal hit-testing could continue beyond the selected wrapped row
- selection now stores stable text-entry and byte-offset anchors
- Pango resolves the nearest UTF-8-safe insertion boundary using the same
  shaped run as rendering, including inline flag boundaries
- hit-testing is constrained to the active wrapped row and uses a visible-row
  index for bounds checks
- highlighting, automatic copy, explicit `Copy Selection`, and URL interaction
  now consume the same pointer-to-text mapping

Automated evidence:

- strict GTK4 MSVC probe: zero warnings and zero errors under `/W4 /WX`
- full GTK4 frontend and launcher rebuilds: zero warnings and zero errors
- complete GTK4 tooling contract suite: all 79 tests pass
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- runtime validation: all 1,431 manifest entries and content hashes verified
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production bootstrapper SHA-256:
  `E69C2DEE0F929F03C90CFFF6426B4C4369D8EC8FE91E277EAE69E414C6201A57`

Installed acceptance: pass. Precise partial-line selection works, the selected
range reaches the clipboard through release-to-copy and `Copy Selection`, and
URL hover/activation works from channel text and topics.

### GTK4 Stage 9 Startup Server-Session Acceptance, Pass 25

Date: 2026-07-26

Installed startup testing with Network List skipped and three auto-connect
entries found two related presentation defects: server transcripts remained
blank until connection completion, and each newly created server session took
focus so the final network replaced the first configured network.

Root cause and correction:

- GTK4 may automatically select the first channel-tree row during insertion,
  before the old `mg_add_chan()` order created that session's transcript
  buffer
- each auto-connect previously delegated null-session creation to
  `servlist_connect()`, which requested focus for every network
- transcript buffers and user-list models now exist before channel-tree row
  insertion, so initial selection presents the correct session buffer
- auto-connect creates each server session explicitly, focuses only the first
  configured network, and connects subsequent sessions in the background
- server rows use their configured network names before connection completion

Automated evidence:

- common core rebuild: zero warnings and zero errors
- full GTK4 frontend rebuild: zero warnings and zero errors
- launcher rebuild: zero warnings and zero errors under `/W4 /WX`
- complete GTK4 tooling contract suite: all 79 tests pass
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- runtime validation: all 1,431 manifest entries and content hashes verified
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production MSI SHA-256:
  `BFF50C83948D281C8BE616871BA269D41B080932CA09C1A91A46165A4D585ABC`
- production bootstrapper SHA-256:
  `8233B425306988E961FE10AD9E8913CF6F67B99D86CB6EED6E129D573564CD2D`

Installed acceptance: pass. Direct ChatLounge and DALnet connections display
server lookup and startup information immediately, and the first configured
network retains focus while later networks connect. An already-connected ZNC
can complete before its intermediate startup state is observable; that is
expected and does not leave the server session blank after connection.

### GTK4 Stage 9 User-List Resize-Policy Acceptance, Pass 26

Date: 2026-07-26

Installed testing found that the configured user-list pane width could still
take effect only after switching channels. Investigation also confirmed that
the historical `gui_ulist_resizable` command preference had disappeared from
Fabulor even though HexChat and older ZoiteChat exposed that policy.

Root cause and correction:

- initial right-pane restoration could finish after an early mapped allocation
  but before final window geometry remained stable
- the pane already hardcoded non-resizing behavior, but no persisted preference
  remained to make that policy explicit or selectable
- restoration now waits for a mapped and visible user list, reapplies the
  clamped saved width, and requires three stable frames before position
  notifications can persist a replacement
- `gui_ulist_resizable` is restored to the preference schema and User List
  Preferences page, defaults to fixed-width `OFF`, and controls GTK4's
  `resize-end-child` policy
- `gui_pane_right_size` owns the complete user-list pane;
  `gui_ulist_nick_width` remains the nickname-column width within that pane

Automated evidence:

- common core rebuild: zero warnings and zero errors
- full GTK4 frontend rebuild: zero warnings and zero errors
- complete GTK4 tooling contract suite: all 79 tests pass
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- runtime validation: all 1,431 manifest entries and content hashes verified
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production MSI SHA-256:
  `54873DD8D7E778A9401E04F5BCC559CE43971875E5C0D15775E68973641C201B`
- production bootstrapper SHA-256:
  `7BAC9C51D4AFAD90F0AA7DE58B5DD83F6350D935560B451C787AC2F4209B49E0`

Installed acceptance: pass. `gui_ulist_resizable` reports `OFF`, and the
configured user-list width now applies without the previous channel-switch
workaround.

### GTK4 Stage 9 Network List Interaction Acceptance, Pass 27

Date: 2026-07-26

Installed testing found that clicking any Network List row immediately entered
network-name edit mode. Keyboard Up and Down selection did not have the same
problem.

Root cause and correction:

- each list row directly exposed a pointer-targetable `GtkEditableLabel`, whose
  internal click handling started editing before the surrounding list could
  treat the gesture as selection only
- display labels now decline pointer targeting so a single click reaches the
  list row and selects it
- explicit list activation enables targeting and starts editing; edit
  completion restores the non-targetable display state
- Add Network still selects the new row and starts its initial rename
  immediately

Automated evidence:

- full GTK4 frontend rebuild: zero warnings and zero errors
- complete GTK4 tooling contract suite: all 79 tests pass
- production MSI and bootstrapper rebuild: zero warnings and zero errors
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- runtime validation: all 1,431 manifest entries and content hashes verified
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production MSI SHA-256:
  `FB918E5CE77FE81470792EEC5160EC29104C311CDC5817D0199E45A375765E66`
- production bootstrapper SHA-256:
  `C5F9F10B93C8D0D2D18739BD3AAB51A098620D21C5F34E71D7002AAA457BB336`

Installed acceptance: pass. Mouse clicks select networks without entering
rename mode, while explicit activation and Add Network retain their intended
editing workflows.

### GTK4 Stage 9 Native Windows Tray Acceptance, Pass 28

Date: 2026-07-26

Installed testing found that `gui_tray_minimize` remained configurable but
could not hide Fabulor to the system tray.

Root cause and correction:

- retiring the GTK3 tray implementations intentionally left the backend
  operations table empty and tray capability detection unavailable
- the replacement backend uses `Shell_NotifyIconW` directly and converts the
  existing tray-state pixbufs to native alpha icons
- icon, tooltip, flashing-state, cleanup, activation, context-menu, and
  Explorer restart behavior are now owned without GTK3 APIs
- initial restore attempts exposed a second boundary defect: unmapping a
  minimized GTK4 Win32 surface could leave a correct, visible native HWND
  without a rendered client surface
- native tray callbacks now queue visibility actions outside GDK's Win32
  message filter
- Windows tray hide/restore preserves GTK's mapped render surface and changes
  only the existing HWND visibility; the shared state snapshot explicitly
  reports that native hidden state
- minimized-state handling no longer depends on the independent GTK active
  state notification arriving first

Automated evidence:

- full GTK4 frontend rebuild: zero warnings and zero errors
- complete GTK4 tooling contract suite: all 79 tests pass
- production MSI and bootstrapper rebuild: zero warnings and zero errors
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- runtime validation: all 1,431 manifest entries and content hashes verified
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production MSI SHA-256:
  `99A3ED37B4CCFE162820DB51639607121962D1BAC7E0928786E381CF570AE971`
- production bootstrapper SHA-256:
  `01BB103CC87D4EC5669C3F0C99444C240273522C43425C1CC05307CD014BCBED`

Installed acceptance: pass. Repeated cycles pass for minimizing to the Windows
notification area, left-click restoration, the Restore Window menu command,
and opening Preferences from the native tray menu.

### GTK4 Stage 9 URL Single-Activation Acceptance, Pass 29

Date: 2026-07-26

Installed testing found that left-clicking a transcript URL opened two browser
tabs, while right-click **Open Link in Browser** correctly opened one.

Root cause and correction:

- XText uses one GTK4 click controller for pointer press/release and a separate
  `GtkGestureDrag` for precise text selection
- the drag controller can complete for a zero-distance primary-button sequence
- drag completion synthesizes the canonical release needed for selection and
  automatic clipboard ownership, while the click controller also reports the
  real release
- both releases could emit the same URL `word_click`
- XText now owns a short-lived suppression token and idle-source reference that
  consumes only the duplicate release from the same gesture sequence

Automated evidence:

- full GTK4 frontend rebuild: zero warnings and zero errors
- complete GTK4 tooling contract suite: all 79 tests pass
- production MSI and bootstrapper rebuild: zero warnings and zero errors
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- runtime validation: all 1,431 manifest entries and content hashes verified
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production MSI SHA-256:
  `973D88FE418CD3E27C84AA5D465AEC697652A4DEC0B47F0D4AA7B7FAB1948721`
- production bootstrapper SHA-256:
  `2C59F241681B3246F8434DD6A0ACCE98C1558B12929A6941E02FF078E13E9EFE`

Installed acceptance: pass. Left-click and right-click **Open Link in Browser**
each open exactly one browser tab. Text selection and release-to-copy continue
to work.

### GTK4 Stage 9 Nick Context-Menu Sizing Acceptance, Pass 30

Date: 2026-07-26

Installed testing found that the user-list nick context menu inherited its
width from hidden identity-detail pages. Long host, server, real-name, country,
or away values could therefore make the visible root menu much wider than its
own entries required.

Root cause and correction:

- `GtkPopoverMenu` uses a horizontally homogeneous stack, so every nested page
  contributes to one natural width
- making that stack non-homogeneous reduced the root page but exposed GTK's
  retained allocation during navigation, clipping nick headings and details
- stable homogeneous page sizing is retained
- the nick presenter alone constrains generated labels to 32 characters with
  end ellipsis; complete action values remain available for clipboard copying
- real names and away messages are plain `GMenu` labels, but the snapshot path
  previously requested markup escaping and displayed apostrophes as `&apos;`
- removing that inappropriate escape restores the original plain-text display

Automated evidence:

- full GTK4 frontend rebuild: zero warnings and zero errors
- complete GTK4 tooling contract suite: all 79 tests pass
- the presenter probe verifies the generated labels receive the width and
  ellipsis policy
- production MSI and bootstrapper rebuild: zero warnings and zero errors
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- runtime validation: all 1,431 manifest entries and content hashes verified
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production MSI SHA-256:
  `747BE38243573EBDFDF162F405EA73AEE1D8A5F9EEA12B597AE35683313BBB4F`
- production bootstrapper SHA-256:
  `6CF3E0FA268E293182668CAD8B20A8DE890DF7ECC6D2C8720120FDCD11C296A9`

Installed acceptance: pass. The root and identity pages retain practical,
stable dimensions; nick headings display correctly; long details ellipsize
cleanly; and away messages display ordinary apostrophes.

### GTK4 Stage 9 Obsolete Identd Retirement, Pass 31

Date: 2026-07-26

Identd was retained as an obsolete built-in service with a dedicated
Preferences page, persisted settings, command hook, automatic per-connection
port mapping, and network-listener lifetime.

Removal:

- delete the internal Identd plugin source and header
- remove its common-core MSVC and Meson registrations and translation source
- stop registering the built-in plugin and `/IDENTD` command
- remove automatic local socket-port and username publication after connection
- retire `identd_server` and `identd_port` from the preference schema and
  preference structure
- remove the Identd Preferences page and apply-time reload command
- remove the Identd-specific preference-change reason and update its focused
  theme-manager tests
- old Identd configuration keys are ignored and omitted on the next canonical
  configuration write

Automated evidence:

- production source/build audit contains no remaining Identd references
- common-core rebuild: zero warnings and zero errors
- full GTK4 frontend rebuild: zero warnings and zero errors
- complete GTK4 tooling contract suite: all 79 tests pass
- production MSI and bootstrapper rebuild: zero warnings and zero errors
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- runtime validation: all 1,431 manifest entries and content hashes verified
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production MSI SHA-256:
  `A01A0EA76E7CCBC7F66DD8E2970EF66FD915DE7926F0BAB56E1E5605DBD0AF59`
- production bootstrapper SHA-256:
  `BE2CBEB5C638744E85651788E26839BB1A9A9BCDA33081C1ADFD2703CFA13ED9`

Installed acceptance: pass. Identd is absent from Preferences and the startup
plugin report, while normal client operation remains intact.

### GTK4 Stage 9 Obsolete User-List Configuration Retirement, Pass 32

Date: 2026-07-26

`gui_ulist_style` survived as an inert compatibility setting. Its only
remaining references were the configuration schema, default initialization,
and preference structure; no runtime or frontend behavior read it.

Removal:

- remove `gui_ulist_style` from the persisted preference schema
- remove its unused default initialization
- remove `hex_gui_ulist_style` from the preference structure
- allow old saved values to be ignored and omitted on the next canonical
  configuration write

Automated evidence:

- complete source audit contains no remaining `gui_ulist_style` references
- common-core rebuild: zero warnings and zero errors
- full GTK4 frontend rebuild: zero warnings and zero errors
- complete GTK4 tooling contract suite: all 79 tests pass
- production MSI and bootstrapper rebuild: zero warnings and zero errors
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- runtime validation: all 1,431 manifest entries and content hashes verified
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production MSI SHA-256:
  `74564D62FC6BA6615B74868FAC1D4B782E711B7652375E4F63D56C15C6AA25BD`
- production bootstrapper SHA-256:
  `E8418A0FF644AF06BBF964B0097CFA763C5A091097108A31269ADB0C25450396`

Installed acceptance: pass. `/SET gui_ulist_style` reports no such variable,
and normal user-list appearance and behavior remain unchanged.

### GTK4 Stage 9 Preferences Navigation Sizing, Pass 33

Date: 2026-07-26

The ellipsized labels in the Preferences category list supplied a very small
minimum width. Lazy creation of a page with wider content could therefore
reallocate most of the category frame's width to the notebook and truncate all
navigation labels.

Fix:

- give the category frame a stable 220-logical-pixel minimum width
- retain end ellipsis for translated labels that exceed the navigation column
- extend the focused native GTK4 probe to verify the frame's minimum-width
  contract when a display is available

Automated evidence:

- strict native GTK4 probe builds and executes with zero warnings and errors
- full GTK4 frontend rebuild: zero warnings and zero errors
- complete GTK4 tooling contract suite: all 79 tests pass
- production MSI and bootstrapper rebuild: zero warnings and zero errors
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- runtime validation: all 1,431 manifest entries and content hashes verified
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production MSI SHA-256:
  `A94CB706940AD5D8C12B590D7B48BD9E5745A21032F73E4D62EA4FCB24F24645`
- production bootstrapper SHA-256:
  `3D828886ED611C9707C56A7024B202130D430C4363BF4987AC30F854D489228F`

Installed acceptance: pass. Repeated selection across Appearance, General,
Alerts, Logging, Advanced, Network setup, and File transfers leaves the
category pane stable and its labels readable.

### GTK4 Stage 9 Obsolete Wingate Proxy Retirement, Pass 34

Date: 2026-07-26

Wingate occupied persisted proxy value `1` and retained separate IRC and DCC
traversal implementations. Removing the obsolete protocol must not renumber or
change the behavior of the retained proxy modes.

Removal and compatibility:

- remove Wingate from the Preferences proxy list
- remove Wingate IRC and DCC traversal and dispatch
- reserve persisted value `1` as the retired Wingate slot
- normalize value `1` and invalid values to disabled
- retain SOCKS4 `2`, SOCKS5 `3`, HTTP `4`, and Auto `5`
- centralize stored/display mapping, authentication support, DCC eligibility,
  configuration normalization, and canonical-save behavior

Automated evidence:

- focused proxy-policy probe covers values `0` through `5`, invalid values,
  sparse menu mapping, authentication support, and DCC proxy eligibility
- common-core rebuild: zero warnings and zero errors
- strict native GTK4 probe builds and executes with zero warnings and errors
- full GTK4 frontend rebuild: zero warnings and zero errors
- complete GTK4 tooling contract suite: all 79 tests pass
- production MSI and bootstrapper rebuild: zero warnings and zero errors
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- runtime validation: all 1,431 manifest entries and content hashes verified
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production MSI SHA-256:
  `3F2DBBD4D06D4092CC860372464595E2F9F3C4FBA754AEDBC9E2EDB450138EFE`
- production bootstrapper SHA-256:
  `8483270883F7EDCE69868A77E7D8702A825E6FB93DE92609BC45683096CEFADA`

Installed acceptance: pass. `/SET net_proxy_type 1` reports normalized value
`0`, a subsequent query remains `0`, and both ZNC and direct IRC connections
operate normally. The ordinary unproxied DCC implementation is unchanged; its
proxy eligibility boundary is covered by the focused policy probe.

### GTK4 Stage 9 SOCKS5 Protocol Hardening, Pass 35

Date: 2026-07-26

SOCKS5 remains supported, but its inherited IRC and DCC implementations used
separate packet construction, assumed complete socket reads/writes, and could
accept weaker method negotiation than the configured authentication policy.

Hardening and compatibility:

- add one bounded SOCKS5 protocol owner shared by IRC and DCC
- support TCP `CONNECT` with no authentication or RFC 1929
  username/password authentication
- require complete bounded credentials when authentication is enabled
- reject unsupported methods and authentication downgrade
- handle partial socket I/O and interrupted calls
- validate versions, reserved fields, destination ports, address types, and
  variable-length replies
- remove unaligned port encoding and terminate closed queued DCC writes
- leave SOCKS4 unchanged and `Proposed`

Automated evidence:

- exact-byte SOCKS5 protocol assertions pass under strict MSVC `/W4 /WX`
- fresh independent Meson/Ninja probe: 1/1 passed
- full GTK4 common/frontend/launcher build: zero warnings and zero errors
- Python contract suites: 7/7 and 79/79 passed
- production installer build: zero warnings and zero errors
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- runtime validation: all 1,431 manifest entries and content hashes verified
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality

Installed acceptance: pass. Direct IRC and ZNC connect through MicroSocks with
both no authentication and username/password authentication. Disabling
authentication against the authenticated proxy produces a controlled
authentication-method rejection.

### GTK4 Stage 9 Right-Pane Allocation Acceptance, Pass 36

Date: 2026-07-26

Applying SOCKS5 Preferences exposed a general GTK4 layout regression:
temporarily detaching the user list emitted divider notifications, and an early
pre-maximize allocation could leave the end pane consuming most of the window
despite a valid saved width.

Fix:

- suppress right-pane persistence while movable layout children are detached
  and reattached
- preserve the accepted right-pane size across generic Preferences application
- schedule one restoration from each final window-surface layout
- recover an implausibly oversized restored pane to the configured nickname
  width or pane minimum
- retain normal user-controlled divider persistence when resizing is enabled

Automated evidence:

- strict native GTK4 probe covers valid, minimum, and oversized recovery
- full GTK4 frontend and launcher rebuild: zero warnings and zero errors
- production MSI and bootstrapper rebuild: zero warnings and zero errors
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- runtime validation: all 1,431 manifest entries and content hashes verified
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production MSI SHA-256:
  `7870EED4BAE51FA9BA663990CB74867D57E6E5332EE9152BBD48B9CCED0C0242`
- production bootstrapper SHA-256:
  `BCAE71D986610B7FA8C953F2DA2315F80C05F99564E686BAA3BD8484820D9452`

Installed acceptance: pass after clean uninstall/install. The user-list pane
remains stable while switching through all channels, and no channel-switch lag
was observed.

### GTK4 Stage 9 Inert Configuration Retirement, Pass 37

Date: 2026-07-26

`text_transparent` survived as a persisted compatibility setting despite
having no behavioral reader in the common core, GTK4 frontend, plugin bridge,
tests, build, or packaging paths.

Removal and compatibility:

- remove `text_transparent` from the persisted preference schema
- remove `hex_text_transparent` from `zoitechatprefs`
- ignore old saved values and omit them on the next canonical configuration
  write
- retain background-image and GTK4 theme behavior unchanged

Automated evidence:

- complete production-source audit contains no remaining
  `text_transparent` references
- common-core rebuild: zero warnings and zero errors
- full GTK4 frontend rebuild: zero warnings and zero errors
- GTK4 tooling contract suite: 79/79 passed
- theme contract suite: 7/7 passed
- production MSI and bootstrapper rebuild: zero warnings and zero errors
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- runtime validation: all 1,431 manifest entries and content hashes verified
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production MSI SHA-256:
  `35B41BEFCFE0B5E67A61505CBF72909976F29EB8585F5C746542EDE2058DF35B`
- production bootstrapper SHA-256:
  `A3E234D5E61F18A71D1218FC27C09B439A15C2C9E8976F7CF104682F318BE904`

Installed acceptance: pass. `/SET text_transparent` reports no such variable
and supported appearance behavior remains operational.

### GTK4 Stage 9 Server-Time Preference Retirement, Pass 38

Date: 2026-07-26

`irc_cap_server_time` presented a user-facing toggle but did not control
capability negotiation. Standard and ZNC server-time capabilities were already
requested whenever advertised, regardless of the saved value.

Removal and compatibility:

- remove `irc_cap_server_time` from the persisted schema, default
  initialization, and preference structure
- remove the misleading Preferences toggle
- retain unconditional negotiation of `server-time`,
  `znc.in/server-time`, and `znc.in/server-time-iso`
- retain existing capability state and timestamp parsing
- ignore old saved values and omit them on the next canonical write

Automated evidence:

- source audit contains no production preference reference while all three
  server-time negotiation paths and parsing remain present
- common-core rebuild: zero warnings and zero errors
- full GTK4 frontend rebuild: zero warnings and zero errors
- GTK4 tooling contract suite: 79/79 passed
- theme contract suite: 7/7 passed
- production MSI and bootstrapper rebuild: zero warnings and zero errors
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- runtime validation: all 1,431 manifest entries and content hashes verified
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production MSI SHA-256:
  `09AC261BF257E4D5BCE0C36171F6BE232C73321C12B4286F6A06F1B5006A0E6D`
- production bootstrapper SHA-256:
  `69EACE6762F060DA8EFF52BE6B6343DDDB7533F09937B1DE85E5411229340271`

Installed acceptance: pass. `/SET irc_cap_server_time` reports no such
variable, Preferences no longer exposes the toggle, and normal IRC and ZNC
timestamps remain operational.

### GTK4 Stage 9 Channel-Switch Latency Acceptance, Pass 39

Date: 2026-07-27

Installed testing found that switching networks and channels could feel
delayed, particularly on busy channels, despite stable IRC lag measurements.
An opt-in production profiler separated the synchronous tab switch,
transcript rewrapping, accessibility refresh, and user-list model attachment.

Diagnosis and fix:

- gate profiling behind `FABULOR_PROFILE_UI`; ordinary launches perform no
  timing calls or file I/O
- write enabled diagnostics to `ui-performance.log` in the Fabulor
  configuration directory
- identify ordinary synchronous tab replacement at approximately 2 to 3 ms
- identify isolated stale-width transcript rewraps at approximately 7 to
  12 ms, limited to the first revisit after viewport-width changes
- rule out accessibility snapshot work in the measured sessions
- identify deferred user-list model replacement as the persistent visual
  delay: attachment began 100 to 150 ms after the transcript in the initial
  run and 50 to 65 ms after it at high idle priority
- replace the historical deferred attachment with one synchronous switch
  transaction so transcript and user list reach the next frame together
- avoid redundant assignments when the requested user-list model is already
  attached

Automated evidence:

- full GTK4 frontend rebuild: zero warnings and zero errors
- GTK4 tooling contract suite: 71/71 passed
- production MSI and bootstrapper rebuild: zero warnings and zero errors
- source formatting validation: pass

Installed acceptance: pass. Direct IRC/ZNC channel switching without a proxy
feels more responsive. Network lag remained approximately 0.2 seconds and was
not correlated with the local UI delay.

### GTK4 Stage 9 Active Source Retirement, Pass 40

Date: 2026-07-27

The final audit found no GTK3 dependency in the supported application build,
but did find dormant GTK3 code and stale build metadata: FiSHLiM retained a
complete GTK3 dialog implementation, Sysinfo retained GTK3 labels, frontend
comments and one theme test described obsolete GTK3 behavior, and the root
Makefile referenced a root `meson.build` that no longer existed.

Removal and containment:

- specialize FiSHLiM and Sysinfo to their GTK4 production paths
- remove obsolete GTK3-facing wording and the unbuilt GTK3 Theme Access test
- remove the inherited application Makefile, post-install script, and 21
  non-configurable Meson fragments
- make `installer\Directory.Build.props` the sole version source for the
  supported MSVC resource generator
- retain `tools\gtk4\meson.build` and `meson_options.txt` as the isolated strict
  GTK4 probe
- retain negative validators for GTK3 headers, DLLs, imports, runtime roots,
  theme files, and source reintroduction
- replace GTK 4.22-deprecated pixbuf texture conversion with one
  lifetime-correct `GdkMemoryTexture` helper
- reconcile `api-inventory.md` as a completed GTK4 inventory, separating its
  historical migration record from authoritative final-state tables

Automated evidence:

- production-source audit: zero active GTK3 references
- strict GTK4 MSVC probe: pass
- fresh MSVC Meson/Ninja probe: 60-step build and runtime test 1/1 pass
- full Release x64 solution: zero warnings and zero errors
- GTK4 tooling contracts: 79/79 passed
- theme contracts: 7/7 passed
- production MSI and bootstrapper rebuild: zero warnings and zero errors
- production MSI validation: 7,624 installed files and zero GTK3 path markers
- runtime validation: all 1,431 manifest entries and content hashes verified
- native import validation: 35 files, 107 packaged edges, and 54 reviewed
  system imports
- frontend bootstrap validation: launcher 9 imports, frontend 32 imports, and
  `fabulor_frontend_main` resolved
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production MSI SHA-256:
  `C303C78D46A48A2844C0B5ABE4160469D6D486AFBEF04F7B467D5C69EDE186A3`
- production bootstrapper SHA-256:
  `FD08D77067709078A797D1E44C9DA8B18BE63FD06DF595833866E03603AB1D78`

Installed acceptance is not required for deleting inactive build metadata.
The GTK4 pixbuf path remains covered by the strict probes and complete frontend
build; ordinary installed-client visual acceptance remains part of the
existing Stage 9 validation workflow.

### GTK4 Stage 9 Tcl Runtime Payload Minimization, Pass 41

Date: 2026-07-27

The installed Tcl feature inherited an entire general-purpose distribution:
5,588 files and 96.34 MiB, including Tk, command shells, import libraries,
build tools, source/tests/examples, and unrelated third-party packages.
Fabulor directly loads only the Tcl engine and initializes an interpreter
against the Tcl 8.6 core library.

Removal and containment:

- replace whole-tree `Runtime\Tcl\bin` and `lib` staging with an explicit
  embedded-runtime allowlist
- retain `tcl86t.dll`, Tcl 8.6 core scripts, encodings, timezone/message data,
  and reviewed `platform`, `msgcat`, `http`, and `tcltest` modules
- exclude Tk, Tcl command shells, import/stub libraries, Critcl, TLS, SQLite,
  Tcllib, TWAPI, and other third-party package collections
- document that add-ons requiring other Tcl packages must distribute and load
  those dependencies within their trusted add-on directory
- lock the absence of broad Tcl tree entries in the staging contract tests

Automated evidence:

- plugin-host staging tests: 7/7 passed
- production WiX profile tests: 24/24 passed
- isolated staged-root Tcl 8.6 initialization: pass
- standard `platform`, `msgcat`, `http`, and `tcltest` package loading: pass
- CP1252 encoding round trip and `Australia/Sydney` timezone formatting: pass
- maintained simple and manifest Tcl sample initialization: pass
- staged Tcl payload: 825 files, 4.95 MiB
- staged payload contains no Tk, shell executable, development library, or
  known third-party package tree
- production MSI and bootstrapper rebuild: zero warnings and zero errors
- production MSI validation: 2,861 installed files and zero GTK3 path markers
- runtime validation: all 1,431 GTK4 manifest entries and content hashes
  verified
- production bundle validation: version `1.0.4`, one embedded MSI, and exact
  embedded/published MSI equality
- production MSI SHA-256:
  `F5354A39D203CD4B5D79D269126A602F90A7C8E9F5D4C10B98642A2ACD2CD7C7`
- production bootstrapper SHA-256:
  `7C15552E3F9058959FB948D6BE81E5AB2CCCEADE8B6409DD4F2C84BDBDD95E2B`

Installed acceptance: pass. A clean uninstall/reinstall starts normally, the
reduced installed `Runtime\Tcl` payload is present, and the configured Tcl
scripts load and function correctly.

### GTK4 Installed Sound Preferences Acceptance

Date: 2026-07-28

The GTK4 sound-event table previously depended on row-property class
initialization order. On the first Preferences opening after process startup,
the table contained rows but rendered the empty sound-file property in both
columns. Reopening Preferences initialized the row class and made event names
appear. The Sounds page also influenced the dialog's natural size when created
lazily.

Resolution and automated evidence:

- select event-name and sound-file values by explicit column identity rather
  than cached property metadata
- create the GTK4 probe view before appending rows to preserve the cold-start
  construction order
- let the Sounds page expand within a stable `900 x 600` Preferences default
  size instead of imposing a page-specific minimum
- strict GTK4 MSVC probe: pass with zero warnings and zero errors
- Release x64 GTK4 frontend build: pass with zero warnings and zero errors
- MSI and bootstrapper build: pass with zero warnings and zero errors, with
  external ICE validation suppressed because the Windows Installer service
  was unavailable in the build session

Installed acceptance: pass after a clean install. The complete event list
renders on the first Sounds-page opening, Preferences retains its dimensions
when switching pages, a Windows `notify.wav` file can be assigned to `Add
Notify` and played, and the assignment persists after restarting Fabulor.

## GTK4 Desktop-Theme Archive Import (2026-07-28)

Scope: contained profile-theme installation and Appearance-page integration.

Automated evidence:

- 33 common/security tests pass, including contained GTK4 extraction,
  overwrite refusal, and the external six-theme `Orchis-Grey.tar.xz` fixture
- the real fixture imports all six GTK4 variants in approximately one second
  without extracting its unrelated symbolic links
- common and GTK4 frontend Release x64 builds pass with zero warnings and zero
  errors
- the theme contract requires bounded private-copy inspection, path and tree
  validation, background UI dispatch, and the supported archive selector

Initial installed acceptance failed. Import produced visible flashing and
selecting an Orchis theme left the UI unresponsive. The corrective candidate:

- consolidates the six validated roots into one extraction process
- refreshes only selector metadata after import instead of touching the live
  global style provider
- defers and coalesces dropdown application until GTK has closed its popup
- resolves light/dark policy to one complete stylesheet instead of layering
  `gtk-dark.css` over the complete `gtk.css`

Corrective automated evidence: all 33 common/security tests pass; the real
six-theme fixture imports in approximately 0.9 seconds; the strict GTK4 probe
and common/frontend Release x64 builds pass with zero warnings and zero errors;
the MSI and bootstrapper also rebuild with zero warnings and zero errors.
Installed acceptance must be repeated before this stage is committed.

The corrective installed selection test no longer froze or crashed. GTK
rejected `Orchis-Grey/gtk-4.0/gtk-dark.css` at line 8652 because the archive
contains uncompiled Sass `$...` tokens and an unsupported
`@define-color ... var(...)` value. Fabulor displayed the parser error and
returned to the system theme.

The importer now checks the required `gtk.css` and optional `gtk-dark.css`
inside private staging and rejects those two unmistakable uncompiled or
unsupported forms before moving any theme root into the profile catalogue.
It does not execute a theme's installer or attempt an ad hoc stylesheet
rewrite. Automated evidence:

- 36 common/security tests pass, including synthetic uncompiled-CSS and
  unsupported-define rejection with no installed destination
- real `Orchis-Grey.tar.xz` negative fixture: rejected in staging
- real `Nordic-darker.tar.xz` positive fixture: contained import passes
- 12 theme-contract tests pass
- common Release x64 test build: zero warnings and zero errors
- strict GTK4 probe and execution under `/W4 /WX`: zero warnings and zero
  errors; a clean process environment avoided the calling shell's duplicate
  `Path`/`PATH` keys
- common, GTK4 frontend, and launcher Release x64 builds: zero warnings and
  zero errors
- production MSI and bootstrapper rebuild: zero warnings and zero errors

Installed import acceptance remains required.

The installed import test also exposed visible console flashing while the
system archive tool performed its inventory and extraction passes. GLib's
portable subprocess API does not expose Windows' no-console creation flag, so
the Windows archive boundary now starts the exact validated `tar.exe` path with
`CreateProcessW`, `CREATE_NO_WINDOW`, hidden and redirected standard handles,
and the same argument-vector and bounded-output contract. Non-Windows builds
retain the GLib subprocess path.

Post-change automated evidence:

- all 36 common/security tests pass
- real Nordic positive and Orchis negative archive fixtures pass
- strict GTK4 probe compiles and executes under `/W4 /WX` with zero warnings
  and zero errors
- common and GTK4 frontend Release x64 builds: zero warnings and zero errors
- production MSI and bootstrapper rebuild: zero warnings and zero errors

Installed acceptance: pass. `Nordic-darker.tar.xz` imported without console
flashing, applied successfully as a GTK4 desktop theme with `Prefer dark`
selected, and switching back to `System default` restored the system
appearance correctly.

Project policy records OpenDesktop.org as the sole approved acquisition source
for Fabulor desktop themes. The client does not download from or assign trust
to that source; imported archives remain subject to all containment and CSS
compatibility checks.

### Bundled Fabulor Dark palette

Boundary: one original colours-only starter palette may ship independently
from GTK4 desktop themes. `Fabulor Dark.hct` contains exactly the tracked
`colors.conf` source and no event definitions or executable content. The
installed archive remains read-only under `share/palettes`; profile `.hct`
files remain under `%APPDATA%\Fabulor\themes` and take precedence on a
case-insensitive name collision.

Automated evidence:

- 37 common/security tests pass, including bundled discovery and profile
  precedence
- 14 theme-contract tests pass
- common, GTK4 frontend, and launcher Release x64 builds complete with zero
  warnings and zero errors
- production MSI and bootstrapper build with zero warnings and zero errors
- decompiled production MSI contains `share/palettes/Fabulor Dark.hct`,
  reports 2,862 installed files, and contains zero legacy GTK files

Installed acceptance: pass. `Fabulor Dark` appears in the Colours palette
selector from the read-only installed archive, previews and applies correctly,
and preserves the existing Cancel and OK transaction behaviour.

## Stage Completion Rule

A stage can move to complete in `migration-plan.md` only when:

1. its API inventory rows are converted or deliberately retired;
2. automated checks are green;
3. affected manual workflows pass;
4. performance and visual results are recorded;
5. packaging impact is either validated or explicitly not applicable; and
6. known regressions have owners and follow-up issues rather than being hidden.
