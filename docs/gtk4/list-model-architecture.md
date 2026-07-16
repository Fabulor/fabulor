# GTK4 List Model Architecture

Status: Stage 5 pass 2 conversion contract

Date: 2026-07-15

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
2. [ ] Convert the user list, including frequent updates, sorting, selection, and
   internal/external drag/drop integration.
3. [ ] Convert channel navigation, expansion state, badges, keyboard switching,
   and channel-family reordering.
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

## Executable Contract

`src/fe-gtk/gtk4-list-models.c` implements the GTK4-only ownership stacks.
The isolated GTK4 MSVC and Meson probes verify:

- sorted insertion and identity-based removal;
- selection persistence when sorted positions move;
- hierarchical expansion and child depth;
- selected child stability when an unrelated root is removed; and
- complete model and selection cleanup.

These probes establish architecture and ownership only. Production workflow,
visual, keyboard, accessibility, and load validation remains mandatory for
each converted list.
