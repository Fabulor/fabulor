# GTK4 Transcript Rendering Architecture

Status: Stage 6 pass 12 rendering, widget-class, input, selection, frame, background, decoration, hit-test, accessibility, display-scale, accessible-text, and performance contracts

## Purpose

`GtkXText` must preserve its IRC formatting and scrollback behavior while its
GTK3 window, drawing, sizing, selection, and event virtual methods are replaced.
The migration keeps the existing Cairo/Pango renderer and changes the toolkit
boundary around it in contained passes.

## Render Target

`src/fe-gtk/xtext-render-target.c` owns the current rendering destination:

- a borrowed active Cairo context supplied by a toolkit paint cycle;
- an optional borrowed offscreen Cairo surface; or
- the contained GTK3 `GdkWindow` fallback while the shipping frontend remains
  on GTK3.

The transcript no longer stores raw draw-window, draw-surface, and draw-context
fields. Rendering helpers request a referenced Cairo context from the owner,
and frame-driven guards ask whether a toolkit context is active. The owner does
not retain or destroy borrowed windows, surfaces, or caller contexts.

GTK4 starts a frame with `gtk_snapshot_append_cairo()`. The returned context is
active until the paired end call clears the target and destroys the snapshot
context. This lets the existing Cairo/Pango renderer become a GTK4 snapshot
producer without duplicating text layout or IRC formatting logic.

## Widget Geometry

`src/fe-gtk/xtext-geometry.c` owns the transcript's toolkit-version geometry
boundary. GTK3 reads the widget allocation; GTK4 reads the widget width and
height. Both paths reject zero or negative dimensions before rendering,
wrapping, selection scrolling, visibility checks, or buffer switching begins.

The transcript no longer obtains operational dimensions from its native
`GdkWindow`. The remaining `gdk_window_get_width()` and
`gdk_window_get_height()` calls are private to the GTK3 window-to-Cairo-surface
capture helper. GTK3 pointer lookup and smooth-scroll surface capture retain
their native window references without making layout depend on them.

## Widget Class Lifecycle

`src/fe-gtk/xtext-widget-class.c` owns the versioned `GtkWidgetClass`
signatures. One static callback table connects the adapter to `GtkXText`
content state while the adapter installs:

- GTK3 preferred-width and preferred-height methods, allocation, realization,
  unrealization, and Cairo `draw`;
- GTK4 `measure`, width/height/baseline allocation, realization,
  unrealization, and `snapshot`.

The fixed 200 by 90 minimum request remains unchanged. Width-change
classification is shared so GTK3 and GTK4 trigger the same line recalculation
or height-only adjustment behavior. GTK4 snapshot dispatch validates widget
geometry, opens the render target's snapshot Cairo context, renders the full
widget area, and closes the context in the same frame.

GTK3 still creates, moves, and releases its native child window inside the
`GtkXText` callbacks. The GTK4 realization path is surface-free, chains to the
parent class, initializes Pango state, and relies on the adapter for snapshot
rendering.

## Input Controllers

Transcript input is connected during widget initialization through the shared
GTK compatibility helpers. GTK3 uses motion, leave, button, scroll, and focus
signals; GTK4 uses `GtkEventControllerMotion`, `GtkGestureClick`,
`GtkEventControllerScroll`, and `GtkEventControllerFocus`.

`src/fe-gtk/xtext-input.c` owns toolkit-neutral selection-press and scroll
direction policy. Modifier-aware motion preserves Shift timestamp selection,
Ctrl colour-copy state, separator dragging, URL hover, tooltip updates, and
selection extension. Single, double, and triple-or-greater left clicks retain
character, word, and line selection semantics. Pointer coordinates and state
are retained for GTK4 selection auto-scroll timers.

The `word_click` signal now carries `FabulorXTextClick` instead of a borrowed
`GdkEventButton`. Its consumer routes popup actions through coordinate-based
menu entry points, preserving button, click count, position, and modifiers.
Cursor changes use GTK4 cursor names or the existing GTK3 cursor objects.

