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

Stage 5 model architecture pass 1 adds GTK4-only flat and hierarchical model
stacks in `gtk4-list-models.c`. The flat stack owns a `GListStore`,
`GtkSortListModel`, and workflow-selected single or multiple selection model;
the tree stack owns root
`GListStore`, `GtkTreeListModel`, and `GtkSingleSelection` instances. The
isolated probes execute ordering, identity, expansion, selection, and cleanup
contracts. No production `GtkTreeView`, `GtkTreeStore`, `GtkListStore`, or cell
renderer has been replaced yet, so quantitative production counts are
unchanged by this pass. A fresh literal type recount records 80 `GtkTreeView`
lines across 18 files.

Stage 5 Notify List pass 2 moves all direct tree-model, iterator, path,
renderer, and selection operations out of `notifygui.c` and into the
cross-version `notify-list.c` owner. Its GTK4 branch compiles typed row objects,
single selection, four `GtkColumnView` factories, identity-preserving refresh,
and explicit binding cleanup. The `GtkTreeView` type inventory is now 76 lines
across 18 files; the retained Notify GTK3 implementation is isolated inside the
owner until production cutover.

Stage 5 Ban List pass 18 moves the four-mode operational table into a
cross-version owner. GTK4 uses typed mode rows, `GtkMultiSelection`, sortable
`GtkColumnView` factories, and coordinate-based row selection; GTK3 retains its
list store and tree selection privately. `banlist.c` now consumes mode-indexed
mask snapshots for remove, crop, and clear, so translated type labels are no
longer part of command dispatch. Its direct tree-model, iterator, path,
selection, and cell-renderer count is zero.

Stage 5 DCC transfer-list pass 19 moves the combined Uploads and Downloads
table into a cross-version owner. GTK4 uses typed mutable rows,
`GtkMultiSelection`, one direction factory, and eight text factories; GTK3
retains its list store and tree selection privately. Transfer refresh, details,
filtering, completion cleanup, accept/resume/abort actions, and activation now
resolve stable DCC identity without direct toolkit rows in the file-transfer
paths in `dccgui.c`. Shared legacy tree helpers remain solely for the distinct
five-column DCC Chat table at this pass and are removed by pass 20 below.

Stage 5 DCC Chat pass 20 moves the remaining five-column table into its own
cross-version owner. GTK4 uses typed mutable rows, `GtkMultiSelection`, and
five text factories; GTK3 retains its list store, renderers, and selection
privately. Refresh, prepend population, Accept, Abort, activation, and removal
now resolve stable DCC identity. `dccgui.c` contains no direct tree-model,
iterator, path, cell-renderer, list-store, or tree-selection dependency, and
the transfer and chat schemas remain separate owners.

Stage 5 Channel List pass 21 moves the high-volume three-column table into a
cross-version owner. GTK4 uses typed immutable rows, `GtkMultiSelection`,
sortable `GtkColumnView` factories, collation-key ordering, and coordinate
hit-testing; GTK3 keeps the specialized append-and-resort `CustomList` path
private for shipping performance. Batched population, filtering, joining,
copying, sorted export, context selection, and width persistence now consume
typed owner methods. `chanlist.c` contains no direct tree-model, iterator,
path, cell-renderer, list-store, tree-selection, or custom-list dependency.

Stage 5 generic editable-list pass 22 moves the shared two-column editor into
a cross-version owner. GTK4 uses typed mutable rows, `GtkSingleSelection`, two
`GtkEditableLabel` column factories, and row-object drag/drop; GTK3 retains
editable renderers and native pointer reordering privately. Add, delete,
in-place edits, Shift+Up/Down movement, pointer movement, and ordered save
snapshots now cross typed methods. `editlist.c` contains no direct tree-model,
iterator, path, cell-renderer, list-store, tree-selection, raw key-event, or
`key-press-event` dependency.

Stage 5 Print Events pass 23 moves both the editable event-text table and its
numbered argument-help table into one cross-version owner. GTK4 uses typed
event/help rows, `GtkSingleSelection`, two `GtkColumnView` instances, and an
`GtkEditableLabel` factory; GTK3 retains its list stores, tree views,
renderers, and selection privately. Edits cross a callback carrying the row's
stable signal index and are committed only after parser and argument-count
validation. `textgui.c` contains no direct tree-model, iterator, path,
cell-renderer, list-store, or tree-selection dependency.

Stage 5 key-bindings pass 24 moves the accelerator/action/data editor into a
cross-version owner. GTK4 uses typed mutable rows, `GtkSingleSelection`,
`GtkShortcutLabel` key capture, `GtkDropDown`, editable data factories, and a
key controller for custom-row movement; GTK3 keeps its store, renderers,
selection, and raw key event private. Ordered save/reset snapshots and
built-in/custom mutation rules cross typed methods. `fkeys.c` contains no
direct tree-model, iterator, path, cell-renderer, list-store, tree-selection,
or raw key-event dependency.

