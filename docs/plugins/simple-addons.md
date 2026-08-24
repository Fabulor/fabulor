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
    helper.dll
```

The required naming rule is:

```text
addons\<addon-name>\<addon-name>.<ext>
```

Examples:

```text
addons\aliases\aliases.tcl
addons\greeter\greeter.py
addons\helper\helper.dll
```

## Runtime Selection

The file extension selects the language runtime:

| Extension | Runtime |
| --- | --- |
| `.tcl` | Tcl |
| `.py` | Python |
| `.dll` | C# (.NET 8 assembly implementing `IFabulorPlugin`) |

Only one primary script should use the folder name. Extra files may live beside it when the runtime supports them.

Native plugin DLLs may still be loaded through the legacy Add-ons GUI for compatibility, but they are trusted local code and are not part of the simple C#/Python/Tcl add-on convention.

## Optional Metadata

Metadata handling depends on the selected runtime. Tcl metadata comments are
optional and fall back to the folder name. Python requires the module name
variable shown below.

Tcl metadata comments:

```text
# Fabulor-Name: Personal aliases
# Fabulor-Version: 1.0
# Fabulor-Description: Adds local helper commands
```

Trusted Python add-ons instead define the metadata variables consumed by the
Python loader:

```python
__module_name__ = "Personal helper"
__module_version__ = "1.0.0"
__module_description__ = "Adds local helper commands"
```

Compiled C# metadata comes from the assembly. Source files and project files may remain beside the primary DLL, but Fabulor loads only the exact `<addon-name>.dll` entrypoint.

## Loading Rules

The intended simple loader behaviour is:

1. Scan direct child folders under `%APPDATA%\Fabulor\addons`.
2. For each folder, look for `<folder-name>.tcl`, `<folder-name>.py`, or `<folder-name>.dll`.
3. Select the runtime from the extension.
4. Load add-ons alphabetically by folder name.
5. Log failures and continue loading the remaining add-ons.

The Python, Tcl, and C# runtimes follow this layout for auto-loading add-ons. Python keeps legacy flat `addons\*.py` loading only as a compatibility fallback. Manual Python loads resolve relative names against the profile `addons` directory. Absolute Python loads are rejected unless they resolve under the profile `addons` directory or under an enabled manifest plugin root. Trusted simple Python add-ons remain in the shared legacy scripting interpreter; manifest Python plugins use separate Python 3.14 subinterpreters. Simple Tcl add-ons receive isolated interpreters and may register user commands directly; Tcl event callbacks remain manifest-only. Simple C# add-ons load into collectible managed load contexts and may use the shared context API without manifest capability declarations because profile add-ons are trusted local code.

The Tcl and managed C# hosts are built into Fabulor rather than installed as separate legacy plugin DLLs, so they do not appear as standalone entries in the Plugins and Scripts window. Loaded add-ons report their own initialization messages and expose their registered behavior.

Trusted simple Tcl add-ons can query the local Windows uptime with
`fabulor::get_windows_uptime_seconds`. The helper returns whole seconds from
the Windows boot timer and avoids launching an external PowerShell or WMI
process. It is not available to manifest plugins.

Fabulor does not compile C# source at startup. Build the add-on for `.NET 8`, reference the installed `Runtime\DotNet\Fabulor.PluginAbstractions.dll`, and place the resulting DLL and any private dependencies in the add-on folder. Restart Fabulor to load or reload simple Tcl and C# add-ons.

After startup, Fabulor prints a sorted `Fabulor loaded plugins and add-ons` report in the initial server window. The report combines active native plugins, Python modules registered with the plugin GUI, and successfully initialised simple Tcl and C# add-ons. Discovery failures are logged separately and are not counted as loaded.

The Add-ons GUI uses the profile `addons` directory as the trust boundary. When a user selects a supported add-on file from outside that directory, Fabulor copies it into `addons` by basename and loads the copied file. Paths containing control characters or quotes are rejected by the GUI loader because they cannot be passed safely to the legacy script-runtime command hooks.

Maintained simple C#, Python, and Tcl greeters live under
`samples\plugins\simple-csharp-greeter\`,
`samples\plugins\simple-python-greeter\`, and
`samples\plugins\simple-tcl-greeter\`. Each sample follows the exact
folder/entrypoint basename rule and intentionally omits `plugin.json`.

## Manifest Plugins

Use `plugins\<plugin-id>\plugin.json` only for advanced packaged plugins that need dependency resolution, capability declarations, strict API versioning, or complex runtime setup.

The manifest host is disabled by default. Users can opt in through
**Settings > Preferences > Chatting > Advanced > Enable manifest plugins
(requires restart)**, accept the trusted-code warning, and restart Fabulor.
The developer/testing override remains available as:

```text
FABULOR_ENABLE_MANIFEST_PLUGINS=1
```

Manifest plugins currently support C#, Python, and Tcl only. Native C/C++ DLL plugins remain on the legacy loader path.
The `--no-plugins` safe-mode option takes precedence over both enable paths.
