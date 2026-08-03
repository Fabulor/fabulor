# Sounds And Alerts

Fabulor can signal activity through Windows notifications, the system tray,
the taskbar, the Windows notification sound, and sound files assigned to
individual IRC events.

The main controls are under:

```text
Settings > Preferences > Chatting > Alerts
Settings > Preferences > Chatting > Sounds
```

Use **Alerts** for broad message categories. Use **Sounds** when a specific IRC
event needs its own sound file.

## Alert Categories

The Alerts page divides incoming activity into three columns:

- **Channel Message** - ordinary messages and actions in channels;
- **Private Message** - private conversations and related direct events; and
- **Highlighted Message** - channel messages that match your nickname or
  configured highlight rules.

Each category can enable one or more alert mechanisms.

| Mechanism | What it does |
| --- | --- |
| **Show notifications on** | Displays a Windows desktop notification |
| **Blink tray icon on** | Changes or blinks the Fabulor notification-area icon |
| **Blink task bar on** | Requests attention on the Fabulor taskbar button |
| **Make a beep sound on** | Plays the Windows instant-message sound, or the sound assigned to Fabulor's **Beep** event |

The Windows notification choice appears only when Fabulor's packaged
notification backend is available. The remaining choices can still appear if
desktop notifications are unavailable.

## A Sensible Starting Configuration

Busy channels can produce excessive alerts. A restrained starting point is:

- leave ordinary **Channel Message** alerts off;
- enable taskbar, tray, or notification alerts for **Private Message**;
- enable one or two mechanisms for **Highlighted Message**; and
- add event sounds only after the broad alert behaviour is satisfactory.

This keeps routine channel traffic visible in the channel switcher without
turning every line into a Windows interruption.

## Windows Notifications

Fabulor notifications remove IRC formatting from their title and body before
passing them to Windows. Depending on the selected category, notifications can
cover channel messages, highlights, private messages, notices, invitations,
and DCC file offers.

The following settings suppress desktop notifications:

- **Omit alerts when marked as being away**;
- **Omit alerts while the window is focused**; and
- **Only show notifications when hidden or iconified**, when the tray feature
  is enabled.

Windows can independently suppress notifications through its notification,
Focus Assist, or Do Not Disturb settings. Fabulor cannot override those Windows
policies.

