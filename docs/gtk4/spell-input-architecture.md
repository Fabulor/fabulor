# GTK4 Spell Input Architecture

Status: GTK4 input ownership with local v1.0.7 optional auto-correct candidate

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
menu. Suggestion queries do not run inside text-change or cursor-change
handlers. Opt-in auto-correct separately queries the preferred suggestion at
an eligible Space boundary, after native insertion has settled. GTK3 retains its existing
`populate-popup` implementation behind an explicit version guard until final
GTK3 removal. GTK4 formatting actions insert directly into the owning editable,
including replacing the current selection, without using global session state.

## Optional Auto-correct Ownership (Local v1.0.7)

`spell-entry-autocorrect.c` owns opt-in, per-entry typing provenance, deferred
Space correction and immediate Backspace undo. `gui_input_autocorrect` defaults
to zero, persists through the existing preference schema and is applied to open
entries by Preferences. Spell checking must also remain enabled.

The initial curated typo table was too narrow; the maintainer subsequently
approved using Windows' preferred suggestion even when alternatives exist.
Only fully keyboard-typed ASCII-letter tokens (1–64 letters) in lower case or
initial-capital form are eligible. The original must pass the existing
word-check signal's misspelling decision, and every active dictionary must
explicitly return misspelled, not an error. The first active English dictionary
backed by `winspell`, in configured language order, supplies the suggestion.

`enchant_dict_suggest()` delegates to WinSpell's `ISpellChecker::Suggest`;
Enchant preserves provider order while filtering excluded suggestions. Only
the first remaining suggestion is considered, with no Fabulor reranking, typo
map or fallback to a lower-ranked word. The same dictionary must accept it.
Other-language valid words, personal/session words, unavailable English WinSpell
dictionaries and backend errors veto correction. Windows chooses case and
Unicode spelling. Replacements are limited to 64 Unicode characters/256 bytes,
letters/combining marks with internal single spaces, apostrophes or hyphens;
controls, commands, URLs and arbitrary replacement text are rejected.

This is ranked preference, not a confidence guarantee: `btter` may mean better,
batter, bitter or butter. Immediate undo is important. Ordinary dictionary-valid
Australian spellings are not rewritten, and automatic edits do not call
Enchant's replacement-training API.

The frontend's word-check veto resolves the session belonging to this input
box, not an unrelated globally focused session. It excludes the connection's
own nickname, the dialogue peer and nicknames known on that connection. It
also applies to underlines. Unknown nickname-shaped ordinary words cannot be
identified reliably; the opt-in preference and immediate undo remain important.

A captured key only arms an expected one-character insertion; it does not
consume Space or bypass native text/IME handling. The changed signal must see
that exact edit before tracking typing provenance. Paste, history restoration,
emoji insertion and other programmatic edits invalidate provenance. A queued
correction runs after GTK updates the cursor, verifying text, selection,
editability, visibility, length limits and dictionary approval again. It edits
only the word range using Unicode character coordinates, preserving surrounding
text and native editable behaviour. A pending correction is cancelled rather
than flushed by Return/keypad Enter. A subsequent ordinary key can flush the
already-triggered Space correction before processing that key.

The IME delegate's public `preedit-changed` signal invalidates pending edits
and suppresses composition correction; without that guard the feature stays
inactive. Slash/configured-command lines, non-letter token shapes (including
URLs, email, mentions, channels, paths and quoted/punctuated/code tokens), all
caps/mixed case, and lines with IRC controls are excluded. Ordinary spell
checking/suggestions remain unchanged except for known-nickname exclusions.

Undo requires an exact corrected snapshot and cursor match. Immediate unmodified
Backspace restores the original word and removes the triggering space. Typing
provenance is then cleared so another Space does not immediately reapply the
same correction. Cursor movement, clicking, focus loss, edits and tab restoration
invalidate this special undo. `mg_restore_speller()` explicitly clears pending
state even if the new context's draft happens to contain identical text.

All snapshots are temporary per-entry memory; no new word log, persistent
learned-pair store, subprocess or network call is introduced. Disposal cancels
idle work; controller/delegate signals are weakly bound to the entry. Native
tests cover retained controllers and detached/destroyed entries as well as
text/cursor guards. Fourteen policy/widget cases and six source-integration
contracts pass. The initial dictionary probe's claim of a read-only/session-only
test was incorrect: upstream WinSpell's session hook called persistent Windows
`Add`, teaching Windows the test word `teh` and causing the installed correction
to veto it. The exact agent-added word was subsequently removed through Windows'
spell-check API, with hashes confirming all other dictionary entries preserved.

