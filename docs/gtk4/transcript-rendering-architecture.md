# GTK4 Transcript Rendering Architecture

Status: Stage 6 pass 5 rendering, widget-class, input, and selection contracts

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

## Planned Passes

1. Validate background images, markers, search highlights, URL hit testing,
   scrolling, accessibility, high DPI, and scrollback performance.

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
- complete and bounded selection payload copies preserve their requested text.

Manual transcript output and latency checks remain required when the GTK4
widget class is connected to this boundary.
