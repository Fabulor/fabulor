# GTK4 Migration Plan

Status: planning baseline

Baseline date: 2026-07-14

## Objective

Move the Fabulor desktop frontend from GTK 3.22+ to GTK 4 while preserving the
current IRC, add-on, accessibility, spell-checking, theming, installer, and
Windows integration behaviour.

The migration is complete only when the production executable links GTK4,
legacy GTK3 runtime files are absent from clean and upgraded installations, and
the validation matrix in [validation-log.md](validation-log.md) passes.

## Current State

- `src/fe-gtk/meson.build` requires `gtk+-3.0 >= 3.22`.
- `win32/zoitechat.props` selects GTK3 headers and import libraries.
- `.github/workflows/windows-build.yml` downloads separate GTK3 build
  dependencies and a GTK4 runtime payload.
- `win32/copy/copy.vcxproj` stages GTK3 DLLs, modules, print backends, emoji
  data, themes, schemas, icons, and shared libraries beside `fabulor.exe`.
- WiX already packages a repository `Runtime/GTK4` tree. The current tree
  contains GTK 4.22.4 and GLib 2.88.0, but the executable does not yet consume
  that GTK4 build surface.
- Theme code is intentionally split into policy/runtime/application layers and
  a GTK3 adapter. This is a useful boundary for introducing a GTK4 adapter.
- The frontend is source-driven rather than GtkBuilder-driven; there are no
  `.ui` files to convert.

## Migration Rules

1. Keep the shipping GTK3 build green until a GTK4 build can exercise the core
   client workflow.
2. Use one maintained frontend source tree. Temporary compatibility helpers are
   acceptable; a permanent forked `fe-gtk3` and `fe-gtk4` implementation is not.
3. Keep production behaviour unchanged unless a PR explicitly documents and
   validates an approved UX change.
4. Migrate ownership and lifetime semantics deliberately. GTK4 child ownership,
   asynchronous dialogs, event controllers, list models, and snapshot drawing
   are architectural changes, not search-and-replace work.
5. Do not switch installer payloads before the GTK4 executable and its runtime
   discovery rules are validated together.
6. Every conversion PR updates [api-inventory.md](api-inventory.md) and records
   its checks in [validation-log.md](validation-log.md).
7. Remove compatibility code once its final caller is on GTK4. Do not accumulate
   a second private widget toolkit.

## Stages

### Stage 0: Documentation And Baseline

Deliverables:

- Record the migration order, API inventory, runtime packaging state, and
  validation matrix.
- Capture baseline x64 build results, installed layout, runtime versions, and
  high-risk frontend files.
- Keep this stage documentation-only.

Exit criteria:

- The four documents under `docs/gtk4` exist and are linked from `To-Do.md`.
- High-risk API families and ownership boundaries have named migration stages.

### Stage 1: Build And Compatibility Foundation

Progress (2026-07-14): the Windows x64 GTK4 archive is pinned by URL, size, and
SHA-256 in `tools/gtk4/dependency-contract.json`. The isolated MSVC and Meson
probes validate GTK/GLib versions, x64 architecture, required files, and GTK3
contamination before compiling, linking, and executing against the same root.
The first compatibility boundary covers single-child assignment and window
destruction, and the same header is compiled against GTK3 and GTK4. Production
targets remain on GTK3; broader frontend conversion has not started.

Deliverables:

- Define one authoritative GTK4 build root for MSVC and Meson.
- Add GTK4 include/library discovery without silently falling back to GTK3.
- Add narrowly scoped compatibility helpers for child assignment, widget
  visibility, window destruction, CSS provider installation, and other repeated
  mechanical transitions.
- Add CI compile probes for converted helpers and modules before enabling the
  full GTK4 target.
- Pin and record the GTK4/GLib build inputs used by CI and local builds.

Exit criteria:

- GTK3 production builds remain clean.
- The GTK4 dependency root is deterministic and version-reportable.
- Converted modules cannot accidentally link a mixed GTK3/GTK4 process.

### Stage 2: Widget Ownership And Layout

Progress (2026-07-14): 39 statically typed single-child assignments now use
the shared GTK3/GTK4 compatibility boundary across 14 source files. This covers
windows, scrolled windows, frames, buttons, the transcript overlay, and the
emoji popover. Direct `gtk_container_*` usage has fallen from 155 lines in 25
files to 117 lines in 23 files. Box packing, dialog content areas, menu/list
children, event boxes, GTK3 viewport access, recursive visibility, and generic
destruction remain explicit for their dedicated conversion passes.

Box-layout progress (2026-07-14): 54 start-ordered additions across the
character chart, list/key editors, ignore dialog, join dialog, event editor,
and theme preferences now use an exact GTK3/GTK4 helper. The GTK4 path maps
expansion to the box axis, centres non-filled children unless they already have
explicit alignment, and adds packing padding to existing directional margins.
Direct `gtk_box_pack_*` usage has fallen from 186 lines in 22 files to 132
lines in 15 files. Mixed start/end layouts are deliberately deferred for
per-surface ordering review.

Mixed-layout progress (2026-07-14): 40 reviewed additions across channel tabs,
ban/DCC/friends/add-on/URL utility windows, raw log, and preferences now use
explicit append order. Trailing rows were converted only where an expanding
list, scroller, or content child already consumes spare space. The channel-tab
family separator is appended after its tab explicitly, preserving the former
start/end visual order. Direct `gtk_box_pack_*` usage is now 92 lines in seven
files, isolated to main-window dynamics, generic dialogs, menus, operational
models, and the spell-entry menu for their dedicated stages.

Visibility/lifecycle progress (2026-07-14): 12 completed utility-window and
dialog trees now use a reviewed reveal boundary. Its GTK3 path retains
recursive `show_all`; its GTK4 path reveals only the completed root because
GTK4 children are visible by default. Twenty-four statically known windows and
dialogs now use typed window destruction. Direct `gtk_widget_show_all` usage
has fallen from 34 lines in 18 files to 22 lines in nine files, and direct
`gtk_widget_destroy` usage from 81 lines in 23 files to 57 lines in 16 files.
Menus, conditionally hidden main-window content, generic signal callbacks, and
arbitrary child widgets remain explicit for their dedicated lifecycle passes.

Lifecycle pass 2 (2026-07-14): the three shared string/integer/boolean prompt
dialogs now use the reviewed reveal boundary, and typed destruction now covers
their six response paths, the ignore confirmation, frontend message/session
windows, six concrete main-window/dialog paths, and five dialog-response signal
connections through an exact callback adapter. Direct `gtk_widget_show_all`
usage is now 19 lines in eight files and direct `gtk_widget_destroy` usage is
37 lines in 13 files. Remaining calls own menus, notebook/channel-view
children, dynamic main-window controls, unparented probes, spell-entry items,
or GTK3-only test fixtures and require type-specific migration.

Generic-dialog layout progress (2026-07-14): the shared string, integer, and
boolean prompts plus the notify-add dialog now attach content through typed
`GtkBox` ownership. A narrow trailing-pair helper preserves GTK3 `pack_end`
order and right alignment while the GTK4 path appends the label/control pair
in visual order and aligns the completed row to the end. Direct
`gtk_box_pack_*` usage is now 88 lines in seven files and direct
`gtk_container_*` usage is 113 lines in 23 files. Menu dialogs and model-heavy
server-list layouts remain with their owning stages.

Main-window layout pass 1 (2026-07-14): 39 reviewed additions now use explicit
append order and one uses typed trailing alignment across quit-dialog content,
channel-mode/topic controls,
transcript scaffolding, information frames, user-list structure, centre panes,
emoji pages, search controls, and reply/input rows. Conditional visibility is
unchanged, including the hidden reply bar. Direct `gtk_box_pack_*` usage has
fallen from 88 lines to 48 lines across the same seven files. The 10 remaining
`maingui.c` matches are dynamic nickname/meter insertion, a trailing tab,
explicit dialog-button reordering, and one commented legacy block.

Dynamic main-window pass 2 (2026-07-14): runtime nickname icons and connection
progress now insert immediately before the permanent trailing nickname button,
and their removal is constrained to the known owning box. User-list buttons
are constructed before meters so explicit append order preserves the prior
bottom-edge layout; meter refresh and dialog-button replacement also use typed
box-owned removal. Refreshed dialog buttons use the reviewed reveal boundary,
and the obsolete commented link-button block is removed. `maingui.c` now has
no direct `gtk_box_pack_*` calls. Repository-wide direct packing has fallen to
38 lines in six files, direct recursive reveal to 18 lines in eight files, and
direct widget destruction to 32 lines in 13 files. The two meter
`GtkEventBox` wrappers remain for the Stage 4 event-controller pass.

Menu-dialog and shared-button ownership pass (2026-07-14): the response-driven
Join Channel dialog now preserves its label/entry end order through the typed
trailing-pair helper, attaches content through its exact `GtkBox`, uses the
reviewed reveal boundary, and destroys its concrete dialog window through the
typed lifecycle helper. `gtkutil_button()` now assigns its label-less child box
and optional parent box through typed helpers while preserving labelled and
label-less callers. Direct packing is now 35 lines in five files, direct
container calls 110 lines in 23 files, direct recursive reveal 17 lines in
eight files, and direct widget destruction 31 lines in 13 files. Menu items,
actions, accelerators, and menu models remain unchanged for Stage 3.

Operational-list shell pass 1 (2026-07-14): the shared tree-view constructor
now requires a `GtkBox` parent and attaches all nine operational-list scrollers
through explicit expanding append semantics. The main user-list constructor
uses the same typed parent contract. Channel-list status, filter, range, and
search-option rows now use explicit append order, while its tree model,
renderers, selection, context menu, and events are unchanged. Direct packing
is now 25 lines in four files and direct container calls 109 lines in 23 files.
The two remaining channel-list packing calls belong to its menu-item row;
server-list layouts remain with their Stage 5 model conversion.

Primary API families:

- `gtk_container_*`
- `gtk_box_pack_*`
- `gtk_widget_show_all`
- `gtk_widget_destroy`
- `GtkEventBox`, `GtkTable`, and legacy child enumeration

Deliverables:

- Move windows, boxes, panes, notebooks, scrollers, and utility constructors to
  GTK4 child APIs and explicit ownership.
- Convert low-risk dialogs and utility windows first.
- Preserve geometry persistence, tab/window modes, and close behaviour.

Exit criteria:

- No converted surface depends on GTK3 container ownership.
- Window creation/destruction and repeated open/close tests are leak- and
  crash-free.

### Stage 3: Actions, Menus, Dialogs, And File Selection (Complete)

Action-identity foundation pass 1 (2026-07-14): the 16 commands shared by
main-menu accelerators and configurable keyboard shortcuts now carry their
stable names and typed internal identifiers directly in the canonical menu
definition. Menu construction and shortcut dispatch consume that same data,
removing duplicated string and positional mappings while preserving every
existing callback, accelerator, and special Ctrl+Q preference check. GTK3 menu
widgets, dynamic menu mutation, state, sensitivity, dialogs, and popup/event
handling remain unchanged.

Stateless action activation pass 2 (2026-07-14): 12 canonical commands now
activate through a per-menu `GSimpleActionGroup`. Their GTK3 menu items use
`GtkActionable` names, retain the shared group after construction, and no
longer attach parallel widget callbacks; configurable shortcuts still dispatch
through the same canonical command path. A focused GTK3 runtime probe verified
one activation and production-equivalent group lifetime. Menu-bar, user-list,
fullscreen, and away remain on existing callbacks until their state and
sensitivity synchronization can be converted as one unit.

Window-view state pass 3 (2026-07-14): menu-bar visibility, user-list
visibility, and fullscreen now bind their GTK3 check items to boolean
`GSimpleAction` state. Menu-bar and user-list preference changes synchronize
the corresponding action in every window; fullscreen activation updates the
requested state immediately and the existing window-state event corrects it to
the platform-observed result. Existing command names, accelerators, callbacks,
and GTK3 behaviour are preserved. Away remains deferred because its state and
sensitivity belong to the active server session rather than the window alone.

Session-aware Away pass 4 (2026-07-14): the final canonical command now binds
to a boolean `GSimpleAction` whose state is updated only by confirmed server
away/back events. Menu creation, active-tab changes, and connect/disconnect
events synchronize action availability with the selected server. Disabled
actions suppress menu and accelerator activation, while configurable shortcuts
retain their existing canonical direct-dispatch path. The plain GTK3 menu-item
presentation and IRC command behaviour remain unchanged; GTK4 menu models are
still deferred.

Search menu-model pass 5 (2026-07-14): the first complete static subtree now
has an immutable `GMenuModel` projection generated from the same canonical
labels and action identities as the live menu. The three Search commands and
the shared action group are retained together on the menu root; the existing
GTK3 submenu remains the displayed surface and preserves its callbacks,
accelerators, and layout. Dynamic `/MENU`, plugin, and user-menu content is
excluded from this pass pending a dedicated mutation and ownership boundary.

Help menu-model pass 6 (2026-07-14): About now carries the seventeenth
canonical command identity and activates through the shared stateless action
path. The retained model builder now projects arbitrary contiguous canonical
subtrees and is used for both the three-command Search submenu and complete
two-command Help menu. The built-in shortcut vocabulary includes About, while
the displayed GTK3 Help menu, Contents URL handling, and About dialog behaviour
remain unchanged.

New menu-model pass 7 (2026-07-14): Channel Tab and Channel Window now carry
the eighteenth and nineteenth canonical command identities. Together with the
existing Server Tab and Server Window actions, they form a retained four-entry
New submenu model generated by the shared range builder. The existing creation
handlers, temporary preference restoration, default shortcuts, and displayed
GTK3 submenu remain unchanged.

Server menu-model pass 8 (2026-07-15): Disconnect, Reconnect, Join a Channel,
and Channel List now carry the twentieth through twenty-third canonical
command identities. Together with the existing Away action, they form a
retained five-command Server model whose two sections preserve the live menu's
separator. Disconnect and Join sensitivity now initializes from the selected
server and follows connection events through the shared action group, while
the existing callbacks, dialogs, IRC commands, and displayed GTK3 menu remain
unchanged.

Channel Switcher menu-model pass 9 (2026-07-15): the Tabs and Tree radio items
now share the twenty-fourth canonical action identity, a string-valued
`channel-switcher` action with typed `tabs` and `tree` targets. The retained
two-item model stores those targets for GTK4. The displayed GTK3 radio items
retain their existing callback because GTK3 does not reliably dispatch a radio
target through `GtkActionable`; both user and programmatic layout changes keep
the action state synchronized without duplicate activation or layout work.

Network Meters menu-model pass 10 (2026-07-15): Off, Graph, Text, and Both now
share the twenty-fifth canonical action identity, a string-valued
`network-meters` action with four typed targets. The retained model stores the
complete submenu for GTK4, while the displayed GTK3 radio items keep their
existing callbacks. Both activation paths use one meter update routine, keep
open menu action groups synchronized, reinitialize lag timers, and refresh the
meter UI exactly as before.

Settings menu-model pass 11 (2026-07-15): Preferences and all nine static
configuration editors now carry canonical stateless action identities. The
retained Settings model preserves the existing two-section structure, and the
displayed GTK3 items activate through the shared action dispatcher. Existing
preference and editor windows, configuration files, labels, and behavior are
unchanged; their dialog lifecycle conversion remains separately scoped.

Window menu-model pass 12 (2026-07-15): the nine operational-window commands,
Clear Text, and Save Text now carry canonical stateless identities. The
retained Window model preserves its separator and nests the existing Search
model after the five transcript commands. Relative downstream boundaries also
correct stale Search and Help offsets introduced when the earlier New submenu
grew by two entries. GTK3 presentation, transcript behavior, and the contained
Save Text file-selection flow remain unchanged.

