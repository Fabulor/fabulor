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

The probe also includes `src/fe-gtk/gtk-compat.h` directly and takes the address
of every compatibility helper. This makes both build systems compile and link
all GTK4 helper branches without creating a display or changing the production
frontend target.
