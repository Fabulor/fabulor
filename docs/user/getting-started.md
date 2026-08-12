# Getting Started

Status: source-verified draft; installed release-candidate verification is
still required.

This page covers a normal first installation and connection on Windows 11 x64.
The installer contains Fabulor and its required GTK4 runtime; a separate GTK or
language-runtime installation is not required for ordinary use.

## Before Installation

You need:

- a 64-bit Windows 11 computer;
- the Fabulor setup program supplied by the project;
- permission to approve the Windows administrator prompt; and
- the address and account details for an IRC network or bouncer.

Close Fabulor before installing an upgrade, changing installed features, or
uninstalling it.

## Install Fabulor

1. Start `FabulorSetup.exe`.
2. Approve the Windows administrator prompt.
3. Keep the default install folder unless you have a specific reason to change
   it.
4. Leave the Start Menu, Desktop shortcut, and IRC/theme integration options
   selected unless you do not want one of those Windows integrations.
5. Open **Advanced options** only when you need Portable mode, optional plugin
   runtimes, translations, or individual built-in plugins.
6. Select **Install** and wait for setup to report completion.
7. Select **Launch Fabulor** to start the installed client, or **Close** to
   leave setup without launching it.

Installed mode normally places the application under:

```text
C:\Program Files\Fabulor
```

Your personal settings, networks, history, logs, sounds, themes, and add-ons
are kept separately under:

```text
%APPDATA%\Fabulor
```

An upgrade replaces application files while retaining that profile. **Modify**
changes installed features, **Repair** restores the selected installation, and
**Uninstall** removes the installed application. Keep a separate backup of the
profile before making major changes or moving to another computer.

When Fabulor is already installed, setup opens in a maintenance layout. It
shows the detected installation and selected features instead of presenting a
second first-install workflow. Installed-mode integration can create:

- **Fabulor** and **Fabulor Safe Mode** Start Menu shortcuts;
- an optional **Fabulor** Desktop shortcut; and
- the selected IRC protocol and theme-file associations.

Portable mode disables all three installed-mode integration choices.

## Setup Progress And Logs

The progress bar and current status remain visible during setup. Detailed
operation messages are collapsed under **Show details** during normal work and
open automatically if setup reports an error. The error view can copy the
displayed diagnostic details or open the log folder.

Setup writes a timestamped log for every run beneath:

```text
%LOCALAPPDATA%\Fabulor\Installer\Logs
```

The filenames begin with `FabulorSetup-`. Setup retains the ten most recent
ordinary logs and keeps failed-session logs for up to 90 days. These logs are
machine-specific diagnostics and do not roam with the Fabulor user profile.

The client accepts validated `irc://` and `ircs://` addresses through its
`--url` startup option. The production installer currently writes a basic
`irc://` association, but Fabulor is not yet published through the Windows
Registered Applications catalogue and another application can be selected
instead. Windows-wide `ircs://` registration is also not yet present. Complete
default-app registration for both schemes remains a release task tracked by
[issue #298](https://github.com/Fabulor/fabulor/issues/298).

## Portable Mode

Portable mode creates a `portable-mode` marker and keeps configuration in the
`Config` folder beside `fabulor.exe`. It also omits installed-mode Start Menu
and Desktop shortcuts, protocol registration, and theme registration.

Choose portable mode only when you specifically need a self-contained folder.
Do not install portable mode under `C:\Program Files` if you expect to edit its
configuration without administrator permission.

## First Start

On first start, Fabulor opens the **Network List** unless **Skip network list on
startup** is enabled.

Complete the global **User Information** fields:

- **Nick name**: the name other IRC users will normally see;
- **Second choice** and **Third choice**: alternatives used when the preferred
  nickname is unavailable; and
- **User name**: the IRC username sent during connection.

These are defaults. An individual network can override them in its editor.
Avoid placing a password, email address, or other secret in an identity field.

## Make A First Connection

1. Select a network in the **Networks** list.
2. Select **Edit...** if its server, identity, TLS, authentication, or autojoin
   settings need changing.
3. Select **Connect**.
4. Read the connection and authentication messages in the network's server
   tab.
5. When connected, join a channel with **Server > Join a Channel...** or use an
   autojoin entry configured for that network.

The server tab is the authoritative place for connection progress, TLS
messages, authentication results, the message of the day, and connection
errors. Fabulor keeps that output available even when a fast bouncer moves the
view into an autojoined channel.

For detailed network, SASL, ZNC, and SOCKS5 settings, continue with
[Networks And Connections](networks-and-connections.md).

## Start Automatically Next Time

In the selected network's editor, enable **Connect to this network
automatically**. Enable **Skip network list on startup** in the main Network
List only after your automatic connections work reliably.

When several networks connect automatically, each receives its own server tab.
The first automatic connection remains the initial network context while the
others connect in the background.

## Close Fabulor Normally

Use **Fabulor > Quit** or the tray menu's **Quit** command. A normal shutdown
allows preferences and saved input history to be written safely. Closing a
window may minimize Fabulor to the notification area when tray minimization is
enabled.

## If The First Connection Fails

- Reopen **Fabulor > Network List** and confirm the selected server and port.
- Read the last messages in the server tab before retrying.
- Confirm that TLS is enabled when using a TLS-only port.
- Confirm the selected login method matches the credentials issued by the
  network or bouncer.
- Temporarily bypass a configured proxy to separate proxy errors from IRC
  connection errors.
- Do not enable **Accept invalid TLS certificates** merely to silence a
  certificate error.

See [Troubleshooting](troubleshooting.md) for safe mode, installation repair,
runtime loading, display faults, clean-profile diagnosis, and useful bug
reports.
