# Networks And Connections

Status: source-verified draft; installed release-candidate verification is
still required.

Fabulor stores connection details as named networks. Each network may contain
one or more IRC servers, its own identity and login settings, autojoin
channels, and commands to run after connection.

Open the editor through **Fabulor > Network List**.

## Network List

The main Network List contains global identity fields and the saved network
list.

![Fabulor's Network List showing blank global identity fields, the saved
network list, favourite filtering, and connection controls.](images/network-list.png)

- **Add** creates a network.
- **Remove** deletes the selected network definition.
- **Edit...** opens the selected network's complete settings.
- **Sort** orders the list alphabetically.
- **Favor** marks or unmarks the selected network as a favourite.
- **Show favorites only** filters the visible list.
- **Connect** starts the selected network.
- **Close** closes the Network List without starting a connection.

Use `Shift+Up` and `Shift+Down` to move a selected network without sorting the
complete list. A single pointer click selects a row; renaming remains an
explicit edit action.

## Servers

Select a network and then **Edit...**. The **Servers** page accepts one server
per row. Use the form:

```text
irc.example.net/6697
```

![Fabulor's network editor showing a test server, TLS controls, global identity
selection, Windows Credential Manager storage, blank identity and password
fields, and the UTF-8 character set.](images/network-editor.png)

The port belongs after `/`. The network-wide **Use TLS for all the servers on
this network** setting applies TLS to every server entry. A `+` before an
individual port also identifies that entry as TLS, but using the visible
network setting is clearer.

When a network contains several servers, Fabulor can try the next entry after a
failed connection. Enable **Connect to selected server only** when you want to
prevent that cycling behavior.

## Identity

Enable **Use global user information** to use the identity from the main
Network List. Clear it to set network-specific values:

- **Nick name**;
- **Second choice**;
- **Real name**; and
- **User name**.

The real name is public IRC connection information, not a private profile
field. It does not have to contain your legal name.

## TLS

Enable **Use TLS for all the servers on this network** whenever the network or
bouncer provides a TLS port. Fabulor verifies the certificate chain and server
name using its packaged trust store.

Leave **Accept invalid TLS certificates** disabled. Enabling it weakens server
authentication and should be limited to a deliberate, temporary test of a
server you control.

See [Security and privacy](security-and-privacy.md) for the endpoint boundary,
local credential storage, and sensitive profile data.

Fabulor can import a PEM client certificate for a network, show its certificate
details, and remove it. Client certificates are needed only when the network or
bouncer explicitly requires certificate authentication. **SASL EXTERNAL
(cert)** uses the imported certificate; do not select it without one.

## Login Methods

Select the method required by the network:

- **SASL PLAIN (username + password)** sends the supplied account name and
  password through the IRC SASL exchange. Use it only with TLS.
- **SASL SCRAM-SHA-1**, **SASL SCRAM-SHA-256**, and **SASL SCRAM-SHA-512** use
  challenge-response authentication when supported by the server.
- **SASL EXTERNAL (cert)** authenticates with the imported client certificate.
- **Server password (/PASS password)** sends the password during initial IRC
  registration. This is commonly required by bouncers.
- The two **NickServ** choices identify after connection using the displayed
  NickServ command form.
- **Challenge Auth** is a network-specific challenge-response method.
- **Custom... (connect commands)** leaves authentication to commands on the
  **Connect commands** page.

If the network provides instructions, follow its exact login method rather than
trying methods at random. A method unsupported by the server will be reported
in the server tab.

## Password Storage

Installed mode should normally use **Store password in Windows Credential
Manager**. If that option is disabled, Fabulor stores the password in encrypted
profile storage.

The password entry displays a placeholder for an existing saved secret. Leave
that placeholder unchanged to retain the current password. Use **Show
password** only where another person cannot see the screen.

Passwords are network-specific. Changing a network name can therefore affect
the Windows Credential Manager entry associated with it.

## Autojoin Channels

The **Autojoin channels** page stores channels for the selected network. Add the
channel name including its prefix, for example:

```text
#fabulor
```

Add a channel key only when the channel requires one. Fabulor joins these
channels after the network connection and authentication sequence reaches the
appropriate point.

The channel list's context menu can also toggle **Autojoin Channel** for the
selected network.

## Connect Commands

The **Connect commands** page runs saved IRC commands after connection. Enter
commands without exposing secrets that could instead use the dedicated
password field and login method.

