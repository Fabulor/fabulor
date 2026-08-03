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

- [Getting started](getting-started.md)
- [Networks and connections](networks-and-connections.md)
- [Main window](main-window.md)
- [Commands](commands.md)
- [Settings reference](settings-reference.md)
- [Preferences](preferences.md)
- [Themes and colours](themes-and-colours.md)
- [Sounds and alerts](sounds-and-alerts.md)
- [Input history and conversation logs](history-and-logs.md)
- [Add-ons](addons.md)
- [Security and privacy](security-and-privacy.md)
- [Troubleshooting](troubleshooting.md)
- [Migrating from XChat or HexChat](migration-from-xchat-and-hexchat.md)
- [Glossary](glossary.md)

## Planned

All planned source drafts and the privacy-safe screenshot set are now
available. Installed release-candidate verification and final editorial review
remain before the user manual is complete.

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

Historical HexChat and XChat documentation are research references, not copy
sources. Their factual IRC explanations and topic structure may guide the
manual, but substantial passages must not be copied. The
[reference map](hexchat-reference-map.md) records how each historical HexChat
chapter is handled.

The user manual limits external links to the two approved theme sources:

- [HexChat colour themes](https://hexchat.github.io/themes.html), for optional
  `.hct` colour themes; and
- [OpenDesktop](https://www.opendesktop.org/), the approved source for optional
  GTK4 desktop themes.

All Fabulor downloads, instructions, support, security information, and plugin
documentation must use repository-relative links.
