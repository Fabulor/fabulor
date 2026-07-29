# GTK4 API Inventory

Status: completed GTK4 inventory

Baseline date: 2026-07-14
Completion date: 2026-07-27

## Method

This inventory combines source review, strict GTK4 compilation, literal symbol
searches under `src/fe-gtk` and supported plugins, runtime/import validation,
and installed-client acceptance. The final audit distinguishes active
production code from comments, tests, archived documents, and the historical
migration record retained below.

Completion means:

- the supported MSVC/WiX application graph is GTK4-only
- active frontend and supported plugin source contains no toolkit-version gate
  or GTK3 branch
- retired GTK3 API families do not compile in production
- the isolated strict MSVC and Meson/Ninja GTK4 probes pass
- negative validators continue to reject GTK3 headers, DLLs, imports, runtime
  roots, theme files, and source reintroduction

## Build Boundary

| Area | Current state | GTK4 target | Status |
|---|---|---|---|
| Meson frontend dependency | inherited application graph removed; isolated `tools/gtk4` probe retained | keep the probe separate from the supported MSVC/WiX production build | retired |
| MSVC headers/libraries | GTK4 root supplies GTK/GLib and related libraries; pinned vcpkg root supplies OpenSSL | retain explicit, reproducible split roots | converted |
| Windows CI build dependencies | one pinned GTK4 archive, vcpkg OpenSSL, and explicit non-GTK support inputs | no GTK3, Lua, Perl, gendef, or MSYS2 augmentation | converted |
| Windows runtime payload | allowlisted GTK4 runtime staged from the compile root | same audited runtime used by the executable | converted |
| Staged release root | deterministic GTK4 runtime, support, plugin-host, and Enchant roots | GTK4-only final layout | converted |

## Compatibility Helper Boundary

`src/fe-gtk/gtk-compat.h` is now a GTK4-only, header-only helper boundary for:

- start-ordered box insertion with explicit expansion, fill, and padding
- horizontal trailing-child insertion with preserved end alignment
- ordered insertion immediately before a permanent trailing child
- trailing label/control pair insertion with preserved end alignment
- box-owned dynamic child removal
- window, scrolled-window, frame, button, overlay, and popover child assignment
- completed-tree reveal with explicit GTK4 root semantics
- window destruction
- dialog-response destruction with an exact GTK signal callback signature
- standard and primary text clipboard updates through the widget display
- closure-owned pointer-enter and focus interactions using GTK4 event controllers
- smooth/discrete scroll normalization through a capture-phase GTK4 controller

The production MSVC build and isolated GTK4 probes compile the same helper
bodies. The probes also take each helper's address so every retained helper is
linked, not merely preprocessed. Production now uses 40 typed child
assignments across 14 source files: 7 windows, 18 scrolled windows, 7 frames,
6 buttons, 1 overlay, and 1 popover.

Production now also uses 156 reviewed start-ordered box additions across 17
files, two horizontal trailing children, two ordered insertions before a
permanent trailing child, three trailing label/control pairs, and five
box-owned dynamic child removals.

The shared operational-list and user-list constructors accept typed GTK4
parents. Models, factories, selection, and drag/drop are specialized to GTK4
and contain no classic tree-view implementation.

## Completion Audit

| Boundary | Final state | Status |
|---|---|---|
| Application build graph | MSVC/WiX only; inherited Makefile and application Meson fragments removed | retired |
| Isolated compile probes | strict MSVC and `tools/gtk4` Meson/Ninja probes retained and passing | converted |
| Toolkit-version gates | zero active `GTK_MAJOR_VERSION`/GTK3 source branches | retired |
| Containers and layout | explicit GTK4 child ownership, box ordering, grids, and pane geometry | converted |
| Operational lists | typed `GListModel` owners, factories, and GTK4 selection models | converted |
| Menus and actions | `GAction`, `GMenuModel`, and owned popover presentation | converted |
| Input and events | GTK4 controllers, gestures, clipboard, drag/drop, and focus ownership | converted |
| Transcript | GTK4 snapshot rendering, selection, hit testing, scrolling, and accessible text | converted |
| Themes | GTK4 CSS/runtime palette plus bounded `.hct` import; GTK3 theme service retired | converted |
| Tray and windows | Windows Shell tray plus GTK4 surface, lifecycle, and geometry ownership | converted |
| Supported plugins | FiSHLiM, Python, Sysinfo, Exec, Checksum, Upd, and notifications build without GTK3 branches | converted |
| Production package | 7,624 installed files, zero GTK3 path markers, and verified runtime/import contracts | converted |

## Migration Record (Historical)

The following pass-by-pass record deliberately preserves then-current
descriptions of GTK3 and mixed-toolkit boundaries. Those statements document
the state at each migration pass; they are not descriptions of the completed
production source. The completion audit above and final functional clusters
below are authoritative.

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

## Final Retired-API Audit

The completion scan counts active compiled production paths rather than
comments or the historical record above.

| Retired family or type | Active matches | Final owner | Status |
|---|---:|---|---|
| toolkit-version gates | 0 | GTK4-only source | retired |
| `gtk_container_*` | 0 | typed widget child APIs | retired |
| `gtk_box_pack_*` | 0 | GTK4 box append/prepend/reorder APIs | retired |
| `gtk_widget_show_all` | 0 | explicit GTK4 visibility | retired |
| `GtkEventBox` / `gtk_event_box_*` | 0 | ordinary widgets and controllers | retired |
| `GtkTable` / `gtk_table_*` | 0 | `GtkGrid` | retired |
| blocking dialog/native-dialog run calls | 0 | response-driven dialog flow | retired |
| classic GTK widget menus/items | 0 | `GMenuModel`, actions, and popovers | retired |
| raw `GdkEvent*` frontend callbacks | 0 | controllers and gestures | retired |
| `GdkDragContext` / `GtkSelectionData` | 0 | GTK4 drag sources, drop targets, and content | retired |
| `gtk_widget_get_window` / `gdk_window_*` | 0 | `GdkSurface`, snapshots, and typed native access | retired |
| `gtk_clipboard_*` | 0 | `GdkClipboard` and content providers | retired |
| `GtkTreeView` family | 0 | GTK4 list/column views and typed models | retired |
| `GtkStatusIcon` / AppIndicator | 0 | Windows Shell notification-area backend | retired |
| screen-scoped CSS providers | 0 | display-scoped providers | retired |

