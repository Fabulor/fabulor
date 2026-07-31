# Activity List Design

Status: optional post-1.0 product design; implementation has not started.

## Purpose

Fabulor already marks inactive channels when they receive new data, messages,
or highlights. The Activity List will make that state easier to review across
all connected networks without forcing the user to inspect every channel in
the channel switcher.

The feature adapts the useful attention-management concept found in other IRC
clients while retaining Fabulor terminology, data ownership, configuration,
and user-interface conventions. It will not copy another client's scripts,
configuration language, or implementation.

## User Experience

The first implementation will provide:

- compact unread counts beside inactive channel and dialog entries;
- `View > Activity List`, opening a focused list of contexts with unread
  activity;
- one row per server tab, channel, or private dialog;
- ordering by activity priority and then most recent activity;
- separate counts for ordinary activity, messages, private messages, and
  highlights;
- keyboard and pointer activation that moves directly to the selected context;
  and
- accessible labels that describe the context, network, highest priority, and
  unread count without relying on colour alone.

The Activity List is an operational view, not a second channel switcher. It
contains only contexts with unread activity and disappears or becomes empty
when all activity has been read.

## Activity Classes

Core event handling assigns each incoming event exactly one highest activity
class:

1. `other`: joins, parts, mode changes, notices, and other low-priority output;
2. `message`: normal channel conversation;
3. `private`: messages in a private dialog; and
4. `highlight`: messages that trigger Fabulor's existing highlight rules.

A highlight in a channel increments only the highlight counter, not both the
message and highlight counters. A private highlight increments only the
highlight counter. This keeps the total unread count equal to the number of
unread events.

Events already marked as producing no activity remain excluded. Playback of
saved logs, internal redraws, topic repainting, and switching a view must not
create activity.

## Read And Clearing Rules

Activity belongs to the destination context, even when the user is viewing a
different network or window.

- An incoming event is considered read without incrementing a counter only
  when its context is selected and its owning top-level Fabulor window has
  foreground keyboard focus.
- A selected context still accumulates unread activity while its window is
  minimized, hidden in the tray, covered by another application, or otherwise
  lacks foreground keyboard focus.
- Activating a context clears all of its counters after its current transcript
  has been presented.
- Activating an Activity List row follows the same path as activating the
  corresponding channel-switcher row.
- Clearing a child context subtracts its counters from its server aggregate.
- Selecting a server tab clears only activity belonging to that server tab,
  not its child channels or dialogs.
- A server's child aggregate is presentation state for its collapsed
  channel-switcher parent. It is not the server tab's own unread count and does
  not create a separate Activity List row.
- Closing a context removes its counters and Activity List row.
- Disconnecting preserves open-context activity until those contexts close;
  activity does not persist across an application restart in the first
  implementation.

The initial implementation will not provide manual unread markers, arbitrary
read positions, or transcript-level read receipts. Those features require a
different persistence and navigation model.

## Model And Ownership

The core session remains authoritative. Each session owns:

- four saturating unsigned counters;
- its highest current activity class;
- the timestamp and monotonic sequence of its latest unread event; and
- its effective activity policy.

The existing tab-state classification is the starting point, but event
classification and counter mutation must live in one core path. GTK code
receives typed change notifications and must not infer activity by parsing
rendered text.

Each server owns an incrementally maintained presentation aggregate of its
child contexts for the collapsed channel switcher. The server session retains
separate counters for events printed directly to its server tab; only those
counters determine the server tab's Activity List row. Incrementing, clearing,
moving, and closing a child context update the presentation aggregate from the
counter delta. The implementation must not scan all sessions to refresh a
server row.

The GTK4 Activity List uses a typed `GListStore`, a sorting model, stable
session identity, and single selection in accordance with
[`list-model-architecture.md`](../gtk4/list-model-architecture.md). Rows update
in place. Rebuilding the entire model for every IRC event is prohibited.

Counters saturate rather than wrap. Display may abbreviate large values, but
accessible text exposes the exact stored value.

## Per-Context Activity Policy

After the basic counters and Activity List are stable, Fabulor may add one
persisted policy per server, channel, or dialog:

- `All activity`
- `Messages and above`
- `Private messages and highlights`
- `Highlights only`
- `Muted`

