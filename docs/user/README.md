# Fabulor User Manual

This directory is the single source for Fabulor user documentation. GitHub
renders the Markdown files directly, and any future documentation site must be
generated from these same files rather than maintained separately.

The manual describes the supported Windows 11 x64 product. Instructions must be
verified against an installed release candidate before they are marked
complete. Historical XChat and HexChat documentation may inform the structure
and IRC concepts, but Fabulor's current interface and behaviour are
authoritative.

## Available

- [Input history and conversation logs](history-and-logs.md)

## Planned

The following pages will be added in contained documentation stages:

1. `getting-started.md`
   - installation, first start, identity, and first connection;
2. `networks-and-connections.md`
   - Network List, TLS, SASL, ZNC, SOCKS5, auto-connect, and autojoin;
3. `main-window.md`
   - server and channel tabs, transcript, input box, user list, menus, tray,
     emoji, flags, and clipboard behaviour;
4. `commands.md`
   - command syntax, built-in command reference, aliases, and context;
5. `preferences.md`
   - current Preferences pages and persistence behaviour;
6. `themes-and-colours.md`
   - Fabulor colour themes, `.hct` import, GTK4 desktop themes, and system
     appearance;
7. `sounds-and-alerts.md`
   - sound events, notifications, away state, and tray alerts;
8. `addons.md`
   - installing, enabling, inspecting, troubleshooting, and safely removing
     C#, Python, and Tcl add-ons;
9. `security-and-privacy.md`
   - TLS policy, credential handling, local data, add-on trust, and update
     status;
10. `troubleshooting.md`
    - connection, display, spell-checking, add-on, installer, and performance
      diagnosis;
11. `migration-from-xchat-and-hexchat.md`
    - supported concepts, intentional compatibility, and retired features; and
12. `glossary.md`
    - IRC and Fabulor terminology.

Plugin authors should use the separate
[plugin authoring guides](../plugin-authoring-guides.md).

## Writing Rules

- Write original Fabulor-specific prose.
- Verify menu names, commands, paths, defaults, and persistence against the
  installed client.
- Target ordinary users first; put implementation details in architecture or
  plugin documentation.
- Use Windows 11 paths and terminology unless a feature explicitly supports
  portable mode.
- Do not promise inherited XChat or HexChat behaviour merely because similar
  code remains.
- Add screenshots only when they clarify a workflow that text cannot explain
  efficiently.
- Keep screenshots under `docs/user/images` and exclude personal servers,
  nicknames, credentials, IP addresses, and private conversations.
- Link to existing technical documentation instead of duplicating it.
- Record unsupported or retired behaviour explicitly where users migrating
  from XChat or HexChat are likely to expect it.

## Reference Policy

The [HexChat manual](https://hexchat.readthedocs.io/en/latest/) and
[XChat documentation](https://xchat.org/docs/) are historical references, not
copy sources. Their factual IRC explanations and topic structure may guide the
manual, but substantial passages must not be copied. The
[reference map](hexchat-reference-map.md) records how each HexChat chapter is
handled.