View menu-model pass 13 (2026-07-15): Topic Bar, User List Buttons, and Mode
Buttons now use synchronized boolean actions alongside the existing Menu Bar,
User List, and Fullscreen actions. The retained View model has three sections
and nests the existing Channel Switcher and Network Meters target models.
Cross-window preference synchronization remains authoritative, while displayed
GTK3 check/radio items and their layout, timer, and visibility behavior remain
unchanged.

Fabulor menu-model pass 14 (2026-07-15): Load Plugin or Script and the
context-sensitive Attach/Detach command now have canonical stateless action
identities. The retained five-section Fabulor model nests New and preserves the
live menu's separator structure. Attach/Detach labels are copied after each
window's context is selected, and plugin loading continues through the existing
contained chooser and no-plugin fallback. This completes static main-menu model
projection; dynamic menu mutation and popup ownership remain separate work.

Dynamic Usermenu model pass 15 (2026-07-15): each main-window menu root now
retains a complete `GMenuModel` projection of `usermenu.conf`. The recursive
builder preserves nested submenus, separator-defined sections, command targets,
preference toggles, icon hints, and the trailing Edit This Menu command.
Usermenu edits atomically replace the model and its action-owned toggle data for
each distinct window, avoiding references to retired configuration entries.
The displayed GTK3 Usermenu and its command parsing remain unchanged; plugin
`/MENU` mutation and contextual popup ownership remain separate work.

Plugin main-menu model pass 16 (2026-07-15): finalized main-menu `/MENU`
entries now rebuild a retained per-window overlay after adds, state/sensitivity
updates, and recursive deletion. The overlay accepts only built-in or previously
created parent paths, preserves nested custom submenus and separator sections,
and records requested position, accelerator, markup, icon, group, and enabled
metadata. Stateless, toggle, and radio actions own copied lookup keys and are
fully retired before replacement, so freed plugin entries cannot remain
reachable. The displayed GTK3 menu and plugin command behavior remain
unchanged; contextual popup roots remain separate work.

Contextual plugin popup model pass 17 (2026-07-15): each `$NICK`, `$URL`,
`$CHAN`, `$TAB`, and `$TRAY` invocation now owns an allowlisted, nested
`GMenuModel` projection and a distinct `fabulor-context` action group. Actions
copy the popup root and target, use weak owner references for stateful refresh,
and are retired with the popup owner. Targeted commands retain nickname and
context substitution; untargeted, toggle, and radio commands retain their
existing dispatch paths. The displayed GTK3 popups and native Windows tray
menu remain unchanged until their later presentation conversion.

Response-driven acknowledgement dialogs pass 18 (2026-07-15): theme-manager
apply errors and colors.conf import result messages no longer enter nested
`gtk_dialog_run()` loops. Each modal dialog now connects the shared exact-signature
response destroy callback, is shown normally, and is destroyed with its parent.
Message text, severity, and buttons remain unchanged, and none of these call
sites consumes a response result. The color manager and every decision-returning
dialog remain separate work.

Theme colour-manager lifecycle pass 19 (2026-07-15): the manager no longer
enters a nested dialog loop. Its response handler keeps Reset in place while
refreshing staged colours and rows, and finalizes the staged-change flag before
destroying on Close or window dismissal. The manager is destroyed with its
preferences parent. Nested live colour pickers are also parent-bound and own
their callback data through destruction, so closing the parent cannot leave a
dangling manager-row pointer. Preview, reset, cancel, and staged commit/discard
semantics remain unchanged.

Client-certificate chooser pass 20 (2026-07-15): the server-list editor now
uses a modal `GtkFileChooserNative` response flow instead of the repository's
last `GtkFileChooserDialog` and its nested run. The request owns copied network,
certificate-root, and destination strings and only a weak parent reference.
Parent destruction hides and releases the chooser; accepted local files retain
the existing private-directory creation, byte copy, file permissions, guarded
button refresh, and parent-bound success/error message. Cancel and missing-file
responses remain silent. Two pre-existing blocking native theme-import flows
are now explicitly tracked for later conversion.

Theme import chooser pass 21 (2026-07-15): colors.conf/HCT and GTK3 theme
archive selection now use modal `GtkFileChooserNative` response flows without
nested native-dialog runs. Each request weakly owns its launching button and
preferences parent, disconnects parent cleanup before accepted dispatch, and
resolves live colour-change or theme-page state only after selection. Parent
destruction hides and releases the chooser. Local single-file selection,
extension filters, archive containment, staged colour updates, pevents import,
theme discovery refresh, result messages, and silent cancellation remain
unchanged. No blocking native-dialog run remains in the frontend inventory.

Dialog lifecycle closure pass 22 (2026-07-15): the manifest-plugin security
confirmation now resumes preference persistence only from an accepted response,
with the preferences window retained for the callback lifetime. The singleton
quit/minimize confirmation is explicitly modal and handles preference changes,
tray activation, cancellation, and shutdown from its response callback. Fatal
font failure hides the unusable transcript widget and exits only after its
owned error dialog is acknowledged. Ordinary frontend messages no longer
enter nested loops; the root-account warning uses an asynchronous modal flag,
while logging errors remain non-modal. Windows command-line help, version, and
directory requests use the native UTF-16 message box because they complete
before the GTK event loop exists. No `gtk_dialog_run()` or
`gtk_native_dialog_run()` call remains in the frontend.

Primary surfaces:

- `menu.c`, context menus, tray menus, and user-defined menus
- synchronous `gtk_dialog_run()` call sites
- message, file chooser, colour, font, and confirmation dialogs

Deliverables:

- Represent commands through `GAction`/`GMenuModel` where appropriate.
- Replace blocking dialog loops with response-driven or asynchronous flows.
- Preserve command sensitivity, accelerators, context, and cancellation paths.

Exit criteria:

- No GTK4 path depends on removed menu widgets or nested dialog event loops.
- File/add-on/theme selection retains the existing containment checks.

### Stage 4: Input, Events, Clipboard, Drag/Drop, And Shortcuts (In Progress)

Shared explicit-copy pass 1 (2026-07-15): ban-list masks, channel-list names
and topics, URL context actions, and URL-history rows now use one typed
GTK3/GTK4 clipboard boundary. The obsolete caller-supplied `GdkAtom` parameter
is removed because all five callers requested the same behavior. GTK3 retains
standard clipboard and X11 primary-selection updates; GTK4 obtains both from
the widget display and avoids writing twice when a backend aliases them.
Transcript selection ownership remains in `xtext.c` for its Stage 6 custom
widget conversion. Paste, edit-box, spell-check, emoji, and drag/drop paths are
unchanged.

Simple controller foundation pass 2 (2026-07-15): the Character Chart hover
label now uses a typed pointer-enter boundary backed by
`GtkEventControllerMotion` on GTK4. Join-dialog entry selection and theme-colour
commit-on-focus-loss use the same closure-owned pattern with
`GtkEventControllerFocus`. Their GTK3 branches retain the existing event
signals without exposing event objects to workflow callbacks. The
scroll-to-bottom `GtkButton` now uses its semantic `clicked` signal instead of
decoding a raw button event, preserving pointer activation while adding the
button's normal keyboard activation. Callback data is released with the
widget/controller signal closure. Message entry, transcript, spell-check,
emoji, menu, and drag/drop event paths remain unchanged.

Channel switcher scroll pass 3 (2026-07-15): tree, viewport, tab, and tab-close
wheel input now shares a typed delta callback. GTK4 uses a capture-phase
`GtkEventControllerScroll` on both axes and consumes the event only when the
existing channel/tab logic handles vertical movement. GTK3 smooth and discrete
wheel events are normalized behind the same boundary. Channel-switch speed,
the preference choosing channel switching versus tab-strip scrolling, smooth
direction, and native tree scrolling when switching is disabled remain
unchanged. Redundant GTK3 event-mask setup and widget parameters in tab-strip
animation helpers are removed.

Main focus pass 4 (2026-07-15): detached message entries, standalone session
windows, and the shared tab window now use the typed focus-enter boundary.
GTK4 focus controllers preserve the window-level contains-focus transition, so
moving focus among descendants does not repeat the window-focus notification.
Session selection, server-session initialization, marker visibility, plugin
`Focus Window` events, and taskbar flashing cleanup remain unchanged. No direct
`focus-in-event` connection or `GdkEventFocus` workflow callback remains in the
production frontend. Key handling, activation/send, completion, history,
spell-check, and emoji insertion are unchanged.

Channel-tab close hover pass 5 (2026-07-15): tab close-button hit testing now
receives typed pointer coordinates and leave notifications through one shared
motion-controller boundary. GTK4 uses `GtkEventControllerMotion`; GTK3 retains
motion and leave event signals behind the compatibility layer. Pointer cursor
selection is likewise widget-scoped on GTK4 and hides native `GdkWindow`
cursor ownership from the tab workflow. Close-button prelight, preference
visibility, left-click close dispatch, right-click context menus, scrolling,
pressed/toggled handling, and the retained outer-tab prelight suppression are
unchanged.

Simple keyboard controller pass 6 (2026-07-15): detached utility-window
Escape, search-bar Escape, and raw-log `Ctrl+Shift+C` now receive only key
values and modifier state through one typed boundary. GTK4 uses a bubble-phase
`GtkEventControllerKey`, allowing window controllers to observe descendant key
input; GTK3 retains `key-press-event` behind the compatibility layer. All three
callbacks remain non-consuming. Embedded utility tabs still ignore Escape,
search toggling and focus restoration are unchanged, and raw-log copying still
requires disabled auto-copy plus both Control and Shift. Main input shortcuts,
completion, history, topic editing, tree/list navigation, transcript input,
spell-check, and emoji handling remain unchanged.

Semantic key actions pass 7 (2026-07-15): topic Return and keypad Enter
submission plus exact `Ctrl+A` selection in the channel key and user-limit
fields now use the typed key-controller boundary. Handled keys remain consumed;
all other keys continue to propagate. Topic command dispatch and focus return,
channel-mode activation, modifier exclusions, field limits, and entry styling
are unchanged. The main chat input's configurable shortcut engine remains the
only direct key-event registration in `maingui.c` and is deferred to its own
behavior-preserving pass.

Main shortcut engine pass 8 (2026-07-15): the chat input now feeds normalized
key value and modifier state from the shared key-controller boundary into
plugin notification, binding lookup, and all 17 configurable actions. GTK4
callback consumption replaces the GTK3-specific signal-stop call while
retaining the existing action return contract. Session resolution, plugin
first refusal, plugin-triggered tab closure checks, exact modifier filtering,
commands, menu actions, page and tab movement, scrolling, buffer insertion,
history, completion, replacement, and completion reset on Space remain
unchanged. The shortcut-editor tree's Shift+Up/Down row ordering remains a
separate raw key workflow for its Stage 5 model conversion.

Topic pointer pass 9 (2026-07-15): topic URL hover and leave now use the shared
motion-controller boundary and a text-view cursor helper. Modified left-button
release uses a typed click boundary backed by `GtkGestureClick` on GTK4; only a
successfully activated URL claims the gesture sequence. GTK3 retains motion,
leave, and button-release events behind the compatibility layer. Text-view
window-to-buffer coordinate conversion, URL/host detection, URL range trimming,
the configured modifier comparison, topic selection/editing, Return submission,
and focus restoration remain unchanged.

External file-drop pass 10 (2026-07-15): private-dialog transcript drops and
user-list nickname drops now share a typed URI-list callback. GTK4 accepts
`GdkFileList` through `GtkDropTarget` and serializes file URIs into the existing
DCC-send boundary. GTK3 retains `text/uri-list` selection delivery behind the
compatibility layer and copies the payload using its explicit byte length.
Private-dialog-only transcript behavior, pointer-resolved user rows, nickname
targets, filename conversion, transfer speed preferences, and one DCC send per
file remain unchanged. Internal channel-view, user-list, and pane-position drag
operations remain isolated for pass 11.

Internal layout-drag pass 11 (2026-07-15): channel-view and user-list sources,
their reciprocal targets, the transcript scrollbar target, and four-position
pane placement now share a typed source identity and coordinate callback.
GTK4 uses `GtkDragSource`, preloaded `GtkDropTarget` controllers, and a
process-local pointer payload that cannot be serialized as an external file or
text drop. GTK3 retains exact `ZOITECHAT_CHANVIEW` and `ZOITECHAT_USERLIST`
targets with `GTK_TARGET_SAME_APP` inside the compatibility layer. Source
identity no longer depends on MOVE/COPY action codes or a leading-character
target check; that legacy check still expected the pre-ZoiteChat HexChat prefix
and rejected the current targets. User-list row hover and completion cleanup,
source icons, top/bottom targeting, collision adjustment, and immediate pane
replacement are preserved. Transcript selection remains with the custom-widget
stage.

Primary API families:

- direct `GdkEvent` handlers and event masks
- `gtk_widget_get_window()` and `gdk_window_*`
- GTK3 clipboard, selection, drag/drop, and accelerator handling

Deliverables:

- Use GTK4 event controllers and gestures for keys, pointers, scrolling, focus,
  and clicks.
- Use `GdkClipboard` and GTK4 content providers.
- Migrate drag/drop and keyboard shortcuts without changing IRC input semantics.
- Measure edit-box latency and emoji-picker responsiveness throughout this
  stage.

Exit criteria:

- Typing, Enter-to-send, completion, history, paste, URLs, spell-check popup,
  emoji insertion, scrolling, selection, and drag/drop pass the interaction
  matrix.

### Stage 5: Lists, Trees, Models, And Channel Navigation

Model architecture pass 1 (2026-07-15): reusable GTK4-only model stacks now
define the first conversion boundary. Flat operational lists use an app-owned
`GListStore`, optional `GtkSortListModel`, and `GtkMultiSelection` before a
future `GtkListView` or `GtkColumnView`. Hierarchical channel navigation uses
per-level `GListStore` ownership, `GtkTreeListModel`, and
`GtkSingleSelection` before a future `GtkListView` with `GtkTreeExpander`.
Executable MSVC and Meson probe contracts cover sorting, selection persistence
across insertion, identity-based removal, hierarchy expansion, depth, and
cleanup. Production GTK3 models and views are deliberately unchanged in this
architecture pass; each following pass will replace one complete owning
surface without parallel model writes. The detailed contract is in
[`list-model-architecture.md`](list-model-architecture.md).

Notify List pass 2 (2026-07-16): the first flat operational list now has one
cross-version owner. `notifygui.c` supplies immutable row snapshots and consumes
selected notify/server identity without `GtkTreeIter`, `GtkTreePath`, hidden
column traversal, cell renderers, or direct tree-model calls. The GTK4 owner
uses typed row objects, `GListStore`, `GtkSortListModel`,
`GtkSingleSelection`, `GtkColumnView`, and four signal-list-item factories;
the production GTK3 branch retains the visible tree while compiling the same
snapshot, selection, and lifecycle contract. Refreshes reuse rows, notify only
changed fields, avoid unchanged-order splices, preserve exact selection, and
fall back to the same notify owner across offline/online row transitions.
Executable probes cover duplicate rejection, refresh ordering, identity
fallback, selected action data, and cleanup. Manual GTK4 visual, keyboard, and
accessibility validation remains for the production frontend cutover.

User-list model pass 3 (2026-07-16): per-session user-list storage is now an
opaque cross-version owner rather than a `GtkListStore` plus externally owned
`GtkTreeRowReference` map in session state. One row snapshot builder supplies
privilege icon/prefix markup, escaped nickname and typing state, hostname,
away/nickname colour, and stable `struct User` identity to both toolkit paths.
The GTK4 branch uses typed row objects, `GListStore`, `GtkSortListModel`, and
`GtkMultiSelection`; it exposes only the sorted list and selection models needed
by the next shared-view pass. Sorting covers privilege/alphabetic ascending and
descending modes plus insertion order, updates can distinguish sort-key changes,
and row properties notify only when presentation data changes. Executable probes
cover duplicate rejection, all sort directions, unsorted insertion, external
sort-key change, update/remove misses, identity removal, and cleanup. The shared
user-list view, factories, pointer hit-testing, menus, keyboard forwarding, and
drag/drop remain in the next contained pass.