GTK4-native APIs that retain historical names, including
`gtk_message_dialog_new()` and the isolated `GtkFileChooserNative` boundary,
are not GTK3 dependencies. They compile under the pinned GTK 4.22 headers;
the chooser boundary remains deliberately narrow because GTK 4.10 deprecated
that interface in favour of the asynchronous file-dialog API.

## Final High-Risk Boundaries

| Boundary | Current implementation | Status |
|---|---|---|
| Main window and tabs | GTK4 child ownership, controllers, drag/drop, surface lifecycle, and persisted pane geometry | converted |
| Transcript | GTK4 snapshot/render-target pipeline, exact selection/hit testing, accessible text, and bounded redraw scheduling | converted |
| Server List and Preferences | typed GTK4 models, response-driven dialogs, stable category sizing, and native chooser containment | converted |
| Menus and context actions | canonical actions, retained menu models, GTK4 popover presenters, and plugin overlay composition | converted |
| Spell input and emoji | GTK4 editable/controller ownership, Enchant 2, action menus, and allocation-aware emoji popover | converted |
| Themes | semantic palette, GTK4 display CSS, Windows appearance monitoring, and bounded `.hct` import | converted |
| Tray and platform integration | direct Windows Shell ownership and GTK4 window restoration | converted |

## Custom Widget Boundaries

### Transcript: `GtkXText`

The completed GTK4 widget uses measurement, allocation, snapshot rendering,
event controllers, clipboard content providers, and surface-independent hit
testing. Its former GTK3 virtual-method and selection-event implementations
are retired.

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

Status: `converted`

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

Status: `converted`

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

Stage 8 DCC grouped-choice pass 67 routes the Transfers window's Both, Uploads,
and Downloads filters through the shared grouped-control boundary. The GTK4
constructor now explicitly activates the first ungrouped member, preserving
GTK3 radio-button defaults for both DCC and the Join dialog. The DCC toggle
callback reads state without an invalid GTK4 toggle-button cast, and no GTK3
group-list internals cross the boundary. Strict MSVC and Meson probes verify
constructor-default activation and subsequent exclusivity. The complete GTK4
inventory retains zero C compiler errors and 56 unique unresolved symbols while
improving from 191 to 183 warnings and from 72 to 69 repeated diagnostics.
`dccgui.obj` contributes no remaining GTK4 warning or unresolved diagnostic;
Preferences is the sole remaining caller of the retired radio-button family.

Stage 8 Preferences grouped-choice pass 68 routes the Appearance page's Tabs
and Tree switcher choices through the same typed grouped-control boundary. The
blank layout slot continues to preserve the stored preference indices, while
the first real control anchors the group and GTK4 activation dispatch stores
the selected index without a retired toggle-button cast. GTK3 keeps native
radio controls. Strict MSVC and Meson probes remain clean, and the complete
GTK4 inventory retains zero compiler errors while improving from 183 to 178
warnings, from 69 to 66 repeated diagnostics, and from 56 to 53 unique
unresolved symbols. `gtk_radio_button_new_with_mnemonic`,
`gtk_radio_button_get_group`, and `GTK_RADIO_BUTTON` are now contained entirely
inside the GTK3 compatibility branch; no active frontend caller uses the
retired radio-button family.

Stage 8 Preferences check-button pass 69 moves all ordinary Preferences toggles,
the three-column alert matrix, and dependent-control sensitivity to typed
check-button state access. Callback signatures no longer claim that GTK4 check
buttons are `GtkToggleButton` instances, and each dependency update uses one
stable state read. GTK3 presentation and signal behavior are unchanged. Strict
MSVC and Meson probes remain clean. The complete GTK4 inventory remains at zero
compiler errors, 178 warnings, 53 unique unresolved symbols, and 66 repeated
diagnostics because genuine toggle buttons remain elsewhere. `setup.c` now has
no direct `GtkToggleButton`, `GTK_TOGGLE_BUTTON`, or toggle active-state call;
its sole remaining compiler/link diagnostic is the retired combo-box wrap-width
operation.

Stage 8 Preferences combo-wrap pass 70 contains the DCC speed-unit selector's
removed `gtk_combo_box_set_wrap_width` call behind a semantic single-column
policy. GTK3 retains its explicit one-column popup; GTK4 uses the native
single-column combo presentation. The helper signature is compiled under the
strict MSVC and Meson probes, while the full frontend profile compiles the live
Preferences call site. Shipping GTK3 remains clean. The complete GTK4 inventory
improves from 178 to 177 warnings, from 66 to 65 repeated diagnostics, and from
53 to 52 unique unresolved symbols. `setup.obj` now contributes no compiler or
linker diagnostics, completing the active Preferences source boundary.

Stage 8 main-window visibility and close-request pass 71 moves utility-window
presentation to typed root lookup, removes generic tabs and rebuilt user-list
button boxes through their known notebook/box owners, and routes completed
window and meter trees through the shared reveal policy. The tabbed main window
now receives GTK4 `close-request` while GTK3 retains `delete-event`, with one
shared policy preserving tray hiding, detached-window handling, and quit
confirmation. Shipping GTK3 and strict probes remain clean. The complete GTK4
inventory retains zero compiler errors and 52 unique unresolved symbols while
improving from 177 to 173 warnings and from 65 to 62 repeated diagnostics.
`maingui.obj` improves from 23 to 20 unresolved diagnostics; menu and theme
owners retain the remaining show-all and root symbols.

Stage 8 main-window finalization pass 72 gives detached and tabbed IRC windows
one lifecycle owner. GTK4 uses weak finalization while GTK3 retains its
`destroy` signal; both unregister the window theme listener before releasing
sessions. Detach/reattach explicitly suppresses that owner, cleans an obsolete
window before freeing its `session_gui`, and restores ownership only when the
shared tab window survives. The shared theme manager also uses weak ownership
for GTK4 windows, so it no longer registers GTK3's removed widget `destroy`
signal. Shipping GTK3 and strict probes remain clean. The complete GTK4
inventory is unchanged at zero compiler errors, 173 warnings, 52 unique
unresolved symbols, and 62 repeated diagnostics because this pass changes
runtime callback ownership rather than linked API families.

Stage 8 auxiliary finalization pass 73 replaces the generic utility-tab API's
untyped close callback with `GDestroyNotify`. GTK4 invokes one owned cleanup at
object finalization; GTK3 retains one bridged `destroy` callback. Ban List,
Channel List, transfers, DCC chat, editable lists, keyboard shortcuts, Ignore,
Plugins and Scripts, Friends, Raw Log, Print Events, and URL Grabber now share
that exact callback contract. Theme listeners, Channel List timers and model
state, and utility models are released by the same owner instead of separate
signal handlers. Quit and fatal-font dialog pointers use GTK4 weak pointers,
and the main user-list listener is disconnected before its `session_gui` can
be freed. Shipping GTK3 and strict probes remain clean. The complete GTK4
inventory remains at zero compiler errors, 173 warnings, 52 unique unresolved
symbols, and 62 repeated diagnostics.