No input or selection callback remains assigned directly on `GtkWidgetClass`.

## Selection And Clipboard Ownership

`src/fe-gtk/xtext-selection.c` owns selection publication and replacement.
`GtkXText` supplies selected text and responds to ownership loss without
exposing toolkit clipboard types to its content logic.

GTK3 registers PRIMARY targets after realization and connects selection-clear
and selection-get signals. It preserves UTF-8, text, compound-text, locale
string, explicit CLIPBOARD, PRIMARY, and SECONDARY behavior. GTK4 stores a
string in one `GdkContentProvider`, publishes it to CLIPBOARD and PRIMARY, and
compares provider identity when PRIMARY changes. An external replacement
clears Unix selection highlighting; Windows preserves its established visible
selection behavior. Teardown disconnects change observation before releasing
the adapter while clipboard-held content remains independently referenced.

## Frame Redraw And Scroll Copy

`src/fe-gtk/xtext-scroll-copy.c` owns partial vertical-copy eligibility and
damage geometry. GTK3 may capture its native window when overlap is non-zero,
smaller than the viewport, and font metrics are valid. It copies retained
pixels and queues only the newly exposed region.

GTK4 has no native capture source, so the same policy rejects the optimization
and the transcript renders the full page into its active snapshot context.
Full rendering no longer checks for a `GdkWindow`. GTK4 damage requests queue a
widget frame, while GTK3 retains region invalidation. Cross-version helpers
also resolve the transcript CSS class and focused root without removed GTK3
APIs.

## Background Composition

`src/fe-gtk/xtext-background.c` owns the optional Cairo source surface and its
frame-local viewport cache. Image surfaces are aspect-fitted and centred over
a black letterbox. Other Cairo surface types repeat from the supplied tile
offset. An absent, invalid, oversized, or failed source falls back to the
current transcript palette colour.

The owner clears its cache at both frame boundaries and whenever the source is
replaced. It retains its own source reference, releases the previous source and
cache exactly once, and limits cache dimensions to 8192 pixels per axis.
`GtkXText` now asks only whether a source exists and requests region painting;
it does not retain cache geometry, tile coordinates, or render-cycle state.

## Markers And Highlights

`src/fe-gtk/xtext-decoration.c` owns decoration state and geometry that does
not belong to IRC text parsing. It calculates the marker line at an entry's
baseline or after the preceding wrapped entry, and classifies stored search
ranges as start, middle, end, and current. Adjacent matches preserve the
existing rule that the next current match takes precedence at their shared
offset.

The same owner retains the transient pointer-hover entry and byte range. It
distinguishes painting from clearing, tracks whether the current text run is
inside the range, and supports temporary suspension while timestamp text is
rendered. `GtkXText` still applies the established underline, selection
palette, marker colour, and Cairo line; it no longer stores or coordinates six
independent hover fields and flags.

## Hit Testing

`src/fe-gtk/xtext-hit-test.c` owns toolkit-neutral coordinate and match-result
policy. It maps pointer y coordinates, including the established negative-y
rounding, to scrollback lines; applies the one-pixel separator tolerance; and
translates stripped URL/nickname/channel match offsets back through IRC
formatting runs with overflow and range validation.

The synchronous `word_click` signal now carries one borrowed-word
`FabulorXTextHit` with captured classification and byte bounds. Its sole
consumer duplicates only a validated match before opening a URL or creating a
URL, nickname, channel, or email menu. It neither reclassifies the scratch word
through global last-match state nor writes a terminator into that buffer.
Plain-word middle menus, dialog nickname menus, selection, timestamp regions,
wrapped lines, and empty clicks retain their existing behavior.

## Accessibility And Display Scale

The widget-class adapter assigns `GtkXText` the toolkit's log role, and widget
initialization supplies the stable localized label `Transcript`. This is the
cross-version semantic baseline for the custom scrollback control; it does not
claim that every rendered line is exposed as accessible text yet.

