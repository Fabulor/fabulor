# Tray Architecture

Status: Stage 7 pass 7 complete for tray action and state ownership

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

The shipping `plugin-tray.c` GTK3 status icon/AppIndicator and Win32 popup menu
remain unchanged in this pass. A later binding pass must populate the snapshot
from live preferences and window/network state, route typed actions to the
existing commands, and compose dynamic `$TRAY` plugin entries. Native Windows
shell-icon ownership and GTK4/Unix presenter selection remain separate
platform tasks.