Stage 5 Preferences sound-event pass 25 moves the event/sound-file table into
a cross-version owner. GTK4 uses typed mutable rows, explicit
`GtkSingleSelection`, and two `GtkColumnView` factories; GTK3 keeps its store,
renderers, tree view, and selection private. Selection and entry updates cross
stable event indices, while browse/play and sound configuration ownership stay
in `setup.c`. The sound-page workflow contains no direct tree-model, iterator,
path, cell-renderer, list-store, or tree-selection dependency.

Stage 5 Preferences category-navigation pass 26 moves the three-group page
hierarchy into a cross-version owner. GTK4 uses typed category/page rows,
per-category child stores, `GtkTreeListModel`, `GtkSingleSelection`,
`GtkListView`, and a tree-expander factory; GTK3 keeps its tree store,
renderer, tree view, and root-selection filter private. Page switching crosses
stable numeric identity, category headings remain non-selectable, groups start
expanded, and startup restores the remembered page. `setup.c` now contains no
direct tree-model, iterator, path, cell-renderer, list-store, tree-selection,
or child-reordering dependency.

Stage 5 Server List network-table pass 27 moves the main network chooser into
a cross-version owner. GTK4 uses typed mutable rows, the shared flat model
stack, `GtkSingleSelection`, `GtkListView`, and an editable-label factory;
GTK3 keeps its list store, renderer, tree view, and selection private. Add,
remove, inline rename, favorite emphasis, favorites-only refresh, sorting,
remembered selection, scrolling, and Shift+Up/Down ordering now use stable
`ircnet` identity. The main chooser path in `servlistgui.c` no longer accesses
a tree model, iterator, path, renderer, or selection. The three detailed
network-editor tables remain for the next contained pass.

Stage 5 Server List editor-table pass 28 moves the Servers, Autojoin channels,
and Connect commands tables into one cross-version owner. GTK4 uses typed
mutable rows, the shared flat model stack, `GtkSingleSelection`, three
`GtkColumnView` instances, and editable-label factories; GTK3 keeps its list
stores, renderers, tree views, and selections private. Stable core-object
identity now drives selection, edits, add/remove, empty-value deletion, and
Shift+Up/Down ordering even when rows have duplicate text. `servlistgui.c` now
contains no direct tree-model, iterator, path, cell-renderer, list-store,
tree-selection, or tree-view access.

Stage 5 user-list model pass 3 replaces the per-session `GtkListStore` and
frontend-owned row-reference hash with `FabulorUserListModel`. The GTK4 branch
owns typed rows, sorted list and multi-selection models, identity-indexed
updates/removals, and IRC-aware ordering. The GTK3 branch's five remaining
`GtkTreeRowReference` reference lines are private to the owner. No direct per-session
store or row-reference ownership remains in `fe-gtk.h`, `maingui.c`, or
`userlistgui.c`. The shared GTK3 user view remains for pass 4, so the
`GtkTreeView` inventory is now 75 lines across 18 files.

The visibility helper is limited to 17 reviewed roots whose descendants have
finished construction and have no intentional hidden state at reveal time.
The boundary deliberately does not abstract generic widget destruction,
remaining mixed start/end box ordering, menu/item visibility, events, or
list/tree models. Those operations have GTK4 lifetime
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
corresponding action, and the typed top-level state observer remains
authoritative for correcting fullscreen state after a platform transition.

Top-level minimized, maximized, fullscreen, and focused transitions now cross
one `FabulorWindowState` boundary. GTK4 observes a realized `GdkToplevel` and
owns its state-notify lifetime; GTK3 event types and `GdkWindowState` remain in
the adapter. Main-window preferences, minimize-to-tray behavior, relayout,
fullscreen action synchronization, tray action refresh, and the Windows
auto-hide taskbar adjustment consume the same snapshot. GTK4 native Windows
message filtering and non-main-window geometry remain separate work.

Main-window and detached-dialog geometry now crosses one
`FabulorWindowGeometry` observer. GTK4 owns a realized `GdkSurface` layout
connection and publishes application-pixel dimensions without coordinates;
GTK3 keeps configure events, window size reads, and available positions inside
the owner. Preference persistence and deferred main-window relayout consume the
same snapshot. DCC and Server List configure callbacks remain for their own
workflow passes.

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

