# Tray Architecture

Status: Stage 7 pass 8 complete for tray action ownership and live binding

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
retain caller-owned labels through the model.

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

## Deferred Presentation Work

`plugin-tray.c` now owns one action model for its complete plugin lifetime. It
populates snapshots from live window visibility, aggregate away state, and the
three blink preferences. Window show/hide/state changes, preference application,
and `fe_set_away` refresh the model. Presenters can request borrowed menu and
action-group handles; each request first refreshes live state.

Typed actions route through the existing visibility, away/back, preferences,
quit, and preference-update paths. Model updates compare snapshots and avoid
menu notifications when state is unchanged; only a visibility-label change
rebuilds menu sections. Plugin deinitialization releases the action owner and
clears its plugin context.

The shipping GTK3 status icon/AppIndicator and Win32 popup menu remain unchanged
in this pass. A later presentation pass must consume the bound model and compose
dynamic `$TRAY` plugin entries. Native Windows shell-icon ownership and
GTK4/Unix presenter selection remain separate platform tasks.
