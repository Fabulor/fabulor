# GTK4 API Inventory

Status: maintained migration inventory

Baseline date: 2026-07-14

## Method

This inventory combines source review with literal symbol searches under
`src/fe-gtk`. Counts are migration indicators, not compiler diagnostics: one
line may contain several calls, wrappers may hide additional work, and some GTK
types remain available in GTK4 while still requiring model or lifecycle changes.
Direct-call counts exclude `gtk-compat.h`; the helper implementations are
tracked separately in the compatibility boundary below.

Update this file in every GTK4 conversion PR. Use these status values:

- `not started`
- `in progress`
- `converted`
- `retired`
- `blocked`

## Build Boundary

| Area | Current state | GTK4 target | Status |
|---|---|---|---|
| Meson frontend dependency | legacy fragments use `gtk+-3.0 >= 3.22`; no top-level project exists | root-driven GTK4 probe established; production integration remains | in progress |
| MSVC headers/libraries | production uses GTK3 through `win32/zoitechat.props`; isolated GTK4 project exists | validated GTK4 root and import libraries in converted targets | in progress |
| Windows CI build dependencies | GTK3 production archive plus GTK4 probe archive | GTK4 URL, size, SHA-256, versions, and x64 identity pinned | in progress |
| Windows runtime payload | separate GTK4 tree already downloaded and packaged | same audited runtime used by the executable | in progress |
| Staged release root | GTK3 DLLs and data beside `fabulor.exe` | GTK4-only final layout | not started |

## Compatibility Helper Boundary

`src/fe-gtk/gtk-compat.h` provides type-specific, header-only helpers for:

- start-ordered box insertion with explicit expansion, fill, and padding
- horizontal trailing-child insertion with preserved end alignment
- ordered insertion immediately before a permanent trailing child
- trailing label/control pair insertion with preserved end alignment
- box-owned dynamic child removal
- window, scrolled-window, frame, button, overlay, and popover child assignment
- completed-tree reveal with distinct GTK3 recursive and GTK4 root semantics
- window destruction
- dialog-response destruction with an exact GTK signal callback signature
- standard and primary text clipboard updates through the widget display
- closure-owned pointer-enter and focus interactions using GTK4 event controllers
- smooth/discrete scroll normalization through a capture-phase GTK4 controller

The production GTK3 build and isolated GTK4 MSVC/Meson probes compile the same
helper bodies. The GTK4 probes also take each helper's address so every GTK4
branch is linked, not merely preprocessed. Production now uses 40 typed child
assignments across 14 source files: 7 windows, 18 scrolled windows, 7 frames,
6 buttons, 1 overlay, and 1 popover.

Production now also uses 156 reviewed start-ordered box additions across 17
files, two horizontal trailing children, two ordered insertions before a
permanent trailing child, three trailing label/control pairs, and five
box-owned dynamic child removals.

The shared tree-view constructor now accepts only `GtkBox` parents for its
nine operational-list scrollers. The main user-list constructor has the same
typed parent contract. Their tree models, renderers, selection, and drag/drop
paths remain outside this Stage 2 ownership conversion.

The visibility helper is limited to 17 reviewed roots whose descendants have
finished construction and have no intentional hidden state at reveal time.
The boundary deliberately does not abstract generic widget destruction,
remaining mixed start/end box ordering, menu/item visibility, events,
clipboard ownership, or list/tree models. Those operations have GTK4 lifetime
or behaviour changes that must remain visible at each caller. Dynamic removal
is limited to five children with a known owning `GtkBox`. The box helper
preserves explicit widget alignment/expansion, adds GTK3 packing padding to
existing directional margins, and is used only where append order is exact.

Stage 3 now stores the 23 command identities shared by main-menu accelerators
and configurable keyboard shortcuts directly on their canonical menu
definitions. Menu construction and shortcut dispatch therefore consume the
same stable names and typed identifiers without relying on parallel positional
tables. This foundation does not yet abstract GTK3 menu widgets, state,
sensitivity, or popup ownership.

