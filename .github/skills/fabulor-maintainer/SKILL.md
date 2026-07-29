---
name: "fabulor-maintainer"
description: "Automatically invoked for work inside fabulor-master. Handles the GTK4-only MSVC build, WiX MSI/Burn installer, Fabulor plugin APIs, and Windows CI."
tools: [read, search, edit, execute]
auto-invoked: true
argument-hint: "Describe the area and task, e.g., 'installer/: update the Python314 runtime component contract.'"
---

# Workspace Scope
- Active for the whole current `fabulor-master` repository, **Windows 11+ only**.
- Treat `src\`, `plugins\`, `installer\`, `win32\`, `Runtime\`, and
  `.github\workflows\windows-build.yml` as maintained surfaces.
- Deprecated non-Windows packaging/build artefacts and non-Windows CI workflows
  are **out of scope**. The inherited application Meson/Make graph is retired;
  retain `tools\gtk4\meson.build` only as the isolated strict GTK4 validation
  probe.
- `docs\plugin-authoring-guides.md` and `docs\plugins\*.md` are the current,
  already-written plugin authoring guides (C#, Python, Tcl,
  schema/compatibility/safe-mode/troubleshooting). Treat them as the public
  contract and keep them updated alongside loader, binding, manifest, or ABI
  changes.
- Read `To-Do.md` before touching plugin loading, the plugin ABI, or the WiX v4
  installer.

# Priority Focus
GTK4 migration and production cutover are complete. Current work is release
readiness, contained installed-UI maintenance, deliberate plugin API growth,
and the staged cleanup recorded in `To-Do.md`. `installer\` is authoritative
and publishes exactly the MSI and bootstrapper;
`win32\zoitechat.sln` is the supported GTK4 native build.

# Constraints
- Windows 11+ only — no Meson options, non-Windows packaging, or non-Windows CI
  matrix entries for new work.
- Preserve the implemented plugin ABI and manifest/loader contracts described
  in `docs\plugins`. Make additive, backward-compatible changes unless a
  breaking `requires_api_version` bump is explicitly approved.
- Lua and Perl integration is retired; do not restore either plugin to the
  source tree, supported solution, extension graph, CI prerequisites, or
  installer.
- Keep `win32\zoitechat.sln` building as the supported GTK4 native entry point.
  Do not restore the retired Inno Setup or GTK3 runtime-copy projects.
- Prefer minimal, localised changes over broad refactors.
- Use Australian English spelling and metric units in new or edited docs,
  comments, and user-facing strings, unless an upstream API/library name
  requires the American spelling verbatim.
- Match each area's existing conventions: tab-indented, Allman-brace C in
  `src\`/`plugins\`; space-indented (size 4) C# in `installer\UX\`; existing
  WiX v4 XML conventions in `installer\`.

# Validation Behaviour
- WiX v4 `installer\` (default/primary): `wix build installer\Product.wxs -o NUL`
  for authoring checks, or `dotnet build installer\Fabulor.wixproj` /
  `msbuild installer\Fabulor.wixproj /t:Rebuild` for a full package build.
  Confirm new/moved files in `Components\*.wxs` are wired into the matching
  `ComponentGroupRef` in `Product.wxs`.
- GTK4 `win32\`: from a Visual Studio developer prompt,
  `msbuild win32\zoitechat.sln /p:Configuration=Release /p:Platform=x64` (add
  `/t:Rebuild` for structural changes).
- CI workflow edits: cross-check against `.github\workflows\windows-build.yml`
  only; do not update the non-Windows workflows for Windows-scoped work.
- Run the focused Python/native contract tests for the owning boundary and the
  applicable suites listed in `.github\workflows\lint.yml`.
- Installed UI/runtime changes require installed-client acceptance before
  publication.
- Never assume permission to run arbitrary or destructive commands. If the
  required toolchain is unavailable, report the command that would have been
  run and fall back to reasoning-based validation.

# Behaviour
1. Identify which surface the task touches (WiX v4 `installer\`, GTK4
   `win32\`, the `ZoiteChatAPI`/plugin-loader rework, plugin sources, or CI).
2. Read only the minimal code needed to form a concrete, falsifiable
   hypothesis, including `To-Do.md` when relevant.
3. Surface any open assumption the task rests on, especially public ABI,
   compatibility, packaging ownership, or retirement policy.
4. Make the smallest safe change that addresses the root cause, matching the
   existing style of the surface being touched.
5. Validate using the commands above, or fall back to reasoning-based
   validation if the toolchain is unavailable.
6. Summarise the change, validation, and any residual risk.

# Output Format
- Files touched
- Change made
- Validation run
- Residual risk or follow-up
