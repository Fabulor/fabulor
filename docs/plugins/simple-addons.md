# Simple Add-ons

## Scope

Simple add-ons are the preferred path for personal scripts, aliases, and small helper tools.

They avoid `plugin.json`. The folder name and file extension provide the structure Fabulor needs.

## Folder Layout

Each add-on lives in its own folder under the user `addons` directory:

```text
%APPDATA%\Fabulor\addons\
  aliases\
    aliases.tcl

  greeter\
    greeter.py

  helper\
    helper.cs
```

The required naming rule is:

```text
addons\<addon-name>\<addon-name>.<ext>
```

Examples:

```text
addons\aliases\aliases.tcl
addons\greeter\greeter.py
addons\helper\helper.cs
```

## Runtime Selection

The file extension selects the language runtime:

| Extension | Runtime |
| --- | --- |
| `.tcl` | Tcl |
| `.py` | Python |
| `.cs` | C# |

Only one primary script should use the folder name. Extra files may live beside it when the runtime supports them.

## Optional Metadata

Metadata can be provided in comments at the top of the script. If metadata is absent, Fabulor uses the folder name.

Tcl and Python:

```text
# Fabulor-Name: Personal aliases
# Fabulor-Version: 1.0
# Fabulor-Description: Adds local helper commands
```

C#:

```csharp
// Fabulor-Name: Helper
// Fabulor-Version: 1.0
// Fabulor-Description: Small C# helper add-on
```

## Loading Rules

The intended simple loader behaviour is:

1. Scan direct child folders under `%APPDATA%\Fabulor\addons`.
2. For each folder, look for `<folder-name>.tcl`, `<folder-name>.py`, or `<folder-name>.cs`.
3. Select the runtime from the extension.
4. Load add-ons alphabetically by folder name.
5. Log failures and continue loading the remaining add-ons.

The Python runtime now follows this layout for auto-loading Python add-ons and keeps legacy flat `addons\*.py` loading only as a compatibility fallback. Manual Python loads resolve relative names against the profile `addons` directory. Absolute Python loads are rejected unless they resolve under the profile `addons` directory or under an enabled manifest plugin root.

The Add-ons GUI uses the profile `addons` directory as the trust boundary. When a user selects a supported add-on file from outside that directory, Fabulor copies it into `addons` by basename and loads the copied file. Paths containing control characters or quotes are rejected by the GUI loader because they cannot be passed safely to the legacy script-runtime command hooks.

## Manifest Plugins

Use `plugins\<plugin-id>\plugin.json` only for advanced packaged plugins that need dependency resolution, capability declarations, strict API versioning, or complex runtime setup.

The manifest host is currently disabled by default while the API is being hardened. It can be enabled for development with:

```text
FABULOR_ENABLE_MANIFEST_PLUGINS=1
```
