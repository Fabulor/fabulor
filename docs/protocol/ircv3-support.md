# IRCv3 Support

Status: source-verified and installed-verified. Rejection-path verification
completed on DALnet, ChatLounge, Libera.Chat, and ZNC. Positive verification
completed on Ergo Testnet on 2026-08-15.

Fabulor negotiates IRCv3 capabilities conservatively. A capability is active
only after the server acknowledges it, capability values are kept separate
from capability names, and negative acknowledgements disable the named
feature.

## Supported Capabilities

Fabulor requests these capabilities when a server advertises them:

- `account-notify`, `account-tag`, `away-notify`, and `extended-join`;
- `batch` and `labeled-response`;
- `cap-notify`;
- `chghost`, `invite-notify`, and `setname`;
- `echo-message`;
- `extended-monitor`;
- `message-tags`;
- `multi-prefix` and `userhost-in-names`;
- `server-time` and the retained ZNC server-time variants;
- `standard-replies`; and
- the retained network-specific capabilities used by Solanum and Twitch.

SASL is requested only when the selected authentication mode is configured and
the server offers the required mechanism. Fabulor supports PLAIN, EXTERNAL,
SCRAM-SHA-1, SCRAM-SHA-256, and SCRAM-SHA-512 according to the selected
Network List setting and build capabilities.

## Batch And Labeled Responses

Fabulor parses IRCv3 message tags, tracks bounded nested batch lifetimes, and
accepts `batch` and `label` tags. At most 64 batches may be active on one
connection. Duplicate batch identifiers, orphaned nested batches, and invalid
identifiers are rejected.

Commands that need response correlation may use a generated per-connection
label when `labeled-response` is active. Labels are not added when the server
does not acknowledge that capability.

## Chat History

Fabulor implements on-demand `draft/chathistory` requests through:

```text
/CHATHISTORY
/CHATHISTORY <subcommand and arguments>
```

With no arguments, the command requests the latest messages for the current
channel or private conversation. The server's `CHATHISTORY` ISUPPORT limit is
honoured and bounded to 500 messages; the fallback request limit is 50.

The capability is intentionally requested only when `/CHATHISTORY` is used.
Negotiating it automatically can change a bouncer's normal playback behavior,
so this policy preserves established ZNC startup and reconnect behavior.
Fabulor records whether the server advertised the capability and rejects the
command locally when it did not. If an advertised capability is subsequently
rejected, Fabulor discards the pending request and reports that chat history is
unsupported.

Installed verification on Ergo Testnet confirmed the positive path. Ergo
advertised `draft/CHATHISTORY=1000`; invoking argument-free `/CHATHISTORY`
caused Fabulor to request the capability, the server acknowledged
`draft/chathistory`, and the history request completed successfully.

History replay uses the existing transcript path, including `server-time`
timestamps and normal message rendering. Fabulor does not provide persistent
server-side history itself; availability and retention are controlled by the
IRC server or bouncer.

## Verification

The protocol test suite covers capability values and disable tokens, supported
capability requests, exact SASL mechanism matching, message-tag escaping and
duplicate handling, bounded batch lifecycle, and generated labels. Installed
verification has covered unsupported connections to DALnet, ChatLounge,
Libera.Chat, and ZNC. It still needs one server or bouncer that advertises
`draft/chathistory`.
