# Fabulor Plugin Authoring Guides

## Index

This page is the entry point for plugin authoring documentation.

1. [C# Plugin Guide](plugins/csharp-plugin-guide.md)
2. [Python Plugin Guide](plugins/python-plugin-guide.md)
3. [Tcl Plugin Guide](plugins/tcl-plugin-guide.md)
4. [Plugin Schema, Compatibility, and Troubleshooting](plugins/plugin-schema-and-troubleshooting.md)
5. Sample manifest plugins under `samples\plugins\`

## Maintenance Notes

1. Keep examples aligned with the public plugin host contract in [To-Do.md](../To-Do.md) and `src/common/fabulor-plugin-host.h`.
2. Keep these guides consistent with actual loader, validation, and callback behaviour in source.
3. Use Australian English for user-facing documentation.
4. Keep the sample plugins under `samples\plugins\` in sync with the documented manifest schema and current runtime behaviour.