If **Show notifications on** is absent rather than merely unchecked, the
notification backend was not available when Preferences built the Alerts
page. See [Notification troubleshooting](#notifications-do-not-appear).

## Highlights

Fabulor normally treats a message containing your nickname as a highlight.
The **Highlighted Messages** section adds three comma-separated lists:

- **Extra words to highlight**;
- **Nick names not to highlight**; and
- **Nick names to always highlight**.

Wildcards are accepted. Use these lists conservatively: a broad wildcard can
turn ordinary traffic into continuous highlight alerts.

The exclusion list is also consulted for some private-event notification and
tray routes, including notices, invitations, and file offers.

## Away And Focus Behaviour

**Omit alerts when marked as being away** suppresses desktop notifications,
beeps, taskbar attention, and assigned event sounds on the corresponding
connection. **Omit alerts while the window is focused** suppresses desktop
notifications and the alert-page beep; taskbar and tray attention also avoid
starting while the window already has focus.

An event sound assigned on the Sounds page is a separate event route. It
observes away suppression, but it is not the same as the alert-page beep and
is not governed by the focused-window omission. If you require silence while
reading a busy channel, do not assign a sound directly to its common message
events.

Away state belongs to each IRC connection. Use `/AWAY [reason]` and `/BACK` for
the current connection. The tray menu can mark all connected networks away or
back.

## Tray Behaviour

The **Tray Behavior** section controls whether Fabulor has a Windows
notification-area icon and how the main window interacts with it.

- **Enable system tray icon** enables the tray integration.
- **Minimize to tray** hides the main window when it is minimised.
- **Close to tray** hides the main window instead of quitting when its close
  button is used.
- **Automatically mark away/back** changes connection state when hiding or
  restoring the window.
- **Only show notifications when hidden or iconified** limits desktop
  notifications while tray integration is enabled.

On Windows, left-click the tray icon to hide or restore Fabulor. Right-click it
for **Restore Window**, **Away**, **Back**, **Preferences**, and **Quit**.

Use **Quit** when you intend to close Fabulor. Closing to the tray leaves the
client and all IRC connections running.

Tray blinking stops and its counts reset when the main window receives focus.
Highlight state takes priority over ordinary message state while both are
pending.

## Per-Tab Alert Overrides

Global Preferences are defaults. A channel or private-message tab can override
four alert choices:

- notifications;
- beep;
- tray icon; and
- taskbar attention.

Right-click the channel-switcher entry and open **Extra Alerts** to toggle:

- **Show Notifications**;
- **Beep on Message**;
- **Blink Tray Icon**; and
- **Blink Task Bar**.

These menu choices save an explicit on or off value for that network and tab.
To remove an override and return to the global default, use `/CHANOPT` in the
target tab:

```text
/CHANOPT alert_balloon DEFAULT
/CHANOPT alert_beep DEFAULT
/CHANOPT alert_tray DEFAULT
/CHANOPT alert_taskbar DEFAULT
```

Use `/CHANOPT` without arguments to inspect all overrides for the current
context. Per-tab values are saved in the profile's `chanopt.conf`.

## Event Sounds

The Sounds page maps individual Fabulor text events to sound files. It is more
specific than the three broad alert categories: the list includes connection,
channel, user, message, DCC, notification-list, capability, and other displayed
events.

To assign a sound:

1. Open **Settings > Preferences > Chatting > Sounds**.
2. Select an event in the list.
3. Select **Browse...** and choose a `.wav` file, or enter its path.
4. Select **Play** to test the file.
5. Select **OK**.
6. Close Fabulor normally after testing so the updated assignment is written
   during normal profile shutdown.

The event list itself comes from Fabulor's current text-event table. You do not
need to create or populate `sound.conf` manually.

### Sound File Location

The installed profile sound directory is:

```text
%APPDATA%\Fabulor\sounds
```

Portable mode uses the `sounds` directory beneath portable `Config`.

On Windows, Fabulor's chooser filters for `.wav` files and playback uses the
Windows sound API. A file selected from the profile `sounds` directory is
stored by filename. A file elsewhere is stored by its full path.

Keeping custom sounds in the profile directory is preferable because the
profile can be backed up or moved as one unit. Do not put personal sound files
beneath `C:\Program Files\Fabulor`.

### Beep Versus Event Sounds

The Alerts page's **Make a beep sound on** uses one shared beep route. If the
**Beep** event has a sound file assigned, that file replaces the Windows system
beep for this route.

Other Sounds-page assignments run for their exact text events. If you enable
the alert beep for channel messages and also assign a sound to **Channel
Message**, one incoming line can trigger both routes. Remove one assignment if
you hear duplicate sounds.

### Persistence

Sound assignments are stored in:

```text
%APPDATA%\Fabulor\sound.conf
```

The file contains event names and sound paths. Unknown event names are ignored
when Fabulor loads it. A normal Fabulor shutdown writes the current non-empty
assignments.

To remove an assignment, select the event and clear its **Sound file** field.
Then close Fabulor normally after accepting the Preferences changes.

## Backup And Reset

Close Fabulor before backing up or replacing alert files. Preserve:

```text
%APPDATA%\Fabulor\fabulor.conf
%APPDATA%\Fabulor\sound.conf
%APPDATA%\Fabulor\chanopt.conf
%APPDATA%\Fabulor\sounds
```

`fabulor.conf` contains global alert and tray choices. `chanopt.conf` contains
per-network and per-tab overrides. `sound.conf` contains event assignments.

To reset event sounds without changing other alert preferences, close Fabulor,
back up `sound.conf`, and remove it. The Sounds page will have no file
assignments after restart.

## Troubleshooting

### The Sounds Event List Is Empty

Close and reopen Preferences once. If the list remains empty, restart the
installed client. The list should be populated from Fabulor's built-in event
table even when `sound.conf` does not yet exist.

### A Sound Will Not Play

- Confirm that the file exists and is readable.
- Use a valid `.wav` file on Windows.
- Select **Play** to test the exact stored field.
- If only a filename is shown, confirm that the file is in the profile
  `sounds` directory.
- If a full path is shown, confirm that the drive and folder still exist.

Fabulor reports the resolved path when it cannot read a selected sound file.

### A Sound Assignment Disappeared

Accept the change with **OK** and close Fabulor normally. Forced termination
can prevent the current sound assignments from being written to `sound.conf`.

### Two Sounds Play For One Message

You probably enabled the alert-page beep and assigned a file to the same text
event. Keep either the broad beep or the event-specific sound.

### Notifications Do Not Appear

1. Confirm the appropriate **Show notifications on** category.
2. Check the away, focused-window, and hidden-only suppression choices.
3. Check for a per-tab override under **Extra Alerts** or `/CHANOPT`.
4. Confirm that Windows permits notifications from Fabulor and is not
   suppressing them.
5. Restart Fabulor and check whether the notification option is still present.

If the option is absent, repair or reinstall the current Fabulor package. The
Windows notification helper is an installed Fabulor component and should not
be downloaded separately.

### Tray Alerts Do Not Appear

- Enable **Enable system tray icon**.
- Enable the appropriate **Blink tray icon on** category.
- Check the current tab's **Blink Tray Icon** override.
- Move focus away from Fabulor while testing; tray blinking does not begin
  while the main window is focused.

### Fabulor Closed Instead Of Hiding

Enable **Close to tray** and confirm that the system tray icon is available.
Use the tray menu's **Quit** command only when you intend to end the process.

For the complete Preferences inventory, see [Preferences](preferences.md).
For `/AWAY`, `/BACK`, `/CHANOPT`, and `/TRAY`, see [Commands](commands.md).