Stage 8 Server List and Preferences finalization pass 74 gives both top-level
windows one versioned cleanup owner. GTK4 uses `GWeakNotify`; GTK3 keeps typed
widget `destroy` callbacks. Server List close/connect policy no longer clears
window or model globals ahead of object release, and the editor finalizer now
clears every borrowed control/list pointer plus securely held password data.
Preferences similarly releases sound/category models and staged theme state
once, while its font chooser has independent weak pointer cleanup. The native
client-certificate chooser replaces its GTK4 parent `destroy` handler with a
weak parent watch that is removed before response-owned unref. The complete
GTK4 inventory remains unchanged at zero compiler errors, 173 warnings, 52
unique unresolved symbols, and 62 repeated diagnostics.

Stage 8 theme-import chooser lifetime pass 75 gives the colour/HCT and legacy
GTK3-theme native choosers one parent-watch contract. GTK4 uses `GWeakNotify`;
GTK3 retains the widget `destroy` signal. Response ownership disconnects that
watch before dialog release, and weak owner lookup prevents callbacks from
reaching controls after Preferences closes. Direct theme-preference
`gtk_widget_get_toplevel()` calls now use the shared root-window adapter.
`gtk_file_chooser_set_local_only()` is contained in the owned-path adapter:
GTK3 requests local-only selection from the toolkit, while GTK4 rejects
non-local `GFile` values when projecting an owned filesystem path. The full
GTK4 inventory improves to zero compiler errors, 166 warnings, 50 unique
unresolved symbols, and 59 repeated diagnostics.

Stage 8 Server List widget boundary pass 76 removes direct `GtkBin`,
`gtk_box_pack_start()`, window-role, and widget-default calls from
`servlistgui.c`. Editable combo child discovery is typed and null-checked;
box expansion, fill, and padding use the shared attachment policy. GTK3 keeps
window roles and can-default/grab-default behavior, while GTK4 uses the window
default-widget API and treats obsolete roles as inert metadata. The shared
utility-window constructor uses the same role boundary, eliminating that
retired symbol from the complete frontend. The full GTK4 inventory improves
to zero compiler errors, 156 warnings, 46 unique unresolved symbols, and 52
repeated diagnostics. `servlistgui.c` contributes no remaining GTK4 compiler
diagnostics.

Stage 8 main-window child and pane boundary pass 77 removes active `GtkBin`,
image-pixbuf inspection, legacy paned child queries, and style-property handle
reads from `maingui.c`. The nickname button uses typed child lookup. Access
icons retain an owned source pixbuf for identity comparison while GTK4 renders
through `GdkTexture`. Shared paned accessors map start/end ownership, and
divider size derives from allocated GTK4 pane geometry while preserving GTK3's
style property. Right-pane restoration no longer connects GTK4 to the removed
`size-allocate` signal; a one-shot frame callback waits for complete child
allocation. The full GTK4 inventory improves to zero compiler errors, 142
warnings, 40 unique unresolved symbols, and 46 repeated diagnostics.

Stage 8 reply-bar visibility boundary pass 78 removes active
`gtk_container_foreach()` and `gtk_widget_set_no_show_all()` use from
`maingui.c`. A shared hidden-until-explicitly-shown helper preserves GTK3's
recursive `show_all()` exclusion and maps GTK4 to direct visibility. A typed
immediate-child reveal helper uses GTK4 sibling traversal and keeps GTK3
container iteration private. The full GTK4 inventory improves to zero compiler
errors, 140 warnings, 38 unique unresolved symbols, and 44 repeated
diagnostics.

Stage 8 legacy widget-menu constructor containment pass 79 makes the shared
main-window icon/submenu declarations and implementations GTK3-only, together
with tray widget-menu item creation, population, destruction, Win32 hover
tracking, and mutable item-label updates. GTK4 retains its tab context menu
model/presenter and the native Windows tray popup. The full GTK4 inventory
improves to zero compiler errors, 123 warnings, 33 unique unresolved symbols,
and 37 repeated diagnostics; `plugin-tray.c` contributes no compiler warnings
from legacy widget-menu construction.

Stage 8 legacy status-icon backend isolation pass 80 gives the removed
`GtkStatusIcon` implementation one explicit GTK3-only compile capability. Its
type declarations, compatibility prototypes, object state, signal callback,
and complete operation table cannot enter a GTK4 object. GTK4 builds without
AppIndicator retain an inert operation table and fail initialization closed,
matching the existing backend-selection policy. The shipping GTK3 fallback is
unchanged. The full GTK4 inventory remains at zero compiler errors and 123
warnings while improving from 33 to 29 unique unresolved symbols and from 37
to 29 repeated diagnostics. All four `gtk_status_icon_*` dependencies leave
the GTK4 link boundary.

Stage 8 application main-loop ownership pass 81 removes GTK4 use of the
retired GTK option group, argument-taking initialization, and global GTK main
loop. `application-main-loop.c` owns one default-context `GMainLoop`, preserves
shutdown requested before entry, quits safely while active, and rejects active
destruction. The strict probe covers both pre-run and active shutdown. GTK3
keeps its existing option group, `gtk_init(&argc, &argv)`, `gtk_main()`, and
`gtk_main_quit()` path, and does not compile the new owner. The full GTK4
inventory remains at zero compiler errors while improving from 123 to 117
warnings, from 29 to 26 unique unresolved symbols, and from 29 to 26 repeated
diagnostics. `gtk_get_option_group`, `gtk_main`, and `gtk_main_quit` leave the
GTK4 link boundary.

Stage 8 Windows icon-theme bootstrap pass 82 moves default-theme acquisition,
search-path extension, and explicit theme selection behind cross-version
helpers. GTK4 resolves its theme from the default display, adds indexed icon
roots, and selects Adwaita without an explicit rescan. It recognizes both the
candidate `Runtime/GTK4/share/icons` layout and the older flattened
`share/icons` layout. GTK3 preserves its default theme, append/rescan behavior,
and Windows safeguard that rejects `hicolor/index.theme` roots known to crash
that runtime. The strict probe verifies GTK4 path insertion and theme-name
selection on an isolated theme object. The full GTK4 inventory remains at zero
compiler errors while improving from 117 to 112 warnings and from 26 to 23
unique and repeated unresolved diagnostics. `gtk_icon_theme_append_search_path`,
`gtk_icon_theme_set_custom_theme`, and `gtk_icon_theme_rescan_if_needed` leave
the GTK4 link boundary; one separate main-window `gtk_icon_theme_get_default`
lookup remains.

