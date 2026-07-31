<!-- Fabulor production roadmap -->
# Fabulor Production Roadmap

Last reconciled: 2026-07-31

## Current Production Baseline

- [x] Fabulor targets Windows 11+ x64 with an MSVC-built GTK4 frontend.
- [x] The product version is `1.0.6`.
- [x] WiX v4 produces the supported MSI and bootstrapper artefacts.
- [x] GTK4 is the sole production frontend, CI, staging, runtime, MSI, and
      bootstrapper profile.
- [x] Clean and upgraded installations contain no legacy GTK3 runtime or theme
      payload.
- [x] The installed runtime is executable-relative and allowlisted.
- [x] Python 3.14, Tcl 8.6, and .NET plugin runtimes are optional installer
      features with explicit private roots.
- [x] Enchant 2.8.19 and WinSpell provide the supported Windows spell-checking
      path; the legacy Enchant fallback is retired.
- [x] Installed and portable modes are implemented by the bootstrapper.
- [x] The inherited WinSparkle updater is retired because its external
      appcast offered ZoiteChat packages as Fabulor updates.
- [x] Inno Setup, GTK3 packaging, Lua, Perl, and bundled user add-ons are
      retired from the supported product.

## Production Readiness

### Completed

- [x] Build the common core, GTK4 frontend, launcher, native plugins, MSI, and
      bootstrapper without compiler warnings or errors.
- [x] Validate production identity, feature selection, upgrade identities,
      embedded MSI equality, and GTK4-only installed payload.
- [x] Validate clean installation and repeated versioned upgrades on Windows.
- [x] Validate normal IRC and ZNC connections, TLS, SOCKS5, reconnect, and
      multi-network startup.
- [x] Bound Windows address-pool fallback so a dead DNS result cannot consume
      the operating system's full connection timeout.
- [x] Validate C#, Python, and Tcl plugin hosts and simple add-on loading.
- [x] Validate spell checking, suggestions, personal dictionaries, URL paste,
      emoji and flag rendering, sounds, tray behavior, and themes.
- [x] Validate GTK4 menus, dialogs, channel navigation, transcript selection,
      clipboard copying, URL activation, user-list ownership, and server-tab
      startup behavior through installed-client testing.
- [x] Record accessibility, rendering, lifecycle, performance, packaging, and
      installed acceptance evidence in
      `docs/gtk4/archive/validation-log.md`.
- [x] Publish `v1.0.6-rc.3` after its production-candidate pull request passed
      all required checks, then verify the updater-retirement upgrade and
      release metadata.

### Remaining Release Gate

- [ ] Complete a SignPath Foundation eligibility audit covering the public
      repository, GPLv3 and bundled-component licensing, release status,
      maintainership, MFA, and absence of proprietary payloads.
- [ ] Publish the required code-signing policy, privacy statement, reviewer and
      signing-approver roles, and SignPath attribution before applying for the
      free open-source signing service.
- [ ] Adapt the protected GitHub release workflow so SignPath can verify the
      production build provenance and manually approve signing of the Fabulor
      MSI and bootstrapper.
- [ ] Obtain SignPath approval, verify the Authenticode signatures and RFC 3161
      timestamps, and run installed acceptance against the final signed
      artefacts.
- [ ] Perform one final release-candidate clean-install and upgrade pass from
      the last public installer.
- [ ] Exercise repair and uninstall once against the final signed or
      release-candidate artefacts.
- [ ] Run a final installed accessibility and keyboard-navigation pass over
      the main window, Preferences, Server List, menus, transcript, and plugin
      surfaces.
- [ ] Run a final busy-channel and multi-network performance soak with
      `ui-performance.log` enabled only for diagnosis.
- [ ] Confirm release notes, version metadata, installer hashes, and bundled
      runtime provenance before publishing `1.0.6`.
- [ ] Record any remaining non-blocking visual or usability issues as separate
      follow-up work instead of reopening the completed GTK4 migration.

## User Documentation

- [x] Establish `docs/user` as the versioned GitHub source for the Fabulor user
      manual and map the historical HexChat documentation to current,
      rewritten, verification-required, and retired topics.
- [ ] Write and verify installation, first-start, first-connection, Network
      List, TLS, SASL, ZNC, SOCKS5, auto-connect, and autojoin guidance.
- [ ] Write and verify the main-window, transcript, input, user-list, menu,
      tray, emoji, flag, clipboard, accessibility, and keyboard guidance.
- [ ] Write and verify the built-in command, Preferences, themes, colours,
      sounds, alerts, saved history, and logging guidance.
