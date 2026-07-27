---
name: "fabulor-maintainer"
description: "Automatically invoked for work inside fabulor-master. Handles the GTK4-only MSVC build, WiX MSI/Burn installer, Fabulor plugin APIs, and Windows CI."
tools: [read, search, edit, execute]
---

# Workspace Scope
- Active for the whole current `fabulor-master` repository, **Windows 11+ only**.
- Treat `src\`, `plugins\`, `installer\`, `win32\`, `Runtime\`, and
  `.github\workflows\windows-build.yml` as the maintained surfaces.
- Deprecated non-Windows packaging/build artefacts and non-Windows CI workflows
  are **out of scope**. The inherited application Meson/Make graph is retired;
  retain `tools\gtk4\meson.build` only as the isolated strict GTK4 validation
  probe.
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
  `ProgramFiles\Fabulor\fabulor.exe` plus `Plugins\`, `Runtime\GTK4\`,
  `Runtime\Python312\`, `Runtime\Tcl\`, `Runtime\DotNet\`, and `Config\`, with
  Installed and Portable feature modes and registry entries only in Installed
  mode.
- **`win32\`** — the supported GTK4 MSVC build path: `zoitechat.sln`, per-project
  `.vcxproj` files, and shared `config.h.tt`/`zoitechat.props` configuration.
  The Inno installer and broad GTK3 runtime-copy project are retired;
  `win32\spelling\build-spelling.bat` remains for spelling dictionaries.
- **`.github\workflows\windows-build.yml`** — the authoritative dependency and
  publication sequence: pinned GTK4, pinned vcpkg OpenSSL, WinSparkle, embedded
  Python 3.14, Tcl, Enchant, MSVC frontend/extensions, then WiX publication.
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
- Lua and Perl source is historical only; do not restore either plugin to the
  supported solution, extension graph, CI prerequisites, or installer.
- Keep `win32\zoitechat.sln` building as the supported GTK4 native entry point.
  Do not restore the retired Inno Setup or GTK3 runtime-copy projects.
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
- **GTK4 `win32\` changes:** from a Visual Studio developer prompt, run
  `msbuild win32\zoitechat.sln /p:Configuration=Release /p:Platform=x64` (add
  `/t:Rebuild` when validating a structural change, or `/t:<project>` to scope to
  one project).
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
1. Identify which surface the task touches (WiX v4 `installer\`, GTK4 `win32\`,
   the `ZoiteChatAPI`/plugin-loader rework per `To-Do.md`, plugin sources, or CI)
   from the files being edited or the task description.
2. Read only the minimal code needed to form a concrete, falsifiable hypothesis,
   including `To-Do.md` when the task touches plugin loading, the ABI, or the
   installer.
3. If the task rests on one of the open assumptions in `copilot-instructions.md`
   (historical source deletion, GTK3 cleanup order, or Meson removal),
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
