# Security And Privacy

Status: source-verified draft; installed release-candidate verification is
still required.

Fabulor is an IRC client. It connects to services selected by the user and
stores configuration and optional conversation data on the local computer.
Security therefore depends on Fabulor, Windows, the chosen IRC network or
bouncer, installed add-ons, and the way local files are handled.

This page explains the current user-facing security boundary. Detailed design
and audit records remain in the [security documentation](../security/README.md).

## What IRC Reveals

Your nickname, user name, real-name field, messages, joins, parts, and other
IRC activity are sent to the selected server or bouncer. Networks may also see
your connection address and record protocol traffic. Channel messages are
visible to the channel's participants and infrastructure; private messages are
not automatically end-to-end encrypted.

The **Real name** field is public IRC connection information. It does not need
to contain your legal name. Avoid putting an email address, password, or other
secret in identity fields, quit messages, away messages, channel topics, or
commands sent to a channel.

Network operators, bouncer operators, channel participants, bots, and add-ons
may each keep their own logs. Clearing a local Fabulor transcript or log cannot
erase copies held elsewhere.

## TLS And Server Identity

Use TLS whenever the IRC network or bouncer provides it. Fabulor verifies the
certificate chain and server name using its packaged trust store.

TLS protects traffic between Fabulor and the endpoint to which Fabulor is
connected. It does not make channel messages private from that endpoint or
their recipients. When connecting through ZNC, the Fabulor-to-ZNC connection
and ZNC's onward connection are separate security boundaries.

Leave **Accept invalid TLS certificates** disabled. Enabling it permits a
connection whose server identity could not be verified. It is an explicit
trusted-configuration exception, not a repair for an expired, mismatched, or
untrusted certificate. Limit it to a temporary test of a server you control.

SASL PLAIN and IRC `/PASS` transmit reusable credentials within the IRC
protocol. Use them only across TLS. SCRAM mechanisms avoid sending the password
itself but still require correct server authentication.

See [Networks and connections](networks-and-connections.md) for the current TLS,
SASL, ZNC, and certificate controls.

## Saved Passwords

For a normal installed copy, enable **Store password in Windows Credential
Manager** for each network. The Fabulor profile then stores a reference to the
network-specific Windows credential rather than the password itself.

When Credential Manager storage is disabled, Fabulor uses encrypted profile
storage. That protects the password from casual reading, but the profile and
its backups must still be treated as sensitive. Portable installations use
this profile-storage path so they remain self-contained.

The password field displays a placeholder when a saved password already
exists. Leave it unchanged to retain the secret. Reveal a password only when
another person cannot see or record the screen.

Before deleting a network whose password is in Credential Manager, edit the
network, clear its saved password, and save the change. You can also review and
remove obsolete Fabulor entries through Windows Credential Manager.

### Secrets That Are Not Network Passwords

Proxy username and password fields are saved with the Fabulor preferences.
Treat `fabulor.conf` and every profile backup containing it as sensitive.

Custom connect commands are stored in `servlist.conf`. Do not type a literal
password into a custom command when the supported `%p` saved-password
substitution can be used. A password entered directly in Fabulor's input box
may also be retained in saved input history.

## Client Certificates

Fabulor can import a PEM client certificate and matching private key for a
network. Imported material is copied beneath the profile `certs` directory.
The private key proves the client's identity and must be protected like a
password.

Use client certificates only when a network or bouncer requires them. Verify
certificate details before selecting **SASL EXTERNAL (cert)**. Remove a
certificate through the Network List when it is no longer required, and do not
share a profile backup containing private-key material.

## Proxies And Bouncers

A proxy or bouncer changes which systems carry or retain connection data; it
does not make IRC activity anonymous or end-to-end encrypted.

- A proxy provider can observe connection metadata and may see unencrypted
  traffic.
- A bouncer receives your IRC session and may retain scrollback, credentials,
  or logs according to its own configuration.
- TLS should remain enabled through a proxy and on the connection to a
  bouncer.
- Use proxy authentication only when required, and protect the profile because
  those credentials are saved there.

Fabulor's reviewed SOCKS5 path supports no authentication or username/password
authentication. SOCKS4 remains available pending a separate product decision,
but it lacks the authentication and addressing features of SOCKS5.

## Direct Chat And File Transfers

DCC chat and file transfer can establish a direct connection between clients.
This may reveal network-address information to the other party and bypass the
IRC server's normal message path.

Treat every received file as untrusted. Keep confirmation enabled, inspect the
filename and sender, save to a controlled location, and scan the file before
opening it. A checksum detects an accidental or deliberate content change only
when it is compared with a trustworthy expected value; it does not prove that
the file itself is safe.

## Local Profile Data

Installed mode normally stores personal state under:

```text
%APPDATA%\Fabulor
```

Portable mode stores it beneath `Config` beside `fabulor.exe`. Depending on
enabled features and use, the profile can contain:

- network names, identities, server addresses, autojoin channels, and connect
  commands;