- [ ] Write and verify add-on installation, capability, safe-mode,
      troubleshooting, security, privacy, migration, and glossary guidance.
- [ ] Perform a final documentation review against the signed release
      candidate and ensure all screenshots exclude personal or sensitive data.

## Security

Detailed evidence lives in:

- [`docs/security/archive/manifest-plugin-disabled-state-audit.md`](docs/security/archive/manifest-plugin-disabled-state-audit.md)
- [`docs/security/archive/plugin-api-boundary-review.md`](docs/security/archive/plugin-api-boundary-review.md)
- [`docs/security/trusted-config.md`](docs/security/trusted-config.md)
- [`docs/security/archive/enchant-windows-crash-analysis.md`](docs/security/archive/enchant-windows-crash-analysis.md)

- [x] Complete manifest-loader disabled-state audit.
- [x] Complete pre-enable design audit.
- [x] Contain plugin roots, manifests, entrypoints, runtime roots, and
      symlink/reparse-point handling.
- [x] Enforce bounded manifest parsing, strict schema validation, per-plugin
      isolation, and dependency-cycle handling.
- [x] Enforce capability validation and runtime gates across C#, Python, and
      Tcl.
- [x] Harden callback allowlists, limits, duplicate handling, queued dispatch,
      lifetime, and cleanup.
- [x] Add an off-by-default trusted-code preference while preserving
      `--no-plugins` precedence and the developer environment override.
- [x] Complete repository secret scanning, static-analysis inventory,
      dependency checks, GitHub Actions review, and payload provenance review.
- [x] Complete the targeted high-risk review of paths, TLS/proxies, process and
      library loading, updater behavior, rendering, URLs, and theme parsing.
- [x] Close or contain the resulting archive, Exec, Enchant, add-on loading,
      log-mask, callback, runtime-root, and theme-import findings.
- [x] Keep API version 1 compact; require a concrete cross-language use case
      before adding shared manifest helpers.
- [x] Complete the deliberate API-boundary review for restricting plugin access
      to the supported `FabulorAPI` surface.

## Plugins And Add-Ons

- [x] Implement the shared native API and plugin lifecycle.
- [x] Implement C#, Python 3.14, and Tcl 8.6 hosts.
- [x] Implement manifest discovery, validation, dependency ordering, language
      loaders, callback dispatch, and lifecycle reporting.
- [x] Keep manifest plugins disabled by default and available through explicit
      trusted-code opt-in.
- [x] Support simple Python and Tcl add-ons from the profile add-ons folder.
- [x] Maintain one simple C#, Python, and Tcl example under `samples/plugins`.
- [x] Move maintained user add-ons to the independent `Fabulor/add-ons`
      repository.
- [x] Document plugin authoring, schema, compatibility, safe mode, and
      troubleshooting.
- [x] Keep broader shared API helpers deferred until concrete add-on
      requirements and cross-language enforcement are approved.

## GTK4 Migration

Detailed architecture and evidence live in:

- [`docs/gtk4/archive/migration-plan.md`](docs/gtk4/archive/migration-plan.md)
- [`docs/gtk4/archive/api-inventory.md`](docs/gtk4/archive/api-inventory.md)
- [`docs/gtk4/list-model-architecture.md`](docs/gtk4/list-model-architecture.md)
- [`docs/gtk4/transcript-rendering-architecture.md`](docs/gtk4/transcript-rendering-architecture.md)
- [`docs/gtk4/spell-input-architecture.md`](docs/gtk4/spell-input-architecture.md)
- [`docs/gtk4/theme-architecture.md`](docs/gtk4/theme-architecture.md)
- [`docs/gtk4/tray-architecture.md`](docs/gtk4/tray-architecture.md)
- [`docs/gtk4/runtime-packaging.md`](docs/gtk4/runtime-packaging.md)
- [`docs/gtk4/archive/validation-log.md`](docs/gtk4/archive/validation-log.md)

- [x] Complete the GTK4 API inventory and retire active GTK3 API branches.
- [x] Convert widget ownership, layout, visibility, and lifecycle.
- [x] Convert actions, menus, dialogs, file selection, input events, shortcuts,
      clipboard, drag/drop, and pointer gestures.
- [x] Convert list, tree, model, channel-navigation, Server List, and
      operational-list ownership.
- [x] Port the transcript and spell-check input widgets to GTK4 rendering,
      selection, accessibility, and event semantics.
- [x] Implement GTK4 desktop-theme and `.hct` colour-palette management with
      transactional preview, persistence, contained archive import, and System
      default restoration.