The channel-view tab popup now has live retained GTK4 presentation. A
toolkit-neutral model owns effective per-session options and copied configured
command data, recursively projects `tabmenu.conf`, and composes the `$TAB`
plugin section. The clicked source owns the shared popover presenter and its
action groups; replacement and source destruction defer teardown beyond the
active dispatch callback. GTK3 retains its existing widget popup unchanged.

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
clipboards. Transcript-owned selection and clipboard code is now isolated in
`xtext-selection.c` for the custom-widget stage.

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
GTK4 drag icons are live `GtkWidgetPaintable` instances owned by the drag
source. The legacy native-window capture, Cairo readback, pixel conversion, and
scaled `GdkPixbuf` icon compile only for GTK3.
The former action-code identity and stale leading-character target test are
removed. The remaining consumer-side `GtkSelectionData` reference is contained
inside the GTK3 branch of the Stage 6 transcript selection adapter, not
drag/drop or transcript content logic.

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
| `gdk_window_*` | 32 | 8 | `GdkSurface`, snapshots, controllers, or removal | 4/6 | in progress; top-level state contained |
| `gtk_clipboard_*` | 1 | 1 | `GdkClipboard` and content providers | 4/6 | in progress |
| `GtkTreeView` | 75 | 18 | choose GTK4 list/model widget per workflow | 5 | in progress; Notify and user model owners converted |
| `GtkStatusIcon` | 6 | 1 | native Win32 tray or supported external backend | 7 | action model complete; presentation not started |
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
| `src/fe-gtk/sexy-spell-entry.c` | 1,424 | 118 | `GtkEntry` subclass, Enchant backend, menus | 6 |
| `src/fe-gtk/spell-entry-style.c` | 330 | 43 | Pango IRC formatting and spell attributes | 6 |
| `src/fe-gtk/spell-entry-menu.c` | 190 | 34 | dynamic spelling, formatting, and colour action model | 6 |
| `src/fe-gtk/spell-entry-widget.c` | 40 | 11 | cross-version pointer and redraw boundary | 6 |
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

Stage 6 transcript render-target pass 1 replaces the widget's raw draw-window,
draw-surface, and draw-context fields with one owned destination boundary.
Existing Cairo/Pango rendering now consumes referenced contexts from that
owner. GTK3 retains its `GdkWindow` fallback privately, while GTK4 can open and
close a Cairo context through `GtkSnapshot` and produce a `GskRenderNode`.
Window geometry, class virtual methods, selection, clipboard ownership, and
input events remain for following passes.

Stage 6 transcript geometry pass 2 moves wrapping, rendering, selection-scroll,
visibility, and buffer-switch dimensions behind `xtext-geometry.c`. The owner
uses GTK3 allocations and GTK4 widget width/height, rejects non-positive sizes,
and leaves native window dimensions only in the GTK3 window-to-surface capture
helper. Native windows remain where GTK3 still requires pointer lookup and
smooth-scroll capture, but no longer define transcript layout.

Stage 6 transcript widget-class pass 3 moves the versioned class signatures to
`xtext-widget-class.c`. GTK3 preferred-size, allocation, realize/unrealize, and
draw slots share one callback contract with GTK4 measure, allocation,
realize/unrealize, and snapshot slots. GTK4 snapshot dispatch uses the existing
geometry and render-target owners; `GtkXText` retains only content lifecycle,
resize state, and the contained GTK3 native-window work. Direct event and
selection slots remain assigned to the following controller and clipboard
passes.

Stage 6 transcript input-controller pass 4 removes button press/release,
motion, leave, and scroll event slots from `GtkXTextClass`. GTK3 signals and
GTK4 motion, gesture-click, scroll, and focus controllers feed normalized
coordinates, modifiers, click counts, and deltas into the existing selection,
separator, URL-hover, tooltip, and scrolling logic. `FabulorXTextClick`
replaces raw `GdkEventButton` word-click payloads, and coordinate-based menu
entry points preserve popup behavior. The only remaining direct transcript
event slots are GTK3 selection ownership and selection payload delivery.

Stage 6 transcript selection pass 5 removes those final direct event slots.
`xtext-selection.c` registers GTK3 targets after realization and delivers the
same UTF-8, text, compound-text, and locale payloads through signals. Its GTK4
branch publishes an owned string `GdkContentProvider` to CLIPBOARD and PRIMARY,
uses provider identity to distinguish its own update from replacement, and
disconnects change observation before widget teardown. `xtext.c` now owns only
selection ranges and text production; it contains no toolkit clipboard,
selection event, target, atom, or payload type.

Stage 6 transcript frame-redraw pass 6 removes the full-page renderer's
unconditional native-window gate. `xtext-scroll-copy.c` calculates GTK3
surface-copy and damage geometry independently of the widget. GTK4 reports no
native capture and falls through to complete snapshot rendering, while GTK3
keeps its existing optimized copy when overlap is bounded. CSS class
attachment, partial redraw requests, and top-level focus checks now use shared
cross-version helpers. Native window capture, pointer lookup, and background
window setup are compiled only in the GTK3 branch.

Stage 6 transcript background-composition pass 7 moves the optional source
surface and its fitted or tiled viewport cache into `xtext-background.c`.
Image sources preserve aspect ratio and use black letterboxing; non-image
sources repeat with the existing tile offsets; invalid, absent, oversized, or
uncomposable sources use the transcript palette background. The cache exists
only within a frame and all Cairo references are released by the owner, so
`GtkXText` no longer carries background cache geometry or render-cycle fields.

Stage 6 transcript decorations pass 8 moves marker geometry, search-match
boundary flags, and transient hover-highlight state into
`xtext-decoration.c`. Search discovery and IRC text parsing remain in
`xtext.c`, while the owner supplies deterministic start/mid/end/current
classification, marker placement before an entry or after its predecessor,
and bounded hover paint/clear state. This removes the widget's individual
highlight entry, offsets, inside, clear, and render-only fields without adding
a GTK dependency to the policy.

