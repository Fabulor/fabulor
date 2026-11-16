# Local v1.0.7 Development And Validation

Initial record: 2026-10-22. Last reconciled: 2026-11-16.

This record covers development and validation for
[Fabulor 1.0.7](https://github.com/Fabulor/fabulor/releases/tag/v1.0.7).
Individual sections distinguish automated checks, focused installed feedback
and broader acceptance.

## Scope And Evidence Labels

Publication preparation is tracked separately in the
[consolidation handoff](local-1.0.7-publication-handoff.md). Its inventory includes
other registered worktrees and separate add-on/OPER work; this validation record
alone does not establish that every isolated change is integrated or release-ready.

This record began with pauses when clicking individual server/channel tabs and
slow transcript redraw with a background image. Mouse-wheel scrolling of the
channel tree was reported to be smooth and was not changed. Dated follow-ups
also cover nickname layout, emoji, DALnet permissions, timestamp rendering,
Sysinfo, spell auto-correct/session isolation, clipped-prefix selection and DCC
sender/configuration testing. This is not a complete audit of unrelated dirty-tree
add-on/OPER development or general release acceptance.

- **Source-verified:** implementation and automated regression coverage.
- **Installed-verified:** maintainer reports and supplied timing captures from
  the locally rebuilt, installed v1.0.7 client.
- Package checks verify the installer contents. The maintainer performed the
  upgrade, and the extracted portable client was tested independently.

## Corrections

### Retained Background Cache — Source-Verified

`src/fe-gtk/xtext-background.c` no longer discards its scaled viewport cache at
both frame boundaries. It reuses one cached image while viewport dimensions,
tile offsets, dimming and all fallback-colour channels match. Source reset,
replacement/removal and owner teardown invalidate or release the cache.
Existing source checks and the 8192-pixel-per-axis viewport limit remain.

`tools/gtk4/test-xtext-background-cache.c` verifies cross-frame reuse and
independent invalidation for each key field, plus source reset/removal.

### Divider Measurement — Source-Verified

`fabulor_gtk_paned_get_handle_size()` in `src/fe-gtk/gtk-compat.h` now measures
GTK4's separator CSS child rather than combining a pending pane position with
an old child allocation. The previous calculation reported a 1-pixel divider
as 151 pixels immediately after a position change from 800 to 650, allowing
that error to feed back into pane restoration.

`tools/gtk4/test-paned-handle-size.c` reproduced the pre-fix failure. After the
fix, four native cases passed: horizontal and vertical panes with normal and
wide dividers. They check pending positions, allocated gaps, hide/show and
restored end-child size. No hard-coded divider thickness was introduced.

### Credits — Installed-Verified

The maintainer reported that the initial local v1.0.7 upgrade completed
successfully and confirmed PickleRick83 and MrBenC in About → Credits → Testers.
This verifies that upgrade and credit display, not a complete installer matrix.

## Installed Timing Observations

The maintainer supplied opt-in UI timing captures in three stages: original
behaviour, a temporary background-image removal comparison, and the installed
fixes with the background image restored. Approximate timings below describe
those samples, not a controlled benchmark or guaranteed performance target.

| Observation | Before corrections | After corrections |
| --- | --- | --- |
| Repeated populated transcript paints | Usually 60–67 ms with the image | Usually 14–20 ms at steady viewport size |
| Nearly empty channel paints | About 49–54 ms with the image | About 1.6–2.1 ms at steady viewport size |
| Temporary narrow transcript widths | Examples at 235, 386 and 535 pixels | Not present in the final supplied capture |
| Expected server/channel layouts | Intermediate narrow layouts occurred | Direct changes between 1136 × 560 and 985 × 529 pixels in the final capture |

Removing the background image before the cache fix reduced populated paints to
approximately 17–23 ms and nearly empty paints to approximately 2–3 ms. That
comparison helped isolate background scaling; disabling the image was a
diagnostic step, not the permanent solution.

The final supplied capture began at 14:22:23 +1100. Its steady-size paints
retained the improvement, while legitimate server/channel size changes could
still cause a first paint of approximately 50–70 ms. The implementation retains
one cached size, not multiple layouts. Occasional user-list and rewrapping
spikes and some repeated full-viewport paints also remain observable.

After reviewing the changes in daily use, the maintainer reported a
"definite improvement" and subsequently stated: "IMHO I'd call this fixed."
The reported issue is therefore **closed by maintainer acceptance**. Further
multi-size caching was discussed but not implemented or required for closure.
This does not claim that every paint or switch meets a fixed latency budget.

## Automated Build And Package Evidence

The local pane-fix build recorded:

- Native Release x64 client build passed; common native suite: 53 passed,
  two external-fixture tests skipped.
- Meson targets passed: existing GTK4 dependency/rendering probe, background
  cache regression test and pane-handle regression test (four cases).
- Python source-contract checks: 46 passed, covering UI profiling and the
  production profile. These are not timing benchmarks.
- Clean MSI/bootstrapper rebuild: zero warnings and errors.
- Production MSI validation: 2,630 installed files, no legacy GTK3 payload.
- GTK4 runtime validation: 1,160 manifest entries, 1,161 installed entries,
  content hashes verified.
- Bootstrapper's embedded MSI matched the standalone MSI byte-for-byte.
- Product metadata validation passed for launcher, frontend, setup and
  bootstrapper application, all version 1.0.7.

The tested local pane-fix MSI SHA-256 was:

```text
b408d3558ec25a14185c236e766e7df699ca9d03bd22165476e86ee52289e3c2
```

This identifies that local package only; it is not a published release checksum
or a publisher signature. Ignored working evidence was retained beneath
`build/local-1.0.7/`, including build notes, regression logs and package
validation output. The tracked record does not depend on those local files
being distributed.

## 2026-10-23 — Private Name-Layout Restoration

**Source-verified and automated-tested; reported regression accepted as fixed.**

After receiving the combined candidate, the maintainer confirmed on 2026-10-23:
"TY. Fixed." This records installed acceptance and closes the reported lost
name-clipping regression. It does not establish explicit acceptance of every
hover/click/copy, background or display-scale scenario below.

The maintainer reported that their earlier private v1.0.7 client correctly
clipped long nicknames, whereas the recent main-checkout rebuild drew them over
timestamps. The private implementation was found as uncommitted changes in
an earlier development worktree, with private installer records under
`build/private-name-layout-1.0.7/`. It had not been included in
the main checkout used for the recent rebuild. This was a lost build feature,
not evidence that the private feature had never existed.

The private source implementation and native tests were imported into the main
checkout, leaving the original worktree untouched. This restores available-width
ellipsis rendering, full-name tooltips and identity-preserving hit testing,
user-list/channel-label clipping, bounded automatic indentation, the private
Windows tooltip-position correction and the stable background-palette fix.
The implementation does not impose a fixed 16-character limit. The accepted
background-cache and divider fixes, profiling and tester credits remain.

Automated evidence for the combined build:

- Native Release x64 client build passed; common suite: 53 passed, two
  external-fixture skips.
- All three Meson targets passed: GTK4 probe (including private name-layout
  checks), background-cache test and pane-handle test.
- 72 Python checks passed: 63 integration/profiling/production/DPI/background
  checks, six bundle tests and three legal-bundle tests. The legal test now
  derives its expected SBOM filename from generated version metadata rather
  than hard-coding 1.0.6.
- Clean MSI/bootstrapper rebuild passed. Production payload validation found
  2,630 installed files and no legacy GTK payload. Runtime hashes verified
  against 1,160 manifest entries and 1,161 installed entries.
- Embedded MSI matched the standalone MSI; all four product metadata checks
  passed at version 1.0.7. Installer definitions, feature selection and upgrade
  identities were not changed.

Local unsigned candidate:

```text
build/local-1.0.7/name-layout-restore/FabulorSetup-v1.0.7-name-layout-restored.exe
SHA-256: d89cd46d3c535b04664fb1dace924e33040036944d09a1ae7e7f06ae0368d75f

build/local-1.0.7/name-layout-restore/Fabulor-v1.0.7-name-layout-restored.msi
SHA-256: 78045bda5c3fa41dfedd7d8af549042b866f43f2ca29d525be4d5941312c05e0
```

Build scripts, logs, source snapshots and the preceding installer copies are
retained beneath that directory. The source snapshot there contains the
pre-integration tracked diff and the imported private-source patch. The earlier
private installers remain untouched. No installation, commit, push or publication
was performed by the agent.

The candidate was supplied with instructions to close Fabulor, back up the
profile and upgrade without clearing scrollback. Suggested checks covered long
names beside timestamps, full-name hover/click/copy behaviour, user-list and
channel labels, 250%/300% tooltip positioning, background dimming/colour stability
and the previously accepted channel-switching improvement. The maintainer's
acceptance above closes the reported regression; the individual scenarios were
not separately reported and do not replace the broader release-validation matrix.

## 2026-10-23 — Unified GTK Emoji Candidate

**Automated checks passed, but installed flag rendering failed acceptance.**

The maintainer supplied `https://imgur.com/a/Qw8lmnA`; its image still shows
country entries as letter pairs rather than graphical flags. The installed
frontend DLL and GTK runtime hashes match the candidate/build copies, and the
installed bundled emoji font matches the source font. The flag fix is not
accepted. The maintainer subsequently confirmed the same letter-pair result
from both entry points.

The original probe inspected the chooser's ancestor font context rather than
the actual flag-label layouts. It has been strengthened to inspect the labels
and their selected font families, including a simulated high-priority font
rule. Those local tests still pass; they do not explain or override the
installed screenshot. The later backend-specific reproduction and follow-up
candidate are recorded below.

The maintainer requested one GTK-style picker from Ctrl+; and the smiley icon,
with graphical flags and no loss of the legacy country's selection. Rather
than replace GTK's private flag widgets, the candidate selects the already
bundled Noto Color Emoji font within a chooser-scoped CSS class. It retains the
actual GTK interface and catalogue; graphical flags are font glyphs, not PNG
substitutions. Runtime payload and installer definitions were not changed.

The native probe checks all 259 country-flag sequences in the pinned GTK
catalogue: each shapes as one glyph without unknown glyphs using the chooser's
actual styled context, and each is present after the chooser populates. A
comparison with the legacy `mg_emoji_flag_codes` array found all 253 legacy
country codes represented. The artwork directory also contains subdivision
PNGs that the legacy country-code picker did not expose; no broader promise
of PNG catalogue equality is made. GTK search terms are retained, so country
code aliases should not be assumed.

Both shortcuts are captured before GTK's editable delegate; GTK 4.22's shortcut
action bypasses its legacy `insert-emoji` signal. Direct signal emissions are
also handled. Native checks cover repeated binding, shared chooser reuse,
Ctrl+;/Ctrl+. controller handling, unrelated-key propagation, selected-text
replacement, Unicode cursor placement, read-only input and teardown. These
checks do not substitute for physical keyboard and installed visual acceptance.

The Release client build, all four Meson targets, 72 Python checks, clean
MSI/bootstrapper rebuild, runtime hashes, production payload, embedded MSI
identity and four-file version metadata validation passed. Prior nickname,
cache, divider and credits changes remain in this candidate. Its distinctly
named files do not replace the accepted name-layout installer copies:

```text
build/local-1.0.7/emoji-investigation/FabulorSetup-v1.0.7-unified-emoji.exe
SHA-256: cf260ac09ae461216f5700d7cd2b1e20dec261324f2c26ada25212b3dd911c55

build/local-1.0.7/emoji-investigation/Fabulor-v1.0.7-unified-emoji.msi
SHA-256: 13d5f15939fff08fc32e04086b33e8c08d2aab263fdaac8570a648dee01586a9
```

Close Fabulor and back up the profile before upgrading. Required maintainer
checks: smiley icon and both shortcuts show the same interface, country flags
are graphical (including previously missing entries), search/recent items and
variants work, insertion replaces only the intended selection or uses the
cursor, Escape dismisses the picker, and themes/250%/300% sizing remain usable.
A screenshot of the installed Flags category will help confirm appearance.
The legacy custom builder remains as a fallback during acceptance. No installed
GUI acceptance, commit, push or publication was performed by the agent.

## 2026-10-23 — Emoji Font-Map Follow-Up

**Reproduced automatically; corrected picker accepted by the maintainer.**

After supplying the graphical-flags screenshot and being asked to check flag
insertion, search and Escape, the maintainer confirmed on 2026-10-23:
"Yes. All works well. Very well in fact." This records installed acceptance
and closes the reported unified-picker/flag-rendering issue. It does not
establish exhaustive coverage of every flag, variant, theme or display scale.

The maintainer supplied `https://imgur.com/a/7ZUNMhK` after receiving the
font-map follow-up. Its screenshot shows the GTK Flags category displaying
country flags graphically, including Australia, instead of the earlier letter
pairs. This is installed visual evidence for the visible flags and layout, not
explicit acceptance of all 259 entries, both access paths, search, insertion,
Escape or the full display-scale matrix.

Running the strengthened probe with `PANGOCAIRO_BACKEND=win32` reproduced the
letter-pair problem: all 259 country flags selected Segoe UI Emoji and shaped
two glyphs. The original test forced Fontconfig and therefore did not cover
this backend difference. This is a reproduced failure mode, not an inspection
of the running client's internal Pango state.

The follow-up explicitly sets a Fontconfig/FreeType font map on the chooser
using GTK's public widget API. The private bundled Noto font is then available
regardless of the default backend, without changing transcript/input font maps.
The chooser-scoped CSS also targets descendants above the existing user-level
palette priority. No private GTK child restructuring or runtime payload change
was introduced.

Tests now inspect each actual flag label's shaped runs and selected font family,
not just an ancestor context or glyph count. They allow frame-clock updates
after catalogue population and run with both Win32 and Fontconfig defaults.
Both runs pass: all 259 country flags are present, select Noto Color Emoji,
and shape one supported glyph. The Release build, all five Meson cases,
72 Python checks, clean installer build, production/runtime payload validation,
embedded MSI equality and four-file product metadata validation passed.

```text
build/local-1.0.7/emoji-fontmap-fix/FabulorSetup-v1.0.7-emoji-fontmap-fix.exe
SHA-256: c4c36bc7ffe4d5ad8eae5cf37a90b21cde30027de71196cf2f994ce8d5297492

build/local-1.0.7/emoji-fontmap-fix/Fabulor-v1.0.7-emoji-fontmap-fix.msi
SHA-256: d548362fb4f9a96f37ba105ffc05073cc4c3b1feaa1409cb53337277477f98ad
```

The first emoji and accepted nickname-fix installer copies remain preserved.
No installation, process restart, commit, push or publication was performed by
the agent. Graphical rendering of the visible country flags is now supported
by the maintainer's screenshot above, followed by their acceptance of the
picker's operation. Exhaustive flag, variant, theme and display-scale coverage
remains part of the broader release-validation work, not a reason to reopen
this accepted issue.

## 2026-10-23 — DALnet DCC Permissions Candidate

**Source-verified, automated-tested and accepted by the maintainer.**

After receiving the candidate and instructions to check DALnet/ZNC visibility,
absence on other networks and allow/list/revoke server responses, the maintainer
confirmed on 2026-10-23: "It works as expected. :)" This records installed
acceptance of the requested feature. It does not establish separately reported
coverage of every multi-selection, disconnect/origin-close or tab-switch case.

The nickname menu adds **DCC Permissions (DALnet)** only when DALnet is
recognised by its exact configured name, the server-reported `dal.net` domain,
or the advertised `NETWORK=DALnet` value retained on the server session. This
handles renamed/ZNC profiles when the relevant identity is retained, not every
opaque bouncer configuration. Recognition is a UI hint, not authenticated
proof of network identity. Domain matching rejects lookalikes such as
`evildal.net` and `dal.net.example`.

Explicit actions request `DCCALLOW +nick`, `DCCALLOW -nick` and `DCCALLOW LIST`.
They do not grant permission automatically or claim that a grant succeeded
before the server replies. The allow action prints a warning that permission
bypasses blocked-file-type restrictions, does not make files safe and does not
enable automatic acceptance. It describes the documented temporary lifetime.
Reference: <https://docs.dal.net/docs/misc.html#3>.

The new dispatch path uses the captured origin session and unique server ID,
validates session liveness, connection state and network at activation, and
sends one validated raw IRC line rather than interpolating a local command.
It does not use the currently focused session. Allow/revoke are disabled for
multiple selections; list remains available. No server permission state is
assumed or shown as a checkbox. Existing `popup.conf` entries are not changed.
Unrelated legacy menu commands retain their existing dispatch behaviour.

Validation passed: Release client build, six Meson cases (including three
DALnet recognition/command/model tests), the existing 72 Python checks plus
three new source-integration checks, clean MSI/bootstrapper build, production
payload/runtime hashes, embedded MSI equality and four-file version metadata.
No warnings/errors were reported in the native or installer build logs.
Production payload remains 2,630 files with no legacy GTK runtime. Installer
feature selection, upgrade identities and profile handling are unchanged.
The security review found no obvious blocker in the new explicit-action path;
connection-switch behaviour is source-checked, not live-network-tested.

```text
build/local-1.0.7/dalnet-dcc-permissions/FabulorSetup-v1.0.7-dalnet-dcc-permissions.exe
SHA-256: 8e77e26a5f5c3b8f37f7d4c5e872f02c72c77cf9a1c4b8440993995d3e239f0e

build/local-1.0.7/dalnet-dcc-permissions/Fabulor-v1.0.7-dalnet-dcc-permissions.msi
SHA-256: dafe079c1acd20fd3ae9de6c182081767edd94cdfbb92004f41222425556f29d
```

The accepted emoji and nickname/performance fixes remain included; their prior
installer copies are preserved. No install, permission grant, file transfer,
commit, push or publication was performed by the agent. The maintainer's
acceptance above closes the requested feature. Exhaustive multi-selection,
tab-switch/disconnect and origin-close coverage remains separate validation;
no executable transfer was required or reported.

## 2026-10-29 — Services Timestamp/Prefix Candidate

**Source-verified and automated-tested; focused installed feedback is positive.**

After receiving this candidate, the maintainer reported: "That seems to have
worked. :)" The reported Services timestamp/prefix overlap therefore appears
resolved in installed use. This preserves the tentative scope of that report;
it does not claim exhaustive blank-line, selection, scrolling or redraw tests.

