# Migrating From XChat Or HexChat

Status: source-verified draft; installed release-candidate verification is
still required.

Fabulor descends from XChat, HexChat, and ZoiteChat, so many IRC concepts will
be familiar. It is nevertheless a separate Windows 11 x64 application with a
GTK4 interface, its own profile, and its own add-on contracts. Treat migration
as a selective transfer rather than copying an old profile wholesale.

## Recommended Approach

1. Keep the old client and its profile unchanged until Fabulor is working.
2. Install Fabulor and start it once so it creates a clean profile.
3. Recreate network connections and identities in Fabulor's **Network List**.
4. Transfer only reviewed themes, sounds, aliases, and compatible scripts.
5. Test one network and one add-on at a time.
6. Keep a backup of `%APPDATA%\Fabulor` before making large changes.

Do not replace the new profile with an XChat, HexChat, or ZoiteChat profile.
Old configuration keys, paths, plugins, and toolkit settings may be ignored,
misinterpreted, or unsupported.

## Profile Location

A normal installation stores personal data under:

```text
%APPDATA%\Fabulor
```

Portable mode stores it under the `Config` directory beside `fabulor.exe`.
Fabulor does not use the former clients' profile directories as a fallback.

The current profile includes files and directories such as:

| Item | Purpose |
| --- | --- |
| `fabulor.conf` | Global preferences |
| `servlist.conf` | Networks and connection settings |
| `colors.conf` | Current transcript palette |
| `sound.conf` and `sounds` | Event-sound assignments and personal sound files |
| `history` | Per-network and per-tab input history |
| `logs` | Conversation logs |
| `themes` | `.hct` palettes and imported GTK4 desktop themes |
| `addons` | Trusted simple C#, Python, and Tcl add-ons |
| `plugins` | Advanced manifest plugins |

Let Fabulor create and maintain `fabulor.conf` and `servlist.conf`. Re-entering
connection settings through the interface also ensures that current TLS,
credential, proxy, and certificate policies are applied.

## Networks And Identities

The familiar network-list model remains: each network can have its own
servers, identity overrides, login method, passwords, auto-connect state, and
autojoin channels. Recreate these entries with **Fabulor > Network List**.

Before entering a saved password, confirm which credential it is:

- a network account or SASL password;
- an IRC server password; or
- a ZNC username and password.

Installed Fabulor can store network passwords in Windows Credential Manager.
Portable mode and profiles that do not use Credential Manager use encrypted
profile storage. Do not paste an old plaintext `servlist.conf` over the current
file.

Review every connection while migrating:

- use TLS and the correct TLS port;
- choose the login method issued by the network or bouncer;
- leave **Accept invalid TLS certificates** disabled unless you have verified
  a deliberate private-certificate deployment;
- use SOCKS5 when a proxy is required; and
- test the connection before enabling automatic connection and autojoin.

See [Networks and connections](networks-and-connections.md) for the current
fields and connection workflow.

## Preferences And Custom Lists

Recreate ordinary preferences through **Settings > Preferences**. Historical
setting names are not a compatibility promise. A removed or unknown `/SET`
key reports `No such variable.` and should not be inserted manually into
`fabulor.conf`.

Review custom list files individually instead of copying every `.conf` file.
Fabulor still has editors for user commands, user-list popups, text replacement,
URL handlers, CTCP replies, and related menus, but their current actions and
substitution rules are authoritative. Back up the clean Fabulor file before
transferring a small, understood entry and verify it in the interface.

The legacy profile-wide `input-history.conf` is not imported. Fabulor stores
input history per network and tab because entries from the old flat file cannot
be assigned reliably to their original context. Conversation logs may be kept
as an archive, but they do not need to be placed in Fabulor's active `logs`
directory.

See [Preferences](preferences.md), [Commands](commands.md), and
[Input history and conversation logs](history-and-logs.md).

## Themes And Sounds

### Colour Themes

Fabulor supports `.hct` colour-theme archives. Place a reviewed `.hct` file in:

```text
%APPDATA%\Fabulor\themes
```

Then select it under **Settings > Preferences > Interface > Colours**. Do not
extract the archive. Fabulor reads its bounded `colors.conf` payload and ignores
legacy event definitions. The retired `.zct` format is not supported.

