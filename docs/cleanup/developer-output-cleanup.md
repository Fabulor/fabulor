# Developer Output Cleanup

The repository includes a guarded cleanup command for reproducible local build
output. It is preview-only unless removal is explicitly requested:

```powershell
pwsh -NoProfile -File .\tools\clean-development-output.ps1
```

Review the listed paths, close Fabulor and any active build tools, then remove
the ordinary intermediate output:

```powershell
pwsh -NoProfile -File .\tools\clean-development-output.ps1 -Apply
```

Release installer products are retained by that command. Remove those as a
separate, explicit operation only when a complete installer rebuild is
intended:

```powershell
pwsh -NoProfile -File .\tools\clean-development-output.ps1 -Apply -IncludeInstallerArtifacts
```

## Ordinary Cleanup Scope

- the root `build`, `builddir`, `html`, and `.vs` output trees;
- WiX, bootstrapper, and managed intermediate `obj` and build trees;
- managed and sample `bin`/`obj` directories;
- Python bytecode caches;
- generated Doxygen temporary files, tag files, and WiX debug databases; and
- the bootstrapper build log.

The optional installer-artifact switch additionally removes `installer\bin`
and `installer\UX\bin`.

## Protected Scope

The command derives its repository root from its own tracked location,
canonicalizes every candidate, and refuses paths outside that worktree. It
never targets:

- `.git`, tracked source, or unlisted local files;
- `Runtime`, which is the dependency payload used by development builds;
- `.vscode` or the retained local `dos2unix.exe` maintenance tool;
- `%APPDATA%\Fabulor`, `C:\Program Files\Fabulor`, or the independent add-ons
  checkout; or
- the external `C:\zoitechat-build` staging tree.

Cleanup also stops rather than traversing a Windows reparse point or a tree
that cannot be fully inspected. If old build output has permissions from a
different account, remove it from the same account that created it instead of
weakening the command's containment checks.

After cleanup, rebuild the affected native, managed, or installer targets
before testing or packaging. The command does not download dependencies or
restage the external production payload.