`src/fe-gtk/xtext-display.c` owns toolkit-neutral display calculations. Pango
ascent, descent, and line-height conversion preserve the existing integer
rounding and post-1.44 line-gap behavior. Strike and underline positions retain
their established logical coordinates. Inline flag images retain their 14 to
64 logical-pixel height bounds and 4:3 layout width, but are loaded at the
widget's device scale and painted back into logical coordinates. Cache keys
include scale so a monitor-scale change cannot reuse a lower-resolution image.

## Accessible Text

GTK4 `GtkXText` implements the native read-only `GtkAccessibleText` interface
through `src/fe-gtk/xtext-accessible.c`. The owner retains valid UTF-8 recent
content, uses character offsets rather than byte offsets, and supplies
character, word, sentence, line, and paragraph slices. Pango log attributes
provide Unicode-aware word and sentence boundaries. The caret is reported at
the end of the log; selection mutation, range geometry, and point-to-offset
mapping return unsupported because the transcript is not an editable control.

The snapshot contains at most 1 MiB and starts at a complete recent entry when
possible. It strips IRC formatting and hidden runs, includes timestamps only
when that buffer displays them, and repairs malformed input before exposure.
Append bursts, scrollback trimming, clears, timestamp changes, and channel
buffer switches mark the snapshot dirty. One idle refresh computes the common
prefix and suffix, then emits only the changed removal and insertion ranges.
Before the interface is first queried, changes only mark the owner dirty and
ordinary sessions perform no snapshot or diff work. The first query refreshes
synchronously; later changes use the idle path. Widget teardown cancels the
idle source before releasing the owner or transcript buffers. GTK3 retains the
pass 10 ATK role and label without taking on a migration-only custom ATK text
subclass.

## Append And Scrollback Performance

`src/fe-gtk/xtext-performance.c` owns the deterministic append-refresh and
scrollback-bound policy. Appends to the visible buffer render immediately at
the bottom so local echo is not delayed. Appends while older content is being
viewed share one idle refresh, and appends to hidden buffers do not schedule a
transcript redraw. An already queued historical-view refresh coalesces later
appends; returning to the bottom cancels that source and renders immediately.

The configured line limit counts wrapped display lines. One appended entry can
add several lines, so trimming now removes as many complete oldest entries as
needed instead of removing only one. The newest entry is always retained even
when it alone wraps beyond the configured limit. Append-owned trimming does not
schedule a separate 40 ms redraw; the append path performs the single required
immediate or idle refresh. If trimming changes visible rows while a refresh is
already pending, that refresh is promoted to a full repaint.

The strict probe gates policy results and operation counts, not elapsed wall
time. It also reports a one-million-decision diagnostic for trend comparison.
This makes CI deterministic while preserving a useful signal for substantial
policy regressions. Production GTK4 transcript latency still requires manual
measurement after the complete widget is connected.

## Ownership Invariants

- An active context may be temporarily replaced and then restored for nested
  rendering.
- Offscreen surfaces take precedence over the active toolkit context.
- GTK4 has no window fallback; rendering outside a snapshot must queue a frame.
- The target must have no active context when it is freed.
- Widget teardown releases the target once and leaves no borrowed target state
  in `GtkXText`.
- Selection publication owns its payload independently of transcript buffers.
- An adapter ignores its own PRIMARY update and reacts only to replacement.
- Selection change observation is disconnected before widget teardown.
- Native-window scroll capture is a GTK3 optimization, never a GTK4 rendering
  prerequisite.
- GTK4 without native capture always reaches full snapshot rendering.
- Background source references and frame caches are owned and released by one
  Cairo-only boundary.
- Background composition failure always produces the palette fallback rather
  than an unpainted region.
- A background cache cannot survive its frame or exceed the viewport safety
  bound.
- Marker placement depends only on entry identity and validated line metrics.
- Search range classification preserves adjacent-match and current-match
  precedence.
- Hover painting, clearing, suspension, and inside-range state belong to one
  owner and are reset at the end of each targeted render.
- Hit classifications and match bounds are captured together before signal
  dispatch and rejected when they exceed the borrowed word.
