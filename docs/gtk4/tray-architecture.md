# Tray Architecture

Status: Stage 7 pass 10 complete for GTK4 popover presenter ownership

## Action Boundary

`tray-action-model.c` owns the toolkit-neutral built-in tray menu contract. A
presenter receives a borrowed `GMenuModel` and `GActionGroup` under the stable
`tray` action namespace. The model exposes these actions:

- toggle window visibility
- set all connected networks away or back
- toggle channel, private-message, and highlight blinking
- open preferences
- request application quit

The owner copies every supplied display label, owns the action group and menu,
and invokes the caller's destroy notification exactly once. Presenters cannot
retain caller-owned labels through the model. During teardown, retained action
groups are disabled and disconnected before the owner is released, so late
presenter activation cannot call freed plugin state.

## State Updates

One `FabulorTrayActionState` snapshot controls the hide/restore label, away and
back sensitivity, and the three stateful blink actions. Updates synchronize
the action states without invoking application callbacks. Unknown away-state
values normalize to the mixed state so neither status action is disabled by
malformed input.

Activating a blink action updates the model before dispatching its typed action
callback. Disabled away or back actions do not dispatch. Menu sections and
action names are deterministic, allowing GTK4, StatusNotifier, and native
Windows presenters to consume the same behavior.

## Dynamic Plugin Composition

`plugin-tray.c` owns one action model and one dynamic-menu owner for its complete
plugin lifetime. It populates snapshots from live window visibility, aggregate
away state, and the three blink preferences. Window show/hide/state changes,
preference application, and `fe_set_away` refresh the model.

Presenters request one immutable menu projection plus separately owned built-in
and plugin action-group references. Each request refreshes live state and
rebuilds the `$TRAY` plugin subtree from the current plugin menu definitions.
`tray-menu-composition.c` inserts that subtree after the first two built-in
sections while preserving linked submenus, item attributes, action names, and
plugin command metadata. The projection owns its menu links independently of
the temporary source model. Plugin actions use the stable `fabulor-context`
namespace and remain separate from the built-in `tray` namespace.

## GTK4 Popover Presenter

`tray-menu-presenter-gtk4.c` owns a candidate `GtkPopoverMenu`, the immutable
menu projection, and both action groups. Projection replacement updates the
menu and the `tray` and `fabulor-context` groups together. The presenter keeps
its own references, so callers can release every projection result immediately
after binding it.

Presenter teardown first closes the popover, removes its menu and action groups,
and unparents it when necessary. Only then are retained references released.
Consequently, a widget retained beyond presenter destruction has neither menu
content nor callable tray/plugin actions. The two namespace strings live in a
toolkit-neutral shared header used by model generation, legacy menu attachment,
and the GTK4 presenter.

Typed actions route through the existing visibility, away/back, preferences,
quit, and preference-update paths. Model updates compare snapshots and avoid
menu notifications when state is unchanged; only a visibility-label change
rebuilds menu sections. Plugin deinitialization releases the action owner and
clears its plugin context.

## Deferred Presentation Work

The shipping GTK3 status icon/AppIndicator and Win32 popup menu remain unchanged
in this pass. The GTK4 presenter remains in the candidate probe until the GTK4
frontend has a production anchor or platform tray backend. Native Windows
shell-icon ownership and GTK4/Unix presenter selection remain separate tasks.
