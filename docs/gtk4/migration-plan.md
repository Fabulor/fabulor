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

Exit criteria:

- Theme switching, restart persistence, tray state, notifications, icons,
  fonts, spell-check UI, and emoji rendering pass on the packaged GTK4 runtime.

### Stage 8: Production Cutover And Runtime Cleanup

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