- encrypted saved passwords or references to Windows credentials;
- proxy credentials;
- imported client certificates and private keys;
- saved input history and conversation logs;
- notify and ignore lists, aliases, replacements, and event configuration;
- captured URLs, sounds, colour choices, and theme files;
- add-ons and data maintained by those add-ons; and
- optional performance diagnostics.

Protect the Windows account with a strong sign-in method and appropriate disk
encryption. Back up the profile only to a protected location. Closing Fabulor
normally before copying the profile reduces the chance of incomplete files.

Deleting or uninstalling Fabulor does not necessarily remove every profile
backup, exported log, downloaded file, browser record, Windows credential, or
copy held by an IRC service. Review those locations separately when removing
sensitive data.

## History, Logs, And Captured URLs

Saved input history can contain commands and unsent drafts as well as sent
messages. Conversation logs can contain private messages, account names,
hostmasks, IP addresses, channel keys, and other sensitive content. Disable
storage you do not need.

Use `/CLEAR HISTORY` and `/CLEAR LOG` to clear the current context. These
commands do not clear every network or channel, browser history, clipboard
history, remote logs, or backup copy. See
[Input history and conversation logs](history-and-logs.md) for their exact
scope.

The Raw Log shows unprocessed protocol traffic and can expose more than the
formatted transcript. The URL Grabber and URL logging can retain addresses
that reveal private interests, tokens, or query parameters. Review all such
content before sharing it.

## Clipboard, Notifications, And Links

Automatic transcript copying places selected text on the Windows clipboard.
If Windows clipboard history or device synchronisation is enabled, copied
private text may persist outside Fabulor. Clear sensitive clipboard entries
after use.

Desktop notifications can display private-message or highlight content on
screen. Configure Fabulor's alert settings and Windows notification privacy for
the environment in which the computer is used.

Opening a link hands it to the default browser. The destination then receives
normal browser connection information and may track the visit. Inspect the
address before opening it, especially when it came from an unknown IRC user.
Fabulor recognising a string as a URL is not a safety judgement.

IRC protocol links use a separate strict parser. Fabulor accepts only the
`irc` and `ircs` schemes with a bounded host, optional valid port, and at most
one channel and key. It rejects user information, fragments, malformed
escaping, control characters, and command separators before the address can
cross the Windows existing-instance boundary. The handoff carries a typed IRC
address and cannot carry an arbitrary Fabulor command. These checks prevent
command interpolation; they do not make an unknown IRC destination safe.

Avoid placing a private channel key in an IRC address. A link may be retained
in chat, browser history, clipboard history, diagnostics, or a process command
line. Configure persistent secrets through the appropriate saved network
settings instead.

## Add-Ons And Commands

Simple add-ons, native plugins, and manifest plugins execute locally as part of
Fabulor. Manifest capability checks limit cooperating access to Fabulor APIs;
they do not create an operating-system sandbox. Add-on code may still use its
language runtime and the permissions of your Windows account.

Review an add-on before installing it. Start with `--no-plugins` if a new
add-on causes suspicious or unstable behaviour. See [Add-ons](addons.md) for
safe installation, blacklisting, removal, and recovery.

The optional Exec plugin deliberately runs operating-system commands. Its
command buffer is bounded, but a harmful command remains harmful. Do not run an
`/EXEC` line copied from chat or supplied by an untrusted script.

FiSHLiM is optional message-encryption functionality. It applies only where it
is deliberately configured and does not replace TLS, safe key handling, or
trust in the other participant.

## Themes And Imported Files

Colour themes and GTK4 desktop themes are local files processed by Fabulor.
Use only the approved sources listed in [Themes and colours](themes-and-colours.md).
Fabulor contains archive extraction and theme-path checks, but those checks do
not make an untrusted download desirable.

## Diagnostics And Bug Reports

Fabulor does not automatically upload telemetry or crash reports. Optional UI
performance logging writes `ui-performance.log` in the profile when the
diagnostic environment switch is enabled. That file can include channel names
and timing information.

Before sharing a screenshot, Event Viewer record, server-tab transcript, Raw
Log, configuration file, crash dump, or performance log:

1. make a copy rather than editing the only diagnostic record;
2. remove passwords, tokens, channel keys, private messages, and client
   certificate material;
3. remove or obscure IP addresses, hostmasks, account names, private channel
   names, and unrelated conversations; and
4. check the result again after redaction.

A public issue is not an appropriate place for an undisclosed vulnerability,
working exploit, private key, or live credential. Contact the project
maintainers privately before providing sensitive security material.

## Updates And Release Verification

Fabulor does not currently perform in-client update checks. The inherited
updater was removed because it did not have a Fabulor-controlled trust and
release boundary. **Help > Check for Updates** therefore remains retired.

A future manual updater is designed but blocked until signed metadata,
Fabulor-controlled release keys, Authenticode identity, bounded downloading,
rollback protection, and installed acceptance are complete. It must not
silently download or install an update.

Until that work is activated, obtain installers only from the Fabulor project
release location communicated by the maintainers. Compare published hashes
when provided. Do not treat a familiar filename, a chat message, or an
unverified mirror as proof that an installer is authentic.

The current design and activation gate are documented in the
[signed update feed design](../security/signed-update-feed-design.md).
