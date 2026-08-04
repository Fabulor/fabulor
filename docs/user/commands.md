# Commands

Status: source-verified draft; installed release-candidate verification is
still required.

Commands provide keyboard access to IRC operations, Fabulor settings, and
commands supplied by user definitions or add-ons. Enter a command in the input
box of the network, channel, or private conversation where it should run.

## Command Syntax

The default command prefix is `/`. Command names are not case-sensitive.

```text
/COMMAND required-argument optional-argument
```

This guide uses:

- `<value>` for a required value;
- `[value]` for an optional value; and
- `A|B` for a choice between values.

Do not type the angle or square brackets. Where an argument contains spaces,
use the command's documented trailing-text form or enclose the value in double
quotes when supported.

Use `/SAY` to send text that would otherwise be interpreted as a command:

```text
/SAY /this begins with a slash
```

## Live Help

The running client is authoritative because installed add-ons and customised
user commands can extend the command list.

```text
/HELP
/HELP -l
/HELP <command>
```

`/HELP` lists built-in, user-defined, and add-on commands. `/HELP -l` includes
the available help text. `/HELP <command>` displays the syntax for one command.

If Fabulor is connected and a name is not a built-in, user-defined, or add-on
command, it forwards the line to the IRC server. This permits standard or
network-specific commands such as `/WHOIS`, `/WHOWAS`, and `/STATS`. When not
connected, an unknown command produces an error instead.

## Everyday Commands

| Command | Purpose |
| --- | --- |
| `/AWAY [reason]` | Mark the current connection away |
| `/BACK` | Remove away status from the current connection |
| `/CLEAR` | Clear the current visible transcript |
| `/CLEAR <amount>` | Remove the requested number of oldest transcript lines |
| `/CLEAR -<amount>` | Remove the requested number of newest transcript lines |
| `/CLEAR ALL` | Clear non-highlighted open transcripts |
| `/CLEAR HISTORY` | Clear saved and in-memory input history for the current context |
| `/CLEAR LOG` | Remove the current context's conversation log and resume logging if enabled |
| `/CLOSE` | Close the current tab or window |
| `/CLOSE -m` | Close all private-conversation tabs |
| `/CYCLE [channel]` | Part and rejoin the current or named channel |
| `/DISCON` | Disconnect the current server |
| `/JOIN <channel>` | Join a channel |
| `/LAGCHECK` | Request an immediate lag measurement |
| `/LASTLOG <text>` | Search the current transcript and show matching lines |
| `/LIST [server options]` | Open the Channel List and request the network's public channel list |
| `/ME <action>` | Send a third-person action to the current destination |
| `/MSG <nickname> <message>` | Send a private message |
| `/NAMES [channel]` | Request the nickname list for a channel |
| `/NICK <nickname>` | Change nickname on the current connection |
| `/NOTICE <nickname|channel> <message>` | Send an IRC notice |
| `/PART [channel] [reason]` | Leave the current or named channel |
| `/QUERY <nickname> [message]` | Open a private conversation and optionally send a message |
| `/QUIT [reason]` | Disconnect the current server with an optional reason |
| `/RECONNECT [ALL]` | Reconnect the current server or all open servers |
| `/SAY <text>` | Send text to the current channel or private conversation |
| `/TOPIC [topic]` | Show or change the current channel topic |
| `/URL <address>` | Open an address using the appropriate Windows handler |

`/CLEAR HISTORY` and `/CLEAR LOG` are scoped operations. They do not erase
other networks or channels. See
[Input History And Conversation Logs](history-and-logs.md).

## Searching The Transcript

`/LASTLOG` searches the currently displayed transcript:

```text
/LASTLOG [-h] [-m] [-r] [--] <text>
```

- `-h` highlights matches;
- `-m` makes matching case-sensitive;
- `-r` treats the search as a regular expression; and
- `--` ends option processing when the search itself begins with a hyphen.

