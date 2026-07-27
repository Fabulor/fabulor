# GTK4 Runtime And Packaging

Status: GTK4-only production launcher/frontend and allowlisted runtime in shipping WiX; GTK3 rollback package retired

Baseline date: 2026-07-14

## Current Packaging Model

Fabulor has one supported GTK payload. The shipping launcher loads
`fabulor-gtk4-frontend.dll` after registering the executable-relative
`Runtime/GTK4/bin` directory, and WiX packages the deterministic staged
allowlist into `ProgramFiles/Fabulor/Runtime/GTK4` with its source-bound
manifest. A separate 12-file production-support manifest supplies only the
reviewed non-GTK root files required by the application and Python host.

The old GTK3 copy project remains only as inactive cleanup work; it is absent
from the supported solution, CI build, and WiX graph.

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
Windows CI now materializes this allowlist as the production runtime before
compiling and packaging the GTK4 frontend. The following candidate passes are
retained as historical steps that led to the production contract.

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

Stage 8 pass 91 closes the first real frontend-startup boundary. A directly
linked GTK4 executable cannot register the nested runtime before Windows
resolves its import table, and MSVC delay-loading is not available for GLib
because normal GLib macros import data symbols such as `g_ascii_table` and
`g_utf8_skip`. The isolated GTK4 profile now builds a minimal Win32-only
`fabulor.exe` and `fabulor-gtk4-frontend.dll`. The launcher registers the
contained runtime, rejects a missing, directory, or reparse-point frontend
module, and loads it with only DLL-load-directory, System32, and registered user
directories enabled. `validate_frontend_bootstrap.py` enforces the PE boundary.

A workspace-only candidate containing the launcher, frontend, reviewed OpenSSL
DLLs, certificate, and 1,431-file pinned runtime opened a responsive Network
List with a System32-only `PATH`. Process inspection confirmed GTK4, GLib,
GObject, and GIO came from `Runtime/GTK4/bin`; normal close returned zero and no
matching Application event was recorded. The shipping WiX payload does not yet
install the frontend DLL and remains on the GTK3 executable until cutover.

Stage 8 pass 92 composes that boundary as a distinct side-by-side MSI named
`Fabulor GTK4 Frontend Candidate`. It has its own UpgradeCode and installs under
`Program Files\Fabulor GTK4 Candidate` without shortcuts, protocol registration,
plugins, Enchant, Python, Tcl, or .NET. The shipping `Fabulor.msi` and
bootstrapper remain unchanged. Windows CI builds and uploads the candidate
separately, then decompiles it and enforces an exact 1,437-file contract: five
root application/OpenSSL files plus the 1,431 runtime files and generated
manifest. Every extracted byte is hash-checked, and PE validation is repeated
against a reconstructed installed layout so the packaged launcher remains
system-only and the frontend resolves only reviewed runtime, OpenSSL, and
Windows imports.

Stage 8 pass 93 expands only that side-by-side candidate with the native
extension set proven against the final GTK4-era dependency root. The exact
contract now contains six autoload plugins, WinRT notifications, WinSparkle,
Enchant 2.8.19 core, WinSpell provider, and Enchant ordering data. The extension
validator checks required imports, ownership, and fourteen dependency edges and
rejects GTK3 or unresolved imports. MSI extraction now enforces 1,447 exact
files and repeats the extension graph validation against packaged bytes.
Shipping GTK3 WiX composition remains unchanged.

Stage 8 pass 94 adds plugin-host parity through
`plugin-host-payload-contract.json`. `stage_plugin_hosts.py` materializes an
installed-layout root for the managed host, .NET 8.0.29 hostfxr/shared runtime,
Python 3.14 runtime and API, Tcl 8.6 runtime, and `hcpython3.dll`; it rejects
reparse points, unsafe destinations, case-insensitive collisions, absent or
empty trees, and ambiguous glob matches. The generated manifest locks 5,821
files and 199,443,761 bytes. Candidate WiX owns those paths explicitly, and
the extracted-MSI validator now checks all 7,270 package files and hashes plus
ten native modules, one data file, and fifteen owned import edges. The normal
shipping package retains its existing host composition but advances its pinned
.NET servicing runtime from 8.0.28 to 8.0.29. Windows CI stages the Python
runtime from the official 3.14.3 x64 embeddable archive and verifies SHA-256
`AD4961A479DEDBEB7C7D113253F8DB1B1935586B73C27488712BEEC4F2C894E6`;
the full hosted Python installation remains a build dependency only.

Stage 9 Tcl payload minimization replaces the inherited full Tcl distribution
with an explicit embedded-runtime allowlist. Fabulor packages `tcl86t.dll`,
the Tcl 8.6 core scripts, encodings, timezone/message data, and reviewed
standard Tcl modules. Tk, shells, import/stub libraries, build tools, source,
tests/examples, and third-party package collections are excluded. The staged
Tcl payload falls from 5,588 files and 96.34 MiB to 825 files and 4.95 MiB.
Isolated initialization, standard-package, encoding, timezone, and maintained
sample-add-on probes pass against only the staged root.

## Sources And Provenance

Windows CI downloads the pinned GTK4 archive from the `ZoiteChat/gvsbuild`
release `zoitechat-2.18.1`. OpenSSL is resolved from the vcpkg baseline in
`tools/windows-deps/vcpkg-configuration.json`; no GTK3 or MSYS2 staging archive
participates in the supported Windows build. The pipeline must:

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
- WinSparkle, Enchant/WinSpell, OpenSSL, and plugins that share GLib or CRT
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
- keep GTK3 archive import and the unsupported Lua/LGI runtime outside the
  production build and package
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
