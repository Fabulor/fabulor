# Privacy Policy

Fabulor is a locally installed IRC client. The Fabulor project does not
operate an account service, advertising service, analytics service, or
telemetry endpoint for the application.

**This program will not transfer any information to other networked systems
unless specifically requested by the user or the person installing or
operating it.**

## User-Directed Network Activity

Fabulor transfers information when the user configures or invokes a feature
that requires a network connection. This includes:

- connecting to an IRC server, IRC network, bouncer, or proxy selected by the
  user;
- sending IRC identity fields, authentication data, messages, commands, and
  connection activity to that selected service;
- opening an IRC address or web link selected by the user;
- starting a direct chat or file transfer;
- using an installed add-on that performs network activity; and
- visiting the Fabulor repository, release pages, issue tracker, or other
  support location in a web browser.

Those destinations are independent services selected by the user. Their own
operators and privacy policies govern information they receive. In
particular, IRC networks, bouncers, proxies, linked websites, and add-on
services may log connection details or content independently of Fabulor.

## Data Stored On The Computer

Depending on the settings and features used, Fabulor may store network
configuration, identities, credential references, encrypted passwords, proxy
credentials, imported client certificates, input history, conversation logs,
URLs, themes, sounds, add-ons, and diagnostic timing data in the user's
Fabulor profile. Network passwords may instead be stored in Windows Credential
Manager when that option is enabled.

Fabulor does not automatically upload crash reports, diagnostic logs,
conversation logs, configuration, or telemetry. The user decides whether to
share any diagnostic material in a report.

Detailed storage locations, clearing commands, credential behavior, and
redaction guidance are documented in
[Security and privacy](docs/user/security-and-privacy.md).

## Installation And System Integration

The installer makes only the system changes selected or required for the
Fabulor installation, such as installing program files and optional shortcuts
or IRC protocol registration. Windows provides the installation, repair, and
uninstallation boundary.

Opening web links hands the address to the user's default browser. Desktop
notifications, clipboard history, credential storage, and browser activity
are then governed by Windows and the selected browser rather than by a
Fabulor-operated service.

## Releases And Code Signing

GitHub hosts the source repository, issue tracker, workflow records, and
release downloads. Users who choose to visit those services are subject to
[GitHub's privacy statement](https://docs.github.com/en/site-policy/privacy-policies/github-general-privacy-statement).

SignPath.io and SignPath Foundation will be used to verify and sign approved
Fabulor release artifacts after signing is activated. Build provenance,
release artifacts, and signing records are processed for that purpose; Fabulor
user profiles and IRC content are not part of the signing submission. See the
[Code signing policy](CODE_SIGNING_POLICY.md).

## Changes To This Policy

Material changes to Fabulor's network behavior, data handling, update process,
or third-party services require a reviewed update to this policy before the
affected feature is released. The repository history records policy changes.
