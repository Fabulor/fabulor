# GTK4 Build Probe

This directory defines the first Fabulor GTK4 build contract. It validates and
compiles against GTK4 without changing the production GTK3 frontend.

The authoritative Windows x64 dependency identity is recorded in
`dependency-contract.json`. Both build probes require the same root layout and
run `validate_root.py` before compiling. The validator checks:

- the downloaded archive size and published SHA-256 when `--archive` is used
- exact GTK and GLib header/pkg-config versions
- the `gtk-4-1.dll` PE architecture and 64-bit GLib configuration
- required headers, import libraries, DLLs, and pkg-config metadata
- containment of required paths beneath the selected root
- absence of known GTK3 headers, DLLs, and import libraries

The archive's pkg-config files retain their original build-machine `prefix`.
The probes therefore use explicit include and library paths beneath the
validated root. They do not consult process `PATH`, `PKG_CONFIG_PATH`, or a
machine-wide GTK installation while resolving build inputs.

## Validate A Root

```powershell
python tools\gtk4\validate_root.py --root C:\fabulor-master\Runtime\GTK4
python tools\gtk4\test_validate_root.py
```

## MSVC Probe

Run this from an x64 Visual Studio developer shell:

```powershell
msbuild tools\gtk4\gtk4-probe.vcxproj /p:Configuration=Release /p:Platform=x64 /p:FabulorGtk4Root=C:\fabulor-master\Runtime\GTK4
```

`FabulorGtk4Root` takes precedence. `FABULOR_GTK4_ROOT` is accepted when the
MSBuild property is unset. The repository `Runtime\GTK4` directory is the local
default only for this isolated probe.

## Meson Probe

This is a self-contained Meson project because the repository currently has
only legacy Meson fragments and no top-level Meson project:

```powershell
python -m mesonbuild.mesonmain setup --buildtype=release build\gtk4-probe-meson tools\gtk4 -Dgtk4_root=C:\fabulor-master\Runtime\GTK4
python -m mesonbuild.mesonmain compile -C build\gtk4-probe-meson
python -m mesonbuild.mesonmain test -C build\gtk4-probe-meson --print-errorlogs
```

The MSVC and Meson probes compile, link, and execute the same `probe.c`. The
program does not initialise a display; it verifies that the loaded GTK/GLib
runtime versions match the headers used at compile time.

The probe includes `src/fe-gtk/gtk-compat.h` and the dedicated compatibility
owner headers directly, then takes the address of every helper. The probe build
also compiles source-owned adapters such as `file-chooser-path.c`. This makes
both build systems compile and link all GTK4 helper branches without creating a
display or changing the production frontend target.

## Full MSVC Frontend Profile

`gtk4-full-frontend.proj` applies the same validated GTK4 root to the production
common and frontend projects while keeping all outputs beneath
`build\gtk4-full`. It is an opt-in conversion target: the normal project and
solution build still default to GTK3 and the existing external output root.

Run the common-library checkpoint or attempt the complete frontend from an x64
Visual Studio developer shell:

```powershell
msbuild tools\gtk4\gtk4-full-frontend.proj /t:BuildCommon /m:1
msbuild tools\gtk4\gtk4-full-frontend.proj /t:Build /m:1
```

`BuildCommon` is required by Windows CI. `Build` intentionally compiles the
whole frontend and currently stops at the remaining GTK3-only source
boundaries; it is the authoritative error inventory during cutover and does
not produce a shipping executable until those boundaries are removed.

## Stage 8 Runtime Candidate

`runtime-payload-contract.json` defines the first production runtime candidate
as exact files and owned data trees from the pinned dependency root. It excludes
headers, import libraries, debug symbols, build tools, GIR source, and Python
build helpers. `stage_runtime.py` copies only that selection, normalizes the GDK
pixbuf loader cache so it contains no build-machine path, and writes a SHA-256
manifest of every staged file.

```powershell
python tools\gtk4\test_stage_runtime.py
python tools\gtk4\stage_runtime.py --root C:\fabulor-master\Runtime\GTK4 --validate-only
python tools\gtk4\stage_runtime.py --root C:\fabulor-master\Runtime\GTK4 --output C:\fabulor-master\build\gtk4-runtime-candidate-root\Runtime\GTK4
```

The output directory must be absent or empty. The normal WiX build consumes this
staged root and requires its generated manifest; the broad transitional GTK4
harvest is retired.

The normal WiX package uses `GTK4Allowlist.wxs`. The validator decompiles and
extracts `Fabulor.msi`, then compares every installed path, size, and SHA-256
beneath `Runtime\GTK4` against the generated manifest, including directory
placement and the packaged manifest itself:

```powershell
python tools\gtk4\test_validate_runtime_msi.py
python tools\gtk4\validate_runtime_msi.py --wix C:\path\to\wix.exe --msi C:\path\to\Fabulor.msi --manifest C:\path\to\runtime-manifest.json
```

`win32-gtk4-runtime.c` defines the early Windows loader boundary for the future
production GTK4 target. It derives `Runtime\GTK4\bin` from the executable path,
rejects missing and reparse-point runtime directories, removes the current
directory and ambient `PATH` from DLL resolution, and retains only the
application, System32, and explicitly registered runtime directories.

`runtime-root-probe.vcxproj` builds a Win32-only executable with no GTK or GLib
imports. Place its output beside `Runtime\GTK4`, then run
`test_runtime_root_probe.py --candidate-root <root>`. The probe configures the
runtime boundary, loads GTK4 with the constrained policy, verifies the loaded
absolute module path, and calls `gtk_get_major_version`. The test removes
ambient GTK paths, runs from an unrelated directory containing an invalid
`gtk-4-1.dll`, and verifies that missing or reparse-point runtime roots fail
closed.

`runtime-import-contract.json` names the native ownership roots and explicitly
reviewed Windows/UCRT imports. `validate_runtime_imports.py` inspects every
packaged DLL and executable with MSVC `dumpbin`, then rejects unresolved
non-system imports, GTK3 or duplicate GLib-family names, duplicate packaged
basenames, missing roots, and native files that are not reachable from an owned
root. Unit tests run in repository lint; the full 35-file candidate graph runs
in the Windows build after staging.

```powershell
python tools\gtk4\test_validate_runtime_imports.py
python tools\gtk4\validate_runtime_imports.py --root C:\fabulor-master\build\gtk4-runtime-candidate-root\Runtime\GTK4 --dumpbin dumpbin
```