Stage 8 main-window icon-theme lookup pass 83 routes the edit-box emoji access
icon availability check through the shared cross-version default-theme helper.
GTK4 now queries the icon theme owned by the default display before falling
back to the existing packaged icon resolver; GTK3 receives the same default
theme as before. Candidate icon order, packaged fallback, tooltip, sensitivity,
and emoji-picker activation are unchanged. The full GTK4 inventory remains at
zero compiler errors while improving from 112 to 110 warnings and from 23 to
22 unique and repeated unresolved diagnostics. The final active
`gtk_icon_theme_get_default` call leaves the GTK4 link boundary.

Stage 8 window-operation pass 84 contains minimize, urgency, WM-class, and
post-fullscreen sizing behind shared cross-version helpers. GTK4 minimizes a
realized window through its display-owned `GdkToplevel` and restores the
configured window size through `gtk_window_set_default_size()`. GTK4 has no
urgency-hint or per-window WM-class API, so those compatibility branches
deliberately preserve compositor focus policy and rely on Fabulor's process
identity established before GTK initialization. GTK3 retains all four original
calls. The strict probe compiles and links every new helper. The full GTK4
inventory remains at zero compiler errors while improving from 110 to 106
warnings, from 22 to 18 unique unresolved symbols, and from 25 to 21
unresolved-symbol diagnostics. `gtk_window_iconify`,
`gtk_window_set_urgency_hint`, `gtk_window_set_wmclass`, and
`gtk_window_resize` leave the GTK4 link boundary.

Stage 8 main-menu font pass 85 contains menu font application and theme-driven
relayout across the toolkit boundary. GTK4 styles and queues the model-owned
`GtkPopoverMenuBar` root, allowing its inherited CSS font and retained
`GMenuModel` presentation to govern generated popovers without inspecting
private children. GTK3 retains recursive menu-shell, item, and submenu styling
and sizing. The full GTK4 inventory remains at zero compiler errors while
improving from 106 to 96 warnings, from 18 to 14 unique unresolved symbols,
and from 21 to 15 unresolved-symbol diagnostics. `GTK_IS_MENU_SHELL`,
`GTK_IS_MENU_ITEM`, `GTK_MENU_ITEM`, and `gtk_menu_item_get_submenu` leave the
GTK4 link boundary; the remaining container symbols belong to the separate
legacy About-dialog layout.

Stage 8 emoji fallback font pass 86 replaces the removed GTK4 style-context
property lookup with an owned cross-version font-description helper. GTK4
copies the widget's effective default description from its `PangoContext`;
GTK3 retains the existing style-context `"font"` query. The caller owns and
frees either result, then preserves the established emoji-family detection,
fallback ordering, and scoped CSS application. Both strict probe systems
compile and link the helper signature. The full GTK4 inventory remains at zero
compiler errors while improving from 96 to 95 warnings, from 14 to 13 unique
unresolved symbols, and from 15 to 14 unresolved-symbol diagnostics.
`gtk_style_context_get` leaves the GTK4 link boundary, removing the final
unresolved symbol from `maingui.c`.

Stage 8 native save-dialog overwrite pass 87 moves the retired GTK3
`gtk_file_chooser_set_do_overwrite_confirmation()` call behind the existing
file-chooser compatibility boundary. GTK3 continues to apply the caller's
explicit confirmation choice. GTK4 leaves confirmation to the native save
dialog because its chooser API no longer exposes an application override;
`FRF_NOASKOVERWRITE` remains accepted but cannot suppress platform-owned GTK4
confirmation. Both strict probe systems compile and link the helper signature.
The full GTK4 inventory remains at zero compiler errors while improving from
95 to 94 warnings, from 13 to 12 unique unresolved symbols, and from 14 to 13
unresolved-symbol diagnostics. The removed overwrite setter leaves the active
GTK4 link boundary.

Stage 8 widget-destruction pass 88 routes Preferences cancellation and About
dialog closure through the typed top-level window helper, while removal of the
About dialog's generated action buttons uses the typed box-child helper. GTK4
therefore destroys owned windows with `gtk_window_destroy()` and detaches owned
children from their parent; GTK3 preserves its existing widget-destruction
behavior. The full GTK4 inventory remains at zero compiler errors while
improving from 94 to 93 warnings, from 12 to 11 unique unresolved symbols, and
from 13 to 11 unresolved-symbol diagnostics. `gtk_widget_destroy` leaves the
active GTK4 link boundary; remaining raw calls are confined to GTK3-only menu
and adapter code.

Stage 8 native About-dialog pass 89 gives GTK4 its native `GtkWindow`-owned
About presentation, website link, GPL 2.0-only license page, built-in Close and
Escape handling, and `gtk_window_present()` path. GTK3 retains the existing
custom Website, License, and Close action-area layout. A typed logo helper
projects the retained `GdkPixbuf` into a temporary GTK4 `GdkTexture` paintable
with balanced ownership while preserving the GTK3 pixbuf call. The full GTK4
inventory remains at zero compiler errors while improving from 93 to 86
warnings, from 11 to 5 unique unresolved symbols, and from 11 to 5
unresolved-symbol diagnostics. `GTK_BUTTON_BOX`,
`gtk_button_box_set_child_secondary`, `GTK_CONTAINER`,
`gtk_container_get_children`, `gtk_dialog_get_action_area`, and
`gtk_widget_show_all` leave the active GTK4 link boundary.

Stage 8 menu-toggle link-closure pass 90 confines the remaining
`GtkCheckMenuItem` state fallback, radio callbacks, and widget-menu
Autojoin/Auto-Connect builders to GTK3. GTK4 continues through the retained
boolean and string-target `GSimpleAction` state plus the tab-context action
model. The clean complete GTK4 profile improves from 86 to 81 warnings and
from five unresolved symbols and diagnostics to zero, producing the first
linked full-profile `fabulor.exe`. Its PE imports include `gtk-4-1.dll` and no
GTK3 DLL. `GTK_CHECK_MENU_ITEM`, `GTK_IS_CHECK_MENU_ITEM`,
`gtk_check_menu_item_get_active`, `gtk_check_menu_item_set_active`, and
`menu_toggle_item` leave the GTK4 link boundary.