The maintainer supplied
<https://imgur.com/gallery/services-tab-bug-d4j37Qp#U1pb6hg>. The screenshot
shows long ChanServ notice prefixes clipped beside message text, with some
timestamps incorrectly drawn over the prefix column. This is distinct from
service-message routing or the previously fixed duplicated outgoing echo.

With timestamp copying enabled, `gtk_xtext_render_stamp()` temporarily assigns
`ent->str = text` but retains the original message's `left_len`. The new
prefix-clipping path used `offset == 0` without checking the existing
`left_only` flag. It could therefore measure a short timestamp using a long
service prefix length, reading past the timestamp string and positioning its
rendering as a nickname. The fix short-circuits prefix geometry with
`!left_only`. Timestamp copying, stored message identities, service routing and
normal nickname clipping remain unchanged.

`tools/gtk4/test_xtext_timestamp_prefix_guard.py` adds three source contracts.
The guard check failed against the pre-fix source and passed afterwards.
These checks do not constitute a native pixel reproduction of the screenshot.
The Release client build and 78 unique Python checks passed (72 existing plus
three DALnet integration checks and three new timestamp contracts). The initial
full native run passed all six cases. A later parallel validation repetition
failed the Win32-default emoji probe with font-fallback/teardown warnings;
its cause is not established. The full suite passed serially, and the serial
validator then verified the production/runtime payload and embedded MSI. Both
the failed log and serial pass are retained locally; this is a separate test
reliability follow-up, not evidence that the timestamp guard fixed that probe.

