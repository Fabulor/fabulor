# Main Window

Status: source-verified draft with an installed-client main-window screenshot;
final release-candidate verification is still required.

Fabulor keeps each IRC network, server tab, channel, and private conversation
in a single main window by default. The layout can be adjusted without losing
the connection or closing the selected conversation.

## Window Layout

The normal tree layout contains:

- the menu bar across the top;
- the channel switcher at the left;
- the topic bar above the transcript;
- the transcript in the centre;
- the user list beside channel transcripts; and
- the nickname box and input box along the bottom.

Some items are meaningful only in a channel. A server tab has no channel user
list, and a private conversation does not have channel mode controls.

![Fabulor showing a public IRC channel with the channel switcher, topic bar,
transcript, user list, nickname box, input box, and network meters visible.](images/main-window-channel.png)

Use **View** to show or hide the menu bar, topic bar, user list, user-list
buttons, and mode buttons. **View > Channel Switcher** changes between the
tree and tab layouts. **View > Fullscreen** enters or leaves full-screen mode.

If the menu bar has been hidden, press `Ctrl+F9` to show it again.

## Channel Switcher

The channel switcher identifies every open network context:

- a server entry owns the connection messages for one network;
- channels appear beneath their server in tree mode; and
- private conversations appear as separate entries.

Select an entry to show its transcript. In tree mode, use the arrow beside a
server to collapse or expand its children. Closing a channel entry parts the
channel when appropriate; closing a server entry can close the connection and
its child contexts.

Right-click an entry for context-specific commands such as auto-connect,
autojoin, detach, or close. The exact choices depend on whether the entry is a
server, channel, or private conversation.

The switcher uses text, icons, and theme colours to distinguish selected and
inactive entries with activity. Do not rely on colour alone: the entry label
and its position beneath the owning server also identify the context.

The channel-switcher layout can be configured under **Settings > Preferences
> Interface > Channel switcher**. Available settings include placement,
sorting, icons, label length, close buttons, mouse-wheel switching, and whether
new channels, private conversations, and utility views open as tabs or
windows.

## Server Tabs

Every connection has a server tab. It displays lookup, connection, TLS,
authentication, capability, message-of-the-day, service, and error output.
Fast bouncer connections may enter an autojoined channel quickly, but the
complete connection output remains in the server tab.

Consult the server tab before retrying a failed connection or reporting a
connection problem. Its output often separates DNS, proxy, TLS, SASL, and IRC
service failures without additional diagnostic tools.

## Topic Bar

In a channel, the topic bar displays the current topic. It can use one or
multiple lines according to **Settings > Preferences > Interface >
Appearance > Allow multi-line topics**.

Channel operators can edit the topic by activating the topic field, changing
the text, and pressing `Enter`. If the server does not permit the change, its
response appears in the transcript or server tab.

Recognised links in topics and transcripts use the same Windows browser
handling as other Fabulor links.

## Transcript

The transcript contains messages and events for the selected context. It may
show timestamps, nicknames, IRC formatting, links, emoji, flags, and a marker
line. Its font, timestamps, indentation, colours, scroll speed, background,
and marker visibility are configured in Preferences.

Use the mouse wheel, `Page Up`, and `Page Down` to read earlier or later text.
`Ctrl+Home` moves to the top and `Ctrl+End` returns to the bottom. When the
transcript is above the newest message, the down-chevron overlay returns
directly to the bottom.

The marker line records the boundary between previously read and newer text.
Use **Window > Move to Marker Line** to find it and **Window > Reset Marker
Line** to remove the current marker.

The **Window** menu also provides transcript commands:

- **Copy Selection** copies the current selection;
- **Clear Text** clears the visible transcript without deleting its saved log;
- **Save Text...** writes the current transcript to a file; and
- **Search** finds text and moves between matches.

Clearing a transcript is not the same as clearing saved conversation logs.
See [Input History And Conversation Logs](history-and-logs.md) for persistent
data and clearing commands.

## Selecting And Copying Text

Drag from the first required character to the last required character. The
highlight follows the exact partial or complete text selection, including
wrapped lines.

