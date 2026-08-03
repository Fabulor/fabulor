# Add-ons

Status: source-verified draft; installed release-candidate verification is
still required.

Fabulor supports add-ons written in C#, Python, and Tcl. Add-ons can add
commands, respond to events, display information, or automate repetitive
tasks. They run on your computer as part of Fabulor, so install only code whose
source and origin you trust.

For the wider local-code and data boundary, see
[Security and privacy](security-and-privacy.md).

This page is for people installing and managing add-ons. Authors should use
the [plugin authoring guides](../plugin-authoring-guides.md).

## Before Installing An Add-On

An add-on is executable code, not a passive theme or configuration file. A
malicious add-on can read or change files available to your Windows account,
send network traffic, or issue commands through Fabulor.

Before installing one:

1. obtain it from a source you trust;
2. read its instructions and, when practical, its source code;
3. confirm that it is intended for Fabulor and its current plugin API;
4. check which files the package contains; and
5. back up the Fabulor profile if the add-on will maintain important data.

Capability declarations used by advanced manifest plugins restrict access to
cooperating Fabulor APIs. They are not a Windows security sandbox.

## Add-On Locations

For a normal installed copy of Fabulor, personal add-ons belong under:

```text
%APPDATA%\Fabulor\addons
```

Portable mode uses the `addons` directory beneath the portable `Config`
directory.

Do not put personal add-ons in `C:\Program Files\Fabulor`. The installer owns
that directory and may replace its contents during an update or repair.

Advanced manifest plugins use a separate `plugins` directory, described later
on this page. Keep the two models separate.

## Simple Add-Ons

Simple add-ons are the recommended choice for personal scripts and small
extensions. They do not need a `plugin.json` manifest.

Each add-on has one folder and one primary file. The folder name and primary
filename must match exactly:

```text
addons\<add-on-name>\<add-on-name>.<extension>
```

For example:

```text
%APPDATA%\Fabulor\addons\aliases\aliases.tcl
%APPDATA%\Fabulor\addons\greeter\greeter.py
%APPDATA%\Fabulor\addons\helper\helper.dll
```

The extension selects the runtime:

| Extension | Add-on type |
| --- | --- |
| `.py` | Python 3.14 script |
| `.tcl` | Tcl 8.6 script |
| `.dll` | C# .NET 8 assembly implementing `IFabulorPlugin` |

Supporting files and private dependencies may sit beside the primary file.
Fabulor loads only the exact primary filename selected by the folder-name
rule.

### Install A Simple Add-On

1. Close Fabulor.
2. Create the add-on's folder under the profile `addons` directory.
3. Place the matching primary file and any supplied supporting files in that
   folder.
4. Start Fabulor.
5. Read the initial server window for the add-on's startup message and the
   **Fabulor loaded plugins and add-ons** report.
6. Use any status or help command documented by the add-on.

Fabulor scans direct child folders alphabetically. A failure in one add-on is
reported without preventing unrelated add-ons from loading.

### Language Notes

Python add-ons should normally import `fabulor`. The intentional `xchat` and
`hexchat` compatibility imports remain available for suitable older scripts.
The former `zoitechat` module is retired.

Tcl add-ons use the `fabulor::*` namespace. The former product namespace is
retired. Each simple Tcl add-on receives its own interpreter and can register
commands, but event callbacks require the advanced manifest model.

C# add-ons must already be compiled for .NET 8 and implement the current
`IFabulorPlugin` contract. Fabulor does not compile C# source at startup. Keep
private dependency DLLs beside the primary assembly, but do not copy
`Fabulor.PluginAbstractions.dll`; Fabulor supplies its installed contract
assembly.

The Tcl and C# hosts are built into Fabulor. They do not appear as separate
runtime entries in **Window > Plugins and Scripts**. Their successfully loaded
add-ons still appear in the startup report and may print their own
initialisation messages.

## Manual Loading And The Plugins Window

Use **Add-ons > Load Plugin or Script...** to select a supported file for an
immediate manual load. Fabulor copies a file selected outside the profile
`addons` directory into that trusted directory before loading it. This manual
path is useful for testing and legacy compatibility; the matching folder and
filename layout remains the dependable way to auto-load an add-on at every
start.

Open **Window > Plugins and Scripts** to inspect plugin and script entries that
participate in the legacy plugin list. The window provides load, reload, and
unload controls. Built-in Tcl and C# hosts, and therefore their simple add-ons,
do not necessarily appear as conventional entries in this list.

![Fabulor's Plugins and Scripts window listing loaded add-ons with their
versions, files, descriptions, and load-management controls.](images/plugins-and-scripts.png)

The corresponding commands are:

```text
/LOAD [-e] <file>
/RELOAD <name>
/UNLOAD <name>
```

Loading a file this way executes it with Fabulor's permissions. Do not use
manual loading to bypass an add-on's installation or trust instructions.

## Update, Disable, Or Remove A Simple Add-On

For a Tcl or C# add-on, close and restart Fabulor after replacing its files.
Restarting is also the most reliable way to test a Python update because it
clears the shared scripting interpreter's previous state.

To disable one add-on without deleting it:

1. close Fabulor;
2. move its folder outside the profile `addons` directory, or rename the
   primary file so it no longer matches the folder; and