User-list view pass 4 (2026-07-16): one cross-version view owner now attaches
the active per-session model, exposes selected `struct User` identities, and
contains scrolling, selection, row hit-testing, and model switching. The GTK4
branch uses `GtkListView`, `GtkMultiSelection`, signal-list-item factories, and
composed icon/prefix/nickname/optional-host rows; factory bind/unbind owns row
notification lifetimes and uses current list-item positions after sorting. The
GTK3 branch retains the shipping columns, width persistence, and pointer menu
placement inside the same owner contract. `userlistgui.c` no longer performs
tree-model traversal or direct tree selection for commands, file drops, pane
drag highlighting, `/USELECT`, tab switching, or selected-user snapshots.
Multi-click and key handling now use shared controllers, while nick-menu popup
coordinates cross a neutral menu boundary. Manual GTK4 visual, keyboard,
accessibility, and large-channel checks remain for the production cutover.

Channel-navigation model pass 5 (2026-07-16): channel hierarchy and
presentation state now belong to one identity-indexed cross-version owner.
`chanview.c` no longer owns a `GtkTreeStore`, retains `GtkTreeIter` values in
channel records, or traverses toolkit rows for insertion, lookup, removal,
reparenting, and family movement. The GTK4 branch supplies typed row objects,
per-parent `GListStore` ownership, `GtkTreeListModel`, and
`GtkSingleSelection`; selection is restored by stable channel identity when
rows move or change parent. The shipping GTK3 branch mirrors the same neutral
hierarchy into a contained tree store and resolves its internal row references
only on demand. Executable probes cover duplicate rejection, hierarchy order,
rename, cyclic movement, reparenting, removal, and selection persistence. The
visible channel tree and grouped tab switcher remain as separate presentation
passes because their row factories and family-button lifecycles are distinct.

Channel-tree view pass 6 (2026-07-16): the visible hierarchical switcher now
uses one cross-version view owner. The GTK4 branch combines the existing typed
hierarchy and single-selection model with `GtkListView`, `GtkTreeExpander`, and
a signal factory for optional icons, ellipsized names, row attributes, and
focus underlines. Selection, parent expansion, pointer hit-testing, focus
scrolling, and expansion queries resolve stable channel identity. Factory row
notifications and the model-owned selection listener are disconnected at their
respective lifetimes; the runtime probe destroys the view and then reuses the
model with fatal GLib critical diagnostics enabled. The shipping GTK3 branch
retains exact tree lines, indentation, compact rows, double-click expansion,
off-screen-only scrolling, context hit-testing, channel switching, and internal
drag/drop behind the same owner. `chanview-tree.c` no longer contains direct
tree-view, renderer, iterator, or path operations. The grouped tab strip is
reserved for the following passes because its family boxes, close controls, animated
scrolling, and reordering form an independent presentation lifecycle.

Channel close/context input pass 7 (2026-07-16): channel-tree and grouped-tab
pointer actions now use the shared cross-version multi-click controller. The
channel callback contract carries the source widget, button, coordinates, and
modifier state rather than `GdkEventButton`; no raw button-event type or
`button-press-event` connection remains in `chanview.c`, `chanview-tree.c`, or
`chanview-tabs.c`. Tree identity resolution remains in the tree owner, while
tab close-button hit-testing remains presentation-local. The retained GTK3 tab
menu adapter uses the current event only for native pointer placement and falls
back to widget anchoring. Middle-click close, right-click menus, close-button
hit areas, tree row targeting, and event consumption are preserved. The grouped
tab-strip owner, family boxes, animated scrolling, and reordering remain for
pass 8.

Grouped-tab state pass 8 (2026-07-16): scroll animations, their movement flags,
and toggle-signal suppression now belong to each `chanview` instead of static
process-wide variables. Cleanup cancels both possible timeout sources before
destroying the tab tree, preventing callbacks from retaining a removed view and
allowing detached windows to animate independently. Absolute and relative tab
focus now resolve through the authoritative flattened channel model, removing
the family-box and toggle-child traversal previously used to find the active or
requested tab. The unused scroll-button visibility allocation callback and its
never-populated widget fields are removed. Family-box creation, tab presentation
updates, and widget reordering remain for the next contained pass.

Grouped-tab reorder pass 9 (2026-07-16): tab and family movement now follows
the authoritative channel hierarchy after the shared move workflow updates it.
A child tab resolves its current sibling position from the model and accounts
for the leading server tab in its family box; moving a server/root repositions
the complete family box at its current model root position. The reorder paths
no longer enumerate GTK children or rediscover families through widget data.
Family-box construction, close presentation, animated scrolling presentation,
keyboard handling, and drag/drop remain for the grouped-tab owner conversion.

Grouped-tab family-owner pass 10 (2026-07-16): every tab view now owns an
explicit map from authoritative model root identity to its family box and
separator. Root insertion creates one record; child insertion resolves its
model parent and uses the current sibling position instead of comparing widget
userdata. Child removal leaves the root family intact, root removal destroys
and unregisters the emptied family after reparenting, and view cleanup releases
all records across orientation and implementation changes. Family discovery,
empty-box pruning, and sorted insertion no longer enumerate GTK children or
store family identity on GTK widgets. Close presentation, scroll geometry,
keyboard handling, and drag/drop remain for later grouped-tab passes.

Grouped-tab item-owner pass 11 (2026-07-16): each tab view now owns one
channel-identity-indexed presentation record containing the tab button, label,
and close button. Rename and colour updates, close hit-testing, hover cleanup,
tab removal, and view teardown resolve those records rather than the former
`tab-label`, `tab-close-button`, and channel GTK object data. Family and item
maps share one private state owner so the implementation remains within the
compile-time-checked channel-view scratch boundary on both pointer widths.
Widgets are destroyed before item records during whole-view cleanup, while an
individual record remains live through its tab destruction callback window.
Cross-version close geometry and hover presentation remain for the next pass.

Grouped-tab close-presentation pass 12 (2026-07-16): close hit-testing now
uses one cross-version descendant-point helper. GTK4 transforms parent pointer
coordinates into the close button with `gtk_widget_compute_point()` and tests
`gtk_widget_contains()`; GTK3 retains the exact translated-allocation bounds
inside the compatibility layer. Explicit prelight and whole-tab prelight
suppression helpers preserve close hover, pointer cursor feedback, left-click
close dispatch, and context fallback without `GdkEventCrossing`, raw enter/leave
signals, allocation reads, or coordinate translation in `chanview-tabs.c`.
The strict GTK4 probe compiles all three new helper signatures and branches.
Animated scrolling presentation, keyboard handling, and drag/drop remain.

Grouped-tab scroll-presentation pass 13 (2026-07-16): animated scroll target
discovery now iterates the authoritative flattened channel model and resolves
each tab through the per-view item owner. A cross-version descendant-origin
helper measures every tab in the shared inner-strip coordinate space; GTK4
uses `gtk_widget_compute_point()` while GTK3 coordinate translation is contained
inside the helper. This removes the final GTK child enumeration and local child
allocation read from `chanview-tabs.c`, and fixes target positions across
multiple nested family boxes. The view state retains its scrolled window
explicitly and obtains horizontal or vertical adjustments from that owner rather
than casting the inner strip's parent. Frame timing, cancellation, speed,
direction, wheel preference, and endpoint behavior remain unchanged. Grouped-tab
keyboard handling and drag/drop remain.

Grouped-tab activation pass 14 (2026-07-16): left-click activation now runs
through the shared cross-version multi-click press controller after visible
close-button dispatch has had priority. Keyboard activation remains on the
cross-version `GtkToggleButton` `toggled` signal, and the GTK3-only `pressed`
signal dependency has been removed. The closing drag/drop audit found no
tab-local drag/drop implementation; moving the complete channel view already
uses the typed Stage 4 source/drop-controller boundary. This completes the
grouped-tab owner conversion. Remaining Stage 5 work is the contained
conversion of operational lists and editors, with manual GTK3 and GTK4 cutover
validation retained in the validation log.

Loaded Add-ons list pass 15 (2026-07-16): the Plugins and Scripts window now
uses a dedicated cross-version typed owner for immutable name, version, file,
description, and canonical-path rows. GTK4 uses the shared flat model stack,
`GtkSingleSelection`, and four resizable `GtkColumnView` factories; GTK3 keeps
the shipping list-store presentation inside the same boundary. Refresh,
unload, and reload no longer read tree models, iterators, or selections in
`plugingui.c`. The strict GTK4 probe compiles the owner and executes append,
row-count, clear, and cleanup checks. Selection-driven actions remain in the
manual GTK3 and GTK4 cutover matrix because the headless probe intentionally
does not create a display-backed view.

URL History list pass 16 (2026-07-16): URL History now uses a dedicated
cross-version owner for immutable URL rows. GTK4 combines the shared flat model
stack, `GtkSingleSelection`, `GtkListView`, and a signal-item factory; GTK3
retains the shipping one-column tree inside the owner. New URLs prepend at
position zero and both branches remove the oldest rows beyond the configured
limit. Selection, copy, clear, and point-to-row lookup no longer expose tree
models, paths, iterators, or selections to `urlgrab.c`. Double-click opening and
right-click menus receive typed press count, coordinates, and modifiers through
the shared multi-click controller; a typed GTK3 URL-menu adapter preserves exact
pointer placement. The strict probe verifies newest-first order, truncation,
clear, and cleanup.

Ignore List pass 17 (2026-07-16): the first editable operational table now has
one cross-version owner for masks and the complete ignore-engine flag word.
GTK4 uses typed row objects, the shared flat model stack, `GtkSingleSelection`,
a sortable `GtkColumnView`, editable-label mask cells, and seven check-button
factories. GTK3 keeps its editable text and toggle renderers inside the same
owner. Typed callbacks validate mask renames and synchronize visible flag
changes with the common ignore engine. Add, delete with next-row selection,
confirmed clear, mask snapshots, and initial population no longer expose tree
models, paths, iterators, cell renderers, or selections to `ignoregui.c`.
Historical flag values, including DCC bit 128 and hidden `IG_NOSAVE` bit 64,
remain intact across edits. The strict probe covers accepted and rejected
renames, callbacks, flag preservation, snapshots, clear, and cleanup.

Ban List pass 18 (2026-07-17): the Ban List now has one cross-version owner for
numeric mode identity, mask, setter, display date, parsed timestamp, and
multi-selection state. GTK4 uses typed rows, the shared flat model stack,
`GtkMultiSelection`, four sortable `GtkColumnView` factories, and coordinate
hit-testing. GTK3 retains its shipping list-store presentation inside the
owner. Refresh population, row and selection counts, right-click selection,
copy mask/entry, selected removal, inverse crop removal, and confirmed clear no
longer expose tree models, paths, iterators, or selections to `banlist.c`.
Mode-filtered command snapshots use numeric Ban, Exempt, Invite, and Quiet
identity instead of translated type labels. The strict probe covers mixed
selection, mode filtering, inversion, select-all, callbacks, clear, and cleanup.

DCC transfer-list pass 19 (2026-07-17): the combined Uploads and Downloads
window now has one cross-version owner for mutable transfer presentation and
stable DCC identity. GTK4 uses typed rows, the shared flat model stack,
`GtkMultiSelection`, one direction-image factory, and eight text factories;
GTK3 keeps its shipping list-store presentation inside the owner. Progress
updates mutate existing rows. Filtering, details, accept, resume, abort,
clear-completed, and row activation consume identity snapshots without direct
tree-model access in the file-transfer paths in `dccgui.c`. The strict probe
covers prepend/append order, duplicate rejection, update, multi-selection,
removal, callbacks, clear, and cleanup. The distinct DCC Chat schema and
lifecycle were left isolated as the immediate pass 20 target.

DCC Chat pass 20 (2026-07-17): the separate Chat window now has one
cross-version owner for mutable status, nick, received and sent counters,
start time, colour, multi-selection, and stable DCC identity. GTK4 uses typed
rows, the shared flat model stack, `GtkMultiSelection`, and five text
factories; GTK3 retains its shipping list-store presentation inside the owner.
Refresh, prepend population, Accept, Abort, row activation, and removal no
longer expose toolkit rows to `dccgui.c`. The strict probe covers duplicate
rejection, update, ordered multi-selection, callbacks, removal, clear, and
cleanup. The combined transfer and distinct Chat schemas are now both
converted without merging their protocol workflows.

Channel List pass 21 (2026-07-17): the high-volume `/LIST` window now has one
cross-version owner for stable row identity, channel, users, topic, collation
key, sorted presentation, multi-selection, hit-testing, export snapshots, and
column widths. GTK4 uses typed immutable rows, the shared flat model stack,
`GtkMultiSelection`, three sortable `GtkColumnView` factories, and incremental
sorting after view creation. GTK3 retains its specialized `CustomList`
append-and-final-resort implementation inside the owner to preserve current
large-list performance. The frontend keeps its immediate first-row display,
250 ms pending batches, local user/search filtering, and authoritative core
row lifetime while no longer traversing toolkit rows for join, copy, context,
save, refresh, or teardown. The strict probe covers collation order, duplicate
rejection, multi-selection, selected text, sorted export records, and cleanup.

Generic editable-list pass 22 (2026-07-17): the shared two-column editor used
by Commands, Popups, User Menu, Replace, URL Handlers, User List Buttons,
Dialog Buttons, and CTCP Replies now has one cross-version owner for copied
name/command rows, editable cells, single selection, add/delete, ordering, and
save snapshots. GTK4 uses typed mutable rows, the shared unsorted flat model
stack, `GtkSingleSelection`, two `GtkEditableLabel` factories, and row-object
drag/drop; GTK3 keeps its editable renderers and native reorderable tree inside
the owner. Shift+Up/Down now enters through the shared key-controller boundary
and performs boundary-safe model movement. Configuration serialization,
post-save reload, button refresh, and user-menu refresh remain in the workflow
wrapper. The strict probe covers editing, movement, snapshots, deletion,
empty-row initialization, and cleanup.

Print Events pass 23 (2026-07-17): the event-text editor and selected-event
argument help now share one cross-version owner for copied row values, stable
signal identity, single selection, callback-gated inline edits, and help-row
population. GTK4 uses typed event and help rows, `GtkSingleSelection`, two
`GtkColumnView` instances, and `GtkEditableLabel`; GTK3 keeps its list stores,
renderers, tree views, and selection inside the owner. Parsing, argument-count
validation, transcript preview, load/save, and theme behavior remain in
`textgui.c`. The strict probe covers accepted and rejected edits, row identity,
selection callbacks, help rows, clearing, and cleanup.

Key-bindings pass 24 (2026-07-17): the shortcut editor now has one
cross-version owner for copied accelerator/action/data rows, custom identity,
single selection, mutation rules, ordering, reset preservation, and save
snapshots. GTK4 uses typed rows, `GtkSingleSelection`, key-capture buttons,
`GtkShortcutLabel`, `GtkDropDown`, editable labels, and a key controller;
GTK3 retains its accelerator/combo/text renderers and list store privately.
Configuration parsing, built-in matching, action dispatch/help, serialization,
and menu accelerator refresh remain in `fkeys.c`. The strict probe covers
normalization, protected built-ins, custom edits, movement, deletion,
snapshots, selection callbacks, clear, and cleanup.

