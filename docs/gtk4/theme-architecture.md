# GTK4 Theme Architecture

Status: Stage 7 pass 5 complete pre-production GTK4 theme ownership stack

## Scope

Fabulor keeps three independent theme surfaces during the GTK4 migration:

- GTK4 desktop CSS controls toolkit widgets;
- `.hct` files control IRC event text and palette settings; and
- `colors.conf` persists Fabulor's semantic colour palette.

GTK4 discovery does not treat GTK3 CSS as compatible, does not package a mock
Windows theme, and does not replace `.hct` or `colors.conf`.

## Discovery Ownership

`src/common/gtk4-theme-discovery.c` is a GTK-independent owner for immutable
theme metadata. It recognizes a theme only when an immediate child directory
contains a regular `gtk-4.0/gtk.css` file. Optional `gtk-dark.css`, localized
`index.theme` names, and preview images are recorded without loading CSS.

Profile themes live under `%APPDATA%\Fabulor\themes` on the shipping Windows
configuration. Desktop roots include the user's data themes, `~/.themes`,
GLib system data directories, and `GTK_DATA_PREFIX/share/themes` when that
prefix is explicitly configured. Canonical paths suppress duplicate scans;
profile identity wins if the same root is reachable from both source groups.

Each result owns its strings and carries a source-qualified stable identifier.
Results sort by localized display name, with profile entries before desktop
entries when names match. Discovery creates the profile root when a valid
configuration directory is supplied, but it does not copy, remove, parse, or
apply a theme.

## Provider Ownership

`src/fe-gtk/theme/theme-gtk4.c` is the GTK4-only CSS-provider adapter. It loads
a candidate base provider and optional dark variant before replacing the active
providers. A missing file or parser error rejects the candidate and leaves the
current theme installed. Parser warnings remain nonfatal but are counted and
exposed with the most recent diagnostic.

The base provider is installed at `GTK_STYLE_PROVIDER_PRIORITY_USER`; an
optional dark provider is installed one priority above it so variant rules can
override the base theme. Follow-system, prefer-light, and prefer-dark policies
are resolved explicitly. The adapter owns its display reference, providers,
active source identifier, variant policy, and diagnostics. Disable and final
teardown remove installed providers from the display before releasing them and
reset all active identity and variant state.

The provider pass does not itself own preferences. The shipping GTK3 adapter
remains unchanged while the GTK4 adapter is exercised through the strict probe.

## Preference Projection

`src/common/gtk4-theme-preferences.c` projects discovery results into an owned,
GTK-independent choice list. The first choice always represents the GTK4
runtime's system default; subsequent choices copy stable identifiers, display
names, source identity, and dark-variant availability. The projection owns its
strings and remains valid after discovery results are released.

Persisted selection is resolved by exact stable identifier. An empty selection
chooses the system default. A stored identifier that is no longer discovered
also resolves safely to the system default while reporting that the stored
selection is unavailable, allowing the production preferences UI to explain or
replace it deliberately. Unknown variant values normalize to follow-system.

`fabulor.conf` reserves `gui_gtk4_theme` and `gui_gtk4_variant` independently
from the existing GTK3 keys. This prevents the migration from treating a GTK3
theme identifier or variant as GTK4-compatible. Production GTK4 widgets and
adapter application remain a cutover task.

## Windows Appearance Policy

The preference owner resolves a `FabulorGtk4ThemeAppearanceDecision` from the
post-discovery selection state, stored variant, Windows light/dark preference,
and high-contrast state. It accepts a resolved boolean for custom-theme
selection rather than a raw stored identifier, so an unavailable persisted
theme cannot accidentally enable custom CSS.

In normal mode, a resolved custom theme follows its explicit light/dark policy
or the Windows application preference. With the system-default choice, no
custom provider is installed and the runtime follows Windows regardless of a
stale custom-theme variant. In high-contrast mode, custom providers and dark
requests are both suppressed so GTK runtime defaults can preserve platform
accessibility. `.hct` and `colors.conf` remain separate and are not overridden
by this widget-theme decision.

