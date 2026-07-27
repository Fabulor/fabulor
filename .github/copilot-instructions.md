# Copilot Instructions for `fabulor-master`

## Supported Product

Fabulor is a Windows 11+, GTK4 IRC client. The supported application build is
the Release x64 MSVC solution at `win32\zoitechat.sln`. The supported package
is the WiX MSI and Burn bootstrapper produced from `installer\`.

Do not restore:

- GTK3 source, runtime, theme, or build branches
- the inherited application Make/Meson graph
- Inno Setup packaging
- Lua or Perl to the supported solution/package
- non-Windows build or packaging work

The isolated `tools\gtk4` Meson project is retained only as an independent
strict GTK4 compile/runtime probe.

## Current Architecture

- `src\common`: IRC core, configuration, security boundaries, and manifest
  plugin host
- `src\fe-gtk`: GTK4-only Windows frontend
- `src\managed`: .NET 8 C# plugin abstractions and collectible host
- `plugins`: supported native extensions plus the Python bridge; Lua and Perl
  integration are retired
- `samples\plugins`: maintained simple and manifest C#, Python, and Tcl samples
- `Runtime\GTK4`: development/runtime source used by the allowlisted staging
  tools
- `Runtime\Python314`, `Runtime\Tcl`, and staged `Runtime\DotNet`: private
  plugin runtimes
- `installer`: authoritative WiX MSI and Burn bootstrapper
- `tools\gtk4`: runtime, import, extension, payload, installer, and bootstrap
  contracts/tests
- `docs\plugins`: current plugin authoring contract
- `docs\gtk4`: migration history and current GTK4 validation evidence
- `docs\cleanup`: staged retirement plans

Internal `ZoiteChat` and inherited `XChat` names may be active ABI,
configuration, compatibility, or copyright-history surfaces. Do not rename or
delete them based only on branding.

## Build and Validation

Run commands from a Visual Studio x64 developer environment where required.

| Task | Command |
| --- | --- |
| Build native solution | `msbuild win32\zoitechat.sln /m /p:Configuration=Release /p:Platform=x64` |
| Build MSI/bootstrapper | `dotnet build installer\Fabulor.wixproj -c Release -p:Platform=x64 --disable-build-servers` |
| Plugin-host staging tests | `python tools\gtk4\test_stage_plugin_hosts.py` |
| Production WiX profile tests | `python tools\gtk4\test_production_wix_profile.py` |
| GTK4 contract suite | Run the Python tests listed in `.github\workflows\lint.yml` |
| Theme contracts | `python tools\validate_theme_contract.py` and `python tools\test_validate_theme_contract.py` |

Use `.github\workflows\windows-build.yml` as the authoritative dependency,
staging, native-build, WiX-build, and package-validation sequence.

## Engineering Rules

- Use Australian English in documentation and user-facing text unless an API
  or protocol fixes the spelling.
- Match the inherited tab-indented, Allman-brace C style.
- Use four-space C# indentation and existing WiX authoring conventions.
- Keep edits local to the owning module and preserve unrelated worktree
  changes.
- Treat C#, Python, and Tcl plugin manifests/capabilities as public contracts.
- Keep simple user add-ons under `%APPDATA%\Fabulor\addons`; repository add-on
  work is moving to the separate `Fabulor/add-ons` repository.
- Runtime and installer payloads must be explicit, contained, hashed, and
  validated. Do not restore whole-tree harvesting for convenience.
- Update the applicable validation or cleanup document after each completed
  stage.

## Cleanup Rules

Follow `docs\cleanup\repository-cleanup-plan.md`.

Before deleting a legacy-looking file:

1. prove it is absent from the solution, CI, WiX, staging contracts, tests, and
   runtime lookup;
2. identify any saved-state or ABI compatibility requirement;
3. remove related stale instructions and documentation in the same stage; and
4. run the build/test surface appropriate to the removed content.

Do not stage the separate, currently modified `addons` worktree in Fabulor
cleanup commits.