The clean installer rebuild and four-file product metadata checks passed.
Payload remains 2,630 installed files, no legacy GTK payload; runtime hashes
match the manifest. Installer definitions/features/upgrade identity were not
changed. All previously accepted local feature changes remain included.

```text
build/local-1.0.7/services-timestamp-fix/FabulorSetup-v1.0.7-services-timestamp-fix.exe
SHA-256: 4f96c1552146841b423262d0ed44693f81834951447f82cb5753061c315c5d61

build/local-1.0.7/services-timestamp-fix/Fabulor-v1.0.7-services-timestamp-fix.msi
SHA-256: 77c28acf749d84f3033b8b6190b393b8d047219a136aa5551cb21ef388233ca8
```

Prior installer copies remain preserved. No installation, profile-setting
change, commit, push or publication was performed by the agent. Required
maintainer checks: repeat the ChanServ help output in Services with timestamp
copying still enabled, including blank lines, selection, scrolling and redraw;
confirm timestamp/prefix separation and ordinary nickname clipping. The
maintainer's focused positive feedback is recorded above; individual scenarios
were not separately reported.

## 2026-10-29 — Windows Sysinfo Candidate

**Source-verified, native-tested and accepted by the maintainer.**

After receiving this installer and the requested local-output command checks,
the maintainer confirmed: "Perfect 💯". This records installed acceptance of
the requested Sysinfo improvements, not separately reported coverage of every
command, summary preference, full-disk condition or driver configuration.

The maintainer confirmed `/SYSINFO -e SOUND` failed while `STORAGE` reported
plausible usage. Inspection found SOUND, ETHERNET and CHIPSET were unconditional
NULL stubs. This candidate implements them via the existing local WMI backend:

- SOUND: `Win32_SoundDevice.Name`.
- ETHERNET: `Win32_NetworkAdapter.Name`, filtered to physical adapters (including
  Wi-Fi). No IP, MAC, serial number or network-configuration fields requested.
- CHIPSET: system PnP controller names matching chipset, SMBus, host bridge,
  LPC or ISA bridge terms. These are driver-provided controller names, not a
  definitive chipset-model inference; a non-match is legitimately unavailable.
- RAM maps case-insensitively to the existing MEMORY command. Existing
  `hide_memory` preferences and the summary entry remain unchanged.

Full fixed disks now contribute their capacity and zero free space. Strict
unsigned-value conversion distinguishes real zero from null/malformed/overflow
values; inconsistent and overflowing totals are rejected. WMI enumeration has
bounded rows/output and two-second per-Next waits, rejects failed/empty name
results and releases enumerators/builders on failure. This is not a guarantee
that every Windows COM/provider call has a bounded total duration. Provider
strings have ASCII controls removed before being formatted as IRC output.
Unavailable results name the requested category rather than implying a known
hardware fault. A small disk-query allocation leak is also corrected.