Preferences sound-event pass 25 (2026-07-17): the event/sound-file table now
has one cross-version owner for copied names, mutable filenames, stable event
indices, and explicit single selection. GTK4 uses typed rows,
`GtkSingleSelection`, and two `GtkColumnView` factories; GTK3 keeps its list
store, renderers, tree view, and selection privately. Browse, play, filename
normalization, core `sound_files` ownership, and preference cancellation state
remain in `setup.c`. The strict probe covers selection callbacks, identity-safe
updates, missing-event rejection, clear, and cleanup.

Preferences category-navigation pass 26 (2026-07-17): the Interface, Chatting,
and Network hierarchy now has one cross-version owner for copied labels,
stable page indices, expansion, non-selectable headings, and remembered single
selection. GTK4 uses typed rows, per-category child stores,
`GtkTreeListModel`, `GtkSingleSelection`, `GtkListView`, and a tree-expander
factory; GTK3 keeps its tree store, renderer, tree view, and selection filter
privately. Lazy page creation and notebook switching remain in `setup.c`. The
strict probe covers hierarchy counts, stable page selection, callback
de-duplication, missing-page rejection, and cleanup.

Server List network-table pass 27 (2026-07-17): the main network chooser now
has one cross-version owner for copied display names, stable `ircnet` identity,
favorite emphasis, single selection, inline rename, filtered refresh order,
and keyboard movement. GTK4 uses typed mutable rows, the shared flat model
stack, `GtkSingleSelection`, `GtkListView`, and an editable-label factory;
GTK3 keeps its list store, renderer, tree view, and selection privately. Add,
remove, sort, favorite toggling, favorites-only refresh, remembered selection,
scrolling, and Shift+Up/Down no longer traverse toolkit rows in
`servlistgui.c`. The strict probe covers insertion order, duplicate rejection,
selection, movement boundaries, updates, removal, clear, and cleanup. The
detailed Servers, Autojoin channels, and Connect commands tables remain the
next contained server-editor pass.

Server List editor-table pass 28 (2026-07-17): the Servers, Autojoin channels,
and Connect commands tables now share one cross-version owner for stable
core-object identity, copied display values, optional secondary text, single
selection, inline editing, and keyboard movement. GTK4 uses typed mutable rows,
the shared flat model stack, `GtkSingleSelection`, `GtkColumnView`, and
editable-label factories; GTK3 keeps its stores, renderers, tree views, and
selections privately. Add, remove, canonicalized edits, empty-value deletion,
channel-key clearing, and Shift+Up/Down no longer traverse toolkit rows in
`servlistgui.c`. The strict probe covers duplicate labels, identity selection,
updates, movement boundaries, one-column constraints, removal, clear, and
cleanup. This completes the Stage 5 implementation inventory; manual GTK4
visual, keyboard, accessibility, and performance checks remain part of the
production cutover validation.

Primary surfaces:

- server list and editor
- channel/user/notify/ignore/ban/DCC lists
- preferences trees, key bindings, event text, and custom list models
- channel tree and tab switcher

Deliverables:

- Choose GTK4 list/model widgets per workflow rather than mechanically retaining
  deprecated tree abstractions.
- Preserve sorting, editing, selection, keyboard navigation, context menus,
  badges/icons, and large-list performance.
- Retire GTK3 cell-renderer assumptions as each surface moves.

Exit criteria:

- All operational lists pass keyboard, mouse, sorting, editing, and scale tests.
- Channel switching and user-list updates remain responsive under IRC load.

### Stage 6: Custom Text And Input Widgets

Transcript render-target pass 1 (2026-07-17): `GtkXText` now owns one contained
render destination instead of raw draw-window, draw-surface, and draw-context
fields. The existing Cairo/Pango renderer requests referenced contexts from
that owner. GTK3 retains its direct-window fallback privately; GTK4 has an
explicit `GtkSnapshot` Cairo begin/end path that produces a render node without
a display. Cairo and Graphene are now explicit strict-probe dependencies. The
probe covers empty targets, offscreen painting, active-context exchange and
restoration, snapshot output, and cleanup. Geometry, class virtual methods,
controllers, selection, clipboard behavior, and performance remain separate
passes. The detailed contract is in
[`transcript-rendering-architecture.md`](transcript-rendering-architecture.md).

Transcript geometry pass 2 (2026-07-17): wrapping, page and partial rendering,
selection auto-scroll, entry visibility, line recalculation, and buffer-switch
size tracking now use one validated widget-geometry boundary. GTK3 reads its
allocation and GTK4 uses current widget width and height. Native `GdkWindow`
size reads remain only in the private GTK3 window-to-surface capture helper;
pointer lookup and smooth-scroll capture keep their required native references.
The strict probe verifies positive and rejected geometry independently of a
display. Class virtual methods, controllers, selection, clipboard behavior,
and performance remain separate passes.

Transcript widget-class pass 3 (2026-07-17): one cross-version adapter now
installs transcript measurement, allocation, realization, unrealization, and
render class methods. GTK3 receives its existing preferred-size and Cairo draw
contracts; GTK4 receives `measure`, width/height/baseline allocation, and
snapshot rendering through the established render target. `GtkXText` retains
content-specific Pango lifecycle, resize recalculation, and the contained GTK3
native child-window operations behind one static callback table. The strict
probe registers a headless GTK4 widget subclass and verifies all five class
slots, fixed minimum requests, and width-change policy. Controllers, selection,
clipboard behavior, and performance remain separate passes.

Transcript input-controller pass 4 (2026-07-17): pointer motion and leave,
single/multiple click press, click release, scrolling, and focus changes now
connect through shared GTK3 signal or GTK4 controller helpers instead of
`GtkWidgetClass` event virtual methods. A modifier-aware motion contract
preserves selection and timestamp behavior. A toolkit-neutral input owner
classifies character/word/line selection and scroll direction, while
`FabulorXTextClick` replaces the borrowed `GdkEventButton` in word-click
dispatch. Coordinate-based popup entry points preserve URL, nickname, channel,
and context-menu placement. Only GTK3 selection ownership and payload slots
remain for the next clipboard pass.

Transcript selection pass 5 (2026-07-17): `xtext-selection.c` now owns the
toolkit-version clipboard boundary. GTK3 retains deferred PRIMARY target
registration, UTF-8 and locale payload conversion, explicit CLIPBOARD updates,
and PRIMARY/SECONDARY ownership through signal connections rather than
`GtkWidgetClass` event slots. GTK4 publishes one owned string content provider
to the display CLIPBOARD and PRIMARY clipboard, tracks provider identity, and
clears Unix selection highlighting when another owner replaces PRIMARY.
Windows keeps its existing persistent-highlight behavior. The strict probe
compiles the GTK4 provider path and verifies complete and bounded payload
copying. No direct selection event or payload virtual method remains in
`GtkXTextClass`.

Transcript frame-redraw pass 6 (2026-07-17): page rendering no longer requires
a native `GdkWindow` before reaching the full renderer. `xtext-scroll-copy.c`
owns the tested overlap, pixel-copy, and damage-region policy. GTK3 retains its
native-window capture optimization when a partial vertical copy is valid;
GTK4 deliberately rejects native capture and performs a complete
snapshot-backed redraw. Shared compatibility helpers now own CSS class
attachment, focused-root detection, and damage redraw requests, with GTK4
expanding partial damage to a widget frame. Native pointer and background
window helpers are compiled only for GTK3. The strict probe verifies upward,
downward, unavailable-capture, and full-height scroll cases.

Transcript background-composition pass 7 (2026-07-17): one Cairo-only owner
now retains the optional background source, builds and clears its frame-local
viewport cache, contains image fitting with black letterboxing or repeated
non-image surfaces, and paints the palette fallback when composition is
unavailable. `GtkXText` no longer stores cache dimensions, tile offsets, or
render-cycle state. The owner keeps the existing 8192-pixel viewport safety
bound and releases all referenced Cairo surfaces during replacement and
teardown. The strict probe verifies exact fallback, letterbox, fitted-image,
surface-presence, and cleanup behavior without a display.

Transcript decorations pass 8 (2026-07-17): `xtext-decoration.c` now owns
marker-line placement, persistent search-match boundary classification, and
transient URL/nickname hover-highlight ranges and paint modes. The text parser
still applies established palette colours and Cairo lines, but no longer
carries six independent hover range and render-state fields or its own search
offset walker. Adjacent search occurrences retain current-match precedence,
timestamp rendering temporarily suspends hover decoration without mutating its
range, and teardown releases one opaque owner. The strict probe covers marker
positions, adjacent and current search matches, hover boundaries, paint/clear
modes, suspension, and cleanup.

Transcript hit-testing pass 9 (2026-07-17): `xtext-hit-test.c` now owns
coordinate-to-scrollback-line mapping, separator tolerance, IRC-formatting
offset adjustment, and validated word-match results. The `word_click` signal
captures classification and match bounds synchronously instead of asking its
consumer to classify the shared scratch word again and read global last-match
state. URL, host, nickname, channel, and email consumers duplicate only the
validated matched substring and no longer terminate text inside the transcript
scratch buffer. The strict probe covers negative-y mapping, separator edges,
formatted offsets, invalid bounds, immutable duplication, and non-link result
types.

Transcript accessibility and display-scale pass 10 (2026-07-17): the custom
widget now exposes the toolkit log role and stable localized `Transcript`
label on GTK3 and GTK4. `xtext-display.c` owns logical font metrics,
strike/underline positions, scale normalization, and inline-image dimensions.
Emoji flags load and cache at device resolution for the widget's scale while
wrapping, hit testing, and cursor placement retain their existing logical
width. The strict probe verifies metric rounding, image bounds and 1x/2x/3x
sizing, coordinate conversion, decoration placement, and the GTK4 role. Full
accessible scrollback text exposure, production screen-reader checks, and
scrollback performance remain separate validation targets.

Transcript accessible-text pass 11 (2026-07-17): GTK4 `GtkXText` now
implements the native read-only `GtkAccessibleText` interface through a tested
owner. It exposes recent plain valid UTF-8 transcript content with character
offsets, displayed timestamps, hidden-run removal, Pango Unicode boundaries,
and a 1 MiB safety bound. Appends, trims, clears, timestamp changes, and buffer
switches coalesce into one idle refresh; common-prefix/suffix comparison emits
minimal removal and insertion notifications after the interface is first
queried. Unobserved sessions perform no snapshot work, and teardown cancels queued work.
The strict probe verifies range slicing, character/word/sentence/line
boundaries, insertion diffs, size containment, and interface registration.
GTK3 keeps its existing ATK role/name. Production screen-reader behavior,
read-only selection/geometry limitations, and scrollback performance remain
explicit validation targets.

Transcript performance pass 12 (2026-07-17): append refresh policy now keeps
bottom-of-buffer local echo immediate, coalesces historical-view bursts, and
prevents append-owned trimming from scheduling a separate delayed redraw.
Wrapped-line limiting removes complete oldest entries until the configured
bound is met while preserving a sole oversized newest entry. Deterministic
policy outcomes are gating; elapsed probe timing is informational.

Spell-input word-boundary pass 1 (2026-07-17): `spell-entry-words.c` now owns
Pango segmentation and paired UTF-8 byte and character ranges. Spell checks
and Pango attributes use bytes; `GtkEditable` dictionary, ignore, and
replacement actions use characters. This corrects non-ASCII targeting and
replaces three parallel arrays plus repeated cleanup blocks with one owner.
The strict GTK4 probe covers `café`, cursor lookup, duplication, and empty
input. Class lifecycle, formatting attributes, menus, and production Enchant
validation remain separate passes. The detailed contract is in
[`spell-input-architecture.md`](spell-input-architecture.md).

Spell-input widget-lifecycle pass 2 (2026-07-17): the custom `GtkEntry`
subclass is retained and now inherits the toolkit's `GtkEditable` contract
instead of registering an empty duplicate interface. No-op draw and legacy
button/style class virtuals are removed. Pointer marking uses a normalized
click boundary, redraw uses `gtk_widget_queue_draw()`, and an owned theme
listener refreshes caret and underline colours before being unregistered at
dispose. GTK3 keeps exact public-layout hit testing; GTK4 uses its internal
editable cursor after click processing because it no longer exposes entry
layout coordinates. Dynamic context-menu position remains a separate pass.

Spell-input styling pass 3 (2026-07-17): `spell-entry-style.c` now owns one
Pango attribute list for hidden IRC controls, bold, italic, strikethrough,
underline, reset, reverse, mIRC colours, and misspelling ranges. The widget
supplies resolved semantic and mIRC colours without exposing GTK or theme
state to the owner. Reverse formatting now swaps the semantic default colours
instead of treating theme-token enum values as mIRC indexes. The strict probe
validates formatting-disabled state, toggles, hidden shaping, colour roles,
reverse semantics, trailing colour-parameter hiding, and spell underline ranges.

Spell-input dynamic-menu pass 4 (2026-07-17): `spell-entry-menu.c` now owns a
GTK-independent `GMenuModel` projection for suggestions, per-language add,
session ignore, spell enablement, IRC attributes, and colours 0-15. GTK4 owns
one action group and current extra-menu model per entry, refreshes suggestions
only for pointer or keyboard context-menu requests, and resolves language
targets against the active dictionary table at activation time. The GTK3
`populate-popup` and markup colour menu remain explicitly contained until
cutover. The strict probe validates disabled, multi-dictionary, no-suggestion,
long-suggestion, formatting, colour, and action-count contracts.

Spell-input URL and initialization pass 5 (2026-07-18): URL-shaped tokens are
now identified from the same immutable UTF-8 word snapshot and excluded from
both live Enchant checks and on-demand suggestion menus. Wrapped absolute URIs
and `www.` links retain normal editable behavior without generating dictionary
work. The entry now initializes its Pango attributes, empty word owner, and
preference state before activating Enchant dictionaries, so startup cannot
recheck or replace uninitialized owners. The strict probe covers ordinary
words beside wrapped HTTPS URLs, multibyte path/query text, emoji, and `www.`
links. Interactive emoji, clipboard, shortcut, persistence, accessibility,
high-DPI, and latency validation remain for the production GTK4 frontend.

Spell-input emoji-picker ownership pass 6 (2026-07-18): `emoji-picker.c` now
owns one popover reference per edit box, exact GTK3/GTK4 parent teardown,
one-time lazy-page state, and validated flag/codepoint sequence construction.
The removed notebook path is replaced by a shared stack and switcher, with
only the visible category populated. GTK4 flag images use paintables and the
existing child/reveal adapters; GTK3 behavior remains compiled by the shipping
frontend. The strict probe exercises page and Unicode policy while compiling
the exact GTK4 popover lifecycle. Production GTK4 interaction, accessibility,
high-DPI, asset, and latency checks remain open until the frontend is runnable.

Primary surfaces:

- `xtext.c` / `xtext.h`
- `sexy-spell-entry.c` / `sexy-spell-entry.h`

Deliverables:

- Port the transcript widget from GTK3 realize/window/draw/event virtual methods
  to GTK4 measurement, allocation, snapshot rendering, focus, selection, and
  event-controller semantics.
- Port or replace the `GtkEntry` spell-check subclass while preserving Enchant
  2.8.19 checking, suggestions, add-to-dictionary, URL paste stability, and
  personal-dictionary persistence.
- Preserve IRC formatting, timestamps, markers, search, URL hit-testing,
  selection, copy, scrolling, and scrollback performance.

Exit criteria:

- Transcript and edit-box behavioural tests pass on normal and high-DPI displays.
- No GTK3 windowing or draw virtual method remains in either widget.
- Input and sent-message display latency meet the baseline thresholds.

### Stage 7: Themes, Tray, Notifications, And Platform Integration

Primary surfaces:

- `theme/theme-gtk3.c` and GTK3 theme import/service assumptions
- CSS provider installation and display/theme change notifications
- `plugin-tray.c`, AppIndicator/StatusNotifier, Win32 tray, and notifications
- icon, fontconfig, GSettings, and runtime path setup in `fe-gtk.c`

Deliverables:

- Add a GTK4 theme adapter for system GTK4 desktop themes and imported themes
  under `%APPDATA%\Fabulor\themes`.
- Retain `.hct` and `colors.conf` palette/event imports. Retire `.zct`, GTK3
  theme import/discovery, and `%APPDATA%\Fabulor\gtk3-themes` after the adapter
  is ready; do not convert or claim direct compatibility for GTK3 CSS.
- Do not package mock Windows GTK themes or an optional default Fabulor theme.
  Follow Windows light/dark and high-contrast policy through the platform
  adapter, using GTK4 runtime defaults when no custom GTK4 theme is selected.
- Preserve Fabulor palette/custom-CSS behaviour and dark-mode selection.
- Validate native Windows tray/notification paths independently from optional
  Unix tray backends.
- Remove GTK3-specific icon-path and theme workarounds only after replacement
  behaviour is proven.

GTK4 theme-discovery pass 1 (2026-07-18):
`gtk4-theme-discovery.c` now owns toolkit-independent metadata for exact
`gtk-4.0/gtk.css` layouts found in desktop roots and the Fabulor profile
`themes` directory. It records source identity, localized display names,
optional dark CSS and previews, suppresses canonical duplicates, and excludes
GTK3-only layouts. No CSS is parsed or applied in this pass, and the shipping
GTK3 adapter remains unchanged. The contract is documented in
[`theme-architecture.md`](theme-architecture.md).

GTK4 CSS-provider pass 2 (2026-07-18):
`theme/theme-gtk4.c` now owns GTK4 display-scoped CSS providers and explicit
follow-system, light, and dark variant policy. Candidate providers are parsed
before active providers are replaced, so missing or invalid CSS leaves the
current theme intact. Parser diagnostics, provider priority, active identity,
and complete teardown are covered by the strict GTK4 probe. Preferences and
production discovery integration remain a later pass, and the shipping GTK3
adapter remains unchanged.

GTK4 theme-preferences pass 3 (2026-07-18):
`gtk4-theme-preferences.c` now projects discovered themes into an owned,
toolkit-independent choice list with an explicit system-default entry. Exact
stable identifiers resolve persisted selections; missing identifiers fall back
safely while remaining distinguishable from an intentional default selection,
and invalid variant values normalize to follow-system. Dedicated
`gui_gtk4_theme` and `gui_gtk4_variant` configuration keys keep GTK4 state
separate from GTK3. Production GTK4 preference widgets and adapter application
remain deferred until the frontend cutover boundary is available.

GTK4 Windows appearance-policy pass 4 (2026-07-18):
the GTK-independent preference owner now resolves whether GTK4 may install a
custom provider and whether it should prefer dark styling from a post-discovery
selection, variant policy, Windows application preference, and high-contrast
state. High contrast suppresses custom CSS and dark requests; the system-default
choice installs no custom provider and follows Windows. The existing Win32
registry, `SPI_GETHIGHCONTRAST`, and settings-change notification paths remain
the platform signal owners. The GTK4 provider adapter consumes the decision and
removes active custom providers for high-contrast or system-default outcomes.
Production signal/UI hookup and packaged visual testing remain deferred.

GTK4 theme-controller pass 5 (2026-07-18):
`theme/theme-gtk4-controller.c` now composes discovery, owned preference
projection, persisted selection resolution, Windows appearance decisions, and
transactional provider application behind one GTK4-only lifecycle owner.
Invalid CSS preserves the prior provider and committed selection; unavailable
stored themes commit an observable system-default fallback; high contrast
removes active custom CSS without discarding the selected preference. Discovery
metadata can be released after refresh, and controller destruction tears down
display providers. Production preferences binding remains deferred.

Notification backend containment pass 6 (2026-07-18):
the built-in notification plugin now owns initialization errors and each
platform backend has an explicit, repeatable lifecycle. On Windows, the WinRT
helper resolves only from the executable-relative `plugins` directory, all
required exports are validated before invocation, partial initialization is
unwound, and teardown clears call targets before unloading the retained module.
The WinRT apartment is initialized before notifier creation and balanced only
when owned. Freedesktop and fallback backends follow the same managed error and
teardown contract. Production GTK4 presentation and packaged interaction tests
remain deferred. The boundary is documented in
[`notification-architecture.md`](notification-architecture.md).

Tray action-model pass 7 (2026-07-18):
`tray-action-model.c` now owns a toolkit-neutral `GMenuModel` and
`GActionGroup` for window visibility, away/back, blink preferences, settings,
and quit. Copied labels, stable action names, snapshot-driven sensitivity and
state, malformed-away normalization, typed dispatch, and final callback-data
cleanup are covered by the strict GTK4 probe. Existing GTK3/AppIndicator and
Win32 presentation remains unchanged; live plugin binding, dynamic `$TRAY`
entries, and native shell-icon ownership are later passes. The contract is
documented in [`tray-architecture.md`](tray-architecture.md).

Tray live-binding pass 8 (2026-07-18):
the built-in tray plugin now owns the action model from initialization through
deinitialization, projects live visibility, aggregate away, and blink settings,
and routes every typed action through the existing application commands.
Window, settings, and away transitions refresh the model; borrowed menu/action
accessors refresh on demand. Snapshot comparisons suppress unchanged menu
notifications, while visibility changes alone rebuild the hide/restore section.
The model is now part of production MSVC and Meson source lists. Presenter
replacement and dynamic `$TRAY` composition remain later passes.

Tray plugin-composition pass 9 (2026-07-18):
the tray boundary now rebuilds the dynamic `$TRAY` plugin subtree on demand and
inserts it into an immutable presenter projection after the first two built-in
sections. Linked menu structure, action names, and plugin metadata survive
source-model release. Presenters receive explicit owned references to the menu,
built-in actions, and plugin actions; teardown disables and disconnects retained
built-in actions before releasing callback state. Shipping GTK3/AppIndicator
and Win32 popup rendering remain unchanged until a presenter consumes this
projection.

GTK4 tray popover-presenter pass 10 (2026-07-18):
`tray-menu-presenter-gtk4.c` now consumes an immutable projection and both
action groups behind one candidate `GtkPopoverMenu` owner. Replacement routes
both `tray` and `fabulor-context` namespaces to the new groups without retaining
caller references. Teardown closes and unparents the popover, removes the menu
and both groups, and leaves an externally retained widget inert. Namespace
constants are shared across model generation, legacy attachment, and GTK4
presentation. Production anchoring and platform tray backend selection remain
deferred.

Tray backend-selection pass 11 (2026-07-18):
`tray-backend-policy.c` now selects disabled, Windows shell, StatusNotifier,
legacy GTK3 status icon, or unavailable from explicit capabilities. Shipping
initialization and preference restart use the policy without changing current
GTK3 behavior. GTK4 and unknown toolkit versions cannot select the removed
legacy status-icon fallback; Unix-like GTK4 builds require an available
StatusNotifier backend. Stable backend names and the full decision matrix are
covered by the strict probe. Native backend implementation remains deferred.

GTK4 theme-preference binding pass 12 (2026-07-18):
`theme-preferences-gtk4.c` now owns the candidate GTK4 theme and variant
controls, lifecycle controller, discovered choice model, and persistence
callback boundary. Selections apply transactionally before committed values
are emitted; invalid CSS restores the prior controls and active provider.
Unavailable saved themes and high contrast have explicit status, appearance
refresh does not produce persistence writes, and teardown disconnects controls
before unparenting the owned surface. Production preferences-window insertion
and packaged Windows appearance testing remain deferred until GTK4 cutover.

GTK4 Windows appearance-monitor pass 13 (2026-07-18):
`theme-appearance-monitor-gtk4.c` now owns a display-scoped GTK4 Win32 message
filter for `WM_SETTINGCHANGE` and `WM_THEMECHANGED`. Signals coalesce onto the
main loop before registry and high-contrast state are queried, and unchanged
state does not reapply providers. Appearance refreshes route through the owned
preference/controller stack without persistence writes. Query or provider
failure preserves the last committed appearance, while teardown removes both
the filter and pending source. Production startup ownership and packaged
Windows visual testing remain deferred until GTK4 cutover.

Theme-format and payload-contract pass 14 (2026-07-18):
`tools/validate_theme_contract.py` now enforces `.hct` as the only active theme
file association, retains `colors.conf` loading and atomic persistence, and
rejects `.zct` from active installers and import filters while preserving
upgrade cleanup. It also rejects repository-authored theme payloads and WiX
harvest rules for `.hct`, `.zct`, `colors.conf`, or `share/themes`, preventing
an optional default Fabulor theme from entering the package. Repository lint
runs the validator and isolated negative tests on every PR. GTK runtime data
and icon assets remain separate allowlisted dependencies.

Exit criteria:

- Theme switching, restart persistence, tray state, notifications, icons,
  fonts, spell-check UI, and emoji rendering pass on the packaged GTK4 runtime.

### Stage 8: Production Cutover And Runtime Cleanup

Runtime staging-contract pass 1 (2026-07-18):
`tools/gtk4/runtime-payload-contract.json` now defines a deterministic Windows
x64 runtime candidate from exact native files and required data trees in the
pinned GTK4 root. `stage_runtime.py` rejects unsafe or build-only selections,
normalizes the GDK pixbuf loader cache, and emits a source-bound SHA-256 file
manifest. Windows CI stages the candidate before the GTK4 probes. The shipping
GTK3 payload and transitional WiX GTK4 harvest remain unchanged until this
candidate passes packaged feature validation.

Parallel candidate-package pass 2 (2026-07-18):
WiX now has an explicit opt-in candidate component surface that packages only
the generated GTK4 staging root and its source-bound manifest. Windows CI builds
and uploads a separate candidate MSI after the unchanged production MSI and
bootstrapper succeed. Candidate mode requires the manifest, uses isolated
output and intermediate directories, and suppresses bootstrapper replacement.
The default installer build continues to harvest the transitional GTK4 root.
The candidate component groups preserve each runtime directory explicitly, and
CI decompiles and extracts the MSI to compare all installed `Runtime/GTK4`
paths, sizes, and SHA-256 hashes against the generated manifest. This rejects
missing, unexpected, duplicate, flattened, or content-mismatched payload entries
before artifact upload.

Executable-relative runtime startup pass 3 (2026-07-18):
the future Windows GTK4 target now has a Win32-only early bootstrap contract
that derives `Runtime/GTK4/bin` from the executable path, rejects missing and
reparse-point runtime directories, replaces current-directory and ambient-PATH
DLL discovery with application/System32/explicit runtime search roots, and
retains that root for process lifetime. A standalone probe with no GTK or GLib
imports proves the boundary before production linking changes: CI runs it from
an unrelated directory containing a fake GTK DLL, verifies the absolute loaded
module path and GTK major version, and requires missing and junction-backed
runtime roots to fail closed. The production GTK4 target must invoke this
bootstrap before delay-loaded GTK-family imports; the shipping GTK3 executable
is unchanged in this pass.

Native import-closure pass 4 (2026-07-18):
`runtime-import-contract.json` now assigns the 35 packaged PE files to four
explicit roots: the GTK frontend runtime, SVG image loader, and console/GUI
spawn helpers. A fail-closed validator inspects every staged DLL and executable
with MSVC `dumpbin`, resolves all 107 packaged dependency edges, and permits
only 54 reviewed Windows/UCRT imports. Duplicate basenames, unresolved imports,
legacy GTK3 or lib-prefixed duplicate GLib-family imports, missing roots, and
unowned packaged binaries fail validation. Focused rejection tests run in lint
and the complete candidate graph runs immediately after staging in Windows CI.
This static closure does not justify trimming dynamically selected modules;
feature-driven package tests remain required before removal.

Shipping WiX allowlist pass 5 (2026-07-18):
the transitional broad GTK4 component surface and `GtkRuntimeCandidate` switch
are retired. `GTK4Allowlist.wxs` now supplies the normal MSI's sole GTK4
component group, requires the generated manifest, and preserves explicit
directory ownership. Windows CI builds the normal `Fabulor.msi` and
`FabulorSetup.exe` from the staged root, then validates all 1,431 runtime files
plus the manifest directly in the shipping MSI. The duplicate candidate MSI
build/upload is removed. Production `fabulor.exe` and the root GTK3 payload are
unchanged pending frontend linking and packaged feature validation.

Full-project MSVC build-profile pass 6 (2026-07-19):
the shared Windows property sheet now has an explicit `FabulorGtkMajor=4`
profile that resolves GTK, GLib, resource tools, headers, and import libraries
from the validated GTK4 development root while retaining the existing root for
non-GTK dependencies. All candidate objects, libraries, resources, PDBs, and
executables are isolated beneath `build/gtk4-full`; the default remains GTK3
and still builds the shipping executable. `gtk4-full-frontend.proj` provides a
green full common-library checkpoint, enforced by Windows CI, and an honest
complete frontend compile. That compile now identifies the production cutover
clusters directly: legacy menu event/widget construction, channel-list and
channel-view container APIs, window positioning/lifecycle calls, and direct
GTK3 theme application and preferences integration. No partial GTK4 executable
is emitted or packaged.

Compositor-owned window-placement pass 7 (2026-07-19):
all shared dialog placement hints, saved main/dialog coordinates, startup
coordinate restoration, and tray hide/restore placement now route through one
cross-version policy. GTK3 retains its pointer, centered, parent-centered,
screen, and coordinate behavior. GTK4 deliberately emits no placement request
because the compositor owns native window positions, and unavailable GTK4
coordinates no longer overwrite saved profile values; dimensions remain a
separate lifecycle concern. The strict probe covers every helper, both MSVC
profiles compile, and the full GTK4 inventory no longer reports placement API
errors. Theme display/provider ownership and remaining visibility/geometry
callbacks stay in their dedicated blockers.

Context-menu event-boundary pass 8 (2026-07-19):
the public URL, channel, nick, and middle-click context-menu entry points now
accept only an origin widget, local coordinates, modifier state, and domain
data. Their duplicate raw-event entry points and adapters are removed. GTK3
preserves pointer-relative popup placement by synthesizing its button event in
one private, version-guarded presenter. GTK4 no longer inherits
`GdkEventButton` through `menu.h`; its built-in contextual commands still need
to move from legacy `GtkMenu` construction to the retained action/model
presenter before the frontend can provide these menus. Plugin context models
remain available as the starting boundary for that follow-up.

Context-menu presenter pass 9 (2026-07-19):
a dedicated GTK4 popover presenter now owns the composed context model and its
built-in and plugin action groups. It attaches to the current origin widget,
reparents safely when the origin changes, points at local click coordinates,
and detaches its model, actions, and widget parent during teardown. The strict
runtime probe mounts the origin in a real toplevel, activates both namespaces,
checks the pointer rectangle, and verifies cleanup. This establishes the GTK4
presentation lifecycle without yet projecting the legacy URL, channel, nick,
or middle-click commands into retained models.

URL context-model pass 10 (2026-07-19):
the fixed URL context surface now has a toolkit-neutral retained model and
typed action dispatch. It owns the URL for action lifetime, selects Connect for
IRC schemes and Open otherwise, exposes Copy, and appends the existing plugin
projection as a separate section. MSVC compiles, links, and runs the strict
probe. Meson with Ninja 1.13.2 compiles all 51 objects; the local final link
still stops at the documented Strawberry GCC 4.8.3 versus MSVC import-library
ABI boundary. Configurable `urlhandlers.conf` nesting/path checks and the live
GTK4 presenter connection remain the next URL pass.