- [x] Convert tray, notification, icon, font, Windows appearance, and platform
      integration.
- [x] Cut production, CI, staging, WiX, and runtime packaging over to GTK4.
- [x] Remove GTK3 source branches, build inputs, runtime files, installer
      components, and compatibility helpers.
- [x] Complete installed acceptance for the GTK4 production frontend.

The GTK4 migration is complete. Further UI work is ordinary product
maintenance and must be tracked as contained follow-up fixes.

## Installer And Runtime

- [x] Install the core executable, native plugins, GTK4 runtime, Python 3.14,
      Tcl 8.6, .NET host, configuration assets, palettes, emoji, fonts, and
      documentation through explicit WiX ownership.
- [x] Keep registry and shell integration scoped to Installed mode.
- [x] Keep Portable mode self-contained and free of installed-mode shell
      integration.
- [x] Replace broad GTK runtime harvesting with a validated allowlist.
- [x] Trim Tcl to the runtime required by supported scripts.
- [x] Remove Python 3.12, legacy Enchant, GTK3, Inno, Perl, Lua, and
      development-only payloads.
- [x] Validate upgrade removal of retired runtime files.
- [ ] Reassess optional GTK4 locales, icons, schemas, and helper tools only
      after the final release-candidate feature pass.
- [x] Publish release-candidate installer and payload hashes with release
      metadata.

## Obsolete Function Retirement

Detailed policy lives in
[`docs/cleanup/obsolete-function-retirement-plan.md`](docs/cleanup/obsolete-function-retirement-plan.md).

- [x] Retire built-in Identd.
- [x] Retire `gui_ulist_style`.
- [x] Retire Wingate proxy mode without renumbering retained proxy values.
- [x] Retire `text_transparent`.
- [x] Retire the misleading `irc_cap_server_time` preference while preserving
      unconditional server-time negotiation.
- [x] Retire Perl source and residual configuration.
- [ ] Review SOCKS4 separately; its current status remains `Proposed`.

## Repository Cleanup

Detailed stages live in
[`docs/cleanup/repository-cleanup-plan.md`](docs/cleanup/repository-cleanup-plan.md).
The planned repository cleanup programme is complete through Stage 7. Future
cleanup work requires a new contained scope rather than extending the completed
programme.

- [x] Remove dead repository metadata and completed prompt scaffolding.
- [x] Remove unsupported Lua and Perl source.
- [x] Remove retired Inno spelling scripts and superseded resource/version
      inputs.
- [x] Move maintained add-ons to `Fabulor/add-ons`.
- [x] Remove Python 3.12 and generated investigation artefacts.
- [x] Move active `win32/copy` payload assets into explicit `data` ownership.
- [x] Audit the unbuilt text frontend, Windows compatibility shims, and
      duplicate backend implementations.
- [x] Review historical migration/security documents for archive policy.
- [x] Retire stale ZoiteChat product branding and build-only identifiers.
- [x] Review internal ZoiteChat/XChat compatibility names separately from
      product branding.
  - [x] Make `fabulor` the sole Fabulor-owned Python module while retaining
        intentional `xchat` and `hexchat` imports.
  - [x] Make `fabulor::*` the sole public Tcl namespace.
  - [x] Make `IFabulorPlugin`, `FabulorContext`, `FabulorEvent`, and
        `FabulorUserInfo` the sole managed C# plugin contract.
  - [x] Make `fabulor_*` the sole native plugin ABI.
  - [x] Review and rename the remaining internal compatibility names.
- [x] Audit ignored local build/runtime output and document a safe developer
      cleanup command in
      [`docs/cleanup/developer-output-cleanup.md`](docs/cleanup/developer-output-cleanup.md).

## Optional Product Follow-Up

- [x] Design a Fabulor-owned, authenticated, and signed update feed in
      [`docs/security/signed-update-feed-design.md`](docs/security/signed-update-feed-design.md).
- [ ] Provision the update signing identities, metadata origin, bounded TUF
      verifier, Authenticode verification, downloader, and manual UI before
      restoring in-client update checks.
- [x] Store saved input history per network and channel under the profile
      `history` directory, and provide scoped `/CLEAR HISTORY` and `/CLEAR LOG`
      maintenance commands.
- [x] Retain Windows regional-indicator text for editbox flag sequences.
- [x] Add country-name and two-letter-code search/filter support to the flag
      picker.
- [ ] Continue contained installed-UI refinements based on reproducible
      screenshots, logs, and real-world acceptance.