Nineteen stateless canonical commands now activate through a per-menu
`GSimpleActionGroup` and `GtkActionable` binding. The same group is retained by
each bound item after construction, while configurable shortcuts continue to
use the canonical dispatcher.

Menu-bar visibility, user-list visibility, and fullscreen now use boolean
`GSimpleAction` state. Shared preference updates synchronize every window's
corresponding action, and the window-state event remains authoritative for
correcting fullscreen state after a platform transition.

Away uses a session-aware boolean `GSimpleAction`. Server-confirmed away/back
events own its state, while active tab changes and connect/disconnect events
update its enabled state for the selected server. Activation continues through
the canonical command dispatcher without optimistically changing server state,
and the existing plain GTK3 menu presentation remains unchanged.

The first complete model boundary now projects the static Search submenu into
an immutable `GMenuModel` containing its three canonical action names. The
model and shared action group are retained on the menu root while the live
GTK3 submenu continues through its existing widget construction. Dynamic
`/MENU`, plugin, and user-menu entries are intentionally excluded until their
mutation and ownership contracts are converted.

The static Help menu is the second complete model boundary. About now has a
canonical action identity and dispatcher entry alongside Contents, and both
items are projected through the shared range-based model builder. This expands
the canonical set to 17 commands while preserving Help menu presentation and
dialog behaviour in GTK3.

The static New submenu is the third complete model boundary. Channel Tab and
Channel Window now have canonical action identities alongside the existing
Server Tab and Server Window commands, and all four entries are projected
through the shared range-based model builder. The existing creation handlers,
preference restoration, default shortcuts, and displayed GTK3 submenu remain
unchanged.

The static Server menu is the fourth complete model boundary. Disconnect,
Reconnect, Join a Channel, and Channel List now have canonical stateless
actions alongside Away. Two retained sections preserve the separator before
Away, while Disconnect and Join availability initialize from and continue to
track the selected server through the shared action group. Existing IRC
commands, dialogs, connection handling, shortcuts, and GTK3 presentation
remain unchanged.

The Channel Switcher submenu is the fifth complete model boundary and the
first targeted selection action. One string-valued `channel-switcher` action
uses canonical `tabs` and `tree` targets, bringing the action registry to 24
distinct identities. The retained model carries those targets directly. GTK3
radio items keep their proven callback path because GTK3 does not reliably
dispatch radio targets through `GtkActionable`; existing layout changes also
synchronize the retained action without re-entering the callback.

The Network Meters submenu is the sixth complete model boundary and the second
targeted selection action. One string-valued `network-meters` action uses
canonical `off`, `graph`, `text`, and `both` targets, bringing the registry to
25 distinct identities. Action and legacy GTK3 radio activation share the same
preference, timer, and meter-refresh path, and selection state is synchronized
across open menu roots without reactivating the command. Preferences continues
to support independent lag and throttle values outside this submenu.

The static Settings menu is the seventh complete model boundary. Preferences
and its nine configuration editors now have canonical stateless action
identities, bringing the registry to 35 distinct identities. A retained
two-section model preserves the separator after Preferences, while the live
GTK3 menu activates the same dispatcher and continues to open the existing
preference and editor surfaces without changing their ownership or lifecycle.

The static Window menu is the eighth complete model boundary. Eleven remaining
commands now have canonical stateless identities, bringing the registry to 46.
Its retained model has two sections: nine operational windows, then five
transcript commands and the existing three-command Search submenu. Search,
Window, and Help boundaries are derived from their preceding ranges, correcting
two stale numeric offsets that had malformed the retained Search and Help
models without affecting the displayed GTK3 menus.