3. start Fabulor and check the startup report.

To remove it permanently, close Fabulor and delete only that add-on's folder.
Back up any settings or data stored there first.

## Advanced Manifest Plugins

Manifest plugins are intended for packaged extensions that need explicit API
versions, dependencies, capabilities, or event callbacks. They support C#,
Python, and Tcl, but not native C or C++ entrypoints.

An installed-profile manifest plugin uses this layout:

```text
%APPDATA%\Fabulor\plugins\<plugin-id>\plugin.json
```

Its entrypoint and supporting files remain in the same plugin folder. Portable
mode uses `Config\plugins`.

Manifest plugins are disabled by default. To enable them:

1. open **Settings > Preferences > Chatting > Advanced**;
2. enable **Enable manifest plugins (requires restart)**;
3. read and accept the trusted-code warning; and
4. restart Fabulor.

The preference enables discovery for all eligible manifest plugins in the
profile. Review every plugin before enabling it. Fabulor validates each
manifest, resolves dependencies, checks the requested API version and
capabilities, and skips invalid or incompatible plugins. A rejection is
reported in the server window.

To disable manifest plugins again, clear the preference and restart Fabulor.
The files remain in the `plugins` directory but are not auto-loaded.

### Disable One Manifest Plugin

Create or edit this profile file:

```text
%APPDATA%\Fabulor\plugin-blacklist.txt
```

List one manifest plugin id per line. Blank lines and lines beginning with `#`
are ignored. Restart Fabulor after changing the file. Removing an id from the
file makes that plugin eligible at the next normal start.

## Start Without Add-Ons

If Fabulor fails to start or behaves incorrectly after an add-on change, start
it once with all automatic plugin loading disabled:

Installed mode can provide a **Fabulor Safe Mode** Start-menu shortcut. It
also disables automatic network connections so the client opens without
immediately processing network traffic. The equivalent plugin-only command is:

```text
"C:\Program Files\Fabulor\fabulor.exe" --no-plugins
```

The `--no-plugins` option takes precedence over the manifest preference and
the developer override. It is a diagnostic start, not a persistent setting.
See [Troubleshooting](troubleshooting.md) for the complete safe-mode and clean
temporary-profile sequence.

While Fabulor is closed, move or rename the suspected add-on folder. For a
manifest plugin, you can instead add its id to `plugin-blacklist.txt`. Then
start Fabulor normally. Re-enable add-ons one at a time when the cause is not
obvious.

If Fabulor cannot be started from a command prompt, close it and temporarily
rename the profile `addons` and `plugins` directories. Restore one directory
at a time after the client starts normally.

## Shipped Optional Plugins

Fabulor can install first-party native plugins alongside the client. Their
availability depends on the installer features selected for that installation.

- **Checksum** adds checksum handling to supported file-transfer activity.
- **Exec** provides `/EXEC`, which runs an operating-system command. Command
  input is length-bounded, but only commands you trust are safe to execute.
- **FiSHLiM** provides its key manager and commands including `/FISHLIM`,
  `/SETKEY`, `/DELKEY`, and `/KEYX`.
- **Sysinfo** provides `/SYSINFO` and **Window > Display System Info**.
- **Python interface** provides Python script loading and management.

Use `/HELP <command>` for the syntax supplied by the version currently loaded.
These installed components are not personal add-on folders and should be
managed through the installer rather than deleted from `Program Files`.

## Troubleshooting

### Nothing Appears To Load

- Confirm the add-on is beneath the active profile, not an old or backup
  profile.
- Confirm the folder and primary filename match exactly.
- Confirm the extension is `.py`, `.tcl`, or `.dll`.
- Restart Fabulor and read the initial server window.
- Check **Window > Plugins and Scripts**, while remembering that simple Tcl and
  C# add-ons are reported through startup messages instead of conventional host
  entries.

### Python Reports An Import Error

- Use `import fabulor` for the current API.
- Retain `import xchat` or `import hexchat` only when the script is compatible
  with those maintained adapters.
- Replace any retired `import zoitechat` reference.
- Confirm the Python feature is installed.

### A C# Add-On Is Skipped

- Confirm it targets .NET 8.
- Confirm the main DLL name matches the folder.
- Confirm it implements the current `IFabulorPlugin` interface.
- Put private dependencies beside the main DLL.
- Rebuild add-ons that reference former product-named contract types.

### A Tcl Add-On Is Skipped

- Confirm the script uses the current `fabulor::*` namespace.
- Confirm its registered command names do not conflict with another simple Tcl
  add-on.
- Confirm the Tcl feature is installed.

### A Manifest Plugin Is Skipped

- Confirm manifest plugins are enabled and Fabulor has been restarted.
- Check that `plugin.json` is in `plugins\<plugin-id>` and that its id matches
  the package instructions.
- Check the server window for an invalid field, missing dependency, unsupported
  API version, denied capability, blacklist, or unsafe-path message.
- Do not place plugin roots, folders, manifests, or entrypoints behind Windows
  symbolic links or junctions.

For schema details and author-facing diagnostics, see
[plugin schema and troubleshooting](../plugins/plugin-schema-and-troubleshooting.md).
