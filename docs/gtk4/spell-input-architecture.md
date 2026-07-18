# GTK4 Spell Input Architecture

Status: Stage 6 spell-input pass 6 emoji-picker ownership and lifecycle boundary

## Purpose

Fabulor's main edit box must retain Enchant 2.8.19 checking, IRC formatting,
suggestions, personal dictionaries, URL exclusions, emoji insertion, and
shortcut behavior while its GTK3 `GtkEntry` subclass is adapted for GTK4.
The conversion keeps the working spell backend and separates text policy from
toolkit rendering and menu ownership in contained passes.

## Word Ownership

`src/fe-gtk/spell-entry-words.c` owns the current valid UTF-8 text snapshot and
its Pango word ranges. Each range records both byte and character coordinates:

- byte offsets address Pango attributes, substring spell checks, and Enchant;
- character offsets address `GtkEditable` selection, deletion, insertion, and
  cursor positions.

The previous widget stored only byte offsets and reused them as character
offsets. That worked for ASCII but could select or replace the wrong text after
a multibyte character. The spell entry now refreshes one range owner whenever
its text or active-language state changes. Add-to-dictionary, session ignore,
replacement, popup lookup, and underline checks request the coordinate system
required by their API.

Malformed input is repaired before segmentation, empty text produces an empty
owner, and invalid indexes or cursor positions fail without exposing internal
storage. The owner duplicates requested words and releases its text and ranges
together. Enchant broker, dictionary, suggestion, and persistence ownership
remain unchanged.

The same owner classifies the whitespace-delimited token surrounding a word.
Absolute URI schemes are recognized through GLib and `www.` links are handled
explicitly. Every word range inside such a token is excluded from live Enchant
checks and context-menu suggestion generation, while selection, insertion,
copy, paste, and cursor handling remain native `GtkEditable` behavior.

## Widget Lifecycle

`SexySpellEntry` remains a `GtkEntry` subclass. GTK3 and GTK4 both make
`GtkEntry` a `GtkEditable`, so the subclass now inherits that implementation
instead of registering an empty duplicate interface. This also preserves the
toolkit's native focus, input-method, selection, and cursor behavior without a
composed editable delegate owned by Fabulor.

Private state is initialized before dictionary activation: the Pango attribute
list, empty word owner, checked state, and IRC-formatting state all exist before
Enchant can request a recheck. Dictionary descriptions also default to a null
language when a provider does not invoke its callback.

The removed GTK3 class virtuals are replaced by explicit boundaries:

- pointer presses use the shared normalized multi-click callback;
- `spell-entry-widget.c` maps GTK3 pointer coordinates through the public Pango
  layout and, on GTK4, reads the cursor updated by the internal text delegate;
- spell and formatting refreshes call `gtk_widget_queue_draw()` instead of
  invalidating a native `GdkWindow` rectangle;
- GTK3 desktop style notifications remain signal-based, while Fabulor theme
  changes update the caret and spell underline palette through an owned theme
  listener; and
- disposal unregisters the theme listener before releasing private state.

GTK4 removed the public `GtkEntry` layout-position API. Therefore this pass
does not manufacture click coordinates from private widget details. The GTK4
dynamic-menu pass will own context-menu position and action state explicitly;
keyboard menus continue to use the editable cursor.

## Styling Ownership

`spell-entry-style.c` owns the complete Pango attribute list for one edit-box
text snapshot. The widget resolves semantic theme colours into a small palette;
the owner then applies hidden IRC controls, bold, italic, strikethrough,
underline, reset, reverse, mIRC foreground/background colours, and spell-error
underlines without accessing a GTK widget or the global theme manager.

Formatting and spell ranges remain UTF-8 byte indexed because Pango attributes
use bytes. The style owner returns one referenced `PangoAttrList`, and the
widget swaps that owner before adding misspelling ranges. The previous
per-attribute helpers, parser state, ineffective attribute-copy cleanup, and
list-emptiness scan have left `SexySpellEntry`.

Colour parameters are hidden whether another character follows the sequence or
the sequence ends the input. The previous end-of-input path applied the colour
but omitted the parameter-shaping range.

Reverse formatting now explicitly swaps the resolved default foreground and
background. Previously it passed semantic theme-token enum values through the
mIRC colour resolver, making the result depend on unrelated enum numbering.

## Menu Ownership

`spell-entry-menu.c` owns the immutable `GMenuModel` projection for spelling
and input formatting. The model contains replacement suggestions, per-language
dictionary choices, session ignore, the spell-check toggle, all five IRC
attribute controls, and colour codes 0-15. Suggestion lists longer than ten
items retain the established nested `More...` grouping.

GTK4 installs one `GSimpleActionGroup` on each spell entry. Replacement and
dictionary targets carry a language identifier rather than an Enchant pointer;
the callback resolves that identifier against the currently active dictionary
table at activation time. Replacing a menu model therefore cannot leave an
action holding a stale dictionary or native menu item.