URL handler-projection pass 11 (2026-07-19):
the retained URL model now recursively projects configured command handlers,
nested submenus, separator-defined sections, icon hints, and enabled state.
Every action closure owns a copy of its command, so reloading
`urlhandlers.conf` cannot invalidate an open popover. Typed activation carries
the retained URL and exact command to the eventual host adapter. Stateful
toggle actions emit the same `set <preference> 0|1` command as GTK3. The strict
probe covers nested structure, disabled-action suppression, toggle state, and
command lifetime. The live presenter adapter remains open; GTK3 continues to
display and execute its existing widget menu unchanged.

Live URL context-adapter pass 12 (2026-07-19):
the production URL entry point now selects the retained model/presenter path
under GTK4 while leaving GTK3 construction unchanged. The adapter snapshots
the reloadable handler list, applies the existing executable path filter,
extracts labels and icon hints, captures toggle state, rebuilds the `$URL`
plugin projection, and releases temporary descriptors after the model has
copied them. One popup owner attached to the origin widget coordinates model,
presenter, action-group, replacement, and widget-destruction cleanup. Open,
Connect, Copy, configured commands/toggles, and plugin actions retain their
existing host dispatch semantics. The isolated full GTK4 inventory accepts the
new adapter range and continues at older surrounding menu blockers.

Channel context-model pass 13 (2026-07-19):
the fixed channel context surface now has a toolkit-neutral retained model and
typed action dispatch. Explicit joined, current-session, network, and Autojoin
inputs select Join or Focus/Part/Cycle and expose a stateful Autojoin action
only when a persistent server network exists. The model owns its channel name
for popup lifetime and appends the existing `$CHAN` plugin projection as a
separate section. The strict probe activates every joined command, verifies
the Autojoin transition and copied-channel lifetime, and covers the unjoined,
networkless Join-only case. The live GTK4 presenter adapter and its server
lifetime lookup remain open; GTK3 continues to use its existing widget menu.

Live channel context-adapter pass 14 (2026-07-19):
the production channel-menu entry point now selects the retained model and
popover presenter under GTK4 while leaving GTK3 construction unchanged. Join,
Focus, Part, and Cycle preserve their existing current-session command forms;
the `$CHAN` plugin model and actions are composed into the same popup. Autojoin
copies only the network name while the popup is open and resolves the current
`ircnet` configuration when activated, avoiding retained `server` or `ircnet`
pointers across disconnect, closure, or network-list edits. One owner attached
to the origin widget releases the presenter, model, copied network name, and
plugin references on replacement or destruction. The isolated full GTK4
inventory compiles the new model and accepts the complete adapter range before
continuing at older surrounding menu blockers.

Nick context-model pass 15 (2026-07-19):
the fixed nick context shell now has a toolkit-neutral retained model and typed
Reply dispatch. The caller supplies the single-user or multi-selection heading
and explicitly enables Reply only for the applicable selection state. The
model owns its nick for action lifetime and appends the existing `$NICK` plugin
projection as a separate section. The strict probe releases the caller's nick
buffer before activation, verifies heading and plugin ordering, dispatches
Reply once, and covers a multi-selection heading without a Reply action.
Recursive `popup.conf` commands, user-info copy actions, WHOIS refresh, and the
live GTK4 presenter adapter remain separate follow-ups; GTK3 is unchanged.

Nick popup-projection pass 16 (2026-07-19):
the retained nick model now recursively projects `popup.conf` commands,
submenus, separator-defined sections, toggle state, enabled state, and icon
hints before Reply and plugin sections. Each action owns its command copy so a
configuration reload cannot invalidate an open popover. Ordinary commands
carry an explicit multi-selection dispatch flag only when more than one nick
was selected; single-target commands retain the copied nick, while toggles
emit the existing `set <preference> 0|1` form independent of selection. The
strict probe covers nesting, disabled suppression, toggle transition, copied
command lifetime, and both dispatch modes. User-info copy/WHOIS refresh and the
live presenter connection remain open; GTK3 continues unchanged.

Nick information-model pass 17 (2026-07-19):
the retained nick model now projects user-information rows beneath the nick
heading. Copyable rows own their clipboard values through typed actions, while
display-only rows such as Last Msg remain actionless. A model-level refresh
query records incomplete WHOIS information without exposing a visible command
or retaining `User`, `session`, submenu-widget, or other live IRC pointers.
Existing constructors and heading/command/Reply/plugin ordering remain
compatible. The strict probe releases caller-owned labels and values before
activation, verifies copied clipboard dispatch and display-only rows, and
covers both refresh states. Live user snapshots, the safe WHOIS request, and
the production GTK4 presenter connection remain the next contained pass; GTK3
continues unchanged.

Live nick context-adapter pass 18 (2026-07-19):
the production nick-menu entry point now selects the complete retained model
and popover presenter under GTK4 while preserving the GTK3 widget path. It
snapshots `popup.conf`, the applicable single-user details, Reply state, and
the `$NICK` plugin projection before releasing all live source pointers.
Commands preserve single-target and live multi-selection dispatch, information
actions copy their retained values, and Reply resolves the current session only
when activated. Incomplete details submit one hidden WHOIS while the supplied
session is known valid. Subsequent user updates locate the active origin through
a weak reference, verify copied nick and network identity, and replace the
presenter's retained projection from the fresh user snapshot. The popup owner
stores no `User`, `session`, `server`, configuration-entry, or widget pointer.
The fixed notify-item suppression, nested handlers, toggle state, plugin target
policy, and origin-owned cleanup remain compatible with GTK3 behavior.

Middle context-model pass 19 (2026-07-19):
a toolkit-neutral retained model now composes the ordered application-menu
sections needed by the middle-click popup. Callers provide translated display
labels separately from stable plugin path names, so sparse `/MENU` additions
merge into the intended top-level submenu without depending on locale or GTK
mnemonic underscores. Matching plugin content is isolated in a trailing
section, while unmatched plugin roots remain available at top level. The model
retains only copied `GMenuModel` content and owns no action group, widget, or
session state. The strict probe releases every caller model and display-label
buffer, then verifies ordering, mnemonic-insensitive matching, plugin section
composition, and unmatched-root preservation. Main-action namespace support
in the popover presenter and the production middle-click adapter remain the
next contained pass; GTK3 continues unchanged.

Live middle context-adapter pass 20 (2026-07-19):
the shared GTK4 popover presenter now accepts an owned configurable built-in
action namespace and an optional plugin namespace while its original
`fabulor-context` contract remains source-compatible. The production
middle-click entry point builds the retained application sections and dynamic
user/plugin projection from the same `mymenu` definitions and prepared state as
the shipping main menu, then presents them through the main `fabulor` action
group. A private unparented projection widget owns dynamic action data, and the
clicked origin owns the model, presenter, projection widget, and complete
cleanup chain. Repeated popups replace that bundle without retaining a
`session`, `server`, menu entry, or source model. GTK3 continues through its
unchanged accelerator-backed widget-menu branch. This completes the retained
URL, channel, nick, and middle-click context-action target.

Live main menu-bar pass 21 (2026-07-19):
`menu_create_main()` now selects a retained `GtkPopoverMenuBar` under GTK4 and
contains the accelerator-backed widget hierarchy in the GTK3 branch. Main and
middle-click menus share one projection builder for built-in sections, user
commands, `/MENU` plugin actions, state, enabled state, and ordering. The GTK4
menu bar owns lightweight action proxies so existing away, fullscreen,
disconnect, join, layout, meter, and visibility synchronization continues
through the shared `GSimpleActionGroup` without retaining removed menu-item
classes. User-menu and plugin changes rebuild the retained root; GTK4 skips the
obsolete widget mutation callback that precedes the existing model sync. Since
popover menu bars require submenu roots, unmatched top-level plugin commands
are grouped under Add-ons on the menu bar while the middle-click popup retains
its direct unmatched-root behavior. The strict probe executes creation,
action activation, model replacement, and cleanup. The full GTK4 inventory
drops from 236 errors and 380 warnings to 225 errors and 365 warnings. Earlier
GTK3-only menu helpers and accelerator refresh code remain a separate target;
shipping GTK3 behavior is unchanged.

Main-menu accelerator boundary pass 22 (2026-07-20):
`menu_create_main()` no longer exposes an untyped accelerator parameter to
GTK4. Its GTK3 declaration and implementation retain the concrete
`GtkAccelGroup` owner, while the GTK4 call shape contains only model/action
state. Legacy icon-menu construction, accelerator attachment, stored key
metadata, recursive widget traversal, and Ctrl+Q rebinding now compile only in
the GTK3 branch. GTK4 keeps `menu_update_quit_accel()` as an explicit no-op
because configurable shortcuts already resolve the current binding table and
dispatch through canonical `menu_key_action()` identities. The shipping GTK3
frontend rebuilds without warnings or errors, and the strict GTK4 probe remains
clean. The full GTK4 inventory drops from 225 errors and 365 warnings to 217
errors and 363 warnings. Remaining widget-menu helpers belong to the tab/tray
popup presentation conversions rather than the retained main-menu boundary.

Tab context-menu presentation pass 23 (2026-07-20):
the channel-view tab popup now selects a retained model and the shared GTK4
popover presenter while its complete widget-menu implementation remains in the
GTK3 branch. The model snapshots effective alert, logging, scrollback, colour,
join/part, Autojoin, and Auto-Connect state; recursively copies `tabmenu.conf`
commands, toggles, submenus, separators, and icon hints; and composes the
existing `$TAB` plugin projection. Actions preserve channel-option persistence,
logging lifecycle, network-list persistence, configured command dispatch,
Detach, and Close behavior. The clicked tab owns the model, presenter, selected
session/tab context, and plugin actions; replacement or source destruction
queues cleanup for the next main-loop turn so an action cannot release its own
dispatch state. Session actions validate that the selected session is still
live before dereferencing it. The strict GTK4 probe and shipping GTK3 frontend
both compile and run with zero warnings and errors. The isolated full GTK4
inventory remains at 217 errors and 363 warnings because `maingui.c` reaches
the older Win32 window-state header before this adapter. The local Meson/Ninja
attempt became idle before compilation and was terminated without leaving a
build process. Utility tabs retain their Detach/Close-only surface, and GTK3
presentation and behavior are unchanged.

Top-level window-state boundary pass 24 (2026-07-20):
main-window preference persistence, minimize-to-tray behavior, fullscreen menu
state, relayout, tray action refresh, and the Windows auto-hide taskbar
adjustment now consume one typed minimized/maximized/fullscreen/focused state
snapshot. GTK3 keeps `window-state-event` and `GdkWindowState` private to the
owner. GTK4 observes the realized `GdkToplevel` `state` property, detects exact
transitions, preserves pre-realization maximize/fullscreen requests, and
releases its retained surface when the window unrealizes or is finalized. The
Windows native handle is resolved from `GdkSurface` only inside the platform
adapter. Strict GTK4 `/W4 /WX` and shipping GTK3 builds pass with zero warnings
and errors. Removing the obsolete top-level GTK4 Win32 header lets the complete
frontend inventory proceed through `maingui.c`; its authoritative summary is
now 261 errors and 552 warnings, with the separate GTK3 window-to-pixbuf capture
and configure-event geometry paths becoming the first main-window blockers.
The per-window Win32 native-message filter is explicitly confined to GTK3 and
remains a tracked GTK4 display-filter follow-up.

Internal drag-icon capture pass 25 (2026-07-20):
the native `GdkWindow` to Cairo image to `GdkPixbuf` capture pipeline now
compiles only for GTK3, where channel-view and user-list drags retain their
scaled snapshot icon. GTK4 continues through the Stage 4 drag controller's
live `GtkWidgetPaintable`, so no native surface readback or replacement
screenshot path is required. The public drag callback remains inert on GTK4
for source compatibility and is not invoked by the controller. Strict GTK4 and
shipping GTK3 builds pass with zero warnings and errors. The complete GTK4
frontend inventory improves from 261 errors / 552 warnings to 257 errors / 552
warnings; its first `maingui.c` blocker is now the independent
`GdkEventConfigure` geometry callback.

Main-window geometry observer pass 26 (2026-07-20):
main and detached-dialog dimension persistence plus resize-triggered relayout
now consume a typed `FabulorWindowGeometry` snapshot. GTK3 keeps
`configure-event`, `GdkEventConfigure`, native size reads, and available
coordinates private to the observer. GTK4 attaches to the realized
`GdkSurface::layout` signal, reports application-pixel width and height, never
fabricates compositor-owned coordinates, and releases the surface on
unrealize/finalization. The strict probe presents a real GTK4 window and
verifies positive layout callbacks, readable dimensions, absent coordinates,
and safe teardown. Strict GTK4 and shipping GTK3 builds pass with zero warnings
and errors. The complete GTK4 frontend inventory improves from 257 errors / 552
warnings to 248 errors / 550 warnings; `maingui.c` now reaches the independent
legacy dialog icon-size enum. DCC and Server List configure callbacks remain
separate workflow conversions that can reuse the observer.

Quit-dialog icon sizing pass 27 (2026-07-20):
the quit workflow now creates its warning image through a cross-version
presentation helper instead of passing the removed `GTK_ICON_SIZE_DIALOG` enum
from `maingui.c`. GTK4 requests the named symbolic image and assigns a stable
48-pixel size, while GTK3 retains its theme-selected dialog size. The strict
GTK4 probe validates icon identity and pixel sizing against a real image object,
and the shipping GTK3 frontend builds without warnings or errors. The complete
GTK4 inventory improves from 248 errors / 550 warnings to 245 errors / 549
warnings; its first remaining `maingui.c` errors are missing explicit Server
List type and function dependencies in the retained GTK4 tab-menu path.

Tab-menu Server List dependency pass 28 (2026-07-20):
the retained GTK4 tab context workflow now includes the common Server List
model and frontend Server List declarations that own `ircnet`, persistence,
and Autojoin editing. The GTK4-only code no longer compiles accidentally only
when a legacy include chain exposes those declarations. Autojoin and
Auto-Connect dispatch, flag persistence, and GTK3 behavior are unchanged.
Strict GTK4 and shipping GTK3 builds pass with zero warnings and errors. The
complete GTK4 inventory improves from 245 errors / 549 warnings to 239 errors /
546 warnings; its first remaining `maingui.c` blocker is the independent
retired button-relief presentation enum.

Flat-button presentation pass 29 (2026-07-20):
channel-mode toggles, emoji-grid choices, search close/previous/next controls,
reply cancellation, and the nickname command button now request semantic flat
presentation through one cross-version helper. GTK4 applies its standard
`flat` CSS class, while GTK3 keeps `GTK_RELIEF_NONE` private to the helper. The
strict probe constructs a real GTK4 button and verifies the class. Strict GTK4
and shipping GTK3 builds pass with zero warnings and errors. The complete GTK4
inventory improves from 239 errors / 546 warnings to 233 errors / 545 warnings;
its first remaining `maingui.c` blocker is the independent legacy Alt modifier
name.

Alt-modifier normalization pass 30 (2026-07-20):
the existing `STATE_ALT` abstraction now selects GTK4's `GDK_ALT_MASK` and
GTK3's equivalent `GDK_MOD1_MASK`. Configurable key normalization and legacy
key-file parsing, menu shortcut dispatch, Ctrl+A modifier exclusion, and
user-list type-to-input filtering all use the same semantic bit. No accelerator
serialization or runtime shortcut behavior changes. Strict GTK4 and shipping
GTK3 builds pass with zero warnings and errors. The complete GTK4 inventory
improves from 233 errors / 545 warnings to 228 errors / 545 warnings; its first
remaining `maingui.c` blocker is the independent retired frame-shadow API.