By default, releasing the left mouse button copies selected text to the
Windows clipboard. If automatic copying is disabled, use `Ctrl+Shift+C` or
**Window > Copy Selection**.

The options under **Settings > Preferences > Chatting > Advanced** control
copying:

- **Automatically copy selected text** copies on button release;
- **Automatically include timestamps** includes displayed timestamps; and
- **Automatically include color information** preserves IRC colour data.

When automatic timestamp copying is disabled, hold `Shift` while selecting to
include timestamps. When automatic colour copying is disabled, hold `Ctrl`
while selecting to include colour information.

## Links

Move the pointer over a recognised link in the transcript. The pointer changes
to indicate that the link is actionable.

- Left-click once to open the link in the default Windows browser.
- Right-click and select **Open Link in Browser** for the explicit menu action.
- Select **Copy Selected Link** to copy the link without opening it.

Only open links from people and networks you trust. A displayed link can lead
to content outside Fabulor, and the destination is handled by the default
Windows application.

## Input Box

Type a message in the input box and press `Enter` to send it to the selected
channel or private conversation. A line beginning with `/` is interpreted as
a command.

The input box supports:

- `Up` and `Down` for previous and later entries in that context's history;
- `Tab` and `Shift+Tab` for nickname or command completion;
- normal Windows text selection, editing, and clipboard operations;
- IRC formatting controls inserted by configured keyboard shortcuts;
- spell checking and replacement suggestions; and
- emoji and flag insertion from the icon at the right.

Saved input history is network- and context-aware. A command entered in one
channel does not become the active history of an unrelated network or channel.

The nickname box identifies the nickname used by the current connection. Its
mode icon reflects the user's current IRC status where available. Both can be
shown or hidden in **Settings > Preferences > Interface > Input box**.

## Spell Checking

When spell checking is enabled, misspelled words are underlined in the input
box. Right-click a word to see suggestions, replace it, ignore it for the
current session, or add it to the personal dictionary. Personal dictionary
entries persist across normal restarts.

Links are excluded from ordinary spelling-word handling. Spell-checking
languages are selected under **Settings > Preferences > Interface > Input
box** using language codes separated by commas.

## Emoji And Flags

Select the emoji icon at the right of the input box to open the picker. Choose
a category, then select an item to insert it at the current cursor position.
Use the close button or `Escape` to dismiss the picker without inserting an
item.

The **Flags** category shows installed flag artwork and inserts the
corresponding Unicode regional-indicator sequence. Search accepts a country
name or two-letter country code. The artwork is part of the installed Fabulor
runtime; it is not copied into the user profile.

Emoji appearance can vary with the active font and Windows font support.
Fabulor supplies fallback font configuration so ordinary emoji and installed
flags remain usable with the supported runtime.

## User List

The user list shows people currently known to be in the selected channel.
Mode icons distinguish operators and other ranked users when supported by the
network. Nickname colours, sorting, hostnames, user counts, away tracking, and
placement are controlled under **Settings > Preferences > Interface > User
list**.

Drag the divider to choose the user-list width. Disable **Allow user list to
resize with the window** when the chosen width should remain fixed while
switching channels or resizing the main window.

Right-click a nickname to open its action menu. Depending on context and your
channel privileges, it can provide:

- a private conversation;
- file sending;
- WHOIS information;
- adding the nickname to the Friends List;
- ignore controls;
- channel-operator actions; and
- reply handling.

The information submenu displays only information currently known to the IRC
client. Fields such as account, country, away message, and last-message time
may be absent or marked unknown until the network supplies them.

Double-click behaviour is configurable. Do not assume that double-clicking a
nickname always starts a private conversation on another user's installation.

## Menus

The main menus group commands by purpose:

- **Fabulor** opens the Network List, creates tabs or windows, loads a plugin
  or script, detaches or closes the current context, and quits;
- **View** controls the visible layout, switcher, network meters, and
  full-screen mode;
- **Server** disconnects or reconnects, joins a channel, opens the Channel
  List, and changes away state;
