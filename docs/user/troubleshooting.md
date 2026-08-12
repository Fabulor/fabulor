# Troubleshooting

Status: source-verified draft; installed release-candidate verification is
still required.

Start with the smallest test that separates the failing part from the rest of
Fabulor. Preserve the original profile and diagnostic files until the cause is
understood.

## First Checks

1. Note the Fabulor version shown under **Help > About Fabulor**.
2. Note the Windows edition, version, and build shown by `winver`.
3. Decide whether the problem affects startup, one network, one channel, one
   add-on, or the whole interface.
4. Read the last relevant lines in the server tab.
5. Close Fabulor normally, restart it, and repeat the shortest known steps.
6. Record whether the installation is normal, portable, upgraded, or clean.

Do not begin by deleting the profile, copying random DLLs into the installation
directory, disabling TLS verification, or repeatedly changing unrelated
settings.

## Safe Mode

If the Start menu shortcut was installed, open **Fabulor Safe Mode**. It starts
Fabulor with automatic connections and automatic plugin loading disabled.
This distinguishes many connection or add-on faults from core startup faults.

The equivalent installed-mode command is:

```text
"C:\Program Files\Fabulor\fabulor.exe" --no-auto --no-plugins
```

Safe mode is temporary. Starting Fabulor normally restores the saved
auto-connect and add-on behaviour.

For portable mode, run the same options against `fabulor.exe` in the portable
installation directory.

## Test With A Clean Temporary Profile

When safe mode still shows the problem, test without changing the real
profile. Close Fabulor and run this from PowerShell:

```powershell
& 'C:\Program Files\Fabulor\fabulor.exe' `
  --cfgdir "$env:TEMP\Fabulor-Test" --no-auto --no-plugins