Validation:

- Release client/native-plugin build and clean installer rebuild passed, with
  no native/installer build warnings or errors reported.
- Seven Meson cases passed, including the new backend executable's three
  native tests: synthetic full/malformed/inconsistent/overflowing WMI disk
  rows, unsigned variants, and UTF-8/control-character handling.
- 81 Python checks passed: 72 existing, three DALnet integration contracts,
  three timestamp contracts and three new Sysinfo integration contracts.
- A standalone probe of the actual backend queried the local machine with
  the same COM/security initialisation as the client. Sound, Ethernet,
  chipset, CPU and GPU were all available. Its log records availability only,
  not hardware names or identifiers. An initial harness probe without the
  client's COM security initialisation was unavailable; correcting the harness
  restored results. This does not establish the cause of the earlier GPU issue.
- One serial validation repetition failed the already-tracked Win32 emoji
  probe with font-fallback/teardown warnings. The initial full run and final
  serial repetition passed all seven cases. Failed and successful logs remain
  preserved; the intermittent probe is still an open follow-up, not fixed by
  this work and not attributable solely to parallel execution.
- Production payload/runtime hashes, embedded MSI equality and four-file
  version metadata passed. Payload remains 2,630 files with no legacy GTK
  runtime. The built Sysinfo DLL contains the three new WMI query classes.

```text
build/local-1.0.7/sysinfo-fix/FabulorSetup-v1.0.7-sysinfo-fix.exe
SHA-256: ca190ca3b574662a4ab9061fe7f4dd3dc343ee8fe02937e5431614c47bfdafa4

build/local-1.0.7/sysinfo-fix/Fabulor-v1.0.7-sysinfo-fix.msi
SHA-256: a27cca48a2c6af1518bd5d295fa948b549190a9d0573ee6c4b2d413b63384c82
```

Review conclusions:

- **Security surface/verdict:** local WMI provider data and user-directed IRC
  formatting; no obvious security blocker. Queries are fixed strings on the
  local namespace; no subprocess, credential collection, new automatic upload
  or plugin trust-policy change. Names remain potentially identifying, so
  maintainer testing should use `-e`; existing announcement defaults and
  category-hidden defaults are unchanged. Provider/type/formatting robustness
  is covered by the native tests, not a general security certification.
- **Installer surface/risk:** rebuilt payload only; low packaging risk. No
  installer definitions, identities, optional-feature choices, profile handling,
  runtime isolation or signing/publication boundaries were changed. Existing
  plugin ABI/host permissions remain unchanged; new core getters are internal.
- **Documentation claims:** command wiring and limitations are source-verified;
  backend availability is native-verified, with subsequent installed-candidate
  acceptance recorded above. Individual command/driver scenarios were not
  separately reported. No release-readiness claim follows from these checks.

Prior accepted fixes and installer copies remain preserved. No profile setting
change, installation, IRC announcement, commit, push or publication was performed
by the agent. The maintainer's acceptance above closes the requested feature.
Exhaustive command/driver and summary-preference coverage remains separate.
Do not fill a real disk to test zero free space; native synthetic rows cover it.

## 2026-10-29 — Optional Spell Auto-correct Candidate

**Initial candidate: installed feedback reported no correction of `teh`.**

**Correction to the original test report:** the initial probe described below
was not read-only/session-only. Upstream WinSpell mapped session additions to
persistent Windows `Add`; the probe unintentionally added `teh` to Windows'
shared custom dictionary. The resulting valid-word veto explains the installed
report. See the follow-up section for exact-word recovery, the provider patch,
guarded test isolation and stronger real-key integration evidence. Installed
acceptance of the replacement is recorded in the follow-up section below.

The maintainer approved an opt-in correction-on-Space design with immediate
Backspace undo and protected IRC input. The new persistent preference is
`gui_input_autocorrect`, default zero. Preferences > Interface > Input box
provides **Auto-correct while typing**, with a requirement for spell checking
and an active English dictionary. Existing profiles are not opted in.

The implementation uses a small curated common-English typo table rather than
blindly selecting a dictionary's first suggestion. Only a fully keyboard-typed
lower-case/initial-capital ASCII-letter token is eligible. Existing word-check
vetoes must allow it, every active dictionary must explicitly reject the
original as misspelled (errors veto), and an English dictionary must accept the
replacement. This respects other-language valid words, personal dictionaries,
session ignores and missing/erroring backends. No automatic replacement training
is performed. The frontend additionally excludes known nicknames on the input
box's originating connection, the user's nickname and a dialogue peer; this
also suppresses their spell underlines. Unknown nickname-shaped words cannot
be recognised reliably and remain subject to the opt-in/undo policy.

Captured key events arm expected edits without consuming Space. The exact
native insertion must occur before a deferred replacement may run; text,
cursor, selection, editability, visibility, length, context and approval checks
prevent stale edits. Character-indexed word edits preserve Unicode prefixes
and suffixes. Return/keypad Enter cancels a pending correction instead of
triggering one. Immediate Backspace restores the original token and removes
the triggering space; provenance is cleared to prevent immediate reapplication.

Paste/history/programmatic edits, IME preedit, focus loss, pointer clicks and
tab restoration invalidate the relevant state. Command lines (slash or custom
command character), punctuation-bearing link/email/channel/mention/path/code
shapes, all-caps/mixed-case tokens and IRC-formatted lines are excluded. Other
misspellings retain manual underlines/suggestions. This initial implementation
is common-English typo correction, not general multilingual auto-correct.

Evidence:

- Release client build and clean MSI/bootstrapper rebuild passed, without
  native/installer build warnings or errors reported.
- Eight Meson cases passed, including eleven new native policy/widget cases:
  protected tokens, correction/undo/no instant reapply, disabled/veto states,
  paste/partial pasted words and Unicode positions, simulated composition,
  Enter/tab/disposal cancellation, cursor/disable invalidation, length/read-only/
  selection/visibility guards, fast typing, middle-of-line preservation and
  approval reentrancy, plus detachment/retained-controller lifetime.
- These tests emit controller signals and perform public GtkEditable edits.
  They are not real Windows input-event, keyboard-layout or IME acceptance.
- 87 Python checks passed: 72 existing checks, three DALnet, three timestamp,
  three Sysinfo and six new auto-correct integration contracts. The source
  contracts cover preference defaults/application, dictionary and command
  guards, originating-session nickname vetoes and cancellation/training policy.
- The initial Enchant/WinSpell probe confirmed the three example word pairs
  before testing session ignore. Its assertion that this was in-memory only
  was incorrect: the provider's hook persisted `teh` in Windows' neutral custom
  dictionary. No Fabulor preference was changed by that probe, but the Windows
  dictionary side effect occurred. This probe did not instantiate the full
  spell widget; it is now explicitly disabled and replaced by guarded tests.
- Production/runtime payload hashes, embedded MSI equality and four-file
  version metadata validation passed. Payload remains 2,630 files with no
  legacy GTK runtime. All accepted local fixes are retained.
- The previously tracked intermittent Win32 emoji probe did not fail these
  final runs; that does not close its reliability follow-up.

```text
build/local-1.0.7/spell-autocorrect/FabulorSetup-v1.0.7-spell-autocorrect.exe
SHA-256: a5227681a68808033cb303ba33b982ba2dd98b27493c5f3fc67533cdb3344abc

build/local-1.0.7/spell-autocorrect/Fabulor-v1.0.7-spell-autocorrect.msi
SHA-256: 385b8cc3fb95773d46439733d9970f7ee393e10eb758cdf89cedcfe06c1891be
```

Agent review conclusions (not independent release approval):