The shipping frontend already reads `AppsUseLightTheme` with
`SystemUsesLightTheme` fallback, queries `SPI_GETHIGHCONTRAST`, and queues a
theme refresh for `WM_SETTINGCHANGE` and `WM_THEMECHANGED`. This pass defines
how those existing signals drive GTK4. The provider adapter consumes the
resolved decision directly, disabling active custom providers whenever the
decision selects runtime defaults. Production signal/UI hookup and packaged
visual validation remain pending.

## Lifecycle Controller

`src/fe-gtk/theme/theme-gtk4-controller.c` is the GTK4-only composition owner.
It discovers themes, projects owned preference choices, resolves the persisted
selection and Windows appearance decision, and applies that decision through a
single provider adapter. Production code can refresh from the configured roots;
tests and import workflows can supply an already-discovered set through the same
commit path.

Refresh is transactional across provider and preference state. Candidate
choices and selection are committed only after provider application succeeds.
Invalid CSS therefore leaves both the active providers and the previously
committed selection intact. An unavailable persisted identifier is different:
it is a valid fallback outcome, so the controller commits the system-default
choice, records that the stored selection was unavailable, and removes custom
providers. High contrast similarly retains the selected preference but applies
the runtime-default appearance decision.

The controller owns its choice projection and provider adapter. Discovery
metadata is borrowed only for the duration of refresh, and shutdown removes
display-scoped providers before releasing copied preference state. It exposes
read-only choices, selected state, appearance, active provider identity, and
diagnostics for the future GTK4 preferences page.

## Invariants

- GTK3-only layouts are never returned by GTK4 discovery.
- The profile directory is `themes`, not `gtk3-themes`.
- Discovery examines only immediate theme directories beneath approved roots.
- A canonical theme root appears at most once.
- Profile and desktop themes retain distinct source identities.
- Missing metadata falls back to the directory name.
- CSS providers are replaced transactionally; an invalid candidate cannot
  displace the current theme.
- Provider installation is display-scoped and teardown removes every installed
  provider before releasing its reference.
- CSS parser errors reject a candidate; parser warnings remain observable and
  nonfatal.
- Preference choices own their presentation metadata and never retain borrowed
  discovery pointers.
- Missing persisted selections fall back to the system-default choice without
  being mistaken for a successful match.
- GTK4 selection and variant keys are independent from their GTK3 predecessors.
- Only a resolved available custom selection can enable a custom provider.
- High contrast always suppresses custom GTK4 CSS and dark preference requests.
- The system-default choice follows Windows and never loads Fabulor theme CSS.
- Controller refresh commits preference state only after provider success.
- Discovery metadata can be released immediately after controller refresh.
- Controller destruction removes active providers before releasing its state.
- `.hct` and `colors.conf` remain independent Fabulor formats.

## Planned Passes

1. Bind the lifecycle controller to the production GTK4 preferences UI and
   persisted configuration.
2. Connect the Windows appearance decision to the production GTK4 runtime and
   validate light, dark, and high-contrast rendering without a bundled theme.
3. Retire GTK3 discovery, imports, and `%APPDATA%\Fabulor\gtk3-themes` only
   after equivalent GTK4 behavior is proven.

## Executable Contract

The strict GTK4 probe creates isolated desktop, profile, and GTK3-only fixture
trees and verifies exact GTK4 recognition, source identity, localized names,
dark variant metadata, canonical duplicate suppression, deterministic sorting,
profile-folder naming, and cleanup. It also verifies variant resolution,
base/dark provider ownership, parser-error rejection, transactional rollback,
missing-file handling, and idempotent teardown. Discovery compiles in the
shipping common library without introducing a GTK dependency there; the GTK4
adapter compiles only at the strict GTK4 boundary until production cutover.
The probe also releases discovery metadata before validating projected choices,
then verifies system-default fallback, exact persisted identity, source and dark
metadata, and invalid-variant normalization.
The Windows appearance matrix covers custom follow-system, explicit light,
explicit dark, system-default, invalid variant, and high-contrast decisions.
The controller contract verifies copied-choice lifetime, invalid-CSS rollback,
unavailable-selection fallback, high-contrast teardown, and final cleanup.