The approved source for optional palettes is the
[HexChat colour-theme collection](https://hexchat.github.io/themes.html).

### Desktop Themes

Old GTK2 and GTK3 themes cannot be migrated. Fabulor uses GTK4 and accepts
compatible desktop-theme archives through **Settings > Preferences >
Interface > Appearance > Import theme archive...**. Do not copy an old
`gtk3-themes` directory.

[OpenDesktop](https://www.opendesktop.org/) is Fabulor's approved source for
optional GTK4 desktop themes. See [Themes and colours](themes-and-colours.md)
for archive requirements and recovery instructions.

### Sounds

Personal sound files can be copied into `%APPDATA%\Fabulor\sounds`. Assign
them to current events under **Settings > Preferences > Chatting > Sounds**.
Reassign sounds in the interface rather than relying on an inherited
`sound.conf`, because the current event list is authoritative.

## Add-Ons And Scripts

Do not copy an entire old add-ons directory and assume every item is safe or
compatible. Review the source, language, dependencies, and commands of each
add-on before installing it.

### Python

The preferred module is:

```python
import fabulor
```

Fabulor deliberately provides `import xchat` and `import hexchat`
compatibility adapters for suitable older Python scripts. These adapters
forward to the current Fabulor Python API; they do not recreate every historic
client behavior. Test each script and update it to `import fabulor` when you
maintain its source. The former `zoitechat` module is retired.

### Tcl

Tcl add-ons must use the `fabulor::*` namespace. Scripts using a former
product-named namespace require editing; no silent legacy namespace alias is
provided. Fabulor includes Tcl 8.6 without Tk or third-party package
collections, so an add-on must carry any additional trusted dependency it
needs.

### C#

C# add-ons must target .NET 8 and implement the current `IFabulorPlugin`
interface. Rebuild assemblies that reference old product-named interfaces or
types. Fabulor supplies `Fabulor.PluginAbstractions.dll`; do not place another
copy beside an add-on.

### Unsupported Plugin Runtimes

Perl and Lua are not supported Fabulor plugin runtimes. The external HexChat
JavaScript interface and the Unix-oriented D-Bus interface are also outside
Fabulor's supported Windows product. Native first-party DLLs remain
installer-managed components, not the recommended interface for new
third-party add-ons.

Use [Add-ons](addons.md) and the separate
[plugin authoring guides](../plugin-authoring-guides.md) for current layouts,
safe mode, manifest capabilities, and troubleshooting.

## Retained And Retired Behaviour

| Area | Fabulor migration status |
| --- | --- |
| IRC commands, networks, channels, queries, DCC, logging, aliases, and custom menus | Familiar concepts retained; verify current commands and interface |
| `.hct` colour themes | Retained through the Fabulor theme manager |
| Python `xchat` and `hexchat` imports | Intentional compatibility adapters retained |
| Python `zoitechat` import | Retired; use `fabulor` |
| Tcl former product namespace | Retired; use `fabulor::*` |
| Old C# product-named interfaces | Retired; rebuild against `IFabulorPlugin` |
| Perl, Lua, external JavaScript, and D-Bus plugins | Not supported |
| GTK2 and GTK3 desktop themes and `gtk3-themes` | Retired; use compatible GTK4 archives |
| `.zct` colour themes | Retired; use `.hct` |
| Built-in Identd | Retired |
| Wingate proxy mode | Retired; a saved selection is treated as disabled |
| SOCKS4 | Retained pending a separate review |
| SOCKS5 | Supported for IRC and DCC TCP connections |
| Winamp or media-player integration | Retired |
| Inherited update checker | Retired; use only project-issued installers |
| `gui_ulist_style`, `text_transparent`, and `irc_cap_server_time` settings | Retired |
| Flat `input-history.conf` | Not imported; history is now per network and tab |

Fabulor negotiates supported server-time capabilities automatically; the old
preference did not control that behavior. Its removal does not disable server
timestamps.

## Migration Checklist

Before retiring the old client, confirm that:

- each required network connects with TLS;
- SASL, network accounts, and ZNC authentication succeed;
- the server tab shows connection and message-of-the-day output;
- auto-connect and autojoin behave as intended;
- passwords are stored using the chosen current method;
- imported client certificates still match their private keys;
- aliases and custom menu actions run only in the intended context;
- every retained add-on loads without an error in the server tab;
- selected colours, GTK4 theme, sounds, spell checking, and notifications
  persist after restart;
- per-network input history and conversation logging behave as expected; and
- Fabulor closes normally and restores from the notification area when that
  option is enabled.

Keep the old profile backup until this checklist has passed during normal use.
If Fabulor fails after transferring a file or add-on, remove the last migrated
item and follow [Troubleshooting](troubleshooting.md), including safe mode and
temporary-profile isolation.