The search result opens in a temporary result context. **Window > Search** is
the interactive alternative for moving among matches in the existing
transcript.

## Channel Moderation

These commands require the relevant channel privilege and network support.

| Command | Purpose |
| --- | --- |
| `/BAN <mask> [type]` | Add a channel ban without automatically kicking matching users |
| `/UNBAN <mask> [mask...]` | Remove one or more channel bans |
| `/QUIET <mask> [type]` | Add a quiet mask when the network supports it |
| `/UNQUIET <mask> [mask...]` | Remove quiet masks |
| `/OP <nickname>` | Give channel-operator status |
| `/DEOP <nickname>` | Remove channel-operator status |
| `/HOP <nickname>` | Give half-operator status |
| `/DEHOP <nickname>` | Remove half-operator status |
| `/VOICE <nickname>` | Give voice status |
| `/DEVOICE <nickname>` | Remove voice status |
| `/KICK <nickname> [reason]` | Remove a user from the current channel |
| `/KICKBAN <nickname> [reason]` | Ban and remove a user |
| `/INVITE <nickname> [channel]` | Invite a user to a channel |
| `/MODE <target> [modes]` | Inspect or change IRC modes |
| `/MOP` | Give operator status to all ordinary users in the current channel |
| `/MDEOP` | Remove operator status from all operators in the current channel |
| `/MHOP` | Give half-operator status to users in the current channel |
| `/MDEHOP` | Remove half-operator status from all half-operators |
| `/MKICK` | Remove every other user from the current channel |
| `/WALLCHOP <message>` | Send a message to the current channel's operators |

Mass actions can affect many people immediately. Confirm the selected network
and channel before using `/MOP`, `/MDEOP`, `/MHOP`, `/MDEHOP`, or `/MKICK`.

## Identity, Services, And Presence

| Command | Purpose |
| --- | --- |
| `/GHOST <nickname> [password]` | Ask NickServ to release a ghosted nickname |
| `/ID <password>` | Identify to NickServ using the connection's service method |
| `/NOTIFY [nickname]` | Show the Friends List or add a nickname |
| `/NOTIFY -n <networks> [nickname]` | Limit a Friends List entry to named networks |
| `/PING <nickname|channel>` | Send a CTCP ping |
| `/CTCP <nickname> <message>` | Send a CTCP request such as `VERSION` |
| `/NCTCP <nickname> <message>` | Send CTCP content inside an IRC notice |
| `/COUNTRY <code|pattern>` | Look up country codes |
| `/COUNTRY -s <code|pattern>` | Search country names and codes |

Avoid placing account passwords directly in the input box. Input history may
be saved to disk. Configure authentication and Windows Credential Manager in
the Network List wherever possible.

## Ignore Commands

```text
/IGNORE <mask> <types...> [NOSAVE] [QUIET]
/UNIGNORE <mask> [QUIET]
```

The supported ignore types are `PRIV`, `CHAN`, `NOTI`, `CTCP`, `DCC`, `INVI`,
and `ALL`. A mask normally uses IRC hostmask form, for example
`nickname!*@host` or `*!*@domain`.

`NOSAVE` creates a non-persistent entry. `QUIET` suppresses the confirmation
output. The **Window > Ignore List** interface is safer when constructing an
unfamiliar mask.

## Connections And Servers

The Network List is the recommended way to configure persistent connections.
The following commands are useful for temporary or diagnostic work:

| Command | Purpose |
| --- | --- |
| `/SERVER <host> [port] [password]` | Connect the current server tab |
| `/SERVER -noproxy <host> [port]` | Connect while bypassing the configured proxy |
| `/SERVER -insecure <host> [port]` | Make an explicitly non-TLS connection |
| `/SERVER -ssl <host> [port]` | Make a TLS connection with certificate verification |
| `/SERVER -ssl-noverify <host> [port]` | Make a TLS connection without certificate verification |
| `/SERVCHAN <host> <port> <channel>` | Connect and then join a channel |
| `/NEWSERVER [-noconnect] <host> [port]` | Open a new server tab, optionally without connecting |
| `/ADDSERVER <network> <host/port>` | Add a network and server to the Network List |
| `/CHARSET [encoding]` | Show or change the current connection encoding |
| `/CHATHISTORY` | Request recent server-side history for the current channel or private conversation |
| `/CHATHISTORY <subcommand and arguments>` | Send an explicit IRCv3 chat-history request |
| `/DNS <nickname|host|address>` | Resolve a nickname, hostname, or address |
| `/FLUSHQ` | Flush the current connection's queued outbound data |

TLS is the default for `/SERVER` and `/SERVCHAN` in the supported Windows
build. Use `-insecure` only when the network explicitly provides a non-TLS
endpoint. `-ssl-noverify` weakens authentication and should be restricted to
a temporary test of a server you control.

A password supplied on a command line can enter saved input history. Prefer a
saved Network List entry with Credential Manager storage.

## DCC

Fabulor retains direct client-to-client chat and file-transfer commands:

```text
/DCC GET <nickname>
/DCC SEND [-maxcps=<rate>] <nickname> [file]
/DCC PSEND [-maxcps=<rate>] <nickname> [file]
/DCC CHAT <nickname>
/DCC PCHAT <nickname>
/DCC LIST
/DCC CLOSE <type> <nickname> <file>
/SEND <nickname> [file]
```

Accept files only from a trusted sender. Receiving a file does not establish
that it is safe to open. DCC can also reveal network addressing information
and may be blocked by firewalls, network address translation, or proxy policy.

## Running Commands Across Contexts

| Command | Scope |
| --- | --- |
| `/ALLCHAN <command>` | Every joined channel on every open server |
| `/ALLCHANL <command>` | Every joined channel on the current server |
| `/ALLSERV <command>` | Every open server |
| `/DOAT <channels|/network> <command>` | Selected channels or network |

The nested command runs separately in every matching context. Avoid using
these commands for destructive moderation, password handling, or text that
must reach only one network.

## Preferences With `/SET`

`/SET` reads and changes global Fabulor preference keys:

```text
/SET
/SET <variable>
/SET <pattern>
/SET <variable> <value>
/SET -e <string-variable>
/SET -quiet <variable> <value>
```

Examples:

```text
/SET gui_tray
/SET gui_tray ON
/SET gui_ulist_*
```

With no arguments, `/SET` lists all current variables. A `*` or `?` pattern
lists matching variables when no value is supplied. Boolean values accept
`ON`, `OFF`, `YES`, `NO`, `1`, or `0`. `-e` erases a string value, and
`-quiet` suppresses normal confirmation output.

Changes are saved immediately to the active profile. A retired or misspelled
key reports `No such variable.` Do not add old XChat, HexChat, or ZoiteChat
keys to the profile merely because they appear in historical documentation.

Preferences remains the recommended interface for ordinary settings because
it validates ranges and explains related options together.

See the [Fabulor settings reference](settings-reference.md) for the complete
Windows `/SET` inventory, value types, and concise descriptions.

## Per-Context Options With `/CHANOPT`

`/CHANOPT` reads or overrides settings for the current network and channel or
private conversation:

```text
/CHANOPT
/CHANOPT <variable>
/CHANOPT <variable> ON|OFF|DEFAULT
```

Current variables are:

- `alert_balloon`;
- `alert_beep`;
- `alert_taskbar`;
- `alert_tray`;
- `text_hidejoinpart`;
- `text_logging`;
- `text_scrollback`; and
- `text_strip`.

`DEFAULT` removes the override and returns to global behaviour. These values
are saved by network and context rather than becoming global Preferences.

## User-Defined Commands

Open **Settings > User Commands** to inspect or edit aliases stored in the
profile's `commands.conf`. Fabulor includes convenient definitions such as
`/J` for `/JOIN`, `/M` for `/MSG`, `/ACTION` for `/ME`, and `/GREP` for a
regular-expression transcript search. Because users can edit these
definitions, inspect the editor rather than assuming an alias has its original
meaning.

