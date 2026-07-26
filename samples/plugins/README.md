# Fabulor Plugin Samples

This directory contains one simple add-on and one advanced manifest plugin for
each supported language.

## Simple Add-ons

Simple add-ons are intended for personal scripts and small trusted helpers.
They do not use `plugin.json`.

| Language | Sample | Profile entrypoint |
| --- | --- | --- |
| C# | `simple-csharp-greeter` | `addons\simple-csharp-greeter\simple-csharp-greeter.dll` |
| Python | `simple-python-greeter` | `addons\simple-python-greeter\simple-python-greeter.py` |
| Tcl | `simple-tcl-greeter` | `addons\simple-tcl-greeter\simple-tcl-greeter.tcl` |

Copy the complete Python or Tcl sample folder under
`%APPDATA%\Fabulor\addons`. For C#, build the project and place
`simple-csharp-greeter.dll` in a profile folder with the same name. Fabulor
supplies `Fabulor.PluginAbstractions.dll`.

## Manifest Plugins

The `example.*.greeter` folders demonstrate advanced plugins with manifests,
capability declarations, and explicit entrypoints. They belong under an
enabled manifest plugin root rather than the simple profile `addons` root.

See [the plugin authoring guides](../../docs/plugin-authoring-guides.md) for
the current loader, trust, build, and capability rules.