- **Settings** opens Preferences and the editors for replacements, CTCP
  replies, buttons, shortcuts, text events, URL handlers, and user commands;
- **Window** opens utility views and operates on the current transcript; and
- **Help** opens the manual, project information, issue reporting, and About
  dialog.

Add-ons may append commands to supported menus. An add-on command is not a
built-in Fabulor feature merely because it appears in a standard menu.

## Utility Windows

The **Window** menu opens the Ban List, Character Chart, Direct Chat, File
Transfers, Friends List, Ignore List, Plugins and Scripts, Raw Log, and URL
Grabber. Some utilities require a connected server or selected channel and are
unavailable when their context does not exist.

See [Add-ons](addons.md) before loading, reloading, or removing executable
plugins and scripts.

The Raw Log displays protocol traffic for the selected server and can contain
sensitive connection information. Review it before sharing a screenshot or
bug report.

## System Tray

Tray behaviour is configured under **Settings > Preferences > Chatting >
Alerts**. The available options enable the tray icon, minimize to the tray,
close to the tray, and automatically mark connections away or back when the
window is hidden or restored.

On Windows, left-click the tray icon to hide or restore the main window.
Right-click it for **Restore Window** or **Hide Window**, **Away**, **Back**,
**Preferences**, and **Quit**. Use **Quit** when you intend to end Fabulor;
closing or minimizing may leave it connected in the notification area.

Tray notifications and icon changes can indicate private messages,
highlights, channel messages, invitations, and file offers according to the
alert settings.

## Default Keyboard Shortcuts

The following defaults are useful for main-window navigation:

| Shortcut | Action |
| --- | --- |
| `Ctrl+Page Up` / `Ctrl+Page Down` | Select the previous or next context |
| `Alt+1` through `Alt+9` | Select a numbered context |
| `Page Up` / `Page Down` | Scroll the transcript |
| `Ctrl+Home` / `Ctrl+End` | Move to the top or bottom of the transcript |
| `Up` / `Down` | Move through input history |
| `Tab` / `Shift+Tab` | Complete a nickname or command |
| `Ctrl+S` | Open the Network List |
| `Ctrl+W` | Close the current context |
| `F7` | Show or hide the user list |
| `Ctrl+F9` | Show or hide the menu bar |
| `F11` | Enter or leave full-screen mode |
| `Alt+A` | Toggle away status |
| `Ctrl+M` | Reset the marker line |
| `Ctrl+Shift+M` | Move to the marker line |
| `Ctrl+Shift+C` | Copy the transcript selection |
| `Ctrl+F` | Search the transcript |
| `Ctrl+G` / `Ctrl+Shift+G` | Find the next or previous match |
| `Ctrl+Shift+T` | Reopen the most recently closed channel tab |
| `F1` | Open the manual |

`Ctrl+Q` quits only when **Enable Ctrl+Q to quit** is enabled under
**Settings > Preferences > Chatting > General**.

Shortcuts can be inspected and changed under **Settings > Keyboard
Shortcuts**. A customised installation may therefore differ from this table.

## Keyboard And Accessibility

Use `Tab` and `Shift+Tab` to move among interactive controls, arrow keys to
move within lists and menus, `Enter` or `Space` to activate the focused item,
and `Escape` to close a menu or popover. Standard Windows focus indicators
show the current keyboard target according to the active GTK4 desktop theme.

Fabulor exposes labels for primary controls, menu commands, list entries, and
emoji actions. Theme colours should not be the only signal for selection,
rank, or activity. The final manual screenshots and accessibility wording will
be confirmed during the installed release-candidate keyboard pass.

## If The Layout Looks Wrong

- Confirm the intended context is selected in the channel switcher.
- Restore hidden sections from **View**.
- Check switcher and user-list placement in Preferences.
- Disable automatic user-list resizing if its divider moves unexpectedly.
- Return the desktop theme to **System default** to distinguish theme CSS from
  a layout problem.
- Close the emoji picker with its close button or `Escape` before testing
  unrelated keyboard focus.
- Record the affected network and channel type, but remove private transcript
  content before taking a diagnostic screenshot.
