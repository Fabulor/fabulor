# GTK4 List Model Architecture

Status: Stage 5 pass 10 conversion contract

Date: 2026-07-16

## Purpose

GTK4 removed `GtkTreeView` and cell renderers. Fabulor will migrate each list
as a complete model, selection, factory, and view surface rather than emulate
the GTK3 tree APIs or maintain two live representations of the same data.

## Chosen Stacks

Flat operational lists use:

1. an app-owned `GListStore` containing typed row objects;
2. `GtkSortListModel` when the workflow is ordered;
3. `GtkSingleSelection` or `GtkMultiSelection` according to the workflow; and
4. `GtkListView` for simple rows or `GtkColumnView` for tabular workflows.

Channel navigation uses:

1. one app-owned `GListStore` for each level of channel-family rows;
2. `GtkTreeListModel` with row passthrough disabled;
3. `GtkSingleSelection`; and
4. `GtkListView` with `GtkTreeExpander` in the row factory.

Sorting channel families remains local to each level. Flattening the complete
tree into one sorted model would mix parents and children and change visible
navigation order.

## Ownership And Identity

- Core session, user, channel, and transfer structures remain authoritative.
- GTK row objects hold the minimum presentation state and stable references
  required by their owning surface.
- Stores own row objects; model adapters and selection models own references
  to their input models; the surface owner releases the stack in reverse order.
- Row objects, not indexes or GTK3 iterators, provide identity across sorting,
  insertion, filtering, and selection changes.
- Core objects must not retain `GtkTreeIter`, `GtkTreePath`, or
  `GtkTreeRowReference` after their owning surface is converted.
- Removing a core object first removes its row by identity, then releases any
  surface-owned callbacks or references.

## Update Rules

- A converted surface writes only to its GTK4 store. Do not mirror updates into
  a retained GTK3 model.
- Batch high-frequency user-list changes where practical and update row
  properties in place when identity is unchanged.
- Preserve selection by row identity; never infer it from a stale sorted
  position.
- Factories bind and unbind every signal and object reference they acquire.
- Context menus, activation, keyboard navigation, editing, drag/drop, and
  accessibility resolve the selected row object through the selection model.

## Migration Sequence

1. [x] Convert a contained flat operational list and validate the shared stack
   and factory lifecycle. The Notify List is the reference implementation.
2. [x] Convert the user list, including frequent updates, sorting, selection, and
   internal/external drag/drop integration.
   - [x] Replace per-session stores and external row references with a typed,
     identity-indexed cross-version model owner.
   - [x] Convert the shared view, factories, selection workflows, pointer
     hit-testing, context menus, keyboard forwarding, and drag/drop.
3. [ ] Convert channel navigation, expansion state, badges, keyboard switching,
   and channel-family reordering.
   - [x] Replace the shared tree store and persistent channel iterators with an
     identity-indexed cross-version hierarchy owner.
   - [x] Convert the visible hierarchical tree, factories, expansion,
     identity selection, hit-testing, and scrolling.
   - [x] Convert tree and tab close/context input to a neutral widget, button,
     coordinate, and modifier boundary.
   - [x] Move toggle suppression and scroll-animation lifetime into each channel
     view and resolve keyboard focus movement through stable model identity.
   - [x] Mirror grouped-tab and family reorder operations from authoritative
     channel-model positions without enumerating GTK children.
   - [x] Own grouped-tab family boxes by model-root identity with deterministic
     creation, insertion, removal, and cleanup.
   - [ ] Convert grouped-tab close presentation, animated scrolling
     presentation, keyboard handling, and drag/drop.
4. [ ] Convert remaining tabular editors and operational lists according to their
   editing and multi-selection requirements.
5. [ ] Remove the final GTK3 tree-model, cell-renderer, iterator, and row-reference
   assumptions only after all owning surfaces have moved.

## Notify List Reference Surface

`notify-list.c` is the first complete owner boundary. The frontend supplies
copied snapshots with separate display and owner names, stable notify identity,
optional per-server identity, status, network, last-seen text, and foreground
colour. Remove and Open Dialog actions resolve the selected row directly;
continuation rows no longer search backwards for a non-empty display name.

The GTK4 branch uses `GtkSingleSelection` and four `GtkColumnView` factories.
Rows are reused by notify/server identity, changed properties alone notify bound
labels, and an unchanged identity order does not splice the store. Selection is
restored exactly where possible and otherwise follows the first row for the same
notify owner when an offline row becomes one or more online server rows.

The production GTK3 branch remains available until the frontend target changes
toolkits, but all of its model, renderer, iterator, and selection operations are
contained inside the owner. `notifygui.c` is toolkit-model independent.

## User List Model Owner

`user-list-model.c` owns one model per session. Session restore state holds only
the opaque owner; it no longer stores a toolkit model or a second row-reference
table. Stable `struct User *` identity indexes each row for constant-time
updates and removals. The retained GTK3 branch contains its five remaining
`GtkTreeRowReference` reference lines internally until the shared view moves.

The GTK4 branch stores typed row objects with icon, prefix markup, nickname and
typing markup, hostname, and optional foreground colour properties. It wraps
the store in the shared sorted multi-selection stack. Its comparator delegates
to Fabulor's existing IRC-aware alphabetic or privilege ordering and safely
normalizes descending results. Presentation-only typing updates do not announce
sort-key changes; identity or privilege/name changes can request re-sorting.