- **Threat surface/verdict:** local draft text, dictionary-provider decisions,
  keyboard/IME events and eventual user-directed IRC transmission; no obvious
  security blocker found. The feature defaults off, excludes command lines,
  vetoes dictionary errors and never initiates a send. Input snapshots are
  temporary entry-owned memory, not a new log or learned-word store. There is
  no subprocess, remote correction service, telemetry or new network request.
- **Hardening/manual follow-up:** synthetic IME events cannot establish all
  platform composition behaviour. Real keyboard layouts, IMEs, accessibility
  input, shortcut interactions and preference persistence require installed
  acceptance. This is not certification of arbitrary third-party GTK callbacks.
- **Installer surface/risk:** rebuilt application payload only; low packaging
  risk. No installer identity, feature-selection, profile migration, private
  runtime, signing or publication policy was changed. The internal preference
  structure and dependent native binaries were rebuilt; the public plugin API
  was not extended or changed.
- **Documentation claims:** preference/behaviour/limitations are source-verified;
  synthetic widget and dictionary behaviour are automated-verified. Installed
  acceptance and broader release readiness are not inferred.

Prior accepted installer copies and the preliminary pre-strict-guard build
are preserved. No installation, IRC message, Fabulor preference change, commit,
push or publication was performed by the agent; the unintended Windows
word-dictionary write is corrected and documented in the follow-up below.
Requested maintainer checks:
confirm default off, enable through Preferences, type (do not paste) `teh` then
Space, undo immediately with Backspace, try `adn`/`recieve`, and verify URL/paste/
command/tab safety. Separate persistence, personal/ignored-word and real Windows
keyboard/IME/accessibility scenarios remain part of the broader matrix.

## 2026-10-29 — Spell Session-Isolation Follow-up

**Initial installed failure diagnosed; replacement automated-tested and
accepted by the maintainer for the reported correction issue.**

The maintainer reported that `teh` and `teh dayz` were not corrected. Saved
preferences showed both spell checking and auto-correct enabled with `en_AU`.
An isolated real-Windows-key test passed in the plain owner widget. The actual
production spell entry, however, classified `teh` as valid, so its conservative
veto correctly prevented replacement.

Root cause: Enchant 2.8.19's WinSpell `add_to_session` hook calls Windows'
persistent `ISpellChecker::Add`, and its removal hook calls persistent Remove.
The agent's earlier purported session-only probe had therefore added `teh` to
`Microsoft/Spelling/neutral/default.dic`. This was an unintended test side
effect, not a user configuration mistake or an auto-correct key-routing bug.

Recovery removed only the exact agent-added word through `ISpellChecker2::Remove`.
Before/after exact counts were one/zero, and hashes/counts of all other non-empty
dictionary entries matched. No bulk dictionary clearing was performed. Other
historical/custom words were left untouched. The old unsafe probe scripts now
fail immediately rather than performing word-changing tests.

The reproducible local recipe patches a build-directory copy of WinSpell to
leave both optional session hooks null. Enchant core already owns per-dictionary
session include/exclude tables, so no Windows-global word mutation is needed.
The checksum-controlled upstream archive and extracted provider source remain
unchanged; this is a documented downstream patch, not pristine upstream binary
behaviour. Explicit personal additions still persist through Enchant's personal
word files. No automatic migration/deletion of historical Windows words is added.
The component inventory/SBOM identifies the downstream distribution as
`2.8.19+fabulor-session-local.1`; its shipped patch notice supplements the
unchanged LGPL evidence and the core still reports upstream version `2.8.19`.

`tools/enchant-msvc/smoke.c` now preflights the actual provider's null hooks before
any word-changing test. It requires and validates an explicit isolated
`ENCHANT_CONFIG_DIR`, tests both session inclusion and exclusion against fresh
dictionaries, and tests personal persistence only in that isolated directory.
The build script uses a unique directory and restores environment overrides.
`XDG_CONFIG_HOME` alone did not isolate Enchant on Windows. A negative control
with the original provider exits 10 before word tests and leaves the isolated
directory untouched.

Evidence:

- Patched Enchant build/smoke, Release client build and clean installer rebuild
  passed. The actual production spell-entry objects (with inert theme services,
  no IRC session) received real Windows keys in their verified foreground test
  window: `teh dayz ` became `the dayz `. The real Ignore action then ignored
  the typo only in that entry; a fresh entry still rejected it.
- No Return/send key is injected. The probes abort if another window has focus
  or modifier/Caps Lock state makes the synthetic input unsafe/unrepresentative.
- Windows and real Enchant dictionary-file hashes remained unchanged throughout
  the patched build, guarded smoke tests and full-entry tests, after the
  separately documented exact-word recovery.
- Eight Meson cases and 92 Python checks passed, including five new provider/
  smoke-isolation contracts. Eleven native auto-correct cases still pass;
  their fast-typing assertion now explicitly preserves `dayz`.
- Production payload and GTK runtime validation passed, plus embedded MSI
  equality and four-file version metadata. Explicit MSI extraction verified
  Enchant core/provider/ordering hashes against the rebuilt stage. The patched
  provider SHA-256 is
  `82c9849683d23f3746f43819e2936bf7fd45a9109f4585c405769c3c08961a39`.
  Extracted patch-notice, regenerated SBOM and notices hashes also match the
  staged legal bundle. Payload is now 2,631 files (one added downstream patch
  notice), with no legacy GTK runtime.

```text
build/local-1.0.7/spell-autocorrect-real-keys-fix/FabulorSetup-v1.0.7-spell-session-fix.exe
SHA-256: 72c3ea7813c01818c9785d9af6b5e0b14d50f4774838802e2b72c75f07bacd4c

build/local-1.0.7/spell-autocorrect-real-keys-fix/Fabulor-v1.0.7-spell-session-fix.msi
SHA-256: b6fbef7be62ef00e3373751cae410e7f0941c32667f120f6b4ed635f0bd8c1a6
```

Review: the surface is local word/dictionary state, not a demonstrated remote
execution or exfiltration issue. The unintended Windows-global mutation needed
a fix; the patched hooks, guarded tests and exact recovery address the identified
path, with no obvious remaining security blocker in that path. Installer risk
is medium for the changed third-party provider bytes, mitigated by the pinned
source/build-local patch, runtime smoke and extracted-MSI hash checks. Installer
identity, optional features, profiles, plugin ABI, signing and publication gates
are unchanged. There is still no general Windows IME/accessibility certification;
focused installed acceptance is recorded below.

Requested maintainer retest: close Fabulor, upgrade, then type `teh` followed by
Space, check immediate Backspace undo, and type `teh dayz ` to confirm only the
first word changes. `dayz` is deliberately not guessed as `days`. The original
candidate and accepted installers remain preserved. No installer was run, IRC
message sent, Fabulor preference changed, commit created or publication performed
by the agent; the targeted Windows-dictionary recovery is explicitly recorded.

**Installed feedback:** after receiving this replacement installer and the
correction/undo retest instructions, the maintainer replied **"Perfect 💯"**.
This records acceptance of the reported auto-correct fix and the replacement
candidate. It does not establish separately reported results for each example,
Backspace sequence, preference persistence, personal/session dictionaries,
commands, nicknames, paste/history, tab changes, real IME or accessibility.
Those broader checks remain separate; the initial candidate's failure and the
test side effect above are retained in the record.

## 2026-10-30 — Windows-preferred Auto-correct Coverage

**Source-verified and automated-tested; focused installed feedback is positive.**

Later maintainer feedback reported that the feature corrected `teh` but missed
`sleke`, `btter` and `turkey`/`Turkey`. The earlier “Perfect 💯” feedback is
retained as acceptance of the session-isolation/`teh` fix, not proof of general
typo coverage. The fixed 20-pair list was the limitation. The maintainer explicitly
approved using Windows' preferred suggestion even when alternatives exist.