Stage 6 transcript hit-testing pass 9 moves scrollback-line calculation,
separator proximity, stripped-format match adjustment, and bounded hit-result
handling into `xtext-hit-test.c`. `word_click` now carries the already captured
classification and byte range beside its normalized click. The main-window
consumer duplicates that range for URL, host, nickname, channel, and email
actions rather than re-running classification, depending on global
`url_last()` state, or writing a terminator into the transcript scratch buffer.

Stage 6 transcript accessibility and display-scale pass 10 assigns the custom
widget a cross-version log role and stable `Transcript` accessible label.
`xtext-display.c` now owns the established Pango metric rounding, decoration
coordinates, inline-image bounds, scale normalization, and device-to-logical
conversion. Flag bitmaps load at widget scale for sharp output while wrapping,
hit testing, and cursor placement retain logical dimensions. Full accessible
scrollback text exposure and production GTK4 screen-reader validation remain
separate from this role-and-name baseline.

Stage 6 transcript accessible-text pass 11 implements GTK4's read-only
`GtkAccessibleText` interface over a recent valid UTF-8 snapshot. The snapshot
omits hidden IRC runs, follows timestamp visibility, is bounded to 1 MiB, and
uses Pango Unicode boundaries for character, word, sentence, line, and
paragraph queries. Append bursts, trimming, clears, timestamp changes, and
buffer switches coalesce into one idle refresh with minimal removal/insertion
notifications after the interface is first queried; ordinary sessions retain
no snapshot maintenance cost. GTK3 retains its existing ATK log role/name; accessible
selection mutation and text geometry remain unsupported for this read-only log.

Status: `in progress`

### Input: `SexySpellEntry`

The current implementation subclasses `GtkEntry` and owns Enchant-backed
underline, suggestion, personal dictionary, and replacement behaviour. GTK4
input controllers, Pango styling, and dynamic menu/action ownership are now
explicitly contained, and URI-shaped tokens are excluded from Enchant work.

Migration decision: retain the GTK4-compatible custom entry subclass and the
toolkit-owned editable implementation rather than introducing a delegated
child text control.

Required preserved behaviour:

- no measurable typing or Enter-to-send regression
- URL paste stability
- suggestions and replacement
- add-to-dictionary and persistent personal dictionary
- active-language handling and nickname exceptions
- IRC formatting and completion integration

Status: `in progress; Stage 6 spell-input pass 6 emoji-picker ownership boundary`

Stage 8 dialog icon-sizing pass 27 removes the quit dialog's direct dependency
on the retired `GTK_ICON_SIZE_DIALOG` enum. GTK4 constructs the named warning
image and applies an explicit 48-pixel presentation size; GTK3 continues using
its theme-defined dialog icon size through the compatibility boundary. The
strict probe verifies both the GTK4 icon identity and pixel-size contract.

Stage 8 tab-menu dependency pass 28 makes the retained GTK4 Autojoin and
Auto-Connect paths include their owning Server List declarations directly.
`maingui.c` no longer relies on unrelated GTK3 include chains for `ircnet`,
`servlist_save()`, or `servlist_autojoinedit()`. Runtime behavior and ownership
remain unchanged; this is an explicit compile-boundary correction.

Stage 8 flat-button presentation pass 29 replaces all seven main-window
`GTK_RELIEF_NONE` call sites with one cross-version semantic helper. GTK4 uses
the standard `flat` CSS class for channel-mode toggles, emoji choices, search
controls, reply cancellation, and the nickname button. GTK3 keeps its existing
relief setting privately inside the compatibility boundary.

Stage 8 Alt-modifier pass 30 makes the existing `STATE_ALT` boundary resolve
to GTK4's `GDK_ALT_MASK` or GTK3's `GDK_MOD1_MASK`. Key normalization and
loading, menu dispatch, Ctrl+A filtering, and user-list type-to-input behavior
now consume that shared semantic name. Accelerator text and stored key-binding
formats remain unchanged.

Stage 8 frame-presentation pass 31 replaces the active main-window shadow-type
calls with semantic framed-scroller and outlined-frame helpers. GTK4 uses the
standard `frame` CSS class for transcript and topic scrollers and lets
`GtkFrame`'s own CSS node render meter outlines. GTK3 keeps the exact `NONE`,
`IN`, and `OUT` shadow styles inside the compatibility boundary.

Stage 8 scroll-to-bottom pass 32 replaces the custom drawing area with a real
themed icon button. The control now owns valid button activation, a
`go-bottom-symbolic` image, flat presentation, tooltip, and explicit accessible
label on both toolkit versions. The raw button event mask, app-paintable flag,
custom Cairo arrow, and invalid drawing-area `clicked` connection are removed.

Stage 8 icon-size pass 33 removes `GtkIconSize` from the shared resolver and
image utility contract. Fabulor now owns semantic 16-pixel menu and 24-pixel
large-toolbar roles. GTK3 maps those roles to native toolkit icon roles for
named images; GTK4 applies explicit logical pixels. Resolved pixbuf assets use
the owned 16- and 24-pixel values without deprecated size lookup. Search
controls, channel tabs/lists, menus, Join, spell, plugin, and pixmap consumers
use the same type.