`user-list-view.c` attaches the active session model to one shared view owner.
Its GTK4 branch uses `GtkListView` because the user list is a headerless
composed row rather than a tabular editor. A signal factory binds icon, coloured
prefix markup, escaped nickname/typing markup, optional hostname, and nickname
foreground colour. Every row notification handler is disconnected on unbind;
hit-testing resolves the live `GtkListItem` position rather than retaining a
sorted index. Multi-selection snapshots expose stable core user identity.

The GTK3 branch retains the shipping tree columns and saved width behaviour
inside the same owner. Frontend commands no longer traverse either toolkit's
model: selection toggles, `/USELECT`, selected nick lists, model switching,
scroll values, file-drop targets, and drag highlighting all use neutral view
operations. Multi-click and key controllers preserve double-click commands and
input forwarding, and the menu module owns GTK3 pointer-event translation for
the neutral coordinate-based nick-menu call.

## Channel Navigation Model Owner

`channel-model.c` owns the two-level server/channel hierarchy independently of
the displayed switcher. Stable `chan *` identity indexes typed row records and
drives root/child traversal, updates, removal, reparenting, and cyclic sibling
movement. Channel records no longer retain a toolkit iterator, and shared
`chanview.c` code has no tree-store, iterator, or row-reference operations.

The GTK4 branch owns one `GListStore` per hierarchy level and exposes the shared
`GtkTreeListModel` and `GtkSingleSelection` stack. Mutations preserve selection
by identity even when a selected child changes position or parent. The GTK3
branch contains the retained `GtkTreeStore` and one on-demand row reference per
record so the shipping tree remains functional until its visible view moves.
The tab implementation no longer creates a hidden tree view solely to share
model storage.

## Channel Tree View Owner

`channel-tree-view.c` owns the visible hierarchical presentation. Its GTK4
branch uses `GtkListView`, `GtkTreeExpander`, and a signal list-item factory
that composes optional icons, ellipsized names, row attributes, and focus
underlines. Factory unbind disconnects row notifications, and view teardown
disconnects the longer-lived selection model before releasing callback state.
Identity selection expands the required parent and scrolls the selected row
into view; expansion state and pointer hit-testing are likewise exposed without
toolkit row handles.

The GTK3 branch contains the retained tree view, cell renderers, paths, and
iterators. `chanview-tree.c` now applies only channel workflow policy and has no
direct `GtkTreeView`, renderer, iterator, or path operations. Shipping behavior
retains icons, compact rows, tree lines, serverless indentation, selection,
off-screen-only focus scrolling, double-click expansion, context hit-testing,
channel switching, and internal drag/drop. The grouped tab strip remains the
next channel-navigation pass because its family boxes, close controls, and
scroll animation have a separate lifecycle.

Tree and tab pointer actions now share the cross-version multi-click controller.
The channel callback receives the source widget, button, coordinates, and
modifier state instead of a `GdkEventButton`. Tree hit-testing stays inside its
view owner; tab close-button hit-testing remains local to the tab presentation.
The GTK3 menu adapter obtains the current event only while placing its retained
popup and has a widget-anchored fallback. Middle-click close and right-click
context behavior remain unchanged while the future tab owner no longer needs a
GTK3 event type in its public contract.

Grouped-tab transient state is now view-owned. Each channel view owns its
forward and backward animation slots, movement flags, and toggle-suppression
guard; cleanup cancels outstanding timeout sources before destroying the tab
widgets. Multiple windows can therefore animate or change focus independently.
Relative and absolute focus movement use the authoritative flattened channel
model rather than enumerating family boxes and toggle-button children. Family
button construction and presentation updates remain in the legacy tab
implementation for later presentation passes.

Grouped-tab reordering now mirrors the hierarchy owner rather than deriving a
new order from widget children. Child tabs use their current model sibling
position plus the leading server tab, while moving a server/root repositions
its complete family box at the current model root position. The shared move
workflow therefore mutates one authoritative order and the retained GTK3 tab
presentation follows it without enumerating tabs or comparing family pointers.

Each grouped-tab view now owns an explicit root-identity-to-family map. A root
creates one family box and separator; children resolve that record from their
model parent and insert at their model sibling position. Removing a child
leaves its root family intact, while removing a root destroys and unregisters
the now-empty family after child reparenting. Orientation changes, view-mode
changes, and teardown release the complete map. Family discovery and pruning
therefore no longer inspect GTK children or store family identity on widgets.

## Executable Contract

`src/fe-gtk/gtk4-list-models.c` implements the GTK4-only ownership stacks.
The isolated GTK4 MSVC and Meson probes verify:

- sorted insertion and identity-based removal;
- selection persistence when sorted positions move;
- hierarchical expansion and child depth;
- selected child stability when an unrelated root is removed; and
- complete model and selection cleanup.
- user-list duplicate rejection, ascending/descending/unsorted ordering,
  external sort-key updates, typed row identity, removal, and cleanup.
- user-list view and multi-click helper signatures under the strict GTK4 build.
- channel hierarchy duplicate rejection, traversal order, rename, cyclic move,
  reparenting, removal, typed tree access, and selection persistence.
- channel tree construction, expansion, identity selection, callback dispatch,
  destruction, listener cleanup, and post-destruction model reuse.

These probes establish architecture and ownership only. Production workflow,
visual, keyboard, accessibility, and load validation remains mandatory for
each converted list.
