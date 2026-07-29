# Repository Cleanup Plan

Last updated: 2026-07-29

## Purpose

This plan removes repository content that no longer belongs to the supported
Windows 11+, GTK4, MSVC/WiX Fabulor project. A file is not removable merely
because it is old or retains a `ZoiteChat`/`XChat` name. Internal ABI names,
configuration compatibility, copyright history, and active installer assets
remain until a dedicated review proves otherwise.

User-maintained add-ons live in the separate `Fabulor/add-ons` repository. The
main client repository must not contain or stage a second add-ons checkout.

## Status Terms

- `Planned`: candidate inventory exists but removal has not started.
- `Implemented`: repository changes and focused checks pass.
- `Accepted`: required build or installed testing passes.
- `Published`: committed, pushed, and represented by a pull request.
- `Retained`: reviewed and intentionally kept.

## Cleanup Stages

| Stage | Scope | Status |
|---|---|---|
| 1 | Dead repository metadata, completed prompt scaffolding, unbuilt Lua source, retired Inno spelling scripts, and superseded resource/version files | Published |
| 1A | Remove bundled add-on scripts after migration to `Fabulor/add-ons` | Published |
| 2 | Retained Perl source, `perl_warnings`, and obsolete Perl-facing messages/configuration | Published |
| 2A | Remove the superseded Python 3.12 runtime and generated investigation artefacts | Published |
| 2B | Remove stale ZoiteChat product branding and dead packaged metadata | Published |
| 2C | Rename active Visual Studio solution, properties, and build-only identifiers for Fabulor | Published |
| 3 | Move active `win32\copy` payload assets into owned `data` locations and retire the legacy copy namespace | Planned |
| 4 | Audit the unbuilt text frontend, Windows compatibility shims, and duplicate backend implementations | Planned |
| 5 | Review historical migration/security documents for archive policy without deleting evidence required for maintenance | Planned |
| 6 | Review internal ZoiteChat/XChat compatibility names separately from product branding | In progress |
| 7 | Audit ignored local build/runtime output and document a safe developer cleanup command | Planned |

## Stage 1

### Removed

- `.pc`: quilt patch-application state from the imported source tree
- `.lgtm.yml`: configuration for the retired LGTM service
- `foundry.json` and `prompts`: completed Foundry-era implementation prompts
  that describe obsolete Python 3.12 and pre-implementation plans
- `plugins\lua`: unsupported, unbuilt, unpackaged Lua plugin source
- `win32\spelling`: retired Inno Setup dictionary builder
- `win32\zoitechat.exe.manifest`: superseded by `fabulor.exe.manifest`
- `src\fe-gtk\zoitechat.rc.tt`: superseded by `fabulor.rc.tt`
- `win32\version.txt`: obsolete `2.18.3` version source; the canonical version
  is `installer\Directory.Build.props`

### Updated

- repository and maintainer instructions now describe the current GTK4-only
  build, Python 3.14 runtime, implemented plugin hosts, validation suites, and
  supported installer artefacts

### Validation

- removed-file reference audit passed across the solution, WiX graph, CI, and
  production staging contracts
- production WiX profile: 24/24 tests passed
- plugin-host staging: 7/7 tests passed
- theme contract and theme tests: 7/7 tests passed
- Release x64 solution rebuilt successfully, including 22/22 native manifest
  and theme tests
- MSI/bootstrapper rebuild was not required because no installer-owned input
  changed

## Stage 1A

The tracked `addons` directory was retired after its maintained scripts moved
to the independent `Fabulor/add-ons` repository. That repository now owns its
source history, documentation, pull requests, and Python/Tcl validation
workflow. No client build, installer, or runtime staging input referenced these
two bundled source files. Plugin-host staging passed 7/7 tests and the
production WiX profile passed 24/24 tests after removal.

## Stage 2

The unbuilt and unpackaged `plugins\perl` source tree, `OLD_PERL` compatibility
macro, `perl_warnings` preference storage, obsolete `/LOAD` guidance, and stale
active-source comments were retired together.

Compatibility policy:

- an existing saved `perl_warnings` value is ignored during configuration load;
- the key is omitted on the next canonical configuration save;
- `/SET perl_warnings` reports no such variable; and
- maintained native, C#, Python, and Tcl plugin paths remain unchanged.

Historical changelog, translation-catalog, migration, and security-audit
references remain as records. Current maintainer guidance and regression tests
must not describe or permit a restorable Perl integration.

