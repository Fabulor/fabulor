# Fabulor Change Log

Notable changes to Fabulor are recorded here. Fabulor began as a modernised
continuation of XChat, HexChat, and ZoiteChat; inherited history remains
available through the Git repository rather than being presented as Fabulor
release history.

Unreleased entries describe changes under development or release review.
The first stable release and its preceding release-candidate series are
recorded separately below.

## 1.0.9 — 2026-11-25

### Fixed

- Keep channels' activity colours independent when collapsing or expanding
  a network in the channel tree ([#333](https://github.com/Fabulor/fabulor/issues/333)).
- Remove the stale **Fabulor Setup 1.0.8** entry from Windows Installed apps
  when upgrading from the published 1.0.8 installer.

## 1.0.8 — 2026-11-23

### Added

- Added **View > Activity List** to review unread server tabs, channels and
  private conversations across networks. The pop-out shows a count breakdown
  and the latest unread highlight's sender and text. **Highlights only**
  filters the list, and **Mark as Read** clears a context without opening it.
- Added unread badges to the tree and tab layouts. A collapsed network
  shows unread activity from its children and updates its colour when
  expanded or collapsed. The selected context can accrue
  unread activity while its window is out of focus, minimised or in the tray.
- Added saved per-context activity policies through the switcher context menu
  and `/ACTIVITY`. Policies follow the channel when a tab is reused.
  **Activity Rules** in Preferences supports exact and
  wildcard context rules and can apply them to open contexts on request.
  Noisy-nick rules can lower or mute unread activity while preserving
  highlights and leaving messages visible in the transcript.
- Added independent channel-name and nickname display limits under
  **Channel switcher** preferences. Shortened names reveal their full text
  on hover, and screen readers and nickname actions use the full names.
  An existing profile's tab-label width seeds the channel limit when no
  separate limit has been saved.

### Fixed

- Corrected command lookup ordering for `/TYPING` in the built-in command
  table.
- Updated Windows Setup to remove the three known Fabulor Python API cache
  files when the Python feature is uninstalled. Setup also removes the
  corresponding directories when empty.
- Removed the stale **Fabulor Setup 1.0.7** entry from Windows Installed apps
  when upgrading from the published 1.0.7 installer.

See the [Activity List guide](docs/user/main-window.md#activity-list) for
usage and the [1.0.8 roadmap](To-Do.md#108-release) for portable acceptance
and installer validation.

## 1.0.7 — 2026-11-16

### Added

- Added an optional native GTK4 panel API for trusted Python add-ons, with
  feature discovery, owner-bound callbacks and automatic cleanup on unload.
  Panels can present pages, fields, status indicators and buttons without
  changing the standard Preferences window. The client does not bundle Aegis
  or an OpenAI SDK; external moderation is configured separately by the add-on.
- Added optional **Auto-correct while typing** under Input box preferences,
  disabled by default. Fully typed misspellings use Windows' preferred spelling
  suggestion on Space, via Enchant's WinSpell provider, only when all active
  dictionaries reject the original and the selected English Windows dictionary
  accepts the replacement. Windows' ordering, case and Unicode spelling are
  preserved; no fixed typo table or Fabulor ranking model is used. Immediate Backspace restores the word
  and removes the space without instantly reapplying the correction. Known
  nicknames, commands, links, pasted/history text, formatted/code-shaped
  tokens and active input-method composition are excluded. No persistent
  automatic training or send-time Enter correction. The maintainer accepted
  the earlier `teh` session-isolation fix, then reported that the fixed table
  was too limited and approved using Windows' preferred suggestion even when
  alternatives exist. On 2026-10-30 the maintainer reported improved behaviour
  with the Windows-suggestion candidate; individual example/undo checks and
  broader keyboard/IME, accessibility and preference coverage remain separate.
- Implemented Windows `/SYSINFO SOUND`, `ETHERNET` and `CHIPSET` queries.
  Ethernet reports physical adapter names only; chipset reports matching
  system-controller names rather than inferring a marketing chipset model.
  Added `RAM` as an alias for `MEMORY` without changing summary/announcement
  preferences. The maintainer accepted the installed candidate on 2026-10-29.
- Added a DALnet-only **DCC Permissions** nickname-menu submenu for allowing
  files from a user, revoking that permission and listing permissions. Network
  recognition includes the server-advertised identity for renamed/bouncer
  connections. Permission commands remain bound to the original live connection,
  and a stale menu cannot grant permissions after reconnecting. The commands
  reject unsafe nickname tokens and do not alter file auto-accept settings or
  existing custom menus. The maintainer accepted the installed DALnet feature
  as working as expected on 2026-10-23.
- Added a **Testers** section to the About dialog credits, acknowledging
  PickleRick83 and MrBenC.
- Unified the input-box smiley icon and Ctrl+;/Ctrl+. shortcuts on one cached
  GTK emoji chooser per entry. A chooser-scoped Fontconfig/FreeType font map
  and colour-font styling select the bundled Noto font, retaining GTK's layout
  and catalogue. The first CSS-only candidate still showed letter-pair flags
  in installed testing; the follow-up explicitly selects the private-font-capable
  backend and registers the bundled font with the chooser's own font map without
  changing transcript or input rendering. The chooser releases its cache when
  the input widget is torn down.
  Native tests cover all 259 country-flag sequences and shared entry points;
  local comparison confirms coverage of all 253 legacy picker country codes.
  A maintainer-supplied screenshot on 2026-10-23 confirms graphical rendering
  of the visible country flags, followed by maintainer acceptance that the
  picker works well. The reported issue is closed; exhaustive release-matrix
  coverage remains separate.

### Fixed

- Restored normal sizing of the native GTK emoji picker on high-DPI Windows
  displays. The packaged runtime now activates Fontconfig's bundled
  bitmap-font scaling rule, and the chooser limits its scrolling area to suit
  the display. The rebuilt installer passed the maintainer's upgrade and
  picker retest at 250% scaling.

- Ignore echoed outgoing non-ACTION CTCP requests addressed to other users
  before flood/DCC parsing or automatic replies. Explicit self-queries and
  ACTION echoes remain supported. VERSION/system strings omit CPU brackets
  when CPU information is unavailable or empty instead of ending at an unmatched
  opening bracket. Sixteen native cases and six source contracts pass. On
  2026-11-08 the maintainer supplied successful direct-EYErc VERSION results in
  both directions, with a complete Fabulor OS/CPU reply and no extra self-VERSION
  lines in the supplied exchange. Broader interaction coverage remains separate.
- Corrected outgoing DCC socket readiness handling. Active and passive senders
  now share one read/write watch instead of competing Windows socket event
  registrations; ACK reads remain active while throttled and after the last
  file byte is sent. Split four-byte acknowledgements append correctly, and
  reads stop at the announced file size. Localhost sender regressions include
  the complete reported ZIP, partial I/O, slow send, resume, throttle recovery,
  cancellation and peer disconnect. The maintainer reported a successful DCC
  retry on 2026-11-08 following separate address/bind configuration guidance;
  the successful direction/file/mode were not separately specified. Broader
  external acceptance remains open. The code fix does not alter NAT/address
  discovery or port-forwarding settings.
- Aligned message hit-testing with rendering after a clipped sender prefix.
  In Services-style dialogue tabs, starting a selection at the visible message
  end could land earlier in the source text, requiring extra rightward movement
  to copy the suffix. First-line body hits now start from the actual message
  column and skip the hidden prefix; clipping, full sender identity, wrapped
  lines and ordinary channel/server paths remain intact. Native widget and
  source regressions pass. On 2026-11-07 the maintainer reported that the fix
  appears to have resolved the installed issue; broader interaction coverage
  remains separate.
- Corrected the bundled WinSpell provider's session hooks, which could persist
  session-only ignored words in Windows' shared custom dictionary and thereby
  suppress auto-correction. Enchant now owns session state without those
  Windows-global hooks. Build smoke tests preflight the provider and isolate
  personal-word tests explicitly; real-key/full-spell-entry probes cover the
  reported `teh dayz` case. The maintainer accepted the replacement installed
  candidate with "Perfect 💯"; individual retest steps were not separately
  reported.
- Count completely full disks in `/SYSINFO STORAGE`; reject invalid, missing
  and overflowing capacity data instead of treating them as valid zeroes.
  Hardened WMI enumeration cleanup, limited rows/output and per-enumeration
  waits, and stripped controls from provider strings before IRC formatting.
  Unavailable queries now identify the requested information category.
- Excluded timestamp-only paints from nickname-prefix clipping. Timestamp
  copying temporarily reuses a message entry with its original prefix length;
  treating that short timestamp as a nickname could measure beyond the stamp
  and draw it in the prefix column. This addresses the Services-tab overlap
  reported on 2026-10-29. The maintainer subsequently reported that the
  correction appears to have worked; exhaustive interaction coverage remains
  separate.
- Restored the private v1.0.7 width-aware nickname and channel-label clipping
  omitted by the subsequent main-checkout rebuild. Long transcript prefixes
  use an ellipsis without entering the timestamp column; clipped labels show
  full-name tooltips while retaining complete identities. This restores
  available-width clipping, not a fixed 16-character limit. The maintainer
  confirmed the reported regression was fixed in the combined local build on
  2026-10-23.
- Retained the private tooltip-positioning, automatic-indent cap and stable
  background-colour corrections alongside the cache and divider fixes.
- Reused the scaled transcript background image between paints instead of
  rebuilding it every frame. Image, viewport, positioning, dimming and
  background-colour changes still invalidate the cache.
- Corrected the user-list divider measurement during tab switching so pending
  pane positions cannot be combined with stale child allocations and briefly
  squeeze the transcript to an incorrect width.

The maintainer accepted the reported tab-switching and background-image
redraw issue as fixed after local installed testing on 2026-10-22. See the
[focused validation record](docs/gtk4/local-1.0.7-validation.md) for evidence
and scope; this does not establish general v1.0.7 release readiness.

## 1.0.6 — 2026-10-04

- Promotes the accepted RC12 client and installer payload to the first stable
  release for Windows 11+ x64, without additional client changes.
- Includes Per-Monitor-V2 DPI awareness, GTK4 themes and background images,
  persistent pane and list layouts, corrected Ban List filters, and the pinned
  Python runtime upgrade correction described below.
- Passed external multi-monitor and detached-window testing, plus local WinGet
  in-place upgrade and clean-install acceptance with working plugin hosts.
- Adds reviewed-manifest preparation and manually gated WinGet submission.
  WinGet availability follows Microsoft's acceptance of the first manifest;
  Microsoft Store packaging remains a separate future task.
- Remains unsigned. Installer SHA-256 verification confirms file integrity,
  not publisher identity; Windows SmartScreen may still warn. The maintainer
  has chosen not to pursue SignPath Foundation integration.

Known follow-up: unusually long transcript nicknames can overlap timestamps;
  optional nickname and channel-name display limits are planned. Generated
  Python bytecode-cache remnants may remain after uninstall. Neither changes
  IRC identities or removes the user's retained profile.

## 1.0.6 Release Candidate Series

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
- Installer support for publishing both `irc://` and `ircs://` through Windows
  Default Apps without deliberately replacing the user's existing choices.
  A clean installation of `v1.0.6-rc.6` confirmed both schemes and Windows
  Default Apps discovery.
- A narrow `fabulor::get_windows_uptime_seconds` command for trusted simple Tcl
  add-ons. It reads the Windows boot timer directly and is unavailable to
  manifest plugins.

### Changed

- Enabled Per-Monitor-V2 DPI awareness for RC11. A pinned downstream GTK 4.22.4
  Win32 patch preserves fractional surface scales, and Fabulor reacts to
  fractional scale changes when restoring panes and redrawing transcripts.
  Installed testing passed at 250% and 300%, including live scale changes,
  themes, backgrounds, menus, and persistent list and pane layouts.
- Made GTK4 the sole frontend, build, CI, staging, runtime, and installer
  profile.
- Renamed Fabulor-owned Python, Tcl, and managed C# plugin APIs while retaining
  intentional XChat and HexChat compatibility imports.
- Renamed the active application core, private native symbols, generated
  marshallers, GTK identifiers, build macros, and dormant D-Bus identifiers
  for Fabulor while preserving explicit saved-state and compatibility
  boundaries.
- Replaced broad runtime harvesting with validated GTK4, Python, Tcl, and .NET
  payload ownership.
- Reduced the allowlisted GTK4 runtime by 271 unused files and 16.21% while
  retaining the native dependency closure, supported translations, Adwaita,
  schemas, fonts, and spawn helpers.
- Moved the remaining Windows copy payload into explicit `data` ownership and
  retired the legacy `win32/copy` namespace.
- Removed the unbuilt text frontend, orphaned dirent shim, and unused
  non-Windows notification and sysinfo backends while retaining the active
  Windows compatibility layers.
- Archived completed GTK4 migration and security-review records while keeping
  current architecture, runtime, plugin, and trusted-configuration guidance in
  their active documentation locations.
- Added a preview-first developer cleanup command for ignored build output,
  with explicit protection for runtime payloads, local settings, installed
  files, profile data, and external staging trees.
- Retired the inherited updater and WinSparkle payload until Fabulor has an
  authenticated, product-owned update feed.
- Moved maintained user add-ons to the independent `Fabulor/add-ons`
  repository.
- Updated the supported licence to GNU GPL version 3.0 only.
- Consolidated channel mode controls into a permanent topic-bar **Modes**
  popover and retired the two obsolete mode-placement settings.
- Refined the GTK4 topic bar, input row, channel tree, user-list header and
  footer, network and channel lists, Preferences categories, and network
  meters for a denser and more responsive layout.
- Added Fill, Fit, and Centre background-image presentation with independent
  dimming control.

### Fixed

- Restored the GTK4 Ban List filters. Bans now starts selected, supported
  Exempts and Invites can be requested and displayed, and unavailable list
  types remain unchecked and disabled.
- Prevented the About dialog and `/SYSINFO OS` from displaying a null operating
  system when Windows Management Instrumentation is temporarily unavailable.
  Fabulor now falls back to GLib's native Windows version query.
- Kept a locked user-list divider at its configured width through initial
  population, channel changes, and later window-state and scale allocations,
  including high-DPI Windows desktop configurations.
- Restored immediate Windows font discovery in the main-font chooser. Fabulor
  now loads the packaged GTK4 Fontconfig configuration from its installed
  runtime location and refreshes the chooser after it becomes visible.
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
- Prevented Modify from creating a hybrid installation when Portable mode or
  the install folder is changed. Switching modes or locations now requires an
  uninstall and fresh install, and protected portable destinations display a
  prominent warning with a user-writable alternative. Expected unelevated
  pre-plan cleanup skips are no longer reported as setup failures.
- Restored installed ISO language and country data used by spell-check
  language-name lookup.
- Kept the tray plugin bound to a valid surviving session when networks or
  channels close, preventing invalid-context warnings after disconnects.
- Classified TLS read closure and retry states explicitly, preserving
  read/write watches and recording OpenSSL, socket, and operating-system error
  details instead of reporting an ambiguous disconnect.
- Registered missing machine-level `irc://` and `ircs://` scheme fallbacks in
  addition to stable Fabulor ProgIDs and Windows Registered Applications
  capabilities. Setup preserves another application's valid handler, retains
  Fabulor-owned fallbacks through major upgrades, and removes a generic scheme
  only while it still points to Fabulor.
- Preserved an explicit IRC URI channel target when a failed endpoint cycles to
  the saved network configuration before login.
- Preserved independently resized Ban List and Ignore List windows across
  close, reopen, and client restart. Automatic GTK content layout no longer
  replaces dimensions selected through `/SET` or a completed manual resize.
- Made `gui_ulist_resizable OFF` lock the user-list divider to
  `gui_ulist_nick_width`, including after late GTK4 allocation changes, while
  retaining manual and window-driven divider resizing when the setting is
  enabled.
- Removed accidental channel-tree and user-list drag sources that could create
  an unusable detached channel tree during ordinary navigation.
- Added clear non-blocking empty states to the Channel List and user list, and
  kept the user-list controls and side-by-side meters aligned at supported
  panel widths.

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
- Aligned C#, Python, and Tcl manifest plugins on the compact shared
  `FabulorAPI`, rejected unreachable capability names, and repeated message
  and log validation at the native boundary.

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
