# GTK4 Theme Architecture

Status: Stage 9 GTK3 theme service and adapter retired

## Scope

Fabulor keeps three independent theme surfaces during the GTK4 migration:

- GTK4 desktop CSS controls toolkit widgets;
- `.hct` files control IRC event text and palette settings; and
- `colors.conf` persists Fabulor's semantic colour palette.

GTK4 discovery does not treat GTK3 CSS as compatible, does not package a mock
Windows theme, and does not replace `.hct` or `colors.conf`.

`.hct` text import is owned by `common/theme-archive-reader.c`, not by a GTK
theme adapter. On Windows it obtains the absolute system directory through the
Windows API and invokes `tar.exe` there with an argument vector, never PATH
lookup or command-string interpolation. It reads
only `colors.conf` and `pevents.conf` to bounded memory, rejects unsafe or
duplicate matching entries, and does not extract the archive to a filesystem
tree.

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

The provider pass does not itself own preferences. The former GTK3 adapter and
its inert GTK4 compatibility surface have been deleted.

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

`fabulor.conf` persists only `gui_gtk4_theme` and `gui_gtk4_variant` for desktop
CSS selection. Retired `gui_gtk3_theme` and `gui_gtk3_variant` keys are ignored
when an older configuration is loaded and are no longer written.

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
diagnostics for the bound GTK4 Preferences page and application owner.

## Preference Surface

`src/fe-gtk/theme/theme-preferences-gtk4.c` owns the candidate GTK4 desktop
theme and variant controls and can either own a standalone controller for
isolated workflows or borrow the application controller. Its
choice model is rebuilt from controller-owned metadata, while committed theme
identifiers remain stable and source-qualified. A callback receives only
successfully applied theme and variant values for later persistence into
`gui_gtk4_theme` and `gui_gtk4_variant`.

Theme and variant changes are transactional. Invalid CSS leaves the previous
provider, stored values, and visible selection intact and exposes the parser
failure as status. Missing saved themes visibly fall back to system default;
high contrast suppresses custom providers while preserving the selected theme.
Appearance refreshes do not emit persistence callbacks. Destruction disconnects
both controls and unparents the surface before releasing controller state.

The GTK4 Preferences page now borrows the application controller. Selection
callbacks update the setup copy only after provider application succeeds;
Cancel and save failure restore the opening selection through the shared
preference-stage owner. Releasing the page leaves application CSS installed.
Shipping GTK3 preferences and their GTK3 theme service remain unchanged.

## Appearance Monitor

`src/fe-gtk/theme/theme-appearance-monitor-gtk4.c` replaces the GTK3 global
window filter contract with GTK4's display-scoped Win32 message filter. It
observes only `WM_SETTINGCHANGE` and `WM_THEMECHANGED`, always continues normal
GDK processing, and coalesces repeated messages into one main-loop refresh.
The queued callback re-queries `AppsUseLightTheme` with
`SystemUsesLightTheme` fallback and `SPI_GETHIGHCONTRAST`, then refreshes the
application callback only when the effective state changed. The monitor has no
Preferences-window dependency; application shutdown removes its display filter
before the shared controller removes and releases display-scoped providers.

If the Windows theme registry values are unavailable, GTK's system-dark
setting supplies the fallback. Query and provider failures retain the last
committed monitor and controller state and expose a diagnostic. System-driven
refresh never invokes the preference persistence callback. Monitor destruction
cancels pending work, removes the exact display filter, releases its display
reference, and only then releases callback data; its borrowed preference owner
must outlive it.

This monitor is in the GTK4 candidate build. The shipping GTK3 filter remains
unchanged until the production frontend creates this owner during GTK4 startup.

## Format And Payload Contract

`tools/validate_theme_contract.py` keeps the Fabulor palette/event formats
independent from GTK desktop CSS. Active WiX and legacy installer sources may
associate only `.hct`; `.zct` remains permitted solely in stale-install cleanup.
The active preferences importer must retain `.hct`, `colors.conf`, and optional
`pevents.conf`, while the runtime must retain atomic `colors.conf` persistence.

The same validator rejects tracked `.hct`, `.zct`, `colors.conf`, or desktop
theme directories from repository-authored payload roots. WiX component rules
cannot harvest those files or a `share/themes` tree. Required GTK runtime data,
icons, and Fabulor application assets remain valid dependencies and are not
treated as an optional default theme. Positive and negative fixtures run in
repository lint so a future packaging change cannot silently reverse this
policy.

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
- Preference callbacks run only after controller application succeeds.
- Appearance-only refresh never writes persisted theme values.
- Preference teardown disconnects controls before releasing callback state.
- Win32 appearance messages are coalesced and applied only on state changes.
- Appearance-monitor teardown removes its display filter and pending source.
- Failed system queries cannot displace the last committed appearance.
- Active installers associate `.hct` and never `.zct`.
- Repository and WiX payload rules cannot introduce an optional default theme.
- `.hct` and `colors.conf` remain independent Fabulor formats.

## Planned Passes

1. Insert the owned GTK4 preference surface into the production preferences
   window and connect its commit callback to the reserved configuration keys.
2. Create the appearance monitor during production GTK4 startup and validate
   light, dark, and high-contrast rendering without a bundled theme.
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
The preference-surface contract verifies successful commit callbacks, invalid
theme rollback without persistence, system-default selection, high-contrast
refresh without persistence, missing saved-theme status, and unparented
teardown after the surface has been attached to a container.
The appearance-monitor contract verifies initial state, coalesced queueing,
unchanged-state suppression, follow-system dark application, high-contrast
provider removal, query-failure rollback, absence of persistence writes, and
pending-source cancellation during teardown.
