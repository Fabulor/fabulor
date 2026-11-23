# Local v1.0.7 Consolidation And Publication Handoff

Date: 2026-11-08

Status: **inventory started; not publication approval or a completed release-readiness review**.
Published baseline during this handoff: 1.0.6. Release under review: 1.0.7.
No commit, push, tag, installation or GitHub release is authorised by this plan.

## Initial Inventory

The initial snapshot, before adding this handoff document, used main HEAD
`c3c4fa09`. Private machine paths and the detailed file list are retained locally
in `build/local-1.0.7/consolidation/inventory.json`; do not publish that file
without reviewing its contents.

| Worktree | Relationship to local main | Initial working-tree changes |
| --- | --- | --- |
| Main checkout | Main HEAD | 70 modified tracked files; 55 untracked files, including local agent/session material |
| `25ed` | Same HEAD, detached | Clean |
| `319a` | Seven commits behind, none ahead | Clean |
| `a199` | One commit behind, none ahead | 19 modified tracked files; two untracked files |

There are no commits unique to those other worktree HEADs relative to local
main at this snapshot. This does **not** mean their uncommitted work is merged.
Of the 21 changed/untracked files in `a199`, 14 are byte-identical to the main
checkout and seven differ. Hash comparison results are retained locally in
`build/local-1.0.7/consolidation/private-comparison.json`.

The seven differences are:

- `.github/workflows/lint.yml`
- `ChangeLog.md`
- `To-Do.md`
- `docs/user/settings-reference.md`
- `installer/Directory.Build.props`
- `src/fe-gtk/setup.c`
- `src/fe-gtk/xtext.c`

**Known integration gap:** the private lint workflow adds
`python tools/test_transcript_background_contract.py`; main has that test file
but not this workflow addition. Review and integrate appropriate CI wiring,
including the other new tests, rather than treating local runs as CI coverage.
The other differences require semantic reconciliation, not wholesale copying:
main contains newer spelling/selection work and documentation/version changes.
Do not reset, clean, remove or overwrite the private worktrees.

## Feature Batches To Account For

These are inventory groups, not independent cherry-pick instructions. Shared
headers, project files, preference definitions and build/test registration need
an integrated review after the groups are reconciled.

| Group | Source and supporting material to retain | Current evidence boundary |
| --- | --- | --- |
| Transcript/performance/layout | XText background/display/widget files; channel/user-list/window geometry; clipped-label helper; source/native probes | Focused installed feedback and local automated evidence; broader soak/scaling/accessibility separate |
| Emoji | Shared native chooser, isolated font map, entry/menu wiring, flags resources/tests | Maintainer accepted reported behaviour; investigation deferred on 2026-11-21 after 36 passing repetitions, with font-fallback warnings retained in the evidence |
| DALnet DCC permissions | Nick context model/menu, origin revalidation, tests and user guidance | Focused installed acceptance; no auto-accept or custom-menu replacement |
| Services timestamp and selection | Timestamp guard, clipped-prefix hit mapping, real-widget fixture/support headers | Focused installed feedback; broader interaction matrix separate |
| Sysinfo | Both Windows backend paths, headers/command aliases and WMI tests | Focused installed acceptance; driver-specific breadth separate |
| Spelling | Auto-correct controller, entry/preferences integration, Unicode undo, ordered WinSpell suggestions | Focused positive installed feedback; broader examples/IME/preferences/accessibility separate |
| Enchant patch | Reproducible provider recipe, isolated smoke guards, component metadata, patch notice and legal generation tests | Source/native/extracted-payload evidence; preserve pinned inputs and personal-dictionary safety |
| CTCP VERSION | CTCP helper/handler, early IRC parser guard, system-string fallback and native/source tests | Local fixes/build/package checks pass; focused direct-EYErc peer queries succeed both ways with well-formed replies and no extra self-lines in supplied output; broader interactions separate |
| DCC transport | Sender readiness state/logic, ACK framing, announced-size bounds, native and source fixtures, configuration guidance | Full original ZIP loopback plus eight cases; successful installed retry with direction/file/mode not separately specified |
| Add-on UI/Python bridge | Add-on panel, common frontend/plugin interfaces, Python APIs, schemas/examples/docs/tests | Separate work exists in this tree and isolated add-on candidates; full reconciliation/review not completed by this inventory |
| OPER/authentication | OPER core/UI, protocol/server/outbound/secret-store integration and tests | Separate work exists in this tree and oper-login candidate; credential handling and acceptance need explicit reconciliation |
| Aegis add-on | Separate add-on source, local candidate packages and its own tests/documentation | Maintainer explicitly confirms Aegis still needs work; not accepted as complete or authorised as a release asset |
| Packaging/docs | Version properties, project source/filter lists, Meson/CI wiring, inventories/notices, roadmap/change log/manual/architecture | Preserve GTK4-only/identity/private runtime/profile invariants; local documentation updates are not publication approval |