Stage 8 button-box layout pass 34 replaces all active button-box construction
with a Fabulor-owned start, end, and spread contract. GTK3 retains native
`GtkButtonBox` layout and spacing behind the compatibility boundary. GTK4 uses
ordinary oriented boxes, homogeneous allocation for spread rows, and explicit
axis alignment for start/end groups. Ban, DCC, editor, key-binding, Ignore,
Notify, Add-ons, Raw Log, Server List, Preferences, Print Events, and URL
History workflows no longer construct the removed widget directly.

Stage 8 DCC geometry pass 35 replaces the detached transfer window's raw
`configure-event` callback and `GdkEventConfigure` dependency with the shared
window-geometry observer. GTK3 continues reading configure-event dimensions
inside that owner; GTK4 observes `GdkSurface::layout`. Width and height are
still retained only for detached utility windows, while tabbed DCC utilities
continue to leave the saved detached size unchanged.

Stage 8 channel-view presentation pass 36 routes the tab strip's unframed
scroller and the tree switcher's inset-framed scroller through the existing
semantic presentation helper. GTK4 toggles the standard `frame` CSS class;
GTK3 retains `GTK_SHADOW_NONE` and `GTK_SHADOW_IN` inside the compatibility
boundary. A full shipping rebuild also removed the deprecated GTK3 icon-size
lookup exposed by `GTK_DISABLE_DEPRECATED`, using Fabulor's existing 16- and
24-pixel roles for resolved pixbuf assets.

Stage 8 dialog-window hint pass 37 moves all active
`GDK_WINDOW_TYPE_HINT_DIALOG` use behind a semantic compatibility helper. GTK3
continues applying the window-manager hint to Join, shared utility, Server
Editor, and Network List windows. GTK4 relies on each workflow's concrete
dialog type plus existing transient and modal relationships; no removed hint
enum reaches active GTK4 code.

Stage 8 Channel List entry-text pass 38 replaces its four removed
`gtk_entry_get_text()` calls with a typed borrowed-text helper. GTK4 reads the
entry through `GtkEditable`; GTK3 retains its native entry getter privately.
Wildcard presence, glob matching, case-insensitive matching, and regular
expression compilation continue to consume the same widget-owned UTF-8 text
without allocating or changing its lifetime.

Stage 8 file-chooser path pass 39 replaces direct filename, filename-list, and
current-folder path APIs with one local-path owner. Both toolkit versions now
cross the chooser boundary through `GFile`; callers receive duplicated local
filesystem paths with explicit ownership, and multiple-selection order is
preserved. GTK 4.10 deprecated `GtkFileChooser` itself, so that temporary API
is isolated in `file-chooser-path.c` under a narrow deprecation scope pending
the later asynchronous `GtkFileDialog` cutover. No deprecation suppression
reaches callers or the general compatibility header.

Stage 8 top-level constructor pass 40 replaces all three active
`gtk_window_new(GTK_WINDOW_TOPLEVEL)` calls with one typed constructor. GTK4
uses its argument-free top-level constructor; GTK3 retains the explicit window
type privately. Shared utility windows, Server Editor, and Server List retain
their existing title, role, size, transient, modal, theme, and destruction
configuration. The two remaining literal constructors are confined to GTK3
theme tests and are not part of the GTK4 frontend profile.

Stage 8 legacy popup-builder pass 41 removes the generic GTK3 menu-widget
builder from the GTK4 compilation surface. Check items, quick items, nested
submenus, configured popup-list expansion, and legacy popup destruction remain
available only to GTK3 callers; GTK4 context workflows already use their typed
`GMenuModel` owners and popover presenter. Icon-label parsing and executable
path filtering remain shared because GTK4 model projection still consumes
them. The legacy builder declarations are likewise GTK3-private in `menu.h`.

Stage 8 Away callback pass 42 makes the old `GtkCheckMenuItem` synchronization
callback GTK3-private. GTK4 continues dispatching the stateful `away-toggle`
action through the shared action descriptor and command handler. The GTK3
fallback retains its exact active-state read and signal-blocking identity for
programmatic synchronization; no removed check-menu type reaches GTK4.

Stage 8 `/MENU` widget-mutation pass 43 makes GTK3 menu-item lookup, path
walking, add/delete/update callbacks, radio and toggle constructors,
accelerator attachment, ordering, and popup injection GTK3-private. The shared
plugin tree, copied action data, `GMenuModel` projection, label normalization,
and `fe_menu_sync()` refresh remain active for GTK4. Core `/MENU` add, delete,
and update paths already finish with that refresh, so GTK4 rebuilds retained
main and contextual models instead of mutating retired widget trees.

Stage 8 system-icon pixbuf pass 44 replaces `gtk_icon_theme_load_icon()` and
`GTK_ICON_LOOKUP_FORCE_SIZE` in shared pixmap fallback loading. GTK4 looks up a
`GtkIconPaintable` from the current display, borrows its `GFile`, opens an owned
stream, and decodes an owned `GdkPixbuf` at Fabulor's menu pixel role. GTK3
retains its native forced-size loader privately. Existing resource/file
precedence and subsequent `GDK_SCALE` handling are unchanged.

