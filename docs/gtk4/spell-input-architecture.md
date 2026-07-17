# GTK4 Spell Input Architecture

Status: Stage 6 spell-input pass 1 word-boundary contract

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

## Invariants

- Pango and Enchant ranges are UTF-8 byte indexed.
- GTK editable operations are Unicode character indexed.
- One word range carries both coordinate systems from the same segmentation.
- Cursor lookup preserves the established inclusive word-end popup behavior.
- Language and preference refreshes replace one owner instead of manually
  freeing three parallel arrays.
- The owner has no GTK widget, event, menu, or clipboard dependency.

## Planned Passes

1. Convert subclass lifecycle, editable delegation, pointer marking, redraw,
   style updates, and focus behavior to explicit GTK3/GTK4 adapters.
2. Move IRC formatting and misspelling Pango attributes behind a tested text
   styling boundary.
3. Replace legacy `populate-popup` menu mutation with GTK4 actions and dynamic
   suggestion, dictionary, ignore, and colour-menu models.
4. Validate emoji insertion, clipboard, shortcuts, URL paste, Enchant latency,
   personal-dictionary persistence, accessibility, and high-DPI behavior in
   the production GTK4 client.

The custom-widget versus composed-input decision remains open until the class
and menu adapters show which option preserves behavior with less ownership
complexity.

## Executable Contract

The strict GTK4 probe verifies:

- ASCII and multibyte words are segmented in stable order;
- the multibyte word `café` has distinct correct byte and character ranges;
- cursor lookup within and at the end of that word returns the same range;
- duplicated word text remains valid UTF-8; and
- empty owners reject invalid range, lookup, and duplication requests.

Production Enchant and menu behavior remains a manual GTK3 regression check
until the complete GTK4 input widget is connected.