Stage 8 candidate-startup pass 91 resolves the nested-runtime bootstrap
boundary exposed by that first executable. MSVC cannot delay-load GLib because
normal `g_ascii_*` and UTF-8 macros import GLib data tables, so the GTK4 profile
now emits a Win32-only `fabulor.exe` launcher and an exported
`fabulor-gtk4-frontend.dll`. The launcher contains no GTK/GLib import, registers
the trusted executable-relative runtime first, rejects a reparse-point frontend
module, and loads the frontend through constrained search flags. A system-only
`PATH` smoke run loaded GTK4, GLib, GObject, and GIO exclusively from the staged
runtime and closed normally.

Stage 8 candidate-MSI pass 92 first gave the launcher/frontend boundary an
isolated package for cutover testing. Stage 9 pass 2 retires that temporary
product graph and validator: the established Fabulor product identity is now
the only WiX graph, and production validation enforces its GTK4-only content,
runtime manifest, PE bootstrap boundary, shortcuts, protocols, and selectable
plugin runtimes.

Stage 8 native-extension compatibility pass 93 adds an isolated build profile
for the bundled checksum, Exec, FiSHLiM, Lua, Python, SysInfo, updater, and
WinRT notification modules. FiSHLiM's key manager now has a GTK4-native list,
dropdown, and modeless-dialog implementation while its retained GTK3 branch
still compiles independently. Enchant 2.8.19 and WinSpell are rebuilt against
the final GTK4-era GLib, and a machine-readable contract verifies nine
candidate modules, one data file, required imports, and fourteen owned import
edges while rejecting GTK3, unresolved, or unowned dependencies. Candidate
WiX composition now includes the compatible six autoload plugins, WinRT
notifications, WinSparkle, Enchant core/provider/data, and exact extracted-byte
validation. Windows native plugin discovery is executable-relative unless the
existing development-runtime gate explicitly permits an override.

Stage 8 plugin-host parity pass 94 adds the supported C#, Python, and Tcl
plugin hosts to the side-by-side candidate without broad runtime harvesting.
`plugin-host-payload-contract.json` and `stage_plugin_hosts.py` select and hash
the managed host, private .NET 8.0.29 runtime, Python 3.14 runtime/API, Tcl 8.6
runtime, and Python native host. Candidate MSI validation now requires all
5,821 host files and checks their extracted bytes alongside the native import
contract. Manifest-plugin startup reporting records successful hosts once and
uses stable `C#`, `Python`, and `Tcl` display labels. A packaged three-language
smoke run loaded every runtime from the candidate root and exited normally.

Stage 8 Win32 display-filter pass 95 replaces GTK4's missing per-window native
message hook with one default-display-owned `GdkWin32MessageFilter`. Startup is
idempotent, shutdown removes the exact callback and balances the retained
display reference, and GTK3 keeps its established per-window filter. Shared
dispatch preserves time-change handling, same-process wheel forwarding, and
single-instance/taskbar commands. `WM_COPYDATA` now accepts only the expected
zero identifier, a NUL-terminated payload between 2 bytes and 64 KiB, and a
live session; wheel messages are consumed only after successful forwarding.
GTK4 setting and theme notifications remain exclusively owned by the existing
display-level appearance monitor. The exact 7,270-file candidate minimized and
restored through the new filter, remained responsive, and exited normally.

Stage 8 top-level visibility closure pass 96 makes visible state a first-class
part of `FabulorWindowState`, including change detection and watched
`notify::visible` transitions. Shared hide and present operations now own GTK3
deiconification plus GTK3/GTK4 visibility and presentation policy. Frontend
commands, tray hide/restore, plugin window-status reporting, and Win32 taskbar
dispatch consume that single owner rather than mixing widget visibility with
surface state. The source audit confirms all active top-level close,
destruction, finalization, geometry, state, and native-message callbacks are
already versioned or shared. An exact packaged candidate passed `/GUI HIDE`
and `/GUI SHOW`, remained responsive, and exited normally, closing the
top-level positioning, visibility, and lifecycle inventory.

Stage 9 GTK4 compatibility specialization pass 6 removes the dual-toolkit
implementation from `gtk-compat.h`. Its public helper names remain stable for
incremental caller cleanup, but every helper now has exactly one GTK4
implementation. The header no longer contains `GTK_MAJOR_VERSION` switches,
GTK3 event structures, container/bin ownership, synchronous dialog and
selection APIs, legacy icon/button-box types, or widget-wide destruction and
recursive-show calls. Repository lint protects that boundary. Direct
toolkit-version branches outside the header remain in the source inventory for
subsequent Stage 9 removal passes.

Stage 9 operational-list source specialization pass 7 removes the inactive
GTK3 implementations from 22 converted list, model, and view sources. The
operational-list boundary now contains no toolkit-version switches, classic
tree views, list/tree stores, cell renderers, or tree selections. GTK4
`GListModel`, selection model, list view, column view, tree-list, expander, and
factory ownership remains intact. Repository lint protects the complete file
set, and the frontend-wide version-branch inventory falls from 564 to 290.

Stage 9 theme source specialization pass 8 removes the inactive GTK3
implementations from nine theme controller and Preferences integration files.
Theme discovery and application now use only display-scoped GTK4 CSS
providers, semantic palette decisions, weak top-level ownership, the GTK4
appearance monitor, and staged GTK4 theme selections. Screen-scoped provider
registration, GTK3 style-context palette sampling, widget destruction, and
toolkit-version switches are retired. Repository lint protects this boundary,
and the frontend-wide version-branch inventory falls from 290 to 253.

Stage 9 window/file helper specialization pass 9 removes the inactive GTK3
implementations from `window-state.c`, `window-geometry.c`, and
`file-chooser-path.c`. Window state and geometry now flow exclusively through
GTK4 surfaces and toplevel state/layout observation; native Windows handles use
`GdkWin32Surface`; chooser values use owned `GFile` and `GListModel` paths.
Legacy window/configure events, position queries, local-only/overwrite settings,
and GTK3 file-list ownership are retired. The frontend-wide version-branch
inventory falls from 253 to 233.

