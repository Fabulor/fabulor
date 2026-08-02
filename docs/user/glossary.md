# Glossary

Status: source-verified draft; installed release-candidate verification is
still required.

This glossary explains IRC and Fabulor terms used throughout the user manual.
Network-specific services, channel rules, and account terminology may differ.

## A

### Add-On

An optional component that extends Fabulor. Trusted simple add-ons use C#,
Python, or Tcl files beneath the profile `addons` directory. Advanced plugins
use a manifest and explicit capabilities beneath the profile `plugins`
directory. See [Add-ons](addons.md).

### Alias

A user-defined command that expands into another command or sequence. Aliases
can reduce repeated typing, but they run in the current tab unless their
definition deliberately changes context.

### Auto-Connect

A Network List setting that tells Fabulor to connect to a network when the
client starts.

### Autojoin

A per-network list of channels that Fabulor joins after the connection and
authentication sequence completes.

### Away

An IRC state indicating that the user is not currently available. Away status
does not disconnect the client. A bouncer may maintain or report away state
while the local client is disconnected.

## B

### Bouncer

A service that stays connected to IRC on a user's behalf and relays traffic
when the IRC client connects to it. A bouncer may provide playback, multiple
networks, and persistent presence. ZNC is a commonly used IRC bouncer.

## C

### Capability

The word has two distinct uses in Fabulor:

- an IRC capability is a feature negotiated with a server during connection;
  and
- a manifest-plugin capability is permission to use a specific host operation
  or event boundary.

The surrounding page should make the intended meaning clear.

### Channel

A named IRC conversation shared by multiple users. Channel names commonly
begin with `#`, although a network may support other prefixes.

### Channel Operator

A user granted channel-management privileges, commonly shown with `@` or an
operator icon. Exact privileges and rank names depend on the network and
channel modes.

### Channel Switcher

The tree or tab control used to move between server, channel, and private
message contexts. Fabulor can group channels beneath their network server tab.

### Client Certificate

A certificate and matching private key presented by Fabulor to an IRC server
when the network uses certificate-based client authentication. This is
different from the server certificate that Fabulor verifies during a TLS
connection.

### Command

An instruction entered into the input box with a leading `/`, such as
`/JOIN #channel`. Enter `//` when ordinary message text must begin with a
literal slash. See [Commands](commands.md).

### Context

The current server, channel, or private-message tab in which a command or
action runs. Context matters for commands that use the active network,
channel, or nickname.

### CTCP

Client-To-Client Protocol. CTCP requests are IRC messages used for functions
such as version queries, actions, time queries, and DCC negotiation. Networks
and users may filter them.

## D

### DCC

Direct Client-to-Client communication, traditionally used for direct chat and
file transfer. DCC may require reachable ports and can be affected by NAT,
firewalls, proxies, and the other user's client.

### Desktop Theme

A GTK4 theme controlling toolkit widgets such as menus, buttons, lists,
fields, and windows. It is separate from the IRC transcript colour theme. See
[Themes and colours](themes-and-colours.md).

## E

### Event

Something Fabulor can display or react to, such as a channel message, join,
quit, highlight, or connection result. Events can have display formats, sound
assignments, notification behavior, and add-on callbacks.

## H

### Highlight

A message that matches the user's nickname or another configured highlight
rule. Highlights can use distinct transcript colours, sounds, notifications,
or taskbar attention.

### History

Commands and messages previously entered into the input box. Fabulor can save
history separately for each network and tab. History is not the same as a
conversation log.

### Hostmask

An IRC identity traditionally written in the form:

```text
nickname!username@host
```

Hostmasks can be used by bans, ignores, and access lists, but they are not
always stable or unique identifiers.

## I

### Identd

A historical service that answered username queries from an IRC server.
Fabulor does not include the retired built-in Identd service.

### IRC

Internet Relay Chat, the protocol and network model used by Fabulor. An IRC
network contains servers, channels, users, services, and negotiated features.

### IRC URI

An address beginning with `irc://` or `ircs://` that identifies an IRC server
and may identify one channel. The `ircs` scheme requests TLS. A URI identifies
a server hostname rather than a saved Fabulor network name, so one shared
bouncer hostname cannot distinguish several ZNC virtual networks.

### IRC Services

Network-operated service accounts such as NickServ, ChanServ, or MemoServ.
Names and commands vary by network. Service passwords should be sent only to
the expected service on the intended network.

## L

### Lag

The delay between Fabulor and the IRC server. The lag meter estimates network
round-trip delay; it does not by itself measure interface rendering delays or
bouncer playback time.

### Log

A file containing text displayed in a server, channel, or private-message tab.
Logs can contain private or sensitive information and are separate from input
history. See [Input history and conversation logs](history-and-logs.md).

## M

### Manifest Plugin

An advanced add-on described by `plugin.json`. Manifest plugins are disabled
by default, require explicit user opt-in, declare capabilities, and run within
Fabulor's language-specific host boundaries.

### Marker Line

