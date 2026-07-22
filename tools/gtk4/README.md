# GTK4 Build Probe

This directory defines the Fabulor GTK4 build and production packaging
contract. GTK4 is the only supported MSVC and CI frontend profile.

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

`gtk4-full-frontend.proj` applies the validated GTK4 root to the production
common, translation, frontend, and launcher projects while keeping all outputs
beneath `build\gtk4-full`. The normal Visual Studio solution uses this same
GTK4-only output and includes the launcher.

Run the common-library checkpoint or attempt the complete frontend from an x64
Visual Studio developer shell:

```powershell
msbuild tools\gtk4\gtk4-full-frontend.proj /t:BuildCommon /m:1
msbuild tools\gtk4\gtk4-full-frontend.proj /t:Build /m:1
```

Windows CI builds the complete profile. `Build` produces a Win32-only
`fabulor.exe` launcher and `fabulor-gtk4-frontend.dll`. The launcher registers
the executable-relative `Runtime\GTK4\bin` directory before loading the
frontend module, so its own PE import table contains no GTK or GLib dependency.
The shared MSVC properties reject attempts to select a GTK3 frontend profile.

## Production Runtime

`runtime-payload-contract.json` defines the production runtime
as exact files and owned data trees from the pinned dependency root. It excludes
headers, import libraries, debug symbols, build tools, GIR source, and Python
build helpers. `stage_runtime.py` copies only that selection, normalizes the GDK
pixbuf loader cache so it contains no build-machine path, and writes a SHA-256
manifest of every staged file.

```powershell
python tools\gtk4\test_stage_runtime.py
python tools\gtk4\stage_runtime.py --root C:\fabulor-master\Runtime\GTK4 --validate-only
python tools\gtk4\stage_runtime.py --root C:\fabulor-master\Runtime\GTK4 --output C:\fabulor-master\build\gtk4-runtime-production-root\Runtime\GTK4
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
python tools\gtk4\validate_runtime_imports.py --root C:\fabulor-master\build\gtk4-runtime-production-root\Runtime\GTK4 --dumpbin dumpbin
python tools\gtk4\test_frontend_runtime_bootstrap.py
python tools\gtk4\validate_frontend_bootstrap.py --launcher C:\fabulor-master\build\gtk4-full\x64\rel\fabulor.exe --frontend C:\fabulor-master\build\gtk4-full\x64\rel\fabulor-gtk4-frontend.dll --runtime-root C:\fabulor-master\build\gtk4-runtime-production-root\Runtime\GTK4 --dumpbin dumpbin
```

The production WiX package combines the launcher, frontend, locked runtime,
validated native plugins, WinRT notifications, rebuilt Enchant/WinSpell
payload, and C#, Python, and Tcl plugin hosts. `production-support-contract.json`
allowlists the certificate, OpenSSL, WinSparkle, CFFI, and Python API support
files that previously came from the broad legacy staging tree.
`plugin-host-payload-contract.json` defines the exact private runtime inputs;
`stage_plugin_hosts.py` emits their installed-layout root and content manifest.
Build the extensions after the full frontend, stage production support, rebuild
Enchant against the final GTK4 root, and stage the plugin hosts before WiX:

```powershell
msbuild tools\gtk4\gtk4-native-extensions.proj /t:Build /m:1 /p:Configuration=Release /p:Platform=x64 /p:FabulorGtk4Root=C:\fabulor-master\Runtime\GTK4 /p:FabulorSupportDepsRoot=C:\fabulor-master\build\vcpkg-installed\x64-windows
python tools\gtk4\test_stage_production_support.py
python tools\gtk4\stage_production_support.py --dependency-root C:\fabulor-master\build\vcpkg-installed\x64-windows --python-build-root C:\gtk-build\python-3.14\x64 --repository-root C:\fabulor-master --winsparkle-root C:\gtk-build\WinSparkle --output C:\fabulor-master\build\gtk4-production-support
python tools\gtk4\validate_native_extensions.py --plugins-root C:\fabulor-master\build\gtk4-full\x64\rel\plugins --payload-root C:\fabulor-master\build\gtk4-production-support --enchant-root C:\fabulor-master\build\gtk4-enchant-stage --runtime-root C:\fabulor-master\build\gtk4-runtime-production-root\Runtime\GTK4 --dumpbin dumpbin
python tools\gtk4\test_stage_plugin_hosts.py
python tools\gtk4\stage_plugin_hosts.py --output C:\fabulor-master\build\gtk4-plugin-host-production-root --payload-root C:\fabulor-master\build\gtk4-production-support --managed-root C:\fabulor-master\src\managed\Fabulor.PluginHost\bin\x64\Release\net8.0 --dotnet-root "C:\Program Files\dotnet" --python-root C:\fabulor-master\Runtime\Python314 --tcl-root C:\fabulor-master\Runtime\Tcl
dotnet build installer\Fabulor.wixproj -c Release -p:Platform=x64
python tools\gtk4\validate_production_gtk4_msi.py --wix C:\path\to\wix.exe --msi C:\fabulor-master\installer\bin\x64\Release\Fabulor.msi
python tools\gtk4\validate_runtime_msi.py --wix C:\path\to\wix.exe --msi C:\fabulor-master\installer\bin\x64\Release\Fabulor.msi --manifest C:\fabulor-master\build\gtk4-runtime-production-root\Runtime\GTK4\runtime-manifest.json
```