Stage 9 spell-input source specialization pass 10 removes the inactive GTK3
implementations from `sexy-spell-entry.c`, `spell-entry-widget.c`, and
`emoji-picker.c`. Spell suggestions and dictionary/language commands now flow
exclusively through the GTK4 action/menu model, text uses `GtkEditable`, caret
styling uses CSS strings, pointer position uses the reviewed delegate boundary,
and the emoji popover has explicit parent/autohide ownership. GTK3 widget menus,
entry-layout inspection, popup population, container attachment, recursive
reveal, and widget destruction are retired. The frontend-wide version-branch
inventory falls from 233 to 218.

Stage 9 transcript-helper source specialization pass 11 removes the inactive
GTK3 implementations from transcript selection, render-target, widget-class,
accessibility, geometry, and supporting headers. Selection now uses GTK4
clipboards/content providers, rendering uses snapshots and explicit Cairo
contexts, widget sizing uses measure/snapshot hooks, geometry uses current
widget dimensions, and accessible content uses `GtkAccessibleText`. GTK3
selection atoms/data, widget clipboards, `GdkWindow` contexts, draw and
preferred-size hooks, allocation queries, and ATK naming are retired. The
frontend-wide version-branch inventory falls from 218 to 197.

Stage 9 transcript-renderer source specialization pass 12 removes the inactive
GTK3 implementation from `xtext.c`. The widget now uses only its GTK4
accessible-text interface, snapshot path, event-controller pointer state,
widget cursor names, allocated geometry, full-redraw fallback, and scheduled
accessibility updates. Child `GdkWindow` creation, native scroll-copy capture,
window pointer/device polling, style-updated callbacks, widget grabs, direct
allocation/window mutation, and GTK3 paint fallback are retired. The
frontend-wide version-branch inventory falls from 197 to 168, completing
source specialization for the transcript subsystem.

Stage 9 tray source specialization pass 13 retires the GTK3 AppIndicator and
`GtkStatusIcon` implementations from `plugin-tray.c`, the version guard from
the GTK4 presenter, and the `appindicator3` dependency probes from the frontend
Meson fragment. The retained tray boundary consists of the GTK4 action/menu
model, backend policy, popover presenter, window-state integration, and native
Windows menu projection. Non-Windows GTK4 builds deliberately expose no
compiled tray backend until a GTK4-native StatusNotifier implementation
exists. The frontend-wide version-branch inventory falls from 168 to 159.

Stage 9 application-lifecycle source specialization pass 14 removes the
inactive GTK3 startup and shutdown paths from `fe-gtk.c`. Initialization uses
GTK4's argument-free `gtk_init`; command-line parsing no longer registers the
GTK3 option group; Windows icon discovery validates the GTK4 runtime and
Adwaita payload; and frontend run/quit ownership uses
`FabulorApplicationMainLoop`. `gtk_main`, `gtk_main_quit`, and the dual
toolkit initialization/icon behavior are retired. The frontend-wide
version-branch inventory falls from 159 to 150.

Stage 9 Server List source specialization pass 15 removes the inactive GTK3
lifecycle implementation from `servlistgui.c`. Certificate-native-dialog
parents use weak references, network editor and Server List windows use typed
`close-request` callbacks, and editor/window cleanup uses weak finalization.
GTK3 parent-destroy signal IDs, destroy handlers, `delete-event`, and
`GdkEventAny` callbacks are retired. The frontend-wide version-branch inventory
falls from 150 to 135.

Stage 9 Channel/Ban List source specialization pass 16 removes the inactive
GTK3 context-menu implementations from `chanlist.c` and `banlist.c`. Channel
List commands continue through the retained action/model presenter; Ban List
copy commands use an explicitly parented, autohiding popover with close
cleanup. GTK3 widget menus, menu-shell/item ownership, box packing, recursive
reveal, pointer popup, and destroy callbacks are retired. The frontend-wide
version-branch inventory falls from 135 to 127.

Stage 9 Preferences/Join source specialization pass 17 removes the inactive
GTK3 lifecycle and viewport implementations from `setup.c` and `joind.c`.
Preferences, its font chooser, and the Join dialog use GTK4 weak finalization;
Preferences page scrollers use explicit child ownership. GTK3 destroy-signal
callbacks, `GtkBin` child inspection, viewport shadow mutation, and all
toolkit-version branches in these sources are retired. The frontend-wide
version-branch inventory falls from 127 to 119.

Stage 9 Channel View/helper source specialization pass 18 removes the inactive
GTK3 lifecycle, icon-loading, tray-detection, modifier-mask, and compile-guard
branches from `chanview.c`, `pixmaps.c`, `gtkutil.c`, `fkeys.h`, and the GTK4
context-menu presenter. Three unused generic GTK3 tree-view helpers and their
public declarations are retired. Residual GTK4 CSS, scroller, and `GdkRGBA`
initializers use their exact current signatures and scalar types. The
frontend-wide version-branch inventory falls from 119 to 113, now entirely
contained in `maingui.c`, `menu.c`, `maingui.h`, and `menu.h`.

Stage 9 main-window source specialization pass 19 removes every inactive GTK3
implementation and toolkit switch from `maingui.c` and `maingui.h`. Main and
tab windows retain GTK4 child ownership, action-model context menus, weak
finalization, gestures and typed drag-and-drop, native display filters, entry
and emoji popovers, transcript scrollers, and close-request handling. GTK3
widget menus, accelerators, GDK window rendering/filtering, event callbacks,
container inspection, and legacy drag destinations are retired. The
frontend-wide version-branch inventory falls from 113 to 56, entirely in
`menu.c` and `menu.h`.

Stage 9 menu source specialization pass 20 removes every inactive GTK3
implementation and toolkit switch from `menu.c` and `menu.h`. Main, context,
nick, channel, middle-click, tab, plugin, and Usermenu presentation retain
`GAction`, `GMenuModel`, and owned GTK4 popover boundaries. GTK3 widget-menu
construction, check/radio items, accelerators, pointer popup, container
inspection, and destruction are retired. The frontend-wide version-branch
inventory falls from 56 to zero. The clean MSVC frontend inventory is also zero
warnings and zero errors.

Stage 9 production artifact validation pass 21 adds a fail-closed Burn release
pair contract. CI extracts `FabulorSetup.exe`, validates the stable bundle and
MSI upgrade identities, project version, per-machine registration, exact
bootstrapper application files, and single embedded MSI chain, then compares
the extracted MSI byte-for-byte with the separately uploaded `Fabulor.msi`.
The existing decompiled-MSI and runtime-manifest checks remain authoritative
for installed paths, required features, GTK3 absence, and GTK4 runtime content.