The local follow-up recipe disables WinSpell's optional add/remove session hooks
in a build-local copy of Enchant 2.8.19; Enchant core's own include/exclude tables
already implement that scope. Explicit personal words still persist in Enchant's
personal files, not through these Windows-global hooks. No historical custom
words are bulk-deleted. Build smoke tests require an explicit isolated
`ENCHANT_CONFIG_DIR` (XDG_CONFIG_HOME alone is insufficient on Windows), preflight
the actual provider's null hooks before any word-changing test, and verify
session additions/exclusions against fresh dictionaries. The unsafe earlier
local probes are retired.

An additional integration probe links the actual production spell-entry objects,
uses inert theme services and no IRC session, and delivers real Windows keys
only to its verified foreground window. The initial probe corrected `teh dayz `
to `the dayz ` under the old table. The updated probe corrects
`teh sleke btter turkey ` to `the sleek better Türkiye `, checks real Backspace
undo and no immediate reapplication, and invokes the real Ignore action,
confirming that a fresh entry still rejects
the ignored typo. Windows and Enchant dictionary-file hashes remain unchanged
by the guarded tests. This is stronger than synthetic controller/editable tests.
The maintainer subsequently accepted the replacement installed candidate with
"Perfect 💯" after receiving correction/undo retest instructions. This closes
the reported correction issue, not exhaustive real IME, preference persistence,
context or accessibility acceptance; individual steps were not separately
reported. Later feedback reopened general correction coverage; the maintainer
approved Windows-preferred suggestions and reported on 2026-10-30:
"This seems to working mopre appropriately. 👍" This is focused positive installed
feedback, not separately confirmed results for every example, undo sequence or
platform interaction. Results vary with Windows language dictionaries and user
word state; `Türkiye` is not a hard-coded country substitution.

## Emoji Picker Ownership

The local v1.0.7 candidate uses a shared `GtkEmojiChooser` owned by
`emoji-picker.c`. Both the input-box smiley icon and captured Ctrl+;/Ctrl+.
shortcuts open the same cached chooser for that entry. GTK 4.22's shortcut
action invokes its implementation directly, so capturing the shortcuts before
the editable delegate is necessary; intercepting only `insert-emoji` is not
sufficient. Direct emissions of that signal are also routed to the shared
chooser. The entry is bound once, and teardown releases the chooser even if
it has already been unparented. The picker measures the current monitor in
GTK logical units and caps the existing scroll viewport through
`GtkScrolledWindow`. GTK owns the layout and catalogue.

The first CSS-only candidate still displayed letter-pair flags in installed
testing. The follow-up sets a Fontconfig/FreeType font map on the chooser using
`gtk_widget_set_font_map()`, so it can discover the private bundled Noto font
even when the application's default Pango backend is Win32. Scoped CSS
selects the bundled colour font for emoji labels and a UI font for the
controls. Runtime staging enables the bundled bitmap scaling rule so glyphs
fit their tiles at high DPI. The maintainer's 2026-10-23 screenshot confirms graphical
rendering of the visible country flags, followed by acceptance of the picker's
operation. Exhaustive release-matrix coverage remains separate. Search, category navigation, recent items and
variants remain GTK-owned. Picking an item replaces selected text or inserts
at the cursor, restores entry focus without selecting all text, and preserves
the resulting Unicode cursor position. Read-only input is not modified.

The native regression test uses the pinned GTK catalogue as a test-only
fixture: all 259 country-flag sequences are present in the populated chooser,
and their actual label layouts select Noto Color Emoji and shape one supported
glyph. The test uses the staged Fontconfig configuration, bounds the flag
glyph dimensions and runs with both Win32 and Fontconfig default backends.
It allows frame-clock style updates after idle population. A local comparison found all 253 codes
from the legacy custom picker in that catalogue. This does not promise that
all country codes are searchable aliases in GTK, nor that every bundled PNG
is offered: the artwork directory also contains subdivision assets that the
legacy country-code picker did not expose.

The legacy lazy-page helpers and custom builder remain as a fallback during
local acceptance; normal editable icon/shortcut use selects the native chooser.
Bundled PNG flag assets are unchanged. This candidate uses colour-font glyphs,
not those PNGs, for its graphical country flags. The visible flags have
installed screenshot evidence and the maintainer accepted the picker as working
well. The legacy builder can now be retired in a separate cleanup; exhaustive
release validation remains separate.

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
- Flag filtering reuses loaded buttons and matches country names or codes.
- Picker use cannot release an ISO table still owned by a spell entry.
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
- flag queries match country names and two-letter codes without case
  sensitivity;
- malformed flag codes and invalid Unicode scalar values are rejected; and
- the GTK3/GTK4 popover ownership implementations compile against their
  respective toolkit headers.

Production GTK4 Enchant/input behaviour still requires installed acceptance
for each new input feature; native policy/widget and dictionary probes alone
do not establish the full platform keyboard, IME or accessibility matrix.