Stage 8 window-surface ownership pass 45 routes tray visibility decisions,
plugin `win_status`, plugin native-window pointers, Win32 tray-menu ownership,
native titlebar styling, and taskbar adjustment through `window-state`. GTK4
reads minimized state from `GdkToplevel` and native handles from a Win32
`GdkSurface`; GTK3 retains `GdkWindow` state and handle access privately.
Restoring a GTK4 window relies on the shared show/present sequence, while GTK3
keeps its explicit deiconify call.

Stage 8 Raw Log framing pass 46 adds a cross-version scrolled-window
constructor beside the existing semantic frame helper. Raw Log now uses GTK4's
argument-free constructor and standard `frame` CSS class, while GTK3 retains
its null-adjustment constructor and `GTK_SHADOW_IN` presentation privately.
Expansion, scroll policies, transcript ownership, theme sampling, buttons, and
keyboard copying are unchanged.

Stage 8 Server List lifecycle and geometry pass 47 moves both Server List
window sizes to the shared surface-layout observer and gives the main list and
network editor typed close callbacks. The editor preserves its save-before-
destroy contract and consumes the close request after destroying itself; the
main list preserves configuration saving, startup-exit behavior, and normal
window destruction. Server List save and validation paths now read entry text
through the borrowed cross-version helper. Legacy GTK3 event structures and
direct window-size reads no longer enter this workflow. Detailed editor and
main-list scroller construction remains the next contained Server List target.

Stage 8 Server List scroller pass 48 moves the three network-editor list
scrollers and the main network-list scroller onto the established cross-version
constructor and semantic frame helper. GTK4 uses the argument-free constructor
and standard `frame` CSS class; GTK3 retains null adjustments and inset shadow
presentation privately. Existing scroll policies, notebook ownership,
tooltips, typed list views, and selection behavior are unchanged. The complete
GTK4 inventory now reports no hard errors in `servlistgui.c`.

Stage 8 Preferences framing pass 49 moves each lazily populated Preferences
page scroller to the shared constructor, semantic frame presentation, and
explicit child owner. GTK4 no longer inspects an internal bin or viewport.
GTK3 privately retains its shadowless auto-created viewport so the existing
single inset border is preserved rather than doubled. Page labels, vertical
scroll policy, notebook ownership, lazy creation, and page content are
unchanged. The complete GTK4 inventory now reports no hard errors in `setup.c`.

Stage 8 user-list framing pass 50 moves the main user-list scroller to the
shared cross-version constructor and semantic frame helper. Its typed list view
continues to attach through the explicit scroller child boundary. Expansion,
automatic scrolling, minimum content width, drag/drop, pointer and keyboard
input, model ownership, and selection behavior are unchanged. The complete
GTK4 inventory now reports no hard errors in `userlistgui.c`; every remaining
hard frontend error is within the theme compatibility boundary.

Stage 8 theme window-ownership pass 51 separates the GTK3 KDE/Wayland CSD
workaround from GTK4 top-level presentation. GTK3 privately retains its
header-bar sizing, icon, title synchronization, and `GdkScreen` style reset.
GTK4 keeps compositor-managed CSD and applies the existing `zoitechat-dark` or
`zoitechat-light` marker directly through widget CSS classes. Native Windows
title-bar dispatch and attached-window lifetime are unchanged. The complete
GTK4 inventory now reports no diagnostics in `theme-manager.c`.

Stage 8 theme provider-ownership pass 52 centralizes application-wide CSS
provider installation and removal in `theme-css`. GTK4 resolves the default
`GdkDisplay` and uses display-scoped provider APIs; GTK3 privately retains the
default `GdkScreen` path. Callers may preserve their reviewed priority, and a
shared CSS string loader owns the GTK3/GTK4 signature difference. Top-level
application CSS remains at application priority plus one, while input and
palette providers retain user priority and existing lifetimes. The complete
GTK4 inventory now reports no diagnostics in `theme-application.c` or
`theme-css.c`.

Stage 8 theme style-access pass 53 makes widget style-context palette sampling
an explicit GTK3 adapter path. GTK3 custom themes retain foreground,
background, selection, and link-accent sampling across their historical state
flags. GTK4 does not call removed style/background accessors and continues to
resolve transcript and widget colors through the semantic runtime palette.
Default IRC colors, user overrides, dark/light mode selection, and RGB16
conversion are unchanged. The complete GTK4 inventory now reports no
diagnostics in `theme-access.c`.

Stage 8 GTK3 theme-adapter containment pass 54 compiles the legacy CSS,
settings, and provider implementation only for GTK3. GTK4 builds the same
production source through an inert compatibility contract: setup and refresh
calls succeed harmlessly, the adapter never reports itself active, and theme
variant probing returns the established light default without loading a GTK3
provider. Both strict MSVC and Meson GTK4 probes compile and execute that
contract. The clean complete GTK4 inventory advances from 37 ordinary errors /
351 warnings to zero ordinary errors / 336 warnings before stopping at the
retired `gdk/gdkwin32.h` include in `xtext.c`.