Frame and scrolled-window presentation pass 31 (2026-07-20):
the topic scroller, transcript scroller, and meter info frames now request
unframed, framed, and outlined presentation through cross-version helpers.
GTK4 toggles the standard scrolled-window `frame` CSS class and relies on the
`GtkFrame` CSS node for its border; GTK3 retains `GTK_SHADOW_NONE`,
`GTK_SHADOW_IN`, and `GTK_SHADOW_OUT` privately. The strict probe verifies both
framed and unframed GTK4 class states on a real widget. Strict GTK4 and shipping
GTK3 builds pass with zero warnings and errors. The complete GTK4 inventory
improves from 228 errors / 545 warnings to 225 errors / 543 warnings; its first
remaining `maingui.c` blocker is the independent legacy button event mask.

Scroll-to-bottom control pass 32 (2026-07-20):
the overlay control is now a real themed icon button instead of a custom
drawing area registered for legacy button events. It uses
`go-bottom-symbolic`, flat styling, the existing 28-pixel footprint and
position, ordinary button activation, a tooltip, and an explicit accessible
label. The custom Cairo arrow, app-paintable setup, raw event mask, draw signal,
and invalid drawing-area `clicked` connection are removed. Visibility and
adjustment tracking are unchanged. Strict GTK4 and shipping GTK3 builds pass
with zero warnings and errors. The complete GTK4 inventory improves from 225
errors / 543 warnings to 224 errors / 541 warnings; its first remaining
`maingui.c` blockers are the independent retired menu icon-size enum uses.

Shared icon-size contract pass 33 (2026-07-20):
the icon resolver and `gtkutil` image boundary now consume Fabulor-owned menu
and large-toolbar size roles instead of `GtkIconSize`. GTK3 maps those roles to
native toolkit icon roles for named images; GTK4 applies stable 16- and
24-pixel logical sizes through the current one-argument named-image API.
Search close/previous/next buttons use cross-version child ownership, and the
channel-tab close icon joins menu, channel-list, Join, spell, plugin, and pixmap
consumers on the shared contract. Resolved pixbuf assets use the owned logical
values. The strict probe verifies both GTK4 pixel sizes on real images. Strict
GTK4 and shipping GTK3 builds pass with zero warnings and errors. The complete
GTK4 inventory improves from 224 errors / 541
warnings to 212 errors / 537 warnings, and `maingui.c` has no remaining compiler
errors. The first remaining errors are now independent button-box layout,
channel-view shadow, DCC geometry, and chooser boundaries.

Button-box layout pass 34 (2026-07-20):
all active button-box constructors and layout setters now use a Fabulor-owned
start, end, or spread role. GTK3 delegates those roles to native
`GtkButtonBox` behavior; GTK4 creates ordinary oriented boxes with retained
spacing, homogeneous spread allocation, and explicit start/end alignment.
Strict GTK4 and shipping GTK3 builds pass with zero warnings and errors. The
complete GTK4 inventory improves from 212 errors / 537 warnings to 196 errors /
485 warnings. The first remaining compiler errors are the independent DCC
configure-event geometry boundary, followed by channel-view shadow
presentation and file-chooser ownership.

DCC geometry pass 35 (2026-07-20):
the detached transfer window now records positive width and height through the
shared cross-version surface-layout observer instead of a raw GTK3 configure
event. The existing detached-only preference condition, initial dimensions,
and reopen behavior are unchanged. Strict GTK4 and shipping GTK3 builds pass
with zero warnings and errors. The complete GTK4 inventory improves from 196
errors / 485 warnings to 188 errors / 484 warnings. Its first remaining errors
are the independent channel-view shadow presentation sites, followed by
channel-list indexing and retained menu ownership.

Channel-view frame presentation pass 36 (2026-07-20):
the scrollable tab strip now requests unframed presentation and the tree
switcher requests framed presentation through the existing cross-version
scroller helper. GTK3 retains its exact `NONE` and `IN` shadow styles; GTK4
uses the standard `frame` CSS class. A full rebuild also replaced the hidden
deprecated GTK3 icon-size lookup with Fabulor's already-owned 16- and 24-pixel
values for resolved pixbuf assets. Strict GTK4 and full shipping GTK3 rebuilds
pass with zero warnings and errors. The complete GTK4 inventory improves from
188 errors / 484 warnings to 186 errors / 483 warnings. The first remaining
error is the Join dialog's retired window type hint, followed by chooser,
channel-list indexing, and retained menu boundaries.

Dialog-window hint pass 37 (2026-07-20):
Join, shared utility windows, Server Editor, and Network List now request their
GTK3 dialog window-manager hint through one semantic compatibility helper.
GTK4 keeps the concrete `GtkDialog` identity and established transient/modal
relationships without the removed GDK hint enum. Full strict GTK4 and shipping
GTK3 rebuilds pass with zero warnings and errors. The complete GTK4 inventory
improves from 186 errors / 483 warnings to 182 errors / 480 warnings. The first
remaining error is channel-list indexing, followed by chooser ownership, the
shared top-level constructor, and retained menu boundaries.

Channel List entry-text pass 38 (2026-07-20):
the search workflow now reads its borrowed pattern through a typed
cross-version entry helper. GTK4 uses `GtkEditable` text ownership and GTK3
retains `gtk_entry_get_text()` inside the compatibility boundary. Empty-pattern
detection, glob and plain-text matching, and regex compilation are unchanged.
Full strict GTK4 and shipping GTK3 rebuilds pass with zero warnings and errors.
The complete GTK4 inventory improves from 182 errors / 480 warnings to 181
errors / 474 warnings. The first remaining errors are the shared file-chooser
folder API, followed by top-level window construction and retained menu
boundaries.

Shared file-chooser path pass 39 (2026-07-20):
all shared chooser results and initial-folder requests now cross one `GFile`
boundary that duplicates local filesystem paths for existing callbacks. Single
selection, multiple selection, current-folder selection, initial folders, and
selected-file ordering retain their existing behavior and explicit cleanup.
GTK 4.10 deprecated `GtkFileChooser` as a whole, so its temporary use is
contained in one source owner with narrowly scoped diagnostics until the later
asynchronous `GtkFileDialog` conversion. Full strict GTK4 and shipping GTK3
rebuilds pass with zero warnings and errors. The complete GTK4 inventory
improves from 181 errors / 474 warnings to 174 errors / 392 warnings. The first
remaining error is the shared top-level window constructor, followed by
retained menu ownership.

Top-level window constructor pass 40 (2026-07-20):
shared utility windows, Server Editor, and Server List now use one cross-version
top-level constructor. GTK4 uses the current argument-free API while GTK3 keeps
`GTK_WINDOW_TOPLEVEL` inside the compatibility boundary. Existing role, title,
default size, transient parent, modality, theme attachment, and destruction
semantics are unchanged. Full strict GTK4 and shipping GTK3 rebuilds pass with
zero warnings and errors. The complete GTK4 inventory improves from 174 errors /
392 warnings to 171 errors / 389 warnings. The first remaining errors are now
the retained `menu.c` GTK3 type and constructor boundaries.

Legacy popup-builder containment pass 41 (2026-07-20):
the generic GTK3 popup widget builder and its public declarations are now
compiled only for GTK3. This includes check items, quick items, nested submenus,
configured popup lists, and legacy popup destruction/presentation. GTK4 keeps
the already-converted URL, channel, nick, middle-click, and tab context models
and presenters; shared icon-label parsing and command path filtering remain
toolkit-neutral. Full strict GTK4 and shipping GTK3 rebuilds pass with zero
warnings and errors. A subsequent clean `/t:Rebuild` rebaseline records 169
errors / 435 warnings; the earlier 169 / 372 incremental count is superseded.
The first remaining error is the legacy Away check-menu callback, followed by
the GTK3 `/MENU` widget mutation owner.

Away check-item callback pass 42 (2026-07-20):
the legacy `GtkCheckMenuItem` callback used by GTK3 state synchronization is now
GTK3-private. GTK4 retains the stateful `away-toggle` action and dispatches the
same Away/Back commands through `menu_away_toggle()`. Full strict GTK4 and
shipping GTK3 rebuilds pass with zero warnings and errors. Clean full-profile
comparison moves from 169 errors / 435 warnings to 171 errors / 438 warnings:
six invalid callback parse errors are removed and the compiler exposes eight
additional errors in the following GTK3 `/MENU` widget mutation owner. That
owner is now the first remaining `menu.c` blocker.

`/MENU` widget-mutation containment pass 43 (2026-07-20):
the GTK3 widget-tree implementation for `/MENU` lookup, insertion, deletion,
state updates, radio/toggle callbacks, ordering, accelerators, and popup
injection is now compiled only for GTK3. GTK4 retains the toolkit-neutral
plugin tree and action projection; every core mutation continues through
`fe_menu_sync()`, which rebuilds live main and contextual menu models. Markup
label normalization remains shared. Full strict GTK4 and shipping GTK3 rebuilds
pass with zero warnings and errors. The clean complete GTK4 inventory improves
from 171 errors / 438 warnings to 118 errors / 405 warnings, with no remaining
`menu.c` errors. The first remaining error is the retired icon lookup flag in
`pixmaps.c`, followed by legacy tray window-state ownership.

System-icon pixbuf pass 44 (2026-07-20):
shared pixmap fallback loading now has explicit GTK3 and GTK4 icon-theme owners.
GTK4 resolves a display-scoped `GtkIconPaintable`, reads its backing `GFile`
through an owned stream, and decodes the existing `GdkPixbuf` result at
Fabulor's 16-pixel menu role. GTK3 retains `gtk_icon_theme_load_icon()` and its
forced-size flag privately. Resource and filesystem icon precedence, warning
behavior, and later `GDK_SCALE` multiplication remain unchanged. Full strict
GTK4 and shipping GTK3 rebuilds pass with zero warnings and errors. The clean
complete GTK4 inventory improves from 118 errors / 405 warnings to 117 errors /
401 warnings, with no `pixmaps.c` diagnostics. The first remaining errors are
the legacy tray `GdkWindow` minimized-state reads.

Window-surface ownership pass 45 (2026-07-20):
tray and plugin window status now use the existing cross-version state snapshot,
and one opaque native-handle accessor owns Win32 surface conversion for tray
menus, plugin pointers, titlebar styling, and taskbar adjustment. GTK4 reads
`GdkToplevelState` and `GdkWin32Surface`; GTK3 retains all `GdkWindow` access
inside `window-state.c`. GTK4 restore uses show/present, while GTK3 preserves
its explicit deiconify call. Full strict GTK4 and shipping GTK3 rebuilds pass
with zero warnings and errors. The clean complete GTK4 inventory moves from
117 errors / 401 warnings to 106 errors / 409 warnings: 11 hard surface errors
are removed and eight downstream warnings become visible. The first remaining
error is Raw Log's retired inset-shadow enum, followed by Server List lifecycle
callbacks.

Raw Log framing pass 46 (2026-07-20):
the Raw Log scroller now uses one cross-version constructor and the existing
semantic framed-scroller helper. GTK4 uses its argument-free constructor and
standard `frame` CSS class; GTK3 privately retains null adjustments and
`GTK_SHADOW_IN`. Existing policies, expansion, transcript child ownership,
theme colour sampling, keyboard copying, and action buttons are unchanged.
Full strict GTK4 and shipping GTK3 rebuilds pass with zero warnings and errors.
The clean complete GTK4 inventory improves from 106 errors / 409 warnings to
105 errors / 407 warnings, with no Raw Log diagnostics. Server List lifecycle
and geometry callbacks are now the first remaining errors.

Server List lifecycle and geometry pass 47 (2026-07-20):
the main Server List and network editor now observe size changes through the
shared surface-layout geometry owner. Typed GTK4 `close-request` callbacks
preserve the two distinct close contracts: the editor saves pending network
fields and performs its own destruction, while the main list saves global
configuration and retains startup-exit behavior before normal destruction.
GTK3 keeps `delete-event` privately. Entry reads used by close-time saving and
validation now cross the existing borrowed-text boundary. Strict GTK4 and
shipping GTK3 rebuilds pass with zero warnings and errors. The clean complete
GTK4 inventory improves from 105 errors / 407 warnings to 70 errors / 384
warnings; the large reduction includes the parser cascade previously caused by
retired event structures. Server List framed-scroller construction is now the
first remaining error family.

Server List scroller pass 48 (2026-07-20):
all four Server List scrollers now use the shared cross-version constructor and
semantic frame helper. GTK4 receives its argument-free constructor and
standard `frame` CSS class, while GTK3 privately retains null adjustments and
`GTK_SHADOW_IN`. Scroll policies, network-editor notebook pages, command
tooltip text, typed list child ownership, and selection behavior are unchanged.
Strict GTK4 and shipping GTK3 rebuilds pass with zero warnings and errors. The
clean complete GTK4 inventory improves from 70 errors / 384 warnings to 66
errors / 379 warnings, with no hard `servlistgui.c` diagnostics. Preferences
framing is now the first remaining error family, followed by user-list framing.

Preferences framing pass 49 (2026-07-20):
the lazily populated Preferences pages now use the shared cross-version
scroller constructor, semantic frame helper, and explicit child owner. GTK4
does not inspect internal scroller children. GTK3 privately retains the
shadowless auto-created viewport that prevents a doubled inset frame around
non-scrollable page content. Labels, vertical scroll policy, notebook pages,
lazy page creation, and preference controls are unchanged. Strict GTK4 and
shipping GTK3 rebuilds pass with zero warnings and errors. The clean complete
GTK4 inventory improves from 66 errors / 379 warnings to 64 errors / 373
warnings, with no hard `setup.c` diagnostics. User-list framing is now the
first remaining error, followed by the theme API boundary.

User-list framing pass 50 (2026-07-20):
the main user-list scroller now uses the shared cross-version constructor and
semantic frame helper, while its typed list view retains explicit child
ownership. Expansion, automatic policies, minimum width, user-model identity,
selection, drag/drop, pointer input, and keyboard forwarding are unchanged.
Strict GTK4 and shipping GTK3 rebuilds pass with zero warnings and errors. The
clean complete GTK4 inventory improves from 64 errors / 373 warnings to 63
errors / 371 warnings, with no hard `userlistgui.c` diagnostics. All 63
remaining hard errors are now confined to `theme/`; the theme API boundary is
the next contained target.

Theme window-ownership pass 51 (2026-07-20):
the KDE/Wayland client-side decoration workaround and `GdkScreen` widget reset
are now explicitly GTK3-only. GTK4 retains compositor-managed decorations and
sets the existing dark/light top-level markers through widget-owned CSS
classes rather than a legacy style context. Windows native-titlebar dispatch,
theme-mode resolution, attached-window cleanup, and change propagation are
unchanged. Strict GTK4 and shipping GTK3 rebuilds pass with zero warnings and
errors. The clean complete GTK4 inventory improves from 63 errors / 371
warnings to 53 errors / 365 warnings, with no `theme-manager.c` diagnostics.
Application and CSS provider display ownership is now the first remaining
theme boundary.

Theme provider-ownership pass 52 (2026-07-20):
application-wide CSS installation and removal now use one cross-version owner.
GTK4 applies providers to the default `GdkDisplay`; GTK3 privately retains its
default `GdkScreen`. The owner accepts the caller's reviewed priority and
reports whether installation succeeded, so the top-level application provider
keeps application priority plus one and retries if no display is available.
CSS string loading also crosses one typed boundary, preserving GTK3's data
loader and using GTK4's string loader. Input and palette provider identity,
fingerprints, removal, and user priority are unchanged. Strict GTK4 and
shipping GTK3 rebuilds pass with zero warnings and errors. The clean complete
GTK4 inventory improves from 53 errors / 365 warnings to 38 errors / 354
warnings, with no diagnostics in `theme-application.c` or `theme-css.c`.
Theme style access is now the first remaining error, followed by the private
GTK3 adapter containment.