The static View menu is the ninth complete model boundary. Topic Bar, User List
Buttons, and Mode Buttons now join Menu Bar, User List, and Fullscreen as
boolean actions, bringing the registry to 49 distinct identities. The retained
three-section model preserves the five visibility toggles, nests the existing
Channel Switcher and Network Meters selection models, and keeps Fullscreen in
its trailing section. Preference changes synchronize each action across open
windows without re-entering GTK3 callbacks.

The static Fabulor menu is the tenth complete model boundary. Load Plugin or
Script and Attach/Detach now have canonical stateless identities, bringing the
registry to 51. Its retained five-section model nests the existing New model
and preserves all separators around Network List, plugin loading, window
attachment controls, Close, and Quit. Each menu root copies the Attach or
Detach label selected for that window before model construction. Dynamic
Usermenu, `/MENU`, plugin top-level content, and popup mutation remain outside
the static model milestone.

The dynamic Usermenu is the first complete mutable model boundary. Every menu
root retains a recursive model built from `usermenu.conf`, including nested
submenus, section separators, copied command targets, boolean preference
actions, icon-path hints, and the editor command. Refresh removes the previous
Usermenu actions before publishing the replacement model, so reloaded list
storage cannot remain reachable through an action callback. Plugin `/MENU`
entries and contextual popup models remain outside this boundary.

Main-menu `/MENU` entries now have a retained dynamic overlay boundary. The
common command layer synchronizes only after markup labels are finalized, state
or sensitivity updates are applied, or recursive child deletion has completed.
Each GTK menu root then rebuilds nested paths and sections from live main-menu
entries, publishes copied action lookup data, and removes every previous plugin
action. Missing parent paths remain excluded as they are in GTK3. Contextual
`$NICK`, `$URL`, `$CHAN`, `$TAB`, and `$TRAY` entries now have a per-invocation
model boundary. The allowlisted root selects the live plugin subtree, while
each action owns copied root and target strings and only a weak reference to
the popup owner. Replacing a stateful model retires its owner's previous action
group atomically; destroying the owner releases the complete model and callback
state. Targeted substitution and direct toggle/radio dispatch preserve GTK3
behavior. The GTK3 popup widgets and native Windows tray presentation remain
in place pending presentation conversion.

Stage 3 dialog lifecycle work is complete. Manifest-plugin enablement and the
quit/minimize decision now resume from explicit response callbacks with bounded
parent or singleton ownership. Fatal font failure prevents further transcript
rendering while its response-owned shutdown dialog is active. Frontend warning
messages are asynchronous, with an explicit modal flag where acknowledgement
must gate interaction. Pre-event-loop Windows command-line information uses a
native UTF-16 message box. The frontend contains no blocking GTK dialog or
native chooser run call.

Stage 4 begins with the shared explicit-copy boundary. Five callers across ban
lists, channel lists, URL actions, and URL history now request one stable
standard-plus-primary copy operation without exposing GTK3 `GdkAtom` values.
The GTK4 branch resolves `GdkClipboard` objects from the widget display and
avoids duplicate writes when the backend aliases primary and standard
clipboards. Transcript-owned selection and clipboard code remains isolated in
`xtext.c` for the custom-widget stage.

The first event-controller boundary covers simple interactions whose callbacks
do not consume coordinates or event metadata. Character Chart hover uses a
GTK4 motion controller, while Join entry focus and theme-colour focus loss use
GTK4 focus controllers; their callback storage is retired with the signal
closure. The scroll-to-bottom button uses ordinary button activation instead
of a raw pointer event. These conversions remove direct event types from three
frontend files without changing input, transcript, or spell-check dispatch.

Channel switcher scroll handling now has one delta-based contract across tree,
viewport, tab, and close-button surfaces. GTK4 receives both-axis deltas from a
capture-phase scroll controller and propagates unhandled movement; GTK3
normalizes smooth and discrete wheel events inside the compatibility boundary.
The channel workflow retains only vertical direction and configured step-count
logic, with no direct scroll-event or event-mask dependency.