Stage 8 Xtext Win32-header containment pass 55 removes the transcript's unused
private GTK3 Win32 GDK include together with redundant direct Windows and GDK
headers. The Windows transcript-export path retains its required CRT file
descriptor declarations, executable-relative flag lookup still uses GLib's
public Win32 helper, and Cairo integration remains on the public GDK header.
The clean complete GTK4 profile now compiles every production frontend source
with zero C compiler errors and 336 warnings, then reaches link for the first
time. Link closure currently reports 89 unique unresolved symbols across 238
diagnostic lines; the missing shared GTK4 list-model implementation is the
first build-input target.

Stage 8 GTK4 list-model link-input pass 56 adds the converted shared flat and
tree model-stack implementation to the production MSVC project only when the
GTK4 profile is selected. GTK3 does not compile or link the GTK4-only source.
The strict probe continues to exercise model construction, sorting, selection,
mutation, and cleanup from the same implementation. The complete GTK4 linker
inventory drops from 89 to 76 unique unresolved symbols and from 238 to 166
repeated unresolved-symbol diagnostics, closing all 13 shared list-model
symbols without changing the 336-warning compile inventory.

Stage 8 entry compatibility link-closure pass 57 completes the typed entry
helper family for borrowed text reads, text replacement, and width requests.
GTK4 routes all three operations through `GtkEditable`; GTK3 privately retains
the corresponding `GtkEntry` calls. Active frontend callers and spell-entry
macros now use that boundary. The complete GTK4 linker inventory drops from 76
to 73 unique unresolved symbols and from 166 to 144 repeated diagnostics, while
the warning inventory falls from 336 to 278 with zero C compiler errors.

Stage 8 container-inset link-closure pass 58 replaces GTK3 container
`border-width` presentation with a semantic uniform-inset owner. GTK3 retains
its original container property. GTK4 uses four equal widget margins and stores
top-level requests until the window child is attached, including explicit zero
insets. The complete GTK4 inventory improves from 73 to 72 unique unresolved
symbols, from 144 to 121 repeated linker diagnostics, and from 278 to 255
warnings while retaining zero C compiler errors.

Stage 8 typed box-child attachment pass 59 routes confirmed box and button-box
children through the existing cross-version box owner, and routes the channel
tab viewport child through the typed scrolled-window owner. GTK3 preserves the
original non-expanding, filling `gtk_container_add` placement. Event surfaces,
list rows, legacy menus, lazy preference pages, and reparenting remain explicit
follow-up ownership boundaries. The complete GTK4 inventory retains zero C
compiler errors and 72 unique unresolved symbols while improving from 255 to
245 warnings and from 121 to 111 repeated linker diagnostics.

Stage 8 content-surface and list ownership pass 60 replaces active event-box
wrappers with a semantic single-child content surface. GTK3 retains real event
boxes and visible-window policy; GTK4 uses CSS-capable boxes with explicit child
ownership. Theme color-manager rows and list insertion now use typed
`GtkListBoxRow` and `GtkListBox` owners. Strict MSVC and Meson runtime checks
verify transparent and visible surface children plus row ownership and order.
The complete GTK4 inventory retains zero C compiler errors while improving from
245 to 233 warnings, from 111 to 104 repeated linker diagnostics, and from 72 to
69 unique unresolved symbols. All retired event-box symbols are closed.

Stage 8 lazy Preferences page ownership pass 61 replaces GTK3 child-list
inspection with explicit per-page creation state. Registered page containers
are known vertical boxes, so page factories now attach once through the typed
box owner and reveal through the cross-version widget-tree boundary. The
complete GTK4 inventory retains zero C compiler errors and 69 unique unresolved
symbols while improving from 233 to 228 warnings and from 104 to 100 repeated
linker diagnostics. `setup.c` no longer calls `gtk_container_get_children`,
`gtk_container_add`, or `gtk_widget_show_all`.

Stage 8 layout reparent ownership pass 62 gives movable channel-view and user-
list roots a retain-and-detach boundary restricted to their known `GtkPaned` or
`GtkGrid` owners. GTK4 clears the matching pane slot or removes the grid child;
GTK3 privately retains generic container removal. Initial and subsequent pane
attachment now use the typed start/end helpers. Strict MSVC and Meson runtime
checks cover pane and grid detach, temporary lifetime, reattachment, and the
unparented case. The complete GTK4 inventory retains zero C compiler errors
while improving from 228 to 225 warnings, from 100 to 97 repeated diagnostics,
and from 69 to 66 unique unresolved symbols. `gtk_container_remove`,
`gtk_paned_pack1`, and `gtk_paned_pack2` are closed from the GTK4 link boundary.