Untracked product source, headers, fixture resources, notices, documentation and
sample files must be included in the reviewed source revision where appropriate.
A commit containing only currently tracked diffs would omit essential work.

## What Is Not A Release Source Payload

- `.pi/` session/checkpoint material and machine-specific/private diagnostics.
- Ignored `build/` trees, developer environments, extracted installers, upstream
  work directories, staged runtimes and temporary binaries.
- User profiles, logs, credentials, dictionaries, personal IP addresses or router
  configuration. Never bulk-copy the working directory into a publication tree.
- Superseded candidate installers or exploratory probes as final release assets.
- Agent-policy files under `.agents/` or `.github/agents/` without a separate
  decision/review about which project instructions belong in version control.

Keep local evidence and prior candidates intact. Select sanitised summaries,
recipes, manifests and relevant failure history for the source/release record;
do not silently delete historical failed tests or the dictionary-probe incident.

## Ordered Handoff Gates

### Transferred review worktree — 2026-11-16

The detached review worktree is based on `63ae178c`, matching the checked
GitHub `main` baseline. The earlier private worktree remains untouched.
The updated transfer retains the previous transcript/layout fixes alongside
the newer selection, timestamp and spelling changes.

The transferred Windows x64 core, frontend, launcher and native extensions
(including the Python host) built locally. The MSVC GTK probe needed an
explicit PangoCairo link dependency for the emoji picker and passed after that
correction. The Python panel, hidden native panel, mocked OPER and offline
CTCP/DCC tests passed. The relevant checks are wired into Lint and Windows
Build locally; remote CI has not run on this revision. These checks do not
exercise a real OPER connection or the installed installer upgrade.

The client transfer contains the optional Python GTK4 panel API, not the
OpenAI SDK or Aegis. For 1.0.7 the maintainer has chosen to keep the SDK an
optional, separately installed dependency. Aegis is still being tested in the
separate add-ons repository and has not been published. Neither belongs in the
1.0.7 client installer.

The Colours, Network List and Desktop Theme Preferences screenshots were
reconciled with the maintainer's latest copies. The replacement Desktop
Theme Preferences image leaves the Background image field blank. The
transferred Network Editor image already matches the maintainer's latest
copy. No screenshot conflicts remain.

The private candidate was rebuilt against the archive-verified GTK4 runtime
with a fresh Enchant stage and managed host build. The pinned embeddable
Python 3.14.3 imported CFFI without the OpenAI SDK. Local package metadata,
MSI production identity and payload, GTK runtime file hashes and setup/MSI
byte equality passed. The tested files and their SHA-256 values are retained
under `build/private-candidate-1.0.7/test-installers/`. This is an unsigned
local candidate from before the emoji sizing fix.

The maintainer backed up the existing profile, reported that the 1.0.6 →
1.0.7 upgrade went as expected, and confirmed that the existing Aegis add-on
loaded. The running client reported version 1.0.7 with Windows and CPU
information. The separately loaded local demo returned `Applied locally: True`,
confirming the installed GTK4 panel and Python callback path without contacting
IRC or OpenAI. This is focused upgrade acceptance, not verification of every
installer mode, live OPER transport or external moderation.

