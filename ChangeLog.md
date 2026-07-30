# Fabulor Change Log

Notable changes to Fabulor are recorded here. Fabulor began as a modernised
continuation of XChat, HexChat, and ZoiteChat; inherited history remains
available through the Git repository rather than being presented as Fabulor
release history.

Fabulor does not yet have a published GitHub release. Dates below identify
development milestones and do not imply public release dates.

## Unreleased - 1.0.6 Release Candidate

### Added

- A native GTK4 frontend for Windows 11+ with accessible menus, dialogs,
  transcript navigation, clipboard selection, URL handling, spell checking,
  emoji and flag picking, notifications, and system-tray integration.
- Optional C#, Python 3.14, and Tcl 8.6 plugin hosts with manifest discovery,
  dependency ordering, capability gates, isolated runtime roots, and maintained
  examples.
- GTK4 desktop-theme archive import and `.hct` colour-palette management with
  preview, cancellation, persistence, and System default restoration.
- Per-network and per-channel input history under the user profile, with
  scoped `/CLEAR HISTORY` and `/CLEAR LOG` maintenance commands.
- Supported installed and portable setup modes through a WiX MSI and
  bootstrapper.

### Changed

- Made GTK4 the sole frontend, build, CI, staging, runtime, and installer
  profile.
- Renamed Fabulor-owned Python, Tcl, and managed C# plugin APIs while retaining
  intentional XChat and HexChat compatibility imports.
- Replaced broad runtime harvesting with validated GTK4, Python, Tcl, and .NET
  payload ownership.
- Moved the remaining Windows copy payload into explicit `data` ownership and
  retired the legacy `win32/copy` namespace.
- Removed the unbuilt text frontend, orphaned dirent shim, and unused
  non-Windows notification and sysinfo backends while retaining the active
  Windows compatibility layers.
- Archived completed GTK4 migration and security-review records while keeping
  current architecture, runtime, plugin, and trusted-configuration guidance in
  their active documentation locations.
- Retired the inherited updater and WinSparkle payload until Fabulor has an
  authenticated, product-owned update feed.
- Moved maintained user add-ons to the independent `Fabulor/add-ons`
  repository.
- Updated the supported licence to GNU GPL version 3.0 only.

### Fixed

- Corrected installed-client layout, menu, transcript selection, URL,
  user-list, server-tab, theme, sound, emoji-picker, tray, and window-lifecycle
  behavior found during extended Windows testing.
- Prevented `/LIST` replies with empty or non-prefixed topics from advancing
  beyond the received parameter and crashing in native string processing.
- Made Server > Channel List request and display a fresh channel list.
- Improved busy-channel and multi-network switching performance.
- Bounded Windows address fallback so an unavailable DNS result cannot consume
  the operating system's full connection timeout.
- Corrected duplicate service-message echoes and hardened plugin shutdown and
  callback cleanup.
- Made the installer interface self-contained so setup can start on a clean
  Windows machine without a separately installed .NET Desktop runtime.
- Restored installed ISO language and country data used by spell-check
  language-name lookup.

### Security

- Audited manifest loading while disabled and before enablement.
- Contained plugin roots, entrypoints, runtime roots, archives, theme imports,
  symlinks, and Windows reparse points.
- Added strict manifest schema validation, dependency-cycle detection,
  capability enforcement, callback limits, queued dispatch, and per-plugin
  cleanup.
- Reviewed TLS, proxy, process execution, library loading, updater, URL,
  rendering, and installer boundaries.
- Integrated GitHub CodeQL and Gitleaks into the security-review process.

## Development Milestones

### 1.0.5 - 2026-07-27

- Consolidated repository and runtime cleanup around the supported
  C#/Python/Tcl plugin model.
- Retired residual Perl integration and superseded runtime payloads.

### 1.0.4 - 2026-07-26

- Completed the first installed GTK4 acceptance pass and rebuilt the production
  installer around the accepted frontend.

### 1.0.3 - 2026-07-09

- Corrected installer upgrade, uninstall, and runtime ownership behavior.

### 1.0.2 - 2026-07-07

- Established the initial Fabulor repository, MSVC build, and WiX installer
  baseline.
