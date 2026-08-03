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
3. Leave **Portable mode** clear for a normal installation.
4. Keep the default install folder unless you have a specific reason to change
   it.
5. Review the optional features, then select **Install**.
6. Wait for setup to report completion before closing it.

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

The installed shell-integration feature registers Fabulor for `irc://` links.
The client also accepts validated `irc://` and `ircs://` addresses through its
`--url` startup option. Windows-wide `ircs://` association is not yet part of
the production registration and remains a release task.

## Portable Mode

Portable mode creates a `portable-mode` marker and keeps configuration in the
`Config` folder beside `fabulor.exe`. It also omits installed-mode Start menu,
protocol, and theme registration.

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