- Consumers duplicate actionable match text and never mutate the transcript
  scratch buffer.
- IRC formatting offset adjustment is bounded before entry offsets are
  updated.
- Transcript layout, hit testing, and decoration placement use logical pixels.
- Inline flags load at device resolution without changing their logical width.
- Invalid scale factors normalize to one and overflowed image dimensions are
  rejected.
- The transcript exposes a log role and stable label on GTK3 and GTK4.
- GTK4 accessible text is valid UTF-8, read-only, character-indexed, and bounded
  to recent content.
- Accessible updates are coalesced and report only the differing character
  range.
- Unobserved accessible text remains lazy and adds no per-message snapshot work.
- Queued accessibility work is cancelled before transcript buffer teardown.
- Visible bottom appends render immediately; historical-view appends coalesce
  into one idle refresh.
- Scrollback trimming removes complete oldest entries until the wrapped-line
  bound is met, but never discards the sole newest entry.
- Append-owned trimming cannot create an independent delayed redraw.

## Planned Passes

1. Validate transcript role, text reading, and live updates with production
   GTK4 screen readers.
2. Measure complete transcript rendering and input latency in the production
   GTK4 client; the deterministic append policy is already validated.

The spell-check input is a separate Stage 6 boundary and will not share
transcript rendering ownership.

## Executable Contract

The strict GTK4 probe verifies:

- an empty target does not manufacture a context;
- offscreen surface contexts can be created and painted;
- active contexts are referenced, exchanged, restored, and cleared safely;
- a GTK4 snapshot Cairo context becomes the active target; and
- ending the snapshot produces a non-empty `GskRenderNode` and leaves no active
  context;
- positive geometry is preserved; and
- zero or negative dimensions are rejected and reset safely;
- the GTK4 widget subclass receives measure, allocation, realize, unrealize,
  and snapshot class methods;
- horizontal and vertical minimum requests remain 200 and 90; and
- unchanged and changed width decisions remain distinct.
- modifier-aware pointer motion has a strict GTK4-compatible signature;
- non-left, single, double, and repeated left presses classify consistently;
- negative, zero, and positive scroll deltas map to up, neutral, and down; and
- complete and bounded selection payload copies preserve their requested text;
- upward and downward native scroll-copy plans preserve retained pixels and
  identify exposed damage; and
- unavailable or full-height capture falls back to complete rendering.
- an absent background source paints the exact palette fallback;
- a fitted image produces exact centred content and black letterboxing; and
- source replacement and removal update surface-presence state safely.
- marker placement before an entry and after its predecessor is deterministic;
- search ranges classify start, middle, end, current, and adjacent boundaries;
- hover ranges exclude their ending byte and preserve paint/clear modes; and
- timestamp suspension and owner cleanup reset decoration state safely.
- negative and positive pointer coordinates map to deterministic scrollback
  lines;
- separator hit tolerance remains exactly one pixel on each side;
- formatting runs translate stripped match offsets back to entry offsets; and
- invalid ranges are rejected while valid matched substrings are duplicated
  without modifying their source word.
- Pango metric rounding and decoration coordinates preserve their established
  logical positions;
- inline flag sizing preserves logical layout at 1x, 2x, and 3x device scales;
- invalid scale and pixel conversions are normalized or rejected safely; and
- the GTK4 widget subclass exposes the log accessibility role.
- read-only accessible content uses character offsets and deterministic
  character, word, sentence, line, and paragraph ranges;
- content replacement reports minimal insertion/removal ranges;
- oversized snapshots remain within the 1 MiB bound; and
- the GTK4 transcript type implements `GtkAccessibleText`;
- hidden and already-pending historical-view appends schedule no additional
  redraw;
- visible bottom appends select immediate rendering while historical-view
  appends select one idle refresh;
- wrapped-line trimming stops at the configured bound where complete-entry
  retention permits and always preserves the newest entry; and
- one million refresh decisions execute as a reported, non-gating diagnostic.

Manual transcript output and latency checks remain required when the GTK4
widget class is connected to this boundary.
