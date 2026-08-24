# Fabulor Plugin Authoring Guides

## Index

This page is the entry point for plugin authoring documentation.

1. [Simple Add-ons](plugins/simple-addons.md)
2. [C# Plugin Guide](plugins/csharp-plugin-guide.md)
3. [Python Plugin Guide](plugins/python-plugin-guide.md)
4. [Python API Reference](plugins/python-api-reference.md)
5. [Tcl Plugin Guide](plugins/tcl-plugin-guide.md)
6. [Plugin Schema, Compatibility, and Troubleshooting](plugins/plugin-schema-and-troubleshooting.md)
7. Simple and manifest samples under `samples\plugins\`

For personal scripts and aliases, prefer the simple add-on layout:

```text
%APPDATA%\Fabulor\addons\<addon-name>\<addon-name>.tcl
%APPDATA%\Fabulor\addons\<addon-name>\<addon-name>.py
%APPDATA%\Fabulor\addons\<addon-name>\<addon-name>.dll
```

Use manifest plugins only when an add-on needs advanced dependency, capability, or versioning metadata. Python simple add-ons load only from the profile `addons` trust boundary; Python manifest entrypoints load only from the enabled manifest roots.

The user-facing Fabulor plugin model is C#, Python, and Tcl. Legacy native C plugin DLLs remain an installed first-party compatibility surface, not a manifest language target for new third-party plugins.

## Maintenance Notes

1. Keep examples aligned with the public plugin host contract in [To-Do.md](../To-Do.md) and `src/common/fabulor-plugin-host.h`.
2. Keep these guides consistent with actual loader, validation, and callback behaviour in source.
3. Use Australian English for user-facing documentation.
4. Keep the simple and manifest plugins under `samples\plugins\` in sync with the documented loader, manifest schema, and current runtime behaviour.
5. Keep the simple add-on guide aligned with the implemented `addons\<name>\<name>.<ext>` loader.