The editor recognises these substitutions:

- `%n`: nickname;
- `%p`: saved network password;
- `%r`: real name; and
- `%u`: username.

Because `%p` expands to a secret, use it only when the service requires a
custom command and no supported login method is suitable.

## Automatic Connections

Enable **Connect to this network automatically** in every network that should
connect when Fabulor starts. The main **Skip network list on startup** option
hides the Network List; it does not itself enable automatic connection.

Test each automatic network before skipping the list. Autojoin channels and
connect commands belong to their own network and do not run against another
network.

## ZNC And Other Bouncers

Create a separate Fabulor network entry for each bouncer network. Use the
bouncer's host and TLS port, then enter the username, network identifier, and
password in the exact form supplied by the bouncer administrator. A common ZNC
configuration combines the ZNC username and network name in the IRC username,
but installations can differ.

For most ZNC configurations:

1. enable TLS;
2. select **Server password (/PASS password)** unless the bouncer specifically
   provides SASL instructions;
3. store the ZNC password in Windows Credential Manager; and
4. add the bouncer network's channels to **Autojoin channels** only when ZNC is
   not already managing them as expected.

A bouncer may deliver connection output and joined channels much faster than a
direct IRC connection. Fabulor retains the server-tab output and supports
`/CYCLE <channel>` by waiting for the server's part confirmation before
rejoining.

## IRC Links

Fabulor accepts validated `irc://` and `ircs://` addresses. A channel belongs
in the path without a leading `#`, for example:

```text
ircs://irc.example.net:6697/fabulor
```

When Fabulor is already running, the link is handed to that instance as a
typed IRC address rather than as a general command. If the matching connection
is active, Fabulor joins an unopened channel or selects the channel when it is
already open. Otherwise it starts a connection for the supplied server.

Installed shell integration currently registers Fabulor for `irc://` links.
Both schemes can be tested explicitly from PowerShell:

```powershell
& 'C:\Program Files\Fabulor\fabulor.exe' `
  --url='ircs://irc.example.net:6697/fabulor'
```

Windows-wide `ircs://` association remains a release task. Portable mode does
not register either protocol.

An IRC address identifies a server host, not a saved Fabulor network name. If
several ZNC network definitions use the same bouncer host, the address cannot
distinguish those virtual networks. Fabulor uses the first matching connected
network; use a network-specific bouncer hostname when reliable external-link
routing matters.

Fabulor rejects malformed ports, user information, fragments, multiple
channels, invalid escaping, control characters, and oversized components.
Validation protects the command boundary; it does not establish that the
server or channel is trustworthy.

## SOCKS5 Proxy

Proxy settings are global under **Settings > Preferences > Network > Network
setup**.

1. Enter the proxy **Hostname** and **Port**.
2. Select **SOCKS5** as the proxy **Type**.
3. Choose whether to use it for all connections, the IRC server only, or DCC
   only.
4. Enable **Use authentication** and enter the username and password only when
   the proxy requires them.
5. In an individual network, clear **Bypass proxy server** to permit that
   network to use the configured proxy.

SOCKS5 supports unauthenticated and username/password negotiation in Fabulor.
If the server tab reports that the proxy rejected every offered authentication
method, make the authentication setting match the proxy configuration.

SOCKS4 remains visible for legacy compatibility but is not the recommended
choice for new configuration.

## Connection Health

The Network setup Preferences page also contains TCP keepalive idle, interval,
and probe values. Leave these at their defaults unless diagnosing stale
connections with a network administrator. They do not measure IRC response
lag.

The graphical lag meter measures IRC round-trip responsiveness. A low lag
reading does not rule out local interface delay, proxy setup delay, DNS lookup
time, TLS negotiation, or a slow IRC network.

## Diagnosing A Failed Connection

Read the server tab from the first lookup message through the final error.
Common distinctions are:

- lookup failure: server or proxy hostname could not be resolved;
- connection refusal or timeout: host, port, firewall, proxy, or server is
  unavailable;
- TLS rejection: wrong port, invalid certificate, hostname mismatch, or
  incompatible TLS policy;
- SASL failure: wrong account, password, mechanism, or server support;
- proxy traversal failure: proxy type, authentication, or destination policy
  mismatch; and
- successful connection followed by service rejection: network- or
  bouncer-specific credentials are incorrect.

Do not erase the server-tab output before recording the relevant lines for a
bug report.