```

This creates a separate temporary configuration. If the problem disappears,
the installed application is probably intact and the cause is somewhere in
the normal profile. Do not copy the entire old profile into the test directory;
restore one relevant file or feature at a time.

To confirm which profile a normal start uses, run:

```text
"C:\Program Files\Fabulor\fabulor.exe" --configdir
```

Fabulor displays the active user configuration directory and exits.

## Installation And Maintenance

Fabulor supports Windows 11 x64. A failure on Windows 10 or a 32-bit Windows
installation is outside the supported production target.

Close Fabulor before installing, updating, modifying, repairing, or
uninstalling it. Start the project-supplied `FabulorSetup.exe` and approve the
Windows administrator prompt. Do not launch an extracted internal MSI when the
bootstrapper is available.

### Setup Does Not Start Or Complete

- Confirm the setup file finished downloading and is not blocked or
  quarantined by security software.
- Confirm another installation or Windows update is not already in progress.
- Restart Windows once if the Windows Installer service reports that it cannot
  be accessed.
- Expand **Show details** and record the final status and relevant displayed
  lines. Setup expands this section automatically after an error.
- Use **Copy error details** when reporting the failure, and use **Open log
  folder** to find the complete timestamped session log.
- Do not disable antivirus protection merely to force an unknown installer to
  run.

Persistent setup logs are stored under:

```text
%LOCALAPPDATA%\Fabulor\Installer\Logs
```

Setup retains the ten newest ordinary logs. Logs from failed sessions are
marked as failed and retained for up to 90 days so an intermittent setup
problem can still be investigated later.

If setup reports that a newer Fabulor version is already installed, use that
version's maintenance entry or a newer bootstrapper. Do not force a downgrade
over the existing installation.

### Installed Files May Be Missing

Run the current setup program and choose **Repair**. Repair is appropriate when
the application, GTK4 runtime, notification helper, plugin runtime, or a
selected first-party plugin has been removed or quarantined.

Do not download individual GTK, Enchant, Python, Tcl, .NET, or frontend DLLs
from unrelated sites. Fabulor's runtime is private, versioned, and installed as
one validated payload.

### A Clean Reinstall Is Needed

1. Close Fabulor.
2. Back up `%APPDATA%\Fabulor` to a protected location.
3. If a genuinely clean profile is required, rename the original profile to
   `Fabulor.backup` rather than deleting it.
4. Uninstall through Fabulor Setup or Windows Installed apps.
5. Restart Windows if files were locked.
6. Install the current package.
7. Test once before restoring individual settings or add-ons from the backup.

The installed application and user profile are separate. Reinstalling the
application does not prove that a profile-specific problem has been removed.

## Startup Problems

### Fabulor Does Not Open

1. Check Task Manager for an existing hidden or unresponsive `fabulor.exe`.
2. Try **Fabulor Safe Mode**.
3. Try a clean temporary profile.
4. Run **Repair** if both attempts fail.
5. Check **Event Viewer > Windows Logs > Application** for an event at the
   failure time.

Record the application version, faulting module, exception code, and timestamp.
Do not assume that `ntdll.dll` or `ucrtbase.dll` is the true cause merely
because Windows names it as the faulting module.

### The GTK4 Frontend Cannot Load

A message such as **Fabulor could not load its GTK-4 frontend module** or
Win32 error `126` normally means the frontend or one of its packaged
dependencies is missing or unavailable.

- Run **Repair**.
- Check whether security software quarantined a file beneath
  `C:\Program Files\Fabulor`.
- Do not add a system-wide GTK installation or change `PATH` to compensate.
- Reinstall from the complete bootstrapper if repair cannot restore the
  payload.

### Fabulor Opens With Unexpected Old Settings

Use `--configdir` to confirm the active profile. Check whether a portable
`Config` directory or `portable-mode` marker is causing a different profile to
be used. Do not edit two profiles and expect them to stay synchronised.

## Connection Problems

The server tab is the primary connection diagnostic. It shows address lookup,
connection attempts, TLS, proxy negotiation, IRC registration, SASL, and
service errors.

### Nothing Connects

- Confirm Windows has network access.
- Confirm the server address and port in **Fabulor > Network List**.
- Confirm TLS is enabled for a TLS port.
- Temporarily bypass the configured proxy to separate proxy failure from IRC
  failure.
- Disable auto-connect and test one network at a time.

### One Network Does Not Connect

- Read the exact server-tab error before changing settings.
- Confirm the network's current server addresses and required authentication
  method.
- Try another server supplied by the same network when its round-robin address
  includes an unavailable result.
- Compare a direct connection with the configured bouncer or proxy.

DNS lookup, address fallback, server ident checks, TLS negotiation, and IRC
registration occur before the normal lag meter can describe an established
session. A low lag-meter value does not prove that connection setup was fast.

### TLS Fails

- Check the computer's date and time.
- Confirm the address matches the certificate name expected by the network.
- Confirm the selected port is the network's TLS port.
- Do not enable **Accept invalid TLS certificates** merely to hide the error.

When a network changes its certificate unexpectedly, confirm the change with
the network operator before weakening verification.

### Authentication Fails

- Confirm the selected login method matches the network or bouncer
  instructions.
- Distinguish the IRC nickname from the services account name.
- Re-enter the saved password deliberately rather than editing the placeholder.
- Use SASL PLAIN only over TLS.
- Do not add a second literal password to **Connect commands** when the normal
  saved-password field is intended.

### A SOCKS5 Proxy Rejects Authentication

An error that the proxy rejected all offered authentication methods usually
means the Fabulor **Proxy Authentication** setting and the proxy's policy do
not match.

- Disable authentication for a proxy that accepts anonymous connections.
- Enable it and provide both username and password when the proxy requires
  username/password authentication.
- Confirm the proxy type is **SOCKS5**, not SOCKS4 or HTTP.
- Confirm the selected network is not set to bypass the proxy.

### Connected Through ZNC But Startup Looks Different

ZNC may already be connected and can deliver server output, scrollback, and
joined channels almost immediately. Fabulor retains the server-tab output even
when the first autojoin channel becomes active. Reselect the server tab to
inspect the complete sequence.

See [Networks and connections](networks-and-connections.md) for detailed
configuration and error categories.

### An IRC Link Does Not Open The Expected Channel

- Confirm the address begins with `irc://` or `ircs://` and contains a server
  name.