PR review follow-up: the emoji chooser now registers the bundled font on its
own map and drops its cached reference when the input widget is unrealised.
The pinned-runtime fixtures render all 259 country flags under both font
backends and cover chooser teardown with and without a widget parent. The
DALnet menu now checks the original connection generation before dispatching
permissions. These changes postdate the private installer above.

The next build enabled the bundled bitmap-font rule and restored normal picker
sizing. The maintainer confirmed the corrected installer and picker at 250%
scaling on 2026-11-16. The
[validation record](local-1.0.7-validation.md#oversized-emoji-picker-at-250)
identifies the tested build and records the separate Aegis load result.

### 1. Reconcile Source And Build Wiring

Aegis remains a separate workstream under testing by maintainer confirmation.
Do not infer its readiness from Fabulor's local validation or the existence
of local Aegis ZIPs. The Fabulor bridge should be validated without Aegis or
the separately installed SDK.

- [ ] Inventory outstanding Aegis work and its source location; identify required
      Fabulor API dependencies and agree the acceptance/release boundary.
- [x] Inventory registered worktrees and their main-relative commit counts.
- [x] Compare the private preparation tree's changed files with the main tree.
- [x] Reconcile the earlier private layout changes with the newer transfer
      and restore transcript background test wiring.
- [ ] Review all tracked/untracked product files by feature; account for shared edits.
- [ ] Complete add-on/Python and OPER scope/security/plugin API review and map their
      acceptance evidence into the same validation record.
- [ ] Wire appropriate new source/native tests into supported CI/build entry points;
      remove dependencies on private absolute paths or stale generated objects.
- [ ] Define the exact reviewed source set, generated inputs and exclusions before
      seeking approval for commits. Avoid `git add .` or blanket directory copying.

### 2. Rebuild And Validate The Integrated Candidate

- [ ] Build frontend, native plugins and private language hosts from the integrated
      source revision; rebuild or verify dependency stages against pinned recipes.
      Do not rely indefinitely on `oper-login/hosts` just because earlier local
      candidates reused it.
- [ ] Regenerate component/legal material and verify the Enchant downstream notice.
- [ ] Run the complete applicable tests, including add-on/OPER and new DCC coverage;
      retain original failures and investigate/explicitly triage the emoji flake.
- [ ] Re-run MSI/runtime/bundle/metadata/import/extracted-payload checks against the
      final bytes, not a previous installer with the same version number.
- [ ] Obtain/reconcile required installed upgrade, feature-selection, repair,
      uninstall/profile-preservation, portable and relevant UI/network acceptance.
- [ ] Review untested networking concerns (including IPv6 listener assumptions),
      security findings and acceptance gaps before deciding release readiness.

The newer CTCP candidate, provenance and focused successful installed EYErc
peer-query results are recorded in the linked local validation record. Neither
source-level tests nor this focused feedback establish every CTCP interaction.
The following DCC candidate remains preserved evidence.

The most recent DCC-specific candidate is provenance/evidence, not automatically the final
publication artefact. Its EXE SHA-256 is
`2b89f4d0bf3d8ea1f8271c0410a17a0561dcaa54fb3aabfd00dea7bca71541e6`.
Its build predates the latest documentation reconciliation. See
[local development validation](local-1.0.7-validation.md) for exact scope.

### 3. Publication Preparation And Explicit Approval

- [ ] Decide the release revision/branch and whether another RC is required.
- [ ] Obtain approval for the planned commits; do not rewrite existing history.
- [ ] Run the required CI/security checks on that exact reviewed revision.
- [ ] Build/validate the final artefacts from that revision through the reviewed
      release process, retain source-to-binary provenance and calculate final hashes.
- [ ] Prepare release notes, known limitations, dependency/licence inventory and
      unsigned-installer disclosure consistent with current signing policy.
- [ ] Obtain explicit approval before push/tag/GitHub release publication; verify
      uploaded assets/hashes after the authorised publication operation.

WinGet, signing services and Microsoft Store work remain separate approval gates.
A local transfer success or a documentation reconciliation is not approval to
publish, and the existing unsigned distribution policy must not be misrepresented.