Stage 8 Channel List context-menu pass 63 replaces the active GTK3 widget menu
with a retained GTK4 menu/action model and the shared popover presenter. The
model owns copied channel and topic selections, preserves Join, copy-channel,
copy-topic, icons, and first-channel Autojoin behavior, and safely outlives
changes to the live list selection. GTK3 keeps its original icon menu privately.
Strict MSVC and Meson tests release the source arrays before activating all four
actions. The complete GTK4 inventory retains zero C compiler errors while
improving from 225 to 213 warnings, from 97 to 87 repeated diagnostics, and from
66 to 61 unique unresolved symbols. `gtk_container_add`, `GTK_MENU`,
`gtk_menu_item_new`, `gtk_menu_new`, and `gtk_menu_popup_at_pointer` are closed
from the GTK4 link boundary.

Stage 8 icon/mnemonic button and Channel List lifecycle pass 64 composes each
GTK4 image and mnemonic label as the explicit child of its button while GTK3
retains native image-button presentation. Channel List and the plugin manager
share the typed constructor, and Channel List construction failures now close
through the cross-version window lifecycle helper. Strict MSVC and Meson tests
verify child order, icon identity and size, visible mnemonic text, and mnemonic
association. The complete GTK4 inventory retains zero C compiler errors while
improving from 213 to 210 warnings, from 87 to 84 repeated diagnostics, and
from 61 to 60 unique unresolved symbols. `gtk_button_set_image` is closed from
the GTK4 link boundary.

Stage 8 Channel View ownership and lifecycle pass 65 moves tab and tree
scrollers to the shared constructor, translates position-based family/tab
ordering to GTK4 sibling ownership, and keeps close-button image policy private
to GTK3. Tab, family, tree, and implementation roots are removed through their
known box owners; recursive reveal uses the shared tree boundary. GTK3 retains
its root `destroy` callback, while GTK4 releases Channel View state when the
main-window-owned root is finalized. Strict probes verify first, middle, and
last box ordering. The complete GTK4 inventory retains zero C compiler errors
while improving from 210 to 204 warnings, from 84 to 80 repeated diagnostics,
and from 60 to 58 unique unresolved symbols. `gtk_box_reorder_child` and
`gtk_button_set_always_show_image` are closed, and `chanview.obj` contributes no
remaining GTK4 warning or unresolved diagnostic.

Stage 8 Join Channel dialog lifecycle pass 66 replaces GTK3 radio-button groups
with a typed choice abstraction: GTK3 retains native radio buttons and GTK4
uses grouped check buttons. Active-state access also covers the dialog's normal
checkbox without invalid GTK4 toggle-button casts. Label wrapping, root-window
lookup, default response ownership, and dialog pointer cleanup now use explicit
cross-version semantics. Strict probes verify three-way exclusivity, ordinary
checkbox state, wrapping, parented and unparented root lookup, and finalization.
The complete GTK4 inventory retains zero C compiler errors while improving from
204 to 191 warnings, from 80 to 72 repeated diagnostics, and from 58 to 56
unique unresolved symbols. `gtk_radio_button_set_group` and
`gtk_label_set_line_wrap` are closed, and `joind.obj` contributes no remaining
GTK4 warning or unresolved diagnostic.

## Functional Clusters

| Cluster | Main files | GTK4 concern | Status |
|---|---|---|---|
| Main windows and tabs | `maingui.c`, `chanview*.c` | child ownership, gestures, DnD, focus | in progress |
| Transcript | `xtext.c`, `xtext.h`, `xtext-render-target.c` | snapshot rendering and event model | in progress; render destination ownership established and private Win32 GDK header retired |
| Edit box and spell check | `maingui.c`, `emoji-picker.c`, `sexy-spell-entry.c`, `spell-entry-*.c` | production interaction and latency validation | in progress; typed entry text/width, word, lifecycle, styling, menu, URL, and emoji-picker boundaries established |
| Menus and commands | `menu.c`, `maingui.c`, `plugin-tray.c` | actions and menu models | in progress; retained main/context/tab presentation and GTK3-only accelerator ownership established |
| Operational lists | `servlistgui.c`, `chanlist.c`, `userlistgui.c`, `dccgui.c`, `banlist.c`, `notifygui.c`, `ignoregui.c`, `plugingui.c`, `urlgrab.c` | list models, factories, editing | converted; toolkit-specific models and views are contained and the shared GTK4 model stack is a production candidate input |
| Preferences/editors | `setup.c`, `fkeys.c`, `textgui.c`, `editlist.c` | generic edit list, Print Events, key bindings, sound events, and preference navigation converted | converted |
| Themes | `theme/*.c`, `common/gtk3-theme-service.c`, `common/gtk4-theme-*.c` | GTK4 CSS compatibility and adapter policy | in progress; pre-production GTK4 theme stack composed behind a lifecycle controller |
| Platform integration | `fe-gtk.c`, `plugin-tray.c`, notifications | displays, surfaces, icons, native tray | not started |

## Theme Inventory

- `theme/theme-manager.c`, `theme-policy.c`, `theme-runtime.c`,
  `theme-application.c`, and `theme-css.c` should remain toolkit-neutral where
  practical.
- `theme/theme-gtk3.c` is the explicit GTK3 adapter. Its implementation is
  compiled only for GTK3; its tested GTK4 branch is inert compatibility code
  pending final removal with the rest of the GTK3 frontend.
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