All remaining production focus-enter workflows now use the controller
boundary. Detached message-entry focus still selects its owning session, while
standalone and shared windows retain marker, plugin notification, server
session, and taskbar-flash behavior. Their callbacks no longer expose
`GdkEventFocus`, and no production source connects `focus-in-event` directly.

Channel-tab close hover now uses typed coordinates and pointer-leave cleanup
through one motion-controller boundary. The GTK4 branch owns a motion
controller and sets the pointer cursor by widget name; GTK3 event masks, event
objects, cursors, and native windows remain confined to the compatibility
layer. The tab workflow retains its existing close-area hit test and prelight
state, while click/context-menu dispatch and outer-tab prelight suppression
remain separate for later event and presentation passes.

The first typed key boundary covers three independent, non-consuming
workflows: detached utility-window Escape, search-bar Escape, and raw-log
`Ctrl+Shift+C`. GTK4 uses a bubble-phase key controller so window-owned
shortcuts retain descendant propagation, while GTK3 key events remain inside
the compatibility layer. Embedded utility tabs do not close on Escape, and
the raw-log auto-copy guard remains unchanged. The main configurable shortcut
engine and widget-specific input paths remain deferred to their owning passes.

The same typed key boundary now covers the small consuming actions in the main
window outside the chat input: Return or keypad Enter submits the topic, while
exact `Ctrl+A` selects the channel key or user-limit field. Their previous
consume/propagate decisions and modifier exclusions are unchanged. The sole
direct key-event registration then remaining in `maingui.c` belonged to the
main configurable shortcut engine.

The main chat input and its configurable shortcut engine now share a small
immutable key-value/modifier input instead of passing `GdkEventKey` through the
17-action table. Plugin notification retains first refusal and the same Unicode
derivation; binding lookup retains exact filtered modifiers; consumed actions
return through the controller callback without naming a GTK3 signal. The only
remaining raw key workflow in `fkeys.c` is shortcut-editor Shift+Up/Down row
ordering, which stays with that tree/model conversion.

Topic URL pointer handling now receives typed coordinates, leave notification,
button identity, and modifier state. GTK4 uses motion and click controllers,
sets the text-view cursor by name, and claims only successful URL activation;
GTK3 event objects and text-window cursor ownership remain inside the
compatibility layer. Existing buffer-coordinate conversion, word parsing,
modifier matching, and editable text behavior remain unchanged.

External file drops now use one URI-list contract across private-dialog
transcripts and user-list nickname targets. GTK4 receives `GdkFileList` from a
drop target and converts each `GFile` to a URI; GTK3 validates the
`text/uri-list` target and copies selection bytes before dispatch. The existing
DCC filename conversion and send loop now reports whether at least one file was
accepted.

Internal layout drag/drop now exposes only `FabulorGtkInternalDragKind`, typed
coordinates, and lifecycle callbacks to channel-view, user-list, scrollbar,
and pane-placement consumers. GTK4 uses local pointer content with
`GtkDragSource` and preloaded `GtkDropTarget`; GTK3 exact-target decoding and
same-application restrictions remain in `gtk-compat.h`. User-list file and
internal hover share typed row selection and deterministic leave/drop cleanup.
The former action-code identity and stale leading-character target test are
removed. The remaining consumer-side `GtkSelectionData` reference is transcript
clipboard ownership in `xtext.c`, not drag/drop, and remains assigned to Stage 6.

## Quantitative API Baseline