Models rebuild only immediately before a secondary-click or keyboard context
menu. Enchant suggestion generation is not part of text-change or cursor-change
processing, preserving the edit-box latency boundary. GTK3 retains its existing
`populate-popup` implementation behind an explicit version guard until final
GTK3 removal. GTK4 formatting actions insert directly into the owning editable,
including replacing the current selection, without using global session state.

## Emoji Picker Ownership

`emoji-picker.c` owns lazy-page state, Unicode sequence construction, and the
popover reference attached to each edit box. A page can claim its contents
exactly once, so stack visibility notifications cannot duplicate hundreds of
buttons or their callbacks. Category item arrays remain immutable borrowed
data and are released independently from the page owner.

The edit box owns one popover through object data with an explicit destructor.
GTK4 parents the popover to the entry and unparents it before dropping the
owned reference; GTK3 retains its relative-widget constructor and destroy
path. Reopening an existing picker reuses that owner instead of connecting an
additional entry-destroy handler.

`GtkStack` and `GtkStackSwitcher` replace the removed notebook path on both
toolkits. Only the visible category is populated, preserving startup and
first-input latency. GTK4 uses paintables for flag images and shared child and
reveal adapters; GTK3 retains pixbuf images and its native emoji signal until
cutover. Flag codes must contain exactly two ASCII letters and Unicode scalar
values are validated before insertion text is allocated.

## Invariants

- Pango and Enchant ranges are UTF-8 byte indexed.
- GTK editable operations are Unicode character indexed.
- One word range carries both coordinate systems from the same segmentation.
- URI-shaped tokens never reach Enchant checking or suggestion generation.
- Cursor lookup preserves the established inclusive word-end popup behavior.
- Language and preference refreshes replace one owner instead of manually
  freeing three parallel arrays.
- The owner has no GTK widget, event, menu, or clipboard dependency.
- The custom entry inherits one toolkit-owned `GtkEditable` implementation.
- No spell-entry draw, button-press, or style-update class virtual remains.
- Widget redraw does not depend on a native window.
- Theme-listener lifetime cannot outlive the spell entry.
- One owner constructs and releases each Pango attribute list.
- Styling accepts resolved colours and has no GTK widget or theme dependency.
- IRC controls and formatting ranges are byte indexed with the text snapshot.
- Reverse formatting swaps semantic defaults rather than mIRC palette indexes.
- One entry owns one GTK4 action group and its current extra-menu model.
- Menu actions never retain Enchant dictionary or GTK menu-item pointers.
- Suggestions are generated on context-menu demand, not while typing.
- Pointer and Shift+F10/Menu-key paths refresh the same dynamic model.
- Spell, formatting, and colour commands remain keyboard accessible.
- Enchant activation cannot recheck uninitialized word or Pango owners.
- One edit box owns at most one emoji popover and its teardown path.
- Lazy emoji pages populate at most once and never own static category arrays.
- Flag insertion accepts exactly two ASCII letters and normalizes their case.
- Invalid or zero Unicode scalar values cannot become insertion text.
- The picker uses stack visibility rather than removed notebook page signals.

## Planned Passes

1. Validate emoji insertion, clipboard, shortcuts, URL paste, Enchant latency,
   personal-dictionary persistence, accessibility, flag assets, and high-DPI
   behavior in the production GTK4 client.

The custom-widget versus composed-input decision is closed for the current
port: retaining the subclass preserves native behavior with less ownership
than duplicating `GtkEntry` around a delegated child.

## Executable Contract

The strict GTK4 probe verifies:

- ASCII and multibyte words are segmented in stable order;
- the multibyte word `café` has distinct correct byte and character ranges;
- cursor lookup within and at the end of that word returns the same range;
- duplicated word text remains valid UTF-8;
- empty owners reject invalid range, lookup, and duplication requests;
- ordinary words remain checkable beside wrapped HTTPS and `www.` links while
  every word inside those URI tokens is excluded from Enchant work;
- a strict GTK4 `GtkEntry` subclass inherits `GtkEditable`; and
- pointer/redraw adapter source compiles against the allowlisted GTK4 headers;
- disabled formatting produces an empty attribute list;
- hidden controls and bold, italic, strikethrough, and underline ranges match;
- mIRC foreground/background colours resolve through the supplied palette;
- trailing mIRC colour parameters remain hidden;
- reverse formatting swaps semantic default colours; and
- misspelling ranges carry the semantic spell underline and colour;
- checked and disabled spell states produce deterministic root models;
- one- and multi-dictionary menus expose stable add, ignore, and replacement
  actions without native pointers;
- long suggestion lists retain the ten-item `More...` boundary; and
- all formatting and colour insertion actions remain present;
- lazy emoji-page state can be claimed exactly once;
- category arrays remain borrowed and flags pages are identified explicitly;
- upper- and lower-case two-letter flags produce the same regional indicators;
- malformed flag codes and invalid Unicode scalar values are rejected; and
- the GTK3/GTK4 popover ownership implementations compile against their
  respective toolkit headers.

Production Enchant behavior remains a manual GTK3 regression check until the
complete GTK4 input widget is connected.