- Until [issue #298](https://github.com/Fabulor/fabulor/issues/298) is resolved,
  Windows can open an `irc://` link in another registered application and does
  not have an installed Fabulor association for `ircs://`. Test either scheme
  with the explicit `--url` PowerShell example in
  [Networks and connections](networks-and-connections.md).
- If the channel is already open, Fabulor should select it rather than send a
  second join request.
- If several saved ZNC networks share one bouncer hostname, the URI cannot
  identify the intended virtual network. Fabulor uses the first matching
  connected network.
- Record any **Invalid IRC URL** message exactly. Do not remove escaping or
  alter the address repeatedly merely to bypass validation.
- Portable mode deliberately has no Windows protocol registration.

## Channels, Messages, And Commands

### A Channel Cannot Be Joined

Read the numeric or service response. The channel may require an invitation,
key, registered account, different name, or permission not controlled by
Fabulor.

### A Command Does Nothing

- Use `/HELP <command>`.
- Confirm the command belongs to Fabulor rather than another IRC client.
- Confirm the current tab supplies the required server, channel, or nickname
  context.
- Check **Window > Plugins and Scripts** when an optional plugin supplies it.
- Inspect **Settings > User Commands** for an alias with the same name.

See [Commands](commands.md) for syntax, aliases, and context rules.

### Sent Text Disappears But Is Not Displayed Immediately

The input box clears when Fabulor accepts the line. Display may still wait for
server echo, local event processing, a busy transcript redraw, or an add-on.
Compare another channel, start without add-ons, and distinguish network lag
from interface delay using the performance steps later on this page.

## Interface And Display

### The Layout Is Compressed Or A Pane Is Missing

- Maximise the window once and switch between two channels.
- Confirm **View > User List**, **View > Topic Bar**, and the chosen channel
  switcher are enabled.
- Drag the relevant divider to a usable position, then close Fabulor normally
  so the value can be saved.
- Confirm the profile is writable if the layout resets after every restart.

To hold a deliberately chosen user-list width, use
`/SET gui_ulist_resizable OFF`, set the desired
`/SET gui_ulist_nick_width <pixels>`, and close normally.

### Transcript Text Cannot Be Selected Or Copied

Drag over the intended characters. If automatic copying is enabled, releasing
the mouse copies the selection; otherwise use `Ctrl+Shift+C`. Check the
selection options under **Settings > Preferences > Chatting > Advanced**.
Clearing the transcript is unrelated to clipboard copying.

### A Link Does Not Open

Confirm that Windows has a default browser for the link's protocol. Try
right-clicking the link and choosing **Open Link in Browser**. If ordinary text
selection also fails, treat it as a transcript interaction problem rather than
a browser problem.

### The Tray Icon Does Not Restore The Window

- Use the tray menu's **Restore Window** command.
- Check Windows taskbar corner-overflow settings for the Fabulor icon.
- Confirm the window is not on another virtual desktop.
- Disable **Minimize to tray** temporarily and reproduce with an ordinary
  taskbar window.

If the process is visible but the window remains inaccessible, record the
steps before ending it through Task Manager.

## Spell Checking

Spell checking is configured under **Settings > Preferences > Interface >
Input box**.

### Words Are Not Underlined

- Confirm **Spell checking** is enabled.
- Confirm **Dictionaries to use** contains valid language codes separated by
  commas.
- Check the available dictionary folders beneath
  `%LOCALAPPDATA%\enchant\myspell\dicts`.
- Test an ordinary word; URL-shaped tokens are intentionally excluded.
- Right-click a misspelled word, not empty input or a link.

### Suggestions Or Personal Dictionary Changes Do Not Persist

Close Fabulor normally after adding a word. Confirm the Windows account can
write its local application-data directories. Do not copy dictionary files
into `Program Files`.

If the complete spelling menu is absent after repair, report the installed
feature state and startup messages. Do not download a replacement Enchant DLL;
Fabulor packages the supported Enchant and WinSpell build.

## Emoji And Flags

- Press `Escape` to close the picker from the keyboard.
- If a category looks empty, switch category once and reopen the picker.
- Search flags by country name or two-letter code.
- Repair the installation if packaged flag artwork is missing.

The installed flag images belong under Fabulor's shared application data, not
the user profile. See [Main window](main-window.md) for picker behaviour.

## Themes And Colours

Use **System default** to separate a desktop-theme problem from transcript
colours. Use **Current colours** or the bundled Fabulor Dark palette to
separate a palette problem from GTK4 widget styling.

If transcript colours are unusable, close Fabulor, back up the profile, and
remove only `colors.conf`. If a desktop archive is rejected, keep the status
message; do not extract it manually over installed files. Detailed rollback,
removal, and recovery steps are in [Themes and colours](themes-and-colours.md).

## Sounds, Notifications, And Tray Alerts

Use **Play** on the Sounds page to test the stored `.wav` independently of an
IRC event. If a notification or alert is missing, check global alert settings,
the current tab's overrides, focused-window suppression, Windows notification
permissions, and whether Fabulor was closed normally after the change.

If the Sounds event table or Windows notification option remains absent after
a restart, run **Repair**. See [Sounds and alerts](sounds-and-alerts.md) for the
complete checklist.

## Add-On Problems

1. Start in safe mode.
2. Confirm the folder and primary filename match.
3. Read the startup report and preceding failure messages.
4. Restore or enable one add-on at a time.
5. Rebuild C# add-ons against the current contract and update retired Python or
   Tcl API names.

Do not diagnose an add-on by copying runtimes into the installation directory.
See [Add-ons](addons.md) for language-specific errors, manifest blacklisting,
and `--no-plugins` recovery.

## Settings Do Not Persist

- Choose **OK** in Preferences rather than **Cancel**.
- Close Fabulor normally; forced termination can lose pending state.
- Confirm the active profile with `--configdir`.
- Confirm the profile and its files are writable by the current Windows user.
- Avoid editing a configuration file while Fabulor is running because a normal
  save can replace the manual edit.
- Check the server tab or warning dialog for a failed `fabulor.conf`,
  `colors.conf`, `sound.conf`, history, or log write.

Back up before manual repair. Rename a suspect file and test its freshly
generated replacement instead of immediately deleting the only copy.

## Interface Performance

The IRC lag meter measures server round-trip delay. It does not measure channel
switching, transcript wrapping, user-list replacement, spell checking, theme
changes, or add-on execution.

For a reproducible interface delay:

1. identify one network and channel that demonstrates it;
2. compare a quiet channel and a busy channel;
3. start without add-ons;
4. test **System default** with colour stripping disabled; and
5. note whether transcript size, user count, or first-open work changes the
   delay.

To collect Fabulor's optional timing log, close Fabulor, open PowerShell, and
run:

```powershell
$env:FABULOR_PROFILE_UI = '1'
& 'C:\Program Files\Fabulor\fabulor.exe'
```

Reproduce the shortest sequence, close Fabulor, then disable logging in that
PowerShell session:

```powershell
Remove-Item Env:FABULOR_PROFILE_UI
```

For normal installed mode, the result is
`%APPDATA%\Fabulor\ui-performance.log`; portable mode writes it in the portable
profile. It can contain channel names. Review and redact it before sharing,
then remove it when diagnosis is complete. Do not leave profiling enabled
during ordinary use.

## Preparing A Useful Bug Report

Include:

- the Fabulor version;
- the Windows version and build;
- clean, upgraded, portable, or development installation;
- the shortest exact reproduction steps;
- expected and actual behaviour;
- whether safe mode and a clean temporary profile change the result;
- add-on, ZNC, proxy, and TLS involvement; and
- the relevant redacted screenshot, server output, setup status, Event Viewer
  entry, or performance-log section.

Use **Help > Report an Issue** for an ordinary reproducible bug. Before
uploading anything, follow the redaction checklist in
[Security and privacy](security-and-privacy.md). Do not publish passwords,
tokens, private keys, private messages, or an undisclosed vulnerability.
