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