| GTK3 family or type | Matching lines | Files | Migration direction | Stage | Status |
|---|---:|---:|---|---:|---|
| `gtk_container_*` | 109 | 23 | explicit widget-specific child APIs | 2 | in progress |
| `gtk_box_pack_*` | 25 | 4 | `gtk_box_append/prepend` and reorder APIs | 2 | in progress |
| `gtk_widget_show_all` | 17 | 8 | explicit visibility; GTK4 children visible by default | 2 | in progress |
| `gtk_widget_destroy` | 31 | 13 | window close and object ownership appropriate to type | 2 | in progress |
| `GtkEventBox` / `gtk_event_box_*` | 8 | 2 | ordinary widgets plus controllers/gestures | 2/4 | not started |
| `GtkTable` / `gtk_table_*` | 2 | 1 | `GtkGrid` | 2 | not started |
| `gtk_dialog_run` | 0 | 0 | response-driven/asynchronous dialog flow | 3 | complete |
| `gtk_native_dialog_run` | 0 | 0 | response-driven native dialog flow | 3 | complete |
| `gtk_message_dialog_new` | 18 | 7 | GTK4 dialog or alert abstraction | 3 | not started |
| `gtk_file_chooser_dialog_new` | 0 | 0 | GTK4 file chooser/native dialog flow | 3 | complete |
| `gtk_menu_*` | 109 | 7 | `GMenuModel`, popovers, and actions | 3 | not started |
| `gtk_menu_item_*` | 45 | 7 | actions/menu models | 3 | not started |
| `GdkEvent` | 69 | 20 | event controllers and gestures | 4 | in progress |
| `GdkDragContext` / `GtkSelectionData` | 11 | 2 | `GtkDragSource`, `GtkDropTarget`, and typed content | 4/6 | in progress; drag/drop contained |
| `gtk_widget_get_window` | 31 | 6 | surface/native access only where unavoidable | 4/6 | in progress |
| `gdk_window_*` | 48 | 8 | `GdkSurface`, snapshots, controllers, or removal | 4/6 | in progress |
| `gtk_clipboard_*` | 1 | 1 | `GdkClipboard` and content providers | 4/6 | in progress |
| `GtkTreeView` | 81 | 18 | choose GTK4 list/model widget per workflow | 5 | not started |
| `GtkStatusIcon` | 6 | 1 | native Win32 tray or supported external backend | 7 | not started |
| screen CSS provider installation | 4 | 3 | display-scoped provider installation | 7 | not started |

## High-Risk Files

The line counts below identify review size, not priority by themselves.

| File | Approx. lines | GTK/GDK reference lines | Primary risk | Stage |
|---|---:|---:|---|---:|
| `src/fe-gtk/maingui.c` | 6,505 | 1,197 | window/tab ownership, input, topic bar, layout | 2-5 |
| `src/fe-gtk/xtext.c` | 6,183 | 713 | custom rendering, selection, events, scrolling | 6 |
| `src/fe-gtk/servlistgui.c` | 3,145 | 824 | editable tree models and response-driven dialogs | 3/5 |
| `src/fe-gtk/menu.c` | 2,887 | 399 | legacy menus, context, sensitivity, commands | 3 |
| `src/fe-gtk/setup.c` | 2,548 | 491 | preferences tree, generated controls, dialogs | 2/3/5 |
| `src/fe-gtk/fkeys.c` | 2,386 | 282 | accelerators and editable cell renderers | 4/5 |
| `src/fe-gtk/fe-gtk.c` | 1,928 | 110 | startup, runtime paths, display/icon setup | 1/7/8 |
| `src/fe-gtk/sexy-spell-entry.c` | 1,878 | 156 | `GtkEntry` subclass, drawing, pointer events | 6 |
| `src/fe-gtk/theme/theme-preferences.c` | 1,946 | 564 | theme UI, models, response-driven dialogs | 3/5/7 |
| `src/fe-gtk/plugin-tray.c` | 1,620 | 147 | GTK3 status icon/AppIndicator and Win32 tray | 7 |
| `src/fe-gtk/dccgui.c` | 1,209 | 245 | transfer models, progress, dialogs | 3/5 |
| `src/fe-gtk/chanlist.c` | 1,182 | 234 | large sortable channel model | 5 |
| `src/fe-gtk/userlistgui.c` | 1,129 | 252 | live user model and interaction | 5 |
| `src/fe-gtk/gtkutil.c` | 1,087 | 289 | shared constructors and ownership helpers | 1/2 |
| `src/fe-gtk/theme/theme-gtk3.c` | 992 | 191 | GTK3 CSS/settings adapter | 7 |
| `src/fe-gtk/chanview-tabs.c` | 981 | 209 | tab buttons, scrolling, drag/drop | 2/4/5 |