Theme style-access pass 53 (2026-07-20):
widget style-context palette sampling is now explicitly GTK3-only. Active GTK3
custom themes preserve normal, selected, link, foreground, and background
queries; GTK4 continues through the semantic runtime palette that already owns
its widget and transcript colors. Default IRC colors, user overrides,
dark/light resolution, marker colors, and RGB16 conversion are unchanged.
Strict GTK4 and shipping GTK3 rebuilds pass with zero warnings and errors. The
dedicated Meson routing test was unavailable because the WSL environment lacks
GTK3 development metadata. The clean complete GTK4 inventory improves from 38
errors / 354 warnings to 37 errors / 351 warnings, with no `theme-access.c`
diagnostics. Every remaining hard error is now in the private GTK3 adapter.

GTK3 theme-adapter containment pass 54 (2026-07-21):
the production GTK3 theme adapter now compiles its legacy CSS, settings,
provider, and filesystem implementation only for GTK3. GTK4 retains a small
inert contract in the same translation unit so callers can be migrated without
activating or loading a GTK3 theme provider. The strict MSVC and Meson GTK4
probes compile and execute that production contract under `/W4 /WX`, and the
shipping GTK3 frontend rebuild remains clean. The complete GTK4 frontend does
not yet link: its clean inventory improves from 37 ordinary errors / 351
warnings to zero ordinary errors / 336 warnings, then stops in `xtext.c` because
GTK4 no longer supplies `gdk/gdkwin32.h`. Xtext Win32 GDK surface/header
containment is the next target.

Xtext Win32-header containment pass 55 (2026-07-21):
the transcript no longer includes the private GTK3 `gdk/gdkwin32.h` header or
unused direct Windows/GDK headers. Its Windows-specific dependencies are now
limited to the CRT file-descriptor declarations used by transcript export and
GLib's public executable-relative path helper; GDK/Cairo remains public and
cross-platform. Shipping GTK3 and the strict GTK4 regression probe pass with
zero errors and warnings. The clean complete GTK4 profile compiles every
production frontend source with zero C compiler errors and 336 warnings, then
reaches link for the first time. Link closure reports 89 unique unresolved
symbols over 238 repeated linker diagnostics. Adding the omitted GTK4
list-model implementation to the production GTK4 build inputs is the next
contained target before retiring the remaining linked GTK3 compatibility calls.

GTK4 list-model link-input pass 56 (2026-07-21):
the production MSVC frontend project now compiles `gtk4-list-models.c` only for
the isolated GTK4 profile. The shipping GTK3 build does not consume the source,
while the strict GTK4 probe continues to compile and exercise the identical
implementation under `/W4 /WX`. Shipping GTK3 and strict GTK4 validation pass
with zero errors and warnings. The clean complete GTK4 profile still compiles
every source with zero C compiler errors and 336 warnings; its expected link
failure improves from 89 to 76 unique unresolved symbols and from 238 to 166
repeated linker diagnostics. All 13 converted shared flat/tree model-stack
symbols are now resolved. The next link-closure target is the remaining legacy
GTK3 compatibility-call surface.

Entry compatibility link-closure pass 57 (2026-07-21):
the existing typed entry-text reader now has matching text-replacement and
width-request helpers. GTK4 uses the `GtkEditable` API that owns these
properties; GTK3 retains its original `GtkEntry` calls. Active frontend entry
workflows and spell-entry macros route through the shared boundary. Shipping
GTK3, strict MSVC GTK4, and Meson GTK4 probe validation pass with zero errors
and warnings, including runtime text and width checks. The clean complete GTK4
profile retains zero C compiler errors, improves from 76 to 73 unique unresolved
symbols and from 166 to 144 repeated linker diagnostics, and reduces warnings
from 336 to 278. The next link-closure target remains the legacy GTK3
container/child ownership surface.

Container-inset link-closure pass 58 (2026-07-21):
uniform content spacing now routes through a semantic GTK3/GTK4 owner. GTK3
retains `gtk_container_set_border_width`; GTK4 applies equal margins to the
content widget. Top-level requests are retained on the window and applied when
its child is attached, so callers that configure an inset before building their
content preserve that ordering. Strict MSVC and Meson GTK4 probes verify direct
widget margins, deferred window-child application, and zero-inset reset.
Shipping GTK3 remains clean. The complete GTK4 profile retains zero compiler
errors and improves from 73 to 72 unique unresolved symbols, from 144 to 121
repeated linker diagnostics, and from 278 to 255 warnings. Typed child
attachment and removal are the next container-ownership target.

Typed box-child attachment pass 59 (2026-07-21):
confirmed ordinary box, button-box, channel-tree, and channel-tab outer
attachments now use the typed cross-version box owner. The channel-tab viewport
uses the typed scrolled-window child owner. GTK3 retains the legacy
non-expanding, filling placement, while GTK4 establishes explicit single-owner
child attachment. Shipping GTK3 and the strict GTK4 probe pass. The clean
complete GTK4 profile retains zero compiler errors and 72 unique unresolved
symbols, improves from 255 to 245 warnings, and reduces repeated unresolved
diagnostics from 121 to 111. Event-surface and list-row attachment, generic lazy
page ownership, legacy menu children, and reparent removal remain separate
targets; typed event/list ownership is next.

Content-surface and list ownership pass 60 (2026-07-21):
lag and throttle meter wrappers plus theme preview/color surfaces now use a
semantic content-surface owner. GTK3 preserves event-box visible-window
behavior; GTK4 uses CSS-capable boxes and explicit children. Theme color-manager
rows and their list use direct typed GTK3/GTK4 ownership. Strict MSVC and Meson
GTK4 probes verify surface parents, row children, and list order, and shipping
GTK3 remains clean. The complete GTK4 profile retains zero compiler errors,
improves from 245 to 233 warnings, from 111 to 104 repeated unresolved
diagnostics, and from 72 to 69 unique unresolved symbols. The GTK4 link boundary
contains no event-box symbols. Channel-list menu construction and lazy
Preferences page attachment are the remaining active generic-add callers;
typed lazy-page ownership is next before child reparent/removal.

Lazy Preferences page ownership pass 61 (2026-07-21):
the Preferences registry now owns explicit per-page creation state instead of
inferring it by enumerating GTK3 container children. Each registered page is a
known vertical box; its factory result is attached once through the typed box
owner and revealed through the existing cross-version tree helper. Shipping
GTK3 and the strict GTK4 probe remain clean. The complete GTK4 profile retains
zero compiler errors and 69 unique unresolved symbols, improves from 233 to 228
warnings, and reduces repeated unresolved diagnostics from 104 to 100.
`setup.c` no longer contributes generic child enumeration, attachment, or
`show_all` calls. Typed child reparenting/removal is next.

Layout reparent ownership pass 62 (2026-07-21):
the main channel-view and user-list roots now retain themselves while detaching
from their known pane or grid owner, then reattach through typed pane/grid APIs
before releasing the temporary reference. Initial main-window pane construction
uses the same start/end helpers. GTK3 privately preserves generic removal and
pack resize/shrink arguments; GTK4 clears the matching pane slot or removes the
grid child. Strict MSVC and Meson probes verify pane/grid detach, lifetime,
reattachment, and an unparented no-op, and shipping GTK3 remains clean. The
complete GTK4 profile retains zero compiler errors while improving from 228 to
225 warnings, from 100 to 97 repeated diagnostics, and from 69 to 66 unique
unresolved symbols. The GTK4 link boundary no longer contains
`gtk_container_remove`, `gtk_paned_pack1`, or `gtk_paned_pack2`. Channel-list
menu item construction is the next remaining generic child-ownership target.

Channel List context-menu pass 63 (2026-07-21):
right-clicking a selected Channel List row now snapshots every selected channel
and topic into an owned GTK4 action model, then presents Join, Copy Channel
Name, Copy Topic Text, and first-channel Autojoin through the shared popover.
Action activation no longer reads the mutable live selection. GTK3 privately
retains its icon menu, selection-done cleanup, and favorites callback. Strict
MSVC and Meson probes release the original arrays before checking labels,
action availability, all four dispatches, and Autojoin state. Shipping GTK3
remains clean. The complete GTK4 profile retains zero compiler errors while
improving from 225 to 213 warnings, from 97 to 87 repeated diagnostics, and from
66 to 61 unique unresolved symbols. `gtk_container_add`, `GTK_MENU`,
`gtk_menu_item_new`, `gtk_menu_new`, and `gtk_menu_popup_at_pointer` leave the
GTK4 link boundary. Remaining Channel List button image and window lifecycle
compatibility are the next contained target.

Icon/mnemonic button and Channel List lifecycle pass 64 (2026-07-21):
Channel List and plugin-manager command buttons now use one typed constructor.
GTK4 owns an explicit horizontal image/label child, preserves the mnemonic's
association with its button, and applies the Fabulor menu icon size; GTK3
retains its native image-button behavior. Channel List construction failures
now close through the shared window lifecycle helper. Strict MSVC and Meson
probes verify the GTK4 child tree, icon, size, label, and mnemonic target, while
shipping GTK3 remains clean. The complete GTK4 profile retains zero compiler
errors while improving from 213 to 210 warnings, from 87 to 84 repeated
diagnostics, and from 61 to 60 unique unresolved symbols.
`gtk_button_set_image` leaves the GTK4 link boundary. Remaining Channel View
button presentation and lifecycle calls are the next contained target.

Channel View ownership and lifecycle pass 65 (2026-07-21):
tab and tree scrollers now use the shared constructor; family and tab ordering
uses a typed position-to-sibling translation; close-button flat/image policy
and recursive reveal use shared presentation boundaries. Tabs, families,
scrollers, and implementation roots are removed through their known box owner.
GTK3 retains the root `destroy` signal, while GTK4 finalizes Channel View state
through a weak callback when the main-window-owned root is released. Strict
MSVC and Meson probes verify ordering at both box ends, and shipping GTK3
remains clean. The complete GTK4 profile retains zero compiler errors while
improving from 210 to 204 warnings, from 84 to 80 repeated diagnostics, and
from 60 to 58 unique unresolved symbols. `gtk_box_reorder_child` and
`gtk_button_set_always_show_image` leave the GTK4 link boundary, and the
Channel List/Channel View container and lifecycle target is complete. Remaining
top-level visibility and lifecycle callbacks are next.

Join Channel dialog lifecycle pass 66 (2026-07-21):
the dialog's three choices now use a typed grouped-control boundary. GTK3 keeps
native radio buttons; GTK4 uses grouped check buttons, and both share checked-
state access with the ordinary persistence checkbox. The invalid-server close
path resolves a typed root window, explanatory text uses cross-version wrapping,
the OK response owns default activation, and GTK4 clears the server's dialog
pointer at finalization while GTK3 retains its destroy callback. Strict MSVC
and Meson probes verify exclusive selection, ordinary checkbox state, wrapping,
root lookup, and teardown; shipping GTK3 remains clean. The complete GTK4
profile retains zero compiler errors while improving from 204 to 191 warnings,
from 80 to 72 repeated diagnostics, and from 58 to 56 unique unresolved
symbols. `gtk_radio_button_set_group` and `gtk_label_set_line_wrap` leave the
GTK4 link boundary. DCC choice controls are the next reusable grouped-control
target within the remaining top-level lifecycle work.

DCC grouped-choice pass 67 (2026-07-21):
the Transfers window's Both, Uploads, and Downloads filters now use the shared
typed grouped-control constructor and checked-state reader. GTK4 explicitly
activates the first group member so Both remains the initial filter, also
correcting the Join dialog's Nothing default; GTK3 retains native radio-button
behavior. Strict MSVC and Meson probes verify default activation and exclusive
selection, and shipping GTK3 remains clean. The complete GTK4 profile retains
zero compiler errors and 56 unique unresolved symbols while improving from 191
to 183 warnings and from 72 to 69 repeated diagnostics. `dccgui.obj` is clean,
and Preferences is now the sole remaining retired radio-button caller and the
next contained target.

Preferences grouped-choice pass 68 (2026-07-21):
the Appearance page's Tabs and Tree switcher choices now use the shared typed
grouped-control constructor and checked-state access. The intentionally blank
list entry still reserves preference index 1, so Tabs and Tree retain indices
0 and 2 and existing configuration remains compatible. GTK3 keeps native radio
buttons, while GTK4 uses grouped check buttons without importing GTK3 group
lists or invalid toggle casts. Strict MSVC and Meson probes and the shipping
GTK3 build remain clean. The complete GTK4 profile retains zero compiler errors
while improving from 183 to 178 warnings, from 69 to 66 repeated diagnostics,
and from 56 to 53 unique unresolved symbols. The retired radio-button family
is now contained wholly inside the GTK3 compatibility branch. Preferences
ordinary check-button state access is the next contained target.

Preferences check-button pass 69 (2026-07-21):
ordinary Preferences toggles, three-column alert choices, and controls that
enable dependent fields now use the shared typed check-button state boundary.
Callback parameters are widget-neutral, and dependency callbacks read state
once before updating the staged preference, child control, and child label.
GTK3 behavior remains unchanged, while GTK4 no longer casts check buttons to
the unrelated toggle-button class. Shipping GTK3, strict MSVC and Meson GTK4
probes, and repository validation remain clean. The complete GTK4 inventory is
unchanged at zero compiler errors, 178 warnings, 53 unique unresolved symbols,
and 66 repeated diagnostics because other frontend paths still own genuine
toggle buttons. The Preferences combo-box wrap-width operation is the next
contained target and the final `setup.obj` diagnostic.

Preferences combo-wrap pass 70 (2026-07-21):
the DCC speed-unit selector now expresses a semantic single-column popup policy
through the shared compatibility boundary. GTK3 continues to request one
column explicitly, while GTK4 relies on its native one-column combo popup and
avoids the removed wrap-width function. Strict MSVC and Meson probes compile
the helper contract, the full GTK4 frontend compiles the live call site, and
shipping GTK3 and repository validation remain clean. The complete GTK4
profile retains zero compiler errors while improving from 178 to 177 warnings,
from 66 to 65 repeated diagnostics, and from 53 to 52 unique unresolved
symbols. `setup.obj` is now clean. Remaining top-level visibility and lifecycle
callbacks are the next planned target.

Deliverables:

- Make GTK4 the only production frontend dependency in MSVC, Meson, and CI.
- Rebuild native dependencies that share GLib/CRT ownership against the final
  runtime where required.
- Switch staging and WiX to an allowlisted GTK4 payload.
- Remove GTK3 DLLs, modules, themes, emoji data, build downloads, properties,
  compatibility helpers, and installer components.
- Validate clean install, in-place upgrade, repair, uninstall, and portable mode.

Exit criteria:

- `fabulor.exe` imports GTK4 and no GTK3 DLL.
- Clean and upgraded installations contain no unintended GTK3 runtime files.
- CI, CodeQL, native tests, installer builds, and the full validation matrix pass.
- Release notes identify the GTK4 cutover and any deliberately retired theme or
  platform behaviour.

## Pull Request Boundaries

Prefer PRs that complete one ownership boundary or one user workflow. Suitable
examples include:

- build-root and compatibility helpers
- utility dialogs and file selection
- menu/action model
- clipboard and shortcut controllers
- channel/user list model
- transcript rendering
- spell-check input
- theme adapter
- tray and notification integration
- packaging cutover

Do not combine transcript rendering, list models, and installer cutover in one
PR. Each PR must keep the shipping build usable and identify its rollback point.

## Open Decisions

- Minimum supported GTK4/GLib versions after evaluating the current 4.22.4 /
  2.88.0 Windows bundle against supported non-Windows distributions.
- GTK4 list widget choices for each large editable model.
- Whether the spell-check entry remains a custom widget or becomes a composed
  input control.
- Which Unix tray backend remains supportable after GTK4 cutover.
- The final allowlisted Windows runtime payload and provenance mechanism.
