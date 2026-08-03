# Preferences

Fabulor's Preferences window controls the client interface, chat behaviour,
alerts, logging, connection defaults, and file transfers. Open it with
**Settings > Preferences**.

For every name accepted by `/SET`, including settings without a direct
Preferences control, see the
[Fabulor settings reference](settings-reference.md).

Most settings on these pages are global: they apply to every network and tab.
Network names, server addresses, identities, passwords, and per-network
connection choices belong in the [Network List](networks-and-connections.md),
not in Preferences.

## Saving Or Cancelling Changes

- Select **OK** to apply and save the staged settings.
- Select **Cancel** or close the window to discard staged changes.
- Theme and colour choices may be previewed while Preferences is open. Cancel
  restores the theme and palette that were active when the window opened.
- If Fabulor cannot save `fabulor.conf` or `colors.conf`, it reports the file
  that failed and leaves retry possible. It explicitly reports a partial save
  if `fabulor.conf` succeeded but `colors.conf` did not.
- Some settings display a restart warning. Close Fabulor normally and start it
  again before judging whether those changes worked.

The window remembers the last Preferences page used during the current Fabulor
session.

## Interface

### Appearance

Use **Appearance** to control the general presentation of the main window.

**General** contains the interface language and main font. A language change
may require a restart.

**Text Box** controls coloured and indented nicknames, the marker line, and the
scroll-to-bottom button. The marker line separates previously read text from
newer activity. The scroll button appears while the transcript is not at the
bottom.

**Timestamps** enables transcript timestamps and sets their format.

**Title Bar** controls whether the current channel modes, user count, and
nickname appear in the window title.

**Topic Bar** controls whether mode buttons sit beside the topic and whether a
long topic may use more than one line.

**Fabulor Theme** contains:

- **Desktop theme**, including **System default** and imported GTK4 themes;
- **Variant**: **Follow system**, **Prefer light**, or **Prefer dark**; and
- **Import theme archive...** for supported GTK4 theme packages.

The **Advanced** area sets a transcript background image, window opacity, and
mouse-wheel transcript speed.

See [Themes and colours](themes-and-colours.md) for installation,
compatibility, rollback, and recovery guidance.

### Colours

Use **Colours** to select a packaged or profile `.hct` palette, adjust the IRC
and client colour roles, and choose where incoming IRC colour codes are
stripped.

The page includes:

- the **Palette theme** list, with **Current colours** and discovered themes;
- standard mIRC colours and local colour slots;
- transcript foreground and background;
- selected-text foreground and background;
- new-data, new-message, highlight, away-user, marker-line, and spell-checker
  colours;
- colour stripping for messages, restored scrollback, and topics; and
- **Manage all client colours...** with a live preview and reset controls.

Palette selection is loaded asynchronously so a larger theme does not block
the interface. Select **OK** to keep the result or **Cancel** to restore the
previous palette.

### Input Box

Use **Input box** to configure message entry, spell checking, saved input
history, and nickname completion.

**Input Box** controls whether IRC colours and attributes are rendered in the
entry, whether the nickname box is shown, whether its user-mode icon is shown,
and whether spell checking is enabled.

On Windows, enter spell-check dictionary language codes from:

```text
%LOCALAPPDATA%\enchant\myspell\dicts
```

Separate multiple dictionary codes with commas.

**Editbox History** sets the maximum entries kept for each context and whether
history is saved and restored. See
[Input history and conversation logs](history-and-logs.md) for storage and
clearing behaviour.

**Nick Completion** sets the completion suffix, sorting order, and the number
of matching nicknames at which Fabulor lists candidates instead of completing
one immediately.

### User List

Use **User list** to control the channel member list.

Available choices include showing hostnames, user-mode icons, coloured
nicknames, and the channel user count. The list may be placed on either side of
the transcript and sorted alphabetically, by privilege, in reverse order, or
left unsorted.

**Allow user list to resize with the window** permits automatic width changes.
Turn it off when you want Fabulor to retain the configured user-list width.

**Away Tracking** marks away users on channels below the configured size.
Tracking very large channels can generate unnecessary network traffic and UI
work, so retain a sensible upper limit.

The page also sets the command run by double-clicking a nickname and the style
of the lag and throttle meters: off, graphical, text, or both.

### Channel Switcher

Use **Channel switcher** to configure the tab strip or network-and-channel
tree.

The page controls:

- **Tabs** or **Tree** layout;
- a separate server-message tab;
- automatic private-message tabs;
- alphabetical sorting, icons, dotted tree lines, and compact text;
- mouse-wheel tab changes, close buttons, and middle-click closing;
- when newly opened tabs receive focus;
- where notices are displayed;
- switcher placement and shortened labels; and
- whether channels, private messages, and utility views open in tabs or
  separate windows.

The **View > Channel Switcher** menu provides quick access to the main layout
choice. Preferences contains the full set of switcher options.

## Chatting

### General

**Default Messages** sets the text used when quitting, leaving a channel, or
marking yourself away.