Common substitution forms include:

- `%2` for the second word supplied to the alias;
- `&2` for the complete text beginning with the second word;
- `%c` for the current channel;
- `%e` for the current network;
- `%n` for the current nickname;
- `%v` for the Fabulor version; and
- `%%` for a literal percent sign.

An alias can call another command. Fabulor limits recursive user commands and
stops an excessive loop with an error.

## Add-On Commands

Add-ons can register commands that appear in `/HELP`. Availability therefore
depends on installed features and loaded add-ons. Shipped optional components
can provide commands such as:

- `/SYSINFO` for system information;
- `/FISHLIM`, `/SETKEY`, `/DELKEY`, and `/KEYX` for FiSHLiM;
- `/EXEC` from the optional Exec plugin; and
- Python or Tcl host-management commands when those runtimes are installed.

Use `/HELP <command>` for the loaded component's current syntax. Add-on
commands are covered in the [add-ons guide](addons.md) rather than treated as
core IRC commands.

## Loading Plugins And Scripts

```text
/LOAD [-e] <file>
/RELOAD <name>
/UNLOAD <name>
```

Loading executes native or scripted code with the permissions of the Fabulor
process. Load only an add-on whose source and origin you trust. The **Window >
Plugins and Scripts** interface provides the corresponding load, reload, and
unload controls.

The optional `/EXEC` command executes an operating-system command and is not a
normal Windows built-in. Its input is length-bounded, but that does not make an
untrusted command safe.

## Advanced Commands

The following commands exist for automation, protocol diagnosis, add-ons, or
interface definitions. They are not required for ordinary IRC use:

| Command | Purpose or caution |
| --- | --- |
| `/ADDBUTTON`, `/DELBUTTON` | Modify user-list command buttons |
| `/ECHO <text>` | Print text locally without sending it |
| `/GETBOOL`, `/GETFILE`, `/GETINT`, `/GETSTR` | Open a GTK prompt and pass its result to another command |
| `/GUI ...` | Control Fabulor window presentation |
| `/MENU ...` | Add or remove menu definitions |
| `/REPLY <message-id> <message>` | Send a reply-tagged message where supported |
| `/SETCURSOR <position>` | Move the input cursor |
| `/SETTAB <name>` | Rename the current tab label |
| `/SETTEXT <text>` | Replace the input-box contents |
| `/SPLAY <sound-file>` | Play a sound file |
| `/TRAY ...` | Control tray icon presentation |
| `/TYPING [active|paused|done] [target]` | Send a typing notification where supported |
| `/USELECT ...`, `/USERLIST` | Automate user-list selection or access |

`/GATE` is a retained low-level gateway command and is not the retired Wingate
proxy mode or the supported SOCKS5 configuration. It is not recommended for
new connections.

## Raw Protocol Commands

`/QUOTE <text>` sends raw IRC protocol text to the server. `/RECV <text>`
injects text into Fabulor as though the server sent it. These commands bypass
normal user-level validation and can disconnect a session, alter state, expose
secrets, or produce misleading local output.

Use them only while following a specific diagnostic procedure and never paste
an untrusted raw command sequence. `/KILLALL` immediately exits Fabulor and is
also unsuitable for normal operation.

## If A Command Does Not Work

- Confirm the command spelling with `/HELP <command>`.
- Check whether the current tab is connected and, for channel operations,
  joined to a channel.
- Confirm that the server supports the command or capability.
- Check channel privileges before moderation commands.
- Inspect **Settings > User Commands** for an alias with the same name.
- Inspect **Window > Plugins and Scripts** when the command belongs to an
  optional add-on.
- Read the server tab for numeric errors or service responses.
- Remember that a command documented by another IRC client may be absent,
  renamed, supplied by an add-on, or deliberately retired in Fabulor.