Stage 9 clean-install layout acceptance pass 22 hardens two runtime boundaries.
`window-geometry.c` owns right-pane saved-size normalization against minimum
and available widths. `xtext.c` owns immediate Cairo context acquisition and
queues a GTK4 frame whenever an input callback runs outside an active snapshot.
Transcript separator interaction can no longer submit a null context to Cairo,
and stale narrow pane state can no longer collapse the visible user list.
`servlistgui.c` now reads and writes every Server List check control through
the GTK4 `GtkCheckButton` boundary; retired `GtkToggleButton` casts can no
longer discard SSL, proxy, global-user, keyring, password-visibility, startup,
or favorites state. Installed-client follow-up found that the converted
trailing-box helper expanded the nickname button itself, displacing the input
field. The helper now right-aligns its containing box while leaving controls at
natural width. The visible user-list container owns the configured minimum, and
its position callback ignores hidden or unallocated state. Emoji picker pages
now expand inside a viewport calculated from the main-window allocation, with
compact and maximum bounds, instead of forcing every scroller to `500 x 330`.
The installed release boundary also requires a versioned WiX major upgrade
when the component graph changes. Fabulor `1.0.4` carries a regenerated
OpenSSL support stage matching the frontend link root; reusing the `1.0.3`
product identity is retired because Windows Installer can otherwise replace
the frontend without registering newly introduced runtime components.

Stage 9 installed-client visual acceptance follow-up pass 23 maps the first
`1.0.4` screenshots back to explicit owners. The channel tree now returns a
child model only for root rows, so leaf channels retain hierarchy without
false expanders. Inline mode buttons use a compact one-line topic surface, and
server sessions remove the channel-only nickname control from the input row.
The transcript exposes a canonical scroll-to-bottom operation which owns both
the adjustment and `scrollbar_down` state. Preferences describe the retained
`.hct`/palette surface as `Fabulor Theme`, with no GTK3 theme wording.
Middle-context composition merges every matching add-on branch into its
canonical submenu, preventing duplicate `Window` headings while preserving
Ban List, Character Chart, Direct Chat, transfers, friends, ignore, plug-ins,
raw log, URL grabber, transcript, and search actions. The production installer
now treats Enchant core, WinSpell provider, and ordering data as one required
three-part payload; omitting `libenchant-2-2.dll` can no longer pass the
production profile contract.

Stage 9 transcript interaction acceptance pass 24 replaces the inherited
screen-coordinate selection boundary. GTK4 drag input now resolves one stable
text-entry and byte-offset anchor, while Pango maps pointer positions to the
same shaped insertion boundaries used for display. Hit-testing is constrained
to the current wrapped row and no longer crosses into an adjacent visual line.
The highlighted range, automatic clipboard publication, explicit
`Copy Selection`, URL hover, and URL activation therefore share one exact
pointer-to-text mapping.

Stage 9 startup server-session acceptance pass 25 initializes each transcript
buffer before its channel-tree row can become GTK4's automatic first
selection. Connection lookup and progress output therefore has a valid visible
buffer from the start. Auto-connect gives focus only to the first configured
network and creates later server sessions in the background, while every
pre-connection server row immediately uses its configured network name.
Direct ChatLounge and DALnet connections passed installed-client acceptance.
An already-connected ZNC can complete too quickly for intermediate connection
progress to remain visible, which is expected bouncer behavior.

Stage 9 user-list resize-policy acceptance pass 26 restores the persisted
`gui_ulist_resizable` contract removed during earlier frontend work and maps it
to GTK4's end-child resize policy. Fixed-width behavior remains the default.
Initial pane restoration now waits for a mapped, visible allocation and three
stable frames before position notifications may update the saved right-pane
width. `gui_pane_right_size` continues to own the complete pane, while
`gui_ulist_nick_width` owns only the nickname column within it.

Stage 9 Network List interaction acceptance pass 27 separates row selection
from network-name editing. Normal `GtkEditableLabel` children do not target
pointer input, allowing a single click to select the containing list row.
Explicit list activation temporarily enables the editor, while newly added
networks retain their immediate rename workflow. Ending an edit restores the
non-targetable display state.

Stage 9 native Windows tray acceptance pass 28 replaces the deliberately empty
post-GTK3 backend with direct Windows Shell ownership. `Shell_NotifyIconW`
owns the GTK4 tray icon, tooltip and state-icon updates, Explorer restart
registration, activation, and native popup dispatch without `GtkStatusIcon` or
AppIndicator. Native tray actions are queued outside GDK's Win32 message
filter. Hiding uses the existing HWND while keeping GTK's render surface mapped;
Fabulor's window-state model tracks that native hidden state explicitly.
Repeated installed testing confirms left-click and menu restoration plus the
Preferences action.

Stage 9 URL single-activation acceptance pass 29 coordinates the primary-click
and selection-drag controllers sharing XText's release path. GTK4 can finish a
zero-distance `GtkGestureDrag` and deliver the real click release for the same
pointer sequence; the selection path's synthetic release and the click release
previously emitted `word_click` twice. A short-lived, owned suppression token
consumes only the duplicate release from that sequence. Installed testing
confirms one browser tab for left-click and context-menu activation while
selection and release-to-copy remain intact.

Stage 9 nick context-menu sizing acceptance pass 30 keeps GTK4's popover-menu
pages horizontally homogeneous so the root menu and nested identity page retain
complete headings while navigating. The nick popup alone limits generated menu
labels to 32 characters with end ellipsis, preventing long WHOIS host, server,
real-name, country, or away fields from dictating an excessive root width.
Identity values retained by menu actions remain complete for clipboard use.
Plain-text identity and away labels no longer pass through markup escaping, so
apostrophes and other ordinary characters display as entered.

Stage 9 obsolete-option retirement pass 31 removes the built-in Identd service.
The internal plugin, `/IDENTD` hook, connection-port publication, listener
lifetime, preference schema fields, Preferences page, apply-time reload path,
translation source registration, MSVC/Meson source entries, and dedicated
change reason are retired together. Existing saved `identd_server` and
`identd_port` keys are ignored and disappear when the canonical configuration
is next written.

Stage 9 obsolete-configuration retirement pass 32 removes
`gui_ulist_style`. The key survived only in the configuration schema, default
initialization, and preference structure; no frontend or core behavior read it.
Existing saved values are ignored and disappear when the canonical
configuration is next written.

Stage 9 Preferences navigation sizing pass 33 gives the category frame a
stable 220-logical-pixel minimum. Ellipsized category labels can no longer
collapse the navigation pane when lazy page creation changes the notebook's
minimum allocation, while unusually long translated labels can still
ellipsize within the retained column.