A read-only `ISpellChecker::Check`/`Suggest` probe of this machine's `en_AU`
checker returned `sleek` first for `sleke`, `better` first for `btter` (also
batter/bitter/butter), and `Türkiye` for both `turkey` and `Turkey`. No Add,
Remove, Ignore, AutoCorrect or settings setter was used by that diagnostic.

Implementation removes the production typo table. At an eligible Space boundary,
`sexy-spell-entry.c` requests the first available suggestion from the first
active English WinSpell dictionary in configured order. Enchant forwards to
Windows' `ISpellChecker::Suggest`, preserving provider order while filtering
excluded suggestions. Fabulor does not rerank or skip to lower-ranked words.
The original must still be rejected by every active dictionary and the existing
word-check/nickname veto; the selected dictionary must accept the replacement.
Errors, missing providers, unsuitable output or no suggestion leave text unchanged.

Input provenance, default-off preference, commands, paste/history, URL/code/
formatting, mixed-case/acronym and IME protections remain. Eligible input is
1–64 fully typed ASCII letters in lower case or initial-capital form. Unicode
and ordinary multiword replacements are supported, with bounded output and no
controls/commands/URLs. Windows' case/diacritics are preserved. Correction is
deferred until native Space insertion settles; Enter cancels rather than flushes
pending work. Immediate Backspace uses Unicode character counts, restores the
original and removes the space; another Space does not immediately reapply.
Automatic corrections do not train or persist word pairs. Unknown alphabetic
nicknames and ambiguous words can still receive an unintended preferred suggestion.

Validation:

- Release client build and installer rebuild passed without reported warnings
  or errors. Fourteen native policy/widget cases passed, including Unicode
  output/cursor/undo in the middle of Unicode text, multiword output, invalid
  suggestions, length limits and resolver reentrancy. Six auto-correct source
  contracts and five existing provider-isolation contracts passed; total Python
  regression coverage remains 92 checks.
- Eight Meson suites passed serially. This passing run does not resolve the
  separately recorded intermittent Win32 emoji probe issue.
- Real Windows keys delivered only to the verified foreground test window,
  using the actual production spell-entry objects and inert theme services,
  transformed `teh sleke btter turkey ` into `the sleek better Türkiye `.
  Real Backspace restored `turkey` and removed the space; Space did not reapply.
  The real Ignore action remained entry-local and a fresh entry rejected `teh`.
  This probe loads no IRC session or Fabulor profile and injects no send key.
- Guarded tests used isolated Enchant config and preflighted the patched
  provider. Before/after hashes of real Windows/Enchant word files matched.
  No dictionary recovery or user-word mutation was needed for this follow-up.
- Production payload/runtime and embedded-MSI validators passed: 2,631 files,
  no legacy GTK; runtime content hashes verified. Four-file version metadata
  passed. Explicit MSI extraction matched the new frontend and launcher,
  unchanged patched Enchant payload, notices and SBOM against their stages.

```text
build/local-1.0.7/spell-autocorrect-coverage/FabulorSetup-v1.0.7-windows-autocorrect.exe
SHA-256: acbb36a398afaa865855c80936ff3e345788241188ebcc5ba699ce49dbdc4801

build/local-1.0.7/spell-autocorrect-coverage/Fabulor-v1.0.7-windows-autocorrect.msi
SHA-256: 9d676784ef3ae75e784c3cc285d0dc9287b7f81c9a047e6d32249ee18310bfca
```

Review: medium behavioural risk from intentionally broader ranked corrections,
not a confidence guarantee or a hard-coded country substitution. No obvious new
security blocker identified in the local suggestion/edit path; no new cloud
service, subprocess, production word log, dictionary training or file-write path.
Backend call duration is not guaranteed bounded, so long-typing/performance
acceptance remains separate. Installer definitions, identity, features, profiles,
plugin ABI and third-party provider bytes are unchanged by this coverage change.
No installation, profile-preference change, IRC message, commit, push, tag or
publication was performed by the agent. Earlier candidates remain preserved.

Required installed retest: type rather than paste the reported examples, press
Space and check preferred spelling plus immediate Backspace undo. Other Windows
language/user dictionary state may produce different suggestions; accepted
personal words and known nicknames must remain untouched. Full-client preference
persistence, tab/paste/command interactions, IME and accessibility acceptance are
not inferred from the isolated integration probe.

**Installed feedback (2026-10-30):** after receiving this candidate, the
maintainer reported **"This seems to working mopre appropriately. 👍"**.
This is positive, qualified feedback on the broader Windows-based correction
behaviour. It does not establish separately reported results for each example,
Unicode undo, dictionary protection or the broader interaction/platform matrix.
Those checks remain separate; no further code or installer change was made in
response to this feedback.

## 2026-11-07 — Clipped-prefix Message Selection

**Source-verified and automated-tested; focused installed feedback accepts the
reported mismatch as fixed.**

