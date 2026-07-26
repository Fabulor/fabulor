# Obsolete Function Retirement Plan

Last updated: 2026-07-26

## Purpose

This document tracks obsolete Fabulor features, configuration keys, and
compatibility code that may be removed. It records why each retirement is
justified, how existing configurations are handled, and which automated and
installed-client checks are required.

Age alone is not sufficient reason to remove a feature. IRC behavior such as
DCC, proxy support, logging, scripting, and tray integration remains supported
unless a separate review establishes that a specific implementation is
obsolete, unsafe, inert, or outside Fabulor's product scope.

## Status

- `Proposed`: identified for investigation; removal is not approved.
- `Approved`: scope and compatibility policy are agreed.
- `Implemented`: code is changed and automated checks pass.
- `Accepted`: installed-client testing passes.
- `Published`: accepted work is committed and pushed to a pull request.
- `Deferred`: intentionally retained until a later cleanup stage.

## Retirement Order

| Order | Feature or key | Status | Compatibility policy |
|---|---|---|---|
| 1 | Built-in Identd service | Published | Ignore old `identd_server` and `identd_port` keys and omit them on the next canonical configuration write. |
| 2 | `gui_ulist_style` | Published | Ignore the inert saved key and omit it on the next canonical configuration write. |
| 3 | Wingate proxy mode | Published | Preserve all other proxy numeric values; treat saved Wingate selection value `1` as disabled. |
| 4 | SOCKS4 proxy mode | Proposed | Review usage, security, DNS, IPv6, and migration impact before deciding. |
| 5 | Retained Perl source and residual configuration | Deferred | Keep source until the final repository cleanup; it remains outside the packaged C#/Python/Tcl plugin model. |
| 6 | Other inert configuration keys | Accepted | `text_transparent` is retired and accepted; audit remaining keys individually. |

## Published Retirements

### Built-In Identd Service

Status: `Published`

Reason:

- Identd is obsolete and was retained as a Windows-specific built-in network
  listener.
- It exposed a command, automatic connection-port publication, persisted
  settings, a Preferences page, and listener lifetime without being required
  for normal IRC operation.

Removed surface:

- internal Identd plugin and `/IDENTD` command
- listener and connection-port publication
- `identd_server` and `identd_port` configuration
- Identd Preferences and apply-time reload behavior
- MSVC, Meson, translation, startup-report, and change-dispatch registrations

Validation:

- common core, GTK4 frontend, and installer build without warnings or errors
- all GTK4 tooling tests and production package validators pass
- installed testing confirms Identd is absent and normal connections work

Published in PR #242.

### `gui_ulist_style`

Status: `Published`

Reason:

- The key had no runtime or frontend reader.
- Its only remaining references were the preference schema, default
  initialization, and preference structure.

Removed surface:

- `gui_ulist_style` persisted schema entry
- unused default initialization
- `hex_gui_ulist_style` preference field

Validation:

- complete source audit contains no remaining references
- core, frontend, and installer builds pass
- installed `/SET gui_ulist_style` reports no such variable
- user-list appearance and behavior remain unchanged

Published in PR #242.

## Published Retirement

### Wingate Proxy Mode

Status: `Published`

Reason:

- The Wingate proxy protocol is an obsolete compatibility path.
- It retains separate IRC and DCC traversal implementations and expands the
  network-facing test and maintenance surface.

Current locations:

- proxy type menu in `src/fe-gtk/setup.c`
- IRC traversal in `src/common/server.c`
- DCC traversal in `src/common/dcc.c`
- proxy type documentation in `src/common/zoitechat.h`

Compatibility requirements:

- Keep the persisted proxy values stable:
  - `0`: disabled
  - `1`: retired Wingate value
  - `2`: SOCKS4
  - `3`: SOCKS5
  - `4`: HTTP
  - `5`: Auto
- Do not renumber SOCKS4, SOCKS5, HTTP, or Auto.
- Normalize a loaded value of `1` to disabled and write `0` on the next
  canonical configuration save.
- Do not alter direct, ZNC, SOCKS, HTTP, or DCC behavior outside the Wingate
  branch.

Required automated validation:

- source audit contains no Wingate traversal or UI references
- focused proxy-value compatibility tests cover values `0` through `5`
- common core and GTK4 frontend build without warnings or errors
- complete GTK4 tooling and native probe suites pass
- installer and all production package validators pass

Required installed validation:

- Preferences no longer offers Wingate
- a saved Wingate value starts with proxying disabled
- direct IRC and ZNC connections remain operational
- retained proxy choices display with the correct persisted values
- ordinary DCC behavior is unchanged when no proxy is selected

Acceptance:

- the production installer passed the complete build and package validation
  suite
- `/SET net_proxy_type 1` normalizes the retired value to `0`
- `/SET net_proxy_type` subsequently reports `0`
- installed ZNC and direct IRC connections operate normally
- the focused proxy-policy probe verifies retained menu/value mappings,
  authentication eligibility, and the unchanged non-proxied DCC boundary

Published in PR #242.

## Retained Proxy Baseline

### SOCKS5 Proxy Mode

Status: `Accepted`

