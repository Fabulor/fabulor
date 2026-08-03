# Security Policy

## Supported Versions

Fabulor is currently preparing its first production release. Security fixes
are provided for the latest published `1.0.6` release candidate and the current
release branch only.

| Version | Supported |
| --- | --- |
| Latest `1.0.6` release candidate | Yes |
| Current release branch | Yes |
| Older release candidates | No |
| ZoiteChat, HexChat, and XChat | No |

When a production version is released, this table will be updated to identify
the maintained release line explicitly.

## Reporting A Vulnerability

Do not report suspected vulnerabilities through a public issue, pull request,
discussion, IRC channel, or other public forum.

Once this repository is public, use **Report a vulnerability** in the
repository's **Security and quality** area. This creates a private GitHub
vulnerability report visible to the Fabulor maintainers.

While the repository remains private, contact a Fabulor organization owner
through an existing private channel and ask to establish a secure reporting
channel. Do not include vulnerability details in the initial message or in an
ordinary repository issue.

Include as much of the following as is safe and relevant:

- the affected Fabulor version and installer filename;
- the Windows version and architecture;
- the affected feature, component, or add-on host;
- clear reproduction steps and required configuration;
- the expected and observed behavior;
- the security impact and realistic attack conditions;
- whether exploitation requires local access, user interaction, or an
  untrusted IRC server, user, file, theme, or add-on;
- sanitized logs, screenshots, crash details, or proof-of-concept material;
  and
- any suggested mitigation or fix.

Remove passwords, authentication tokens, IRC credentials, personal messages,
private server details, and other unrelated personal data before submitting a
report.

## Scope

This policy covers security defects in:

- the Fabulor Windows client and installer;
- the native core and GTK4 frontend;
- IRC, TLS, proxy, URI, file, archive, theme, and process boundaries;
- the bundled C#, Python, and Tcl add-on hosts;
- Fabulor-owned native plugins;
- the packaged runtime and dependency integration; and
- the Fabulor build, release, update, and signing processes.

For an unmodified upstream dependency defect, report the issue to the upstream
project unless Fabulor's packaging or use of that dependency creates a
Fabulor-specific vulnerability. Security defects in add-ons maintained in the
separate `Fabulor/add-ons` repository should be reported under that
repository's policy.

General bugs, crashes without a security impact, feature requests, and support
questions should use the normal issue templates.

## Responsible Testing

Only test systems, accounts, networks, and data that you own or have explicit
permission to test. Do not disrupt public IRC networks, other users, project
services, or third-party infrastructure. Do not use real credentials or expose
private data in a proof of concept.

Fabulor does not currently operate a bug bounty program. This policy does not
authorize unlawful activity or testing outside the scope above.

## Response And Disclosure

Maintainers will acknowledge a complete report as soon as practical, assess
its scope and severity, and keep the reporter informed when meaningful
progress occurs. Complex reports may require additional reproduction details
or coordination with an upstream project.

Please allow maintainers a reasonable opportunity to investigate, prepare a
fix, validate release artifacts, and notify affected users before public
disclosure. The disclosure date and content should be coordinated through the
private report. Maintainers will credit reporters who request recognition,
subject to their consent and the accuracy of the report.

Confirmed vulnerabilities may be documented through a GitHub Security
Advisory and, where appropriate, assigned a CVE after a fix or mitigation is
available.
