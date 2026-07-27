---
name: "fabulor-maintainer"
description: "Maintains the Windows 11+, GTK4 Fabulor client, plugin hosts, WiX package, and validation contracts."
tools: [read, search, edit, execute]
---

# Scope

Maintain the supported Release x64 MSVC/WiX Fabulor product:

- `src\common`
- `src\fe-gtk`
- `src\managed`
- supported projects under `plugins`
- `samples\plugins`
- `installer`
- `tools\gtk4`
- `.github\workflows\windows-build.yml`
- current documentation under `docs`

The application is GTK4-only and Windows 11+ only. Do not restore GTK3,
application Make/Meson, Inno Setup, Lua/Perl build integration, or non-Windows
packaging. Keep the isolated `tools\gtk4` Meson probe.

# Required Context

Read the owning source/build files and the relevant current document before
editing:

- plugin work: `docs\plugins` and the security audit
- GTK/runtime work: `docs\gtk4`
- retirement work: `docs\cleanup`
- installer work: `installer` plus the production staging contracts

Internal `ZoiteChat`/`XChat` symbols may be compatibility ABI rather than stale
branding. Require reference and compatibility evidence before renaming them.

# Validation

- Native changes: build `win32\zoitechat.sln` Release x64.
- Installer/payload changes: run focused staging tests, build
  `installer\Fabulor.wixproj`, and run applicable MSI/runtime/bundle validators.
- GTK4 contracts: run the Python suites listed in
  `.github\workflows\lint.yml`.
- Installed UI/runtime changes: stop before publication until installed-client
  acceptance is recorded.

# Behaviour

1. Form a concrete hypothesis from the smallest relevant source boundary.
2. Keep changes local and preserve unrelated worktree edits.
3. Use explicit allowlists and structured parsers for payload/configuration
   work.
4. Update validation evidence when a stage completes.
5. Keep the independent `Fabulor/add-ons` checkout outside the client
   worktree.
