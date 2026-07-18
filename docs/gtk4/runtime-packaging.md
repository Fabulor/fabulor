# GTK4 Runtime And Packaging

Status: allowlisted GTK4 runtime in the shipping WiX package; GTK3 executable retained

Baseline date: 2026-07-14

## Current Packaging Model

Fabulor currently has two GTK payload concepts:

1. The shipping executable is built against a GTK3 gvsbuild dependency root.
   `win32/copy/copy.vcxproj` stages GTK3 DLLs and supporting data into the
   release root beside `fabulor.exe`.
2. The WiX project packages the deterministic staged allowlist into
   `ProgramFiles/Fabulor/Runtime/GTK4` and requires its source-bound manifest.

This is a transition state, not the final runtime design. The installed GTK4
payload does not by itself make the current executable GTK4-compliant.

## Repository Runtime Baseline

The current `Runtime/GTK4` tree reports:

- GTK 4.22.4 (`gtk-4-1.dll`)
- GLib 2.88.0 (`glib-2.0-0.dll`)
- 6,112 files
- 1,153,962,024 bytes before WiX include filtering

Top-level content:

| Directory | Files | Approx. bytes | Current WiX treatment |
|---|---:|---:|---|
| `bin` | 309 | 898,278,551 | top-level DLLs only |
| `etc` | 24 | 70,922 | recursively harvested |
| `include` | 3,128 | 32,011,432 | not packaged |
| `lib` | 713 | 149,064,116 | selected runtime subtrees |
| `python` | 2 | 366,347 | not packaged |
| `share` | 1,934 | 73,804,309 | selected runtime subtrees |
| `wheels` | 2 | 366,347 | not packaged |

Before Stage 8 pass 5, the transitional broad WiX rules selected approximately
1,721 files and 198,353,537 bytes, including development metadata and an empty
`lib/gio` harvest that emitted WIX8600. Those rules are now retired. The normal
MSI and bootstrapper consume the 1,431-file staged allowlist plus its manifest.

Stage 8 pass 1 adds `tools/gtk4/runtime-payload-contract.json` and
`tools/gtk4/stage_runtime.py`. The contract selects the native dependency
closure rooted at GTK4 and the SVG pixbuf loader, the two GLib spawn helpers,
and explicit runtime data trees. Staging rejects missing, escaping, reparse,
duplicate, or build-only files; normalizes the pixbuf loader cache; and emits a
deterministic file/size/SHA-256 manifest tied to the pinned source archive.
Windows CI materializes this candidate before compiling the GTK4 probe. WiX
continues to use the transitional component group until candidate feature and
payload validation is complete.

Stage 8 pass 2 adds an opt-in `GtkRuntimeCandidate=true` WiX composition. It
selects `Components/GTK4Candidate.wxs`, requires the generated runtime manifest,
and builds an MSI in an isolated output tree with bootstrapper generation
disabled. Windows CI runs this only after the normal MSI/bootstrapper build and
uploads it as `Fabulor GTK4 Runtime Candidate MSI x64`. The candidate package is
named `FabulorGtk4RuntimeCandidate.msi` for payload inspection; its application
executable remains the shipping GTK3 build until the production frontend
cutover.

Candidate component groups own `bin`, `etc/fonts`, `lib/gdk-pixbuf-2.0`, and
each selected `share` subtree explicitly. This avoids WiX harvest-prefix
flattening. After the candidate MSI is built, the current `validate_runtime_msi.py`
decompiles and extracts it, then requires the installed `Runtime/GTK4` path set
to equal all 1,431 generated manifest paths plus `runtime-manifest.json`, with
no missing, unexpected, duplicate, misplaced, size-mismatched, or hash-mismatched
entry.

Stage 8 pass 3 adds `win32-gtk4-runtime.c` as the early loader boundary for the
future production GTK4 executable. It uses only Win32 APIs before GTK/GLib is
available, derives `Runtime/GTK4/bin` from the executable path, rejects missing
or reparse-point path segments, calls `SetDefaultDllDirectories` without the
current directory, and retains one `AddDllDirectory` registration for the
packaged runtime. The operation is idempotent for process startup.

Windows CI now stages a complete candidate-root shape and builds a standalone
bootstrap probe beside `Runtime/GTK4`. The probe has no GTK or GLib imports,
runs from an unrelated directory with ambient GTK paths removed and an invalid
current-directory `gtk-4-1.dll`, then verifies that the real module came from
the packaged absolute path and reports GTK major version 4. Separate negative
cases require missing and junction-backed runtime roots to fail closed.

