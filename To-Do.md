<!-- Fabulor production roadmap -->
# Fabulor Production Roadmap

Last reconciled: 2026-11-25 (1.0.9 release)

## Current Production Baseline

- [x] Fabulor targets Windows 11+ x64 with an MSVC-built GTK4 frontend.
- [x] The [latest stable release](https://github.com/Fabulor/fabulor/releases/latest)
      defines the supported production baseline.
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
- [x] Portable mode defaults to a user-writable profile folder and rejects
      protected `Program Files` and Windows destinations before planning.
- [x] The inherited WinSparkle updater is retired because its external
      appcast offered ZoiteChat packages as Fabulor updates.
- [x] Inno Setup, GTK3 packaging, Lua, Perl, and bundled user add-ons are
      retired from the supported product.

## 1.0.9 Patch

- [x] Fix [#333](https://github.com/Fabulor/fabulor/issues/333), where
      collapsing a network could apply its activity colour to channel rows.
      The regression test failed on 1.0.8 and passed with the fix.
- [x] Verify the Windows x64 build, all nine GTK test suites and portable
      startup and shutdown. The maintainer confirmed the live portable test
      passed.
- [x] Test the rebuilt Setup as an in-place upgrade from published 1.0.8.
      The MSI removed the stale 1.0.8 Setup entry. The installed binaries,
      profile configuration and add-on files matched their expected SHA-256
      hashes.
- [x] Confirm the installed client shows 1.0.9 in **Help > About Fabulor**.
      The maintainer verified the channel-tree activity colours in the
      installed build.

## 1.0.8 Release

On 2026-11-23, the maintainer reported no known open bugs in the 1.0.8
portable candidate and accepted the Activity List enhancement as complete.
The right-click menu and the tooltips that show full channel names and
nicknames when shortened received positive feedback.
Emoji picker investigation and cleanup are deferred beyond 1.0.8. Revisit
the intermittent probe if a test fails again or a visible issue returns.
Aegis remains in development as a separate add-on.

- [x] Verify the Python API cache cleanup for 1.0.8. A full uninstall of the
      local test bundle removed all three target cache files and the installation
      directory. See [Installer And Runtime](#installer-and-runtime) for scope
      and [the validation record](docs/gtk4/local-1.0.7-validation.md#follow-up-after-maintainer-run-uninstall)
      for the test result.
- [x] Implement and validate the Activity List and unread counts for 1.0.8.
      The local portable candidate completes the first three stages of the
      [Activity List design](docs/product/activity-list-design.md#staged-delivery).
      These cover the core unread model and focus rules, channel-switcher
      counts, and a navigable Activity List.
  - Stage 1 portable test across six networks: the maintainer reported
    expected channel-switcher colours, including green for ZNC buffextras
    activity involving their nickname, a yellow highlight while testing
    with Borg, and blue for new server-tab data. The maintainer also
    reported that unread channels kept their marks through minimising and
    restoring, while channels they read cleared and stayed clear. The
    maintainer confirmed that a new message reached the selected channel
    while Fabulor was minimised.
  - Stage 2 portable candidate: Release x64 build, the GTK4 dependency
    probe and local IRC smoke tests in tree and tab layouts passed. The
    probe checked in-place count updates, stable label dimensions and an
    exact accessibility description above the visual 999+ limit.
    The maintainer confirmed the real-world channel-switcher check:
    collapsed network totals reflected unread activity in two channels,
    and opening one cleared its count while the other remained unread.
  - Stage 3 portable candidate adds View > Activity List with one row per
    unread server tab, channel or private dialog. Release x64, the 59-case
    native suite, the GTK4 dependency probe and local IRC smoke tests
    passed. GTK checks covered priority and recency, in-place row updates,
    duplicate names on different networks, activation by row identity,
    row removal, the empty state and rendered accessible count descriptions.
    On 2026-11-22, the maintainer reported “All tests pass” after the
    Activity List navigation and focus check. The check covered opening
    an unread context, including in a detached window, and clearing only
    its row.
  - The follow-up Activity List menu adds **Mark as Read**. It clears the
    chosen row and its channel-switcher count without opening the context.
    The maintainer reported that choosing the action had no effect. GTK
    could dismiss the menu before dispatching the action, which discarded
    the target. The fix retains the target through dismissal. The GTK
    probe verifies this order, including a change in row position. A
    Windows portable test verified right-click clearing in the list and
    channel tree, and clearing the selected row with Shift+F10 and Enter.
    On 2026-11-22, the maintainer confirmed the corrected portable build
    worked as expected.
- [x] Add a **Highlights only** toggle to the Activity List. Filtering leaves
      unread counts running for hidden rows. The GTK probe verified
      promotion into the filtered list and restoration of hidden rows. The
      Windows portable check confirmed that hidden channels kept their counts.
- [x] Show the sender and latest unread highlight text in the Activity List.
      The Windows portable check confirmed that a later ordinary message
      leaves the preview in place and opening the context clears it.
      The maintainer confirmed that the preview looked good and worked as
      expected in live testing.
- [x] Implement persistent activity policies, automatic context rules and
      noisy-nick filtering in the follow-up 1.0.8 portable candidate. See
      the [Activity List guide](docs/user/main-window.md#activity-list).
  - Release x64, all 63 native cases and both GTK tests passed. In the
    local IRC test, an automatic mute kept the channel out of the Activity
    List. A muted nick's ordinary message and action stayed in the log,
    and its direct highlight appeared in the list. `/ACTIVITY all` saved
    an explicit override for the muted channel.
  - The maintainer confirmed that activity policies set through the tab
    context menu stayed selected during live portable use.
  - The maintainer verified **Save and Apply to Open Contexts** on the
    already-open ChatLounge ZNC server tab. Its `/ACTIVITY` output changed
    from `auto (muted)` to `auto (all)` after applying an exact rule for
    `ChatLounge ZNC` above the wildcard server-tab mute.
- [x] Check the new activity rules on live networks and a busy bot channel.
  - In a live RSS feed channel, the maintainer confirmed that **Highlights
    only** keeps ordinary feed posts visible in the transcript without
    adding to the unread counter.
  - The maintainer initially reported missing feed posts. After setting the
    channel to **Automatic** and adding `nick | clu | muted`, feed posts
    appeared in the transcript without incrementing the unread counter.
    The earlier behaviour did not recur in this retest.
  - The maintainer reported no noticeable lag while using the portable
    candidate with default UI settings in the busy `#news-rss` channel.
- [x] Add optional, independent nickname and channel-name display limits.
      Clean profiles default to 16 Unicode characters followed by an
      ellipsis when shortened. Existing profiles keep their saved tab-width
      setting, which seeds the channel cap when no cap has been saved.
      The GTK and transcript tests cover Unicode clipping and retaining
      complete names for interaction, copying and searching. Windows
      portable checks covered both switcher layouts, the user list, live
      transcript events and migration of a saved tab-width value.

On 2026-11-22, the maintainer confirmed that live testing of the
**Highlights only** toggle and the name limits passed in the combined
1.0.8 portable build.

- [x] Add [Activity List screenshots](docs/user/main-window.md#activity-list)
      to the user guide. The highlight preview uses sample IRC messages.
- [x] Build the Windows x64 `FabulorSetup-v1.0.8.exe` candidate. Product
      metadata, the production MSI and GTK4 runtime contents passed validation.
      The bundle contains the same MSI verified independently by SHA-256.
- [x] Test the final 1.0.8 installer as an in-place upgrade from released
      1.0.7 and verify profile and add-on preservation.
  - The maintainer confirmed **Help > About** shows 1.0.8 after the upgrade.
    The installed launcher and GTK4 frontend matched the candidate binaries
    by SHA-256.
  - The maintainer confirmed that networks, preferences and add-ons carried
    over and that `%APPDATA%\Fabulor` was backed up as a precaution.
- [x] Verify that uninstall removes the 1.0.8 MSI payload and preserves
      `%APPDATA%\Fabulor`. Windows Installer reported successful removal.
      The launcher, frontend, bundled Python and the three targeted API
      cache files were absent after uninstall. All 1,920 files left under
      `Runtime\Python314` matched the manually installed OpenAI SDK and
      its dependencies by SHA-256.
  - After manually removing the old 1.0.7 Setup entry, the maintainer
    reinstalled and uninstalled 1.0.8 successfully. The follow-up check found
    zero Fabulor uninstall registrations. The installed executable was gone
    and the installed-mode profile was still present.
- [x] Resolve the stale 1.0.7 Setup entry left after the 1.0.8 upgrade and
      uninstall. Both Burn logs show that the bootstrapper skipped
      post-apply registry cleanup because it was not elevated.
  - The maintainer removed the old 1.0.7 Setup entry manually. A follow-up
    registry check found zero Fabulor uninstall registrations, and the
    installed-mode profile was still present.
  - A repeat upgrade from the hash-verified published 1.0.7 installer
    reproduced the duplicate Setup entries. Running 1.0.8 Repair with the
    bootstrapper launched as administrator removed the stale entry. Only
    the 1.0.8 Setup registration and its MSI remained.
  - The revised MSI removes the published 1.0.7 Setup registration when it
    detects an upgrade from that MSI and finds the matching bundle provider.
    An ordinary upgrade from the hash-verified published installer passed.
    The MSI log confirmed removal of the old key. Windows listed a single
    1.0.8 Setup entry, the installed binaries matched the revised candidate,
    and the Aegis SDK still imported.
- [x] Verify a clean 1.0.8 install with the existing user profile. The
      maintainer reported that the install completed with their data intact.
- [x] Retest the installer rebuilt after release review. An ordinary upgrade
      from the published 1.0.7 left one 1.0.8 Setup entry. The installed
      binaries and existing profile settings matched their expected SHA-256
      hashes, and the add-on directory remained present.
- [x] Verify repair with the final 1.0.8 installer. The elevated Repair
      ran the 1.0.8 MSI repair and returned success. The installed client
      binaries matched the candidate. The profile configuration files were
      unchanged by SHA-256, the add-on directory remained present, and the
      separately installed OpenAI SDK still imported.

## 1.0.7 Development And Release

- [x] Correct CTCP own-echo handling before flood/DCC parsing and in the
      handler, preserving ACTION display and deliberate self-queries. Omit
      unavailable/empty CPU brackets from cached system strings. Sixteen native
      cases and six source contracts pass; both original defects fail the same
      tests against preserved pre-fix source. Local installer built/validated.
- [x] Record focused installed CTCP success on direct EYErc (2026-11-08):
      supplied output shows MossIRC's normal reply to Alchera and a complete
      Fabulor OS/CPU reply to Barry. No extra self-VERSION lines appear in the
      supplied exchange; the away notice is normal network feedback.
- [ ] Keep broader installed CTCP coverage separate: explicit self-query,
      CPU-unavailable fallback, ACTION/DCC/custom-reply interactions and other
      network/capability combinations. Exact original duplicate wire sequence
      remains unconfirmed; no further code change is indicated by this retest.

- [x] Inventory registered local worktrees and compare private preparation
      changes with main; record the initial source/test/document inventory in
      [the consolidation handoff](docs/gtk4/local-1.0.7-publication-handoff.md).
      Two worktrees are clean; main and the private preparation tree are dirty.
- [x] Reconcile the 1.0.7 client, review the add-on bridge and OPER changes,
      wire the new CI checks, and validate the integrated installer.
      The maintainer confirmed the corrected installer before publication.
      See [PR #328](https://github.com/Fabulor/fabulor/pull/328).

- [x] Reconcile this work's local feature/acceptance records with supporting
      user/architecture/runtime guidance and the documentation index. Replace
      stale blanket pending wording with scoped installed feedback; distinguish
      published 1.0.6 stable from local unpublished 1.0.7 in README/SECURITY.
      Add DCC bind/address troubleshooting without publishing personal IPs.
      Unrelated add-on/OPER work and broad release acceptance remain separate.

- [x] Correct competing outgoing DCC read/write watches on Windows, preserve
      ACK reads through throttling/final completion, append fragmented ACKs
      correctly and cap reads at the announced file size. Eight native sender
      loopback cases and six source contracts pass, including the full reported
      ZIP with matching received SHA-256. Both active/passive entry points use
      the unified helper; handshakes/UI/accounting are not full-client tested.
- [x] Record focused installed DCC success: after further connection/bind
      failures and guidance to clear the public address from Bind to and put
      the WAN address in DCC IP address, the maintainer reported "It succeeded
      this time" (2026-11-08). Direction, file and active/passive mode were not
      separately specified for that successful retry.
- [ ] Complete broader external DCC acceptance: ordinary/passive transfers
      where supported, both directions, full reported ZIP, progress/completion
      and cancel. Distinguish local bind failures, connection timeouts and
      ACTIVE data stalls. Network settings were changed manually by the
      maintainer, not by the code fix or automated tests.

- [x] Correct message hit-testing after a clipped sender prefix. Newly appended
      entries can clamp their stored indent while rendering resets the body to
      the separator column; selection previously counted the full hidden prefix.
      Nine native widget cases and three source contracts cover the correction,
      both selection directions, colour/bold/Unicode text, ordinary channel/
      server entries, wrapped lines, recalculated indents and prefix identity.
- [x] Record focused installed acceptance of the selection fix. After receiving
      the candidate and right-to-left selection/copy checks, the maintainer
      reported: "That seems to have fixed that. TY. 👍" (2026-11-07). This closes
      the reported mismatch; individual clipboard, direction and channel/server
      checks were not separately reported. Broader interaction/platform coverage
      remains separate from this focused feedback.

- [x] Implement the initial opt-in spell auto-correct for a conservative English
      typo list on Space (superseded by Windows suggestions below), with active-dictionary validation, known-nickname
      vetoes and immediate Backspace undo. Preserve commands, pasted/history
      text, links, code-shaped tokens, formatting and active IME composition;
      never trigger correction on Enter. Default off, no automatic learned-pair
      persistence. Eleven native policy/widget cases, six source integration
      checks and an Australian English dictionary smoke probe pass.
- [x] Diagnose the initial installed `teh` failure: the agent's session-ignore
      probe hit upstream WinSpell's persistent Windows Add hook. Recover only
      that test-added word, verify every other dictionary entry is preserved,
      disable both provider session hooks in the reproducible build recipe,
      and isolate/guard future word tests. Real Windows keys in the actual
      spell entry now correct `teh dayz ` to `the dayz `, and Ignore stays local
      to the dictionary. Extracted MSI provider hashes match the rebuilt stage.
- [x] Record installed acceptance of the replacement auto-correct candidate.
      After receiving the session-isolation fix and correction/undo retest
      instructions, the maintainer confirmed: "Perfect 💯". This accepts the
      reported correction fix; individual checks were not separately reported.
      The first candidate was not accepted.
- [x] Replace the fixed typo table with the first available Windows/WinSpell
      suggestion, preserving its order, case and Unicode spelling. The maintainer
      approved preferred suggestions despite alternatives after reporting missed
      `sleke`, `btter` and `turkey`/`Turkey`. Fourteen native cases, six source
      integration contracts and real Windows keys in the production spell entry
      cover broader corrections and Unicode Backspace undo. The earlier positive
      feedback did not establish general typo coverage.
- [x] Record focused positive installed feedback for the Windows-suggestion
      candidate: "This seems to working mopre appropriately. 👍" (2026-10-30).
      This supports improved correction behaviour, not separately reported
      results for every example or undo sequence.
- [ ] Separately confirm the reported examples and undo in the installed
      Windows-suggestion candidate: `sleke` → `sleek`, `btter` → `better`,
      and the current Windows `Türkiye` suggestion.
- [ ] Complete the broader auto-correct matrix: off-by-default/persistence,
      Preferences enable/disable, Space and immediate Backspace, dictionary/
      session-ignore behaviour, nicknames, commands, URL/paste/history/emoji,
      tab changes, real Windows keyboard/IME and accessibility interactions.
      Automated controller emissions are not installed platform acceptance.

- [x] Implement Windows Sysinfo sound-device, physical network-adapter and
      matching chipset-controller name queries; add RAM/MEMORY alias handling,
      count full disks correctly and harden WMI value parsing/cleanup. Native
      synthetic tests, source contracts and a local availability-only WMI smoke
      probe passed. No IP/MAC collection or announcement-default change.
- [x] Record installed acceptance of the Sysinfo candidate. After receiving
      the installer and local-output command checks, the maintainer confirmed:
      "Perfect 💯". This accepts the requested Sysinfo improvements; separate
      coverage of every command, summary preference and driver configuration
      was not reported. Chipset naming remains driver-dependent.

- [x] Guard timestamp-only paints from nickname-prefix clipping when timestamp
      copying reuses a message entry. The regression source contract fails
      before the fix and passes afterwards; builds/package checks passed.
- [x] Record focused installed feedback for the 2026-10-29 Services timestamp
      candidate. The maintainer reported: "That seems to have worked. :)"
      The reported overlap appears resolved; exhaustive blank/help-line,
      selection/redraw and ordinary nickname-clipping coverage was not
      separately reported.
- [ ] Deferred (maintainer decision, 2026-11-21): investigate the intermittent
      Win32 emoji-probe failure observed during
      parallel and serial validation runs. Full-suite passes and failed
      repetitions are retained, including in the 2026-10-29 Sysinfo evidence;
      serial execution alone does not eliminate it. On 2026-11-21 the released
      source passed 24 serial and 12 parallel repetitions, with all 259 flags
      verified each time. Win32 font-fallback warnings still occur. The earlier
      source reproduces the teardown warning, which the release code avoids.
      The historical non-zero exit remains unreproduced in this investigation.
      The maintainer and testers report normal behaviour with the fixed picker.
      Resume investigation if a test fails again or a visible problem is reported.
      See the [follow-up evidence](docs/gtk4/local-1.0.7-validation.md#2026-11-21--intermittent-emoji-probe-follow-up).

- [x] Add DALnet-only DCC permission actions to the nickname menu without
      replacing `popup.conf`. Recognise the configured/network-advertised
      DALnet identity or a `dal.net` server hostname; bind permission actions
      to the original session/server ID, recheck live connection and network,
      validate nickname tokens and leave auto-accept settings untouched.
      Native policy/model tests and source-integration checks pass.
- [x] Record installed acceptance of the DALnet DCC menu candidate. On
      2026-10-23 the maintainer confirmed: "It works as expected. :)" after
      being asked to check DALnet/ZNC visibility, absence on other networks
      and allow/list/revoke server responses. This accepts the requested
      feature; exhaustive disconnect/origin-close, multi-selection and
      tab-switch scenarios were not separately reported.

- [x] Build a local unified GTK emoji chooser candidate for the smiley icon and
      Ctrl+;/Ctrl+. shortcuts, selecting the bundled colour-emoji font only
      inside that chooser. The first CSS-only candidate failed installed flag
      rendering; a follow-up explicitly assigns a chooser-only FreeType font
      map and checks actual label fonts under both Win32 and Fontconfig default
      backends. The maintainer's 2026-10-23 screenshot confirms graphical
      rendering of the visible country flags in the follow-up. The maintainer
      subsequently accepted the picker as working well; the broader release
      display-scale matrix remains separate.
      Native tests cover shared ownership, shortcut
      routing, selection replacement, cursor position, read-only input and all
      259 country-flag sequences; all 253 legacy country codes are represented.
      No GTK-private widget restructuring or production catalogue parsing.
- [x] Record installed acceptance of the unified emoji picker. On 2026-10-23,
      after graphical flag evidence and a request to check insertion, search
      and Escape, the maintainer confirmed: "Yes. All works well. Very well in
      fact." This closes the reported picker issue, not explicit coverage of
      every flag, variant, theme or 250%/300% scenario. The candidate uses
      bundled font glyphs, not PNG substitution.
- [x] Correct the oversized emoji picker in the integrated 1.0.7 installer.
      Activate the bundled bitmap-font rule during GTK4 runtime staging and
      verify the installed rule and rendered glyph size. On 2026-11-16 the
      maintainer confirmed a successful upgrade and normal picker sizing at
      250%. See the [validation record](docs/gtk4/local-1.0.7-validation.md#oversized-emoji-picker-at-250).
- [ ] Retire the superseded custom emoji builder and reconcile its legacy tests
      and packaging claims in a separate, contained cleanup after acceptance.

- [x] Ship long nickname and channel-name handling in 1.0.7. Nicknames clip
      to the available width in the transcript and user list. Channel names
      clip to the available width in the channel tree and tabs. Full-name
      tooltips expose the complete name. This incorporates the private
      `a199` worktree changes for indent bounds, tooltip positioning and
      background colour alongside the accepted cache and divider fixes.
      Native and package checks passed locally.
- [x] Record installed acceptance of the restored private name-layout feature.
      On 2026-10-23 the maintainer confirmed the rebuilt candidate fixed the
      reported regression. This closes that report, not explicit coverage of
      every hover/click/copy, background or 250%/300% tooltip scenario.
      See [local v1.0.7 validation](docs/gtk4/local-1.0.7-validation.md).

- [x] Credit PickleRick83 and MrBenC in About → Credits → Testers. The
      maintainer confirmed the local upgrade and displayed credits on
      2026-10-22.
- [x] Fix repeated background-image scaling during transcript paints and
      incorrect divider-size calculations during server/channel tab switches.
      Native regression tests cover cache reuse/invalidation and pane
      positioning, including hide/show and normal/wide dividers in both
      orientations.
- [x] Record focused installed-client acceptance of the reported tab-switching
      and background-image redraw issue. On 2026-10-22 the maintainer reported
      a definite improvement and subsequently accepted the issue as fixed.
      See [local v1.0.7 validation](docs/gtk4/local-1.0.7-validation.md).

Broader performance soak and release coverage are tracked below. The
[1.0.7 release](https://github.com/Fabulor/fabulor/releases/tag/v1.0.7)
uses the unsigned installer accepted during the 250% picker retest.
The WinGet acceptance and Microsoft Store tracks are separate.

## 1.0 Release Gate

### Completed

- [x] Build the common core, GTK4 frontend, launcher, native plugins, MSI, and
      bootstrapper without compiler warnings or errors.
- [x] Validate production identity, feature selection, upgrade identities,
      embedded MSI equality, and GTK4-only installed payload.
- [x] Validate clean installation and repeated versioned upgrades on Windows.
- [x] Validate the RC9 clean-install, installed-mode modification, portable
      install, and uninstall paths. Installed modification preserved
      `%APPDATA%\Fabulor`; the portable profile remained self-contained; and
      uninstall preserved the installed-mode user profile.
- [x] Validate normal IRC and ZNC connections, TLS, SOCKS5, reconnect, and
      multi-network startup.
- [x] Bound Windows address-pool fallback so a dead DNS result cannot consume
      the operating system's full connection timeout.
- [x] Validate C#, Python, and Tcl plugin hosts and simple add-on loading.
- [x] Provide trusted simple Tcl add-ons with a narrow native Windows-uptime
      query backed by `GetTickCount64`, avoiding unreliable GUI-hosted child
      process execution without expanding manifest-plugin permissions.
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
- [x] Publish `v1.0.6-rc.4` with strict IRC URI parsing and a typed Windows
      existing-instance handoff after installed testing confirmed that links
      join unopened channels and focus channels that are already open.
- [x] Complete a one-week external tester soak with an experienced HexChat
      user exercising the installed client broadly. The only report was one
      non-reproducible channel-switcher tree duplication that cleared after a
      normal restart and has not recurred.
- [x] Fix tray plugin context lifetime after network and channel teardown so
      post-disconnect tray refreshes cannot query a freed session
      ([issue #297](https://github.com/Fabulor/fabulor/issues/297)). Installed
      `v1.0.6-rc.6` acceptance confirmed that the warning no longer recurs in
      the tested disconnect and tray workflows.
- [x] Classify TLS read closure and retry states, preserve the required socket
      watches, and record useful diagnostics for unexpected encrypted
      disconnects ([issue #294](https://github.com/Fabulor/fabulor/issues/294)).
      Installed `v1.0.6-rc.6` acceptance confirmed the corrected diagnostic
      behaviour.
- [x] Publish `v1.0.6-rc.6` from the accepted source revision with release
      notes, the versioned installer, and its SHA-256 checksum.
- [x] Publish `v1.0.6-rc.7` with the contained Windows uptime API correction
      and complete installed acceptance before beginning the final interface
      refinement pass.
- [x] Publish `v1.0.6-rc.8` after the installed UI regression pass confirmed
      the permanent channel Modes popover, responsive main-window and list
      layouts, persistent Ban List and Ignore List dimensions, background-image
      controls, and empty states.
- [x] Preserve the optional Python, Tcl, and .NET plugin runtime payloads during
      major upgrades, retain MSI rollback safety, and safely remove only stale
      superseded Burn registrations and caches
      ([PR #312](https://github.com/Fabulor/fabulor/pull/312)). Installed
      testing confirmed that Python add-ons load after the corrected upgrade.
- [x] Add a prominent known-issue warning to the published RC8 release notes:
      its original installer may not preserve the bundled Python runtime during
      an in-place upgrade, while clean installations remain unaffected.

### Remaining Checks

- [x] Document the Windows SmartScreen warning shown for unsigned release
      candidates, including official-source and SHA-256 verification without
      advising users to disable Defender or SmartScreen.
- [x] Validate the RC9 candidate payload through an RC8-to-RC9 in-place
      upgrade, clean installation, installed-mode modification, portable
      installation, and uninstall. Selected plugin runtimes and add-ons loaded
      correctly, modified features changed as requested, portable data stayed
      self-contained, and installed-mode profile data survived uninstall.
- [x] Publish `v1.0.6-rc.9` from the reviewed and merged correction source.
- [x] Publish `v1.0.6-rc.10`. Focused installed testing at 250% Windows desktop
      scale has confirmed that a locked user-list divider retains its configured
      width through startup, late layout changes, server/channel switching, and
      long nickname rendering. The main-font chooser also lists installed fonts
      without requiring a filter selection.
- [x] Activate and accept Per-Monitor-V2 DPI awareness for RC11. The
      GTK 4.22.4 Win32 fractional-scale patch, native GTK regression tests,
      pinned builder, immutable Fabulor runtime archive, staged payload, Fabulor
      fractional surface-scale redraw handling, and Windows Release build have
      passed automated validation. Installed testing confirms genuine
      per-monitor process awareness and clean 300%-to-250%-to-300% rescaling
      without pane drift. Extended use at 300% also passed with multiple themes,
      backgrounds, menus, Ban and Ignore Lists, compact mode, and single-line
      topics without a reported stability or layout regression.
- [x] Confirm RC12 multi-monitor and detached-window acceptance. The maintainer
      reported successful tester results on 2026-10-04 and no regressions during
      the public testing soak.
- [x] Record multi-monitor testing at 100%, 125%, 150%, 175%, 200%, and 225%.
      On 2026-11-22 the maintainer confirmed that the tester passed all six
      scales. The 250% and 300% results are recorded in the RC11 entry above.
- [x] Validate the corrected major-upgrade installer over the earlier PMv2 test
      build with Python 3.14.6. Confirm that the selected pinned Python 3.14.3
      files, including `python314.dll`, are installed without a separate Repair,
      and that Python add-ons load after the upgrade. The published RC11 upgrade
      skipped the older-version files before removing the test build. Installed
      acceptance passed: Python 3.14.3 replaced 3.14.6 and add-ons loaded without
      Repair. PR #322 is merged and the tested RC12 installer is published.
- [x] Validate the final `1.0.6` WiX candidate with the maintainer's local
      WinGet manifest before publishing source changes, release assets, or a
      manifest submission. Installation, RC12 upgrade, optional runtimes,
      uninstall and clean reinstallation passed. Burn detected the existing
      install; public WinGet package detection and Repair remain separate checks.
- [ ] Exercise repair and uninstall once against the final signed or
      release-candidate artefacts.
- [ ] Run a final installed accessibility and keyboard-navigation pass over
      the main window, Preferences, Server List, menus, transcript, and plugin
      surfaces.
- [ ] Run a final busy-channel and multi-network performance soak with
      `ui-performance.log` enabled only for diagnosis.
- [ ] Confirm release notes, version metadata, installer hashes, and bundled
      runtime provenance before publishing `1.0.6`.
- [x] Implement the installed Windows registered-application boundary and
      guarded machine-level fallback claims for both `irc://` and `ircs://`
      without replacing another application's valid handler
      ([issue #298](https://github.com/Fabulor/fabulor/issues/298)). A clean
      installation of `v1.0.6-rc.6` confirmed both schemes, Windows Default
      Apps discovery, and preservation of the user's existing default-handler
      choice.
- [ ] Verify clean install, upgrade, modify, repair, uninstall, and running and
      closed-client dispatch for both registered IRC link schemes.
- [ ] Record any remaining non-blocking visual or usability issues as separate
      follow-up work instead of reopening the completed GTK4 migration.

### Release Watch List

- [ ] Monitor the one-time duplicated channel-switcher tree report. Keep it as
      a roadmap note unless it recurs with a screenshot or reproducible
      channel-switch sequence; only then promote it to a tracked issue.
      On 2026-11-21 the maintainer confirmed no known recurrence or further
      reports.

## User Documentation

- [x] Establish `docs/user` as the versioned GitHub source for the Fabulor user
      manual and map the historical HexChat documentation to current,
      rewritten, verification-required, and retired topics.
- [ ] Write and verify installation, first-start, first-connection, Network
      List, TLS, SASL, ZNC, SOCKS5, auto-connect, and autojoin guidance.
  - [x] Write the source-verified `getting-started.md` and
        `networks-and-connections.md` drafts.
  - [ ] Verify both pages against the final installed release candidate before
        marking this stage complete.
- [ ] Write and verify the main-window, transcript, input, user-list, menu,
      tray, emoji, flag, clipboard, accessibility, and keyboard guidance.
  - [x] Write the source-verified `main-window.md` draft.
  - [x] Add privacy-safe screenshots where they materially clarify the layout.
  - [ ] Verify the page through the final installed release-candidate keyboard
        and accessibility pass before marking this stage complete.
- [ ] Write and verify the built-in command, Preferences, themes, colours,
      sounds, alerts, saved history, and logging guidance.
    - [x] Write the source-verified `commands.md` draft.
    - [ ] Verify the published command syntax and context requirements against
          an installed release candidate.
      - [x] Verify the optional Exec plugin with `/EXEC ping google.com` in the
            installed client.
    - [x] Write the source-derived `settings-reference.md` draft covering every
          live Windows `/SET` key, its value type, and its purpose.
    - [x] Compare all 226 live Windows `/SET` registry entries with the settings
          reference and verify names, value types, numeric-choice guidance,
          sensitive-value warnings, retired-name guidance, and duplicates.
  - [x] Write the source-verified `preferences.md` draft.
  - [ ] Verify the Preferences page inventory, save/cancel behaviour, and
        restart-required settings against an installed release candidate.
  - [x] Write the source-verified `themes-and-colours.md` draft.
  - [ ] Verify bundled and profile `.hct` selection, GTK4 archive import,
        variants, System default, rollback, and recovery guidance against an
        installed release candidate.
  - [x] Write the source-verified `sounds-and-alerts.md` draft.
  - [ ] Verify Windows notifications, tray and taskbar attention, global and
        per-tab suppression, event-sound assignment, and persistence against
        an installed release candidate.
- [ ] Write and verify add-on installation, capability, safe-mode,
      troubleshooting, security, privacy, migration, and glossary guidance.
  - [x] Write the source-verified `addons.md` draft.
  - [ ] Verify simple C#, Python, and Tcl installation, startup reporting,
        manual loading, manifest opt-in, blacklist handling, and
        `--no-plugins` recovery against an installed release candidate.
  - [x] Write the source-verified `security-and-privacy.md` draft.
  - [ ] Verify TLS and certificate warnings, Credential Manager and encrypted
        profile storage, client-certificate removal, local-data paths,
        diagnostic redaction, and retired updater status against an installed
        release candidate.
    - [x] Write the source-verified `troubleshooting.md` draft.
    - [ ] Verify safe mode, temporary-profile isolation, repair, connection and
          proxy diagnosis, spell checking, theme and sound recovery, UI
          profiling, and bug-report collection against an installed release
          candidate.
    - [x] Write the source-verified `migration-from-xchat-and-hexchat.md` draft.
    - [ ] Verify selective profile migration, network recreation, retained
          Python compatibility imports, current Tcl and C# contracts, theme and
          sound transfer, and retired-feature guidance against an installed
          release candidate.
    - [x] Write the source-verified `glossary.md` draft.
    - [ ] Verify glossary terminology against the final manual and installed
          release candidate, removing implementation terms that ordinary users
          do not need.
- [ ] Perform a final documentation review against the final installed release
      candidate and ensure all screenshots exclude personal or sensitive data.

## Distribution And Future Signing

On 2026-10-04 the maintainer selected WinGet as the next distribution route,
with Microsoft Store packaging deferred. Fabulor will not pursue SignPath
Foundation. The WiX candidate remains unsigned; WinGet does not sign it.

- [x] Complete a SignPath Foundation eligibility audit covering repository
      visibility, GPLv3 and bundled-component licensing, release status,
      maintainership, MFA, governance, metadata, and build provenance.
  - [x] Record the evidence, gaps, decision, and remediation order in
        `docs/security/signpath-foundation-eligibility-audit.md`.
- [x] Complete the bundled-component inventory, package all required licence
      and notice files, and generate a deterministic CycloneDX 1.6 release
      SBOM. See `docs/legal/bundled-components-and-sbom.md`.
- [x] Normalize product name, company/publisher, and version metadata across
      the application, frontend, MSI, bootstrapper, and SignPath artifact
      restrictions. See `docs/security/signpath-artifact-metadata.md`.
- [x] Complete public repository governance for the SignPath application.
  - [x] Require organization-wide two-factor authentication and secure methods.
  - [x] Confirm every participating member has completed secure MFA setup.
  - [x] Close six superseded draft pull requests, delete fourteen obsolete
        remote feature branches, and enable automatic head-branch deletion.
  - [x] Complete the private-to-public disclosure scan. Full-history Gitleaks
        results contained only two reviewed source-text false positives; the
        maintainer accepts publication of the historical Gmail commit address.
  - [x] Make `Fabulor/fabulor` public, protect `main` and release tags, and
        require review of source, installer, build, and workflow changes.
- [x] Publish the required code-signing policy, privacy statement, reviewer and
      signing-approver roles, and SignPath attribution, then submit the
      SignPath Foundation application.
  - [x] Add the policies, named roles, sensitive-path ownership, and signed
        release disclosure to the repository.
  - [x] Confirm the policy links resolve publicly.
  - [x] Record the 2026-08-13 application result. SignPath declined the initial
        application because the newly public project does not yet have enough
        community adoption and independent visibility; no engineering,
        security-control, or licensing defect was identified.
- [x] Validate `Fabulor.yaml` against the final candidate and test through
      WinGet locally before publication. On 2026-10-04 the maintainer accepted
      the RC12 in-place upgrade and subsequent clean installation, with plugins
      loading and no reported regressions. Separate Repair coverage remains open.
- [x] Prepare `.github/workflows/winget-publish.yml` using Microsoft
      WingetCreate, the `WINGET_PUBLISH_TOKEN` repository secret, verified
      versioned release assets, and multi-file manifests. Stable release events
      prepare review artefacts only; submission requires a manual run with
      `submit` selected. Hosted execution remains to be verified after merge.
- [ ] After acceptance and publication of immutable release assets, submit the
      manifest to `microsoft/winget-pkgs` and verify its published install path.
- [ ] Consider Microsoft Store packaging separately after WinGet distribution.
- [ ] If signing is selected later, review build provenance and approval
      controls, verify Authenticode and RFC 3161 timestamps, test the signed
      packages, and update the disclosure before publication.
- [x] Design a Fabulor-owned, authenticated, and signed update feed in
      [`docs/security/signed-update-feed-design.md`](docs/security/signed-update-feed-design.md).
- [ ] Provision the update signing identities, metadata origin, bounded TUF
      verifier, Authenticode verification, downloader, and manual UI before
      restoring in-client update checks.

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
- [x] Replace inherited IRC URL command interpolation with a strict shared URI
      parser and a typed, size-limited Windows handoff to the running client.
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

- [x] Verify Python cache ownership and uninstall behaviour against 1.0.7,
      then address any confirmed Fabulor-generated leftovers while preserving
      user-authored files and profile data. RC11 left
      `python/__pycache__/*.pyc` beneath the install directory.
      On 2026-11-21 the maintainer reported a recent uninstall, an initial quick
      look that found no cache files, and one Fabulor Setup entry in Installed
      apps. A subsequent recursive scan of the current installation found
      three `.pyc` files in `python/__pycache__` and another 1,788 under
      `Runtime/Python314`, totalling 11.0 MB. Two runtime folders denied access,
      so these are minimum counts. Cache presence in the current installation
      was followed by a maintainer-run uninstall and another scan. All 1,791
      visible cache files remained, including the three under
      `python/__pycache__`; no Fabulor uninstall registrations were found.
      The published MSI's file table did not include any of the 3,710
      accessible leftovers. The local installer fix targets the three
      Fabulor API caches by name. A full uninstall of the local test bundle
      removed all three caches and the installation directory.
      The additional runtime files include packages installed outside the MSI.
      See the [scan evidence](docs/gtk4/local-1.0.7-validation.md#2026-11-21--installed-python-cache-observations).

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
- [x] Publish release-candidate installer and payload hashes with release
      metadata.
- [x] Replace the engineering-oriented bootstrapper screen with compact
      install and maintenance layouts, collapsed advanced options and details,
      a completion state, keyboard labels, and display-scaling constraints.
- [x] Add timestamped installer logs beneath
      `%LOCALAPPDATA%\Fabulor\Installer\Logs`, retaining the ten newest normal
      logs and failed logs for 90 days, with folder and error-copy actions.
- [x] Add a separately selectable installed-mode Desktop shortcut while
      retaining the Start Menu and Safe Mode shortcuts and excluding all
      shortcut features from Portable mode.
- [x] Validate the installer improvement pass through an installed upgrade:
      the Desktop shortcut was created and a persistent setup log was written.
      The MSI passed local ICE validation with zero warnings and errors after
      aligning the Desktop component with its per-user `HKCU` key path.

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

- [x] Correct IRCv3 capability handling for valued, dynamically announced,
      disabled, and SASL capabilities; add focused protocol tests.
- [x] Implement bounded IRCv3 `batch` tracking and `labeled-response` support.
- [x] Implement on-demand `draft/chathistory` requests while preserving normal
      ZNC playback behavior. See
      [`docs/protocol/ircv3-support.md`](docs/protocol/ircv3-support.md).
- [x] Verify `/CHATHISTORY` rejection against unsupported DALnet, ChatLounge,
      Libera.Chat, and ZNC connections.
- [x] Verify `/CHATHISTORY` against a server that advertises
      `draft/chathistory`. Ergo Testnet advertised the uppercase ISUPPORT token
      `draft/CHATHISTORY=1000`, acknowledged the lowercase IRCv3 capability
      `draft/chathistory`, and returned history for the argument-free
      `/CHATHISTORY` request on 2026-08-15.
- [x] Store saved input history per network and channel under the profile
      `history` directory, and provide scoped `/CLEAR HISTORY` and `/CLEAR LOG`
      maintenance commands.
- [x] Retain Windows regional-indicator text for editbox flag sequences.
- [x] Add country-name and two-letter-code search/filter support to the flag
      picker.
- [x] Define the optional post-1.0 Activity List, unread-counter, per-context
      policy, automatic-policy, and noisy-nick design in
      [`docs/product/activity-list-design.md`](docs/product/activity-list-design.md).
- [ ] Design and implement post-1.0 inline image previews for direct image
      URLs. Keep loading asynchronous and acceptance-gated; default to
      click-to-load, preserve transcript selection and scroll position, and
      enforce strict download, decode, redirect, cache, concurrency, and
      private-network access limits before automatic previews are offered.
- [x] Implement the fixed nickname and channel-name limits in the
      [1.0.8 release](#108-release).
- [x] Reassess optional GTK4 locales, icons, schemas, and helper tools after the
      release-candidate feature pass. The locked runtime now retains supported
      translations, Adwaita, schemas, fonts, native dependencies, and spawn
      helpers while removing 271 unused files (16.21%). See
      [`docs/gtk4/runtime-packaging.md`](docs/gtk4/runtime-packaging.md).
- [ ] Review SOCKS4 separately; its current status remains `Proposed`.
- [ ] Continue contained installed-UI refinements based on reproducible
      screenshots, logs, and real-world acceptance.
    - [x] Consolidate channel modes into the permanent topic-bar **Modes**
          popover and retire the two obsolete mode-placement settings.
    - [x] Make the user-list footer, meters, minimum width, channel tree,
          topic bar, input row, Preferences categories, Network List, Channel
          List, and background-image controls responsive and visually
          consistent.
    - [x] Add non-blocking empty states for an unpopulated or filtered Channel
          List and a channel with no visible users.
    - [x] Preserve independently resized Ban List and Ignore List window
          dimensions across close, reopen, and client restart. Empty and
          populated content no longer overrides `/SET` values or dimensions
          captured after a completed manual resize.
    - [x] Make `gui_ulist_resizable OFF` lock the user-list divider to
          `gui_ulist_nick_width`; retain manual divider resizing only while the
          setting is enabled.
    - [x] Remove accidental channel-tree and user-list drag sources that could
          create an unusable detached channel tree during ordinary navigation.
    - [x] Complete the final installed RC8 UI regression pass and reconcile
          release-facing documentation.
    - [x] Package RC8 after the UI pull request is reviewed and merged.
