# Windows Per-Monitor-V2 DPI Candidate

## Status

Per-Monitor-V2 (PMv2) is enabled for RC11 after installed testing passed at
250% and 300%, including live scale changes and extended use with themes,
backgrounds, menus, pane layouts, and Ban and Ignore Lists. Published RC10
remains System-DPI-aware. The broader scale and mixed-monitor matrix continues
during RC11 feedback; System-DPI awareness remains the manifest-revert fallback.

GTK 4.22.4's Win32 backend stores surface scales as integers in several paths.
That rounds 250% to 2x while 300% becomes 3x and caused the disproportionate
interface and pane behaviour observed during RC10 testing. Fabulor does not
apply an application-level compensation because that would give rendering,
input, drag-and-drop, window geometry, and monitor discovery different scale
models.

## Candidate Source Contract

`tools/gtk4/pmv2-candidate-contract.json` pins:

- GTK 4.22.4 source commit
  `7f99ab1a26408b6499a18f353f081e3c0598ea5c`;
- GLib 2.88.0, matching the production runtime rather than taking an unrelated
  dependency update;
- ZoiteChat/gvsbuild base commit
  `499c9d27fcb061a0fa480488d8bfb71994726b1d` (release tag
  `zoitechat-2.18.1`);
- the SHA-256 values of the Win32 implementation patch and its GTK-level
  conversion/transition regression-test patch;
- `cargo-c` 0.10.24+cargo-0.98.0, the last locked release compatible with the
  pinned Rust 1.95 toolchain; and
- the required 100%, 125%, 150%, 175%, 200%, 225%, 250%, and 300% acceptance
  scales.

The downstream implementation patch is confined to GTK's Win32 backend. It preserves the
exact `DPI / 96.0` surface scale through monitor discovery, DPI changes,
rendering damage, pointer/tablet input, drag-and-drop coordinates, and window
sizing. `gdk_surface_get_scale()` remains fractional; the legacy widget scale
factor remains its ceiling. The patch uses Windows' suggested
`WM_DPICHANGED` rectangle and DPI-aware non-client sizing to avoid cumulative
geometry drift.

The accompanying GTK test covers exact DPI conversion at all eight acceptance
scales, physical/logical coordinate round trips, and repeated forward/reverse
DPI transitions without changing a saved logical width. Fabulor's remaining
native auto-hidden-taskbar workaround intentionally passes the physical desktop
pixel rectangle returned by Win32 straight back to `SetWindowPos`; those values
must not be converted as GTK logical units.

Fabulor listens for fractional `GdkSurface::scale` changes after the main window
is realised. It restores a locked user-list pane and redraws the transcript
even when the legacy integer scale factor has not changed. Existing
`gui_win_*`, `gui_pane_*`, `gui_ulist_nick_width`, and `text_max_indent` values
retain their GTK logical-unit meaning; there is no preference migration.

## Reproducing The Candidate Runtime

Use the pinned ZoiteChat/gvsbuild revision, add both tracked GTK patches after
its existing `0001-remove-direct-composition.patch`, pin the compatible
`cargo-c` release recorded in the contract, and build GTK 4.22.4 in an isolated
x64 Release root. The development command used for this candidate is:

```powershell
gvsbuild build `
  --build-dir build\pmv2-runtime-2.18.1 `
  --patches-root-dir build\gvsbuild-2.18.1-pmv2\gvsbuild\patches `
  --msys-dir C:\msys64 `
  --configuration release `
  --platform x64 `
  gtk4
```

The accepted isolated build produced
`GTK4_Gvsbuild_zoitechat-2.18.1-pmv2.1_x64.zip` (47,028,696 bytes, SHA-256
`00792b5cc34d7a8da5b16c4dc6e453a6429d966dae7a18a0a7797e82238e2d4b`).
`tools/gtk4/pmv2-runtime-dependency-contract.json` records that identity and its
immutable Fabulor-owned prerelease URL. The build retains the same third-party
component versions and licence inventory as the pinned production builder, plus
the Adwaita licence copies shipped in its source archive. The production runtime
contract now deliberately pins this exact archive for RC11.

The allowlisted candidate runtime can be staged without duplicating or
weakening the production file-selection contract:

```powershell
python tools\gtk4\stage_runtime.py `
  --root build\pmv2-runtime-2.18.1\gtk\x64\release `
  --output build\pmv2-runtime-staged\Runtime\GTK4 `
  --source-contract tools\gtk4\pmv2-runtime-dependency-contract.json
```

## Pre-activation Testing

Before changing the executable manifest, test the patched runtime through
GTK's existing process-start opt-in:

```powershell
$env:GDK_WIN32_PER_MONITOR_HIDPI = '1'
& 'C:\path\to\candidate\fabulor.exe'
```

The environment setting must exist before GTK creates its first window. It is
for candidate validation only and will not be a production requirement.

For every listed scale, test startup and restart; normal, snapped, maximised,
full-screen, minimised, and restored states; server/channel switching; locked
and resizable user lists; long nicknames; transcript indentation and marker
line; scrolling and input hit-testing; menus, popovers, dialogs, font browsing,
background images, themes, Ban/Ignore and Server List windows, DCC, tray, and
auto-hidden taskbar behaviour.

Move the window in both directions between differently scaled monitors and
change Windows scaling while Fabulor is running. Repeat transitions and
restarts to confirm saved logical dimensions remain unchanged and no divider
or window edge creeps.

## Production Activation Gate

The opt-in and installed gates passed: a live process reported per-monitor
awareness, rendered correctly at 250% and 300%, and switched in both directions
without a restart or pane drift. Extended installed use also passed with
multiple themes, backgrounds, menus, compact mode, single-line topics, and Ban
and Ignore Lists. RC11 therefore declares `PerMonitorV2` with compatible
fallbacks in `win32/fabulor.exe.manifest`. The embedded manifest is authoritative
before GTK initialisation; the test environment variable is no longer required.

If any rendering, input, window-state, geometry, or pane regression remains,
the manifest activation stays out of RC11 and production continues using
System-DPI awareness while the downstream GTK patch is refined.
