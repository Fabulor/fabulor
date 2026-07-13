# Copilot instructions for `fabulor-master`

> **Scope note.** ZoiteChat is being modernised from its HexChat-fork origins into a
> ground-up rework targeting **Windows 11 and later only**. Cross-platform concerns
> inherited from the fork — Meson and other deprecated non-Windows packaging/build surfaces,
> WSL, D-Bus, libcanberra, AppIndicator, and the Lua/Perl scripting plugins — are
> **out of scope** for new work unless a task explicitly says otherwise. That
> tooling still exists in the tree as legacy residue from the fork; do not extend
> it, and flag it rather than quietly working around it if it blocks a task.

> **Current priority phase.** Per Barry: **finish the WiX v4 installer
> (`installer\`) first**, before starting the deeper `ZoiteChatAPI` plugin
> modernisation and rebranding work in `To-Do.md`. Weight day-to-day judgement
> calls toward installer completion; treat the plugin API/rebranding rework as
> the next phase, not the current one, unless a task explicitly says otherwise.

## Build, test, and lint commands

There are two Windows build surfaces today. The WiX v4 installer under `installer\`
is the **primary, long-term installer** and the current priority — Barry's stated
near-term direction is to **finish the WiX v4 installer before starting the deeper
modernisation/rebranding work** described in `To-Do.md`. The long-term plan is for
ZoiteChat to ship exactly **two installer artefacts**, both produced from
`installer\`: an **MSI** (`installer\Product.wxs`) and a **bootstrapper `.exe`**
(the Burn bundle in `installer\Bootstrapper\`). The legacy `win32\` MSVC solution
and Inno Setup script are **legacy/transitional** and are expected to be retired
once the WiX v4 installer covers everything it currently does — kept building for
now so nothing regresses, but not a permanent third installer path (see
"Assumptions to verify with Barry" below for the exact cutover timing/criteria).

| Task | Command |
| --- | --- |
| Build the WiX v4 installer bundle | `dotnet build installer\Fabulor.wixproj` (or `msbuild installer\Fabulor.wixproj /p:Configuration=Release`) from a Visual Studio developer prompt (`VsDevCmd.bat`) with the WiX v4 toolset and .NET SDK on `PATH` |
| Rebuild the WiX v4 installer from clean | `msbuild installer\Fabulor.wixproj /t:Rebuild /p:Configuration=Release` |
| Validate WiX authoring only, without a full package build | `wix build installer\Product.wxs -o NUL` (or the relevant `.wxs` file under `installer\Components\`) |
| Build the legacy Windows solution (Release, x64) | `msbuild win32\zoitechat.sln /p:Configuration=Release /p:Platform=x64` (from a `VsDevCmd.bat`-initialised shell — see `.github\workflows\windows-build.yml` for the full dependency-fetch sequence: GTK3 gvsbuild bundle, WinSparkle, embedded Python 3.14, Perl, libarchive, `gendef`) |
| Rebuild the legacy solution from clean | `msbuild win32\zoitechat.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64` |
| Build spelling dictionaries for the legacy client | `win32\spelling\build-spelling.bat` |
| Regenerate the legacy Inno Setup script from its T4 template | Re-run the T4 transform for `win32\installer\zoitechat.iss.tt` via the Visual Studio T4 tooling associated with `win32\installer\installer.vcxproj` |

There is no automated unit test suite in this repository. Treat a successful build
(WiX v4 `dotnet build`/`wix build`, and/or legacy `msbuild` when that path is
touched) plus manual smoke-testing of the affected feature as the validation bar,
unless a task adds tests of its own.

## High-level architecture

- **`src\`** — core C application code: `common\` (protocol/back-end logic shared
  by all frontends), `fe-gtk\` (GTK front end — GTK3-based in the current source),
  `fe-text\` (text-only front end), and `dirent\` (Windows compatibility shim).
  This is the HexChat-derived C codebase that the new
  `ZoiteChatAPI` plugin ABI (see `To-Do.md`) is being layered onto.
- **`plugins\`** — the existing built-in C plugins (`checksum`, `exec`, `fishlim`,
  `lua`, `perl`, `python`, `sysinfo`, `upd`), inherited from the HexChat fork.
  **Assumption to verify with Barry:** whether this entire directory is being
  trimmed or fully replaced by the new manifest-driven, three-language
  (C#/Python/Tcl) plugin model in `To-Do.md`, or partially retained. Do not delete
  or extend any plugin here without confirming first.
- **`installer\`** — the WiX v4 Burn-based installer, treated as the authoritative,
  long-term installer. `Product.wxs` and `Bootstrapper\Bundle.wxs` define the
  bundle; `Components\*.wxs` cover `Config`, `Core`, `DotNet`, `GTK4`, `Plugins`,
  `Python312`, and `Tcl`; `UX\` holds the C# bootstrapper application project. Per
  `To-Do.md`, the target installed layout is `ProgramFiles\Fabulor\fabulor.exe`
  plus `Plugins\`, `Runtime\GTK4\`, `Runtime\Python312\`, `Runtime\Tcl\`,
  `Runtime\DotNet\`, and `Config\`, with Installed and Portable feature modes,
  registry entries only in Installed mode, and Fabulor's own updater (not Windows
  Update or MSIX).
- **`win32\`** — the legacy MSVC solution (`zoitechat.sln`), its Inno Setup
  installer (`win32\installer\zoitechat.iss.tt`, T4-templated), and supporting
  scripts (`win32\spelling\build-spelling.bat`, `version-template.ps1`). Being
  superseded by `installer\`: the confirmed long-term direction is exactly two
  installer artefacts (MSI + bootstrapper `.exe`) from `installer\`, with no
  ongoing Inno Setup output. Treat this as legacy/transitional, not a permanent
  parallel path. **Assumption to verify with Barry:** the exact retirement
  timeline/cutover criteria — until confirmed, keep it building rather than
  deleting or disabling it outright.
- **`To-Do.md`** — the authoritative roadmap for the plugin API rework:
  - the `ZoiteChatAPI` C struct/ABI and `UserInfo` type;
  - C# binding (`ZoiteChatContext`, `IZoiteChatPlugin.Init`);
  - embedded Python 3.12 binding (`zoitechat` module, `init_python_binding`);
  - embedded Tcl 8.6 binding (`zoitechat::*` namespace, `init_tcl_binding`);
  - the `plugin.json` manifest schema (id, name, version, language, entrypoint,
    `requires_api_version`, dependencies, capabilities, description, author,
    homepage) under `plugins\<plugin-id>\plugin.json`;
  - a Kahn's-algorithm-based dependency resolver and `IPluginLoader` factory
    (`CSharpLoader`, `PythonLoader`, `TclLoader`);
  - a callback/event system with per-language dispatch and a main-thread
    execution guarantee;
  - a security model (manifest validation, per-plugin interpreter isolation, safe
    mode flag, optional blacklist, lifecycle logging).

  Read this file in full before working on anything plugin- or installer-related —
  it reflects current priorities, not just a backlog.
- **Deprecated non-Windows packaging/build artefacts** — inherited from the
  HexChat fork and not part of the new client direction. Legacy residue; do not
  invest new effort here or use it as a model for new Windows-only work.
- **`meson.build` / `meson_options.txt`** — the legacy cross-platform build system.
  Out of scope for new Windows-only work; do not add options here or treat it as a
  build target to keep in sync with Windows changes. **Assumption to verify with
  Barry:** whether it is scheduled for outright removal or simply left unmaintained.
- **`po\`** and **`.tx\`** — translation strings and Transifex config, inherited
  from the fork; out of scope unless a task specifically targets localisation.
- **`Runtime\`** — embedded language runtimes backing the (current and prospective)
  multi-language plugin system; expect this to grow as the Python 3.12/Tcl 8.6
  embedding work in `To-Do.md` lands.
- **`prompts\`** — a **Foundry Local** prompt library: `manifest.json` plus a set of
  `.prompt` files already covering the plugin rework (`c_abi_and_csharp_binding`,
  `python_binding`, `tcl_binding`, `unified_loader`, `callback_system`,
  `wix_installer`, `plugin_authoring_guides`). This is unrelated to GitHub
  Copilot's own `.github\prompts` mechanism — do not confuse the two, and do not
  merge or move content between them.
- **`docs\`** — plugin authoring documentation for the new API, already written
  and current per `To-Do.md`'s "Documentation & Developer Guides" section:
  `docs\plugin-authoring-guides.md` is the index; `docs\plugins\csharp-plugin-guide.md`,
  `docs\plugins\python-plugin-guide.md`, and `docs\plugins\tcl-plugin-guide.md`
  cover each binding language; `docs\plugins\plugin-schema-and-troubleshooting.md`
  documents the `plugin.json` schema, compatibility rules, safe mode, and the
  troubleshooting workflow. These guides describe the target design (folder
  layout `plugins\<plugin-id>\plugin.json` + entrypoint, full field table for
  `plugin.json`) ahead of implementation — nothing under `ZoiteChatAPI`,
  `IPluginLoader`, or `CallbackEntry` exists in `src\` yet, so treat these guides
  as the authoritative spec to implement against, and keep them updated in step
  with any loader/binding code you write.
- **`.github\workflows\windows-build.yml`** — the only currently relevant CI
  pipeline for day-to-day work; it builds the legacy `win32\zoitechat.sln` path and
  packages the legacy Inno Setup installer.

## Key conventions

- **Use Australian English spelling and metric units** in all new or edited
  documentation, comments, and user-facing strings (e.g. "colour", "organise",
  "kilometres"), unless an upstream API, library, or protocol name requires the
  American spelling verbatim (e.g. `color` in a GTK property name, or a Win32 API
  symbol).
- **Match the existing HexChat-derived C style** — inspect a few neighbouring
  `.c`/`.h` files before assuming a convention, but in general (confirmed from
  `.editorconfig` and files such as `src\common\util.c`): tab indentation (width
  4, for `*.{c,cpp,h,hpp,m}`), Allman-style braces (opening brace on its own line
  for functions and control blocks), and a space before the argument-list
  parenthesis (`func (arg)`, not `func(arg)`). Preserve existing GPL header blocks
  and copyright history at the top of files; do not replace them.
- **C# code style:** space indentation, size 4, per `.editorconfig`. Follow the
  conventions already used in the WiX v4 `installer\UX\` bootstrapper application
  project for new C# code related to the installer or the plugin host.
- **WiX v4 authoring:** keep new installable content in its own `Components\*.wxs`
  fragment, mirroring the existing `Config`/`Core`/`DotNet`/`GTK4`/`Plugins`/
  `Python312`/`Tcl` split, rather than adding large amounts of new content directly
  into `Product.wxs`. Keep component `Id`s, GUIDs, and feature references
  consistent between `Product.wxs` and the `Components\*.wxs` files. Keep
  Installed-mode-only behaviour (e.g. registry entries) properly scoped so it does
  not leak into Portable mode. Validate with `wix build` on the affected `.wxs`
  file, then a full `dotnet build installer\Fabulor.wixproj` (or equivalent
  `msbuild`) before considering an installer change complete.
- **Keep the legacy `win32\` + Inno Setup path building for now.** Treat it as
  transitional, not permanent: do not invest new long-term work there, but do not
  let unrelated changes silently break its build either, until Barry confirms a
  retirement plan.
- **Preserve plugin ABI compatibility as described in `To-Do.md`.** Once the
  `ZoiteChatAPI` struct, `plugin.json` manifest schema, and loader interfaces
  exist, treat them as a public contract — additive, backward-compatible changes
  only, unless a task explicitly calls for a breaking API version bump
  (`requires_api_version` in `plugin.json` exists precisely to manage this).
- **No cross-platform build guidance for new work.** Do not add Meson options,
  non-Windows packaging/build steps, or non-Windows CI matrix entries when implementing new
  features; if touching a legacy cross-platform file is genuinely unavoidable,
  call it out explicitly as a special case rather than a routine part of the
  workflow.
- **Prefer minimal, localised changes.** This is a long-lived fork mid-rework;
  avoid drive-by reformatting or renaming unrelated to the task at hand.

## Assumptions to verify with Barry

These are inferred from `To-Do.md` and repository structure, not confirmed facts.
Treat them as open questions, not settled scope, until Barry confirms:

1. Whether the existing `plugins\` directory of built-in C plugins (`checksum`,
   `exec`, `fishlim`, `lua`, `perl`, `python`, `sysinfo`, `upd`) is being trimmed,
   fully replaced by the new manifest-driven model, or partially retained. Note
   that `docs\plugins\plugin-schema-and-troubleshooting.md` documents a
   `plugins\<plugin-id>\plugin.json` + entrypoint folder layout, which does not
   match the current flat `plugins\checksum\`, `plugins\exec\`, etc. layout — this
   is suggestive, not confirmation, that the existing plugins will be migrated to
   or replaced under the new layout.
2. The exact retirement timeline/cutover criteria for `win32\zoitechat.sln` and
   the legacy Inno Setup installer relative to the WiX v4 `installer\` path. The
   end state is confirmed (two artefacts: MSI + bootstrapper `.exe`, both from
   `installer\`, no ongoing Inno Setup output) — what's still open is exactly
   when/how the cutover happens and what "installer complete" means in scope
   (e.g. full feature parity with the legacy installer, or a defined subset).
3. Whether the GTK front end (`src\fe-gtk\`, currently GTK3-based) is being
   carried forward toward GTK4 as-is, or reworked as part of the plugin API
   modernisation — `Runtime\GTK4\` currently only appears in the installer's
   planned layout in `To-Do.md`, not in `src\` itself.
4. Whether `meson.build`/`meson_options.txt` and deprecated non-Windows
  packaging/build artefacts are scheduled for outright removal, or simply left unmaintained
   alongside the Windows-only rework.
