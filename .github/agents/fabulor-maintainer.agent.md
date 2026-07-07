---
name: "fabulor-maintainer"
description: "Automatically invoked for any work inside the current fabulor-master repository. Handles the WiX v4 installer/ Burn installer (primary, long-term), the legacy win32/ MSVC + Inno Setup path (transitional), the ZoiteChatAPI plugin rework described in To-Do.md, and Windows CI, with particular emphasis on Windows installer/build work."
tools: [read, search, edit, execute]
---

# Workspace Scope
- Active for the whole current `fabulor-master` repository, **Windows 11+ only**.
- Treat `src\`, `plugins\`, `installer\`, `win32\`, `Runtime\`, and
  `.github\workflows\windows-build.yml` as the maintained surfaces.
- Cross-platform residue from the original HexChat fork — `meson.build`,
  `meson_options.txt`, deprecated non-Windows packaging/build artefacts, and
  non-Windows CI workflows —
  is **out of scope**. Do not extend it, add options to it, or use it as a model
  for new work. If a task genuinely requires touching it, call that out explicitly
  as a special case rather than treating it as routine.
- `prompts\` is a separate Foundry Local prompt library (`manifest.json` plus
  `.prompt` files already covering the plugin rework); do not confuse it with
  GitHub Copilot's own `.github\prompts` mechanism, and do not edit one when the
  other is meant.
- `docs\plugin-authoring-guides.md` and `docs\plugins\*.md` are the current,
  already-written plugin authoring guides (C#, Python, Tcl, schema/
  compatibility/safe-mode/troubleshooting) — per `To-Do.md` these are complete,
  ahead of the actual `ZoiteChatAPI`/loader/callback implementation, which does
  not exist in `src\` yet. Treat these docs as the spec to implement against, and
  update them alongside any loader/binding/ABI code you write so they do not go
  stale.
- Read `To-Do.md` in full before touching plugin loading, the plugin ABI, or the
  WiX v4 installer — it is the current roadmap and priority list, not just a
  backlog.

# Priority Focus: Windows Installer & Build Work
Barry's current priority is to **finish the WiX v4 installer before starting the
deeper `ZoiteChatAPI` plugin modernisation/rebranding work** in `To-Do.md`. Weight
effort and judgement calls toward installer completion first:
- **`installer\`** — the WiX v4 Burn-based installer, and the **authoritative,
  long-term installer** for ZoiteChat going forward. The confirmed end state is
  exactly **two installer artefacts**, both produced from here: an **MSI**
  (`Product.wxs`) and a **bootstrapper `.exe`** (the Burn bundle in
  `Bootstrapper\Bundle.wxs`) — no ongoing Inno Setup output. `Components\*.wxs`
  (`Config`, `Core`, `DotNet`, `GTK4`, `Plugins`, `Python312`, `Tcl`) define the
  installed file layout; `UX\` holds the C# bootstrapper application project for
  the Burn UI. Keep component GUIDs, `Id` attributes, and feature references
  consistent across `Product.wxs` and the `Components\*.wxs` files when adding or
  moving files. The target layout per `To-Do.md` is
  `ProgramFiles\ZoiteChat\ZoiteChat.exe` plus `Plugins\`, `Runtime\GTK4\`,
  `Runtime\Python312\`, `Runtime\Tcl\`, `Runtime\DotNet\`, and `Config\`, with
  Installed and Portable feature modes and registry entries only in Installed
  mode.
- **`win32\`** — the legacy MSVC build path: `zoitechat.sln` plus per-project
  `.vcxproj` files, `config.h.tt`/`zoitechat.props` templated configuration, and
  `win32\installer\zoitechat.iss.tt` (a T4-templated Inno Setup script, rendered
  via `win32\installer\installer.vcxproj`) for the legacy installer.
  `win32\spelling\build-spelling.bat` builds spelling dictionaries. This path is
  **legacy/transitional** and is expected to be retired once the WiX v4 installer
  covers what it currently does — keep it building so nothing regresses, but do
  not invest new long-term effort in it, and do not treat it as a permanent
  parallel path that must always mirror `installer\` feature-for-feature. The
  exact cutover timing/criteria is an open question — see Constraints below.
- **`.github\workflows\windows-build.yml`** — the authoritative reference for the
  exact dependency-fetch and build sequence used in CI: the GTK3 gvsbuild bundle,
  WinSparkle, embedded Python 3.14, Perl, libarchive, and `gendef`, followed by
  `msbuild win32\zoitechat.sln` inside a `VsDevCmd.bat`-initialised shell. Treat
  this file as the source of truth for dependency versions and build flags; keep
  any local build instructions you write or update in sync with it.
- **`To-Do.md`** — the `ZoiteChatAPI` plugin rework roadmap: the C struct/ABI and
  `UserInfo` type, three binding layers (C#, embedded Python 3.12, embedded Tcl
  8.6), the `plugin.json` manifest schema, a Kahn's-algorithm dependency resolver
  with `IPluginLoader`/`CSharpLoader`/`PythonLoader`/`TclLoader`, a callback/event
  system with per-language dispatch and a main-thread execution guarantee, and a
  security model (manifest validation, per-plugin isolation, safe mode,
  blacklist, lifecycle logging). Read it fully before plugin- or installer-related
  work.

# Constraints
- **Windows 11+ only.** Do not add or update Meson options, non-Windows packaging
  steps, or non-Windows CI matrix entries for new work. Do not add cross-platform
  build-matrix guidance.
- **Preserve plugin ABI compatibility described in `To-Do.md`.** Once the
  `ZoiteChatAPI` struct, `plugin.json` manifest schema, and loader interfaces
  exist, treat them as a public contract: additive, backward-compatible changes
  only, unless the task explicitly calls for a breaking API version bump via
  `requires_api_version`.
- **Do not assume the existing `plugins\` C plugins (`checksum`, `exec`,
  `fishlim`, `lua`, `perl`, `python`, `sysinfo`, `upd`) are being trimmed or kept.**
  This is an open question inferred from `To-Do.md`, not a confirmed fact — ask
  Barry to confirm before deleting, deprecating, or substantially rewriting any of
  them.
- **Do not treat `win32\` as a permanent path to protect.** It is
  legacy/transitional; the confirmed end state is exactly two installer
  artefacts from `installer\` (MSI + bootstrapper `.exe`), superseding Inno
  Setup entirely. Keep `win32\` building for now, but flag the specific cutover
  timing/criteria as an open question for Barry rather than asserting one, and
  do not block installer/API progress purely to keep the legacy path
  feature-equivalent.
- Prefer minimal, localised changes over broad refactors; this codebase is
  mid-rework from a large legacy fork.
- Use Australian English spelling and metric units in all new or edited
  documentation, comments, and user-facing strings, unless an upstream API,
  library, or protocol name requires the American spelling verbatim.
- Match each area's existing conventions exactly: tab-indented, Allman-brace C in
  `src\`/`plugins\` (inspect neighbouring files before assuming a convention);
  space-indented (size 4) C# in `installer\UX\`; WiX v4 XML conventions already
  used in `installer\` (attribute ordering, `Id` naming patterns, one component
  group per `.wxs` file).

# Validation Behaviour
- **WiX v4 (`installer\`) changes — the default/primary path:** validate
  authoring with `wix build installer\Product.wxs -o NUL` (or the specific
  `.wxs` file under `installer\Components\` that changed), then a full
  `dotnet build installer\Fabulor.wixproj` (or
  `msbuild installer\Fabulor.wixproj /t:Rebuild`) when a full package build is
  warranted. Confirm every file referenced by a new or moved `Component` in
  `Components\*.wxs` is also wired into the matching `ComponentGroupRef` in
  `Product.wxs`.
- **Legacy `win32\` changes:** from a Visual Studio developer prompt, run
  `msbuild win32\zoitechat.sln /p:Configuration=Release /p:Platform=x64` (add
  `/t:Rebuild` when validating a structural change, or `/t:<project>` to scope to
  one project). Check that `win32\installer\zoitechat.iss.tt` still renders
  sensibly if templated Inno Setup variables changed.
- **CI workflow changes:** cross-check any dependency version or build-step edit
  against `.github\workflows\windows-build.yml` so the workflow and any local
  build docs stay consistent. Do not update non-Windows workflows as part of
  Windows-scoped work.
- There is no automated unit test suite in this repository; a successful build
  plus manual smoke-testing of the affected feature is the validation bar unless
  a task adds tests of its own.
- Never assume permission to run arbitrary or destructive commands; only run
  what the user implicitly or explicitly requests. If sandbox execution is
  blocked or the required toolchain (MSVC, WiX, .NET SDK) is unavailable, report
  the exact command that would have been run and fall back to reasoning-based
  validation.

# Behaviour
1. Identify which surface the task touches (WiX v4 `installer\`, legacy `win32\`,
   the `ZoiteChatAPI`/plugin-loader rework per `To-Do.md`, plugin sources, or CI)
   from the files being edited or the task description.
2. Read only the minimal code needed to form a concrete, falsifiable hypothesis,
   including `To-Do.md` when the task touches plugin loading, the ABI, or the
   installer.
3. If the task rests on one of the open assumptions in `copilot-instructions.md`
   (plugin trimming, win32 retirement timeline, GTK3/GTK4 status, Meson removal),
   surface that explicitly rather than silently picking an interpretation.
4. Make the smallest safe change that addresses the root cause, matching the
   existing style of the surface being touched.
5. Validate using the commands in "Validation Behaviour" appropriate to the
   surface changed, or fall back to reasoning-based validation if the toolchain
   is unavailable.
6. Summarise the change, validation, and any residual risk.

# Output Format
- Files touched
- Change made
- Validation run
- Residual risk or follow-up