The default is `All activity`. A policy controls whether an event enters the
Activity List and unread counters; it does not hide transcript output, disable
logging, or alter sound and tray-alert settings.

This setting belongs with Fabulor's existing per-context channel options and
will be exposed through the context menu and a documented command. Because the
current channel-option values are tri-state booleans, the implementation must
add a validated enum field rather than overload an existing flag.

## Automatic Policies

Automatic policy assignment is a later, separately testable stage. It may set
the activity policy when a context is created using bounded rules for:

- an exact network name;
- an exact channel or dialog name; and
- an optional, explicitly selected wildcard match.

Only documented activity properties may be changed. Fabulor will not implement
an unrestricted property setter equivalent to a scriptable configuration
engine. Rules must have length and count limits, reject invalid policy values,
use IRC-aware name comparison where applicable, and be editable in
Preferences.

Existing contexts may be re-evaluated only through an explicit user action.
Opening Preferences must not silently rewrite context policies.

## Noisy-Nick Suppression

A final optional stage may let a user reduce the activity priority generated by
specific bots or noisy nicks while leaving their transcript output intact.
This must be a separate bounded rule set, not part of ignore masks.

The first version should support exact nick matches. Hostmask or wildcard
matching may be considered only after nickname-change behavior, account tags,
case mapping, and rule precedence have documented tests.

Suppression can lower an event's attention class but must never promote it,
alter IRC protocol handling, or prevent a genuine direct highlight from
remaining visible unless the user explicitly selected that behavior.

## Performance Requirements

Fabulor has previously required tuning for busy channels, so this feature has
strict limits:

- classify and update an event in constant time;
- do not scan transcripts, user lists, or all open sessions per event;
- do not rebuild the channel tree or Activity List per event;
- coalesce GTK row notifications during bursts without delaying core state;
- keep sorting work limited to rows whose priority or latest-event order
  changed;
- perform no file I/O on the incoming-message path; and
- remove every queued UI update safely when its session or window closes.

Performance logging must be able to distinguish core counter time, model update
time, and visible GTK presentation time during acceptance testing.

## Staged Delivery

### Stage 1: Core Activity Model

- Add typed activity classes, per-session counters, sequence ordering, and
  server aggregates.
- Route existing tab-state changes through the single classifier.
- Add tests for incrementing, priority, saturation, clearing, closing,
  foreground and background windows, minimized and tray-hidden windows,
  server-session versus child aggregates, and no-activity events.

### Stage 2: Channel-Switcher Counts

- Display compact counts without changing row dimensions during updates.
- Preserve current colour indications.
- Add accessible descriptions and verify tree and tab switcher modes.

### Stage 3: Activity List

- Add `View > Activity List`.
- Implement the typed, sorted GTK4 model and direct navigation.
- Cover keyboard use, screen-reader labels, lifecycle cleanup, and
  multi-window identity.

### Stage 4: Persistent Context Policies

- Add the validated activity-policy enum to per-context persistence.
- Add context-menu controls and command help.
- Verify defaults, upgrades, network case mappings, and corrupted settings.

### Stage 5: Automatic Policy Rules

- Add the constrained rule model and Preferences editor.
- Apply rules only when contexts are created or when explicitly requested.
- Bound rule count, pattern length, matching cost, and saved file size.

### Stage 6: Noisy-Nick Rules

- Start with exact nick matches and explicit priority reduction.
- Test nick changes, network case mappings, highlights, dialogs, reconnects,
  and precedence with context policies.

### Stage 7: Acceptance And Documentation

- Soak-test multiple networks and a high-volume bot channel.
- Compare channel-switch latency and message throughput with the feature off
  and on.
- Complete the user manual, configuration reference, accessibility notes, and
  release notes before enabling the feature by default.

Each stage is a contained change and requires its own automated validation and
real-world acceptance before the next stage begins.

## Out Of Scope

This design does not introduce:

- synchronization of unread state between devices or bouncer clients;
- persistent transcript read positions;
- an unrestricted WeeChat-style buffer property language;
- script execution from matching rules;
- hidden or discarded IRC events;
- replacement of the existing channel switcher; or
- automatic activation before post-1.0 acceptance is complete.
