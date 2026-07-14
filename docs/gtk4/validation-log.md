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

### Stage 2 Visibility And Lifecycle, Pass 2

Date: 2026-07-14

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
- [ ] GitHub Actions required checks: 5/5

Scope: completed shared prompts, exact dialog response callbacks, and concrete
application windows. Generic Escape/button destroy helpers, menu ownership,
notebook/channel-view children, dynamic controls, unparented probes,
spell-entry menu items, and GTK3-only tests remain explicit.

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