## Custom Widget Boundaries

### Transcript: `GtkXText`

Current GTK3 virtual methods include realize/unrealize, size allocation, button
press/release, motion, selection ownership, draw, preferred-size calculation,
scroll, and leave notification. GTK4 requires a coordinated redesign around
measurement, allocation, snapshot rendering, event controllers, clipboard
content, and surface-independent hit testing.

Required preserved behaviour:

- IRC colours, attributes, timestamps, indentation, wrapping, and hidden text
- URL/nickname hit testing and context menus
- text selection, copy, clipboard ownership, and search highlighting
- marker line, scrollback replay, scroll-to-bottom state, and smooth scrolling
- background image/colour and theme updates
- accessibility exposure and high-DPI rendering

Status: `not started`

### Input: `SexySpellEntry`

The current implementation subclasses `GtkEntry`, overrides draw and button
events, implements `GtkEditable`, and owns Enchant-backed underline, popup,
suggestion, personal dictionary, and replacement behaviour.

Migration decision required: retain a GTK4-compatible custom entry subclass or
compose a standard GTK4 text control with spell-check state and controllers.

Required preserved behaviour:

- no measurable typing or Enter-to-send regression
- URL paste stability
- suggestions and replacement
- add-to-dictionary and persistent personal dictionary
- active-language handling and nickname exceptions
- IRC formatting and completion integration

Status: `not started`

## Functional Clusters

| Cluster | Main files | GTK4 concern | Status |
|---|---|---|---|
| Main windows and tabs | `maingui.c`, `chanview*.c` | child ownership, gestures, DnD, focus | not started |
| Transcript | `xtext.c`, `xtext.h` | snapshot rendering and event model | not started |
| Edit box and spell check | `maingui.c`, `sexy-spell-entry.c` | editable composition/subclass and controllers | not started |
| Menus and commands | `menu.c`, `plugin-tray.c` | actions and menu models | not started |
| Operational lists | `servlistgui.c`, `chanlist.c`, `userlistgui.c`, `dccgui.c`, `banlist.c`, `notifygui.c`, `ignoregui.c` | list models, factories, editing | not started |
| Preferences/editors | `setup.c`, `fkeys.c`, `textgui.c`, `editlist.c` | generated widgets, models, async dialogs | not started |
| Themes | `theme/*.c`, `common/gtk3-theme-service.c` | GTK4 CSS compatibility and adapter policy | not started |
| Platform integration | `fe-gtk.c`, `plugin-tray.c`, notifications | displays, surfaces, icons, native tray | not started |

## Theme Inventory

- `theme/theme-manager.c`, `theme-policy.c`, `theme-runtime.c`,
  `theme-application.c`, and `theme-css.c` should remain toolkit-neutral where
  practical.
- `theme/theme-gtk3.c` is the explicit GTK3 adapter and should not be renamed in
  place until a GTK4 adapter has equivalent tests.
- `common/gtk3-theme-service.c` discovers/imports GTK3 CSS directories such as
  `gtk-3.0` and `gtk-3.24`. GTK3 CSS is not assumed to be valid GTK4 CSS.
- Existing extraction containment protections remain mandatory regardless of
  the eventual GTK4 theme import policy.

## Inventory Maintenance

For each conversion PR:

1. Change relevant rows from `not started` to `in progress`, `converted`, or
   `retired`.
2. Re-run the symbol counts for touched API families.
3. Add newly discovered API families or hidden ownership dependencies.
4. Link the PR and its validation record from `validation-log.md`.
5. Do not mark a family converted while compatibility calls remain in active
   production paths.
