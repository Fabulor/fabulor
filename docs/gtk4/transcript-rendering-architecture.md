# GTK4 Transcript Rendering Architecture

Status: Stage 6 pass 2 render-target and geometry contracts

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

## Ownership Invariants

- An active context may be temporarily replaced and then restored for nested
  rendering.
- Offscreen surfaces take precedence over the active toolkit context.
- GTK4 has no window fallback; rendering outside a snapshot must queue a frame.
- The target must have no active context when it is freed.
- Widget teardown releases the target once and leaves no borrowed target state
  in `GtkXText`.

## Planned Passes

1. Add GTK4 measure, allocation, realize/unrealize, and snapshot class methods.
2. Move pointer, click, scroll, leave, focus, and selection input to controllers.
3. Replace GTK3 selection ownership and clipboard payload callbacks.
4. Validate background images, markers, search highlights, URL hit testing,
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
- zero or negative dimensions are rejected and reset safely.

Manual transcript output and latency checks remain required when the GTK4
widget class is connected to this boundary.