A horizontal line in the transcript indicating the boundary between earlier
text and newer activity. It can help locate messages received since the tab
was last read.

### Message Of The Day (MOTD)

Text sent by an IRC server during connection. It commonly contains network
information, policies, and support details. Fabulor displays it in the server
tab.

### Mode

An IRC setting applied to a user or channel. Modes can represent privileges,
channel restrictions, account state, or network-specific behavior. Their
letters and meanings are defined by the network.

## N

### Network

A group of connected IRC servers sharing users and channels. In Fabulor, a
Network List entry also holds the servers, identity, authentication, TLS,
proxy, auto-connect, and autojoin settings used for that connection.

### Nickname Or Nick

The visible name identifying a user on an IRC network. A nickname may be
registered with network services, but it should not be treated as permanent
proof of identity unless the network confirms account authentication.

### Notice

An IRC message intended not to trigger an automatic reply. Servers, services,
channels, and users can send notices, and Fabulor routes them according to the
current event and display settings.

## O

### Operator

Depending on context, either a channel operator with local channel privileges
or an IRC operator with network-administration privileges. They are different
roles.

## P

### Palette Or Colour Theme

The colours used by the IRC transcript, selection, marker line, activity
states, and spell-check indicator. Fabulor palettes use `colors.conf` and can
be distributed as `.hct` files. They do not change GTK4 widgets.

### Plugin

A general term for an optional component loaded by Fabulor. The manual uses
**simple add-on** and **manifest plugin** when their different trust and
installation models matter.

### Portable Mode

An installation mode that stores configuration beneath the `Config` directory
beside `fabulor.exe` rather than `%APPDATA%\Fabulor`. It omits installed-mode
Start menu, protocol, and theme registration.

### Private Message Or Query

A conversation addressed to a user rather than a channel. Fabulor normally
opens it in a separate context beneath the relevant network.

### Profile

The directory containing a user's Fabulor settings and local data. Installed
mode normally uses `%APPDATA%\Fabulor`; portable mode uses `Config` beside the
executable.

### Proxy

An intermediary used to reach an IRC server or DCC peer. Fabulor supports
several proxy choices; SOCKS5 is the reviewed modern option for proxying IRC
and DCC TCP connections. A proxy does not replace TLS server verification.

## S

### Safe Mode

A diagnostic start that disables automatic connections and third-party
plugins. The installed **Fabulor (Safe Mode)** shortcut starts Fabulor with
`--no-auto --no-plugins`.

### SASL

Simple Authentication and Security Layer. IRC networks commonly use SASL to
authenticate an account during connection, before channels are joined. The
selected mechanism and credentials must match the network or bouncer.

### Server

A computer providing an IRC connection endpoint. Several servers can belong
to one network. A bouncer can also act as the immediate server endpoint from
Fabulor's perspective.

### Server Tab

The Fabulor context that displays connection progress, TLS and authentication
results, server capabilities, the MOTD, notices, and errors for one network
connection.

### Simple Add-On

A trusted C#, Python, or Tcl component placed beneath
`%APPDATA%\Fabulor\addons`. Simple add-ons are convenient for reviewed personal
scripts but do not use the manifest capability model.

### SOCKS5

A proxy protocol supported by Fabulor for IRC and DCC TCP connections. Fabulor
supports no authentication or username/password authentication; it does not
claim GSSAPI or UDP support.

### Spell Checking

Input-box word checking provided through Fabulor's packaged Enchant and
WinSpell integration. Suggestions and the personal dictionary are available
from the input-box context menu when spell checking is enabled.

## T

### Tab

A server, channel, or private-message context represented in the channel
switcher. The switcher can use a tree presentation even though these contexts
are conventionally called tabs.

### TLS

Transport Layer Security, which encrypts the connection between Fabulor and
the immediate IRC server or bouncer and verifies that endpoint's certificate.
It does not provide end-to-end encryption of ordinary IRC messages.

### Topic

Channel information set by users with the required privilege. Topics often
contain rules, links, or a description and may be shown in the topic bar and
transcript.

### Transcript

The main scrollable text area displaying messages, events, and restored log
content for the current context.

### Tray Or Notification Area

The Windows taskbar area that can hold Fabulor's background icon. When tray
minimization is enabled, closing or minimizing the main window may leave
Fabulor running there.

## U

### User List

The list of users in the current channel, usually shown on the right. Prefixes,
icons, and colours can indicate channel privileges or user state.

## Z

### ZNC

An IRC bouncer. Fabulor connects to ZNC as a server while ZNC maintains the
upstream IRC connection, networks, channels, and optional playback. Fast ZNC
playback can produce a large burst of messages immediately after connection.

## Related Pages

- [Getting started](getting-started.md)
- [Networks and connections](networks-and-connections.md)
- [Main window](main-window.md)
- [Commands](commands.md)
- [Add-ons](addons.md)
- [Security and privacy](security-and-privacy.md)
- [Troubleshooting](troubleshooting.md)