Stage 9 obsolete Wingate proxy retirement pass 34 removes the Wingate menu
entry and its separate IRC and DCC traversal implementations. A shared proxy
policy preserves the stored numeric values of SOCKS4, SOCKS5, HTTP, and Auto,
while retired value `1` and invalid values normalize to disabled. The
Preferences display mapping, authentication sensitivity, connection dispatch,
DCC proxy eligibility, configuration loading, canonical saving, and `/SET`
updates now consume that policy instead of relying on contiguous menu rows.

Stage 9 SOCKS5 protocol hardening pass 35 introduces a shared bounded protocol
owner for IRC and DCC traversal. It validates method selection, RFC 1929
credentials, destination encoding, reply headers and address lengths; handles
partial socket I/O; rejects authentication downgrade; and terminates queued
DCC traversal on closed or malformed proxy connections. Installed direct IRC
and ZNC testing passes with no authentication and username/password
authentication. SOCKS4 remains unchanged and proposed for separate review.

Stage 9 right-pane allocation acceptance pass 36 preserves the saved user-list
pane width while Preferences temporarily reparents the channel tree and user
list. Transient divider notifications cannot overwrite the saved width, final
restoration follows the GTK4 window-surface layout, and an invalid oversized
restored pane falls back to the configured nickname/minimum width. Clean
install testing confirms stable geometry across every channel with no observed
switching lag.

Stage 9 inert-configuration retirement pass 37 removes `text_transparent`.
The setting survived only in the persisted schema and preference structure;
the GTK4 renderer, background-image support, theme application, and all other
frontend behavior had no reader for it. Existing saved values are ignored and
disappear on the next canonical configuration write.

Stage 9 server-time preference retirement pass 38 removes
`irc_cap_server_time` from the persisted schema, preference storage, default
initialization, and Preferences. The toggle never gated negotiation:
`server-time` and both ZNC server-time variants remain unconditionally
requested when advertised, and timestamp parsing is unchanged.

Stage 9 channel-switch latency acceptance pass 39 uses an opt-in production
profiler to separate transcript, accessibility, and user-list work. Installed
evidence identified deferred user-list model attachment as the persistent
visual delay. Transcript and user-list replacement now complete in one
synchronous switch transaction, redundant model assignments are skipped, and
installed direct IRC/ZNC switching is accepted without observed regression.

Stage 9 active-source retirement pass 40 removes the final FiSHLiM GTK3 dialog
branch, stale Sysinfo GTK3 labels, obsolete GTK3-facing wording/test residue,
and the inherited application Makefile/Meson graph. The strict `tools/gtk4`
probe and every negative GTK3 validator remain. The source audit reports zero
active GTK3 references; strict probes, all 86 tooling/theme contracts, the
full native solution, runtime/import validators, MSI, and bootstrapper pass.

## Functional Clusters

| Cluster | Main files | GTK4 concern | Status |
|---|---|---|---|
| Main windows and tabs | `maingui.c`, `chanview*.c` | child ownership, gestures, DnD, focus | converted and specialized to GTK4; main/tab windows and Channel View contain no toolkit-version branches |
| Transcript | `xtext.c`, `xtext.h`, `xtext-render-target.c` | snapshot rendering and event model | converted and specialized to GTK4; renderer, selection, accessibility, geometry, widget-class, and render-target GTK3 branches are retired |
| Edit box and spell check | `maingui.c`, `emoji-picker.c`, `sexy-spell-entry.c`, `spell-entry-*.c` | production interaction and latency validation | converted and specialized to GTK4; input, spell-entry, emoji, focus, and drag-and-drop paths contain no toolkit-version branches |
| Menus and commands | `menu.c`, `maingui.c`, `plugin-tray.c` | actions and menu models | converted and specialized to GTK4; main/context/plugin/Usermenu action models and popover presenters contain no toolkit-version branches |
| Operational lists | `servlistgui.c`, `chanlist.c`, `userlistgui.c`, `dccgui.c`, `banlist.c`, `notifygui.c`, `ignoregui.c`, `plugingui.c`, `urlgrab.c` | list models, factories, editing | converted and specialized to GTK4; classic GTK3 tree/list and Channel/Ban widget-menu implementations are retired |
| Preferences/editors | `setup.c`, `fkeys.c`, `textgui.c`, `editlist.c` | generic edit list, Print Events, key bindings, sound events, and preference navigation converted | converted; Preferences page ownership, window, and font-chooser lifecycle are specialized to GTK4 |
| Themes | `theme/*.c`, `common/gtk4-theme-*.c`, `common/theme-archive-reader.c` | GTK4 CSS compatibility, discovery, preferences, and bounded `.hct` reading | converted and specialized to GTK4; the GTK3 service, adapter, screen/style branches, and version switches are retired |
| Platform integration | `fe-gtk.c`, `plugin-tray.c`, notifications | displays, surfaces, icons, native tray | converted and specialized to GTK4; native Windows Shell tray accepted and no toolkit-version branches remain |

## Theme Inventory

- `theme/theme-manager.c`, `theme-policy.c`, `theme-runtime.c`,
  `theme-application.c`, and `theme-css.c` should remain toolkit-neutral where
  practical.
- The GTK3 theme adapter, discovery/import service, `%APPDATA%\Fabulor\gtk3-themes`
  workflow, dedicated tests, and saved GTK3 theme keys are retired. GTK3 CSS is
  never treated as valid GTK4 CSS.
- Retained `.hct` imports use `common/theme-archive-reader.c`; GTK4 desktop CSS
  discovery and application remain independent of palette/event archives.
- Stage 8 pass 97 makes `theme-manager.c` the application-lifetime GTK4 theme
  owner. The Preferences surface borrows that controller, successful choices
  update only staged settings, cancel or save failure restores the original
  selection, and the display appearance monitor calls the same controller
  without depending on a Preferences window. Frontend shutdown removes the
  monitor before releasing display-scoped providers.

## Completion Invariants

Future frontend work must preserve these conditions:

1. Keep active frontend and supported plugin source free of toolkit-version
   gates and GTK3-only API families.
2. Keep `win32\fabulor.sln` and WiX as the only application build/package
   graph.
3. Retain `tools\gtk4` Meson only as the isolated strict GTK4 probe.
4. Keep GTK3 runtime, import, source, and theme rejection tests enabled.
5. Update this inventory and `validation-log.md` if a supported GTK4 API is
   deprecated or an ownership boundary changes.
