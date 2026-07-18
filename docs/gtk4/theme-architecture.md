# GTK4 Theme Architecture

Status: Stage 7 pass 1 discovery boundary

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

## Invariants

- GTK3-only layouts are never returned by GTK4 discovery.
- The profile directory is `themes`, not `gtk3-themes`.
- Discovery examines only immediate theme directories beneath approved roots.
- A canonical theme root appears at most once.
- Profile and desktop themes retain distinct source identities.
- Missing metadata falls back to the directory name.
- CSS provider lifetime and preference persistence are not discovery concerns.
- `.hct` and `colors.conf` remain independent Fabulor formats.

## Planned Passes

1. Add the GTK4 CSS-provider adapter with explicit base/variant ownership and
   parsing diagnostics.
2. Project discovered GTK4 themes into preferences and persist selection and
   light/dark variant policy.
3. Validate Windows light, dark, and high-contrast behavior without a bundled
   optional default theme.
4. Retire GTK3 discovery, imports, and `%APPDATA%\Fabulor\gtk3-themes` only
   after equivalent GTK4 behavior is proven.

## Executable Contract

The strict GTK4 probe creates isolated desktop, profile, and GTK3-only fixture
trees and verifies exact GTK4 recognition, source identity, localized names,
dark variant metadata, canonical duplicate suppression, deterministic sorting,
profile-folder naming, and cleanup. The owner also compiles in the shipping
common library without introducing a GTK dependency there.