**Away** can suppress repeated identical away messages and automatically mark
you back before sending a message.

**Miscellaneous** controls raw MODE display, automatic WHOIS for Notify List
contacts, join/part visibility, nickname-change visibility, hostmasks in
join/part messages, and whether `Ctrl+Q` quits Fabulor.

### Alerts

Alerts are configured independently for channel messages, private messages,
and highlighted messages. Depending on Windows notification availability, the
page offers notifications, tray-icon blinking, taskbar blinking, and the
Windows instant-message sound.

Alerts can be omitted while you are away or while the Fabulor window has
focus.

**Tray Behavior** controls the tray icon, minimise-to-tray, close-to-tray,
automatic away/back state while hidden, and whether notifications are limited
to times when Fabulor is hidden or minimised.

**Highlighted Messages** accepts comma-separated words or nicknames. Wildcards
are supported. Separate fields add extra highlight words, suppress highlights
for selected nicknames, or always highlight selected nicknames.

### Sounds

The Sounds page lists Fabulor text events and the sound file assigned to each
event.

1. Select an event.
2. Enter a sound file or select **Browse...**.
3. Select **Play** to test it.
4. Select **OK** to keep the assignment.

On Windows the file chooser filters for `.wav` files. Files placed in the
profile's `sounds` directory can be stored by filename; files elsewhere use
their path. Assignments persist in `sound.conf`.

See [Sounds and alerts](sounds-and-alerts.md) for recommended event choices,
file placement, notification interaction, and troubleshooting.

### Logging

Use **Logging** to restore previous scrollback, limit transcript lines, write
conversation logs, and record URLs.

The log filename supports these substitutions:

- `%s` - server;
- `%c` - channel or private-message name; and
- `%n` - network.

Logging has its own timestamp switch and format. URL logging and the in-memory
URL Grabber can be enabled separately, with a maximum number of grabbed URLs.

Use **Open Data Folder** to open the active Fabulor profile. See
[Input history and conversation logs](history-and-logs.md) before changing log
paths or deleting stored data.

### Advanced

**Auto Copy Behavior** controls transcript selection:

- automatically copy selected text when the left mouse button is released;
- include timestamps automatically, or include them only while `Shift` is
  held during selection; and
- include IRC colour information automatically, or include it only while
  `Ctrl` is held during selection.

Without automatic copying, use `Ctrl+Shift+C` to copy the current transcript
selection.

**Miscellaneous** sets the default real name, alternative fonts, compact list
spacing, automatic reconnect, lag-check interval, reconnect delay, autojoin
delay, and preferred ban-mask form.

**Enable manifest plugins (requires restart)** enables trusted manifest-based
plugins from the profile `plugins` directory. Fabulor displays a confirmation
because these plugins run with the same operating-system privileges as your
user account. Enable only plugins whose source and origin you trust, then
restart Fabulor. See [Add-ons](addons.md) for installation, capability,
blacklist, and safe-start guidance.

## Network

### Network Setup

These are global connection defaults. A network's address, nickname, login,
TLS, and autojoin configuration remains in the Network List.

**Your Address** optionally binds outgoing connections to one local address.
This is normally useful only on computers with multiple network addresses.

**File Transfers** controls how Fabulor determines and advertises the address
and listening-port range used for DCC transfers.

**Proxy Server** sets the hostname, port, proxy type, and whether the proxy is
used for all connections, IRC only, or DCC only. Supported choices shown by
the interface are disabled, SOCKS4, SOCKS5, HTTP, and automatic detection.

**Connection Health** sets TCP keepalive idle time, probe interval, and probe
count.

**Proxy Authentication** applies only to HTTP and SOCKS5. Leave it disabled
for a proxy that does not require a username and password.

### File Transfers

Use **File transfers** to configure incoming DCC files and transfer limits.

Incoming offers can ask for confirmation, ask for a destination folder, or be
saved without interaction. You can choose download and completed-file folders
and include the sender's nickname in filenames.

Fabulor can automatically open send, receive, and direct-chat views. Separate
limits control one upload, one download, all uploads, and all downloads. Rate
values use KiB/s or MiB/s.

Automatically accepting files without interaction increases risk. Use it only
when you understand who can send files to you and where those files will be
written.

## Where Preferences Are Stored

For an installed copy of Fabulor, the profile is:

```text
%APPDATA%\Fabulor
```

Portable mode uses the `Config` directory beside `fabulor.exe` instead.

The main preferences are stored in `fabulor.conf`; palette colours are stored
in `colors.conf`; and event sounds are stored in `sound.conf`. Other profile
files hold network definitions, key bindings, channel options, ignore and
notify lists, commands, history, logs, themes, sounds, and add-ons.

To back up your configuration safely:

1. Close Fabulor normally.
2. Copy the entire profile directory to a protected location.
3. Treat the backup as sensitive because it can contain network identity and
   credential-related data.

Do not edit or replace files beneath `C:\Program Files\Fabulor` to change user
preferences. Updates and repairs may replace installed files; personal
configuration belongs in the profile.