Automated evidence:

- the Release x64 solution rebuilt successfully with 22/22 native tests;
- the repository Perl-retirement contract and production WiX profile passed
  25/25 tests;
- plugin, runtime, payload, import, and theme suites passed all 71 remaining
  tests, for 96/96 Python tests overall; and
- the theme contract validator passed.

The shared product version advanced to 1.0.5 so Burn and MSI detect the rebuilt
client as an upgrade rather than handing maintenance to the cached 1.0.4
bundle. The MSI and bootstrapper rebuilt with zero warnings and errors, and
production bundle validation confirmed version 1.0.5 with one embedded MSI.

## Stage 2A

The Python plugin host and production installer use Python 3.14 exclusively.
The ignored `Runtime\Python312` development payload and its obsolete
`Python312.wxs.bak` installer fragment were removed. A tracked WiX binary log
and two untracked file-opening investigation probes were also deleted.

`Runtime\Python314`, the configured Python 3.14 build interpreter, active
installer output, dependency caches, and the local `dos2unix.exe` maintenance
tool remain intact.

Regression coverage prevents the Python 3.12 payload, backup WiX fragment, and
tracked WiX binary log from returning.

Validation:

- production WiX profile: 26/26 tests passed;
- plugin-host staging: 7/7 tests passed; and
- active build, installer, CI, and maintainer guidance contain no Python 3.12
  references.

## Stage 2B

Current diagnostics, Python interface descriptions, spell-check setup, source
comments, and troubleshooting guidance now identify the product as Fabulor.
The installer no longer packages the dead ZoiteChat changelog shortcut, and
single-instance forwarding no longer targets a separately running retired
ZoiteChat client.

Native and installer builds completed with zero warnings and errors.

## Stage 2C

The supported Visual Studio entry points are `win32\fabulor.sln` and
`win32\fabulor.props`. Active projects, CI, tests, tools, and maintainer
documentation use Fabulor-named build properties and output paths.

Historical security records retain the build names used when their recorded
scans were performed. Runtime internals and public plugin compatibility names
remain outside this build-only stage.

## Stage 6

The public plugin API rename is split by language and ABI boundary so each
change can be built, packaged, and tested independently.

The Python pass makes `fabulor` the sole Fabulor-owned module for simple and
manifest plugins. The inherited `zoitechat` and `_zoitechat` Python modules are
removed from source and the production payload. The intentional `xchat` and
`hexchat` compatibility modules remain and forward to `fabulor`.

The embedded Python bridge is now `_fabulor_embedded`, the isolated manifest
runtime installs only the `fabulor` API, and maintained samples and authoring
guidance use `import fabulor`. The native `zoitechat_*` entry points called by
the bridge remain until the separate native ABI pass; Tcl and C# names remain
for their own contained passes.

Compatibility policy:

- existing Python add-ons must replace `import zoitechat` with
  `import fabulor`;
- `xchat` and `hexchat` imports continue to work; and
- no silent `zoitechat` alias is packaged, so stale add-ons fail clearly
  instead of perpetuating a second Fabulor API name.

Automated evidence for the Python pass:

- the focused capability, isolation, and staging suites pass 22/22 tests;
- the complete Python runtime, packaging, installer-profile, and theme suites
  pass 108/108 tests plus the theme contract validator;
- the Python native extension rebuilds under MSVC level-4
  warnings-as-errors; and
- the version 1.0.6 MSI and bootstrapper rebuild with zero warnings and errors,
  then pass production, runtime-content, and bundle validation.

Installed-client acceptance remains required before this pass is published.

## Deliberately Retained

- `win32\copy` currently supplies WiX assets and ISO code data at runtime. It is
  active despite its legacy location and remains until Stage 3 moves each file
  to an explicit owner.
- Native generated symbol prefixes, Tcl compatibility names, and
  `ZoiteChat*` managed types remain active API or ABI surfaces for their
  contained Stage 6 passes.
- `src\fe-text` and `src\dirent` require build/reference audits before any
  removal.
- GTK4 migration and security records remain evidence, even where they describe
  retired intermediate states.

## Stage Gate

Each stage must:

1. prove every candidate is absent from active build, package, CI, runtime, and
   documented user workflows;
2. preserve compatibility policy explicitly where saved state or public APIs
   are involved;
3. run focused tests plus the appropriate native/package validation;
4. keep the independent add-ons checkout outside the client worktree; and
5. be committed and reviewed independently from unrelated cleanup stages.