The maintainer reported that right-to-left selection in non-channel/server tabs
needed an unexpectedly large rightward start position to include the message
suffix. The supplied [screenshot](https://imgur.com/d1VKCUN) shows a Services
dialogue with clipped sender prefixes and a partially selected message.
Right-to-left selection itself is normal; the defect is coordinate mapping.

Root cause: `gtk_xtext_append_entry()` clamps a new entry's indent to `MARGIN`.
Clipped-prefix rendering nevertheless resets the body to `buffer->indent`, while
hit-testing walked the full hidden prefix from the clamped original indent.
Later width recalculation can produce a negative indent that happens to align
that old mapping, explaining why geometry/state can affect the symptom.
A sanitized geometry probe hit byte 81 instead of the actual byte-92 end.
A copy of the pre-fix production widget fails the new regression at the body
start (byte 23 instead of byte 37); the corrected widget passes.

`find_x()` now accepts a starting byte clamped to the current subline. Only a
clipped first-line body hit uses `buffer->indent` and `left_len + 1`; prefix/
ellipsis, unclipped channel/server and wrapped-line origins remain unchanged.
The underlying sender/message strings, timestamp-copy guard, selection ordering,
clipboard publication and nick/URL dispatch code are not rewritten. This is a
shared transcript geometry fix, not a special reversal of drag direction or
Services-only routing change.

Validation:

- Nine native cases use the actual GTK widget and append path, then extract
  selections in memory in both directions. They cover freshly clamped and
  recalculated indents, timestamps on/off, ordinary channel/plain-server paths,
  Unicode, colour/bold text, wrapped continuation lines and full-prefix identity.
  Tests do not publish to or overwrite the user's Windows clipboard. Test-only
  configuration/marshaller shims and inert application-service stubs are not
  shipped in the client.
- The baseline widget regression fails; all nine native cases pass with the fix.
  Three new Python source contracts pass. Existing regression groups total
  72 checks; the additional integration group has 23 checks, for 95 Python
  checks overall. All nine Meson suites pass serially. This does not resolve
  the separately recorded intermittent emoji-probe reliability issue.
- A fresh isolated Release frontend/extensions build and MSI/bootstrapper rebuild
  pass without reported warnings or errors. They compile the current dirty
  source tree, including the existing add-on panel/OPER work, without modifying
  that work or replacing it with an older build. The same existing private
  runtime/host stages are reused; all named earlier candidates are preserved.
- Production payload/runtime, embedded-MSI equality, four-file version metadata,
  frontend bootstrap and native-extension import validation pass. The MSI has
  2,631 files and no legacy GTK runtime. Explicit extraction verifies the new
  frontend/launcher and staged Enchant/notices/SBOM hashes.

```text
build/local-1.0.7/selection-hit-fix/FabulorSetup-v1.0.7-selection-hit-fix.exe
SHA-256: b218333057a1e545ecdb6da8d34525144d2da712eafebb9202e916e4217358c5

build/local-1.0.7/selection-hit-fix/Fabulor-v1.0.7-selection-hit-fix.msi
SHA-256: b694a845f5987316f9b5a99a86aec3e87a6102e18a9f532c5874544b103ebaba
```

Review: low-risk functional geometry correction; no obvious new security blocker
identified in the reviewed path. Byte starts are bounded to the subline, full
source identity is preserved, and no new network, profile, credential, logging
or clipboard-publication path is introduced. Installer definitions, product/
upgrade identity, optional feature/private runtime policy and publication gates
are unchanged. No installer was run, IRC command/message sent, profile preference
or dictionary changed, credential provider invoked, commit created or artefact
published by the agent. This is not independent security certification or broad
install/modify/repair/uninstall acceptance.

Requested installed check: close Fabulor and upgrade, then start a right-to-left
drag at the visible end of a line in Services or another affected dialogue tab.
Confirm the whole intended suffix is selected/copied without moving far right,
and sanity-check left-to-right and ordinary channel/server selection. Physical
mouse delivery and actual Windows clipboard contents are not inferred from the
isolated widget regression.

**Installed feedback (2026-11-07):** after receiving this candidate and the
selection/copy retest instructions, the maintainer replied **"That seems to have
fixed that. TY. 👍"**. This records focused acceptance of the reported selection
mismatch as resolved. It does not establish separately reported results for
clipboard contents, both directions, every tab type, wrapping, timestamp mode,
font/display scale or accessibility. Broader coverage remains separate. No further
source or installer change was needed in response to this feedback.

## 2026-11-07 — Outgoing DCC Sender

**Source-verified and loopback-tested; focused installed DCC success reported
on 2026-11-08. Broader external acceptance remains open.**

The maintainer reported failures/timeouts sending
`FabulorSetup-v1.0.7-selection-hit-fix.zip` without ZNC or DALnet. The original
144,340,100-byte ZIP passes CRC and contains the preserved installer with the
expected SHA-256. It is below 2 GiB/4 GiB boundaries.

`dcc_accept()` and the passive sender in `dcc_connect_finished()` registered
separate WRITE and ACK READ watches. Each `fe_input_add()` creates its own
Windows GIO socket channel. A synthetic loopback reproduction against runtime
GLib 2.88.0 stalled at 1,024 bytes on three runs; a combined-watch control
completed 2 MiB on three runs. This proves the conflicting readiness pattern,
not the failure stage of every reported external attempt.

The sender now uses one readiness source, drains ACKs on writable wakes too,
and drops WRITE while throttled, waiting for a slow-send ACK or awaiting the
final ACK. Unthrottling restores WRITE. The common frontend/IRC/plugin watch
implementation, receive/chat/proxy paths and network settings are unchanged.
A second defect is corrected: fragmented four-byte ACK reads previously
restarted at the beginning of the buffer instead of appending at `ack_pos`.
Reads are also capped at the remaining advertised size, including after
partial writes or if the source grows during a transfer.

`tools/gtk4/test_dcc_sender_native.py` compiles five sender functions verbatim
from production `dcc.c`, plus production `fe_input_add/remove`, against the real
`DCC` structure and bundled GLib. UI/signals, accounting and close services are
test substitutes: this is not a full-client or handshake acceptance test.
Eight cases pass: the complete original ZIP (144,340,100 bytes, matching received
SHA-256), slow send with observed split ACK reads, injected partial writes and
would-block plus one-byte ACK reads, absolute and legacy-offset resume, mid-send
throttle/recovery, cancellation and peer disconnect. Successful cases check exact
announced progress/hash; all cases check source removal. No received-file writes,
external recipients, IRC messages, profile/dictionary changes or clipboard use.
Six source contracts cover both active/passive entry points and closure/framing.
The 26 integration checks and all nine existing Meson suites also pass; this does
not resolve the previously recorded intermittent emoji-probe reliability issue.

The isolated Release and installer builds are under
`build/local-1.0.7/dcc-send-fix/`; earlier evidence and failure logs remain under
`build/local-1.0.7/dcc-investigation/`. The prior canonical installer was backed
up before rebuilding. Packaging reuses existing private plugin-host/runtime/legal
stages; unrelated dirty-tree add-on/OPER work is retained. No installation,
commit, push or publication is authorised by these checks.

Final package checks pass: 2,631 installed files with no legacy GTK, runtime
content hashes, bundle chain, metadata/import/bootstrap checks and extracted
frontend/launcher/Enchant/legal hashes. The unsigned candidate is
`build/local-1.0.7/dcc-send-fix/FabulorSetup-v1.0.7-dcc-send-fix.exe`;
its SHA-256 is
`2b89f4d0bf3d8ea1f8271c0410a17a0561dcaa54fb3aabfd00dea7bca71541e6`.
The MSI checksum and scoped review are in that directory.

At the initial investigation, saved DCC settings had no external address
override/server-based discovery and used automatic ports. Those settings and
router/firewall rules were not changed or externally tested by the automated
checks or code patch. Behind NAT, address/port reachability remained separate.

**Installed follow-up (2026-11-08):** the maintainer reported a successful transfer
to HexChat (same-PC/LAN versus external placement was not established), followed
by a partner connection timeout and a reverse offer failing with "Failed to bind
to any address or port." Read-only inspection of selected saved settings found
ports 50000–50004, server address discovery off, an empty DCC IP address, and a
public IPv4 address in Bind to. This is consistent with attempting to bind a
router-owned public address rather than a local PC interface. Guidance was to
clear Bind to, put the WAN IPv4 address in DCC IP address, keep discovery off and
restart Fabulor to clear its cached local address. The maintainer then reported
**"It succeeded this time"**. This records a successful installed DCC retry,
not separately confirmed file, direction, active/passive mode, recipient hash,
restart procedure or independent verification of every router/settings change.
The maintainer reported configuring router forwarding manually; the agent did
not write profile settings or router/firewall rules. No new source or installer
change was made for this configuration follow-up.

Broader installed acceptance must distinguish local bind failures,
QUEUED/zero-byte connection timeouts and ACTIVE data-transfer stalls, and cover
ordinary/passive transfers where supported, both directions, the full reported
ZIP, completion/hash verification and cancellation with a willing recipient.

## 2026-11-08 — Documentation Reconciliation

The ChangeLog, roadmap and feature-specific records were cross-checked against
this evidence. Stale blanket "pending" wording for Windows-suggestion feedback
and WinSpell session-isolation acceptance was corrected in the user manual,
spell architecture and runtime-packaging guide. The documentation index/title
now reflect this record's broader scope, and README/SECURITY identify the
published 1.0.6 stable baseline separately from unpublished local 1.0.7 work.
DCC troubleshooting/settings guidance now distinguishes Bind to, advertised WAN
address, local forwarding destination, ZNC and active/passive connection roles.
No personal IP addresses were added. Historical failure logs and archived release
records remain historical; focused feedback does not close broader platform or
release matrices. This reconciliation changes documentation only, not the
preserved candidate binaries or their hashes. Validation checked 98 local file
links across 13 reviewed documents, the new DCC evidence anchor and
`git diff --check`; all passed. External URLs were not fetched by that link check.

## 2026-11-08 — CTCP VERSION Candidate

**Source-verified and automated-tested; focused successful installed direct-EYErc
peer-query results recorded on 2026-11-08.**

The maintainer reported `/ctcp Barry version` on a direct EYErc connection:
Alchera is Fabulor and Barry is MossIRC. The pasted exchange includes the normal
MossIRC reply plus two Alchera/Fabulor VERSION lines ending at an opening bracket.
Which client displayed those last lines and their exact raw sequence remain to
be confirmed; the direct connection rules out ZNC in this test, not all
server-capability differences.

An offline MSVC probe compiles `ctcp_handle()` and `get_sys_str()` verbatim from
production source. Network send, event/UI, ignore/custom-reply and system-info
services are inert substitutes; no live IRC, WMI, profiles or clipboard are used.
The unmodified handler replies to an own echoed VERSION request addressed to
another nick by sending a VERSION notice to its own nick. Unlike the ordinary
message path, the CTCP path lacks an own-echo guard. This is a reproducible code
path, not confirmation that EYErc negotiated echo-message in the reported test.
Separately, a null CPU result terminates `g_strconcat(os, " [", cpu, "]", NULL)`
at the missing CPU argument, leaving a cached unmatched bracket. An empty CPU
result produces empty brackets. Both are reproduced offline; normal CPU replies,
peer requests with/without echo capability and explicit self-queries are controls.

Evidence is under `build/local-1.0.7/ctcp-investigation/`: `probe.c`,
`under-test.inc`, `probe.cmd`, `probe-before.log` (initial fixture type warning)
and `probe-before-clean.log` (corrected inert ACTION-service signature, no reported
warnings). The probe records behaviour; it is not a full parser/network regression
suite. The exact cause of the duplicate display count is not yet established.
The subsequent pasted reply to `/ctcp Alchera version` also ends at `[`.
A separate "requested CTCP VERSION ... (answered)" entry has a different timestamp;
those excerpts alone do not establish a loop or the exact duplicate count.

The follow-up source fix introduces `ctcp_is_own_echo()`, used both before flood
checking/quoted DCC parsing in `proto-irc.c` and defensively in `ctcp_handle()`.
It requires negotiated echo-message, the local sender nick under the server's
casemap, a different target and a non-ACTION request. ACTION echoes and explicit
self-queries are preserved; NOTICE replies still use the notice path, not request
handling. `get_sys_str()` now includes CPU brackets only for a nonempty CPU result,
preserving its caching and ownership without changing WMI queries or inventing
hardware data. Custom replies/ignore policy for real peer requests remain unchanged.

`tools/gtk4/test_ctcp_native.py` compiles the production classifier/handler and
system-string function with inert services and `/W3 /WX`. Sixteen cases cover
peer requests with/without echo, own/case-folded/channel echoes, no-capability
behaviour, explicit self-query, ACTION, own versus peer DCC, hidden VERSION,
missing/empty CPU, cached and OS-only strings. Preserved pre-fix source fails
both the own-echo and missing-CPU assertions; fixed source passes all cases.
These are function-level tests, not a live network/parser/plugin test. Six source
contracts verify early guards, framing boundaries and NOTICE separation. The
32 integration checks, eight DCC sender cases (including the original ZIP) and
all nine existing Meson suites pass; the emoji reliability follow-up remains open.

The isolated Release/frontend/extensions and MSI/bootstrapper builds pass with no
reported warnings/errors. Package/runtime/bundle/metadata/import checks pass:
2,631 installed files, no legacy GTK, verified runtime hashes and one MSI chain
package. Extracted frontend/launcher/Enchant/legal bytes match staging roots.
The first package validation failed with WIX7013 because concurrent WiX extraction
shared a temporary cabinet path; retained `package-validation.log` records this.
Serial validation passes in `package-validation-serial.log`; this is an extraction
concurrency limitation, not a claim that the validation tools are race-free.

Unsigned local candidate:
`build/local-1.0.7/ctcp-version-fix/FabulorSetup-v1.0.7-ctcp-version-fix.exe`

EXE SHA-256:
`8afff75a271604eccff51c4401324a9581e92126fc4b1e69b4eab336deeb6817`

MSI SHA-256:
`523bb09a1b8a871c34069fd611106db6c798395fb05bd12b77baf2ae2e2b836e`

Build/package logs, extracted hashes and review are in `ctcp-version-fix/`;
native and before-failing evidence remains in `ctcp-investigation/`. The prior
canonical installer was backed up, and previous candidates remain untouched.
Existing private host/runtime/legal stages are reused as documented for the DCC
candidate; unrelated dirty-tree add-on/OPER work is retained. No IRC messages,
credential-provider/WMI invocation, clipboard or profile/dictionary changes,
installation, commit, push, tag or publication was performed by these tests.
The requested installed retest covered direct EYErc VERSION requests in both
directions and explicit self-query, well-formed replies and any remaining
duplicate, using only relevant sanitised lines rather than whole raw logs.

**Installed feedback (2026-11-08):** the maintainer supplied an outgoing VERSION
to Barry, a normal away notice and MossIRC's VERSION reply. The reverse result
shows a complete Fabulor 1.0.7 build/Windows/CPU reply, including closing brackets.
No unsolicited self-VERSION lines appear in that supplied exchange. This records
focused successful peer queries in both directions and supports resolution of
the reported symptoms in this retest. It does not establish an installed
CPU-unavailable case, explicit self-query, ACTION/DCC/custom-reply interactions,
other networks or the exact original duplicate wire sequence. The now-present CPU
information does not prove the earlier WMI failure's cause or permanent recovery;
the fallback was exercised offline. No further source or installer changes were
made in response to these results.

## Oversized Emoji Picker At 250%

On 2026-11-16 the integrated 1.0.7 client displayed oversized emoji at 250%
Windows scaling. A comparison with the last pre-handoff client isolated the
difference to the GTK runtime configuration. The pinned archive contained
`10-scale-bitmap-fonts.conf` in `conf.avail` only. Enabling that exact file in
`conf.d` restored a seven-column picker. Runtime staging now copies and hashes
the enabled file, and the chooser bounds its scrolling area in GTK logical units.

All 12 staging tests and nine Windows GTK suites passed. The fixture on the
250% display measured all 259 country-flag glyphs under both default font
backends. Maximum ink was 24 × 23 logical pixels with the fix and 136 × 129
with the original runtime. MSI validation verified all 1,050 staged runtime
files and the manifest. WiX bundle validation matched the exact embedded MSI.

The extracted MSI client displayed the compact picker using the smiley button
and `Ctrl+;`. It inserted a selected emoji. A country flag appeared as regional
letters in the Consolas input box after selection.

**Installed feedback (2026-11-16):** The maintainer confirmed that the rebuilt
installer worked as expected, the picker returned to normal size at 250% and
the separately installed Aegis preview loaded. The provider was awaiting
channel input, so this verifies add-on loading only.

Tested unsigned setup SHA-256:
`4f335cb4b96eceedfa609e6363c2ec410f0f430b8d205cfdba0b651a39d45d0f`.
Embedded MSI SHA-256:
`28a9cdf57f148c22c6924b345fde703a729b19db75ce1df81fd1d3f4011077c3`.

## Boundaries

- Client source is tracked in the 1.0.7 release PR. The installers used for
  local acceptance are unsigned.
- The v1.0.6 WinGet acceptance and subsequent Microsoft Store track are
  separate from this local v1.0.7 acceptance.
- This closes the specific reported performance/layout issue. It does not
  close the general performance soak, accessibility, display-scale or
  install/modify/repair/uninstall matrices in [the roadmap](../../To-Do.md).
- Cache-key and pane tests cover dimming/colour invalidation, hide/show and
  normal/wide divider geometry. They do not establish manual acceptance for
  every theme, dimming setting, window resize, divider drag or display scale.
