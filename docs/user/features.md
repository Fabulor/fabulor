# Features

Fabulor is a native GTK4 IRC client for Windows 11 x64. This page summarises
the features available in the current client and links to the relevant manual
chapters for operating details.

## IRC Client

- Connect to multiple IRC networks and channels at the same time.
- Organise networks, servers, identities, passwords, and automatic channel
  joins through the Network List.
- Use direct server connections or an IRC bouncer such as ZNC.
- Connect with TLS, SASL authentication, client certificates, or a SOCKS5
  proxy where required.
- Open registered `irc://` and `ircs://` links in a running or newly started
  Fabulor instance.
- Browse channel lists, private conversations, user information, ban lists,
  ignore lists, friends, file transfers, raw logs, and captured URLs.
- Send and receive IRC messages, notices, actions, CTCP requests, and DCC file
  transfers.

See [Networks and connections](networks-and-connections.md),
[Main window](main-window.md), and [Commands](commands.md).

## IRCv3 Support

Fabulor negotiates supported IRCv3 capabilities with each server. Current
support includes:

- SASL authentication, including PLAIN, EXTERNAL, and supported SCRAM
  mechanisms;
- server timestamps, account tags and notifications, away notifications,
  extended joins, userhost-in-names, and multi-prefix;
- message tags, echoed messages, bounded batch processing, and labelled
  responses; and
- on-demand `draft/chathistory` requests when the connected server or bouncer
  advertises that capability.

Capability availability varies by network. Fabulor reports unsupported chat
history requests without leaving them pending. See
[IRCv3 support](../protocol/ircv3-support.md) for the implementation and
verification status.

## Windows Integration

- Follow the active Windows light or dark appearance when the system desktop
  theme is selected.
- Minimise to the notification area and restore the main window from its tray
  menu.
- Display desktop notifications and play user-assigned event sounds.
- Use the Windows credential facilities supported by the Network List.
- Start with the installed Safe Mode shortcut when troubleshooting themes or
  add-ons.

See [Getting started](getting-started.md),
[Sounds and alerts](sounds-and-alerts.md), and
[Troubleshooting](troubleshooting.md).

## Appearance And Input

- Select the system desktop theme or import compatible GTK4 themes.
- Import `.hct` IRC colour themes or customise individual chat colours.
- Configure fonts, timestamps, channel-switcher layout, user-list width,
  topic display, and other interface preferences.
- Use spell checking with suggestions and a persistent personal dictionary.
- Insert Unicode emoji and search the flag picker by country name or code.
- Select and automatically copy transcript text, and open detected links in
  the default browser.

See [Preferences](preferences.md),
[Themes and colours](themes-and-colours.md), and
[Main window](main-window.md).

## History And Logs

- Keep input history separately for each network and channel or private
  conversation.
- Write conversation logs using the same network-and-channel organisation.
- Clear the current conversation's saved input history or logs from Fabulor.
- Collect an optional UI performance log for investigating slow channel or
  network switching.

History and logging remain under the user's control. See
[Input history and conversation logs](history-and-logs.md) and
[Security and privacy](security-and-privacy.md).

## Add-ons And Automation

- Load Fabulor add-ons written in C#, Python, or Tcl.
- Use per-add-on manifests, declared capabilities, contained runtime roots,
  and explicit enablement for the manifest plugin host.
- Manage loaded plugins and scripts from the client.
- Define aliases, user commands, event actions, and keyboard shortcuts.
- Use the optional bounded `/EXEC` plugin when command execution is needed.

Add-on availability depends on the installer features selected for the local
installation. See [Add-ons](addons.md), [Commands](commands.md), and the
[plugin authoring guides](../plugin-authoring-guides.md).

## Security And Maintenance

- Store user configuration and writable data under the Fabulor profile rather
  than the installation directory.
- Redact sensitive service-command text from the visible transcript.
- Validate imported themes and add-on paths before loading them.
- Run automated CodeQL, secret-scanning, repository-lint, test, and Windows
  build workflows in the project repository.
- Produce a bundled-component licence inventory and CycloneDX software bill
  of materials with installer builds.

Release candidates are currently unsigned. Consult the
[security and privacy guide](security-and-privacy.md), the project
[security policy](../../SECURITY.md), and the
[code-signing policy](../../CODE_SIGNING_POLICY.md) for the current release
boundaries.

## Platform Scope

Fabulor targets Windows 11 x64. Windows 10 use is best-effort and unsupported.
Features may also depend on the connected IRC network, negotiated IRCv3
capabilities, selected installer components, and enabled add-ons.