This establishes runtime discovery but does not yet make direct GTK imports
safe from the nested directory. At production cutover, GTK-family imports must
be delay-loaded behind the bootstrap or moved into a module loaded only after
the bootstrap succeeds. The current GTK3 executable is not changed here.

Stage 8 pass 4 adds `runtime-import-contract.json` and
`validate_runtime_imports.py`. The contract defines four native ownership roots
and the reviewed Windows/UCRT import surface. The validator inspects all staged
PE files, resolves imports case-insensitively to unique packaged basenames or
the explicit system allowlist, rejects GTK3 and lib-prefixed duplicate
GLib-family names, and requires every packaged native file to be reachable from
an ownership root.

The current candidate contains 35 PE files, 107 packaged import edges, and 54
distinct reviewed system imports. The roots are `gtk-4-1.dll`, the SVG pixbuf
loader, and both GLib spawn helpers. This proves static native closure and
package ownership; it does not detect data-driven module loading or make an
unused-file decision. GIO, pixbuf, icon, locale, font, schema, and typelib
trimming remains gated on packaged feature tests and module/process evidence.

Stage 8 pass 5 retires the broad `GTK4.wxs` harvest and candidate-mode switch.
`GTK4Allowlist.wxs` is now the sole `GTK4Components` provider and the normal
WiX build requires `runtime-manifest.json`. Windows CI builds the ordinary
`Fabulor.msi` and `FabulorSetup.exe` against the staged root, then decompiles,
extracts, and hash-validates the GTK4 payload in that same shipping MSI. The
duplicate candidate MSI build and artifact are removed. This cuts shipping WiX
packaging over to the allowlist but deliberately retains the GTK3 executable
and root payload until production frontend linking and feature validation pass.

## Sources And Provenance

Windows CI currently downloads both GTK3 and GTK4 archives from the
`ZoiteChat/gvsbuild` release `zoitechat-2.18.1`. It also augments the GTK3 build
root with MSYS2 hicolor and libarchive packages. The final GTK4 pipeline must:

- pin source URLs and immutable release/package versions
- verify expected SHA-256 values before extraction
- record package names, versions, licences, and source repositories
- produce a machine-readable payload manifest for the exact release tree
- fail CI on unexpected files or hashes
- retain enough provenance to reproduce Enchant and other native integrations

The first GTK4 build contract now pins the Windows x64 archive as:

- file: `GTK4_Gvsbuild_zoitechat-2.18.1_x64.zip`
- size: 299,109,782 bytes
- SHA-256: `3910a612083c2a155c5a4a2026990701841c0d7f7de28756b2f0865decb161be`
- GTK: 4.22.4
- GLib: 2.88.0

`tools/gtk4/validate_root.py` verifies the archive before extraction and the
root after extraction. The root check includes exact header and pkg-config
versions, x64 PE and pointer-size identity, required build files, path
containment, and absence of known GTK3 build markers. The archive's pkg-config
files retain their original build-machine prefix, so the isolated probes use
validated root-relative include and library paths instead of ambient
`PKG_CONFIG_PATH` resolution.

The repository security audit already identifies the lack of a complete runtime
bill of materials and download hash verification. GTK4 cutover must close that
finding rather than carrying it forward.

## Target Installed Layout

The target layout remains:

```text
Program Files/Fabulor/
  fabulor.exe
  plugins/
  Runtime/
    GTK4/
      bin/
      etc/
      lib/
      share/
    Python314/
    Tcl/
    DotNet/
```

Runtime resolution must be executable-relative. Do not rely on process `PATH`,
the current directory, user-installed GTK, or a machine-wide GTK installation.
If GTK DLLs remain outside the executable directory, startup must load them by
trusted absolute path or establish a constrained DLL directory before any GTK
dependent module is loaded.

## Theme Payload Policy

The final package does not include the legacy `MS-Windows` GTK theme, optional
Windows 10 GTK theme downloads, or another optional default Fabulor theme.
Fabulor follows Windows light/dark and high-contrast policy and otherwise uses
the GTK4 runtime defaults when no custom theme is selected.

Supported user-facing formats are `.hct`, `colors.conf`, system GTK4 desktop
themes, and imported GTK4 desktop themes under `%APPDATA%\Fabulor\themes`.
The `.zct` association is retired. Existing `%APPDATA%\Fabulor\gtk3-themes`
content is not deleted during upgrade, but it stops being discovered after the
GTK4 adapter replaces the shipping GTK3 theme service. Required GTK runtime
data and icon assets remain allowlisted runtime dependencies, not optional
default themes.