SOCKS5 remains a supported proxy mode and is the validated migration target
for any future SOCKS4 retirement decision. Its retained scope is TCP `CONNECT`
for IRC and DCC, with either no authentication or RFC 1929
username/password authentication. Fabulor does not claim SOCKS5 GSSAPI or UDP
`ASSOCIATE` support.

Hardening:

- share bounded greeting, authentication, destination, and reply validation
  between synchronous IRC and queued DCC traversal
- require both username and password when authentication is enabled
- reject a proxy-selected authentication method that Fabulor did not offer
- reject silent downgrade from requested username/password authentication
- handle partial socket reads and writes and interrupted system calls
- validate protocol versions, reserved bytes, address types, field lengths,
  destination ports, and domain-form reply lengths
- fail queued DCC traversal cleanly when the proxy closes or returns malformed
  data

Validation:

- exact-byte protocol probes cover no-authentication and RFC 1929 requests,
  credential and hostname bounds, IPv4/domain destinations, reply address
  forms, and malformed responses
- strict MSVC and independent Meson/Ninja probes pass
- common core, GTK4 frontend, and production installer build without warnings
  or errors
- all 86 Python contract tests and all production package validators pass
- installed direct IRC and ZNC connections pass with both no authentication
  and username/password authentication
- disabling authentication against an authenticated proxy is rejected rather
  than silently downgraded

SOCKS4 remains `Proposed` and was not changed by this work.

## Review Candidates

### SOCKS4 Proxy Mode

Status: `Proposed`

SOCKS4 lacks modern authentication, DNS, and IPv6 behavior, but it remains a
real proxy protocol rather than an inert setting. Before removal, determine
whether Fabulor users rely on it and whether SOCKS5 is an adequate migration
path. Any decision must preserve the numeric values of the retained proxy
types.

### Retained Perl Source And Residual Configuration

Status: `Deferred`

Perl is not built, packaged, autoloaded, or documented as part of Fabulor's
C#/Python/Tcl plugin model. Its source remains intentionally retained until the
final repository cleanup. That later review should include:

- `plugins/perl`
- `perl_warnings` configuration and preference storage
- obsolete user-facing references suggesting that a Perl plugin can be
  installed
- stale comments and build metadata

Removal must not disturb the maintained C#, Python, Tcl, or native first-party
plugin paths.

### Other Inert Configuration Keys

Status: `Accepted`

Perform a mechanical reference audit over every persisted preference. A key is
a retirement candidate when it has no behavioral reader beyond schema,
defaults, structure storage, tests, or historical documentation.

Each key must be handled as a separate contained stage unless several keys
share one inseparable behavior boundary. Saved-key handling and `/SET`
compatibility must be recorded before implementation.

#### `text_transparent`

Status: `Accepted`

The mechanical reference audit found that `text_transparent` survived only in
the persisted preference schema and `zoitechatprefs` structure. No core,
frontend, plugin, test, build, or packaging behavior read it.

Compatibility policy:

- ignore an existing saved `text_transparent` value
- omit the key on the next canonical configuration write
- report no such variable for `/SET text_transparent`
- retain supported background-image and GTK4 theme behavior unchanged

Automated evidence:

- complete source audit contains no remaining `text_transparent` references
- common core and GTK4 frontend rebuild with zero warnings and errors
- all 86 GTK4 and theme tooling tests pass
- production MSI and bootstrapper rebuild with zero warnings and errors
- all production package and embedded-payload validators pass

Installed acceptance:

- `/SET text_transparent` reports no such variable
- supported appearance behavior remains operational

Publication remains pending.

#### Remaining Audit Results

- `perl_warnings` remains deferred with the retained Perl source.
- `gui_single` is already commented out and is not a live persisted key.

#### `irc_cap_server_time`

Status: `Accepted`

The preference did not control capability negotiation. Fabulor already
requested `server-time`, `znc.in/server-time`, and
`znc.in/server-time-iso` whenever a server advertised them, regardless of the
saved value or Preferences toggle.

Compatibility policy:

- retain unconditional server-time capability negotiation and timestamp
  parsing
- remove the misleading Preferences toggle
- ignore an existing saved `irc_cap_server_time` value
- omit the key on the next canonical configuration write
- report no such variable for `/SET irc_cap_server_time`

Automated evidence:

- source audit confirms the preference has no remaining production reference
- `server-time`, `znc.in/server-time`, and `znc.in/server-time-iso`
  negotiation and parsing remain present
- common core and GTK4 frontend rebuild with zero warnings and errors
- all 86 GTK4 and theme tooling tests pass
- production MSI and bootstrapper rebuild with zero warnings and errors
- all production package and embedded-payload validators pass

Installed acceptance:

- `/SET irc_cap_server_time` reports no such variable
- Preferences no longer exposes the server-time toggle
- normal IRC and ZNC timestamps remain operational

Publication remains pending.

## Retirement Workflow

1. Audit source, UI, build, packaging, tests, and documentation references.
2. Record the compatibility and saved-configuration policy.
3. Obtain approval for the contained removal.
4. Implement without unrelated cleanup.
5. Run focused tests, native builds, the complete tooling suite, installer
   builds, and production package validators.
6. Produce a new installer for installed-client testing.
7. Update this plan and the relevant GTK4 or security records after installed
   acceptance.
8. Commit and push only after the user requests publication.
