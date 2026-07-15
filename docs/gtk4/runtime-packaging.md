# GTK4 Runtime And Packaging

Status: transitional inventory

Baseline date: 2026-07-14

## Current Packaging Model

Fabulor currently has two GTK payload concepts:

1. The shipping executable is built against a GTK3 gvsbuild dependency root.
   `win32/copy/copy.vcxproj` stages GTK3 DLLs and supporting data into the
   release root beside `fabulor.exe`.
2. The WiX project also harvests `Runtime/GTK4` into
   `ProgramFiles/Fabulor/Runtime/GTK4` and requires `bin/gtk-4-1.dll` to exist.

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

The current `installer/Components/GTK4.wxs` include rules select approximately
1,721 files and 198,353,537 bytes. This is still broad and includes development
metadata such as GIR data and large locale/icon sets that require a deliberate
runtime-use decision before release trimming.

The `lib/gio` include currently matches zero files and produces WIX8600. The
warning should be removed by correcting or deleting the include after the final
runtime layout is known; it must not be suppressed globally.

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

- generate a clean runtime tree from pinned inputs
- separate build-only and runtime files
- emit a locked file manifest with hashes and ownership/category metadata
- validate executable-relative startup without ambient GTK paths

### 3. Parallel Package Validation

- package the GTK4 candidate without removing the shipping GTK3 payload
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