`tools/validate_theme_contract.py` enforces this boundary in repository lint.
It verifies the active associations and import/persistence tokens, then rejects
repository-authored default-theme files and WiX harvest rules for `.hct`,
`.zct`, `colors.conf`, or `share/themes`. Isolated tests prove each rejection
path while preserving the explicit stale `.zct` upgrade cleanup.

## Required Runtime Categories

The final allowlist should be derived from clean-machine execution and explicit
feature ownership. Review at least:

- GTK4, GDK, GLib, GObject, GIO, Cairo, Pango, HarfBuzz, FreeType, Fontconfig,
  pixbuf, image codecs, libffi, gettext/intl, iconv, PCRE2, XML, and compression
  DLLs actually imported at runtime
- GDK pixbuf loaders and generated loader cache
- GIO modules only if a shipped feature uses them
- GLib schemas and compiled schema cache
- GTK4 settings and data
- fontconfig configuration and Fabulor emoji fallback configuration
- hicolor/application/tray/emoji icons actually required
- translations supported by the application and runtime
- TLS certificate data and spawn helpers where used
- licences and notices required for redistribution
- introspection typelibs only if the retained Lua/LGI path uses them at runtime
- WinSparkle, Enchant/WinSpell, libarchive, and plugins that share GLib or CRT
  ownership with the frontend

Do not package headers, import/static libraries, build scripts, Python wheels,
debug symbols, caches containing build-machine paths, or GIR source metadata
unless a documented runtime feature requires them.

## Native Dependency Compatibility

The final process must not mix incompatible GLib or C runtime ownership across
DLL boundaries. Before cutover:

- rebuild Enchant 2.8.19 and WinSpell against the final MSVC/UCRT GTK4-era GLib
  bundle and rerun the personal-word-list smoke test
- rebuild bundled native plugins against the final import libraries
- verify libarchive ownership and theme extraction tests
- verify Lua/LGI and any introspection modules against retained typelib/runtime
  versions
- inspect `fabulor.exe` and every bundled DLL for GTK3 imports
- reject duplicate GLib/GObject/GIO DLL families in the installed process search
  path

## Packaging Stages

### 1. Freeze The Baseline

- record current archive URLs, versions, hashes, file counts, and sizes
- record the current GTK3 staged/installed files
- keep the current installer functional while source conversion proceeds

### 2. Build A GTK4 Staging Root

- [x] generate a clean runtime candidate from pinned inputs
- [x] separate build-only and runtime files through an explicit contract
- [x] emit a source-bound file manifest with sizes and SHA-256 hashes
- [ ] add ownership/category metadata after native feature verification
- [x] validate executable-relative startup without ambient GTK paths

### 3. Parallel Package Validation

- [x] package and publish the GTK4 runtime candidate without replacing the
  shipping MSI/bootstrapper or removing the GTK3 payload
- run clean-machine smoke tests against the GTK4 executable
- compare Process Monitor/module lists against the allowlist
- identify optional modules through feature tests, not startup alone

### 4. Cutover

- switch CI build/link inputs to GTK4
- switch `win32/copy` and WiX component groups to the GTK4 allowlist
- remove legacy GTK3 compatibility feature/components and duplicate root data
- add upgrade removal coverage for every retired GTK3 path

### 5. Trim And Lock

- remove unused locales, icons, typelibs, schemas, modules, and helper tools only
  after the full validation matrix passes
- fail CI if the generated payload differs from the locked manifest
- publish MSI/bootstrapper and payload hashes with release metadata

## Installer Validation

Each packaging PR must cover as applicable:

- clean x64 install
- in-place upgrade from the last GTK3 release
- same-version developer upgrade where supported
- repair
- uninstall with no orphaned GTK3/GTK4 files
- installed and portable modes
- launch from Start menu, terminal, unrelated current directory, and safe mode
- spell check, emoji/flags, fonts, icons, themes, tray, notifications, file
  chooser, TLS, plugin runtimes, and updater
- MSI ICE validation or a documented environment limitation
- artifact and installed-file hashes

## Cutover Gate

Do not remove the GTK3 payload until all of these are true:

- production `fabulor.exe` imports GTK4 and does not import GTK3
- every bundled native DLL is compatible with the final GLib/CRT family
- clean and upgrade installs pass the validation matrix
- the GTK4 payload manifest is allowlisted, hashed, and provenance-backed
- no required feature depends on an omitted module or data file
- legacy GTK3 files are removed by upgrade and absent from clean install
- the known empty `lib/gio` harvest warning is resolved
